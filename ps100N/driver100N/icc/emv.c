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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>

unsigned short uiTimeoutCount;
unsigned char aucR_Apdu[16];
unsigned char EMV_AsyncReset(unsigned char ucReader)
{
        unsigned char ucResult;
        ICCINFO *pCurrICCInfo;
        unsigned char ucPresent;

        pCurrICCInfo = &ICCInfo[ucReader];

        /*
            if( pCurrICCInfo->ucStatus != ICCSTATUS_INSERT )
                return(LIBICCERR_NOICC);
        */

        ucResult = HALICC_ResetUart(ucReader);
     //   printk("[%s,%d] ucResult:0x%x\n",__func__, __LINE__,ucResult);
        /*
            if( !ucResult )
            {
            	ucResult = HALICC_EnableSend(ucReader);
            }
        */
        if ( !ucResult )
        {
                ucResult = HALICC_SetMethod(ucReader, ICCMETHOD_ASYNC);
        }
    //    printk("[%s,%d] ucResult:0x%x\n",__func__, __LINE__,ucResult);
        if ( !ucResult )
        {
                ucResult = HALICC_SetClock(ucReader, 0, 1);
        }
        if ( !ucResult )
        {
                pCurrICCInfo->ucATRLen = 0;
                memset(pCurrICCInfo->aucATR, 0, sizeof(pCurrICCInfo->aucATR));
                ucResult = HALICC_SetBaud(ucReader, 1);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_SetDi(ucReader, 1);
        }
   //     printk("[%s,%d] ucResult:0x%x\n",__func__, __LINE__,ucResult);
        if ( !ucResult )
        {
                ucResult = HALICC_SetProtocol(ucReader, 1);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_SetConvention(ucReader, 1);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_SetPowerType(ucReader, pCurrICCInfo->ucPowerType);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_SetPEC(ucReader, 0);
        }
   //     printk("[%s,%d] ucResult:0x%x\n",__func__, __LINE__,ucResult);
        if ( !ucResult )
        {
      //  	printk("[%s,%d] ucResult:0x%x\n",__func__, __LINE__,ucResult);
                ucResult = HALICC_Active(ucReader);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_SetTimer(ucReader, 0x61, 0x00, 0x6C, 0x00, 0);

        }
        if ( !ucResult )
        {
                ucResult = HALICC_Detect(ucReader, &ucPresent);
        }
        if ( !ucResult )
        {
                if ( ucPresent )
                {
                        pCurrICCInfo->ucSession = ICCSESSION_COLDRESET;
                        pCurrICCInfo->ucResetType = LIBICC_COLDRESET;
                        pCurrICCInfo->ucStatus = ICCSTATUS_POWERED;
                        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET1;
                        uiTimeoutCount = 0;
                  //      printk("[%s,%d]\n",__func__,__LINE__);
                        return( ucResult );
                }
        }
        HALICC_Deactive(ucReader);
        pCurrICCInfo->ucSession = ICCSESSION_IDLE;

        return(ucResult);
}

unsigned char EMV_WarmReset(unsigned char ucReader)
{
        unsigned char ucResult;
        ICCINFO *pCurrICCInfo;
        unsigned char ucPresent;

        pCurrICCInfo = &ICCInfo[ucReader];

        udelay(10);
        ucResult = HALICC_HandleContact(ucReader, HALICCCONTACT_CLR, HALICCCONTACT_RST, 0);
        if ( !ucResult )
        {
                pCurrICCInfo->ucATRLen = 0;
                memset(pCurrICCInfo->aucATR, 0, sizeof(pCurrICCInfo->aucATR));
                ucResult = HALICC_SetBaud(ucReader, 1);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_SetDi(ucReader, 1);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_SetProtocol(ucReader, 1);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_SetConvention(ucReader, 1);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_SetPEC(ucReader, 0);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_SetTimer(ucReader, 0x61, 0x00, 0x6C, 0x00, 0);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_Detect(ucReader, &ucPresent);
        }
        if ( !ucResult )
        {
                if ( ucPresent )
                {
                        pCurrICCInfo->ucSession = ICCSESSION_COLDRESET;
                        pCurrICCInfo->ucResetType = LIBICC_WARMRESET;
                        pCurrICCInfo->ucStatus = ICCSTATUS_POWERED;
                        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET1;
                        uiTimeoutCount = 0;
                        return( ucResult );
                }
        }
        HALICC_Deactive(ucReader);
        pCurrICCInfo->ucSession = ICCSESSION_IDLE;

        return(ucResult);
}

