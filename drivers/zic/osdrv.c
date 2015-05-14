/*
	Operating System
--------------------------------------------------------------------------
	FILE  osdriver.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2002-10-03		Xiaoxi Jiang
    Last modified :	2002-10-03		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :

ChangeLog:
0729 这里驱动目前仅涉及IC卡，其余部分暂时不增加。

*/

#include <os.h>
#include <osdata.h>

extern int fd_icc;
#define DRV_DBG printf
extern DRVOUT	myOSICCDrvOut;
void OSDRV_Init(void)
{
//    HALDRV_Init();
    OSICC_Init();
}

void OSDRV_Active(unsigned char ucDrvID,DRVIN *pDrvIn,DRVOUT *pDrvOut)
{
    DRV Drv;

    Drv.ucDrvID = ucDrvID;
    Drv.ucDrvCMD = HALDRVCMD_EXECUTE;
    Drv.pDrvIn = pDrvIn;
    Drv.pDrvOut = pDrvOut;
#if 0
    HALDRV_Process(&Drv);
#else
    DRV_DBG("%s\n", __func__);

    OSICC_Init();
    ioctl(fd_icc,IC_CMD_PROCESS, &Drv);

#endif
}

void OSDRV_AbortAllExcept(unsigned char ucDrvKeepNR)
{
    unsigned char ucDrvNR;

    while (ucDrvNR < HALDRV_MAXNB)
    {
        if ( ucDrvNR != ucDrvKeepNR)
            OSDRV_Abort(ucDrvNR);
        ucDrvNR++;
    }
}

unsigned char OSDRV_Wait(DRVOUT *pDrvOut)
{
    OSICC_Init();
    ioctl(fd_icc, IC_CMD_GETSTATUS, &myOSICCDrvOut);
    while (  (myOSICCDrvOut.ucGenStatus == HALDRVSTATE_RUNNING) ||(myOSICCDrvOut.ucGenStatus == HALDRVSTATE_WAIT) );
    printf("************ %s ucGenStatus=%x\n", __func__, myOSICCDrvOut.ucGenStatus);
    memcpy(pDrvOut, &myOSICCDrvOut, sizeof(DRVOUT));


    return(pDrvOut->ucDrvStatus);
}

void OSDRV_Abort(unsigned char ucDrvID)
{
    DRV		Drv;
    DRVOUT 	DrvOutAbort;

    memset(&DrvOutAbort,0x00,sizeof(DRVOUT));
    Drv.ucDrvID = ucDrvID;
    Drv.ucDrvCMD = HALDRVCMD_RESET;
    Drv.pDrvIn = 0;
    Drv.pDrvOut = &DrvOutAbort;
//    HALDRV_Process((DRV *)&Drv);
    OSICC_Init();
    ioctl(fd_icc,IC_CMD_PROCESS, &Drv);

    OSDRV_Wait(&DrvOutAbort);

    return;
}

void OSDRV_Call(DRV *pDrv)
{
//    HALDRV_Process(pDrv);
    OSICC_Init();
    ioctl(fd_icc,IC_CMD_PROCESS, pDrv);

    OSDRV_Wait(pDrv->pDrvOut);

    return;
}


unsigned char OSDRV_Ioctl(unsigned char ucDrvID,unsigned char ucType,unsigned char *pucData)
{
#if 0
    return HALDRV_Ioctl(ucDrvID,ucType,pucData);
#endif
}

DRVOUT *OSDRV_CallAsync(DRV *pDrv)
{
//    HALDRV_Process(pDrv);
    OSICC_Init();
    ioctl(fd_icc,IC_CMD_PROCESS, pDrv);

    return(pDrv->pDrvOut);
}



