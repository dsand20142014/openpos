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

unsigned char Os__init_com3(unsigned short uiParam1,unsigned short uiParam2,unsigned char ucParam3);
void Os__txcar3(unsigned char ucCh);
unsigned short Os__rxcar3(unsigned short uiTimeout);
