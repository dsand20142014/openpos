#ifndef __NEW_DRV_H_
#define __NEW_DRV_H_


#include <signal.h>
#include <sys/mman.h>
#include <pthread.h>

#include "osdriver.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  KEY_SIG  (SIGIO)
#define  MAG_SIG  (SIGRTMIN+6)
#define  MFR_SIG  (SIGRTMIN+7)
#define  ICC_SIG  (SIGRTMIN+8)
#define  APP_KEY_SIG  (SIGRTMIN+9)

#define DRV_TIMEOUT     (1<<0)
#define KEY_DRV         (1<<1)
#define MAG_DRV         (1<<2)
#define MFR_DRV         (1<<3)
#define ICC_DRV         (1<<4)


#define KEY_DRV_ERROR         (1<<1)
#define MAG_DRV_ERROR         (1<<2)
#define MFR_DRV_ERROR         (1<<3)
#define ICC_DRV_ERROR         (1<<4)
#define DRV_ID_ERROR          (1<<5)
#define MULT_CALL_ERROR       (1<<6)
#define  SECURITY_SIG   (SIGRTMIN+10)

typedef struct
{
    unsigned int drv_type;
    DRV_OUT drv_data;
} NEW_DRV_TYPE;  


extern unsigned int Os_Wait_Event(unsigned int drv_id, NEW_DRV_TYPE *new_drv, unsigned int timeout);


#ifdef __cplusplus
}
#endif

#endif

