/*
	Operating System
--------------------------------------------------------------------------
	FILE  osmodem.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2002-10-03		Xiaoxi Jiang
    Last modified :	2002-10-03		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __OSMODEM_H
#define __OSMODEM_H

#define modem           0
#define rs232           1
#define HDLC            2
#define uart1           10


#define DIAL_OK             0x30        /* execution ok */
#define DIAL_INCOMING_CALL  0x32        /* call incoming */
#define DIAL_NO_CARRIER     0x33        /* carrier lost */
#define DIAL_COMMAND_ERR    0x34        /* command error */
#define DIAL_CONNECT        0x35        /* modem connection OK */
#define DIAL_NO_TONE        0x36        /* no ringing tone */
#define DIAL_PBMODEM        0x38        /* modem problem */
#define DIAL_BUSY           0x39        /* called phone line is already busy */
#define DIAL_MEM_FULL       0x3B        /* memory full */
#define DIAL_FORBIDDEN      0x3D        /* number dead */
#define DIAL_DELAYED        0x3E        /* call delayed */
#define DIAL_ABORT          0x3F        /* dialing aborted */
#define DIAL_LINE_BUSY      0x40        /* calling phone line is busy */
#define DIAL_NO_ANSWER      0x41        /* called phone line is no answer */
#define DIAL_NO_LINE        0x42        /* no phone line */
#define DIAL_NO_MODEM       0x43
#define DIAL_NO_NUMBER      0x44
#define DIAL_NOT_CONNECTED  0xFF        /* portable base not connected */
#define STILL_DIALING       0x3A        /* For background dialing : dialing not finished */

#define X28_OK              0           /* execution ok */
#define X28_HOST_ERR        1           /* connection site NOK */
#define X28_TO_ERR          2           /* character reception time out */
#define X28_CARRIER_LOST    3           /* carrier lost */
#define X28_NOK             4           /* execution error */
#define X28_LOGON_ERR       5           /* not an X25 network */
#define X28_PAD_ERR         6           /* received LIB DTE,ERROR,RESET */

#define COM_OK              0           /* execution OK */
#define COM_ERR_TO          1           /* character reception time out during OSUART_RxChar1 */
#define COM_CARRIER_LOST    2           /* carrier lost */
#define COM_NOK             3           /* for other execution error */
#define COM_PBPAR           4           /* wrong parameters for OSUART_Init1 */

#define OSMODEMERR_SUCCESS          0x00

#define OSMODEMDEBUG_DATASEND       0x01
#define OSMODEMDEBUG_DATARECV       0x02
#define OSMODEMDEBUG_CMDSEND        0x04
#define OSMODEMDEBUG_CMDRECV        0x08
#define OSMODEMDEBUG_INFO           0x20
#define OSMODEMDEBUG_ERROR          0x40
#define OSMODEMDEBUG_DISPMSG        0x80

#define OSMODEMMODEL_TDK73K         0x01
#define OSMODEMMODEL_SILICON        0x02
#define OSMODEMMODEL_CX86500        0x03
#define OSMODEMMODEL_CMX868         0x04
#define OSMODEMMODEL_IRMODEM        0x05

#define OSMODEM_MAXPHONELEN			50

#define OSMODEMPARAM                pModemCFG->aucParam

#define OSMODEMPARAM_COUNTRY        (OSMODEMPARAM[0])
#define OSMODEMPARAM_DIALTYPE       (OSMODEMPARAM[1])
#define OSMODEMPARAM_TONEDETECT     (OSMODEMPARAM[2])
#define OSMODEMPARAM_TONEDETECT1    (OSMODEMPARAM[2]&0x80)
#define OSMODEMPARAM_TONEDETECT2    (OSMODEMPARAM[2]&0x40)
#define OSMODEMPARAM_TONEDETECT3    (OSMODEMPARAM[2]&0x20)
#define OSMODEMPARAM_TONEDETECT4    (OSMODEMPARAM[2]&0x10)
#define OSMODEMPARAM_TONEDETECT5    (OSMODEMPARAM[2]&0x08)
#define OSMODEMPARAM_TONEDETECT6    (OSMODEMPARAM[2]&0x04)
#define OSMODEMPARAM_TONEDETECT7    (OSMODEMPARAM[2]&0x02)
#define OSMODEMPARAM_TONEDETECT8    (OSMODEMPARAM[2]&0x01)

#define OSMODEMPARAM_TIMEDIAL1      (OSMODEMPARAM[10])
#define OSMODEMPARAM_TIMEDIAL2      (OSMODEMPARAM[11])
#define OSMODEMPARAM_TIMEDIAL3      (OSMODEMPARAM[12])
#define OSMODEMPARAM_TIMEDIAL4      (OSMODEMPARAM[13])
#define OSMODEMPARAM_TIMEDIAL5      (OSMODEMPARAM[14])
#define OSMODEMPARAM_TIMEDIAL6      (OSMODEMPARAM[15])
#define OSMODEMPARAM_TIMEDIAL7      (OSMODEMPARAM[16])
#define OSMODEMPARAM_TIMEDIAL8      (OSMODEMPARAM[17])

