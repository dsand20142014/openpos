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
#include "halicc.h"
//#include <haltimer.h>
#include "libicc.h"
//#include <tools.h>
#include <linux/string.h>//zyadd
#include <linux/libconv.h>

#include <linux/module.h>	//for min()
#include <linux/kernel.h>

#include <linux/jiffies.h>

#define ICCPROTOCOLT1_IFSD          254
#define ICCPROTOCOLT1_NAD           0x00

#define ICCPROTOCOLT1_S_IFSREQ      0xC1
#define ICCPROTOCOLT1_S_SYNCREPLY   0xE0
#define ICCPROTOCOLT1_S_IFSREPLY    0xE1
#define ICCPROTOCOLT1_S_BWTREPLY    0xE3

#define ICCPROTOCOLT1_IFRAMESEND    0x01
#define ICCPROTOCOLT1_SFRAMESEND    0x02
#define ICCPROTOCOLT1_SFRAMEREPLY   0x04

#define ICCPROTOCOLT1_SFRAMEREQ     0x01
#define ICCPROTOCOLT1_SFRAMERESP    0x02
#define ICCPROTOCOLT1_IFRAME        0x03
#define ICCPROTOCOLT1_IFRAMELINK    0x04
#define ICCPROTOCOLT1_RFRAME        0x05

#define ICCPROTOCOLT0_SHEBAO_WWT	0X10000

//zy add 0729
extern void *CONV_LongHex(unsigned char *pucDest,unsigned int uiLen,unsigned long *pulSrc);
extern void *CONV_ShortHex(unsigned char *pucDest,unsigned int uiLen,unsigned short *puiSrc);

unsigned char GJPSAM_T0SendReceive(unsigned char ucReader);

unsigned char PROT_AsyncCommand(unsigned char ucReader,
                                unsigned char *pucInData,
                                unsigned short uiInLen)
{
    ICCINFO *pCurrICCInfo;

 //   printk("[%s, %d]  uiInLen:0x%x\n ", __func__, __LINE__,uiInLen);

    if ( uiInLen < 2)
    {
        return(LIBICCERR_BADCMDINPARAM);
    }
    pCurrICCInfo = &ICCInfo[ucReader];

    pCurrICCInfo->uiAPDUSendLen = 0;
    pCurrICCInfo->uiAPDUSentLen = 0;
    memset(pCurrICCInfo->aucAPDUSendBuf,0,LIBICC_MAXCOMMBUF);
    pCurrICCInfo->uiAPDURecvLen = 0;
    pCurrICCInfo->uiAPDURecvedLen = 0;
    memset(pCurrICCInfo->aucAPDURecvBuf,0,LIBICC_MAXCOMMBUF);

    pCurrICCInfo->uiAPDUSendLen = uiInLen-1;
    pCurrICCInfo->ucAPDUCase = *(pucInData++);
    memcpy(pCurrICCInfo->aucAPDUSendBuf,pucInData,pCurrICCInfo->uiAPDUSendLen);
//    printk("[%s, %d]  pCurrICCInfo->ucProtocol:0x%x,pCurrICCInfo->uiAPDUSendLen:0x%x\n ", __func__, __LINE__,pCurrICCInfo->ucProtocol
 //   	,	pCurrICCInfo->uiAPDUSendLen);

    if ( pCurrICCInfo->ucProtocol == 0)
    {
        switch (pCurrICCInfo->uiAPDUSendLen)
        {
        case 4:
            pCurrICCInfo->ucAPDUCase = ICCAPDU_CASE1;
            break;
        case 5:
            if ( pCurrICCInfo->ucAPDUCase != ICCAPDU_CASE0 )
            {
                pCurrICCInfo->ucAPDUCase = ICCAPDU_CASE2;
            }
            else
            {
                pCurrICCInfo->ucAPDUCase = ICCAPDU_CASE1;
            }
            break;
        default:
            if ( pCurrICCInfo->uiAPDUSendLen == (5+pCurrICCInfo->aucAPDUSendBuf[4]))
            {
                pCurrICCInfo->ucAPDUCase = ICCAPDU_CASE3;
            }
            else
            {
                if ( pCurrICCInfo->uiAPDUSendLen == (5+pCurrICCInfo->aucAPDUSendBuf[4]+1))
                {
                    pCurrICCInfo->ucAPDUCase = ICCAPDU_CASE4;
                }
                else
                {
                    printk("[%s, %d]  from this return :0x%x\n ", __func__, __LINE__,uiInLen);
                    return(LIBICCERR_PROTOCOLPARAM);
                }
            }
            break;
        }
    }
 //   printk("[%s, %d]  uiInLen:0x%x\n ", __func__, __LINE__,uiInLen);
    if (  (pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE1)
            &&(pCurrICCInfo->uiAPDUSendLen == 4)
       )
    {
        pCurrICCInfo->uiAPDUSendLen = 5;
        pCurrICCInfo->aucAPDUSendBuf[5] = pCurrICCInfo->aucAPDUSendBuf[4];
        pCurrICCInfo->aucAPDUSendBuf[4] = 0;
    }

   // printk("[%s, %d]  uiInLen:0x%x\n ", __func__, __LINE__,uiInLen);
    pCurrICCInfo->ucLastError = LIBICCERR_SUCCESS;
//  printk("[%s,%d] pCurrICCInfo->ucProtocol:0x%x\n", __func__, __LINE__, pCurrICCInfo->ucProtocol);
    switch (pCurrICCInfo->ucProtocol)
    {

    case 0:
        if ( pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE4 )
        {
//            pCurrICCInfo->aucAPDUSendBuf[4] = pCurrICCInfo->aucAPDUSendBuf[pCurrICCInfo->uiAPDUSendLen-1];
            pCurrICCInfo->uiAPDUSendLen = 5+pCurrICCInfo->aucAPDUSendBuf[4];
        }
        return(PROT_T0Command(ucReader));
    case 1:
        return(PROT_T1Command(ucReader));
    default:
        LIBICC_CODENOTIMPLEMENT;
	printk("[%s,%d] pCurrICCInfo->ucProtocol:0x%x\n", __func__, __LINE__, pCurrICCInfo->ucProtocol);
        return(LIBICCERR_UNKNOWNPROTOCOL);
    }
}

unsigned char PROT_T0Command(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    pCurrICCInfo->ucLastError = LIBICCERR_SUCCESS;
    pCurrICCInfo->uiAPDURecvLen = 0;
    pCurrICCInfo->uiAPDURecvedLen = 0;
    memset(pCurrICCInfo->aucAPDURecvBuf,0,LIBICC_MAXCOMMBUF);
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
    default:
        pCurrICCInfo->ucLastError = LIBICCERR_UNKNOWNSTATUS;
        break;
    }
    pCurrICCInfo->uiAPDURecvLen = 0;
    pCurrICCInfo->uiAPDURecvedLen = 0;
    memset(pCurrICCInfo->aucAPDURecvBuf,0,LIBICC_MAXCOMMBUF);
    pCurrICCInfo->uiRecvLen = 0;
    pCurrICCInfo->uiRecvedLen = 0;
    memset(pCurrICCInfo->aucRecvBuf,0,LIBICC_MAXCOMMBUF);
    /*EN: Send T0 APDU */
    while (!pCurrICCInfo->ucLastError)
    {
        switch ( pCurrICCInfo->ucStandard )
        {
        case ICCSTANDARD_ISO7816:
        case ICCSTANDARD_EMV:
        case ICCSTANDARD_GJISAM:
        case ICCSTANDARD_GJPSAM:
        //	printk("[%s,%d]  pCurrICCInfo->uiAPDUSendLen:0x%x\n",__func__,__LINE__,  pCurrICCInfo->uiAPDUSendLen);
            PROT_T0SendReceive(ucReader);
            break;
//	        case ICCSTANDARD_GJPSAM:
//	            GJPSAM_T0SendReceive(ucReader);
//	            break;
        case ICCSTANDARD_SHEBAO:
            SHEBAO_T0SendReceive(ucReader);
            break;
        default:
            break;
        }
        if ( !pCurrICCInfo->ucLastError )
        {
            switch ( pCurrICCInfo->ucStandard )
            {
            case ICCSTANDARD_ISO7816:
                if ( !ISO7816_SetAPDUNextCommand(ucReader))
                    continue;
                break;
            case ICCSTANDARD_EMV:
                if ( !EMV_SetAPDUNextCommand(ucReader))
                    continue;
                break;
            case ICCSTANDARD_GJISAM:
                if ( !GJISAM_SetAPDUNextCommand(ucReader))
                    continue;
                break;
            case ICCSTANDARD_GJPSAM:
                if ( !GJPSAM_SetAPDUNextCommand(ucReader))
                    continue;
                break;
            case ICCSTANDARD_SHEBAO:
                if ( !SHEBAO_SetAPDUNextCommand(ucReader))
                    continue;
                break;
            default:
                break;
            }
        }
        break;
    }
    PROT_T0FinishedProc(ucReader);
    if ( pCurrICCInfo->ucLastError == LIBICCERR_SUCCESS )
        return(LIBICCERR_PROCESSEND);

    return(pCurrICCInfo->ucLastError);
}

