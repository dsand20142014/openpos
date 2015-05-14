/*
    Library Function for ICC
--------------------------------------------------------------------------
    FILE  protocol.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :       2003-03-06      Xiaoxi Jiang
    Last modified : 2003-03-06      Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "libicc.h"
extern int fd_icc;

#define PROT_DBG(args...) Uart_Printf(args)

unsigned char PROT_T0Command(unsigned char ucReader);

unsigned char PROT_AsyncCommand(unsigned char ucReader)
{
    unsigned char ucResult;
    ICCINFO *pCurrICCInfo;
    ucCardReader = ucReader;
    pCurrICCInfo = &ICCInfo[ucReader];
    uiRecvedLen = 0;
    uiRecvLen = 0;
    uiAPDUSendLen = 0;
    uiAPDUSentLen = 0;
    uiAPDURecvLen = 0;
    uiAPDURecvedLen = 0;
    uiAPDUSendLen = ucCardSendLen;

    //Uart_Printf("%s ucCardSendLen=%#x\n", __FUNCTION__, ucCardSendLen);
    switch ( uiAPDUSendLen )
    {
    case 4:
        ucAPDUCase = ICCAPDU_CASE1;
        break;
    case 5:
        if ( ucAPDUCase != ICCAPDU_CASE0 )
            ucAPDUCase = ICCAPDU_CASE2;
        else
            ucAPDUCase = ICCAPDU_CASE1;
        break;
    default:
        if ( uiAPDUSendLen == ( 5 + aucCardSendBuf[4] ) )
            ucAPDUCase = ICCAPDU_CASE3;
        else
        {
            if ( ( uiAPDUSendLen == ( 5 + aucCardSendBuf[4] + 1 ) )
                    || ( (  aucCardSendBuf[uiAPDUSendLen] == 0 ) && ( uiAPDUSendLen == ( 5 + aucCardSendBuf[4] ) ) ) )
                ucAPDUCase = ICCAPDU_CASE4;
            else
                return( LIBICCERR_PROTOCOLPARAM );
        }
        break;
    }

    if ( ( ucAPDUCase == ICCAPDU_CASE1 ) && ( uiAPDUSendLen == 4 ) )
    {
        uiAPDUSendLen = 5;
        aucCardSendBuf[5] = aucCardSendBuf[4];
        aucCardSendBuf[4] = 0;
    }
//   Uart_Printf("%s ucAPDUCase=%#x\n", __FUNCTION__, ucAPDUCase);
    pCurrICCInfo->ucLastError = LIBICCERR_SUCCESS;

    if ( ucAPDUCase == ICCAPDU_CASE4 )
        uiAPDUSendLen = 5 + aucCardSendBuf[4];

    ucResult = OSICC_ChangeProtocol(ucReader);
    return( PROT_T0Command(ucReader) );
}

unsigned char PROT_T0Command(unsigned char ucReader)
{
    unsigned char ucResult;
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    //Uart_Printf("%s pCurrICCInfo->ucStatus=%#x\n", __FUNCTION__, pCurrICCInfo->ucStatus);

    switch ( pCurrICCInfo->ucStatus )
    {
    case ICCSTATUS_INSERT:
        pCurrICCInfo->ucLastError = LIBICCERR_NOTPOWERED;
        break;
    case ICCSTATUS_REMOVE:
        pCurrICCInfo->ucLastError = LIBICCERR_CARDREMOVE;
        break;
    case ICCSTATUS_POWERED:
        break;
    case ICCSTATUS_ISSYNCCARD:
        pCurrICCInfo->ucLastError = LIBICCERR_ISSYNCCARD;
        break;
    default:
        pCurrICCInfo->ucLastError = LIBICCERR_UNKNOWNSTATUS;
        break;
    }
    /* EN: Send T0 APDU */
    pCurrICCInfo->ucSessionStep = 3;
    //Uart_Printf("%s pCurrICCInfo->ucLastError=%#x\n", __FUNCTION__, pCurrICCInfo->ucLastError);

#if 0 //zy
    if (pCurrICCInfo->ucLastError!=LIBICCERR_SUCCESS)
    {
        pCurrICCInfo->ucLastError=LIBICCERR_SUCCESS;
        Uart_Printf("%s make a fake Error!\n", __FUNCTION__);
    }
