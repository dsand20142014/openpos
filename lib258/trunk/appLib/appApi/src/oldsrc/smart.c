/*******************************************************************************
*IC卡应用层接口
*
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "osicc.h"
#include <stddef.h>
/* sand_tda8007.c */
#include <asm/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/signal.h>
#include <termios.h>
#include <errno.h>
#include <linux/ioctl.h>


#include "smart.h"

extern ISO7816IN	Iso7816In;
extern ISO7816OUT	Iso7816Out;


ICC_ORDER IccOrder;
union ORDER_IN_TYPE OrderType;
union AS_ORDER_TYPE* pAsType;
SYNC_ORDER_TYPE* pSyncOrder;
ICC_ANSWER* pAnswer;


#if 1
unsigned char SMART_ISO(
    unsigned char ucReader,
    unsigned char ucCardType,
    unsigned char* pucIn,
    unsigned char* pucOut,
    unsigned short* puiLengthOut,
    unsigned char ucOrderType)
{
    unsigned char ucLength;
    unsigned short uiAdd;

    /* Order to execute */
    IccOrder.ptout = pucOut;
    IccOrder.pt_order_in = &OrderType;
    if ( ucCardType == SMART_ASYNC )
    {
        IccOrder.order = ASYNC_ORDER;
        IccOrder.pt_order_in->async_order.order_type = ucOrderType;
        pAsType = &IccOrder.pt_order_in->async_order.as_order;
        IccOrder.pt_order_in->async_order.NAD = 0x00;
        switch ( ucOrderType )
        {
        case TYPE0:
            pAsType->order_type0.Lc = *pucIn;       /* Length of Data transmitted */
            pAsType->order_type0.ptin = pucIn+1;    /* Data transmitted           */
            break;
        case TYPE1:
            pAsType->order_type1.CLA = *pucIn;
            pAsType->order_type1.INS = *(pucIn+1);
            pAsType->order_type1.P1  = *(pucIn+2);
            pAsType->order_type1.P2  = *(pucIn+3);
            break;
        case TYPE2:
            pAsType->order_type2.CLA = *pucIn;
            pAsType->order_type2.INS = *(pucIn+1);
            pAsType->order_type2.P1  = *(pucIn+2);
            pAsType->order_type2.P2  = *(pucIn+3);
            pAsType->order_type2.Le  = *(pucIn+4);  /* Length of data received   */
            break;
        case TYPE3:
            pAsType->order_type3.CLA = *pucIn;
            pAsType->order_type3.INS = *(pucIn+1);
            pAsType->order_type3.P1  = *(pucIn+2);
            pAsType->order_type3.P2  = *(pucIn+3);
            pAsType->order_type3.Lc  = *(pucIn+4);  /* Length of data transmitted*/
            pAsType->order_type3.ptin = pucIn+5;    /* Data transmitted          */
            break;
        case TYPE4:
            pAsType->order_type4.CLA = *pucIn;
            pAsType->order_type4.INS = *(pucIn+1);
            pAsType->order_type4.P1  = *(pucIn+2);
            pAsType->order_type4.P2  = *(pucIn+3);
            pAsType->order_type4.Lc  = *(pucIn+4);  /* Length of data transmitted*/
            pAsType->order_type4.ptin = pucIn+5;    /* Data transmitted          */
            /* Length of data received   */
            pAsType->order_type4.Le  = *(pucIn+5+ *(pucIn+4));
            break;
        default:
            Uart_Printf("SMART_CARD_ERROR!\n");
            return (SMART_CARD_ERROR);
        }
    }
    else
    {
        IccOrder.order = SYNC_ORDER;
        pSyncOrder = &IccOrder.pt_order_in->sync_order;

        switch ( *pucIn )                               /* Address            */
        {
        case 0x05:                                      /* SLE4418 ou SLE4428 */
        case 0x06:                                      /* SLE4432 ou SLE4442 */
            uiAdd = (*(pucIn+2) << 8) | (*(pucIn+3) & 0x00FF);
            uiAdd *= 8;                                 /* Convert to bits    */
            pSyncOrder->ADDH = uiAdd >> 8;             /* P1                 */
            pSyncOrder->ADDL = uiAdd & 0x00FF;         /* P2                 */
            break;
        default:                                        /* Other cards        */
            pSyncOrder->ADDH = *(pucIn+2);             /* P1                 */
            pSyncOrder->ADDL = *(pucIn+3);             /* P2                 */
            break;
        }

        switch ( *pucIn )                               /* Data lenght        */
        {
        case 0x04:                                      /* GFM2K              */
        case 0x05:                                      /* SLE4418 ou SLE4428 */
        case 0x06:                                      /* SLE4432 ou SLE4442 */
            ucLength = *(pucIn+4);
            ucLength *= 8;                              /* Length * 8         */
            break;
        default:                                        /* Other cards        */
            ucLength = *(pucIn+4);
            break;
        }

        switch ( ucOrderType )
        {
        case TYPE2:
            pSyncOrder->card_type = *pucIn;     /* Synchronous Type           */
            pSyncOrder->INS = *(pucIn+1);       /* INS                        */
            /* P1                         */
            /* P2                         */
            pSyncOrder->Len = ucLength;         /* Length of data received    */
            break;
        case TYPE3:
            pSyncOrder->card_type = *pucIn;     /* Synchronous Type           */
            pSyncOrder->INS = *(pucIn+1);       /* INS                        */
            /* P1                         */
            /* P2                         */
            pSyncOrder->Len = ucLength;         /* Length of data transmitted */
            pSyncOrder->ptin = pucIn+4;         /* Data transmitted           */
            break;
        default:
            return (SMART_CARD_ERROR);
        }
    }

    /* Send Command */
    pAnswer = OSICC_Command (ucReader, &IccOrder);

#if 0
    /* Answer Command */
    /******************/
    if ( pAnswer->drv_status != OK )
    {
        return (SMART_DRIVER_ERROR);
    }
    if ( pAnswer->card_status != ASY_OK )
    {
        return (SMART_CARD_ERROR);
    }

    if ( pAnswer->Len > *puiLengthOut )
    {
        return (SMART_OWERFLOW);
    }
#endif
    memcpy(pucOut, IccOrder.ptout, pAnswer->Len);
    *puiLengthOut = pAnswer->Len;

    return (SMART_OK);
}
#endif