unsigned char PROT_T0SendReceive(unsigned char ucReader)
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
    HALICC_SetPEC(ucReader,4);
    ucINS = pCurrICCInfo->aucAPDUSendBuf[1];
    ucINSMasked = ~pCurrICCInfo->aucAPDUSendBuf[1];
    pCurrICCInfo->ucAPDUOneByteFlag = 0;
    uiAPDURecvedLen = 0;

//    printk("\n[%s,%d]Send data to IC card: pCurrICCInfo->uiAPDUSendLen:0x%x\n", __func__, __LINE__,  pCurrICCInfo->uiAPDUSendLen);
    #if 0
        printk("\nSend data to IC card:\n");
        for (uiI=0; uiI< pCurrICCInfo->uiAPDUSendLen; uiI++)
            printk("%02x ",pCurrICCInfo->aucAPDUSendBuf[uiI]);
        printk("\n");
    #endif

    ucStatus = LIBICCERR_SUCCESS;
 //   printk("[%s,%d], ucStatus:0x%x \n",__func__, __LINE__, ucStatus);
    HALICC_SetTimer(ucReader,0x61,0x00,0x06,0x00,1);
    if ( !ucStatus )
    {
    	//  printk("[%s,%d], ucStatus:0x%x ",__func__, __LINE__, ucStatus);
        do
        {
            ucStatus = PROT_T0RecvChar(ucReader,&ucCh);
          //  printk("[%s,%d], ucStatus:0x%x  ucCh:0x%x\n ",__func__, __LINE__, ucStatus, ucCh);
            if ( ucStatus )          break;

            if ( ucCh == 0x60 )
            {
                /*EN: In TDA8007, don't need to reset timer because timer has been restart*/
#ifdef _WIN32
                HALICC_EnableRecv(ucReader);
#endif
                continue;
            }
     //       printk("[%s,%d], ucCH:0x%x\n",__func__, __LINE__, ucCh);
            if (  ( (ucCh&0xF0) == 0x60)   ||( (ucCh&0xF0) == 0x90)  )
            {
                pCurrICCInfo->ucAPDUOneByteFlag = 0;
                pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen] = ucCh;
                pCurrICCInfo->uiAPDURecvedLen ++;
                uiAPDURecvedLen ++;
                ucStatus = PROT_T0RecvChar(ucReader,&ucCh);
                if ( !ucStatus )
                {
                    pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen] = ucCh;
                    pCurrICCInfo->uiAPDURecvedLen ++;
                    uiAPDURecvedLen ++;
                }
                break;
            }

      //      printk("[%s,%d], ucCH:0x%x \n",__func__, __LINE__, ucCh);
            if ( ucCh == ucINS )
            {
                pCurrICCInfo->ucAPDUOneByteFlag = 0;
                if ( pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE2 )
                {   /*EN: data from card */
                    pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV2;
#ifdef _WIN32
                    HALICC_EnableRecv(ucReader);
#endif
                    for (uiI=uiAPDURecvedLen; uiI<(pCurrICCInfo->aucAPDUSendBuf[4]+2); uiI++)
                    {
                        ucStatus = PROT_T0RecvChar(ucReader,&ucCh);
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
                    HALICC_SetTimer(ucReader,0x61,0x00,0x06,0x00,1);
                    continue;
                }
                break;
            }
        //    printk("[%s,%d], ucCH:0x%x \n",__func__, __LINE__, ucCh);

            if ( ucCh == ucINSMasked )
            {
                pCurrICCInfo->ucAPDUOneByteFlag = 1;
                if ( pCurrICCInfo->ucAPDUCase == ICCAPDU_CASE2 )
                {   /*EN: data from card */
                    pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV2;
                    ucStatus = PROT_T0RecvChar(ucReader,&ucCh);
                    if ( !ucStatus )
                    {
                        pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen] = ucCh;
                        pCurrICCInfo->uiAPDURecvedLen ++;
                        uiAPDURecvedLen ++;
                        if ( uiAPDURecvedLen >= (pCurrICCInfo->aucAPDUSendBuf[4]+2) )
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
                    HALICC_SetTimer(ucReader,0x61,0x00,0x06,0x00,1);
                    continue;
                }
            }

     //       printk("[%s,%d], ucCH:0x%x ",__func__, __LINE__, ucCh);
            if ( ucStatus )            break;
            ucStatus = LIBICCERR_BADPROCBYTE;
            pCurrICCInfo->ucLastError = LIBICCERR_BADPROCBYTE;
        }
        while ( !ucStatus );
    }

   // printk("[%s,%d], ucCH:0x%x \n",__func__, __LINE__, ucCh);

    HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,1);

    return(ucStatus);
}

unsigned char PROT_T0RecvChar(unsigned char ucReader,unsigned char *pucCh)
{
    volatile unsigned int uiTimeOut = 2000;
    ICCINFO *pCurrICCInfo;
    unsigned long jiffies_base = jiffies;
    unsigned long  timers = 0;

    pCurrICCInfo = &ICCInfo[ucReader];
   // uiTimeOut = 2000;
//    printk("[%s,%d]",__func__, __LINE__);
//ZYTODO
//	HALTIMER_Start(HALTIMERTYPE_DECREASE,&uiTimeOut);

    while ( !pCurrICCInfo->ucLastError    && (uiTimeOut)  )
    {
 //   	 printk("[%s,%d]  pCurrICCInfo->uiRecvLen :0x%x   pCurrICCInfo->uiRecvedLen:0x%x\n",__func__, __LINE__,
  //  			 pCurrICCInfo->uiRecvLen,   pCurrICCInfo->uiRecvedLen);
        if ( pCurrICCInfo->uiRecvLen != pCurrICCInfo->uiRecvedLen )
        {
        	// printk("[%s,%d]  *pucCh:0x%x\n",__func__, __LINE__, *pucCh);
            if ( pucCh )
            {
                *pucCh = pCurrICCInfo->aucRecvBuf[pCurrICCInfo->uiRecvLen];
              //  printk("[%s,%d]  *pucCh:0x%x\n",__func__, __LINE__, *pucCh);
                 uiTimeOut = 1000;
            }
            pCurrICCInfo->uiRecvLen ++;
            if ( pCurrICCInfo->uiRecvLen >= LIBICC_MAXCOMMBUF)
                pCurrICCInfo->uiRecvLen = 0;
            break;
        }

        timers++;

   	 if ( ( (jiffies -  jiffies_base) > msecs_to_jiffies(2000) ) ||
   			 (timers >= 20*10000*10000 )  )
        {
   		   uiTimeOut = 0;

    	 printk("recv TimeOut %ld \r\n", jiffies -  jiffies_base);
         break;
	//udelay(1);
        }
   	//  if (timers++ >= 1000)  break;
    }

  //  printk("[%s,%d] uiTimeOut:0x%x\n",__func__, __LINE__,uiTimeOut);
//ZYTODO
//	HALTIMER_Stop(&uiTimeOut);
   if ( !uiTimeOut )
   {
	   printk("439 recv TimeOut %ld \r\n", jiffies -  jiffies_base);
        pCurrICCInfo->ucLastError = LIBICCERR_CARDREMOVE;
        PROT_T0FinishedProc(ucReader);
    }


    return(pCurrICCInfo->ucLastError);
}

