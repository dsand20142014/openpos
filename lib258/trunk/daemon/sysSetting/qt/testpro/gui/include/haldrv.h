/*
    Hardware Abstraction Layer
--------------------------------------------------------------------------
    FILE  haldrv.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :       2003-01-06      Xiaoxi Jiang
    Last modified : 2003-01-06      Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __HALDRV_H__
#define __HALDRV_H__

#ifndef HALDRVDATA
#define HALDRVDATA extern
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#include <osdrv.h>

#define HALDRVERR_SUCCESS           0x00
#define HALDRVERR_BADDRVID          0x01
#define HALDRVERR_BADCMD            0x02
#define HALDRVERR_NOTEXIST          0x03
#define HALDRVERR_NOHANDLE          0x04
#define HALDRVERR_NOTRESPONSE       0x05
#define HALDRVERR_INSTALLED         0x06
#define HALDRVERR_NOTINSTALL        0x07

#define HALDRVERR_CMDNOTSUPPORT     0xFE
#define HALDRVERR_PROCESSPENDING    0xFF

/* Must keep same as osdriver.h!!! */

#define HALDRVCMD_ABORT     0       /* abort driver     */
#define HALDRVCMD_EXECUTE   1       /* execute order    */
#define HALDRVCMD_STATEREQ  2       /* state request    */
#define HALDRVCMD_RESET     3       /* reset driver     */
#define HALDRVCMD_IOCTL    10      /* config driver    */

#define HALDRV_MAXNB        13      /* number of drivers */
#define HALDRVID_RESERVNB   13
#define HALDRVID_MMI        0       /* Man-Machine-Interface (Display-Keyboard) driver number */
#define HALDRVID_PRN        1       /* Printer driver number */
#define HALDRVID_PAD        2       /* PINPAD driver number */
#define HALDRVID_EEP        3       /* EEPROM driver number */
#define HALDRVID_MAG        4       /* magnetic card reader card driver number */
#define HALDRVID_DAT        5       /* DATe driver number */
#define HALDRVID_COM        6       /* COM driver number */
#define HALDRVID_ICC        7       /* Smart card driver number */
#define HALDRVID_I2C        8       /* I2C driver number */
#define HALDRVID_CHE        9       /* CHEQUE reader */
#define HALDRVID_PME        10      /* PME peripheral */
#define HALDRVID_GSM        11      /* GSM module */
#define HALDRVID_ADC        12      /* ADC module */
#define HALDRVID_MIFARE     14      /* MIFARE module */
#define HALDRVID_WIFI       16      /* WIFI module */
#define HALDRVID_APP        0x7F
#define HALDRVID_OS			0x80

#define HALDEVTYPE_IIC          0x01
#define HALDEVTYPE_UART         0x02
#define HALDEVTYPE_ETHERNET     0x03

#define HALDRVCFG_GET       0
#define HALDRVCFG_SET       1

#define HALDRVSTATE_STOPED      0   /* Order execution terminated on error    */
#define HALDRVSTATE_ENDED       1   /* DRIVER TREATMENT ENDED OK              */
#define HALDRVSTATE_ABSENT      2   /* Peripheral does not answer             */
#define HALDRVSTATE_FREE        3   /* Driver free for ordering               */
#define HALDRVSTATE_WAIT        4   /* Order waiting for execution            */
#define HALDRVSTATE_RUNNING     5   /* Order is running                       */
#define HALDRVSTATE_FULL        6   /* Driver waiting stack is full           */
#define HALDRVSTATE_UNKNOWN     7   /* Unknown order                          */
#define HALDRVSTATE_REJECTED    8   /* Driver does not exist (wrong driver id)*/
#define HALDRVSTATE_NOABORT     9   /* Abort refused to the requester         */
#define HALDRVSTATE_RESET       10  /* Orders in waiting stack are reseted    */
#define HALDRVSTATE_CONTINUE    0xFF
/*
typedef struct
{
    unsigned char ucType;
    unsigned char ucLen;
    unsigned char aucData[256];
}DRVIN;

typedef struct
{
    unsigned char ucReqNR;
    unsigned char ucGenStatus;
    unsigned char ucDrvStatus;
    unsigned char aucData[256];
}DRVOUT;

typedef struct
{
    unsigned char ucDrvID;
    unsigned char ucDrvCMD;
    DRVIN   *pDrvIn;
    DRVOUT  *pDrvOut;
}DRV;

typedef struct
{
    unsigned char ucDrvID;
    unsigned char ucDrvCMD;
    DRVIN   pDrvIn;
    DRVOUT  pDrvOut;
}DRV_MMAP;



typedef struct
{
    unsigned char ucDrvID;
    DRVIN   *pDrvIn;
    DRVOUT  *pDrvOut;
    unsigned char (*pfnInstall)(void);
    unsigned char (*pfnUninstall)(void);
    unsigned char (*pfnTest)(void);
    unsigned char (*pfnIoctl)(unsigned char ucType,unsigned char *pucData);
    unsigned char (*pfnState)(DRVIN *pDrvIn,DRVOUT *pDrvOut);
    unsigned char (*pfnRun)(DRVIN *pDrvIn,DRVOUT *pDrvOut);
    unsigned char (*pfnAbort)(DRVIN *pDrvIn,DRVOUT *pDrvOut);
    unsigned char (*pfnReset)(DRVIN *pDrvIn,DRVOUT *pDrvOut);
    unsigned char (*pfnRecvHook)(unsigned char ucDevID,unsigned char *pucRecvData,unsigned int uiRecvLen);
}HALDRV;
*/
void HALDRV_Init(void);
unsigned char HALDRV_Install(HALDRV *pHALDrv);
unsigned char HALDRV_Uninstall(unsigned char ucDrvID);
unsigned char HALDRV_Process(DRV *pDrv);
unsigned char HALDRV_Abort(DRV *pDrv);
unsigned char HALDRV_Run(DRV *pDrv);
unsigned char HALDRV_State(DRV *pDrv);
unsigned char HALDRV_Reset(DRV *pDrv);
unsigned char HALDRV_Ioctl(unsigned char ucDrvID,unsigned char ucType,unsigned char *pucData);
unsigned char HALDRV_IICSendProcess(unsigned char ucSlaveAddr,
                    unsigned char ucDrvID,
                    unsigned char ucDrvCMD,
                    unsigned char ucType,
                    unsigned char *pucSendData,
                    unsigned int uiSendLen);


HALDRVDATA volatile HALDRV *apHALDrv[HALDRV_MAXNB];


#ifdef __cplusplus
}
#endif

#endif
