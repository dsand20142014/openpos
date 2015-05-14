/*
	Test System
--------------------------------------------------------------------------
	FILE  pboc.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-08-04		Xiaoxi Jiang
    Last modified :	2003-08-04		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osicc.h"
#include "smart.h"
#include "libicc.h"

ISO7816IN	Iso7816In;
ISO7816OUT	Iso7816Out;


/*
功能描述：清空输出缓冲区
入口参数：无
返回值：无
*/
void PBOC_SetIso7816Out(void)
{
    Iso7816Out.uiLen = sizeof(Iso7816Out.aucData);
    memset(Iso7816Out.aucData,0,sizeof(Iso7816Out.aucData));
}


/*
功能描述：判断IC卡的返回值
入口参数：ucReader为读卡位置
返回值：函数执行状态
*/
unsigned char PBOC_ISOCheckReturn(unsigned char ucReader)
{
    unsigned char ucResult,ucI;

    Iso7816Out.ucSW1 = Iso7816Out.aucData[Iso7816Out.uiLen-2];
    Iso7816Out.ucSW2 = Iso7816Out.aucData[Iso7816Out.uiLen-1];
#if 1
    Uart_Printf("Iso7816Out.ucSW1=%02x\n",Iso7816Out.ucSW1);
    Uart_Printf("Iso7816Out.ucSW2=%02x\n",Iso7816Out.ucSW2);
#endif

    switch (Iso7816Out.ucSW1)
    {
    case 0x90:
#if 1
        Uart_Printf("PBOC_ISOCheckReturn return %x\n", Iso7816Out.uiLen);
        for (ucI=0;ucI<Iso7816Out.uiLen;ucI++)
            Uart_Printf("%02x ",Iso7816Out.aucData[ucI]);
        Uart_Printf("\n********\n");
#endif
        return(SUCCESS);
    case 0x6C:
        PBOC_SetIso7816Out();
        Iso7816In.ucP3  = Iso7816Out.ucSW2;
        if ( (ucResult = SMART_ISO(ucReader,
                                   SMART_ASYNC,
                                   (unsigned char *)&Iso7816In,
                                   Iso7816Out.aucData,
                                   &Iso7816Out.uiLen,
                                   TYPE2)) != OK)
            return(ucResult);
        return(PBOC_ISOCheckReturn(ucReader));

    case 0x61:
    case 0x9F:
        return(PBOC_ISOGetResponse(ucReader,Iso7816Out.ucSW2));
        //return(UTIL_CheckCardErr(Iso7816Out.ucSW1,Iso7816Out.ucSW2));
    default:
        //return(UTIL_CheckCardErr(Iso7816Out.ucSW1,Iso7816Out.ucSW2));
        break;
    }
}



/*
功能描述：选择IC卡的文件
入口参数：ucReader为读卡位置，pucData为发送给IC卡的数据，ucDataLen为数据长度
返回值：函数执行状态
*/
unsigned char PBOC_SelectFile(unsigned char ucFlag,unsigned char ucReader,
                              unsigned char *pucData,unsigned char ucDataLen)
{
    unsigned char	ucResult;
    unsigned char ucI;
    unsigned char * p ;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0xA4;
    if (ucFlag)
        Iso7816In.ucP1  = 0x04;
    else
        Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 00;
    Iso7816In.ucP3 = ucDataLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucDataLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x00;

#if 1
    p = (unsigned char *)&Iso7816In;
    for (ucI=0;ucI<5+ucDataLen;ucI++)
        Uart_Printf("%02x",p[ucI]);
    Uart_Printf("\n");
#endif

    PBOC_SetIso7816Out();

    // Uart_Printf("=========1 SMART_ISO\n");
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE3)) != OK)
    {
        Uart_Printf("!!!!!!SMART_ISO ret=%d\n", ucResult);
        return(ucResult);
    }
    //Uart_Printf("==========2 SMART_ISO\n");

#if 1
    Uart_Printf("%x\n", Iso7816Out.uiLen);
    for (ucI=0;ucI<Iso7816Out.uiLen+2;ucI++)
        Uart_Printf("%02x ",Iso7816Out.aucData[ucI]);
    Uart_Printf("\n********\n");