unsigned char PROT_T1Command(unsigned char ucReader)
{
    unsigned char ucStatus;
    unsigned char ucSentLen;
    ICCINFO *pCurrICCInfo;
    unsigned char ucLastFrameType;

    pCurrICCInfo = &ICCInfo[ucReader];
    pCurrICCInfo->uiAPDUSentLen = 0;
    pCurrICCInfo->uiAPDURecvLen = 0;
    pCurrICCInfo->uiAPDURecvedLen = 0;
    pCurrICCInfo->uiT1FrameSentLen = 0;
    pCurrICCInfo->uiT1FrameSendLen = 0;
    memset(pCurrICCInfo->aucT1FrameSendBuf,0,LIBICC_MAXCOMMBUF);
    pCurrICCInfo->uiRecvLen = 0;
    memset(pCurrICCInfo->aucRecvBuf,0,LIBICC_MAXCOMMBUF);
    ucLastFrameType = 0x00;

    ucStatus = LIBICCERR_SUCCESS;
    pCurrICCInfo->ucT1Status |= ICCPROTOCOLT1_IFRAMESEND;
    memset(pCurrICCInfo->aucAPDURecvBuf,0,LIBICC_MAXCOMMBUF);
 //printk("[%s,%d]",__func__, __LINE__);
    do
    {
        if ( HALICC_CheckBGT(ucReader) )
            break;
        if ( pCurrICCInfo->ucStatus != ICCSTATUS_POWERED )
        {
            ucStatus = LIBICCERR_CARDREMOVE;
            break;
        }
    }while ( !ucStatus );
//	 printk("[%s,%d]",__func__, __LINE__);
//      while( !HALICC_CheckBGT(ucReader));
    if ( !ucStatus )
    {
        if ( pCurrICCInfo->ucT1IFSReqFlag )
        {
            pCurrICCInfo->ucT1Status |= ICCPROTOCOLT1_SFRAMESEND;
            pCurrICCInfo->ucT1IFSReqFlag = 0;
            pCurrICCInfo->ucSession = ICCSESSION_DATAEXCH;
            pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND1;
            PROT_T1PackSFrameIFSReq(ucReader);
            //gxd import packet
            ucStatus = PROT_T1SendReceiveFrame(ucReader,ICCPROTOCOLT1_SFRAMEREQ,ucLastFrameType);
        }
    }
    if ( !ucStatus )
    {
        for (pCurrICCInfo->uiAPDUSentLen=0; pCurrICCInfo->uiAPDUSentLen<pCurrICCInfo->uiAPDUSendLen;)
        {
            pCurrICCInfo->ucSession = ICCSESSION_DATAEXCH;
            pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND1;
            ucSentLen = PROT_T1PackIFrame(ucReader);
            if ( (pCurrICCInfo->uiAPDUSentLen+ucSentLen) < pCurrICCInfo->uiAPDUSendLen )
            {
                ucStatus = PROT_T1SendReceiveFrame(ucReader,ICCPROTOCOLT1_IFRAMELINK,ucLastFrameType);
                ucLastFrameType = ICCPROTOCOLT1_IFRAMELINK;
            }
            else
            {
                ucStatus = PROT_T1SendReceiveFrame(ucReader,ICCPROTOCOLT1_IFRAME,ucLastFrameType);
            }
            if ( ucStatus )
            {
                if ( ucStatus == LIBICCERR_T1IFRAMERESEND )
                    continue;
                else
                    break;
            }
            else
            {
                pCurrICCInfo->uiAPDUSentLen += ucSentLen;
            }
        }
    }
 //printk("[%s,%d] pCurrICCInfo->ucStatus:0x%x\r\n",__func__, __LINE__,pCurrICCInfo->ucStatus);
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
    default:
        pCurrICCInfo->ucLastError = LIBICCERR_UNKNOWNSTATUS;
        break;
    }
    PROT_T1FinishedProc(ucReader);
 //printk("[%s,%d]",__func__, __LINE__);
    if ( pCurrICCInfo->ucLastError == LIBICCERR_SUCCESS )
        return(LIBICCERR_PROCESSEND);

    return(pCurrICCInfo->ucLastError);
}