#endif

    while ( !pCurrICCInfo->ucLastError )
    {
        pCurrICCInfo->ucSessionStep ++;
        ucResult = PROT_T0SendReceive(ucReader);

        if ( ucResult )
            return( ucResult );

        if ( !pCurrICCInfo->ucLastError )
        {
            if ( !ISO7816_SetAPDUNextCommand(ucReader) )
                continue;
        }

        break;
    }

    PROT_T0FinishedProc(ucReader);
    return( pCurrICCInfo->ucLastError );
}

unsigned char PROT_T0SendReceive(unsigned char ucReader)
{
    unsigned char  ucCh;
    unsigned short  uiI, i;
    unsigned char  ucLen;
    unsigned char  ucCLA;
    unsigned char  ucINS;
    unsigned char ucResult;
    unsigned char  ucINSMasked;
    unsigned short  usAPDURecvedLen;
//    unsigned char data aucTimerValue[4];
    ICCINFO *pCurrICCInfo;
    int counts=10000;

    pCurrICCInfo = &ICCInfo[ucReader];
    uiAPDUSentLen = 0;
    bFinished = 0;
    usAPDURecvedLen = 0;
    pCurrICCInfo->ucSession = ICCSESSION_DATAEXCH;
    uiRecvLen = 0;
    uiRecvedLen = 0;
    uiAPDURecvLen = 0;
    uiAPDURecvedLen = 0;
    ucAPDUOneByteFlag = 0;
    OSICC_SetPEC(ucReader,4);
    ucCLA = aucCardSendBuf[0];
    ucINS = aucCardSendBuf[1];
    ucINSMasked = ~aucCardSendBuf[1];
    ucAPDUOneByteFlag = 0;

    OSICC_WR_GLOBAL_VAR(ucReader);

    ucResult = OSICC_EnableSend(ucReader);
    sleep(1);

    do
    {
        OSICC_RD_GLOBAL_VAR(ucReader);
        ucResult = PROT_CallbackT0SendChar(ucReader);

#if 0 //zy
        if ( ucResult )
            return( ucResult );

        if ( pCurrICCInfo->ucLastError )
            return( pCurrICCInfo->ucLastError );
#endif
    }
    while ( !bFinished );

    OSICC_SetTimer(ucReader,0x61,0xFF,0xFF,0xFF);
    /*
        for (i=0; i<uiAPDUSendLen; i++)
        {
            Uart_Printf("%#02x ", aucCardSendBuf[i]);
        }
        Uart_Printf("\n");

        Uart_Printf("%s send ok, ucSessionStep=%d uiAPDUSentLen=%d\n\n\n",
               __FUNCTION__, ICCInfo[ucReader].ucSessionStep, uiAPDUSentLen );
    */

    //处理过程字节 0x61.0x13
    do
    {
        OSICC_RD_GLOBAL_VAR(ucReader);

        ucResult = PROT_T0RecvChar(ucReader,&ucCh);
        //PROT_DBG("====>!!! RCV: %#x, ucResult=%d\n",ucCh, ucResult);
        if ( ucResult )
        {
            Uart_Printf("%s ucResult=%x\n", __FUNCTION__, ucResult);
            break;
        }

        if ( ucCh == 0x6F )
        {
            aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
            uiAPDURecvedLen ++;
            usAPDURecvedLen ++;
            ucResult = PROT_T0RecvChar(ucReader,&ucCh);

            if ( !ucResult )
            {
                aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
                ucLen = ucCh;
                uiAPDURecvedLen ++;
                usAPDURecvedLen ++;
            }

            if ( ucCh != 0 )
            {
                for ( uiI = 0;uiI < ucLen + 2;uiI ++ )
                {
                    ucResult = PROT_T0RecvChar(ucReader,&ucCh);

                    if ( !ucResult )
                    {
                        aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
                        uiAPDURecvedLen ++;
                        usAPDURecvedLen ++;
                    }
                    else
                        break;
                }
                break;
            }
            else
                break;
        }//ucCh=0x6F

        if ( ucCh == 0x60 )
        {
            /* EN: In TDA8007, don't need to reset timer because timer has been restart */
            /*			uiAPDURecvedLen = 0;
            			usAPDURecvedLen = 0;
            			uiRecvLen = 0;
            			uiRecvedLen = 0;
            			uiAPDURecvLen = 0;*/
            continue;
        }//ucCh=0x60

        if ( ( ( ucCh & 0xF0 ) == 0x60 ) || ( ( ucCh & 0xF0 ) == 0x90 ) )
        {
            ucAPDUOneByteFlag = 0;
            aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
            uiAPDURecvedLen ++;
            usAPDURecvedLen ++;
// 			HALICC_SetTimer(ucReader,0x61,0x00,0x06,0x00);
            // Uart_Printf("ucCh=%x, wait for SW2\n", ucCh);
            ucResult = PROT_T0RecvChar(ucReader,&ucCh);

            if ( !ucResult )
            {
                aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
                uiAPDURecvedLen ++;
                usAPDURecvedLen ++;
            }
            break;
        }//( ucCh & 0xF0 ) == 0x60 ) || ( ( ucCh & 0xF0 ) == 0x90 )

        if ( ucCh == ucINS )
        {
            ucAPDUOneByteFlag = 0;
            //PROT_DBG("ucCh==ucINS, ucADPUCase=%x ucSessionStep=%x\n", ucCh,ucAPDUCase,  pCurrICCInfo->ucSessionStep);
            if ( ucAPDUCase == ICCAPDU_CASE2 )
            {
                /* EN: data from card */
                pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV2;
#if 1
                for ( uiI = usAPDURecvedLen;uiI < aucCardSendBuf[4];uiI ++ )
                {
                    ucResult = PROT_T0RecvChar(ucReader,&ucCh);

                    if ( !ucResult )
                    {
                        aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
                        uiAPDURecvedLen ++;
                        usAPDURecvedLen ++;
                        //PROT_DBG("%s RCV: %#x\n", __FUNCTION__, ucCh);
                    }
                    else
                        break;
                }
#endif
                continue;
            }
            else
            {
                /* EN: data to card */
                if ( pCurrICCInfo->ucSessionStep == ICCSESSIONSTEP_DATAEXCHRECV2 )
                {
//	            	aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
//	            	uiAPDURecvedLen ++;
                    continue;
                }

                if ( pCurrICCInfo->ucSessionStep == ICCSESSIONSTEP_DATAEXCHRECV3 )
                {
                    OSICC_EnableRecv(ucReader);
                    pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV4;

                    for ( uiI = 0;uiI < ( aucCardSendBuf[4] + 2 );uiI ++ )
                    {
                        ucResult = PROT_T0RecvChar(ucReader,&ucCh);

                        if ( !ucResult )
                        {
                            aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
                            uiAPDURecvedLen ++;
                            //Uart_Printf("%s RCV: %#x\n", __FUNCTION__, ucCh);
                        }
                        else
                            break;
                    }
                }
                else
                {
                    aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
                    uiAPDURecvedLen ++;
                    usAPDURecvedLen ++;
//					HALICC_SetWaitTime(ucReader,0x61,0x00,0x00,0x06,0x00);
//					continue;
//					pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND2;
                }
                break;
            }
        }//ucCh == ucINS

        if ( ucCh == ucINSMasked )
        {
            ucAPDUOneByteFlag = 1;

            if ( ucAPDUCase == ICCAPDU_CASE2 )
            {
                /* EN: data from card */
                pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV2;
                ucResult = PROT_T0RecvChar(ucReader,&ucCh);

                uiRecvLen --;
                uiRecvedLen --;

                if ( !ucResult )
                {
                    aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
                    uiAPDURecvedLen ++;
                    usAPDURecvedLen ++;

                    if ( usAPDURecvedLen < ( aucCardSendBuf[4] + 2 ) )
                    {
                        continue;
                    }
                    else
                        break;
                }
            }
            else
            {
                /* EN: data to card */
                ucAPDUOneByteFlag = 1;
//				uiAPDUSentLen --;
//				pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND2;
                ucResult = PROT_CallbackT0SendChar(ucReader);
//	        	ucResult = PROT_T0RecvChar(ucReader,&ucCh);
                continue;
                /*
                				if( pCurrICCInfo->ucSessionStep == ICCSESSIONSTEP_DATAEXCHRECV3 )
                				{
                		   			HALICC_EnableCommunicate(ucReader,0);
                					pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV4;

                	                for( uiI = 0;uiI < ( aucCardSendBuf[4] + 2 );uiI ++ )
                	                {
                	                    ucResult = PROT_T0RecvChar(ucReader,&ucCh);

                	                    if( !ucResult )
                	                    {
                	                        aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
                	                        uiAPDURecvedLen ++;
                	                    }else
                	                        break;
                	                }
                				}
                				else
                				{
                					aucAPDURecvBuf[uiAPDURecvedLen] = ucCh;
                					uiAPDURecvedLen ++;
                					usAPDURecvedLen ++;
                					HALICC_SetWaitTime(ucReader,0x61,0x00,0x00,0x06,0x00);
                				}*/
            }
        }//ucCh == ucINSMasked

        Uart_Printf("!!! LIBICCERR_BADPROCBYTE\n");
        ucResult = LIBICCERR_BADPROCBYTE;
        pCurrICCInfo->ucLastError = LIBICCERR_BADPROCBYTE;
        break;
    }
    while ( !ucResult );



    OSICC_SetWaitTime(ucReader,0x00,0x00,0x00,0x00,0x00);
    return( ucResult );
}

