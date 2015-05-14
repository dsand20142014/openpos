#ifndef __DRVS_H_
#define __DRVS_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "fsync_drvs.h"


unsigned int wait_event_process(unsigned int driver, NEW_DRV_TYPE *new_drv, unsigned int timeout);

unsigned int wait_nokey_process(unsigned int driver, NEW_DRV_TYPE *new_drv, unsigned int timeout);

unsigned int rfid_ioctl_process(char *data);

unsigned int rfid_read_process(char *data);

unsigned int icc_ioctl_process(char *data);

unsigned int icc_read_process(char *data);



#ifdef __cplusplus
}
#endif

#endif