unsigned char PROT_T1SendReceiveFrame(unsigned char ucReader,unsigned char ucFrameType,unsigned char ucLastFrameType)
{
    unsigned char ucStatus;
    unsigned char ucRetry;
    unsigned char ucLastError;
    unsigned char ucPowerOffFlag;
    unsigned char ucSendPCB;
    unsigned char ucPCB;
    unsigned char ucILinkFlag;
    unsigned char ucRBlockErr;
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    switch( pCurrICCInfo->ucStatus )
    {
    case ICCSTATUS_INSERT:
        pCurrICCInfo->ucLastError = LIBICCERR_NOTPOWERED;
        break;
    case ICCSTATUS_REMOVE:
        pCurrICCInfo->ucLastError = LIBICCERR_CARDREMOVE;
        break;
    case ICCSTATUS_POWERED:
        pCurrICCInfo->ucLastError = LIBICCERR_SUCCESS;
        break;
    default:
        pCurrICCInfo->ucLastError = LIBICCERR_UNKNOWNSTATUS;
        break;
    }
    if( pCurrICCInfo->ucLastError )
        return( pCurrICCInfo->ucLastError );
    else
    {
        if( pCurrICCInfo->ucStatus != ICCSTATUS_POWERED )
            return(LIBICCERR_CARDREMOVE);
    }
    ucLastError = LIBICCERR_SUCCESS;
    ucPowerOffFlag = 0;
    ucILinkFlag = 0;
    ucRBlockErr = 0;

    pCurrICCInfo->uiAPDURecvLen = 0;
    pCurrICCInfo->uiAPDURecvedLen = 0;
    memset(pCurrICCInfo->aucAPDURecvBuf,0,LIBICC_MAXCOMMBUF);

    for(ucRetry=0;ucRetry < 3;)
    {
        pCurrICCInfo->uiRecvLen = 0;
        ucLastError = pCurrICCInfo->ucLastError;
        pCurrICCInfo->ucLastError = LIBICCERR_SUCCESS;
        memset(pCurrICCInfo->aucRecvBuf,0,LIBICC_MAXCOMMBUF);
        /* Set BGT Timer */
        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND1;
        HALICC_SetTimer(ucReader,0x61,0x00,0x16,0x00,1);
        ucStatus = PROT_T1ReceiveFrame(ucReader);
        if( !ucStatus )
        {
            ucPCB = pCurrICCInfo->aucRecvBuf[1];
            if(  (ucPCB == 0xC1 )
               &&(ucFrameType != ICCPROTOCOLT1_SFRAMEREQ)
              )
            {
                /*EN: S Frame IFS request */
                if(  (pCurrICCInfo->aucRecvBuf[3] < 0x10)
                   ||(pCurrICCInfo->aucRecvBuf[3] == 0xFF)
                  )
                {
                    ucStatus = LIBICCERR_T1INVALIDBLOCK;
                    pCurrICCInfo->ucLastError = LIBICCERR_T1BADPROTOCOL;
                }else
                {
                    pCurrICCInfo->ucT1IFSC = pCurrICCInfo->aucRecvBuf[3];
                    /*EN: Send S Frame IFS Reply */
                    PROT_T1PackSFrameIFSReply(ucReader);
                    pCurrICCInfo->uiT1FrameSentLen = 0;
                    continue;
                }
            }
            if( ucPCB == 0xC2 )
            {
                /*EN: S Frame abort request */
                ucStatus = LIBICCERR_T1SABORTREQUEST;
                pCurrICCInfo->ucLastError = LIBICCERR_T1SABORTREQUEST;
                ucPowerOffFlag = 1;
                break;
            }
            if(  (ucPCB == 0xC3 )
               &&(ucFrameType != ICCPROTOCOLT1_SFRAMEREQ)
              )
            {
                /*EN: S Frame BWT request */
                if( pCurrICCInfo->aucRecvBuf[3] == 0x00)
                {
                    pCurrICCInfo->aucRecvBuf[3] = 0x01;
                }
                pCurrICCInfo->ucWTX = pCurrICCInfo->aucRecvBuf[3];
                /*EN: Send S Frame BWT Reply */
                PROT_T1PackSFrameBWTReply(ucReader);
                pCurrICCInfo->uiT1FrameSentLen = 0;
                continue;
            }
            if( !ucStatus )
            {
                switch( ucFrameType )
                {
                case ICCPROTOCOLT1_SFRAMEREQ:
                    if(  (ucPCB & 0xE0) != 0xE0 )
                    {
                        /*EN: EMV 5.2.5.1-6 Not SFRAMERESP resend */
                        pCurrICCInfo->uiT1FrameSentLen = 0;
                        ucRetry++;
                        continue;
                    }
                    if(  (ucPCB & 0x0F) != (pCurrICCInfo->aucT1FrameSendBuf[1]&0x0F))
                    {
                        /*EN: EMV 5.2.5.1-6 Not SAME AS SFRAME REQUEST resend */
                        pCurrICCInfo->uiT1FrameSentLen = 0;
                        ucRetry++;
                        continue;
                    }
                    if(  (ucPCB & 0x0F) == 0x01 )
                    {
                        /*EN: Length not same */
                        if( pCurrICCInfo->aucRecvBuf[3] != ICCPROTOCOLT1_IFSD )
                        {
                            pCurrICCInfo->uiT1FrameSentLen = 0;
                            ucRetry++;
                            continue;
                        }
                    }
                    break;
                case ICCPROTOCOLT1_IFRAME:
                    if(  (ucPCB & 0xE0) == 0xE0 )
                    {
                        /*EN: Bad protocol, Receive S RESP Frame */
                        ucStatus = LIBICCERR_T1INVALIDBLOCK;
                        pCurrICCInfo->ucLastError = LIBICCERR_T1BADPROTOCOL;
                        break;
                    }
                    /*EN: Receive R Frame */
                    if( (ucPCB & 0xE0) == 0x80 )
                    {
                        if( ucILinkFlag )
                        {
                            /*EN: Error, Resend Frame */
                            if( ((ucPCB & 0x40)>>6) != pCurrICCInfo->ucT1RecvSeq )
                            {
                                pCurrICCInfo->uiT1FrameSentLen = 0;
                                ucRetry++;
                                continue;
                            }
                            /*EN: Error, need resend R Frame */
                            ucStatus = LIBICCERR_T1INVALIDBLOCK;
                            pCurrICCInfo->ucLastError = LIBICCERR_T1ILINKERR;
                            break;
                        }else
                        {
                            if(((ucPCB & 0x10)>>4) == pCurrICCInfo->ucT1SendSeq )
                            {
                                ucSendPCB = pCurrICCInfo->aucT1FrameSendBuf[1];
                                if( (ucPCB & 0xE0) == 0x80  )
                                {
                                    ucStatus = LIBICCERR_T1INVALIDBLOCK;
                                    break;
                                }
                                ucStatus = LIBICCERR_T1INVALIDBLOCK;
                                pCurrICCInfo->ucLastError = LIBICCERR_T1INVALIDSEQ;
                                break;
                            }
                            ucRBlockErr ++;
                            if( ucRBlockErr >= 3 )
                            {
				                ucPowerOffFlag = 1;
				                break;
                            }
                            pCurrICCInfo->ucT1SendSeq = (~pCurrICCInfo->ucT1SendSeq)&0x01;
                            PROT_T1PackIFrame(ucReader);
                            pCurrICCInfo->uiT1FrameSentLen = 0;
                            continue;
                        }
                        break;
                    }
                    /*EN: Receive I Frame */
                    if(  (ucPCB & 0xA0) == 0x00 )
                    {
                        if(  ((ucPCB & 0x40)>>6) != pCurrICCInfo->ucT1RecvSeq )
                        {
                            /*EN: Bad protocol, bad SEQ */
                            ucStatus = LIBICCERR_T1INVALIDBLOCK;
                            pCurrICCInfo->ucLastError = LIBICCERR_T1INVALIDSEQ;
                            break;
                        }
                        ucILinkFlag = 0;
                        pCurrICCInfo->ucT1RecvSeq = (~pCurrICCInfo->ucT1RecvSeq)&0x01;
                        if( (pCurrICCInfo->uiAPDURecvedLen+pCurrICCInfo->aucRecvBuf[2]) < LIBICC_MAXCOMMBUF)
                        {
                            memcpy(&pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen],
                                &pCurrICCInfo->aucRecvBuf[3],
                                pCurrICCInfo->aucRecvBuf[2]);
                            pCurrICCInfo->uiAPDURecvedLen += pCurrICCInfo->aucRecvBuf[2];
                            break;
                        }else
                        {
                            pCurrICCInfo->ucLastError = LIBICCERR_OVERRECVBUF;
                            break;
                        }
                    }
                    /*EN: Receive I Link Frame */
                    if(  (ucPCB & 0xA0) == 0x20 )
                    {
                        ucILinkFlag = 1;
                        if(  ((ucPCB & 0x40)>>6) != pCurrICCInfo->ucT1RecvSeq )
                        {
                            /*EN: Bad protocol, bad SEQ */
                            ucStatus = LIBICCERR_T1INVALIDBLOCK;
                            pCurrICCInfo->ucLastError = LIBICCERR_T1INVALIDSEQ;
                            break;
                        }
                        pCurrICCInfo->ucT1RecvSeq = (~pCurrICCInfo->ucT1RecvSeq)&0x01;
                        if( (pCurrICCInfo->uiAPDURecvedLen+pCurrICCInfo->aucRecvBuf[2]) < LIBICC_MAXCOMMBUF)
                        {
                            memcpy(&pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen],
                                &pCurrICCInfo->aucRecvBuf[3],
                                pCurrICCInfo->aucRecvBuf[2]);
                            pCurrICCInfo->uiAPDURecvedLen += pCurrICCInfo->aucRecvBuf[2];
                        }else
                        {
                            pCurrICCInfo->ucLastError = LIBICCERR_OVERRECVBUF;
                        }
                        /*EN: Response Frame */
                        PROT_T1PackRFrame(ucReader,pCurrICCInfo->ucT1RecvSeq,0);
                        pCurrICCInfo->uiT1FrameSentLen = 0;
                        continue;
                    }
                    break;
                case ICCPROTOCOLT1_IFRAMELINK:
                    if(  (ucPCB & 0xE0) == 0xE0 )
                    {
                        /*EN: Bad protocol, Receive S RESP Frame */
                        ucStatus = LIBICCERR_T1INVALIDBLOCK;
                        pCurrICCInfo->ucLastError = LIBICCERR_T1BADPROTOCOL;
                        break;
                    }
                    if(  (ucPCB & 0xE0) != 0x80 )
                    {
                        /*EN: Bad protocol, Not R Frame */
                        ucStatus = LIBICCERR_T1INVALIDBLOCK;
                        pCurrICCInfo->ucLastError = LIBICCERR_T1BADPROTOCOL;
                        break;
                    }
                    /*EN: Receive R Frame */
                    if( (ucPCB & 0xE0) == 0x80 )
                    {
                        if(  ((ucPCB & 0x10)>>4) != pCurrICCInfo->ucT1SendSeq )
                        {
                            /*EN: Bad protocol, bad SEQ */
                            ucStatus = LIBICCERR_T1INVALIDBLOCK;
                            pCurrICCInfo->ucLastError = LIBICCERR_T1INVALIDSEQ;
                            break;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
        if( ucStatus )
        {
        	if( ucStatus == LIBICCERR_APDURECVTIMEOUT2)
        	{
        		ucPowerOffFlag = 1;
        		break;
        	}
/*
            if( ucRetry )
            {
                pCurrICCInfo->ucLastError = ucLastError;
            }else
            {
                ucLastError = pCurrICCInfo->ucLastError;
            }
*/
            switch(ucFrameType)
            {
            case ICCPROTOCOLT1_SFRAMEREQ:
                if( pCurrICCInfo->ucLastError == LIBICCERR_APDUPARITYERROR )
                {
                     /*EN: RESEND FRAME */
                    pCurrICCInfo->uiT1FrameSentLen = 0;
                    ucRetry++;
                    continue;
                }
                if( ucStatus == LIBICCERR_T1INVALIDBLOCK )
                {
                    /*EN: RESEND S FRAME REQ */
                    pCurrICCInfo->uiT1FrameSentLen = 0;
                    ucRetry++;
                    continue;
                }
                ucPowerOffFlag = 1;
                break;
            case ICCPROTOCOLT1_IFRAME:
                if( ucStatus != LIBICCERR_T1INVALIDBLOCK )
                {
                    ucPowerOffFlag = 1;
                    break;
                }
                if( ucStatus == LIBICCERR_T1INVALIDBLOCK )
                {
                    ucSendPCB = pCurrICCInfo->aucT1FrameSendBuf[1];
                    if( (ucSendPCB & 0xE0) == 0x80 )
						#if 0
                    {
                        pCurrICCInfo->uiT1FrameSentLen = 0;
                        if(  (pCurrICCInfo->ucLastError != LIBICCERR_T1RFRAMEERR )
                           &&(pCurrICCInfo->ucLastError != LIBICCERR_T1RFRAMEBADEDC )
                           &&(!pCurrICCInfo->ucLastError )
                          )
                        {
                            ucRetry ++;
                        }
                        pCurrICCInfo->ucLastError = ucStatus;
                        continue;
                    }
			#else
                    if( pCurrICCInfo->ucLastError )
                    	{
				 pCurrICCInfo->uiT1FrameSentLen = 0;
				  if(  (pCurrICCInfo->ucLastError != LIBICCERR_T1RFRAMEERR )
                           &&(pCurrICCInfo->ucLastError != LIBICCERR_T1RFRAMEBADEDC ))
	                        {
	                            ucRetry ++;
	                        }
	                        pCurrICCInfo->ucLastError = ucStatus;
	                        continue;
			}
			#endif
					
                    if(  (pCurrICCInfo->ucLastError == LIBICCERR_APDUPARITYERROR )
                       ||(pCurrICCInfo->ucLastError == LIBICCERR_T1BADEDC )
                      )
                    {
                        PROT_T1PackRFrame(ucReader,pCurrICCInfo->ucT1RecvSeq,0x01);
                    }else
                    {
                        PROT_T1PackRFrame(ucReader,pCurrICCInfo->ucT1RecvSeq,0x02);
                    }
			if(pCurrICCInfo->ucLastError)
				ucRetry++;
                    pCurrICCInfo->uiT1FrameSentLen = 0;
                    pCurrICCInfo->ucLastError = ucStatus;
             //       ucRetry++;
                    continue;
                }
                pCurrICCInfo->ucLastError = ucStatus;
                if(  (pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEBADEDC )
                   ||(pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEERR )
                  )
                {
                    ucPCB = pCurrICCInfo->aucRecvBuf[1];
                    if(  (!ucLastError)
                       &&(((ucPCB & 0x10)>>4) == pCurrICCInfo->ucT1SendSeq )
                      )
                    {
                        pCurrICCInfo->ucLastError = LIBICCERR_T1INVALIDSEQ;
                    }
                }
                if( ucLastError )
                {
                    if(  (pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEBADEDC )
                       ||(pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEERR )
                      )
                    {
#if 0
                        if( ucRetry != 2 )
                        {
                            pCurrICCInfo->ucT1SendSeq = (~pCurrICCInfo->ucT1SendSeq)&0x01;
                            PROT_T1PackIFrame(ucReader);
                            pCurrICCInfo->uiT1FrameSentLen = 0;
                            continue;
                        }else
                        {
                            pCurrICCInfo->uiT1FrameSentLen = 0;
                            continue;
                        }
#else
                        if(  (ucLastError == pCurrICCInfo->ucLastError )
//                           ||(ucRetry == 1 )
                          )
                        {
                            pCurrICCInfo->ucT1SendSeq = (~pCurrICCInfo->ucT1SendSeq)&0x01;
                            PROT_T1PackIFrame(ucReader);
                            pCurrICCInfo->uiT1FrameSentLen = 0;
                            continue;
                        }else
                        {
                            pCurrICCInfo->uiT1FrameSentLen = 0;
                            continue;
                        }
#endif
                    }else
                    {
                        /*EN: SEND R FRAME */
                        if(  (ucLastError == LIBICCERR_T1BADEDC )
                           ||(ucLastError == LIBICCERR_APDUPARITYERROR )
                          )
                        {
                            PROT_T1PackRFrame(ucReader,pCurrICCInfo->ucT1RecvSeq,0x01);
                        }else
                        {
                            PROT_T1PackRFrame(ucReader,pCurrICCInfo->ucT1RecvSeq,0x02);
                        }
                        pCurrICCInfo->uiT1FrameSentLen = 0;
                        ucRetry++;
                        continue;
                    }
                }
                if(  (pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEBADEDC )
                   ||(pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEERR )
                  )
                {
                    pCurrICCInfo->uiT1FrameSentLen = 0;
                    continue;
                }
                /*EN: SEND R FRAME */
                if(  (pCurrICCInfo->ucLastError == LIBICCERR_T1BADEDC )
                   ||(pCurrICCInfo->ucLastError == LIBICCERR_APDUPARITYERROR )
                  )
                {
                    PROT_T1PackRFrame(ucReader,pCurrICCInfo->ucT1RecvSeq,0x01);
                }else
                {
                    PROT_T1PackRFrame(ucReader,pCurrICCInfo->ucT1RecvSeq,0x02);
                }
                pCurrICCInfo->uiT1FrameSentLen = 0;
                ucRetry++;
                continue;
            case ICCPROTOCOLT1_IFRAMELINK:
                if(  (pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEERR )
                   ||(pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEBADEDC )
                  )
                {
                    pCurrICCInfo->uiT1FrameSentLen = 0;
                    continue;
                }
                if( pCurrICCInfo->ucLastError == LIBICCERR_T1INVALIDSEQ )
                {
                    pCurrICCInfo->uiT1FrameSentLen = 0;
                    ucRetry++;
                    continue;
                }
                if(  (pCurrICCInfo->ucLastError == LIBICCERR_T1BADEDC )
                   ||(pCurrICCInfo->ucLastError == LIBICCERR_APDUPARITYERROR )
                  )
                {
                    PROT_T1PackRFrame(ucReader,pCurrICCInfo->ucT1RecvSeq,0x01);
                    pCurrICCInfo->uiT1FrameSentLen = 0;
                    ucRetry++;
                    continue;
                }
                PROT_T1PackRFrame(ucReader,pCurrICCInfo->ucT1RecvSeq,0x02);
                pCurrICCInfo->uiT1FrameSentLen = 0;
                ucRetry++;
                continue;
            default:
                PROT_T1PackRFrame(ucReader,pCurrICCInfo->ucT1RecvSeq,0x02);
                pCurrICCInfo->uiT1FrameSentLen = 0;
                ucRetry++;
                continue;
            }
        }
        if( !ucStatus )
            break;
        if( ucPowerOffFlag == 1)
            break;
    }
    if(  (ucRetry >= 3)
       &&(ucStatus)
      )
    {
        ucPowerOffFlag = 1;
    }
    if( ucPowerOffFlag )
    {
        HALICC_Deactive(ucReader);
    }
    return(pCurrICCInfo->ucLastError);
}

unsigned char PROT_T1ReceiveFrame(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;
    unsigned char ucEDC;
    unsigned char ucPCB;

    pCurrICCInfo = &ICCInfo[ucReader];
    do
    {
        if ( pCurrICCInfo->uiRecvLen > 3 )
        {
            if ( pCurrICCInfo->uiRecvLen == pCurrICCInfo->aucRecvBuf[2]+4 )
            {
                break;
            }
        }
        if ( pCurrICCInfo->ucStatus != ICCSTATUS_POWERED )
        {
            pCurrICCInfo->ucLastError = LIBICCERR_CARDREMOVE;
            break;
        }
    }
    while (  (pCurrICCInfo->ucLastError == LIBICCERR_SUCCESS )
#ifndef _WIN32
             ||(pCurrICCInfo->ucLastError == LIBICCERR_APDUPARITYERROR)
#else
#endif
          );

    HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,1);

    if ( pCurrICCInfo->ucLastError == LIBICCERR_SUCCESS )
    {
        ucEDC = PROT_T1ComputeEDC(pCurrICCInfo->ucT1EDCMode,
                                  pCurrICCInfo->aucRecvBuf,
                                  pCurrICCInfo->uiRecvLen-1);
        if ( ucEDC != pCurrICCInfo->aucRecvBuf[pCurrICCInfo->uiRecvLen-1] )
        {
            pCurrICCInfo->ucLastError = LIBICCERR_T1BADEDC;
        }
        else
        {
            /*EN: Check NAD */
            if ( pCurrICCInfo->aucRecvBuf[0] != ICCPROTOCOLT1_NAD )
            {
                pCurrICCInfo->ucLastError = LIBICCERR_T1BADNAD;
            }
            else
            {
                /*EN: Check PCB */
                ucPCB = pCurrICCInfo->aucRecvBuf[1];
                if ( ucPCB == 0xC2 )
                    return(LIBICCERR_SUCCESS);
                if ( (ucPCB & 0xE0) == 0xA0 )
                {
                    /*EN: Invalid Frame Type */
                    pCurrICCInfo->ucLastError = LIBICCERR_T1BADPCB;
                }
                else
                {
                    if ( (ucPCB & 0x80) == 0x00 )
                    {
                        /*EN: I Frame length must not 0 and 255 */
                        if (  (pCurrICCInfo->aucRecvBuf[2] == 0x00 )
                                ||(pCurrICCInfo->aucRecvBuf[2] == 0xFF )
                           )
                        {
                            pCurrICCInfo->ucLastError = LIBICCERR_T1BADIFRAMELEN;
                        }
                    }
                    if ( (ucPCB & 0xE0) == 0x80 )
                    {
                        /*EN: R Frame length must is 0 */
                        if ( pCurrICCInfo->aucRecvBuf[2] != 0x00 )
                        {
                            pCurrICCInfo->ucLastError = LIBICCERR_T1BADRFRAMELEN;
                        }
                        else
                        {
                            /*EN: R Frame indicate error */
                            if ( (ucPCB & 0x0F) != 0x00 )
                            {
                                if ( (ucPCB & 0x0F) == 0x01 )
                                {
                                    pCurrICCInfo->ucLastError = LIBICCERR_T1RFRAMEBADEDC;
                                }
                                else
                                {
                                    pCurrICCInfo->ucLastError = LIBICCERR_T1RFRAMEERR;
                                }
                            }
                        }
                    }
                    if ( (ucPCB & 0xC0) == 0xC0 )
                    {
                        /*EN: S Frame length must is 1 */
                        if ( pCurrICCInfo->aucRecvBuf[2] != 0x01 )
                        {
                            pCurrICCInfo->ucLastError = LIBICCERR_T1BADSFRAMELEN;
                        }
                        else
                        {
                            /*EN: S Frame unsupport */
                            if (  ((ucPCB & 0x1F) != 0x00)
                                    &&((ucPCB & 0x1F) != 0x01)
                                    &&((ucPCB & 0x1F) != 0x02)
                                    &&((ucPCB & 0x1F) != 0x03)
                               )
                            {
                                pCurrICCInfo->ucLastError = LIBICCERR_T1SFRAMEERR;
                            }
                        }
                    }
                }
            }
        }
    }
    if (  ( pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEBADEDC )
            ||( pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEERR )
       )
        return(LIBICCERR_SUCCESS);
    if (  ( pCurrICCInfo->ucLastError == LIBICCERR_APDUPARITYERROR)
            ||( pCurrICCInfo->ucLastError == LIBICCERR_T1BADEDC )
            ||( pCurrICCInfo->ucLastError == LIBICCERR_T1BADNAD )
            ||( pCurrICCInfo->ucLastError == LIBICCERR_T1BADPCB )
            ||( pCurrICCInfo->ucLastError == LIBICCERR_T1BADIFRAMELEN )
            ||( pCurrICCInfo->ucLastError == LIBICCERR_T1BADRFRAMELEN )
            ||( pCurrICCInfo->ucLastError == LIBICCERR_T1BADSFRAMELEN )
            ||( pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEBADEDC )
            ||( pCurrICCInfo->ucLastError == LIBICCERR_T1RFRAMEERR )
            ||( pCurrICCInfo->ucLastError == LIBICCERR_T1SFRAMEERR )
       )
    {
        return(LIBICCERR_T1INVALIDBLOCK);
    }
    else
    {
        return(pCurrICCInfo->ucLastError);
    }
}

unsigned char PROT_T0FinishedProc(unsigned char ucReader)
{
    unsigned char aucResult[LIBICC_MAXCOMMBUF];
    unsigned short uiLen;
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    pCurrICCInfo->ucSession = ICCSESSION_IDLE;
    pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_IDLE;

    memset(aucResult,0,sizeof(aucResult));
	// printk("[%s,%d]\n",__func__, __LINE__);
    uiLen = 3;
    if ( !pCurrICCInfo->ucLastError )
    {
        aucResult[uiLen] = LIBICCERR_EXECUTEOK;
    }
    else
    {
        aucResult[uiLen] = pCurrICCInfo->ucLastError;
    }
    uiLen ++;
//    if( !pCurrICCInfo->uiAPDURecvLen )
    {
        pCurrICCInfo->uiAPDURecvLen = pCurrICCInfo->uiAPDURecvedLen;
    }
 //printk("[%s,%d]\n",__func__, __LINE__);
    if ( pCurrICCInfo->uiAPDURecvLen <= LIBICC_MAXCOMMBUF-2 )
    {
        if ( pCurrICCInfo->uiAPDURecvLen >= 2)
        {
      //  printk("[%s,%d] uiLen:0x%x  pCurrICCInfo->uiAPDURecvLen-2:0x%x\r\n",
	//	__func__, __LINE__,uiLen,pCurrICCInfo->aucAPDURecvBuf,pCurrICCInfo->uiAPDURecvLen-2);
            aucResult[uiLen++] = pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvLen-2];
            aucResult[uiLen++] = pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvLen-1];
            memcpy(&aucResult[uiLen],pCurrICCInfo->aucAPDURecvBuf,
                   pCurrICCInfo->uiAPDURecvLen-2);
            uiLen += pCurrICCInfo->uiAPDURecvLen-2;
        }
        else
        {
            memcpy( &aucResult[uiLen],pCurrICCInfo->aucAPDURecvBuf,
                    pCurrICCInfo->uiAPDURecvLen);
            uiLen += pCurrICCInfo->uiAPDURecvLen;
        }
    }
 //printk("[%s,%d]\n",__func__, __LINE__);
    if ( (uiLen-1) > 255 )
    {
        aucResult[0] = 0xFF;
        aucResult[1] = (uiLen-3)/256;
        aucResult[2] = (uiLen-3)%256;
    }
    else
    {
        aucResult[0] = uiLen-3;
        memcpy(&aucResult[1],&aucResult[3],uiLen-3);
    }
	// printk("[%s,%d]\n",__func__, __LINE__);
    HALICC_DRVEnd(LIBICCERR_SUCCESS,aucResult,uiLen);
	// printk("[%s,%d]\n",__func__, __LINE__);
    return(pCurrICCInfo->ucLastError);
}

