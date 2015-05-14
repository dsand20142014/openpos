#ifndef __OS_DOWN_H
#define __OS_DOWN_H

#define OSDWN_TTY_OPEN  1
#define OSDWN_TTY_CLOSE 2

char *g_tty_name;
int  g_tty_fd;

/* define error */
#define DWN_DOWN_SUCCESS     0
#define DWN_DOWN_TIMEOUT   (-1)
#define DWN_DOWN_BADSIZE   (-2)
#define DWN_DOWN_CHECKSUM  (-3)
#define DWN_DOWN_ABORT     (-4)
#define DWN_DOWN_NOTAPP    (-5)
#define DWN_DOWN_BADAPPCRC (-6)
#define DWN_DOWN_BOTAPP    (-7)
#define DWN_DOWN_SAVE_FAIL (-8)
#define DWN_DOWN_READ_ERR  (-9)
#define DWN_DOWN_SEND_ERR  (-10)

//int OSDWN_download(unsigned int maxbytes,unsigned int *nbytes);
int OSDWN_download(unsigned char **pmalloc, unsigned int maxbytes, unsigned int *nbytes);
int OSDWN_respone(unsigned char *pindata,unsigned short uidatalen);
void OSDWN_tty_init(unsigned char ucport);
void OSDWN_tty_mode(int mode);
void OSDWN_tty_open(unsigned char ucport);
void OSDWN_tty_close(void);
//void OSDWN_timeout(int signum, siginfo_t *info,void *myact);
void OSDWN_err_msg(int etype);
#endif