#if 1
unsigned char EMV_SetAPDUNextCommand(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;
	unsigned short uiI;
 pCurrICCInfo = &ICCInfo[ucReader];
#if 0
	printk("\nRecv data form IC card:\n");
    for(uiI=0;uiI<pCurrICCInfo->uiAPDURecvedLen;uiI++)
    	printk("%02x ",pCurrICCInfo->aucAPDURecvBuf[uiI]);
    printk("\n");	
#endif    
   
    //EN: Get Response
    if( pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x61 )
    {
    	pCurrICCInfo->aucAPDUSendBuf[0] = 0x00;
        pCurrICCInfo->aucAPDUSendBuf[1] = 0xC0;
        pCurrICCInfo->aucAPDUSendBuf[2] = 0x00;
        pCurrICCInfo->aucAPDUSendBuf[3] = 0x00;
        pCurrICCInfo->aucAPDUSendBuf[4] = pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1];
        pCurrICCInfo->uiAPDUSendLen = 5;
        pCurrICCInfo->uiAPDUSentLen = 0;
        pCurrICCInfo->uiAPDURecvedLen -= 2;
        pCurrICCInfo->ucAPDUCase = ICCAPDU_CASE2;
        return(LIBICCERR_SUCCESS);
    }
     /**************************************fante test*****************************************/
	 if(  (pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4 )
	    &&(!pCurrICCInfo->ucProcWord)
	    &&((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x62)
	    	||(pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x63)
	    	||((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2]&0xF0 == 0x90)
        		&&!((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2]==0x90)
        		     &&(pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1]==0x00)))
       	  )	
       	)
      
     {       
 
   	pCurrICCInfo->ucProcWord = 1;
    	memset(aucR_Apdu,0,sizeof(aucR_Apdu));
    	aucR_Apdu[0]=pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2];
    	aucR_Apdu[1]=pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1];
	 }
/*******************************************************/	

    if(  (pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4 )
       &&(pCurrICCInfo->uiAPDUSentLen > 5 )
       &&((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x62)
	    	||(pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x63)
	    	||((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2]&0xF0 == 0x90)
        		&&!((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2]==0x90)
        		 &&(pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1]==0x00)))
          )
        )
    {
    	pCurrICCInfo->aucAPDUSendBuf[0] = 0x00;
        pCurrICCInfo->aucAPDUSendBuf[1] = 0xC0;
        pCurrICCInfo->aucAPDUSendBuf[2] = 0x00;
        pCurrICCInfo->aucAPDUSendBuf[3] = 0x00;
        pCurrICCInfo->aucAPDUSendBuf[4] = 0x00;
        pCurrICCInfo->uiAPDUSendLen = 5;
        pCurrICCInfo->uiAPDUSentLen = 0;
        pCurrICCInfo->uiAPDURecvedLen -= 2;
        pCurrICCInfo->ucAPDUCase = ICCAPDU_CASE2;
        return(LIBICCERR_SUCCESS);
    }
    if( pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x6C )
    {
        pCurrICCInfo->aucAPDUSendBuf[4] = pCurrICCInfo->aucAPDURecvBuf[1];
        pCurrICCInfo->uiAPDUSentLen = 0;
        pCurrICCInfo->uiAPDURecvedLen = 0;
        memset(pCurrICCInfo->aucAPDURecvBuf,0,LIBICC_MAXCOMMBUF);
        return(LIBICCERR_SUCCESS);
    }
/**************************************fante test*****************************************/
    if(pCurrICCInfo->ucProcWord)
    {
    	pCurrICCInfo->ucProcWord = 0;
    	pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2]=aucR_Apdu[0];
    	pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1]=aucR_Apdu[1];
    }	
/*******************************************************/	
    return(LIBICCERR_APDUNONEXTCOMMAND);
}
#endif

#if 0
unsigned char EMV_SetAPDUNextCommand(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;
        unsigned short uiI;

        pCurrICCInfo = &ICCInfo[ucReader];
#if 0
        printk("\n[%s,%d] Recv data form IC card:\n", __func__, __LINE__);
        for (uiI = 0; uiI < pCurrICCInfo->uiAPDURecvedLen; uiI++)
                printk("%02x ", pCurrICCInfo->aucAPDURecvBuf[uiI]);
        printk("\n");
#endif
        //EN: Get Response
        if ( pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x61 )
        {
                pCurrICCInfo->aucAPDUSendBuf[0] = 0x00;
                pCurrICCInfo->aucAPDUSendBuf[1] = 0xC0;
                pCurrICCInfo->aucAPDUSendBuf[2] = 0x00;
                pCurrICCInfo->aucAPDUSendBuf[3] = 0x00;
                pCurrICCInfo->aucAPDUSendBuf[4] = pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1];
                pCurrICCInfo->uiAPDUSendLen = 5;
                pCurrICCInfo->uiAPDUSentLen = 0;
                pCurrICCInfo->uiAPDURecvedLen -= 2;
                pCurrICCInfo->ucAPDUCase = ICCAPDU_CASE2;
                return(LIBICCERR_SUCCESS);
        }
