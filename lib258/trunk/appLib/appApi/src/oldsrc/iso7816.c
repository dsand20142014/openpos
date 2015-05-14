/*
	Library Function for ICC
--------------------------------------------------------------------------
	FILE  iso7816.c
--------------------------------------------------------------------------
    INTRODUCTION
    Created :		2003-02-22		Xiaoxi Jiang
    Last modified :	2003-02-22		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/
/*
#include <halicc.h>
#include <tools.h>
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "s3c2410_tda8007.h"
#include "osicc.h"
#include "libicc.h"

extern int fd_icc;
#define ISO_DBG(args...) Uart_Printf(args)

unsigned char ISO7816_AsyncReset(unsigned char ucReader,unsigned char ucRate)
{
    ICCINFO *pCurrICCInfo;
    unsigned char ucResult, i, val, addr;


    pCurrICCInfo = &ICCInfo[ucReader];
    ucResult = OSICC_ResetUart(ucReader);
    // pCurrICCInfo->ucRate = ucRate;


    OSICC_RD_REG(ucReader, REG_USR);
    OSICC_RD_REG(ucReader, REG_HSR);

    if ( !ucResult )
    {
        ucATRLen = 0;

        pCurrICCInfo->ucStatus = ICCSTATUS_POWERED;
        pCurrICCInfo->ucResetType = LIBICC_COLDRESET;
        pCurrICCInfo->ucSession = ICCSESSION_COLDRESET;
        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET1;

        memset( aucATR,0,sizeof( aucATR ) );

        OSICC_SetClock(ucReader,0,1);
        OSICC_SetMethod(ucReader,ICCMETHOD_ASYNC);
        OSICC_SetBaud(ucReader,1);
        OSICC_SetDi(ucReader, 1);
        OSICC_SetProtocol(ucReader,1);
        OSICC_SetConvention(ucReader,1);
        OSICC_SetPowerType(ucReader,HALICC_POWER5V);//pCurrICCInfo->ucPowerType);
        OSICC_SetPEC(ucReader,0);
        OSICC_Active(ucReader);
        OSICC_SetWaitTime(ucReader,0x61,0xFF, 0x00,0x6C,0x00);
        return( ucResult );
    }


    return( ucResult );
}



unsigned char ISO7816_WarmReset(unsigned char ucReader,unsigned char ucRate)
{
    ICCINFO *pCurrICCInfo;
    unsigned char ucResult;

    pCurrICCInfo = &ICCInfo[ucReader];
    ucResult = OSICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,NULL);
    // pCurrICCInfo->ucRate = ucRate;

    if ( !ucResult )
    {
        ucATRLen = 0;
        pCurrICCInfo->ucStatus = ICCSTATUS_POWERED;
        pCurrICCInfo->ucResetType = LIBICC_WARMRESET;
        pCurrICCInfo->ucSession = ICCSESSION_COLDRESET;
        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET1;

        OSICC_SetBaud(ucReader,4);
        OSICC_SetDi(ucReader,1);
        OSICC_SetProtocol(ucReader,1);
        OSICC_SetConvention(ucReader,1);
        OSICC_SetPEC(ucReader,0);
        OSICC_SetWaitTime(ucReader,0x61,0x00,0x00,0x6C,0x00);
        return( ucResult );
    }

//	OSICC_Deactive(ucReader);
//	pCurrICCInfo->ucSession = ICCSESSION_IDLE;
    return( ucResult );
}

void ISO7816_ResetRecvCharProc(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;
    unsigned char ucResult;
    char c, i;

    pCurrICCInfo = &ICCInfo[ucReader];

    //if ( uiRecvLen != uiRecvedLen )//未全部读取内核数据时,Linux版本设计为一次读取
    {
        switch ( pCurrICCInfo->ucSessionStep )
        {
        case ICCSESSIONSTEP_COLDRESET1:
            break;
        case ICCSESSIONSTEP_COLDRESET2:
        case ICCSESSIONSTEP_COLDRESET3:
            if ( ucATRLen < LIBICC_MAXATRBUF )
            {
                uiRecvLen += read(fd_icc, &aucATR[ucATRLen], LIBICC_MAXATRBUF);
                ucATRLen += uiRecvLen;
            }

            ucResult = ATR_CheckFinished(aucATR,ucATRLen);

            if ( ( ucResult != LIBICCERR_ATRNOTFINISHED ) && ( ucResult != LIBICCERR_ATRHASLASTCHAR ) )
            {
#if 0
                /****************** DEBUG *********************************************/
                Uart_Printf("%s:ucATRLen=%d uiRecvLen=%d\n", __FUNCTION__,ucATRLen, uiRecvLen);
                for (i=0; i<ucATRLen; i++)
                    Uart_Printf("[%d]=%x\t", i, aucATR[i]);
                /****************** DEBUG *********************************************/