unsigned char PROT_T1FinishedProc(unsigned char ucReader)
{
    unsigned char aucResult[LIBICC_MAXCOMMBUF];
    unsigned short uiLen;
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    pCurrICCInfo->ucSession = ICCSESSION_IDLE;
    pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_IDLE;
    pCurrICCInfo->ucT1Status = 0;

    memset(aucResult,0,sizeof(aucResult));
    uiLen = 3;
    if ( !pCurrICCInfo->ucLastError )
    {
        aucResult[uiLen] = LIBICCERR_EXECUTEOK;
    }
    else
    {
        aucResult[uiLen] = pCurrICCInfo->ucLastError;
    }
    uiLen ++;
#if 0
    if ( pCurrICCInfo->aucRecvBuf[2] )
    {
        if ( pCurrICCInfo->aucRecvBuf[2] > 2)
        {
            aucResult[ucLen++] = pCurrICCInfo->aucRecvBuf[pCurrICCInfo->aucRecvBuf[2]+1];
            aucResult[ucLen++] = pCurrICCInfo->aucRecvBuf[pCurrICCInfo->aucRecvBuf[2]+2];
            memcpy(&aucResult[ucLen],&(pCurrICCInfo->aucRecvBuf[3]),
                   pCurrICCInfo->aucRecvBuf[2]-2);
            ucLen += pCurrICCInfo->aucRecvBuf[2]-2;
        }
        else
        {
            memcpy( &aucResult[ucLen],&(pCurrICCInfo->aucRecvBuf[3]),
                    pCurrICCInfo->aucRecvBuf[2]);
            ucLen += pCurrICCInfo->aucRecvBuf[2];
        }
    }
#else
   //printk("recved len %d.\n",pCurrICCInfo->uiAPDURecvedLen);
    if ( pCurrICCInfo->uiAPDURecvedLen )
    {
        if ( pCurrICCInfo->uiAPDURecvedLen > 2)
        {
            aucResult[uiLen++] = pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-2];
            aucResult[uiLen++] = pCurrICCInfo->aucAPDURecvBuf[pCurrICCInfo->uiAPDURecvedLen-1];
            memcpy(&aucResult[uiLen],&(pCurrICCInfo->aucAPDURecvBuf[0]),
                   pCurrICCInfo->uiAPDURecvedLen-2);
            uiLen += pCurrICCInfo->uiAPDURecvedLen-2;
        }
        else
        {
            memcpy( &aucResult[uiLen],&(pCurrICCInfo->aucAPDURecvBuf[0]),
                    pCurrICCInfo->uiAPDURecvedLen);
            uiLen += pCurrICCInfo->uiAPDURecvedLen;
        }
    }