unsigned char PROT_T0RecvChar(unsigned char ucReader,unsigned char *pucCh)
{
    unsigned short  uiTimeout;
    int ret;

#if 0
    if ( ICCInfo[ucReader].ucDi != 4 )
    {
        uiTimeout = 2000;
        Os__timer_start(&uiTimeout);
    }
#endif

    // Uart_Printf("%s ucLastError=%#x\n", __FUNCTION__, ICCInfo[ucReader].ucLastError);
    while ( !ICCInfo[ucReader].ucLastError )
    {
#if 0
        if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER00 ) )
            return( LIBICCERR_NOICC );

        if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER02 ) )
            return( LIBICCERR_NOICC );

        if ( bRemoveFiscalFlag && ( ucReader == CARDCMD_READER01 ) )
            return( LIBICCERR_NOICC );
#endif


        /*
                if ( uiRecvLen != uiRecvedLen )
                {
                    if ( pucCh )
                        *pucCh = aucCardRecvBuf[uiRecvLen];

                    uiRecvLen ++;

                    if ( uiRecvLen >= LIBICC_MAXCOMMBUF )
                        uiRecvLen = 0;
                    break;
                }
        */
        OSICC_RD_GLOBAL_VAR(ucReader);
        //Uart_Printf("%s uiRecvLen=%d uiRecvedLen=%d\n", __FUNCTION__, uiRecvLen, uiRecvedLen);