#endif

    return(PBOC_ISOCheckReturn(ucReader));
}



/*
功能描述：获取IC卡返回数据
入口参数：ucReader为读卡位置，ucLen为返回的数据长度
返回值：函数执行状态
*/
unsigned char PBOC_ISOGetResponse(unsigned char ucReader,
                                  unsigned char ucLen)
{
    unsigned char ucResult;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0xC0;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3  = ucLen;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE2)) != OK)
        return(ucResult);
    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：根据短文件标志符读取文件记录
入口参数：ucReader为读卡位置，ucSFI为短文件标志符，ucRecordNumber为记录号
返回值：函数执行状态
*/
unsigned char PBOC_ISOReadRecordbySFI(unsigned char ucReader,
                                      unsigned char ucSFI,unsigned char ucRecordNumber)
{
    unsigned char	ucResult;


    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0xB2;
    Iso7816In.ucP1  = ucRecordNumber;
    Iso7816In.ucP2  = (ucSFI << 3) | 0x04;
    Iso7816In.ucP3  = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE2)) != OK)
        return(ucResult);



    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：读取当前文件的记录
入口参数：ucReader为读卡位置，ucSFI为短文件标志符，ucRecordNumber为记录号
返回值：函数执行状态
*/
unsigned char PBOC_ReadRecord(unsigned char ucReader,unsigned char ucRecordNumber)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0xB2;
    Iso7816In.ucP1  = ucRecordNumber;
    Iso7816In.ucP2  = 0x04;
    Iso7816In.ucP3  = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE2)) != OK)
        return(ucResult);
    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：根据短文件标志符读取文件
入口参数：ucReader为读卡位置，ucSFI为短文件标志符，ucOffset为偏移量，ucInLen为读取的数据长度
返回值：函数执行状态
*/
unsigned char PBOC_ISOReadBinarybySFI(unsigned char ucReader,
                                      unsigned char ucSFI,unsigned char ucOffset,unsigned char ucInLen)
{
    unsigned char	ucResult;
    unsigned char ucI;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0xB0;
    Iso7816In.ucP1  = ucSFI; //zy
    //Iso7816In.ucP1  = ucSFI | 0x80;
    Iso7816In.ucP2  = ucOffset;
    Iso7816In.ucP3  = ucInLen;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE2)) != OK)
        return(ucResult);


    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：从SAM卡中取税控卡编号，随机数，mac1
入口参数：ucReader为读卡位置
返回值：函数执行状态
*/
unsigned PBOC_Get_RegisterNb(unsigned char ucReader)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0xC0;
    Iso7816In.ucIns = 0xF0;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3  = 0x10;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE2)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：从用户卡中产生MAC2
入口参数：ucReader为读卡位置，pucData为传给卡的数据，ucLen为数据长度
返回值：函数执行状态
*/
unsigned PBOC_Get_RegisterSign(unsigned char ucReader,
                               unsigned char *pucData,unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0xC0;
    Iso7816In.ucIns = 0xE4;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x04;
    Iso7816In.aucData[Iso7816In.ucP3+1] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE4)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：税控卡注册函数
入口参数：ucReader为读卡位置，pucData为传给卡的数据，ucLen为数据长度
返回值：函数执行状态
*/
unsigned PBOC_Terminal_Register(unsigned char ucReader,
                                unsigned char *pucData,unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0xC0;
    Iso7816In.ucIns = 0xF1;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x08;
    Iso7816In.aucData[Iso7816In.ucP3+1] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE4)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：税控卡解锁函数
入口参数：ucReader为读卡位置，pucData为传给卡的数据，ucLen为数据长度
返回值：函数执行状态
*/
unsigned PBOC_Pin_UNBlock(unsigned char ucReader,
                          unsigned char *pucData,unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0x84;
    Iso7816In.ucIns = 0x24;
    Iso7816In.ucP1  = 0x01;
    Iso7816In.ucP2  = 0x01;
    Iso7816In.ucP3 = 0x0c;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x08;
    Iso7816In.aucData[Iso7816In.ucP3+1] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE4)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：校验税控卡的PIN
