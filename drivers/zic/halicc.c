/*
	Hardware Abstraction Layer
--------------------------------------------------------------------------
	FILE  haldrv.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-01-06		Xiaoxi Jiang
    Last modified :	2003-01-06		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/

#include <stdarg.h>
#include <string.h>
#define HALDRVDATA
#include <hal.h>
#include <haldata.h>
#undef HALDRVDATA
#include <halmag.h>
#include "halicc.h"
#include <halmmi.h>
#include <haldot.h>


void HALDRV_Init(void)
{
    void HALDRV_IICRecvProcess(unsigned char *,unsigned int);
    unsigned char ucI;

	for(ucI=0;ucI<HALDRV_MAXNB;ucI++)
	{
		apHALDrv[ucI] = 0;
	}
   // IICData.pfnRecvCharProc = HALDRV_IICRecvProcess;
}

unsigned char HALDRV_Install(HALDRV *pHALDrv)
{
	unsigned char ucResult;
	unsigned char ucDrvID;

	ucDrvID = pHALDrv->ucDrvID;
	if( ucDrvID < HALDRVID_RESERVNB )
	{
		if( apHALDrv[ucDrvID] )
			return(HALDRVERR_SUCCESS);
		else
		{
			if( pHALDrv->pfnInstall )
			{
				apHALDrv[ucDrvID] = pHALDrv;
				ucResult = (*(pHALDrv->pfnInstall))();
/*
				if( ucResult )
				{
        			if( pHALDrv->pfnUninstall )
        				(*(pHALDrv->pfnUninstall))();
    				apHALDrv[ucDrvID] = 0;
				}
*/
				return(ucResult);
			}
			return(HALDRVERR_NOHANDLE);
		}
	}
	return(HALDRVERR_BADDRVID);
}

unsigned char HALDRV_Uninstall(unsigned char ucDrvID)
{
	unsigned char ucResult;
	volatile HALDRV *pHALDrv;

	ucResult = HALDRVERR_SUCCESS;
	if( ucDrvID < HALDRVID_RESERVNB )
	{
		if( !apHALDrv[ucDrvID] )
			return(HALDRVERR_NOTINSTALL);
		else
		{
			pHALDrv = apHALDrv[ucDrvID];
			if( pHALDrv->pfnUninstall )
			{
				ucResult = (*(pHALDrv->pfnUninstall))();
			}
			apHALDrv[ucDrvID] = 0;
			return(ucResult);
		}
	}
	return(HALDRVERR_BADDRVID);
}

unsigned char HALDRV_Process(DRV *pDrv)
{
	switch( pDrv->ucDrvCMD )
	{
	case HALDRVCMD_ABORT:
		return(HALDRV_Abort(pDrv));
	case HALDRVCMD_EXECUTE:
		return(HALDRV_Run(pDrv));
	case HALDRVCMD_STATEREQ:
		return(HALDRV_State(pDrv));
	case HALDRVCMD_RESET:
		return(HALDRV_Reset(pDrv));
	default:
		return(HALDRVERR_BADCMD);
	}
}

unsigned char HALDRV_Abort(DRV *pDrv)
{
	volatile HALDRV *pHALDrv;

	if( pDrv->ucDrvID >= HALDRV_MAXNB)
	{
		return(HALDRVERR_BADDRVID);
	}
	if( !apHALDrv[pDrv->ucDrvID] )
	{
		return(HALDRVERR_NOTEXIST);
	}
	pHALDrv = apHALDrv[pDrv->ucDrvID];
	if( pHALDrv->pfnAbort )
	{
		return((*(pHALDrv->pfnAbort))(pDrv->pDrvIn,pDrv->pDrvOut));
	}else
	{
		return(HALDRVERR_NOHANDLE);
	}
}

unsigned char HALDRV_Run(DRV *pDrv)
{
	volatile HALDRV *pHALDrv;

	if( pDrv->ucDrvID >= HALDRV_MAXNB)
	{
		return(HALDRVERR_BADDRVID);
	}
	if( !apHALDrv[pDrv->ucDrvID] )
	{
		return(HALDRVERR_NOTEXIST);
	}

	/*
	* zy: ����豸��ѡ����Ӧ����
	*/
	pHALDrv = apHALDrv[pDrv->ucDrvID];
	if( pHALDrv->pfnRun )
	{
		return((*(pHALDrv->pfnRun))(pDrv->pDrvIn,pDrv->pDrvOut));
	}else
	{
		return(HALDRVERR_NOHANDLE);
	}
}