#endif
// printk("[%s,%d]\n",__func__, __LINE__);
    if ( (uiLen-1) > 255 )
    {
        aucResult[0] = 0xFF;
        aucResult[1] = (uiLen-3)/256;
        aucResult[2] = (uiLen-3)%256;
    }
    else
    {
        aucResult[0] = uiLen-3;
        memcpy(&aucResult[1],&aucResult[3],uiLen-3);
    }
#if 0
	unsigned int ucI =0;
	printk("PROT_T1FinishedProc uiLen %d.\n",uiLen);
	for(ucI=0;ucI<uiLen;ucI++)
		printk("[%d]%02x ",aucResult[ucI], ucI);
#endif
    HALICC_DRVEnd(LIBICCERR_SUCCESS,aucResult,uiLen);
 //printk("[%s,%d]\n",__func__, __LINE__);
    return(pCurrICCInfo->ucLastError);
}

unsigned char PROT_T1ComputeEDC(unsigned char ucMode,unsigned char *pucInData,unsigned short uiInLen)
{
    unsigned char ucEDC;
    unsigned short uiI;

    if ( !ucMode )
    {
        ucEDC = 0x00;
        for (uiI=0; uiI<uiInLen; uiI++)
        {
            ucEDC ^= *(pucInData+uiI);
        }
        return(ucEDC);
    }
    return(LIBICCERR_SUCCESS);
}

