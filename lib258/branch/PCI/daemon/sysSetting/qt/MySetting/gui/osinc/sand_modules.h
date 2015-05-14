#ifndef __LUXAPP_MODULES__
#define MODULES_EXTERN extern
#else
#define MODULES_EXTERN
#endif

#include "osdriver.h"


MODULES_EXTERN	void Os__abort_drv (unsigned char IdDrv);
MODULES_EXTERN	void Os__call_drv(DRIVER *pDrv);
MODULES_EXTERN	unsigned char Os__wait_drv(DRV_OUT *pDrvOut);
