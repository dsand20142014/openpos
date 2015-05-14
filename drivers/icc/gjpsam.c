/*
	Library Function for ICC
--------------------------------------------------------------------------
	FILE  gjsam.c
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

unsigned short uiGjPsamTimeoutCount;


unsigned char GJPSAM_AsyncReset(unsigned char ucReader)
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
                ucResult = HALICC_SetDi(ucReader, 4);
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
                ucResult = HALICC_SetPowerType(ucReader, pCurrICCInfo->ucPowerType);
        }
        if ( !ucResult )
        {
                ucResult = HALICC_SetPEC(ucReader, 0);
        }
        if ( !ucResult )
        {
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
                        uiGjPsamTimeoutCount = 0;
                        return( ucResult );
                }
        }
        HALICC_Deactive(ucReader);
        pCurrICCInfo->ucSession = ICCSESSION_IDLE;

        return(ucResult);
}

unsigned char GJPSAM_WarmReset(unsigned char ucReader)
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
                        uiGjPsamTimeoutCount = 0;
                        return( ucResult );
                }
        }
        HALICC_Deactive(ucReader);
        pCurrICCInfo->ucSession = ICCSESSION_IDLE;

        return(ucResult);
}

unsigned char GJPSAM_SetAPDUNextCommand(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
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
        if (  (pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4 )
                        && (pCurrICCInfo->uiAPDUSentLen > 5 )
                        && (  (  (pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x9F)
                                 && (pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1] == 0xFF)
                              )
                              || (  (pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x62)
                                    && (pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1] == 0x83)
                                 )
                              || (  (pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2] == 0x63)
                                    && (pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1] == 0x35)
                                 )
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

        return(LIBICCERR_APDUNONEXTCOMMAND);
}

unsigned char GJPSAM_CheckICCParam(unsigned char ucReader)
{
        return(LIBICCERR_SUCCESS);
}

void GJPSAM_RecvCharProc(unsigned char ucReader, unsigned char ucCh)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        switch (pCurrICCInfo->ucSession)
        {
        case ICCSESSION_COLDRESET:
        case ICCSESSION_WARMRESET:
                GJPSAM_ResetRecvCharProc(ucReader, ucCh);
                break;
        case ICCSESSION_DATAEXCH:
                GJPSAM_APDUExchRecvCharProc(ucReader, ucCh);
                return;
        default:
                break;
        }
}

void GJPSAM_ResetRecvCharProc(unsigned char ucReader, unsigned char ucCh)
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
                break;
        default:
                LIBICC_CODENOTIMPLEMENT
                printk("ERROR ATR=");
                for (ucI = 0; ucI < pCurrICCInfo->ucATRLen; ucI++)
                {
                        printk("%02x,", pCurrICCInfo->aucATR[ucI]);
                }
                printk("\nucCh=%02x\n", ucCh);
                break;
        }
}

void GJPSAM_APDUExchRecvCharProc(unsigned char ucReader, unsigned char ucCh)
{
}

void GJPSAM_TimeoutProc(unsigned char ucReader, unsigned char ucTimeoutType)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        switch (pCurrICCInfo->ucSession)
        {
        case ICCSESSION_COLDRESET:
        case ICCSESSION_WARMRESET:
                GJPSAM_ResetTimeoutProc(ucReader, ucTimeoutType);
                break;
        default:
                break;
        }
}

void GJPSAM_ResetTimeoutProc(unsigned char ucReader, unsigned char ucTimeoutType)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        switch (ucTimeoutType)
        {
        case 1:
                uiGjPsamTimeoutCount ++;
                if ( uiGjPsamTimeoutCount >= 105 )
                {
                        HALICC_SetTimer(ucReader, 0x00, 0x00, 0x00, 0x00, 0);
                        pCurrICCInfo->ucLastError = LIBICCERR_CARDMUTE;
                }
                else
                        return;
                break;
        case 3:
                switch (pCurrICCInfo->ucSessionStep)
                {
                case ICCSESSIONSTEP_COLDRESET1:
//			HALICC_EnableRecv(ucReader);
                        HALICC_HandleContact(ucReader, HALICCCONTACT_SET, HALICCCONTACT_RST, 0);
                        //HALICC_SetTimer(ucReader,0x65,0xC0,0x7C,0x01,1);
                        HALICC_SetTimer(ucReader, 0x65, 0x99, 0x86, 0x01, 1);
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

unsigned char GJPSAM_T0SendReceive(unsigned char ucReader)
{
        unsigned char ucStatus;
        unsigned char ucINS;
        unsigned char ucINSMasked;
        unsigned char ucCh;
        unsigned short uiI;
        unsigned short uiAPDURecvedLen;
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        pCurrICCInfo->ucAPDUOneByteFlag = 0;
        pCurrICCInfo->uiAPDUSentLen = 0;
        pCurrICCInfo->ucSession = ICCSESSION_DATAEXCH;
        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND1;
        HALICC_SetPEC(ucReader, 4);
        ucINS = pCurrICCInfo->aucAPDUSendBuf[1];
        ucINSMasked = ~pCurrICCInfo->aucAPDUSendBuf[1];
        pCurrICCInfo->ucAPDUOneByteFlag = 0;
        uiAPDURecvedLen = 0;

        ucStatus = LIBICCERR_SUCCESS;
        HALICC_SetDi(ucReader, 4);
        HALICC_SetTimer(ucReader, 0x61, 0x00, 0x06, 0x00, 1);
        if ( !ucStatus )
        {
                do
                {
                        ucStatus = PROT_T0RecvChar(ucReader, &ucCh);
                        if ( ucStatus )
                                break;
                        if ( ucCh == 0x60 )
                        {
                                /*EN: In TDA8007, don't need to reset timer because timer has been restart*/