unsigned char PROT_T1PackIFrame(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;
    unsigned char ucAPDULen;
    unsigned char ucPCB;

    pCurrICCInfo = &ICCInfo[ucReader];

    ucPCB = 0x00;
    if ( pCurrICCInfo->ucT1SendSeq )
    {
        ucPCB |= 0x40;
    }
    pCurrICCInfo->ucT1SendSeq = (~pCurrICCInfo->ucT1SendSeq)&0x01;
    if ( (pCurrICCInfo->uiAPDUSendLen-pCurrICCInfo->uiAPDUSentLen) > pCurrICCInfo->ucT1IFSC )
    {
        ucAPDULen = pCurrICCInfo->ucT1IFSC;
        ucPCB |= 0x20;
    }
    else
    {
        ucAPDULen = pCurrICCInfo->uiAPDUSendLen-pCurrICCInfo->uiAPDUSentLen;
    }
    PROT_T1PackFrame(ucReader,ucPCB,&(pCurrICCInfo->aucAPDUSendBuf[pCurrICCInfo->uiAPDUSentLen]),
                     ucAPDULen);

    return(ucAPDULen);
}

unsigned char PROT_T1PackRFrame(unsigned char ucReader,unsigned char ucSEQ,unsigned char ucError)
{
    unsigned char ucPCB;

    ucPCB = 0x80;
    if ( ucSEQ )
    {
        ucPCB |= 0x10;
    }
    if ( ucError & 0x0F )
    {
        ucPCB |= (ucError & 0x0F);
    }
    return(PROT_T1PackFrame(ucReader,ucPCB,0,0));
}

unsigned char PROT_T1PackSFrameIFSReq(unsigned char ucReader)
{
    unsigned char ucCh;

    ucCh = ICCPROTOCOLT1_IFSD;
    return(PROT_T1PackFrame(ucReader,ICCPROTOCOLT1_S_IFSREQ,&ucCh,1));
}

unsigned char PROT_T1PackSFrameSyncReply(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;
    unsigned char ucCh;

    pCurrICCInfo = &ICCInfo[ucReader];

    ucCh = pCurrICCInfo->aucRecvBuf[3];
    return(PROT_T1PackFrame(ucReader,ICCPROTOCOLT1_S_SYNCREPLY,&ucCh,1));
}

unsigned char PROT_T1PackSFrameIFSReply(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;
    unsigned char ucCh;

    pCurrICCInfo = &ICCInfo[ucReader];

    ucCh = pCurrICCInfo->ucT1IFSC;
    return(PROT_T1PackFrame(ucReader,ICCPROTOCOLT1_S_IFSREPLY,&ucCh,1));
}

unsigned char PROT_T1PackSFrameBWTReply(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;
    unsigned char ucCh;

    pCurrICCInfo = &ICCInfo[ucReader];

    ucCh = pCurrICCInfo->aucRecvBuf[3];
    return(PROT_T1PackFrame(ucReader,ICCPROTOCOLT1_S_BWTREPLY,&ucCh,1));
}

unsigned char PROT_T1PackFrame(unsigned char ucReader,unsigned char ucPCB,
                               unsigned char *pucInData,
                               unsigned char ucInLen)
{
    ICCINFO *pCurrICCInfo;
    unsigned char ucEDC;

    pCurrICCInfo = &ICCInfo[ucReader];
    pCurrICCInfo->uiT1FrameLastSendLen = pCurrICCInfo->uiT1FrameSendLen;
    memcpy(pCurrICCInfo->aucT1FrameLastSendBuf,
           pCurrICCInfo->aucT1FrameSendBuf,
           LIBICC_MAXCOMMBUF);
    pCurrICCInfo->uiT1FrameSendLen = 0;
    pCurrICCInfo->uiT1FrameSentLen = 0;
    pCurrICCInfo->aucT1FrameSendBuf[pCurrICCInfo->uiT1FrameSendLen++] = ICCPROTOCOLT1_NAD;
    pCurrICCInfo->aucT1FrameSendBuf[pCurrICCInfo->uiT1FrameSendLen++] = ucPCB;
    pCurrICCInfo->aucT1FrameSendBuf[pCurrICCInfo->uiT1FrameSendLen++] = ucInLen;
    memcpy( &pCurrICCInfo->aucT1FrameSendBuf[pCurrICCInfo->uiT1FrameSendLen],
            pucInData,ucInLen);
    pCurrICCInfo->uiT1FrameSendLen += ucInLen;
    ucEDC = PROT_T1ComputeEDC(pCurrICCInfo->ucT1EDCMode,
                              pCurrICCInfo->aucT1FrameSendBuf,
                              pCurrICCInfo->uiT1FrameSendLen);
    pCurrICCInfo->aucT1FrameSendBuf[pCurrICCInfo->uiT1FrameSendLen++] = ucEDC;

    return(LIBICCERR_SUCCESS);
}

void PROT_CallbackSendChar(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    switch ( pCurrICCInfo->ucProtocol)
    {
    case ICCPROTOCOL_T0:
        PROT_CallbackT0SendChar(ucReader);
        break;
    case ICCPROTOCOL_T1:
        PROT_CallbackT1SendChar(ucReader);
        break;
    default:
        LIBICC_CODENOTIMPLEMENT;
        break;
    }
}

void PROT_CallbackT0SendChar(unsigned char ucReader)
{


    ICCINFO *pCurrICCInfo;


    pCurrICCInfo = &ICCInfo[ucReader];
 //   printk("[%s,%d] pCurrICCInfo->uiAPDUSentLen:0x%x\n",__func__,__LINE__,pCurrICCInfo->uiAPDUSentLen);
    if ( pCurrICCInfo->ucAPDUOneByteFlag )
    {
        HALICC_EnableRecvAfterSend(ucReader);
    }
    switch (pCurrICCInfo->ucSessionStep)
    {
    case ICCSESSIONSTEP_DATAEXCHSEND2:
        if ( pCurrICCInfo->uiAPDUSentLen == 4 )
        {
        //	printk("[%s,%d]",__func__,__LINE__);
            pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV1;
            HALICC_EnableRecvAfterSend(ucReader);
        }
        break;
    case ICCSESSIONSTEP_DATAEXCHSEND4:
        if ( pCurrICCInfo->uiAPDUSentLen >= (pCurrICCInfo->uiAPDUSendLen-1))
        {
            pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV1;
            HALICC_EnableRecvAfterSend(ucReader);
        }
        break;
    default:
        return;
    }
   // printk("[%s,%d] pCurrICCInfo->aucAPDUSendBuf[%d]:0x%x\n",__func__, __LINE__,pCurrICCInfo->uiAPDUSentLen, pCurrICCInfo->aucAPDUSendBuf[pCurrICCInfo->uiAPDUSentLen]);
    HALICC_SendChar(ucReader,pCurrICCInfo->aucAPDUSendBuf[pCurrICCInfo->uiAPDUSentLen]);
    pCurrICCInfo->uiAPDUSentLen ++;
    if ( pCurrICCInfo->ucAPDUOneByteFlag )
    {
        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV1;
        return;
    }
}

void PROT_CallbackT1SendChar(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;
    /*
        unsigned char aucTimerValue[3];
    */
    pCurrICCInfo = &ICCInfo[ucReader];
    if ( pCurrICCInfo->uiT1FrameSentLen == pCurrICCInfo->uiT1FrameSendLen-1 )
    {
        HALICC_EnableRecvAfterSend(ucReader);
        pCurrICCInfo->uiRecvLen = 0;
        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHRECV1;
    }
    /*
        if( pCurrICCInfo->uiT1FrameSentLen == pCurrICCInfo->uiT1FrameSendLen-1 )
        {
            HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,1);
            //EN: Set BWT Timer
            CONV_ShortHex(aucTimerValue,3,&pCurrICCInfo->uiCWT);
            HALICC_SetTimer(ucReader,0x7C,aucTimerValue[2],aucTimerValue[1],aucTimerValue[0],1);
        }
    */
    HALICC_SendChar(ucReader,pCurrICCInfo->aucT1FrameSendBuf[pCurrICCInfo->uiT1FrameSentLen]);
    pCurrICCInfo->uiT1FrameSentLen ++;
}

