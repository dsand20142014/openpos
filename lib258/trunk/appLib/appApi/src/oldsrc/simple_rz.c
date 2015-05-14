
#define VERSION "3.73 1-30-03 by zy"
#define PUBDIR "/usr/spool/uucppublic"


#include <stdio.h>
#include <stdlib.h>

#define LOGFILE "/tmp/rzlog"
#define LOGFILE2 "rzlog"
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>



extern int errno;

#define OK 0
#define FALSE 0
#define TRUE 1
#define ERROR (-1)

/*
 * Max value for HOWMANY is 255.
 *   A larger value reduces system overhead but may evoke kernel bugs.
 */
#ifndef HOWMANY
#define HOWMANY 96
#endif

/* Ward Christensen / CP/M parameters - Don't change these! */
#define ENQ 005
#define CAN ('X'&037)
#define XOFF ('s'&037)
#define XON ('q'&037)
#define SOH 1
#define STX 2
#define EOT 4
#define ACK 6
#define NAK 025
#define CPMEOF 032
#define WANTCRC 0103	/* send C not NAK to get crc not checksum */
#define TIMEOUT (-2)
#define RCDO (-3)
#define GCOUNT (-4)
#define ERRORMAX 5
#define RETRYMAX 5
#define WCEOT (-10)
#define PATHLEN 257	/* ready for 4.2 bsd ? */
#define UNIXFILE 0xF000	/* The S_IFMT file mask bit for stat */

int Zmodem=0;		/* ZMODEM protocol requested */
int Nozmodem = 0;	/* If invoked as "rb" */
unsigned Baudrate = 9600;


#include "rbsb.c"	/* most of the system dependent stuff here */
#include "crctab.c"
char endmsg[90] = {0};	/* Possible message to display on exit */
char Zsendmask[33];	/* Additional control chars to mask */

void bibi(int n);
char *substr();
FILE *fout;

/* send cancel string to get the other end to shut up */
canit()
{
    static char canistr[] =
    {
        24,24,24,24,24,24,24,24,24,24,0
    };

    zmputs(canistr);
    Lleft=0;	/* Do read next time ... */
}

/*
 * Routine to calculate the free bytes on the current file system
 *  ~0 means many free bytes (unknown)
 */
long getfree()
{
    return(2147483647);	/* many free bytes ... */
}

int Lastrx;
long rxbytes;
int Crcflg;
int Firstsec;
int Eofseen;		/* indicates cpm eof (^Z) has been received */
int errors;
int Restricted=0;	/* restricted; no /.. or ../ in filenames */

#define DEFBYTL 2000000000L	/* default rx file size */
long Bytesleft;	/* number of bytes of incoming file left */
long Modtime;		/* Unix style mod time for incoming file */
int Filemode;		/* Unix style mode for incoming file */
long Totbytes, Totalleft;
long Totfiles, Filesleft;
char Pathname[PATHLEN];
char *Progname;		/* the name by which we were called */

int Batch=0;
int Thisbinary;		/* current file is to be received in bin mode */
int Rxbinary=FALSE;	/* receive all files in bin mode */
int Rxascii=FALSE;	/* receive files in ascii (translate) mode */
int Blklen;		/* record length of received packets */

#ifdef SEGMENTS
int chinseg = 0;	/* Number of characters received in this data seg */
char secbuf[1+(SEGMENTS+1)*1024];
#else
char secbuf[1025];
#endif


time_t timep[2];
char Lzmanag;		/* Local file management request */
char Lzconv;		/* Local ZMODEM file conversion request */
char zconv;		/* ZMODEM file conversion request */
char zmanag;		/* ZMODEM file management request */
char ztrans;		/* ZMODEM file transport request */
int Zctlesc;		/* Encode control characters */
int Zrwindow = 1400;	/* RX window size (controls garbage count) */

/*
 * Log an error
 */
void
zperr1(s)
char *s;
{
    if (Verbose <= 0)
        return;
    fprintf(stderr, "Retry %d: ", errors);
    fprintf(stderr, s);
    fprintf(stderr, "\n");
}

void
zperr2(s,p)
char *s, *p;
{
    if (Verbose <= 0)
        return;
    fprintf(stderr, "Retry %d: ", errors);
    fprintf(stderr, s, p);
    fprintf(stderr, "\n");
}

