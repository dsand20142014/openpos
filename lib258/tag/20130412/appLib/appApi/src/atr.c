/*
	Library Function for ICC
--------------------------------------------------------------------------
	FILE  atr.c
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

//#include "halicc.h"
#include "libicc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
extern int fd_icc;

unsigned char ATR_ColdReset(unsigned char ucReader,unsigned char ucRate)
{
    unsigned char ucResult;
    ICCINFO *pCurrICCInfo;
    char buf[100], ret;
    long int i=0;

    memset(buf, 0, 100);
    memset( &ICCInfo[ucReader],0,sizeof( ICCINFO ) );
    pCurrICCInfo = &ICCInfo[ucReader];
//	ucCardReader = ucReader;
repeat:
    i=0;
    pCurrICCInfo->ucLastError = 0;
    uiRecvLen = 0;
    uiRecvedLen = 0;
    bFinished = 0;

    ucResult = ISO7816_AsyncReset(ucReader,ucRate);
		Uart_Printf("ISO7816_AsyncReset return %d\n", ucResult);

    /* 等待TO3中断 或 处理其他拔卡等异常中断 */
    do
    {
				//Uart_Printf("********** 0  bTO3=%d \n", bTO3);
        /* 获取中断信息值，现只实现超时、无卡中断 */
        OSICC_RD_INT_STATUS(ucReader);

        if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER00 ) )
            return( LIBICCERR_NOICC );

        if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER02) )
            return( LIBICCERR_NOICC );
        /*
        		if( bRemoveFiscalFlag && ( ucReader == CARDCMD_READER01 ) )
        			return( LIBICCERR_NOICC );

        		if( ( ucReader == CARDCMD_READER00 ) && bAbnoUserFlag  )
        		{
        			pCurrICCInfo->ucLastError = LIBICCERR_VCCPROBLEM;
        			return( LIBICCERR_VCCPROBLEM );
        		}

        		if( ( ucReader == CARDCMD_READER01 ) && bAbnoFiscalFlag )
        			return( LIBICCERR_VCCPROBLEM );

        		if( ( ucReader == CARDCMD_READER02 ) && bAbnoControlFlag )
        			return( LIBICCERR_VCCPROBLEM );
        */
        //if (i++>1000)  goto repeat;
    }
    while ( !bTO3 );
        //Uart_Printf("**********0.5 bTO3=%d bFinished=%d\n", bTO3, bFinished);
    ISO7816_TimeoutProc(ucReader,LIBICC_WWTOUT);

    //接收数据阶段计时
    while ( !bTO3 )
    {
        //Uart_Printf("**********1 bTO3=%d \n", bTO3);
        OSICC_RD_INT_STATUS(ucReader);

        if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER00 ) )
            return( LIBICCERR_NOICC );

        if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER02 ) )
            return( LIBICCERR_NOICC );
        /*
        		if( bRemoveFiscalFlag && ( ucReader == CARDCMD_READER01 ) )
        			return( LIBICCERR_NOICC );
        */
        //Uart_Printf("********** bTO3=%d \n", bTO3);

        ISO7816_ResetRecvCharProc(ucReader);

        if ( bFinished )
        {
            Uart_Printf("finished=%x\n", bFinished);
            break;
        }
    }

    if ( bTO3 )
        ISO7816_TimeoutProc(ucReader,LIBICC_WWTOUT);

    return( pCurrICCInfo->ucLastError );
}

