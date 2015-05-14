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
20100531 ���ӷǽӴ�����
0729 ������Ŀǰ���漰IC�������ಿ����ʱ�����ӡ�

*/
#include <stdio.h>
#include <osdriver.h>
#include <haldrv.h>
#include <linux/ioctl.h>
#include <string.h>
#include <sys/mman.h>

#include <stdlib.h>

extern int fd_icc;
#define DRV_DBG Uart_Printf
extern DRV_MMAP *myOSICCDrv;
//extern DRV_MMAP *myOSICCDrv2;
//extern DRV_MMAP *myOSPCDDrv;//����ֱ�ӷ���
extern DRV_MMAP *myOSMFDrv;//����ֱ�ӷ���

#define IC_CMD_PROCESS   _IOWR('I', 0, DRV)
#define IC_CMD_GETSTATUS _IOWR('I', 1, DRVOUT)
#define IC_CMD_GETDRVOUT _IOWR('I', 2, DRVOUT)

#define MF_CMD_PROCESS	 _IOWR('P',  0,DRV)

//#define M1_TEST     0


DRVOUT *ic_drvout_data;

//���ȼ�> DBG
//#define DBG1 //Uart_Printf

extern int fd_mifare;

extern DRVIN	OSMIFAREDrvIn;
extern DRVOUT	OSMIFAREDrvOut;
extern DRVOUT	OSMIFAREDrvOut2;

void OSDRV_Init(void)
{
    OSICC_Init();
}


void OSDRV_Active(unsigned char ucDrvID,DRVIN *pDrvIn,DRVOUT *pDrvOut)
{
    DRV Drv;
    int i;
    DRV_MMAP tmp_drv;
    DRVOUT *pReturn;

#ifdef DBG
    Uart_Printf("%s START\n", __func__);
#endif

    if(fd_icc < 0)
    {
        OSDRV_Init();
        
        if(fd_icc < 0)
        {
            printf("### %s  fd_icc < 0 ###\n", __func__);
            return;
        }
    }

    switch ( ucDrvID )
    {
        case HALDRVID_ICC:
        {
              memset(pDrvOut, 0, sizeof(DRVOUT));
        
            tmp_drv.ucDrvID = ucDrvID;
            tmp_drv.ucDrvCMD = HALDRVCMD_EXECUTE;
            tmp_drv.pDrvIn.ucType = pDrvIn->ucType;
            tmp_drv.pDrvIn.ucLen = pDrvIn->ucLen;
            memcpy((void *)tmp_drv.pDrvIn.aucData, pDrvIn->aucData, sizeof(pDrvIn->aucData));
         //   memset((void *)tmp_drv.pDrvOut.aucData, 0, sizeof(tmp_drv.pDrvOut.aucData));

//             printf("###%s,%d### ucDrvCMD:0x%x  ucLen=%d\n",__func__, __LINE__,  
//                                                tmp_drv.ucDrvCMD, tmp_drv.pDrvIn.ucLen);
//                
	    	ioctl(fd_icc,IC_CMD_PROCESS, (void *)&tmp_drv);
	    	pReturn = (DRVOUT *)&tmp_drv;
#if 0
	    	  printf("\r\n$$$ ===== [%s %d] return -=====\r\n", __func__,__LINE__);
	    	  for (i=0; i<pReturn->aucData[0]+1; i++)
	    	    printf("%02x ", pReturn->aucData[i]);
	    	    printf("\r\n");
#endif
	    	 //   printf("\r\n*************** %s ************************\r\n", __func__);
	    	    memcpy(pDrvOut, pReturn, sizeof(DRVOUT));
            break;
        }
    }
}

void OSDRV_Abort(unsigned char ucDrvID);
void OSDRV_AbortAllExcept(unsigned char ucDrvKeepNR)
{
    unsigned char ucDrvNR;

    while (ucDrvNR < HALDRV_MAXNB)
    {
        if ( ucDrvNR != ucDrvKeepNR)
        {
            OSDRV_Abort(ucDrvNR);
        }
        ucDrvNR++;
    }
}

unsigned char OSDRV_Wait(DRVOUT *pDrvOut)
{
    int i, ret;
//xu.xb    
#if 0
    while(myOSICCDrv->pDrvOut.ucGenStatus != HALDRVSTATE_ENDED)
    {
        usleep(1000);   //xu.xb
    }
    //ioctl(fd_icc, IC_CMD_GETDRVOUT, pDrvOut);
#else

printf("before %s\n",__func__);
    ret = read(fd_icc, pDrvOut, sizeof(pDrvOut));
printf("finish\n");
#endif

    return(pDrvOut->ucDrvStatus);
}

/* just for mifare wait */
unsigned char OSDRV_Wait_mifare(DRVOUT *pDrvOut)
{
    unsigned char ucResult;
    

//    while (myOSMFDrv->pDrvOut.ucGenStatus != HALDRVSTATE_ENDED)
//    {
//        usleep(1000);   //xu.xb
//    }

    read(fd_mifare, pDrvOut, sizeof(pDrvOut));

    ucResult=pDrvOut->ucDrvStatus;

    return ucResult;
}


void OSDRV_Abort(unsigned char ucDrvID)
{
    DRV		Drv;
    DRVOUT 	DrvOutAbort;
    DRVOUT *pReturn;
    //xu.xb
    memset(&DrvOutAbort,0xAA,sizeof(DRVOUT));

    Drv.ucDrvID = ucDrvID;
    Drv.ucDrvCMD = HALDRVCMD_RESET;
    Drv.pDrvIn = 0;
    Drv.pDrvOut = &DrvOutAbort;

	switch(ucDrvID){
		case HALDRVID_ICC:
			ioctl(fd_icc,IC_CMD_PROCESS, &Drv);
			pReturn = (DRVOUT *)&Drv;
			memcpy(&DrvOutAbort, pReturn, sizeof(DRVOUT));
			//OSDRV_Wait(&DrvOutAbort);
			break;
		case HALDRVID_MIFARE:

		    if(fd_mifare < 0)
		    {
		        break;
		    }

			ioctl(fd_mifare,MF_CMD_PROCESS, &Drv);

			OSDRV_Wait_mifare(&DrvOutAbort);

#ifdef  M1_TEST
			printf("--------- %s -------------\n", __func__);
#endif

			break;
	}
    return;
}


void OSDRV_Call(DRV *pDrv)
{
//    HALDRV_Process(pDrv);

	 DRVOUT *pReturn, outsave;
    ioctl(fd_icc,IC_CMD_PROCESS, pDrv);
    pReturn = (DRVOUT *)pDrv;
	memcpy(&outsave, pReturn, sizeof(DRVOUT));
	memcpy(pDrv->pDrvOut, &outsave, sizeof(DRVOUT));

   // OSDRV_Wait(pDrv->pDrvOut);

    return;
}


unsigned char OSDRV_Ioctl(unsigned char ucDrvID,unsigned char ucType,unsigned char *pucData)
{
#if 0
    return HALDRV_Ioctl(ucDrvID,ucType,pucData);
#endif
}
#if 0
DRVOUT *OSDRV_CallAsync(DRV *pDrv)
{
//    HALDRV_Process(pDrv);
    ioctl(fd_icc,IC_CMD_PROCESS, pDrv);

    return(pDrv->pDrvOut);
}


#endif