void
zperr3(s,p,u)
char *s, *p, *u;
{
    if (Verbose <= 0)
        return;
    fprintf(stderr, "Retry %d: ", errors);
    fprintf(stderr, s, p, u);
    fprintf(stderr, "\n");
}

#include "zm.c"
#include "zmr.c"

/*
 * Ack a ZFIN packet, let byegones be byegones
 */
void
ackbibi()
{
    register n;

    //vfile("ackbibi:");
    Readnum = 1;
    stohdr(0L);
    for (n=3; --n>=0; )
    {
        purgeline();
        zshhdr(4,ZFIN, Txhdr);
        switch (readline(100))
        {
        case 'O':
            readline(1);	/* Discard 2nd 'O' */
            //vfile("ackbibi complete");
            return;
        case RCDO:
            return;
        case TIMEOUT:
        default:
            break;
        }
    }
}

int tryzhdrtype=ZRINIT;	/* Header type to send corresponding to Last rx close */
openit(name, openmode)
char *name, *openmode;
{
    Uart_Printf("ooooooooooopenit Pathname=%s, name=%s\n", Pathname, name);
    if (strcmp(name, "-"))
        fout = fopen(name, openmode);
    else if (isatty(1))
        fout = fopen("stdout", "a");
    else
        fout = stdout;
}

#ifdef MD
/*
 *  Directory-creating routines from Public Domain TAR by John Gilmore
 */

/*
 * After a file/link/symlink/dir creation has failed, see if
 * it's because some required directory was not present, and if
 * so, create all required dirs.
 */
make_dirs(pathname)
register char *pathname;
{
    register char *p;		/* Points into path */
    int madeone = 0;		/* Did we do anything yet? */
    int save_errno = errno;		/* Remember caller's errno */

    if (errno != ENOENT)
        return 0;		/* Not our problem */

    for (p = strchr(pathname, '/'); p != NULL; p = strchr(p+1, '/'))
    {
        /* Avoid mkdir of empty string, if leading or double '/' */
        if (p == pathname || p[-1] == '/')
            continue;
        /* Avoid mkdir where last part of path is '.' */
        if (p[-1] == '.' && (p == pathname+1 || p[-2] == '/'))
            continue;
        *p = 0;				/* Truncate the path there */
        if ( !mkdir(pathname, 0777))  	/* Try to create it as a dir */
        {
            //vfile("Made directory %s\n", pathname);
            madeone++;		/* Remember if we made one */
            *p = '/';
            continue;
        }
        *p = '/';
        if (errno == EEXIST)		/* Directory already exists */
            continue;
        /*
         * Some other error in the mkdir.  We return to the caller.
         */
        break;
    }
    errno = save_errno;		/* Restore caller's errno */
    return madeone;			/* Tell them to retry if we made one */
}

#if (MD != 2)
#define	TERM_SIGNAL(status)	((status) & 0x7F)
#define TERM_COREDUMP(status)	(((status) & 0x80) != 0)
#define TERM_VALUE(status)	((status) >> 8)
/*
 * Make a directory.  Compatible with the mkdir() system call on 4.2BSD.
 */
mkdir(dpath, dmode)
char *dpath;
int dmode;
{
    int cpid, status;
    struct stat statbuf;

    if (stat(dpath,&statbuf) == 0)
    {
        errno = EEXIST;		/* Stat worked, so it already exists */
        return -1;
    }

    /* If stat fails for a reason other than non-existence, return error */
    if (errno != ENOENT) return -1;

    switch (cpid = fork())
    {

    case -1:			/* Error in fork() */
        return(-1);		/* Errno is set already */

    case 0:				/* Child process */
        /*
         * Cheap hack to set mode of new directory.  Since this
         * child process is going away anyway, we zap its umask.
         * FIXME, this won't suffice to set SUID, SGID, etc. on this
         * directory.  Does anybody care?
         */
        status = umask(0);	/* Get current umask */
        status = umask(status | (0777 & ~dmode)); /* Set for mkdir */
        execl("/bin/mkdir", "mkdir", dpath, (char *)0);
//        _exit(2);		/* Can't exec /bin/mkdir */
        Uart_Printf("Can't exec /bin/mkdir \n");
        return 2;

    default:			/* Parent process */
        while (cpid != wait(&status)) ;	/* Wait for kid to finish */
    }

    if (TERM_SIGNAL(status) != 0 || TERM_VALUE(status) != 0)
    {
        errno = EIO;		/* We don't know why, but */
        return -1;		/* /bin/mkdir failed */
    }

    return 0;
}
#endif /* MD != 2 */
#endif /* MD */

