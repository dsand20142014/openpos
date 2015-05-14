#ifndef __SAND_GPRS__
#define __SAND_GPRS__
#include <appdef.h>

unsigned char OSGSM_GprsDial(void);
unsigned char OSGSM_GprsCheckDial(unsigned short uiTimeout);
unsigned char OSGSM_GprsHangupDial(void);
unsigned char OSGSM_GprsReset(void);
unsigned char OSGSM_GprsSetAPN(unsigned char* pucPtr);
unsigned char OSGSM_GprsGetCCID(unsigned char* pucCCID,unsigned char* pucCCIDLen);
unsigned char OSGSM_GprsSetDialParam(WIRELESS_PARAM wl);
unsigned char OSGSM_GprsPowerOn(void);
unsigned char OSGSM_GprsPowerOff(void);
unsigned char OSGSM_HW_GprsReset(void);
unsigned char OSSGSM_GprsSmartDial(void);

void set_ppp_restart(unsigned char flag);


#endif