repeat:
        if (uiRecvLen!=uiRecvedLen)
        {
            ret=read(fd_icc, pucCh, 1);
            aucCardRecvBuf[uiRecvLen]=*pucCh;
            uiRecvLen+=ret;
            if ( uiRecvLen >= LIBICC_MAXCOMMBUF )
                uiRecvLen = 0;

            if (ret>0)
            {
#if 0
                Uart_Printf("%s readbuf ret=%d uiRecvLen=%d *pucCh=%x, lastError=%d\n",
                       __FUNCTION__, ret, uiRecvLen, *pucCh, ICCInfo[ucReader].ucLastError);
#endif
                break;
            }
            else goto repeat;

        }


        if ( ICCInfo[ucReader].ucDi != 4 )
        {
            if ( !uiTimeout )
            {
//				Os__timer_stop(&uiTimeout);

                if ( ucReader == CARDCMD_READER01 )
                    return( LIBICCERR_CARDMUTE );

                if ( ucReader == CARDCMD_READER00 )
                    return( LIBICCERR_CARDMUTE );

                if ( ucReader == CARDCMD_READER02 )
                    return( LIBICCERR_CARDMUTE );
            }
        }
    }
#if 0
    if ( ICCInfo[ucReader].ucDi != 4 )
        Os__timer_stop(&uiTimeout);
#endif
    return( ICCInfo[ucReader].ucLastError );
}

