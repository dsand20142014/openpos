/*
	Operating System
--------------------------------------------------------------------------
	FILE  osdrv.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-08-15		Xiaoxi Jiang
    Last modified :	2003-08-15		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __OSDRV_H__
#define __OSDRV_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OSDRVIOCTL_GET      0
#define OSDRVIOCTL_SET      1

typedef DRVIN DRV_IN;
typedef DRVOUT DRV_OUT;
typedef DRV DRIVER;

#ifdef __cplusplus
}
#endif

#endif