/*
 * Putsec writes the n characters of buf to receive file fout.
 *  If not in binary mode, carriage returns, and all characters
 *  starting with CPMEOF are discarded.
 */
putsec(buf, n)
char *buf;
register n;
{
    register char *p;

    if (n == 0)
        return OK;
    if (Thisbinary)
    {
        for (p=buf; --n>=0; )
            putc( *p++, fout);
    }
    else
    {
        if (Eofseen)
            return OK;
        for (p=buf; --n>=0; ++p )
        {
            if ( *p == '\r')
                continue;
            if (*p == CPMEOF)
            {
                Eofseen=TRUE;
                return OK;
            }
            putc(*p ,fout);
        }
    }
    return OK;
}

/*
 * substr(string, token) searches for token in string s
 * returns pointer to token within string if found, NULL otherwise
 */
char *
substr(s, t)
register char *s,*t;
{
    register char *ss,*tt;
    /* search for first char of token */
    for (ss=s; *s; s++)
        if (*s == *t)
            /* compare token with substring */
            for (ss=s,tt=t; ;)
            {
                if (*tt == 0)
                    return s;
                if (*ss++ != *tt++)
                    break;
            }
    return NULL;
}


/*
 * If called as rb use YMODEM protocol, etc.
 */
chkinvok(s)
char *s;
{
    register char *p;

    p = s;
    while (*p == '-')
        s = ++p;
    while (*p)
        if (*p++ == '/')
            s = p;
    if (*s == 'v')
    {
        Verbose=1;
        ++s;
    }
    Progname = s;
    if (s[0]=='r' && s[1]=='z')
        Batch = TRUE;
    if (s[0]=='r' && s[1]=='c')
        Crcflg = TRUE;
    if (s[0]=='r' && s[1]=='b')
        Batch = Nozmodem = TRUE;
}

/*
 * Totalitarian Communist pathname processing
 */
checkpath(name)
char *name;
{
    if (Restricted)
    {
        if (fopen(name, "r") != NULL)
        {
            canit();
            fprintf(stderr, "\r\nrz: %s exists\n", name);
            bibi(-1);
        }
        /* restrict pathnames to current tree or uucppublic */
        if ( substr(name, "../")
                || (name[0]== '/' && strncmp(name, PUBDIR, strlen(PUBDIR))) )
        {
            canit();
            fprintf(stderr,"\r\nrz:\tSecurity Violation\r\n");
            bibi(-1);
        }
    }
}

/*
 * Initialize for Zmodem receive attempt, try to activate Zmodem sender
 *  Handles ZSINIT frame
 *  Return ZFILE if Zmodem filename received, -1 on error,
 *   ZCOMPL if transaction finished,  else 0
 */
