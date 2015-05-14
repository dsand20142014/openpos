/*
	Library Function for ICC
--------------------------------------------------------------------------
	FILE  iso7816.c
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
#include <linux/module.h>
#include <linux/kernel.h>


/*zy add 0729*/
void SHSAM1_ResetRecvCharProc(unsigned char ucReader,unsigned char ucCh);
void SHSAM1_APDUExchRecvCharProc(unsigned char ucReader,unsigned char ucCh);
void SHSAM1_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);
/*zy add 0729 end*/

unsigned char SHSAM1_AsyncReset(unsigned char ucReader)
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
        ucResult = HALICC_SetBaud(ucReader,4);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetDi(ucReader,1);
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
            pCurrICCInfo->ucStatus = ICCSTATUS_POWERED;
    	    pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET1;
    	    return( ucResult );
    	}
    }
    HALICC_Deactive(ucReader);
    pCurrICCInfo->ucSession = ICCSESSION_IDLE;

    return(ucResult);
}

unsigned char SHSAM1_WarmReset(unsigned char ucReader)
{
    unsigned char ucResult;
    ICCINFO *pCurrICCInfo;
    unsigned char ucPresent;

    pCurrICCInfo = &ICCInfo[ucReader];

    ucResult = HALICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,0);
    if( !ucResult )
    {
        pCurrICCInfo->ucATRLen = 0;
        memset(pCurrICCInfo->aucATR,0,sizeof(pCurrICCInfo->aucATR));
        ucResult = HALICC_SetBaud(ucReader,4);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetDi(ucReader,1);
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
    	    return( ucResult );
    	}
    }
    HALICC_Deactive(ucReader);
    pCurrICCInfo->ucSession = ICCSESSION_IDLE;

    return(ucResult);
}

unsigned char SHSAM1_SetAPDUNextCommand(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    if(  (pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE2)
       ||(pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4)
      )
    {   //EN: Get Response
        if(  (pCurrICCInfo->aucAPDURecvBuf[0] == 0x61)
           ||(  (pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4)
              &&(  (pCurrICCInfo->aucAPDURecvBuf[0] == 0x62)
                 ||(pCurrICCInfo->aucAPDURecvBuf[0] == 0x63)
//                 ||(pCurrICCInfo->aucAPDURecvBuf[0] == 0x90)
                )
             )
          )
        {
            pCurrICCInfo->aucAPDUSendBuf[1] = 0xC0;
            pCurrICCInfo->aucAPDUSendBuf[2] = 0x00;
            pCurrICCInfo->aucAPDUSendBuf[3] = 0x00;
            if( pCurrICCInfo->aucAPDURecvBuf[0] == 0x61 )
            {
                pCurrICCInfo->aucAPDUSendBuf[4] = pCurrICCInfo->aucAPDURecvBuf[1];
            }else
            {
            	if( pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4 )
            	{
            		pCurrICCInfo->aucAPDUSendBuf[4] = pCurrICCInfo->aucAPDUSendBuf[pCurrICCInfo->
uiAPDUSendLen];
            	}else
            	{
            		pCurrICCInfo->aucAPDUSendBuf[4] = 0x00;
            	}
            }
            pCurrICCInfo->uiAPDUSendLen = 5;
            pCurrICCInfo->uiAPDUSentLen = 0;
            pCurrICCInfo->uiAPDURecvedLen = 0;
            memset(pCurrICCInfo->aucAPDURecvBuf,0,LIBICC_MAXCOMMBUF);
            pCurrICCInfo->ucAPDUCase = ICCAPDU_CASE2;
            return(LIBICCERR_SUCCESS);
        }
    }
    if( pCurrICCInfo->aucAPDURecvBuf[0] == 0x6C )
    {
        pCurrICCInfo->aucAPDUSendBuf[4] = pCurrICCInfo->aucAPDURecvBuf[1];
        pCurrICCInfo->uiAPDUSentLen = 0;
        pCurrICCInfo->uiAPDURecvedLen = 0;
        memset(pCurrICCInfo->aucAPDURecvBuf,0,LIBICC_MAXCOMMBUF);
        return(LIBICCERR_SUCCESS);
    }
    return(LIBICCERR_APDUNONEXTCOMMAND);
}

unsigned char SHSAM1_CheckICCParam(unsigned char ucReader)
{
    return(LIBICCERR_SUCCESS);
}

void SHSAM1_RecvCharProc(unsigned char ucReader,unsigned char ucCh)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    switch(pCurrICCInfo->ucSession)
    {
    case ICCSESSION_COLDRESET:
    case ICCSESSION_WARMRESET:
        SHSAM1_ResetRecvCharProc(ucReader,ucCh);
    case ICCSESSION_DATAEXCH:
        SHSAM1_APDUExchRecvCharProc(ucReader,ucCh);
        return;
    default:
        break;
    }
}

void SHSAM1_ResetRecvCharProc(unsigned char ucReader,unsigned char ucCh)
{
	unsigned char ucResult;
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    switch(pCurrICCInfo->ucSessionStep)
    {
    case ICCSESSIONSTEP_COLDRESET2:
    case ICCSESSIONSTEP_COLDRESET3:
		if( pCurrICCInfo->ucATRLen < LIBICC_MAXATRBUF)
		{
	    	pCurrICCInfo->aucATR[pCurrICCInfo->ucATRLen] = ucCh;
	    	pCurrICCInfo->ucATRLen ++;
	    }
	    ucResult = ATR_CheckFinished(pCurrICCInfo->aucATR,pCurrICCInfo->ucATRLen);
	    if(  (ucResult != LIBICCERR_ATRNOTFINISHED)
	       &&(ucResult != LIBICCERR_ATRHASLASTCHAR)
	      )
        {
            HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,1);
            pCurrICCInfo->ucSession = ICCSESSION_IDLE;
            pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_IDLE;
            pCurrICCInfo->ucLastError = ucResult;
            ATR_Process(ucReader);
            ATR_FinishedProc(ucReader);
        }else
        {
            HALICC_SetTimer(ucReader,0x61,0x00,0x80,0x25,1);
           	pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET3;
        }
    	break;
    default:
        LIBICC_CODENOTIMPLEMENT
        break;
    }
}

void SHSAM1_APDUExchRecvCharProc(unsigned char ucReader,unsigned char ucCh)
{
}

void SHSAM1_TimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    switch(pCurrICCInfo->ucSession)
    {
    case ICCSESSION_COLDRESET:
    case ICCSESSION_WARMRESET:
        SHSAM1_ResetTimeoutProc(ucReader,ucTimeoutType);
        break;
    default:
        break;
    }
}

void SHSAM1_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    switch(pCurrICCInfo->ucSessionStep)
    {
    case ICCSESSIONSTEP_COLDRESET1:
        HALICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_RST,0);
        HALICC_SetTimer(ucReader,0x61,0x00,0x78,0x00,1);
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
//        printk("TimeoutType=%02x\n",ucTimeoutType);
        break;
    default:
        break;
    }
    ATR_FinishedProc(ucReader);
}

