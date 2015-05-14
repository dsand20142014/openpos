/*
*osuart.h
*/
#ifndef __OSUART_H_
#define __OSUART_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OSUART_TTY_OPEN  1
#define OSUART_TTY_CLOSE 2

char *g_tty_name;
int  g_tty_fd;

int  OSUART_tty_init(unsigned char ucport);
void OSUART_tty_mode(int fd, int mode);
int OSUART_tty_open(unsigned char ucport);
void OSUART_tty_close(int fd);
int OSUART_tty_SendByte(int fd, unsigned char	*pucData, unsigned int uiLen);
int OSUART_tty_RecvByte(int fd, unsigned char *pucData, unsigned int uiLen);

#ifdef __cplusplus
}
#endif

#endif