tryz()
{
    register c, n;
    register cmdzack1flg;
    Uart_Printf("tryz1\n");
    if (Nozmodem)		/* Check for "rb" program name */
        return 0;
    Uart_Printf("tryz2\n");

    for (n=15; --n>=0; )
    {
        /* Set buffer length (0) and capability flags */
#ifdef SEGMENTS
        stohdr(SEGMENTS*1024L);
#else
        stohdr(0L);
#endif
#ifdef CANBREAK
        Txhdr[ZF0] = CANFC32|CANFDX|CANOVIO|CANBRK;
#else
        Txhdr[ZF0] = CANFC32|CANFDX|CANOVIO;
#endif
        if (Zctlesc)
            Txhdr[ZF0] |= TESCCTL;
        Txhdr[ZF0] |= CANRLE;
        Txhdr[ZF1] = CANVHDR;
        Uart_Printf("tryz 3\n");
        /* tryzhdrtype may == ZRINIT */
        zshhdr(4,tryzhdrtype, Txhdr);
        //while(1)
        //	Uart_Printf("tryz 4\n\n\n");
        if (tryzhdrtype == ZSKIP)	/* Don't skip too far */
            tryzhdrtype = ZRINIT;	/* CAF 8-21-87 */
again:
        switch (zgethdr(Rxhdr))
        {
            Uart_Printf("tryz 4, zgethdr(Rxhdr)=%d\n\r", zgethdr(Rxhdr));
        case ZRQINIT:
            if (Rxhdr[ZF3] & 0x80)
                Usevhdrs = 1;	/* we can var header */
            continue;
        case ZEOF:
            continue;
        case TIMEOUT:
            continue;
        case ZFILE:
            zconv = Rxhdr[ZF0];
            zmanag = Rxhdr[ZF1];
            ztrans = Rxhdr[ZF2];
            if (Rxhdr[ZF3] & ZCANVHDR)
                Usevhdrs = TRUE;
            tryzhdrtype = ZRINIT;
            c = zrdata(secbuf, 1024);
            if (c == GOTCRCW)
                return ZFILE;
            zshhdr(4,ZNAK, Txhdr);
            goto again;
        case ZSINIT:
            Zctlesc = TESCCTL & Rxhdr[ZF0];
            if (zrdata(Attn, ZATTNLEN) == GOTCRCW)
            {
                stohdr(1L);
                zshhdr(4,ZACK, Txhdr);
                goto again;
            }
            zshhdr(4,ZNAK, Txhdr);
            goto again;
        case ZFREECNT:
            stohdr(getfree());
            zshhdr(4,ZACK, Txhdr);
            goto again;
        case ZCOMMAND:
#ifdef REGISTERED
            /* Enabling this code without registering is theft */
            if (Restricted)
            {
                sprintf(endmsg, "ZCOMMAND Restricted.");
                return ERROR;
            }
#endif
            cmdzack1flg = Rxhdr[ZF0];
            if (zrdata(secbuf, 1024) == GOTCRCW)
            {
                void exec2();

                if (cmdzack1flg & ZCACK1)
                    stohdr(0L);
                else
                    stohdr((long)sys2(secbuf));
                purgeline();	/* dump impatient questions */
                do
                {
                    zshhdr(4,ZCOMPL, Txhdr);
                }
                while (++errors<20 && zgethdr(Rxhdr) != ZFIN);
                ackbibi();
                if (cmdzack1flg & ZCACK1)
                    exec2(secbuf);
                return ZCOMPL;
            }
            zshhdr(4,ZNAK, Txhdr);
            goto again;
        case ZCOMPL:
            goto again;
        default:
            continue;
        case ZFIN:
            ackbibi();
            return ZCOMPL;
        case ZCAN:
            return ERROR;
        }
    }
    return 0;
}

/*
 * Process incoming file information header
 *  Returns 0 for success, other codes for errors
 *  or skip conditions.
 */
procheader(name)
char *name;
{
    register char *openmode, *p;
    static dummy;
    struct stat f;

    /* set default parameters and overrides */
    openmode = "w";
    Thisbinary = (!Rxascii) || Rxbinary;
    if (zconv == ZCBIN && Lzconv != ZCRESUM)
        Lzconv = zconv;			/* Remote Binary override */
    if (Lzconv)
        zconv = Lzconv;
    if (Lzmanag)
        zmanag = Lzmanag;

    /*
     *  Process ZMODEM remote file management requests
     */
    if (!Rxbinary && zconv == ZCNL)	/* Remote ASCII override */
        Thisbinary = 0;
    if (zconv == ZCBIN)	/* Remote Binary override */
        Thisbinary = TRUE;
    else if (zmanag == ZMAPND)
        openmode = "a";

    Bytesleft = DEFBYTL;
    Filemode = 0;
    Modtime = 0L;

    if (!name || !*name)
        return 0;

    p = name + 1 + strlen(name);
    if (*p)  	/* file coming from Unix or DOS system */
    {
        sscanf(p, "%ld%lo%o%lo%d%ld%d%d",
               &Bytesleft, &Modtime, &Filemode,
               &dummy, &Filesleft, &Totalleft, &dummy, &dummy);
        if (Filemode & UNIXFILE)
            ++Thisbinary;
        ++Totfiles;
        Totbytes += Bytesleft;
        if (Verbose)
        {
            fprintf(stderr,  "Incoming: %s %ld %lo %o\n",
                    name, Bytesleft, Modtime, Filemode);
            fprintf(stderr,  "YMODEM header: %s\n", p);
        }
    }


    else  		/* File coming from CP/M system */
    {
        for (p=name; *p; ++p)		/* change / to _ */
            if ( *p == '/')
                *p = '_';

        if ( *--p == '.')		/* zap trailing period */
            *p = 0;
    }
#if 0//zy add path prefix
    strcpy(Pathname, name);
    checkpath(name);
#else

    strcpy(Pathname, "/mnt/pos/");
    strcat(Pathname, name);
    checkpath(name);
#endif
    Uart_Printf("%s %s\n", __func__, Pathname);
    if (*name && stat(name, &f)!= -1)
    {
        zmanag &= ZMMASK;
        if (zmanag==ZMPROT)
            goto skipfile;
        //vfile("Current %s is %ld %lo", name, f.st_size, f.st_mtime);
        if (Thisbinary && zconv==ZCRESUM)
        {
            rxbytes = f.st_size & ~511;
            if (Bytesleft < rxbytes)
            {
                rxbytes = 0;
                goto doopen;
            }
            else
                openit(name, "r+");
            if ( !fout)
                return ZFERR;
            if (fseek(fout, rxbytes, 0))
            {
                closeit();
                return ZFERR;
            }
            //vfile("Crash recovery at %ld", rxbytes);
            return 0;
        }
        switch (zmanag & ZMMASK)
        {
        case ZMNEWL:
            if (Bytesleft > f.st_size)
                goto doopen;
        case ZMNEW:
            if ((f.st_mtime+1) >= Modtime)
                goto skipfile;
            goto doopen;
        case ZMCLOB:
        case ZMAPND:
            goto doopen;
        default:
            goto skipfile;
        }
    }
    else if (zmanag & ZMSKNOLOC)
    {
skipfile:
        //vfile("Skipping %s", name);
        return ZSKIP;
    }
doopen:
    openit(name, openmode);
#ifdef MD
    if ( !fout)
        if (make_dirs(name))
            openit(name, openmode);
#endif
    if ( !fout)
        return ZFERR;
    return 0;
}