入口参数：ucReader为读卡位置，pucData为传给卡的数据，ucLen为数据长度
返回值：函数执行状态
*/
unsigned PBOC_VerifyFiscal_Pin(unsigned char ucReader,
                               unsigned char *pucData,unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0xC0;
    Iso7816In.ucIns = 0xF9;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x08;
    Iso7816In.aucData[Iso7816In.ucP3+1] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE4)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}



/****added by guo.wl*/
//分发用户卡中的发票号，一次分发一卷发票号

/*
功能描述：分发用户卡中的发票号，一次分发一卷发票号
入口参数：ucReader为读卡位置
返回值：函数执行状态
*/
unsigned char PBOC_DitributeInvoice(unsigned char ucReader)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0xC0;
    Iso7816In.ucIns = 0xE9;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3 = 0x16;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE2)) != OK)
        return(ucResult);

    //因为 ISSUE_INVOICE 时 金额超限和 DISTRIBUTE_INVOICE_NB 时 无发票分发 的返回码都是 0x94 + 0x01
    //所以这里先加一个预判断
    Iso7816Out.ucSW1 = Iso7816Out.aucData[Iso7816Out.uiLen-2];
    Iso7816Out.ucSW2 = Iso7816Out.aucData[Iso7816Out.uiLen-1];
    if (Iso7816Out.ucSW1==0x94 && Iso7816Out.ucSW2==0x01)
    {
        return NoInvoiceRoll;
    }
    else
    {
        return(PBOC_ISOCheckReturn(ucReader));
    }
}
//用于税控卡录入发票号

/*
功能描述：用于税控卡录入发票号
入口参数：ucReader为读卡位置，pucData为传给卡的数据，ucLen为数据长度
返回值：函数执行状态
*/
unsigned char PBOC_InputInvoiceNB(unsigned char ucReader,
                                  unsigned char *pucData,unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0xC0;
    Iso7816In.ucIns = 0xF7;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE3)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：校验用户卡的PIN
入口参数：ucReader为读卡位置，pucData为传给卡的数据，
ucPinIndex为索引值，ucLen为数据长度
返回值：函数执行状态
*/
unsigned PBOC_VerifyUser_Pin(unsigned char ucReader,
                             unsigned char *pucData, unsigned char ucPinIndex, unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0x20;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = ucPinIndex;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE3)) != OK)
        return(ucResult);

    //因为 VERIFY 时 密钥类型错误 和 READ BINARY, READ RECORD 时 文件类型错误 的返回码都是 0x69 + 0x81
    //所以这里先加一个预判断
    Iso7816Out.ucSW1 = Iso7816Out.aucData[Iso7816Out.uiLen-2];
    Iso7816Out.ucSW2 = Iso7816Out.aucData[Iso7816Out.uiLen-1];
    if (Iso7816Out.ucSW1==0x69 && Iso7816Out.ucSW2==0x81)
    {
        return KeyTypeErr;
    }
    else
    {
        return(PBOC_ISOCheckReturn(ucReader));
    }
}

/*
功能描述：通过税控卡生成防伪税控码
入口参数：ucReader为读卡位置，pucData为传给卡的数据，ucLen为数据长度
返回值：函数执行状态
*/
unsigned char PBOC_IssueInvoice(unsigned char ucReader,
                                unsigned char *pucData,unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0xC0;
    Iso7816In.ucIns = 0xF2;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x08;
    Iso7816In.aucData[Iso7816In.ucP3+1] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE4)) != OK)
        return(ucResult);

    //因为 ISSUE_INVOICE 时 金额超限和 DISTRIBUTE_INVOICE_NB 时 无发票分发 的返回码都是 0x94 + 0x01
    //所以这里先加一个预判断
    Iso7816Out.ucSW1 = Iso7816Out.aucData[Iso7816Out.uiLen-2];
    Iso7816Out.ucSW2 = Iso7816Out.aucData[Iso7816Out.uiLen-1];
    if (Iso7816Out.ucSW1==0x94 && Iso7816Out.ucSW2==0x01)
    {
        return IssInvoiceValLimitOver;
    }
    else
    {
        return(PBOC_ISOCheckReturn(ucReader));
    }
}