#if 0
/*
1|?ü?èê?￡oIC?¨èè?′??
è??ú2?êy￡oucReader?a?áD′?¨????, pucReset?a?′??ó|′eD??￠, puiLength?aó|′e3¤?è
·μ???μ￡ooˉêy?′DD×′ì?
*/
unsigned char TaxSMART_WarmReset(
    unsigned char ucReader,
    unsigned char * pucReset,
    unsigned short * puiLength)
{
    unsigned char ucResult,buf[30],ucI;
    /* Power On */
    IccOrder.ptout = pucReset;
    IccOrder.order = WARM_RESET;

    /* Send Command */
    pAnswer = Os__ICC_command (ucReader, &IccOrder);

    /* Answer Command */
    if ( pAnswer->drv_status != OK )
    {
        return (SMART_DRIVER_ERROR);
    }

#ifdef SMART_RESET
    Uart_Printf("TaxSMART_WarmReset pAnswer->card_status =%02x\n",pAnswer->card_status);
#endif

    switch ( pAnswer->card_status )
    {

#ifdef SMART_RESET
        memset(buf,0,sizeof(buf));
        Uart_Printf("TaxSMART_WarmReset POWER_ON>>>>> %02x\n",strlen((char *)IccOrder.ptout));
        for (ucI=0; ucI<pAnswer->Len; ucI++)
            Uart_Printf("%02x ",*(IccOrder.ptout+ucI));
        Uart_Printf("\n");
#endif

    case ICC_ASY:
        ucResult = SMART_OK;
        break;
    case 0x38:
        ucResult = TIMEOUTERR;
        break;
    case 0x11:
        ucResult = TSERR;
        break;
    case 0x01:
        if (ucReader)
            ucResult =  NoSIMcardErr;
        else
            ucResult =  NoCPUcardErr ;
        break;
    case 0x2e:
        ucResult = TCKERR;
        break;
    case 0x2a:
        ucResult = TCKERR;
        break;
    case 0x29:
        ucResult = TC2ERR;
        break;
    default:
        ucResult =  pAnswer->card_status;
        break;
    }
    if ( pAnswer->Len > *puiLength )
    {
        ucResult = SMART_OWERFLOW;
    }
    *puiLength = pAnswer->Len;
    //if(ucResult != SMART_OK)
    return (ucResult);
}
#endif


/*
基于PAD200的移植实现
*
*输入：unsigned char ucReader
*输出：unsigned char *pucReset
*输出：unsigned char *pucLen
*输出：unsigned char ucRate
*/
unsigned char SMART_Reset(unsigned char ucReader,unsigned char *pucReset,
                          unsigned char *pucLen,unsigned char ucRate)
{
    unsigned char ucREGHSR;
    unsigned char ucResult;
    unsigned char ucPresent;

    ICCINFO *pCurrICCInfo;

    pCurrICCInfo = &ICCInfo[ucReader];

    ucPresent = OSICC_Detect(ucReader);

    ucResult = OSICC_ChangeReader(ucReader);


    if ( !ucResult )
        ucResult = ATR_ColdReset(ucReader,ucRate);

    if ( ucResult == LIBICCERR_ISSYNCCARD )
        pCurrICCInfo->ucStatus = ICCSTATUS_ISSYNCCARD;

#if 1//何用？
    memcpy( pucReset,aucCardRecvBuf,uiRecvedLen );
    *pucLen = uiRecvedLen;
#endif

    return( ucResult );
}