/*
 * Receive 1 or more files with ZMODEM protocol
 */
rzfiles()
{
    register c;

    for (;;)
    {
        switch (c = rzfile())
        {
        case ZEOF:
        case ZSKIP:
        case ZFERR:
            switch (tryz())
            {
            case ZCOMPL:
                return OK;
            default:
                return ERROR;
            case ZFILE:
                break;
            }
            continue;
        default:
            return c;
        case ERROR:
            return ERROR;
        }
    }
    /* NOTREACHED */
}

/*
 * Receive a file with ZMODEM protocol
 *  Assumes file name frame is in secbuf
 */
rzfile()
{
    register c, n;

    Eofseen=FALSE;
    n = 20;
    rxbytes = 0l;
    Uart_Printf("rzfile Pathname=%s\n", Pathname);
    if (c = procheader(secbuf))
    {
        return (tryzhdrtype = c);
    }

    for (;;)
    {
#ifdef SEGMENTS
        chinseg = 0;
#endif
        stohdr(rxbytes);
        zshhdr(4,ZRPOS, Txhdr);
nxthdr:
        switch (c = zgethdr(Rxhdr))
        {
        default:
            //vfile("rzfile: Wrong header %d", c);
            if ( --n < 0)
            {
                sprintf(endmsg, "rzfile: Wrong header %d", c);
                return ERROR;
            }
            continue;
        case ZCAN:
            sprintf(endmsg, "Sender CANcelled");
            return ERROR;
        case ZNAK:
#ifdef SEGMENTS
            putsec(secbuf, chinseg);
            chinseg = 0;
#endif
            if ( --n < 0)
            {
                sprintf(endmsg, "rzfile: got ZNAK", c);
                return ERROR;
            }
            continue;
        case TIMEOUT:
#ifdef SEGMENTS
            putsec(secbuf, chinseg);
            chinseg = 0;
#endif
            if ( --n < 0)
            {
                sprintf(endmsg, "rzfile: TIMEOUT", c);
                return ERROR;
            }
            continue;
        case ZFILE:
            zrdata(secbuf, 1024);
            continue;
        case ZEOF:
#ifdef SEGMENTS
            putsec(secbuf, chinseg);
            chinseg = 0;
#endif
            if (rclhdr(Rxhdr) != rxbytes)
            {
                /*
                 * Ignore eof if it's at wrong place - force
                 *  a timeout because the eof might have gone
                 *  out before we sent our zrpos.
                 */
                errors = 0;
                goto nxthdr;
            }
            if (closeit())
            {
                tryzhdrtype = ZFERR;
                //vfile("rzfile: closeit returned <> 0");
                sprintf(endmsg,"Error closing file");
                return ERROR;
            }
            //vfile("rzfile: normal EOF");
            return c;
        case ERROR:	/* Too much garbage in header search error */
#ifdef SEGMENTS
            putsec(secbuf, chinseg);
            chinseg = 0;
#endif
            if ( --n < 0)
            {
                sprintf(endmsg, "Persistent CRC or other ERROR");
                return ERROR;
            }
            zmputs(Attn);
            continue;
        case ZSKIP:
#ifdef SEGMENTS
            putsec(secbuf, chinseg);
            chinseg = 0;
#endif
            Modtime = 1;
            closeit();
            sprintf(endmsg, "Sender SKIPPED file");
            return c;
        case ZDATA:
            if (rclhdr(Rxhdr) != rxbytes)
            {
                if ( --n < 0)
                {
                    sprintf(endmsg,"Data has bad addr");
                    return ERROR;
                }
#ifdef SEGMENTS
                putsec(secbuf, chinseg);
                chinseg = 0;
#endif
                zmputs(Attn);
                continue;
            }
moredata:
            if (Verbose>1)
                fprintf(stderr, "%7ld ZMODEM%s\n",
                        rxbytes, Crc32r?" CRC-32":"");
#ifdef SEGMENTS
            if (chinseg >= (1024 * SEGMENTS))
            {
                putsec(secbuf, chinseg);
                chinseg = 0;
            }
            switch (c = zrdata(secbuf+chinseg, 1024))
#else
            switch (c = zrdata(secbuf, 1024))
#endif
            {
            case ZCAN:
#ifdef SEGMENTS
                putsec(secbuf, chinseg);
                chinseg = 0;
#endif
                sprintf(endmsg, "Sender CANcelled");
                return ERROR;
            case ERROR:	/* CRC error */
#ifdef SEGMENTS
                putsec(secbuf, chinseg);
                chinseg = 0;
#endif
                if ( --n < 0)
                {
                    sprintf(endmsg, "Persistent CRC or other ERROR");
                    return ERROR;
                }
                zmputs(Attn);
                continue;
            case TIMEOUT:
#ifdef SEGMENTS
                putsec(secbuf, chinseg);
                chinseg = 0;
#endif
                if ( --n < 0)
                {
                    sprintf(endmsg, "TIMEOUT");
                    return ERROR;
                }
                continue;
            case GOTCRCW:
                n = 20;
#ifdef SEGMENTS
                chinseg += Rxcount;
                putsec(secbuf, chinseg);
                chinseg = 0;
#else
                putsec(secbuf, Rxcount);
#endif
                rxbytes += Rxcount;
                stohdr(rxbytes);
                sendline(XON);
                zshhdr(4,ZACK, Txhdr);
                goto nxthdr;
            case GOTCRCQ:
                n = 20;
#ifdef SEGMENTS
                chinseg += Rxcount;
#else
                putsec(secbuf, Rxcount);
#endif
                rxbytes += Rxcount;
                stohdr(rxbytes);
                zshhdr(4,ZACK, Txhdr);
                goto moredata;
            case GOTCRCG:
                n = 20;
#ifdef SEGMENTS
                chinseg += Rxcount;
#else
                putsec(secbuf, Rxcount);
#endif
                rxbytes += Rxcount;
                goto moredata;
            case GOTCRCE:
                n = 20;
#ifdef SEGMENTS
                chinseg += Rxcount;
#else
                putsec(secbuf, Rxcount);
#endif
                rxbytes += Rxcount;
                goto nxthdr;
            }
        }
    }
}