#endif
                OSICC_SetWaitTime(ucReader,0x00,0x00,0x00,0x00,0x00);
                pCurrICCInfo->ucLastError = ucResult;
                ATR_Process(ucReader);
                ATR_FinishedProc(ucReader);
            }
            else
            {
                OSICC_SetWaitTime(ucReader,0x61,0x00,0x00,0x80,0x25);
                pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_COLDRESET3;
            }
            break;
        default:
            Uart_Printf("%s, protocol:%d  not implemented!\n", __FUNCTION__, pCurrICCInfo->ucProtocol);
            break;
        }
    }
}

void ISO7816_TimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    bTO3 = 0;

    switch ( pCurrICCInfo->ucSession )
    {
    case ICCSESSION_COLDRESET:
    case ICCSESSION_WARMRESET:
        ISO7816_ResetTimeoutProc(ucReader,ucTimeoutType);
        break;
    default:
        break;
    }
}

void ISO7816_ResetTimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType)
{
    switch ( ucTimeoutType )
    {
    case 0:
    case 1:
    case 2:
    default:
        ISO_DBG("%s ICCInfo[ucReader].ucSessionStep=%x\n",__FUNCTION__, ICCInfo[ucReader].ucSessionStep);
        switch ( ICCInfo[ucReader].ucSessionStep )
        {
        case ICCSESSIONSTEP_COLDRESET1:
            OSICC_HandleContact(ucReader,HALICCCONTACT_SET,HALICCCONTACT_RST,NULL);

            ICCInfo[ucReader].ucSessionStep = ICCSESSIONSTEP_COLDRESET2;
            OSICC_SetWaitTime(ucReader,0x61,0x00,0x00,0x78,0xff);

//			        OSICC_SetWaitTime(ucReader,0x61,0x00,0x00,0x78,0x00);
            return;
        case ICCSESSIONSTEP_COLDRESET2:
            OSICC_SetWaitTime(ucReader,0x00,0x00,0x00,0x00,0x00);
            ICCInfo[ucReader].ucLastError = LIBICCERR_ISSYNCCARD;
            OSICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,NULL);
            break;
        case ICCSESSIONSTEP_COLDRESET3:
            OSICC_SetWaitTime(ucReader,0x00,0x00,0x00,0x00,0x00);
            ICCInfo[ucReader].ucLastError = LIBICCERR_CARDMUTE;
            OSICC_HandleContact(ucReader,HALICCCONTACT_CLR,HALICCCONTACT_RST,NULL);
            break;
        default:
            break;
        }
        break;
    }

    ATR_FinishedProc(ucReader);
}

unsigned char ISO7816_SetAPDUNextCommand(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    // EN: Get Response
#if 0
    if ( aucAPDURecvBuf[uiAPDURecvedLen - 2] == 0x61 )
    {
        aucCardSendBuf[0] = 0x00;
        aucCardSendBuf[1] = 0xC0;
        aucCardSendBuf[2] = 0x00;
        aucCardSendBuf[3] = 0x00;
        aucCardSendBuf[4] = aucAPDURecvBuf[uiAPDURecvedLen - 1];
        uiAPDUSendLen = 5;
        uiAPDUSentLen = 0;
        uiAPDURecvedLen -= 2;
        ucAPDUCase = ICCAPDU_CASE2;
        return( LIBICCERR_SUCCESS );
    }
#endif

    if ( ( ucAPDUCase == ICCAPDU_CASE4 ) && ( uiAPDUSentLen > 5 )
            && ( ( ( aucAPDURecvBuf[uiAPDURecvedLen - 2] == 0x9F ) &&( aucAPDURecvBuf[uiAPDURecvedLen - 1] == 0xFF ) )
                 || ( ( aucAPDURecvBuf[uiAPDURecvedLen - 2] == 0x62 ) && ( aucAPDURecvBuf[uiAPDURecvedLen - 1] == 0x83 ) )
                 || ( ( aucAPDURecvBuf[uiAPDURecvedLen - 2] == 0x63 ) && ( aucAPDURecvBuf[uiAPDURecvedLen - 1] == 0x35 ) ) ) )
    {
        aucCardSendBuf[0] = 0x00;
        aucCardSendBuf[1] = 0xC0;
        aucCardSendBuf[2] = 0x00;
        aucCardSendBuf[3] = 0x00;
        aucCardSendBuf[4] = 0x00;
        uiAPDUSendLen = 5;
        uiAPDUSentLen = 0;
        uiAPDURecvedLen -= 2;
        ucAPDUCase = ICCAPDU_CASE2;
        return( LIBICCERR_SUCCESS );
    }

    if ( aucCardRecvBuf[0] == 0x6C )
    {
        aucCardSendBuf[4] = aucAPDURecvBuf[1];
        uiAPDUSentLen = 0;
        uiAPDURecvedLen = 0;
        uiAPDURecvLen = 0;
        memset( aucAPDURecvBuf,0,LIBICC_MAXCOMMBUF );
        return( LIBICCERR_SUCCESS );
    }

//	if( aucAPDURecvBuf[0] == aucAPDURecvBuf[1] )
    if ( ( aucAPDURecvBuf[0] == aucCardSendBuf[1] ) && ( uiAPDURecvedLen == 1 ) )
        return( LIBICCERR_SUCCESS );

    return( LIBICCERR_APDUNONEXTCOMMAND );
}