unsigned char HALDRV_State(DRV *pDrv)
{
	volatile HALDRV *pHALDrv;

	if( pDrv->ucDrvID >= HALDRV_MAXNB)
	{
		return(HALDRVERR_BADDRVID);
	}
	if( !apHALDrv[pDrv->ucDrvID] )
	{
		return(HALDRVERR_NOTEXIST);
	}
	pHALDrv = apHALDrv[pDrv->ucDrvID];
	if( pHALDrv->pfnState )
	{
		return((*(pHALDrv->pfnState))(pDrv->pDrvIn,pDrv->pDrvOut));
	}else
	{
		return(HALDRVERR_NOHANDLE);
	}
}

unsigned char HALDRV_Reset(DRV *pDrv)
{
	volatile HALDRV *pHALDrv;

	if( pDrv->ucDrvID >= HALDRV_MAXNB)
	{
		return(HALDRVERR_BADDRVID);
	}
	if( !apHALDrv[pDrv->ucDrvID] )
	{
		return(HALDRVERR_NOTEXIST);
	}
	pHALDrv = apHALDrv[pDrv->ucDrvID];
	if( pHALDrv->pfnReset )
	{
		return((*(pHALDrv->pfnReset))(pDrv->pDrvIn,pDrv->pDrvOut));
	}else
	{
		return(HALDRVERR_NOHANDLE);
	}
}

unsigned char HALDRV_Ioctl(unsigned char ucDrvID,unsigned char ucType,unsigned char *pucData)
{
	volatile HALDRV *pHALDrv;

    if( ucDrvID > HALDRV_MAXNB )
        return(HALDRVERR_NOTEXIST);
	if( !apHALDrv[ucDrvID] )
	{
		return(HALDRVERR_NOTEXIST);
	}
	pHALDrv = apHALDrv[ucDrvID];
	if( pHALDrv->pfnIoctl )
	{
		return((*(pHALDrv->pfnIoctl))(ucType,pucData));
	}else
	{
		return(HALDRVERR_NOHANDLE);
	}
}

unsigned char HALDRV_IICSendProcess(unsigned char ucSlaveAddr,
					unsigned char ucDrvID,
					unsigned char ucDrvCMD,
					unsigned char ucType,
					unsigned char *pucSendData,
					unsigned int uiSendLen)
{
#if 0
	unsigned char aucDriverIICData[IICBUFLEN];
	unsigned int uiDriverIICDataLen;

	memset(aucDriverIICData,0,sizeof(aucDriverIICData));
	uiDriverIICDataLen = 0;

	aucDriverIICData[uiDriverIICDataLen++] = HALIICMAINADDR;
	aucDriverIICData[uiDriverIICDataLen++] = ucDrvID;
	aucDriverIICData[uiDriverIICDataLen++] = ucDrvCMD;
	aucDriverIICData[uiDriverIICDataLen++] = ucType;
	memcpy(&aucDriverIICData[uiDriverIICDataLen],pucSendData,uiSendLen);
	uiDriverIICDataLen += uiSendLen;

	if( HALIIC_SendData(ucSlaveAddr,
			aucDriverIICData,uiDriverIICDataLen) )
	{
		return(HALDRVERR_NOTRESPONSE);
	}
#endif	
	return( HALDRVERR_SUCCESS );
}

void HALDRV_IICRecvProcess(unsigned char *pucRecvData,unsigned int uiRecvLen)
{
#if 0
    unsigned char ucDrvNR;
    unsigned char *pucPtr;
    volatile HALDRV *pHALDrv;

#if 0
#ifdef OSDRIVER_DEBUG
    unsigned int uiI;
#endif
#endif

#if 0
#ifdef OSDRIVER_DEBUG
	printk("IIC DRIVER Data %d=",uiRecvLen);
#if 0
	for(uiI=0;uiI<uiRecvLen;uiI++)
	{
		printk("%02x,",*(pucRecvData+uiI));
	}
#endif
	printk("\n");
#endif
#endif
    pucPtr = pucRecvData;
    ucDrvNR = *pucPtr;
    pucPtr ++;
    if( ucDrvNR < HALDRV_MAXNB )
    {
    	if( apHALDrv[ucDrvNR] )
    	{
			pHALDrv = apHALDrv[ucDrvNR];
			if( pHALDrv->pfnRecvHook )
			{
				(*(pHALDrv->pfnRecvHook))(HALDEVTYPE_IIC,pucRecvData,uiRecvLen);
			}else
			{
#ifdef OSDRIVER_DEBUG
		    	printk("DRVID=%d no hook\n",ucDrvNR);
#endif
			}
    	}else
    	{
#ifdef OSDRIVER_DEBUG
		    printk("DRVID=%d not install\n",ucDrvNR);
#endif
    	}
    }
#ifdef OSDRIVER_DEBUG
    printk("INVALID DRVID=%d\n",ucDrvNR);
#endif
#endif
}