/*
 * Close the receive dataset, return OK or ERROR
 */
closeit()
{
    time_t time();

    if (fout == stdout)
    {
        fflush(stdout);
        fout = 0;
        return OK;
    }
    if (fclose(fout)==ERROR)
    {
        fprintf(stderr, "File close ERROR\n");
        return ERROR;
    }
    if (Modtime)
    {
        timep[0] = time(NULL);
        timep[1] = Modtime;
        utime(Pathname, timep);
    }
    Uart_Printf("%s %s\n", __FUNCTION__, Pathname);

    return OK;
}
/*
 * Fetch a pathname from the other end as a C ctyle ASCIZ string.
 * Length is indeterminate as long as less than Blklen
 * A null string represents no more files (YMODEM)
 */
wcrxpn(rpn)
char *rpn;	/* receive a pathname */
{
    register c;

    purgeline();

et_tu:
    Firstsec=TRUE;
    Eofseen=FALSE;
    sendline(Crcflg?WANTCRC:NAK);
    flushmo();
    Lleft=0;	/* Do read next time ... */
    switch (c = wcgetsec(rpn, 100))
    {
    case WCEOT:
        zperr2( "Pathname fetch returned %d", c);
        sendline(ACK);
        flushmo();
        Lleft=0;	/* Do read next time ... */
        readline(1);
        goto et_tu;
    case 0:
        sendline(ACK);
        flushmo();
        return OK;
    default:
        return ERROR;
    }
}

