/*
	Operating System
--------------------------------------------------------------------------
	FILE  osdriver.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2002-10-03		Xiaoxi Jiang
    Last modified :	2002-10-03		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __OSDRIVER_H
#define __OSDRIVER_H

/* --- GLOBALES STRUCTURES DECLARATIONS --- */

struct  sdrv_in                         /* buffer structure for driver ordering */
{
    unsigned char   type;                   /* type of function to execute */
    unsigned char   length;                 /* data length */
    unsigned char   xxdata[512];          /*  data transmitted to the driver */
};
typedef struct sdrv_in  DRV_IN;

struct  sdrv_out                        /* return buffer structure from the driver */
{
    unsigned char   reqnr;                  /* reserved system */
    volatile unsigned char   gen_status;             /* general status : bus i2c error, running, ended .. */
    unsigned char   drv_status;             /* order execution report */
    unsigned char   xxdata[512-1];        /*  pay attition to minus one 120313 lyf  returning data */
    unsigned char   length;
};
typedef struct sdrv_out DRV_OUT;


struct  sdriver                         /* buffer commande structure for driver */
{
    unsigned char   drv_nr;                 /* driver identification */
    unsigned char   type;                   /* order type EXEC, RESET, ABORT */
//#if defined(_XA_) || defined(__arm)
    DRV_IN  *   pt_drv_in;              /* order buffer address */
    DRV_OUT *   pt_drv_out;             /* return buffer address */
//#else
//    unsigned short    pt_drv_in;              /* order buffer address */
//    unsigned short    pt_drv_out;             /* return buffer address */
//#endif
};
typedef struct sdriver  DRIVER;


/* --- CONSTANTES DECLARATIONS --- */

/* order type */
/* ------------ */

#define xabort          0       /* abort driver     */
#define execute         1       /* execute order    */
#define statreq         2       /* state request    */
#define reset_drv       3       /* reset driver     */

/* ident. driver */
/* ------------- */

#define drv_mmi         0       /* Man-Machine-Interface (Display-Keyboard) driver number */
#define drv_prn         1       /* Printer driver number */
#define drv_pad         2       /* PINPAD driver number */
#define drv_eep         3       /* EEPROM driver number */
#define drv_mag         4       /* magnetic card reader card driver number */
#define drv_dat         5       /* DATe driver number */
#define drv_com         6       /* COM driver number */
#define drv_icc         7       /* Smart card driver number */
#define drv_i2c         8       /* I2C driver number */
#define drv_che         9       /* CHEQUE reader */
#define drv_pme         10      /* PME peripheral */
#define drv_gsm         11      /* GSM module */
#define NR_DRV          12      /* number of drivers */


#define DRVCFG_GET       0
#define DRVCFG_SET       1

/* general driver state */
/* ------------------------ */

#define DRV_STOPPED     0   /* Order execution terminated on error    */
#define DRV_ENDED       1   /* DRIVER TREATMENT ENDED OK              */
#define DRV_ABSENT      2   /* Peripheral does not answer             */
#define DRV_FREE        3   /* Driver free for ordering               */
#define DRV_WAIT        4   /* Order waiting for execution            */
#define DRV_RUNNING     5   /* Order is running                       */
#define DRV_FULL        6   /* Driver waiting stack is full           */
#define DRV_UNKNOWN     7   /* Unknown order                          */
#define DRV_REJECTED    8   /* Driver does not exist (wrong driver id)*/
#define DRV_NOABORT     9   /* Abort refused to the requester         */
#define DRV_RESET       10  /* Orders in waiting stack are reseted    */

/* 25 keys keyboard */
/* ---------------- */

#define KEY_ENTER       0x3A
#define KEY_CLEAR       0x3B
#define KEY_00_PT       0x3C
#define KEY_CAMERA		0xCA
// backspace key
#define KEY_BCKSP       0x3D
#define KEY_F1          0x40
#define KEY_F2          0x41
#define KEY_F3          0x42
#define KEY_F4          0x43
#define KEY_F5          0x44
#define KEY_F6          0x45
#define KEY_F7          0x46
#define KEY_F8          0x47
#define KEY_F9          0x48
#define KEY_F           0x45	/*=KEY_F6*/
#define KEY_R           0x46	/*=KEY_F7*/

// Special keys values for INI(F0) and paperfeed
#define KEY_SYS         0x3F
#define KEY_PAPER_FEED  0x3E


typedef struct
{
    unsigned char ucDebug;
    unsigned char ucExistFlag;
    unsigned char ucIICAddr;
    unsigned char ucIICProcFlag;
    unsigned char ucType;
    unsigned char ucFunc;
    unsigned char ucSoftMainVer;
    unsigned char ucSoftSubVer;
    unsigned char ucPcbMainVer;
    unsigned char ucPcbSubVer;
    unsigned char aucBoardType[16];
    unsigned char ucDisplayRowNb;
    unsigned char ucDisplayColNb;
    unsigned char ucKeyRowNb;
    unsigned char ucKeyColNb;
    unsigned char ucPrintType;
    unsigned char aucResv[16];
}PADCFG;

#endif

