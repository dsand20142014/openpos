#ifndef __SAND_KEY_H__
#define __SAND_KEY_H__


#ifndef __LUXAPP_KBD__
#define KBD_EXTERN extern
#else
#define KBD_EXTERN
#endif

#include "osdriver.h"


#define KEY_REP_OK 0
#define KEY_REP_ERR 1

KBD_EXTERN unsigned char  Os__get_cam_key();
KBD_EXTERN unsigned char scancode_to_asc(unsigned char in);
KBD_EXTERN unsigned char Os__xget_key ();
//KBD_EXTERN unsigned char Os_open_key_fd(void);
//KBD_EXTERN unsigned char Os__get_key_status();//
//KBD_EXTERN void Os__stop_key();
KBD_EXTERN DRV_OUT * OSMMI_GetKey();
KBD_EXTERN DRV_OUT * Os__get_key();
KBD_EXTERN unsigned char Os__read_eeprom(unsigned short uiStartAddr, unsigned char ucLen, unsigned char *pucPtr);
KBD_EXTERN void Os__crypt(unsigned char *pucCryptKey);
KBD_EXTERN DRV_OUT * Os__get_key_pp(void);
KBD_EXTERN unsigned char Os__xget_key_pp(void);

KBD_EXTERN unsigned char *Os__xget_pin(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB);
KBD_EXTERN DRV_OUT *Os__get_fixkey(unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB);
KBD_EXTERN DRV_OUT *Os__get_pin(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB);
//end

/*
* 返回0表示成功，返回-1表示失败
*/
int Os__set_key(unsigned char key);
int Os__set_key_ensure(unsigned char key);

#endif