#ifdef _WIN32
                                HALICC_EnableRecv(ucReader);
#endif
                                continue;
                        }
                        if (  ( (ucCh&0xF0) == 0x60)
                                        || ( (ucCh&0xF0) == 0x90)
                           )
                        {
                                pCurrICCInfo->ucAPDUOneByteFlag = 0;
                                pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen] = ucCh;
                                pCurrICCInfo->uiAPDURecvedLen ++;
                                uiAPDURecvedLen ++;
                                ucStatus = PROT_T0RecvChar(ucReader, &ucCh);
                                if ( !ucStatus )
                                {
                                        pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen] = ucCh;
                                        pCurrICCInfo->uiAPDURecvedLen ++;
                                        uiAPDURecvedLen ++;
                                }
                                break;
                        }
                        if ( ucCh == ucINS )
                        {
                                pCurrICCInfo->ucAPDUOneByteFlag = 0;
                                if ( pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE2 )
                                {   /*EN: data from card */
                                        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV2;
#ifdef _WIN32
                                        HALICC_EnableRecv(ucReader);
#endif
                                        for (uiI = uiAPDURecvedLen; uiI < (pCurrICCInfo->aucAPDUSendBuf[4] + 2); uiI++)
                                        {
                                                ucStatus = PROT_T0RecvChar(ucReader, &ucCh);
                                                if ( !ucStatus )
                                                {
                                                        pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen] = ucCh;
                                                        pCurrICCInfo->uiAPDURecvedLen ++;
                                                        uiAPDURecvedLen ++;
                                                }
                                                else
                                                {
                                                        break;
                                                }
                                        }
                                }
                                else
                                {   /*EN: data to card */
                                        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND3;
                                        HALICC_SetTimer(ucReader, 0x61, 0x00, 0x06, 0x00, 1);
                                        continue;
                                }
                                break;
                        }
                        if ( ucCh == ucINSMasked )
                        {
                                pCurrICCInfo->ucAPDUOneByteFlag = 1;
                                if ( pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE2 )
                                {   /*EN: data from card */
                                        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV2;
                                        ucStatus = PROT_T0RecvChar(ucReader, &ucCh);
                                        if ( !ucStatus )
                                        {
                                                pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen] = ucCh;
                                                pCurrICCInfo->uiAPDURecvedLen ++;
                                                uiAPDURecvedLen ++;
                                                if ( uiAPDURecvedLen >= (pCurrICCInfo->aucAPDUSendBuf[4] + 2) )
                                                {
                                                        break;
                                                }
                                                else
                                                {
#ifdef _WIN32
                                                        HALICC_EnableRecv(ucReader);
#endif
                                                        continue;
                                                }
                                        }
                                }
                                else
                                {   /*EN: data to card */
                                        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND3;
                                        HALICC_SetTimer(ucReader, 0x61, 0x00, 0x06, 0x00, 1);
                                        continue;
                                }
                        }
                        if ( ucStatus )
                                break;
                        ucStatus = LIBICCERR_BADPROCBYTE;
                        pCurrICCInfo->ucLastError = LIBICCERR_BADPROCBYTE;
                }
                while ( !ucStatus );
        }

        HALICC_SetTimer(ucReader, 0x00, 0x00, 0x00, 0x00, 1);

        return(ucStatus);
}