/*
功能描述：通过税控卡，生成日交易签名
入口参数：ucReader为读卡位置，pucData为传给卡的数据，ucLen为数据长度
返回值：函数执行状态
*/
unsigned char PBOC_DailyCollectSign(unsigned char ucReader,
                                    unsigned char *pucData,unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0xC0;
    Iso7816In.ucIns = 0xFB;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x80;
    Iso7816In.aucData[Iso7816In.ucP3+1] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE4)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：更新记录
入口参数：ucReader为读卡位置，ucSFI为短文件标志符，ucRecordNumber为记录号，
pucData为传给卡的数据，ucLen为数据长度
返回值：函数执行状态
*/
unsigned PBOC_UpdateRecord(unsigned char ucReader,
                           unsigned char ucSFI,unsigned char ucRecordNumber,
                           unsigned char *pucData,unsigned char ucLen,unsigned char ucFlag)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0xDC;
    Iso7816In.ucP1  = ucRecordNumber;
    if (ucFlag)
        Iso7816In.ucP2  = (ucSFI << 3) | 0x04;
    else
        Iso7816In.ucP2  = 0x04;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE3)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}


/*
功能描述：从税务管理卡取8字节随机数
入口参数：ucReader为读卡位置，ucLen为返回的数据长度
返回值：函数执行状态
*/
unsigned char PBOC_GetChallenge(unsigned char ucReader,
                                unsigned char ucLen)
{
    unsigned char ucResult;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0x84;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3  = ucLen;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE2)) != OK)
        return(ucResult);
    return(PBOC_ISOCheckReturn(ucReader));
}


/****************************************************************************
* 名称：CARDCMD_UpdateBinary
* 功能：Update Binary 修改二进制文件
* 入口参数：ucReader：卡座号；ucSFI：短文件标示符；usOffset：偏移量
			*pucData：修改数据指针；ucLen：数据长度；ucFlag：是否按短文件标示符
* 出口参数：0：正常；非0：异常
****************************************************************************/
unsigned char PBOC_UpdateBinary(unsigned char ucReader,unsigned char ucSFI,
                                unsigned short usOffset,unsigned char *pucData,unsigned char ucLen,unsigned char  ucFlag)
{
    unsigned char ucResult;
    unsigned char aucBuf[2];
    unsigned short uiI;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0xD6;

    if ( ucFlag )
    {
        Iso7816In.ucP1 = ucSFI | 0x80;
        Iso7816In.ucP2 = usOffset;
    }
    else
    {
        short_tab(&Iso7816In.ucP1,2,&usOffset);
        Uart_Printf("Iso7816In.ucP1=%02x\n",Iso7816In.ucP1);
        Uart_Printf("Iso7816In.ucP2=%02x\n",Iso7816In.ucP2);
    }

    Uart_Printf("***************PBOC*************************\n");
    Uart_Printf("ucLen=%d\n",ucLen);
    for (uiI=0;uiI<ucLen;uiI++)
    {
        Uart_Printf("%02x ",pucData[uiI]);
        if (uiI%30==0)
            Uart_Printf("\n");
    }
    Uart_Printf("\n");


    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE3)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}


/*
功能描述：通过用户卡进行数据申报
入口参数：ucReader为读卡位置，pucData为传给卡的数据，ucLen为数据长度
返回值：函数执行状态
*/
unsigned PBOC_DeclareDuty(unsigned char ucReader,
                          unsigned char *pucData,unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0xC0;
    Iso7816In.ucIns = 0xF4;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x8E;
    Iso7816In.aucData[Iso7816In.ucP3+1] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE4)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：将申报数据写入用户卡
入口参数：ucReader为读卡位置，pucData为传给卡的数据，ucLen为数据长度
返回值：函数执行状态
*/
unsigned PBOC_DataCollect(unsigned char ucReader,
                          unsigned char *pucData,unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0xC0;
    Iso7816In.ucIns = 0xE6;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE3)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：更新监控回读信息
