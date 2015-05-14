/* skb25_communication.h */
#ifndef __SAND_COM__
#define COM_EXTERN extern
#else
#define COM_EXTERN
#endif

#include "osmodem.h"
extern int _g_com2_openned;
extern unsigned char Os__init_com(COM_PARAM *ComParameter);
extern unsigned char Os__txcar(unsigned char Charac);
extern unsigned short Os__rxcar(unsigned short TimeOut);
//extern int SerialPortInit(char *dev,int speed,int databits,int parity,int stopbits,int hwf,int swf, int block,struct termios *otty);

