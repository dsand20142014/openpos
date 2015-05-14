/*  */
#ifndef __IMX258_MAG_H__
#define __IMX258_MAG_H__


#define DEVICE_NAME "s3c2410_mag"
#define MAG_MAJOR 0

#define MAX_TK_BITLen		704
#define MAX_TK_BUFLen		140


typedef struct
{
	unsigned short uiStartBit;						
	unsigned char aucBitBuf[MAX_TK_BITLen];		
}TK_BITBUFF;

typedef struct
{
    unsigned char ucReqNR;
    unsigned char ucGenStatus;
    unsigned char ucDrvStatus;
    unsigned char aucData[256];
}DRVOUT;

typedef struct
{
    unsigned char ucBufLen;
    unsigned char aucTKBuf[MAX_TK_BUFLen];
}TK_BUFF;

typedef struct
{
    unsigned char ucErr;
    unsigned char ucParErr;
    unsigned char ucTKLen;
    unsigned char aucTKData[MAX_TK_BUFLen];
}TK_DATA;


typedef struct
{
    int pin;
    int data;
}PIN_INFO;


//磁卡配置参数
typedef struct
{
	unsigned char ucMagDebug;
	unsigned char ucMagVer[5];
}OSMAGCFG;

OSMAGCFG MagCfg;
	
#define IMX258_MAG_IOCTL_BASE	'M'
#define MAG_RESET _IOW(IMX258_MAG_IOCTL_BASE, 0, int)
#define MAG_READ _IOR(IMX258_MAG_IOCTL_BASE, 1, int)
#define MAG_READ_STATUS _IOWR(IMX258_MAG_IOCTL_BASE, 2, int) //读取刷卡状态
#define MAG_SET_CFG _IOWR(IMX258_MAG_IOCTL_BASE, 3, OSMAGCFG)//set parameter

#define IMX258_MAG_IOC_MAXNR 10

#endif