void PROT_CallbackRecvChar(unsigned char ucReader,unsigned char ucCh)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    switch ( pCurrICCInfo->ucProtocol)
    {
    case ICCPROTOCOL_T0:
        PROT_CallbackT0RecvChar(ucReader,ucCh);
        break;
    case ICCPROTOCOL_T1:
        PROT_CallbackT1RecvChar(ucReader,ucCh);
        break;
    default:
        LIBICC_CODENOTIMPLEMENT;
        break;
    }
}

void PROT_CallbackT0RecvChar(unsigned char ucReader,unsigned char ucCh)
{
    ICCINFO *pCurrICCInfo;
    unsigned short uiOffset;

    pCurrICCInfo = &ICCInfo[ucReader];
    uiOffset = pCurrICCInfo->uiRecvedLen;
    uiOffset ++;
    if ( uiOffset >= LIBICC_MAXCOMMBUF )
    {
        uiOffset = 0;
    }
    if ( uiOffset == pCurrICCInfo->uiRecvLen )
    {
        /*EN: Over buffer, skip received char */
        return;
    }
    else
    {
        pCurrICCInfo->aucRecvBuf[pCurrICCInfo->uiRecvedLen] = ucCh;
        pCurrICCInfo->uiRecvedLen = uiOffset;
    }
}

void PROT_CallbackT1RecvChar(unsigned char ucReader,unsigned char ucCh)
{
    ICCINFO *pCurrICCInfo;
    unsigned char aucTimerValue[3];

    pCurrICCInfo = &ICCInfo[ucReader];

    //ZYTODO
    //EN: Set CWT Timer
    CONV_ShortHex(aucTimerValue,3,&pCurrICCInfo->uiCWT);
    HALICC_SetTimer(ucReader,0x68,aucTimerValue[2],aucTimerValue[1],aucTimerValue[0],1);

    if (  pCurrICCInfo->uiRecvLen < LIBICC_MAXCOMMBUF )
    {
        pCurrICCInfo->aucRecvBuf[pCurrICCInfo->uiRecvLen] = ucCh;

        pCurrICCInfo->uiRecvLen ++;
//	printk("[%02X] ",ucCh);
    }
    if ( pCurrICCInfo->uiRecvLen > 3 )
    {
        if ( pCurrICCInfo->uiRecvLen == pCurrICCInfo->aucRecvBuf[2]+4 )
        {
            HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,1);
        }
    }
}

void PROT_CallbackTimeout(unsigned char ucReader,unsigned char ucTimeoutType)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    switch ( pCurrICCInfo->ucProtocol)
    {
    case ICCPROTOCOL_T0:
        PROT_CallbackT0Timeout(ucReader,ucTimeoutType);
        break;
    case ICCPROTOCOL_T1:
        PROT_CallbackT1Timeout(ucReader,ucTimeoutType);
        break;
    default:
        LIBICC_CODENOTIMPLEMENT;
        break;
    }
}

void PROT_CallbackT0Timeout(unsigned char ucReader,unsigned char ucTimeoutType)
{
    ICCINFO *pCurrICCInfo;
    unsigned char aucTimerValue[3];
    unsigned long ulTime;

    pCurrICCInfo = &ICCInfo[ucReader];

    HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,1);

    switch (pCurrICCInfo->ucSessionStep)
    {
    case ICCSESSIONSTEP_DATAEXCHSEND1:
        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND2;
        //EN: Set WWT Timer
        CONV_LongHex(aucTimerValue,3,&pCurrICCInfo->ulWWT);
        HALICC_SetTimer(ucReader,0x7C,aucTimerValue[2],aucTimerValue[1],aucTimerValue[0],1);
        HALICC_EnableSend(ucReader);
        return;
    case ICCSESSIONSTEP_DATAEXCHSEND3:
        switch ( pCurrICCInfo->ucStandard )
        {
        case ICCSTANDARD_ISO7816:
        case ICCSTANDARD_EMV:
        case ICCSTANDARD_GJISAM:
        case ICCSTANDARD_GJPSAM:
            //EN: Set WWT Timer
            CONV_LongHex(aucTimerValue,3,&pCurrICCInfo->ulWWT);
            break;
        case ICCSTANDARD_SHEBAO:
            ulTime = ICCPROTOCOLT0_SHEBAO_WWT;
            CONV_LongHex(aucTimerValue,3,&ulTime);
            break;
        default:
            //EN: Set WWT Timer
            CONV_LongHex(aucTimerValue,3,&pCurrICCInfo->ulWWT);
        }
        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND4;
        HALICC_SetTimer(ucReader,0x7C,aucTimerValue[2],aucTimerValue[1],aucTimerValue[0],1);
        HALICC_EnableSend(ucReader);
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
}

void PROT_CallbackT1Timeout(unsigned char ucReader,unsigned char ucTimeoutType)
{
    ICCINFO *pCurrICCInfo;
    unsigned long ulBWT;
    unsigned char aucTimerValue[3];

    pCurrICCInfo = &ICCInfo[ucReader];

    HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,1);
    switch (pCurrICCInfo->ucSessionStep)
    {
    case ICCSESSIONSTEP_DATAEXCHSEND1:
        pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_DATAEXCHSEND2;
        //EN: Set BWT Timer
        ulBWT = pCurrICCInfo->ulBWT * pCurrICCInfo->ucWTX;
        ulBWT+=20;
        CONV_LongHex(aucTimerValue,3,&ulBWT);
        HALICC_SetTimer(ucReader,0x7C,aucTimerValue[2],aucTimerValue[1],aucTimerValue[0],1);
        pCurrICCInfo->ucWTX = 1;
        HALICC_EnableSend(ucReader);
        return;
    case ICCSESSIONSTEP_DATAEXCHSEND2:
        pCurrICCInfo->ucLastError = LIBICCERR_APDUSENDTIMEOUT;
        break;
    case ICCSESSIONSTEP_DATAEXCHRECV1:
        pCurrICCInfo->ucLastError = LIBICCERR_APDURECVTIMEOUT2;
        break;
    default:
        pCurrICCInfo->ucLastError = LIBICCERR_APDUEXCHUNKNOWN;
        break;
    }
}

void PROT_CallbackCardRemove(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    switch ( pCurrICCInfo->ucProtocol)
    {
    case ICCPROTOCOL_T0:
        PROT_CallbackT0CardRemove(ucReader);
        break;
    case ICCPROTOCOL_T1:
        PROT_CallbackT1CardRemove(ucReader);
        break;
    default:
        LIBICC_CODENOTIMPLEMENT;
        break;
    }
}

void PROT_CallbackT0CardRemove(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    pCurrICCInfo->ucLastError = LIBICCERR_CARDREMOVE;
}

void PROT_CallbackT1CardRemove(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
//    if( pCurrICCInfo->ucSessionStep != ICCSESSIONSTEP_IDLE )
    {
        pCurrICCInfo->ucLastError = LIBICCERR_CARDREMOVE;
    }
}

void PROT_CallbackFault(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
//    if( pCurrICCInfo->ucSessionStep != ICCSESSIONSTEP_IDLE )
    {
        pCurrICCInfo->ucLastError = LIBICCERR_VCCPROBLEM;
//        PROT_T0FinishedProc(ucReader);
    }
}

void PROT_CallbackParityError(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
    switch ( pCurrICCInfo->ucProtocol)
    {
    case ICCPROTOCOL_T0:
        PROT_CallbackT0ParityError(ucReader);
        break;
    case ICCPROTOCOL_T1:
        PROT_CallbackT1ParityError(ucReader);
        break;
    default:
        LIBICC_CODENOTIMPLEMENT;
        break;
    }
}

void PROT_CallbackT0ParityError(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
//    if( pCurrICCInfo->ucSessionStep != ICCSESSIONSTEP_IDLE )
    {
        pCurrICCInfo->ucLastError = LIBICCERR_APDUPARITYERROR;
        HALICC_EnableRecv(ucReader);
//        PROT_T0FinishedProc(ucReader);
    }
}

void PROT_CallbackT1ParityError(unsigned char ucReader)
{
    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];
//    if( pCurrICCInfo->ucSessionStep != ICCSESSIONSTEP_IDLE )
    {
        pCurrICCInfo->ucLastError = LIBICCERR_APDUPARITYERROR;
//        PROT_T1FinishedProc(ucReader);
    }
}