#if 1
        /**************************************fante test*****************************************/
        if (  (pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4 )
                        && (!pCurrICCInfo->ucProcWord)
                        && ((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x62)
                            || (pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x63)
                            || ((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2]&0xF0 == 0x90)
                                && !((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x90)
                                     && (pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1] == 0x00)))
                           )
           )

        {
                pCurrICCInfo->ucProcWord = 1;
                memset(aucR_Apdu, 0, sizeof(aucR_Apdu));
                aucR_Apdu[0] = pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2];
                aucR_Apdu[1] = pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1];
        }
        /*******************************************************/
#endif
        if (  (pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4 )
                        && (pCurrICCInfo->uiAPDUSentLen > 5 )
                        && ((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x62)
                            || (pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x63)
                            || (((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2]&0xF0) == 0x90)
                                && !((pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x90)
                                     && (pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1] == 0x00)))
                           )
           )
        {
                pCurrICCInfo->aucAPDUSendBuf[0] = 0x00;
                pCurrICCInfo->aucAPDUSendBuf[1] = 0xC0;
                pCurrICCInfo->aucAPDUSendBuf[2] = 0x00;
                pCurrICCInfo->aucAPDUSendBuf[3] = 0x00;
                pCurrICCInfo->aucAPDUSendBuf[4] = 0x00;
                pCurrICCInfo->uiAPDUSendLen = 5;
                pCurrICCInfo->uiAPDUSentLen = 0;
                pCurrICCInfo->uiAPDURecvedLen -= 2;
                pCurrICCInfo->ucAPDUCase = ICCAPDU_CASE2;
                return(LIBICCERR_SUCCESS);
        }
        if ( pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x6C )
        {
                pCurrICCInfo->aucAPDUSendBuf[4] = pCurrICCInfo->aucAPDURecvBuf[1];
                pCurrICCInfo->uiAPDUSentLen = 0;
                pCurrICCInfo->uiAPDURecvedLen = 0;
                memset(pCurrICCInfo->aucAPDURecvBuf, 0, LIBICC_MAXCOMMBUF);
                return(LIBICCERR_SUCCESS);
        }
#if 1
        /**************************************fante test*****************************************/
        if (pCurrICCInfo->ucProcWord)
        {
                pCurrICCInfo->ucProcWord = 0;
                pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] = aucR_Apdu[0];
                pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1] = aucR_Apdu[1];
        }
        /*******************************************************/
#endif
        return(LIBICCERR_APDUNONEXTCOMMAND);
}
#endif

unsigned char EMV_CheckICCParam(unsigned char ucReader)
{
        return(LIBICCERR_SUCCESS);
}

void EMV_RecvCharProc(unsigned char ucReader, unsigned char ucCh)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        switch (pCurrICCInfo->ucSession)
        {
        case ICCSESSION_COLDRESET:
        case ICCSESSION_WARMRESET:
                EMV_ResetRecvCharProc(ucReader, ucCh);
                break;
        case ICCSESSION_DATAEXCH:
                EMV_APDUExchRecvCharProc(ucReader, ucCh);
                return;
        default:
                break;
        }
}

void EMV_ResetRecvCharProc(unsigned char ucReader, unsigned char ucCh)
{
        unsigned char ucI;
        unsigned char ucResult;
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        switch (pCurrICCInfo->ucSessionStep)
        {
        case ICCSESSIONSTEP_COLDRESET1:
                break;
        case ICCSESSIONSTEP_COLDRESET2:
        case ICCSESSIONSTEP_COLDRESET3:
                if ( pCurrICCInfo->ucATRLen < LIBICC_MAXATRBUF)
                {
                        pCurrICCInfo->aucATR[pCurrICCInfo->ucATRLen] = ucCh;
                        pCurrICCInfo->ucATRLen ++;
                }
#if 0
                ucResult = ATR_CheckFinished(pCurrICCInfo->aucATR, pCurrICCInfo->ucATRLen);
                if (  (ucResult != LIBICCERR_ATRNOTFINISHED)
                                && (ucResult != LIBICCERR_ATRHASLASTCHAR)
                   )
                {
                        HALICC_SetTimer(ucReader, 0x00, 0x00, 0x00, 0x00, 1);
                        pCurrICCInfo->ucLastError = ucResult;
                        ATR_Process(ucReader);
                        ATR_FinishedProc(ucReader);
                }
                else
                {
                        if ( ucResult == LIBICCERR_ATRNOTFINISHED )
                        {
                                HALICC_SetTimer(ucReader, 0x05, 0x00, 0x00, 0x00, 1);
                                HALICC_SetTimer(ucReader, 0x65, 0xC0, 0x61, 0x27, 1);
                                pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET3;
                        }
                        else
                        {
                                HALICC_SetTimer(ucReader, 0x05, 0x00, 0x00, 0x00, 1);
                                HALICC_SetTimer(ucReader, 0xE5, 0xC0, 0x61, 0x27, 1);
                                pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET3;
                        }
                }
#else

              HALICC_SetTimer(ucReader, 0x05, 0x00, 0x00, 0x00, 1);
			  HALICC_SetTimer(ucReader, 0xE5, 0xC0, 0x61, 0x27, 1);
			  pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET3;
#endif
                break;
        default:
                LIBICC_CODENOTIMPLEMENT
                printk("ERROR ATR=");
                for (ucI = 0; ucI < pCurrICCInfo->ucATRLen; ucI++)
                {
                	 printk("ERROR ATR=");
                        printk("%02x,", pCurrICCInfo->aucATR[ucI]);
                }
                printk("\nucCh=%02x\n", ucCh);
                break;
        }
/*
        for (ucI = 0; ucI < pCurrICCInfo->ucATRLen; ucI++)
                      {
                              printk("%02x,", pCurrICCInfo->aucATR[ucI]);
                      }
                      printk("\nucCh=%02x\n", ucCh);
                      */
}