#if 1
//come from PAD2000
unsigned char SMART_Command(unsigned char ucReader,unsigned char *pucSendBuf,unsigned char ucLen,
                            unsigned char *pucRecvBuf,unsigned char *pucLen)
{
    unsigned char ucResult;

    memcpy( aucCardSendBuf,pucSendBuf,ucLen );
    ucCardSendLen = ucLen;
    Uart_Printf("%s ucCardSendLen=%d\n", __FUNCTION__, ucCardSendLen);
    ucResult = PROT_AsyncCommand(ucReader);
    memcpy( pucRecvBuf,aucCardRecvBuf,ucCardRecvLen );
    *pucLen = ucCardRecvLen;
    return( ucResult );
}
#endif

//压缩BCD码转换为十六进制
//89860099--->0x89,0x86,0x00,0x99
int StrToEBCD(char *buf,char *ucBuffer,int   BufLen)
{
    unsigned   char   temp1,temp2;
    int   Len=BufLen/2,i;

    for   (i=0; i<Len; i++)
    {
        temp1=buf[i*2];
        if   (temp1>='a')
            temp1=temp1   -   'a'   +   10;
        if   (temp1>='A')
            temp1=temp1   -   'A'   +   10;
        if   (temp1>='0')
            temp1=temp1-'0';


        temp2=buf[i*2   +   1];
        if   (temp2>='a')
            temp2=temp2   -   'a'   +   10;
        if   (temp2>='A')
            temp2=temp2   -   'A'   +   10;
        if   (temp2>='0')
            temp2=temp2-'0';
        ucBuffer[i]=((temp1&0x0f)<<4)|(temp2&0x0f);
    }
    return   0;
}

int udelay(int v)
{
    int times=50;

    while (times--)
        while (v--);
}