#define OSMODEMPARAM_COUNTRYCHINA       0x26
#define OSMODEMPARAM_DIALTYPETONE       0x00
#define OSMODEMPARAM_DIALTYPEPULSE      0x01

#define OSMODEMPARAM_DISABLE            0x00
#define OSMODEMPARAM_ENABLE             0x01


/* downloading parameters  */
/* ---------------------- */

#define DOWNLOAD_STN      0             /* by phone access (Switching Telephone Network)*/
#define DOWNLOAD_X25      1             /* by X25 network */

#define AUTOMATIC_CALL  0               /* automatic call */
#define MANUAL_CALL     1               /* manual call */

struct  init_communication
{
    /* initialisation structure for the serial port */
    unsigned char stop_bits;        /* nbr of bit stop 1 or 2 */
    unsigned int  speed;            /* speed from 75 to 19200bds */
    unsigned char com_nr;           /* 0 : modem, 1 : rs232 */
    unsigned char data_bits;        /* nbr of data bits 7 or 8 */
    unsigned char parity;           /* parity E ou O ou N */
};
typedef struct init_communication COM_PARAM;

#if 0 //zy 0716
struct init_modem
{
    unsigned char dp;
    unsigned char chdt;
    unsigned char dt1;
    unsigned char dt2;
    unsigned char ht;
    unsigned char wt;
    unsigned char ssetup;
    unsigned char dtimes;
    unsigned char timeout;
    unsigned char asmode;
};
typedef struct init_modem	MODEM_PARAM;
#endif


struct  dialling
{
    /* numbering structure */
    unsigned char type[2];           /* '0':pulse(numeric),'1':tone(frequency) followed by 0 */
    unsigned char PABX[5];           /* 4 digits followed by 0 */
    unsigned char prefix[5];         /* 4 digits followed by 0 */
    unsigned char tone_detect[2];    /* '0': no ringing tone followed by 0 */
    unsigned char phone_nr[OSMODEM_MAXPHONELEN];      /* number to call 16 digits followed by 0 */
    unsigned char auto_man;          /* automatic or manual call */
    unsigned short crc;              /* crc of the structure */
};
typedef struct dialling DIAL_PARAM;

/* total receipt parameter  */
typedef struct
{
    unsigned char RFU[2];            /* reserved */
    unsigned char ticket_number;     /* sequential number of the partial total receipt*/
}typ_ticket_info;

struct sx28
{
    /* structure for Os__x28 connection */
    unsigned char set_para[26];         /* parameters setting : 25 characters followed by 0 */
    unsigned char x25_host_number[16];  /* number to call : 15 characters followed by 0 */
    unsigned char x25_password[10];     /* pass word and calling data : 9 characters followed by 0 */
};
typedef struct sx28 X28_PARAM;

struct sparax25
{
    /* structure for x25 downloading */
    X28_PARAM para_x28;                 /* X28 datas */
    unsigned char header[30];           /* header to send after the connection to the X25 server (usually retailer number) */
};
typedef struct sparax25 X25_CONNECTION;

union sconnection
{
    /* union for data connection */
    DIAL_PARAM      stn;
    X25_CONNECTION  x25;
};

struct sdownload
{
    /* structure for requesting a downloading */
    char appli_nr;                      /* application number */
    char nr_of_redial;                  /* nbr of connection retry, if = 0 : no downloading */
    char calling_type;                  /* calling type  / STN or X25 */
    char dial_time[4];                  /* calling time (HHMM in ascii) */
    char redial_delay[4];               /* waiting time between retry (HHMM in ascii) */
    union sconnection connection_param; /* connection data according the calling type */
};
typedef struct sdownload DOWNLOAD_PARAM;

typedef struct
{
    unsigned char (*pfnInit)(void);
    unsigned char (*pfnTest)(void);
    unsigned char (*pfnDialInit)(COM_PARAM *pComParam);
    unsigned char (*pfnDial)(DIAL_PARAM * pDialParam);
    unsigned char (*pfnBackDial)(DIAL_PARAM * pDialParam);
    unsigned char (*pfnCheckBackDial)(unsigned short uiTimeout);
    void (*pfnReadDialParam)(DIAL_PARAM *pDialParam);
    unsigned char (*pfnSetParam)(unsigned char ucParamNb, unsigned char ucValue);
    unsigned char (*pfnGetRegister)(unsigned short uiParamNb, unsigned char *pucValue);
    unsigned char (*pfnSetRegister)(unsigned short uiParamNb, unsigned char ucValue);
    unsigned char (*pfnTxChar)(unsigned char ucCh);
    unsigned short (*pfnRxChar)(unsigned short uiTimeout);
    unsigned char (*pfnHangup)(void);
    void  (*pfncallbackRecvPPPEvent)(void);
}MODEMFUNC;

#endif