unsigned char PROT_T0FinishedProc(unsigned char ucReader)
{
    unsigned char ucLen;
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    pCurrICCInfo->ucSession = ICCSESSION_IDLE;
    pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_IDLE;

    //PROT_DBG("%s \n", __FUNCTION__);

    if ( pCurrICCInfo->ucLastError )
        ucCardRecvStatus = pCurrICCInfo->ucLastError;
    else
        ucCardRecvStatus = LIBICCERR_EXECUTEOK;

    ucLen = 0;

    if ( !uiAPDURecvLen )
        uiAPDURecvLen = uiAPDURecvedLen;

    if ( pCurrICCInfo->ucLastError )
        return( pCurrICCInfo->ucLastError );

    if ( ( aucCardRecvBuf[uiRecvedLen - 2] & 0xF0 ) == 0x90 )
    {
        if ( ( aucCardRecvBuf[uiRecvedLen - 1] & 0xF0 ) != 0 )
        {
            pCurrICCInfo->ucLastError = LIBICCERR_BADPROCBYTE;
//			return( pCurrICCInfo->ucLastError );
        }
    }
    if ( uiAPDURecvLen <= LIBICC_MAXCOMMBUF - 2 )
    {
        memcpy( aucCardRecvBuf,aucAPDURecvBuf,uiAPDURecvedLen );
        ucCardRecvLen = uiAPDURecvedLen;
    }
    return( pCurrICCInfo->ucLastError );
}



unsigned char PROT_CallbackT0SendChar(unsigned char ucReader)
{
    unsigned short uiTimeout;
#if 0
    if ( ICCInfo[ucReader].ucDi != 4 )
    {
        uiTimeout = 2000;
        Os__timer_start(&uiTimeout);
    }
#endif

    bSentByte = 0;

    if ( ucAPDUOneByteFlag ) //only one
    {
        OSICC_EnableRecvAfterSend(ucReader);
        OSICC_EnableSend(ucReader);
    }

    //Uart_Printf("%s ucGTR=%#x\n", __FUNCTION__, ICCInfo[ucReader].ucGTR);
    if ( ICCInfo[ucReader].ucGTR == 0 )
    {
        //Uart_Printf("%s ICCInfo[ucReader].ucSessionStep=%x uiAPDUSentLen=%d\n", __FUNCTION__, ICCInfo[ucReader].ucSessionStep, uiAPDUSentLen);
        switch ( ICCInfo[ucReader].ucSessionStep )
        {
        case ICCSESSIONSTEP_DATAEXCHSEND1:
            if ( uiAPDUSentLen == 4 )
            {
                ICCInfo[ucReader].ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV1;
                OSICC_EnableRecvAfterSend(ucReader);
                bFinished = 1;
            }
            break;

        case ICCSESSIONSTEP_DATAEXCHSEND2:
            if ( uiAPDUSentLen == ( uiAPDUSendLen - 1 ) )
            {
                //				HALICC_EnableCommunicate(ucReader,1);
                ICCInfo[ucReader].ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV2;
                OSICC_EnableRecvAfterSend(ucReader);
                bFinished = 1;
            }
            break;
            //		case ICCSESSIONSTEP_DATAEXCHSEND3:
        default:
            if ( uiAPDUSentLen == 4 )
            {
                ICCInfo[ucReader].ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV3;
                OSICC_EnableRecvAfterSend(ucReader);
                bFinished = 1;
            }
            break;
        }
    }

    do
    {
        /*
                if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER00 ) )
                    return( LIBICCERR_NOICC );

                if ( bRemoveCardFlag && ( ucReader == CARDCMD_READER02 ) )
                    return( LIBICCERR_NOICC );

                if ( bRemoveFiscalFlag && ( ucReader == CARDCMD_READER01 ) )
                    return( LIBICCERR_NOICC );

                if ( ICCInfo[ucReader].ucDi != 4 )
                {
                    if ( !uiTimeout )
                    {
        //			Os__timer_stop(&uiTimeout);

                        if ( ucReader == CARDCMD_READER01 )
                            return( LIBICCERR_CARDMUTE );

                        if ( ucReader == CARDCMD_READER00 )
                            return( LIBICCERR_CARDMUTE );

                        if ( ucReader == CARDCMD_READER02 )
                            return( LIBICCERR_CARDMUTE );
                    }
                }
        */
        OSICC_RD_GLOBAL_VAR(ucReader);
        // Uart_Printf("%s wait bSentByte=%#x\n", __FUNCTION__, bSentByte);
    }
    while ( !bSentByte );

    //  Uart_Printf("%s bSentByte=%#x\n",__FUNCTION__, bSentByte);

    bSentByte = 0;

#if 0
    if ( ICCInfo[ucReader].ucDi != 4 )
        Os__timer_stop(&uiTimeout);
#endif

    if ( ucAPDUOneByteFlag )
    {
        bFinished = 1;
        return( SUCCESS );
    }

    if ( ( ICCInfo[ucReader].ucSessionStep == ICCSESSIONSTEP_DATAEXCHSEND2 )
            && ( uiAPDUSentLen == uiAPDUSendLen ) )
        return( SUCCESS );

    return( SUCCESS );
}