void EMV_APDUExchRecvCharProc(unsigned char ucReader, unsigned char ucCh)
{
}

void EMV_TimeoutProc(unsigned char ucReader, unsigned char ucTimeoutType)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        switch (pCurrICCInfo->ucSession)
        {
        case ICCSESSION_COLDRESET:
        case ICCSESSION_WARMRESET:
                EMV_ResetTimeoutProc(ucReader, ucTimeoutType);
                break;
        default:
                break;
        }
}

void EMV_ResetTimeoutProc(unsigned char ucReader, unsigned char ucTimeoutType)
{
        ICCINFO *pCurrICCInfo;
        unsigned char ucResult;

        pCurrICCInfo = &ICCInfo[ucReader];

        switch (ucTimeoutType)
        {
        case 1:
                uiTimeoutCount ++;
                if ( uiTimeoutCount >= 106 )
                {
                        HALICC_SetTimer(ucReader, 0x00, 0x00, 0x00, 0x00, 0);
                        printk("[%s,%d]\n",__func__,__LINE__);
                        pCurrICCInfo->ucLastError = LIBICCERR_CARDMUTE;
                }
                else
#if 1
                {	ucResult = ATR_CheckFinished(pCurrICCInfo->aucATR, pCurrICCInfo->ucATRLen);
                	                if (  (ucResult != LIBICCERR_ATRNOTFINISHED)
                	                                && (ucResult != LIBICCERR_ATRHASLASTCHAR)
                	                   )
                	                {
                	                        HALICC_SetTimer(ucReader, 0x00, 0x00, 0x00, 0x00, 1);
                	                        pCurrICCInfo->ucLastError = ucResult;
                	                        ATR_Process(ucReader);
                	                        ATR_FinishedProc(ucReader);
                	                }
                	                return;
                }
#else
                return;
#endif

                break;
        case 3:
                switch (pCurrICCInfo->ucSessionStep)
                {
                case ICCSESSIONSTEP_COLDRESET1:
//			HALICC_EnableRecv(ucReader);
                        HALICC_HandleContact(ucReader, HALICCCONTACT_SET, HALICCCONTACT_RST, 0);
                       // HALICC_SetTimer(ucReader, 0x65, 0xc0, 0x78, 0x00, 1);
                        HALICC_SetTimer(ucReader, 0x65, 0xc0, 0x7C, 0x00, 1);
                        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET2;
                        return;
                case ICCSESSIONSTEP_COLDRESET2:
                        HALICC_SetTimer(ucReader, 0x00, 0x00, 0x00, 0x00, 0);
                        HALICC_HandleContact(ucReader, HALICCCONTACT_CLR, HALICCCONTACT_RST, 0);
                        pCurrICCInfo->ucLastError = LIBICCERR_ISSYNCCARD;
                        break;
                case ICCSESSIONSTEP_COLDRESET3:
                        HALICC_SetTimer(ucReader, 0x00, 0x00, 0x00, 0x00, 0);
                        HALICC_HandleContact(ucReader, HALICCCONTACT_CLR, HALICCCONTACT_RST, 0);
                        printk("[%s,%d]\n",__func__,__LINE__);
			//HALICC_Deactive(ucReader);
                        pCurrICCInfo->ucLastError = LIBICCERR_CARDMUTE;
//	        printk("TimeoutType=%02x\n",ucTimeoutType);
                        break;
                default:
                        break;
                }
                break;
        default:
                LIBICC_CODENOTIMPLEMENT
                break;
        }
	
	
        ATR_FinishedProc(ucReader);
}