入口参数：ucReader为读卡位置，pucData为传给卡的数据，ucLen为数据长度
返回值：函数执行状态
*/
unsigned PBOC_UpdateControls(unsigned char ucReader,
                             unsigned char *pucData,unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0xC0;
    Iso7816In.ucIns = 0xF6;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = 0x00;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE3)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}


/*
功能描述：卡内部认证
入口参数：ucReader为读卡位置，pucData为传给卡的数据，
ucPinIndex为索引值，ucLen为数据长度
返回值：函数执行状态
*/
unsigned PBOC_InternalAuthentication(unsigned char ucReader,
                                     unsigned char *pucData, unsigned char ucPinIndex, unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0x88;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = ucPinIndex;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x00;//0x08;
    Iso7816In.aucData[Iso7816In.ucP3+1] = 0x00;
    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE4)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}

/*
功能描述：卡外部认证
入口参数：ucReader为读卡位置，pucData为传给卡的数据，
ucPinIndex为索引值，ucLen为数据长度
返回值：函数执行状态
*/
unsigned char PBOC_ExternalAuthentication(unsigned char ucReader,
        unsigned char *pucData, unsigned char ucPinIndex, unsigned char ucLen)
{
    unsigned char	ucResult;

    Iso7816In.ucCla = 0x00;
    Iso7816In.ucIns = 0x82;
    Iso7816In.ucP1  = 0x00;
    Iso7816In.ucP2  = ucPinIndex;
    Iso7816In.ucP3 = ucLen;
    memcpy(&Iso7816In.aucData[0],pucData,ucLen);
    Iso7816In.aucData[Iso7816In.ucP3] = 0x00;

    PBOC_SetIso7816Out();
    if ( (ucResult = SMART_ISO(ucReader,
                               SMART_ASYNC,
                               (unsigned char *)&Iso7816In,
                               Iso7816Out.aucData,
                               &Iso7816Out.uiLen,
                               TYPE3)) != OK)
        return(ucResult);

    return(PBOC_ISOCheckReturn(ucReader));
}

#if 0
/*
功能描述：检查IC卡返回信息，测试IC卡兼容性时使用
入口参数：ucReader为读卡位置
返回值：函数执行状态
*/
extern unsigned char respone_data[256];
unsigned char PBOC_ISOCheckReturn_test(unsigned char ucReader)
{
    unsigned char ucResult,ucI,buf[100];

    memset(buf,0,sizeof(buf));
    Iso7816Out.ucSW1 = Iso7816Out.aucData[Iso7816Out.uiLen-2];
    Iso7816Out.ucSW2 = Iso7816Out.aucData[Iso7816Out.uiLen-1];

    Uart_Printf("Iso7816Out.ucSW1=%02x\n",Iso7816Out.ucSW1);
    Uart_Printf("Iso7816Out.ucSW2=%02x\n",Iso7816Out.ucSW2);

    display_array(respone_data, respone_data[3]+2);
    Os__clr_display(255);
    Os__display_recvdata(0, 2,  Iso7816Out.aucData, Iso7816Out.uiLen);
    MSG_WaitKey(3);

    switch (Iso7816Out.ucSW1)
    {
    case 0x90:
        if (Iso7816Out.ucSW2 == 0x60)
            return UNKNOWERR;
        return(SUCCESS);
    case 0x6C:
        PBOC_SetIso7816Out();
        Iso7816In.ucP3  = Iso7816Out.ucSW2;
        if ( (ucResult = SMART_ISO(ucReader,
                                   SMART_ASYNC,
                                   (unsigned char *)&Iso7816In,
                                   Iso7816Out.aucData,
                                   &Iso7816Out.uiLen,
                                   TYPE2)) != OK)
            return(ucResult);
        return(PBOC_ISOCheckReturn_test(ucReader));
    case 0x61:
    case 0x9F:
        if (Iso7816Out.ucSW2 == 0xAF)
        {
            return UNKNOWERR;
        }
        else
            return(PBOC_ISOGetResponse(ucReader,Iso7816Out.ucSW2));
    case 0x60:
        if (Iso7816Out.ucSW2 == 0x60)
            return(SUCCESS);
    default:
        return(UTIL_CheckCardErr(Iso7816Out.ucSW1,Iso7816Out.ucSW2));
    }
}

#endif

