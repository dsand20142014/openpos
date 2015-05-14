/*
	Library Function for ICC
--------------------------------------------------------------------------
	FILE  emv.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-02-22		Xiaoxi Jiang
    Last modified :	2003-02-22		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/

#include "halicc.h"
#include "libicc.h"
//#include <tools.h>
#include <linux/string.h>
#include <linux/delay.h>

extern unsigned char ATR_SpProcess(unsigned char ucReader);

 unsigned short QD_uiTimeoutCount;
unsigned char QD_aucR_Apdu[16];

void QINGDAO_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);
void QINGDAO_ResetRecvCharProc(unsigned char ucReader,unsigned char ucCh);
void QINGDAO_APDUExchRecvCharProc(unsigned char ucReader,unsigned char ucCh);
void QINGD_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);



unsigned char QINGDAO_AsyncReset(unsigned char ucReader)
{
    unsigned char ucResult;
    ICCINFO *pCurrICCInfo;
    unsigned char ucPresent;

    pCurrICCInfo = &ICCInfo[ucReader];

    ucResult = HALICC_ResetUart(ucReader);
   
    if( !ucResult )
    {
    	ucResult = HALICC_SetMethod(ucReader,ICCMETHOD_ASYNC);
    }
    if( !ucResult )
    {
    	ucResult = HALICC_SetClock(ucReader,0,1);
    }
    if( !ucResult )
    {
        pCurrICCInfo->ucATRLen = 0;
        memset(pCurrICCInfo->aucATR,0,sizeof(pCurrICCInfo->aucATR));
        ucResult = HALICC_SetBaud(ucReader,1);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetDi(ucReader,4);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetProtocol(ucReader,1);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetConvention(ucReader,1);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetPowerType(ucReader,pCurrICCInfo->ucPowerType);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetPEC(ucReader,0);
    }
    if( !ucResult )
    {
        ucResult = HALICC_Active(ucReader);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetTimer(ucReader,0x61,0x00,0x6C,0x00,0);
	}
    if( !ucResult )
    {
        ucResult = HALICC_Detect(ucReader,&ucPresent);
    }
    if( !ucResult )
    {
        if( ucPresent )
        {
            pCurrICCInfo->ucSession = ICCSESSION_COLDRESET;
            pCurrICCInfo->ucResetType = LIBICC_COLDRESET;
            pCurrICCInfo->ucStatus = ICCSTATUS_POWERED;
    	    pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET1;
    	    QD_uiTimeoutCount = 0;
    	    return( ucResult );
    	}
    }
    HALICC_Deactive(ucReader);
    pCurrICCInfo->ucSession = ICCSESSION_IDLE;

    return(ucResult);
}

unsigned char QINGDAO_WarmReset(unsigned char ucReader)
{
    unsigned char ucResult;
    ICCINFO *pCurrICCInfo;
    unsigned char ucPresent;

    pCurrICCInfo = &ICCInfo[ucReader];

	//HALTIMER_Delay(10);
	mdelay(1);
    ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0);
    if( !ucResult )
    {
        pCurrICCInfo->ucATRLen = 0;
        memset(pCurrICCInfo->aucATR,0,sizeof(pCurrICCInfo->aucATR));
        ucResult = HALICC_SetBaud(ucReader,1);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetDi(ucReader,4);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetProtocol(ucReader,1);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetConvention(ucReader,1);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetPEC(ucReader,0);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetTimer(ucReader,0x61,0x00,0x6C,0x00,0);
	}
    if( !ucResult )
    {
        ucResult = HALICC_Detect(ucReader,&ucPresent);
    }
    if( !ucResult )
    {
        if( ucPresent )
        {
            pCurrICCInfo->ucSession = ICCSESSION_COLDRESET;
            pCurrICCInfo->ucResetType = LIBICC_WARMRESET;
            pCurrICCInfo->ucStatus = ICCSTATUS_POWERED;
    	    pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET1;
    	    QD_uiTimeoutCount = 0;
    	    return( ucResult );
    	}
    }
    HALICC_Deactive(ucReader);
    pCurrICCInfo->ucSession = ICCSESSION_IDLE;

    return(ucResult);
}

unsigned char QINGDAO_CheckICCParam(unsigned char ucReader)
{
    return(LIBICCERR_SUCCESS);
}

void QINGDAO_RecvCharProc(unsigned char ucReader,unsigned char ucCh)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    switch(pCurrICCInfo->ucSession)
    {
    case ICCSESSION_COLDRESET:
    case ICCSESSION_WARMRESET:
        QINGDAO_ResetRecvCharProc(ucReader,ucCh);
        break;
    case ICCSESSION_DATAEXCH:
        //QINGDAO_APDUExchRecvCharProc(ucReader,ucCh);
        return;
    default:
        break;
    }
}

void QINGDAO_ResetRecvCharProc(unsigned char ucReader,unsigned char ucCh)
{
	unsigned char ucI;
	unsigned char ucResult;
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    switch(pCurrICCInfo->ucSessionStep)
    {
    case ICCSESSIONSTEP_COLDRESET1:
    	break;
    case ICCSESSIONSTEP_COLDRESET2:
    case ICCSESSIONSTEP_COLDRESET3:
		pCurrICCInfo->aucATR[pCurrICCInfo->ucATRLen] = ucCh;
	    pCurrICCInfo->ucATRLen ++;

	    ucResult = ATR_CheckFinished(pCurrICCInfo->aucATR,pCurrICCInfo->ucATRLen);
	    if(  (ucResult != LIBICCERR_ATRNOTFINISHED)
	       &&(ucResult != LIBICCERR_ATRHASLASTCHAR)
	      )
        {
            HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,1);
            pCurrICCInfo->ucLastError = ucResult;
            ATR_SpProcess(ucReader);
            ATR_FinishedProc(ucReader);
        }else
        {
        	if( ucResult == LIBICCERR_ATRNOTFINISHED )
        	{
        		HALICC_SetTimer(ucReader,0x05,0x00,0x00,0x00,1);
            	HALICC_SetTimer(ucReader,0x65,0xC0,0x61,0x27,1);
           		pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET3;
           	}else
           	{
        		HALICC_SetTimer(ucReader,0x05,0x00,0x00,0x00,1);
            	HALICC_SetTimer(ucReader,0xE5,0xC0,0x61,0x27,1);
           		pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET3;
           	}
        }
    	break;
    default:
        LIBICC_CODENOTIMPLEMENT;
        printk("ERROR ATR=");
        for(ucI=0;ucI<pCurrICCInfo->ucATRLen;ucI++)
        {
        	printk("%02x,",pCurrICCInfo->aucATR[ucI]);
        }
        printk("\nucCh=%02x\n",ucCh);
        break;
    }
}

void QINGD_TimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    switch(pCurrICCInfo->ucSession)
    {
    case ICCSESSION_COLDRESET:
    case ICCSESSION_WARMRESET:
        QINGD_ResetTimeoutProc(ucReader,ucTimeoutType);
        break;
    default:
        break;
    }
}

void QINGD_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    switch(ucTimeoutType)
    {
	case 1:
		QD_uiTimeoutCount ++;
		if( QD_uiTimeoutCount >= 105 )
		{
	        HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,0);
	        pCurrICCInfo->ucLastError = LIBICCERR_CARDMUTE;
	    }else
			return;
		break;
    case 3:
	    switch(pCurrICCInfo->ucSessionStep)
	    {
	    case ICCSESSIONSTEP_COLDRESET1:
//			HALICC_EnableRecv(ucReader);
	        HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_RST,0);
	        HALICC_SetTimer(ucReader,0x65,0xc0,0x78,0x09,1);
	    	pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET2;
	    	return;
	    case ICCSESSIONSTEP_COLDRESET2:
	        HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,0);
	        HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0);
	        pCurrICCInfo->ucLastError = LIBICCERR_ISSYNCCARD;
	        break;
	    case ICCSESSIONSTEP_COLDRESET3:
	        HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,0);
	        HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0);
	        pCurrICCInfo->ucLastError = LIBICCERR_CARDMUTE;
//	        Uart_Printf("TimeoutType=%02x\n",ucTimeoutType);
	        break;
	    default:
	        break;
	    }
	    break;
	default:
		LIBICC_CODENOTIMPLEMENT;
		break;
	}
    ATR_FinishedProc(ucReader);
}