/*
void PROT_CallbackT0Timeout(unsigned char ucReader,unsigned char ucTimeoutType)
{
	ICCINFO *pCurrICCInfo;
    unsigned char aucTimerValue[4];

	pCurrICCInfo = &ICCInfo[ucReader];
	bTO3 = 0;
    HALICC_SetWaitTime(ucReader,0x00,0x00,0x00,0x00,0x00);

 	switch( ucTimeoutType )
	{
		case 0:
		case 1:
		case 2:
		default:
		  	switch( pCurrICCInfo->ucSessionStep )
		    {
			    case ICCSESSIONSTEP_DATAEXCHSEND1:
//					pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND2;
					// EN: Set WWT Timer
					CONV_LongHex(aucTimerValue,4,&pCurrICCInfo->ulWWT);
					HALICC_SetWaitTime(ucReader,0x7C,aucTimerValue[3],aucTimerValue[2],aucTimerValue[0],aucTimerValue[1]);
					HALICC_EnableCommunicate(ucReader,1);
					return;
			    case ICCSESSIONSTEP_DATAEXCHSEND3:
//					pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND4;
					// EN: Set WWT Timer
					CONV_LongHex(aucTimerValue,4,&pCurrICCInfo->ulWWT);
					HALICC_SetWaitTime(ucReader,0x7C,aucTimerValue[3],aucTimerValue[2],aucTimerValue[0],aucTimerValue[1]);
					HALICC_EnableCommunicate(ucReader,1);
					return;
			    case ICCSESSIONSTEP_DATAEXCHSEND2:
			    case ICCSESSIONSTEP_DATAEXCHSEND4:
					pCurrICCInfo->ucLastError = LIBICCERR_APDUSENDTIMEOUT;
					break;
			    case ICCSESSIONSTEP_DATAEXCHRECV1:
					pCurrICCInfo->ucLastError = LIBICCERR_APDURECVPROCTIMEOUT;
					break;
			    case ICCSESSIONSTEP_DATAEXCHRECV2:
					pCurrICCInfo->ucLastError = LIBICCERR_APDURECVTIMEOUT1;
					break;
			    case ICCSESSIONSTEP_DATAEXCHRECV3:
			    case ICCSESSIONSTEP_DATAEXCHRECV4:
					pCurrICCInfo->ucLastError = LIBICCERR_APDURECVTIMEOUT2;
					break;
			    default:
					pCurrICCInfo->ucLastError = LIBICCERR_APDUEXCHUNKNOWN;
					break;
			}
			break;
    }
}
*/
/*
void PROT_CallbackT0CardRemove(unsigned char ucReader)
{
	ICCINFO *pCurrICCInfo;

	pCurrICCInfo = &ICCInfo[ucReader];
	pCurrICCInfo->ucLastError = LIBICCERR_CARDREMOVE;
	ucCardRecvStatus = LIBICCERR_CARDREMOVE;
	ucCardRecvLen = 0;
}
*/
/*
void CONV_LongHex(unsigned char aucTimerValue[],
			unsigned char ucI,unsigned long *ulLong)
{
	unsigned long ulValue;
	unsigned char i;

	ulValue = *ulLong;

	for( i = 0;i < ucI; i ++ )
	{
		aucTimerValue[i] = ulValue;
		ulValue = ulValue >> 8;
	}
}
*/