unsigned char ATR_WarmReset(unsigned char ucReader,unsigned char ucRate)					// 热复位
{
    unsigned char ucResult;
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    ucCardReader = ucReader;
    uiRecvLen = 0;
    uiRecvedLen = 0;
    pCurrICCInfo->ucLastError = 0;
    bFinished = 0;
    ucResult = ISO7816_WarmReset(ucReader,ucRate);

    do
    {
        if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER00 ) )
            return( LIBICCERR_NOICC );

        if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER02 ) )
            return( LIBICCERR_NOICC );

        if ( bRemoveFiscalFlag && ( ucReader == CARDCMD_READER01 ) )
            return( LIBICCERR_NOICC );

    }
    while ( !bTO3 );

    ISO7816_TimeoutProc(ucReader,LIBICC_WWTOUT);

    while ( !bTO3 )
    {
        if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER00 ) )
            return( LIBICCERR_NOICC );

        if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER02 ) )
            return( LIBICCERR_NOICC );

        if ( bRemoveFiscalFlag && ( ucReader == CARDCMD_READER01 ) )
            return( LIBICCERR_NOICC );

        ISO7816_ResetRecvCharProc(ucReader);

        if ( bFinished )
            break;
    }

    if ( bTO3 )
        ISO7816_TimeoutProc(ucReader,LIBICC_WWTOUT);

    return( !ucResult );
}

unsigned char ATR_CheckFinished(unsigned char *pucATR,unsigned short ucATRLen)		// 判断是否结束复位应答
{
    unsigned short uiI;
    unsigned short uiATRTotalLen;
    unsigned char ucTDn;
    unsigned char ucTDIndex;
    unsigned char ucNextTDnExist;
    unsigned char ucTCharNB;

    uiI = 0;
    ucTDIndex = 0;

    if ( ucATRLen >= 2 )
    {
        uiI = 1;
        ucTDn = *( pucATR + uiI );
        uiATRTotalLen = ( ucTDn & 0x0F ) + 2;

        do
        {
            ucTDn = *( pucATR + uiI );

            if ( ucTDIndex == 1 )
            {
                /* EN: TCK Exist */
                if ( ( ucTDn & 0x0F ) || ( ucTDn & 0x80 ) )
                    uiATRTotalLen ++;
            }

            ucNextTDnExist = ucTDn & 0x80;
            ucTCharNB = MISC_GetCharBitNB(ucTDn & 0xF0,1);
            uiATRTotalLen += ucTCharNB;

            if ( uiATRTotalLen > ucATRLen )
            {
                if ( uiATRTotalLen == ucATRLen + 1 )
                    return( LIBICCERR_ATRHASLASTCHAR );
                else
                    return( LIBICCERR_ATRNOTFINISHED );
            }
            else
            {
                if ( ( uiATRTotalLen == ucATRLen ) && ( !ucNextTDnExist ) )
                    return( LIBICCERR_SUCCESS );
                else
                {
                    if ( ucNextTDnExist )
                    {
                        uiI += ucTCharNB;
                        ucTDIndex ++;
                    }
                    else
                    {
                        if ( uiATRTotalLen > ucATRLen )
                            return( LIBICCERR_ATRTOOLONG );
                    }
                }
            }
        }
        while ( 1 );
    }
    return( LIBICCERR_ATRNOTFINISHED );
}