/*
选择文件并读取内容
APDU:00A40400055449443031(Name:SELECT FILE,CLA:00,INS:A4,P1:04,P2:00,Lc:05,DATA:5449443031)
Response:('6113'还有19字节需要返回)
APDU:00C0000013(Name:GET RESPONSE,CLA:00,INS:C0,P1:00,P2:00,Le:13)
Response:6F0984055449443031A5001000000000000003('9000'正确执行)
APDU:00A4000002EF02(Name:SELECT FILE,CLA:00,INS:A4,P1:00,P2:00,Lc:02,DATA:EF02)
Response:('9000'正确执行)
APDU:00B00000DD(Name:READ BINARY,CLA:00,INS:B0,P1:00,P2:00,Le:DD)
Response:010110000000000000032006111020071110010101D0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FBD0C2C4C9CBB0BBA7C3FB('9000'正确执行)
*/
//#define DATA_TEST
int smart_main(char argc, char *argv[])
{


    unsigned char ucResult, aucRcvBuf[50], aucSendBuf[50],ucLen, ucRcvLen, tmp[50];
    unsigned char ucReader,ucType,ucI,ucTemp=0;
    unsigned long ulI;
    unsigned char ret, readerID;
    unsigned char cmd_len, i, end=0, ch[2];
    int ii, times=0;//计数

    if (argc==2)
        ii = atoi(argv[1]);
    else
        ii = 0x00;

    Uart_Printf("readID=%d\n", ii);

    readerID=ii;


    ret = OSICC_init();
    if (!ret)
    {
        Uart_Printf("open dev failed!\n");
        return -1;
    }
    OSICC_Detect(ii);

    SMART_Reset(readerID, aucRcvBuf, &ucLen, 1);
    while (!OSICC_Detect(ii));
    return 0;
#if 1
    if (readerID==0x01 || readerID==0x03)
    {
        //SAM01 税控卡
        Uart_Printf("to select sam01\n");
        ret=PBOC_SelectFile(1, readerID, "TSAM01", 6);
        Uart_Printf("SelectFile ret=%d\n", ret);
    }
    else
    {
        //TID01 用户卡
        Uart_Printf("to select TID01\n");
        ret=PBOC_SelectFile(1, readerID, "TID01", 5);
        Uart_Printf("SelectFile ret=%d\n", ret);
    }


    Uart_Printf("to select EF02\n");
    ret=PBOC_SelectFile(0, readerID, "\xEF\x02", 2);
    Uart_Printf("SelectFile ret=%d\n", ret);


    Uart_Printf("to read EF02\n");
    ret = PBOC_ISOReadBinarybySFI(readerID, 0x00, 0x00, 0xdd);
    Uart_Printf("PBOC_ISOReadBinarybySFI ret=%d\n", ret);
#endif

    /*
        ret=PBOC_SelectFile(1, readerID, "TSAM01", 6);
        Uart_Printf("OK! SelectFile TSAM01 ret=%d\n", ret);


        Uart_Printf("to select EF02\n");
        ret=PBOC_SelectFile(0, readerID, "\xEF\x02", 2);
        Uart_Printf("SelectFile EF02 ret=%d\n", ret);

        Uart_Printf("to readBinaryBySFI EF02\n");
        PBOC_ISOReadBinarybySFI(readerID, 0x00, 0x00, 0xff);
    */


#if 0
    Uart_Printf("to select TID01 with 0x00\n");
    readerID=0x00;
    ret=PBOC_SelectFile(1, readerID, "TID01", 5);
    Uart_Printf("OK! SelectFile TSAM01 ret=%d\n", ret);


    Uart_Printf("to select EF02\n");
    ret=PBOC_SelectFile(0, readerID, "\xEF\x02", 2);
    Uart_Printf("SelectFile EF02 ret=%d\n", ret);
#endif


    /*
            //just more test file
            ret=PBOC_SelectFile(0, readerID, "\xEF\x03", 2);
            Uart_Printf("SelectFile ret=%d\n", ret);
    */

#if 0
    Uart_Printf("PBOC_ISOReadRecordbySFI=%d, 0x01 filedata=======>:\n", ret);
    for (i=0; i<Iso7816Out.uiLen; i++)
    {
        Uart_Printf("%02x", Iso7816Out.aucData[i]);
    }
    Uart_Printf("\n\n");
#endif

#if 0
    ret=PBOC_ISOReadRecordbySFI(readerID,  0x0, 0x02);
    Uart_Printf("PBOC_ISOReadRecordbySFI=%d, 0x02 filedata=======>:\n", ret);
    for (i=0; i<Iso7816Out.uiLen; i++)
    {
        Uart_Printf("%02x", Iso7816Out.aucData[i]);
    }
    Uart_Printf("\n\n");
#endif


#if 0
    //读取EF文件内容
    ret=PBOC_ISOReadBinarybySFI(readerID, 0x03, 0x0, 0xDD);
    Uart_Printf("PBOC_ISOReadBinarybySFI=%d, filedata=======>:\n", ret);
    for (i=0; i<Iso7816Out.uiLen; i++)
    {
        Uart_Printf("%02x", Iso7816Out.aucData[i]);
    }
    Uart_Printf("\n\n");

    sleep(1);

    Uart_Printf("to write \"1234567890\"to 0x02\n");
    ret=PBOC_UpdateBinary(readerID, 0x03, 0, "1234567890", 10, 0);
    Uart_Printf("PBOC_UpdateBinary=%d\n", ret);

    sleep(1);

    Uart_Printf("to read again\n");
    ret=PBOC_ISOReadBinarybySFI(readerID, 0x3, 0x0, 0xDD);
    Uart_Printf("PBOC_ISOReadBinarybySFI=%d, filedata=======>:\n", ret);
    for (i=0; i<Iso7816Out.uiLen; i++)
    {
        Uart_Printf("%02x", Iso7816Out.aucData[i]);
    }
    Uart_Printf("\n\n");
#endif

#if 0
    do
    {
        memset(tmp, 0, 50);
        Uart_Printf("enter APDU string:\n");
        scanf("%s", tmp);

        cmd_len = strlen(tmp);
        StrToEBCD(tmp, aucSendBuf, cmd_len);

        cmd_len=cmd_len/2;
        /*
            Uart_Printf("cmd:(%d)\n", cmd_len);
            for (i=0; i<cmd_len; i++)
            {
                memset(tmp, 0, 50);
                Uart_Printf("enter APDU string:\n");
                scanf("%s", tmp);

                cmd_len = strlen(tmp);
                StrToEBCD(tmp, aucSendBuf, cmd_len);

                cmd_len=cmd_len/2;

                Uart_Printf("cmd:(%d)\n", cmd_len);
                for (i=0; i<cmd_len; i++)
                {
                    Uart_Printf("%02x ", aucSendBuf[i]);
                }
                Uart_Printf("\n");
                SMART_Command(readerID, aucSendBuf, cmd_len, aucRcvBuf, &ucRcvLen);
            }
        */
        SMART_Command(readerID, aucSendBuf, cmd_len, aucRcvBuf, &ucRcvLen);
    }
    while (1);
#endif

out:
#ifndef DATA_TEST
    OSICC_close();
#endif
    return 0;
}