/*
 * Adapted from CMODEM13.C, written by
 * Jack M. Wierda and Roderick W. Hart
 */

wcrx()
{
    register int sectnum, sectcurr;
    register char sendchar;
    int cblklen;			/* bytes to dump this block */

    Firstsec=TRUE;
    sectnum=0;
    Eofseen=FALSE;
    sendchar=Crcflg?WANTCRC:NAK;

    for (;;)
    {
        sendline(sendchar);	/* send it now, we're ready! */
        flushmo();
        Lleft=0;	/* Do read next time ... */
        sectcurr=wcgetsec(secbuf, (sectnum&0177)?50:130);
        if (sectcurr==(sectnum+1 &0377))
        {
            sectnum++;
            cblklen = Bytesleft>Blklen ? Blklen:Bytesleft;
            if (putsec(secbuf, cblklen)==ERROR)
                return ERROR;
            Totbytes += cblklen;
            if ((Bytesleft-=cblklen) < 0)
                Bytesleft = 0;
            sendchar=ACK;
        }
        else if (sectcurr==(sectnum&0377))
        {
            zperr1( "Received dup Sector");
            sendchar=ACK;
        }
        else if (sectcurr==WCEOT)
        {
            if (closeit())
                return ERROR;
            sendline(ACK);
            flushmo();
            Lleft=0;	/* Do read next time ... */
            ++Totfiles;
            return OK;
        }
        else if (sectcurr==ERROR)
            return ERROR;
        else
        {
            zperr1( "Sync Error");
            return ERROR;
        }
    }
    /* NOTREACHED */
}
/*
 * Wcgetsec fetches a Ward Christensen type sector.
 * Returns sector number encountered or ERROR if valid sector not received,
 * or CAN CAN received
 * or WCEOT if eot sector
 * time is timeout for first char, set to 4 seconds thereafter
 ***************** NO ACK IS SENT IF SECTOR IS RECEIVED OK **************
 *    (Caller must do that when he is good and ready to get next sector)
 */

wcgetsec(rxbuf, maxtime)
char *rxbuf;
int maxtime;
{
    register checksum, wcj, firstch;
    register unsigned short oldcrc;
    register char *p;
    int sectcurr;

    for (Lastrx=errors=0; errors<RETRYMAX; errors++)
    {

        if ((firstch=readline(maxtime))==STX)
        {
            Blklen=1024;
            goto get2;
        }
        if (firstch==SOH)
        {
            Blklen=128;
get2:
            sectcurr=readline(1);
            if ((sectcurr+(oldcrc=readline(1)))==0377)
            {
                oldcrc=checksum=0;
                for (p=rxbuf,wcj=Blklen; --wcj>=0; )
                {
                    if ((firstch=readline(1)) < 0)
                        goto bilge;
                    oldcrc=updcrc(firstch, oldcrc);
                    checksum += (*p++ = firstch);
                }
                if ((firstch=readline(1)) < 0)
                    goto bilge;
                if (Crcflg)
                {
                    oldcrc=updcrc(firstch, oldcrc);
                    if ((firstch=readline(1)) < 0)
                        goto bilge;
                    oldcrc=updcrc(firstch, oldcrc);
                    if (oldcrc & 0xFFFF)
                        zperr1( "CRC");
                    else
                    {
                        Firstsec=FALSE;
                        return sectcurr;
                    }
                }
                else if (((checksum-firstch)&0377)==0)
                {
                    Firstsec=FALSE;
                    return sectcurr;
                }
                else
                    zperr1( "Checksum");
            }
            else
                zperr1("Sector number garbled");
        }
        /* make sure eot really is eot and not just mixmash */
        else if (firstch==EOT && Lleft==0)
            return WCEOT;
        else if (firstch==CAN)
        {
            if (Lastrx==CAN)
            {
                zperr1( "Sender CANcelled");
                return ERROR;
            }
            else
            {
                Lastrx=CAN;
                continue;
            }
        }
        else if (firstch==TIMEOUT)
        {
            if (Firstsec)
                goto humbug;
bilge:
            zperr1( "TIMEOUT");
        }
        else
            zperr2( "Got 0%o sector header", firstch);

humbug:
        Lastrx=0;
        while (readline(1)!=TIMEOUT)
            ;
        if (Firstsec)
        {
            sendline(Crcflg?WANTCRC:NAK);
            flushmo();
            Lleft=0;	/* Do read next time ... */
        }
        else
        {
            maxtime=40;
            sendline(NAK);
            flushmo();
            Lleft=0;	/* Do read next time ... */
        }
    }
    /* try to stop the bubble machine. */
    canit();
    return ERROR;
}