unsigned char ATR_Process(unsigned char ucReader)
{
    unsigned short uiI;
    unsigned char ucI;
    unsigned char ucNB;
    unsigned char ucTDn;
    unsigned char ucCh;
    unsigned char ucTA1;
    unsigned char ucTA2;
    unsigned char ucTA3;
    unsigned char ucTB1;
    unsigned char ucTB2;
    unsigned char ucTB3;
    unsigned char ucTC1;
    unsigned char ucTC2;
    unsigned char ucTC3;
    unsigned char ucTD1;
    unsigned char ucMask;
    unsigned char *pucATR;
    unsigned char ucResult;
    unsigned char  ucTCharNB;
    unsigned char  ucTA2Exist;
    unsigned char  ucTD1Exist;
    unsigned char  ucTB1Exist;
    unsigned char  ucProtocol;
    unsigned char  ucNextTDnExist;
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    ucTC1 = 0x00;
    ucTA1 = 0x11;
    ucTD1 = 0x00;
    ucTC2 = 10;
    ucTC3 = 0x00;
    ucTB1 = 0x00;
    ucTB2 = 0x00;
    ucTB3 = 0xFF;
    ucTA3 = 0x20;
    ucTB1Exist = 0;
    ucTD1Exist = 0;
    ucTA2Exist = 0;
    ucProtocol = 0;
    ucResult = LIBICCERR_SUCCESS;
    pucATR = aucATR;

    if ( ucATRLen > 34 )
        return( LIBICCERR_ATRTOOLONG );

    if ( ( *pucATR != 0x3B ) && ( *pucATR != 0x3F ) )
        ucResult = LIBICCERR_BADTS;

    if ( ( !ucResult ) && ( ucATRLen >= 2 ) )
    {
        uiI = 1;

        for ( ucI = 0;ucI < 3;ucI ++ )
        {
            ucTDn = *( pucATR + uiI );
            ucNextTDnExist = ucTDn & 0x80;
            ucTCharNB = MISC_GetCharBitNB(ucTDn & 0xF0,1);

            if ( !ucTCharNB )
                break;

            ucMask = 0x10;
            ucNB = 0;

            do
            {
                if ( ucTDn & ucMask )
                {
                    uiI ++;
                    ucCh = *( pucATR + uiI );

                    switch ( ucNB )
                    {
                    case 0:
                        switch ( ucI )
                        {
                        case 0:
                            ucTA1 = ucCh;
                            break;
                        case 1:
                            ucTA2 = ucCh;
                            ucTA2Exist = 1;
                            break;
                        case 2:
                            if ( ( ucCh == 0xFF ) || ( ucCh <= 0x0F ) )
                            {
                                ucResult = LIBICCERR_BADTA3;
                                break;
                            }
                            else
                                ucTA3 = ucCh;
                            break;
                        default:
                            break;
                        }
                        break;
                    case 1:
                        switch ( ucI )
                        {
                        case 0:
                            ucTB1Exist = 1;
                            if ( ( pCurrICCInfo->ucResetType == LIBICC_COLDRESET ) && ucCh )
                            {
                                ucResult = LIBICCERR_BADTB1;
                                break;
                            }
                            break;
                        case 1:
                            ucTB2 = LIBICCERR_BADTB2;
                            break;
                        case 2:
                            ucTB3 = ucCh;
                            break;
                        default:
                            break;
                        }
                        break;
                    case 2:
                        switch ( ucI )
                        {
                        case 0:
                            ucTC1 = ucCh;
                            break;
                        case 1:
                            if ( ( ucCh == 0x14 ) || ( ucCh == 0x01 ) )
                            {
                                ucTC2 = ucCh;
                                break;
                            }

                            if ( ( ucCh == 0x00 ) || ( ucCh != 0x0A ) )
                            {
                                ucResult = LIBICCERR_BADTC2;
                                break;
                            }

                            ucTC2 = ucCh;
                            break;
                        case 2:
                            if ( ucCh != 0x00 )
                                ucResult = LIBICCERR_BADTC3;
                            else
                                ucTC3 = ucCh;
                            break;
                        default:
                            break;
                        }
                        break;
                    case 3:
                        switch ( ucI )
                        {
                        case 0:
                            ucTD1Exist = 1;
                            ucTD1 = ucCh;

                            if ( ( ( ucCh & 0x0F ) == 0x00 ) || ( ( ucCh & 0x0F ) == 0x01 ) )
                                ucProtocol = ucCh & 0x0F;
                            else
                            {
                                ucResult = LIBICCERR_BADTD1;
                                break;
                            }
                            break;
                        case 1:
                            if ( (  ucCh & 0x0F ) != 0x01 )
                            {
                                if ( ( ( ucCh & 0x0F ) == 0x0E ) || ( ( ucCh & 0x0F ) == 0x0F ) )
                                {
                                    if ( ( ( ucTD1 & 0x0F ) == 0x00 ) && ucTD1Exist )
                                        break;
                                    else
                                        ucResult = LIBICCERR_BADTD2;
                                }
                                else
                                    ucResult = LIBICCERR_BADTD2;
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                }
                ucMask = ucMask << 1;
                ucNB ++;

                if ( ucResult )
                    break;

            }
            while ( ucMask );

            if ( ucResult )
                break;

            if ( !ucNextTDnExist )
                break;
        }
    }
    /*
    	if( !ucResult )
    	{
    		if( ( !ucTB1Exist ) && ( pCurrICCInfo->ucResetType == LIBICC_COLDRESET ) )
    		ucResult = LIBICCERR_BADTB1;
    	}*/

    /* EN: Check TCK */
    if ( ( !ucResult ) && ucTD1Exist && ( ( ucTD1 & 0x0F ) || ( ucTD1 & 0x80 ) ) )
//	if( ( !ucResult ) && ( ucProtocol ) )
    {
        ucCh = 0x00;

        for ( ucI = 1;ucI < ucATRLen;ucI ++ )
            ucCh = ucCh ^ aucATR[ucI];

        if ( ucCh )
        {
            pCurrICCInfo->ucLastError = LIBICCERR_BADTCK;
            ucResult = LIBICCERR_BADTCK;
        }
    }

    if ( !ucResult )
    {
        switch ( ucTA1 & 0x0F )
        {
        case 1:
            pCurrICCInfo->ucDi = 1;
            break;
        case 2:
            pCurrICCInfo->ucDi = 2;
            break;
        case 3:
//	    	    pCurrICCInfo->ucDi = 3;
//				break;
        case 4:
            pCurrICCInfo->ucDi = 4;
            break;
        case 8:
            pCurrICCInfo->ucDi = 8;
            break;
        default:
            ucResult = LIBICCERR_BADTA1;
            break;
        }

        if ( !ucResult )
        {
//	  		if( ( ucTA1 & 0xF0 ) != 0x10 )
//   			ucResult = LIBICCERR_BADTA1;
        }
    }

    if ( !ucResult )
    {
        if ( !ucTA2Exist )
            pCurrICCInfo->ucDi = 1;

        if ( ucTA2Exist )
        {
            if ( ucTA2 & 0x10 )
                ucResult = LIBICCERR_BADTA2;
        }
    }

    if ( !ucResult )
    {
        pCurrICCInfo->ucProtocol = ucProtocol;
        pCurrICCInfo->ucGTR = ucTC1;
        pCurrICCInfo->ucFi = ( ucTA1 & 0xF0 ) >> 4;

        if ( ucTC2 )
        {
            pCurrICCInfo->ulWWT = 960 * ucTC2 * pCurrICCInfo->ucDi + 480 * pCurrICCInfo->ucDi;
            pCurrICCInfo->ulWWT += 1;
        }
        else
            ucResult = LIBICCERR_BADTC2;
    }

    if ( !ucResult )
    {
        if ( ucProtocol )
        {
            ucCh = ucTB3 & 0x0F;

            if ( ucCh > 5 )
                ucResult = LIBICCERR_BADTB3;

            if ( !ucResult )
            {
                ucCh = ( ucTB3 & 0xF0 ) >> 4;

                if ( ucCh > 4 )
                    ucResult = LIBICCERR_BADTB3;

                if ( !ucResult )
                {
                    ucCh = ( ucTB3 & 0xF0 ) >> 4;

                    if ( ucCh > 4 )
                        ucResult = LIBICCERR_BADTB3;
                }

                if ( !ucResult )
                {
                    if ( ( ucTC1 == 0x00 ) && ( ucTB3 == 0x00 ) )
                        ucResult = LIBICCERR_BADTB3;
                    else
                    {
                        if ( ( ucTC1 > 0x1E ) && ( ucTC1 != 0xEF ) )
                            ucResult = LIBICCERR_BADTB3;
                        else
                        {
                            ucCh = 1;

                            for ( uiI = 0;uiI < ( ucTB3 & 0x0F );uiI ++ )
                                ucCh *= 2;

                            ucCh += 11;
                            if ( ucTC1 == 0xFF )
                            {
                                if ( ucCh <= 11 )
                                    ucResult = LIBICCERR_BADTB3;
                            }
                            else
                            {
                                if ( ucCh <= ( ucTC1 + 1 ) )
                                    ucResult = LIBICCERR_BADTB3;
                            }
                        }
                    }
                }
            }
        }
    }

    OSICC_SetProtocol(ucReader,ucProtocol);
    OSICC_SetGuardTime(ucReader,pCurrICCInfo->ucGTR);

#if 0
    if ( pCurrICCInfo->ucRate == 1 )
        OSICC_SetDi(ucReader,pCurrICCInfo->ucDi);
    else
        OSICC_SetDi(ucReader,pCurrICCInfo->ucRate);
#else
    OSICC_SetDi(ucReader,pCurrICCInfo->ucDi);
#endif

    /* EN: Set TDA8007 register*/
    if ( ( !ucResult ) && ( !ucProtocol ) )
    {
        pCurrICCInfo->uiCWT = 1;
        ucCh = ucTB3 & 0x0F;

        for ( uiI = 0;uiI < ucCh;uiI ++ )
            pCurrICCInfo->uiCWT *= 2;

        pCurrICCInfo->uiCWT += 11;
        pCurrICCInfo->uiCWT += 4;
        pCurrICCInfo->ulBWT = 1;
        ucCh = ( ucTB3 & 0xF0 ) >> 4;

        for ( uiI = 0;uiI < ucCh;uiI ++ )
            pCurrICCInfo->ulBWT *= 2;

        pCurrICCInfo->ulBWT *= 960;
        pCurrICCInfo->ulBWT *= pCurrICCInfo->ucDi;
        pCurrICCInfo->ulBWT += 11;
        pCurrICCInfo->ulBWT += pCurrICCInfo->ucDi * 960;
        pCurrICCInfo->ucWTX = 1;
    }
    else
        pCurrICCInfo->ucLastError = ucResult;

    return( LIBICCERR_SUCCESS );
}

unsigned char ATR_FinishedProc(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    switch ( pCurrICCInfo->ucLastError )
    {
    case LIBICCERR_ATRTOOLONG:
    case LIBICCERR_BADTA1:
    case LIBICCERR_BADTB1:
    case LIBICCERR_BADTD1:
    case LIBICCERR_BADTA2:
    case LIBICCERR_BADTB2:
    case LIBICCERR_BADTC2:
    case LIBICCERR_BADTD2:
    case LIBICCERR_BADTA3:
    case LIBICCERR_BADTB3:
    case LIBICCERR_BADTC3:
        if ( pCurrICCInfo->ucResetType == LIBICC_COLDRESET )
            ucCardRecvStatus = LIBICCERR_NEEDWARMRESET;
        else
            ucCardRecvStatus = pCurrICCInfo->ucLastError;
        break;
    case LIBICCERR_SUCCESS:
        ucCardRecvStatus = LIBICCERR_ISASYNCCARD;
        break;
    default:
        ucCardRecvStatus = pCurrICCInfo->ucLastError;
        break;
    }

    bFinished = 1;
    return( pCurrICCInfo->ucLastError );
}
#if 0 //comment by liuguoming,since this function is realized in libmisc.c
unsigned char MISC_GetCharBitNB(unsigned char ucValue,char ucBit)
{
    unsigned char ucCount;
    unsigned char i;

    for ( i = 1,ucCount = 0;i != 0;i = i << 1 )
    {
        if ( ucBit )
        {
            if ( ucValue & i )
                ucCount ++;
        }
        else
        {
            if ( !( ucValue & i ) )
                ucCount ++;
        }
    }

    return( ucCount );
}
#endif



















