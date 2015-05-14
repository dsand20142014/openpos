
#ifndef _SS205_API_H
#define _SS205_API_H
#include "haldrv.h"
#include "ss205new.h"
#include "osdriver.h"

#define SIGN_MAXBMP 40560			//bmp文件最大字节数 20k

int Os__sign(char * bmpname);
int Os__sign_xprint(unsigned char *ucPath);
int Os__sign_print(void);
int Os__prn_forward(unsigned char);
int Os__prn_xforward(unsigned char);
int Os__prn_backward (unsigned char);
int OSPRN_XReverseFeed(unsigned char);
int Os__fontmode (fmode_t fmode); /*new*/
int Os__linefeed(unsigned int);
int Os__xlinefeed(unsigned int);
//int Os__print(int fd,unsigned char *data);
//int Os__print(unsigned char *data);
DRV_OUT *Os__print(unsigned char *data);
//int Os__print(int fd,unsigned char *data);
unsigned char Os__xprint(unsigned char *);
//unsigned char Os__GB2312_xprint(unsigned char *, unsigned char) ;

int OSPRN_XPrintGB2312(unsigned char ucASCIIFont, unsigned char ucGBFont, unsigned char * Text);
DRVOUT *OSPRN_PrintASCII(unsigned char *ucFont, unsigned char *Text);
//t Os__conv_ascii_GB2312 (unsigned char * ) ;
int Os__graph_xprint (unsigned char * , unsigned char ) ;
int  gb2312_16x16(unsigned char *,unsigned char *);
int ascii8x16(unsigned char *ch, unsigned char *dotdat);
int Os__checkpaper();
int OSCFG_PrinterSettingLoad(OSPRNCFG *);
int Os__printer_idle();
#ifdef QC430_v01
int Os__papercut(void);
#endif

#endif  //__SS205_API_H