char *rbmsg = "%s ready. Type \"%s file ...\" to your modem program\n\r";
wcreceive(argc, argp)
char **argp;
{
    register c;
    Uart_Printf("%s Batch=%d argc=%d\n", __func__,Batch, argc);
    if (Batch || argc==0)
    {
        Crcflg=1;
        //		fprintf(stderr, rbmsg, Progname, Nozmodem?"sb":"sz");
        Uart_Printf("zzzzzzzzzzzzz0 ready to receive with zzzzzz zmodem, c=%x\n", c);
        c=tryz();
        Uart_Printf("zzzzzzzzzzzzz1 ready to receive with zzzzzz zmodem, c=%x\n", c);
        if (c)
        {
            if (c == ZCOMPL)
                return OK;
            if (c == ERROR)
                goto fubar;
            c = rzfiles();
            if (c)
                goto fubar;
        }
        else
        {
            for (;;)
            {
                if (wcrxpn(secbuf)== ERROR)
                    goto fubar;
                if (secbuf[0]==0)
                    return OK;
                if (procheader(secbuf))
                    goto fubar;
                if (wcrx()==ERROR)
                    goto fubar;
            }
        }
    }
    else
    {
        Bytesleft = DEFBYTL;
        Filemode = 0;
        Modtime = 0L;

        procheader("");
#if 1	//zy add path prefix
        strcpy(Pathname, "/mnt/pos/");
        strcat(Pathname, *argp);
#else
        strcpy(Pathname, *argp);
#endif
        checkpath(Pathname);
        Uart_Printf("\nrz: ready to receive wwwwwwwwwwwwwww %s\r\n", Pathname);
        if ((fout=fopen(Pathname, "w")) == NULL)
            return ERROR;
        if (wcrx()==ERROR)
            goto fubar;
    }
    return OK;
fubar:
    canit();
    Modtime = 1;
    if (fout)
        fclose(fout);
    if (Restricted)
    {
        unlink(Pathname);
        fprintf(stderr, "\r\nrz: %s removed.\r\n", Pathname);
    }
    return ERROR;
}

/* called by signal interrupt or terminate to clean things up */
void
bibi(int n)
{
    if (Zmodem)
        zmputs(Attn);
    canit();
    mode(0);
    fprintf(stderr, "rz: caught signal %d; exiting", n);
#if 0
    exit(3);
#else
    return;
#endif
}

/* for APP call, zy*/
int simple_rz(char argc, char *argv[])
{
    register char *cp;
    register npats;
    char *virgin, **patts;
    int exitcode = 0;

    Rxtimeout = 100;
    setbuf(stderr, NULL);

    Restricted=TRUE;
#if 0
    chkinvok(virgin=argv[0]);
#else
    Batch = TRUE;
#endif

    Tty = 0;
    Ttystream = stdin;
    npats = 0;

    mode(1);

    if (wcreceive(npats, patts)==ERROR)
    {
        exitcode=1;
        canit();
    }
    if (exitcode && !Zmodem)	/* bellow again with all thy might. */
        canit();
    if (endmsg[0])
        fprintf(stderr, "  %s: %s\r\n", Progname, endmsg);
    fprintf(stderr, "%s %s finished.\r\n", Progname, VERSION);
    fflush(stderr);
    if (exitcode)
    {
        mode(0);
        return 1;
    }


    mode(0);
    if (exitcode)
        return 1;

    return 0;
    /* NOTREACHED */
}

/*
 * Strip leading ! if present, do shell escape.
 */
sys2(s)
register char *s;
{
    if (*s == '!')
        ++s;
    return system(s);
}
/*
 * Strip leading ! if present, do exec.
 */
void
exec2(s)
register char *s;
{
    if (*s == '!')
        ++s;
    mode(0);
    execl("/bin/sh", "sh", "-c", s);
}

/* End of rz.c */

