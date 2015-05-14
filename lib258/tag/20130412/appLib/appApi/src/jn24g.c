/*锦诺2.4G读卡器支持*/
#include <stdio.h>



#include <string.h>
#define JN24GDATA
#include "jn24g.h"
#undef JN24GDATA

static int fd_jn24;

#if 0
//define function process table
const JN24GCMDTBL JN24G_CmdTable[] =
{
    { JN24G_POS_CMD_COMTEST,	    JN24G_ComTest },
    { JN24G_POS_CMD_CHECKCARD,    	JN24G_CheckCard },
    { JN24G_POS_CMD_TRANSFER,		JN24G_TransFer },
    { JN24G_POS_CMD_ENDTRANS,		JN24G_EndTransFer },
    { JN24G_POS_CMD_READERCFG,		JN24G_ReaderCfg },
    { JN24G_POS_CMD_CARDCFG,		JN24G_CardCfg }
};


const unsigned int uiMaxJN24GCmd = sizeof(JN24G_CmdTable)/sizeof(JN24GCMDTBL);
#endif
unsigned char JN24G_CalcLRC( unsigned char *pucData,unsigned int uiLen )
{
    unsigned char ucLrc;
    unsigned int uiI;

    ucLrc = 0;
    for ( uiI=0; uiI<uiLen; uiI++ )
    {
        ucLrc ^= pucData[uiI];
    }
    return ucLrc;
}

unsigned char JN24G_CheckRev(int fd, unsigned int uiWaitTime )
{
    unsigned char ucI,ucChar,aucData[256];
    unsigned int uiI,uiRevLen,uiJ;
    JN24G_REVCMD *pRevCmd;
    JN24G_SENDCMD *pSendCmd;
    int ucStatus;

    pRevCmd = &JN24gRevCmd;
    pSendCmd = &JN24gSendCmd;
    memset( aucData,0,sizeof(aucData) );

    Uart_Printf("*********** %s %d**************\n", __func__, __LINE__);

    ucStatus = OSUART_tty_RecvByte(fd, &ucChar,1);	//STX
    if ( !ucStatus )
        return JN24GERR_WAIT;


    if ( ucChar!=0x02 )
        return JN24GERR_WAIT;

    uiI = 0;
    aucData[uiI++] = ucChar;
    ucStatus = OSUART_tty_RecvByte( fd, &ucChar,1 );//从机地址0x01
    if ( !ucStatus )
        return JN24GERR_WAIT;

    aucData[uiI++] = ucChar;

    for ( ucI=0; ucI<3; ucI++ )
    {
        ucStatus = OSUART_tty_RecvByte(fd,  &ucChar,1 );//Recv Length
        if ( !ucStatus )
            return JN24GERR_WAIT;
        aucData[uiI++] = ucChar;
    }

    uiRevLen = aucData[2]*100 + aucData[3]*10 + aucData[4];		// 长度数据转换
    if ( uiRevLen+7 > sizeof(pRevCmd->aucData) )
        return JN24GERR_LENGTH;

    for ( uiJ=0; uiJ < uiRevLen+2; uiJ++ )
    {
        ucStatus = OSUART_tty_RecvByte(fd,  &ucChar,1);
        if (!ucStatus )
            return JN24GERR_WAIT;
        aucData[uiI++] = ucChar;
    }

    pRevCmd->uiLen = uiRevLen + 7;
    memcpy( pRevCmd->aucData,aucData,uiRevLen+7 );
    ucStatus = JN24GERR_SUCCESS;

    return 0x00;
}

int  JN24G_WaitRevEnd( int fd, unsigned int uiTimeDelay )
{
    int ucStatus;
    unsigned int uiCount;

    uiCount = uiTimeDelay/10;
    do
    {
        ucStatus = JN24G_CheckRev(fd,  100 );
        if (ucStatus==0x00) break;
        if ( uiCount )
            uiCount--;
    }
    while ( (ucStatus==JN24GERR_WAIT)&&uiCount );

    if ( (uiCount==0) )
        ucStatus = -1;

    return ucStatus;
}

#if 0
void JN24G_SendCmd( unsigned char ucAddr,unsigned char ucCmd,unsigned char *pucSendData,unsigned int uiSendLen )
{
    JN24G_SENDCMD *pSendCmd;
    JN24G_REVCMD *pRevCmd;

    UART1_RxFlush();
    UART1_TxFlush();
    pRevCmd = &JN24gRevCmd;
    memset( pRevCmd,0,sizeof(JN24G_REVCMD) );
    pSendCmd = &JN24gSendCmd;
    pSendCmd->ucAddr = ucAddr;
    memcpy( &pSendCmd->aucSendData[0],pucSendData,uiSendLen );
    pSendCmd->uiSendLen = uiSendLen;
    UART1_SendData( pSendCmd->aucSendData,pSendCmd->uiSendLen );
}

unsigned char JN24G_ComTest( unsigned char ucAddr,unsigned char *pucInData,unsigned int uiInLen )
{
    unsigned char ucStatus;

    JN24G_SendCmd( ucAddr,JN24G_CMD_COMTEST,pucInData,uiInLen );
    //ucStatus = JN24G_WaitRevEnd( 50 );

    return ucStatus;
}

unsigned char JN24G_ComTest1( unsigned char ucAddr,unsigned char *pucInData,unsigned int uiInLen )
{
    unsigned char ucStatus;
    JN24G_REVCMD *pRevCmd;

    pRevCmd = &JN24gRevCmd;
    memset( pRevCmd,0,sizeof(JN24G_REVCMD) );
    JN24G_SendCmd( ucAddr,JN24G_CMD_COMTEST,pucInData,uiInLen );
    //ucStatus = JN24G_CheckRev( 30 );
    if ( !ucStatus )
    {
        if ( memcmp(pRevCmd->aucData,(unsigned char *)"\x02\x01\x00\x00\x03\x00\x00\x00\x03\x00",10) )
            ucStatus = 1;
    }

    return ucStatus;
}

unsigned char JN24G_CheckCard( unsigned char ucAddr,unsigned char *pucInData,unsigned int uiInLen )
{
    unsigned char ucStatus = 0;

    JN24G_SendCmd( ucAddr,JN24G_CMD_CHECKCARD,pucInData,uiInLen );
    //ucStatus = JN24G_WaitRevEnd( 50 );

    return ucStatus;
}

unsigned char JN24G_TransFer( unsigned char ucAddr,unsigned char *pucInData,unsigned int uiInLen )
{
    unsigned char ucStatus = 0;

    JN24G_SendCmd( ucAddr,JN24G_CMD_TRANSFER,pucInData,uiInLen );
    //ucStatus = JN24G_WaitRevEnd( 50 );

    return ucStatus;
}

unsigned char JN24G_EndTransFer( unsigned char ucAddr,unsigned char *pucInData,unsigned int uiInLen )
{
    unsigned char ucStatus = 0;

    JN24G_SendCmd( ucAddr,JN24G_CMD_ENDTRANS,pucInData,uiInLen );
    //ucStatus = JN24G_WaitRevEnd( 50 );

    return ucStatus;
}

unsigned char JN24G_ReaderCfg( unsigned char ucAddr,unsigned char *pucInData,unsigned int uiInLen )
{
    unsigned char ucStatus = 0;

    JN24G_SendCmd( ucAddr,JN24G_CMD_READERCFG,pucInData,uiInLen );
    //ucStatus = JN24G_WaitRevEnd( 50 );

    return ucStatus;
}

unsigned char JN24G_CardCfg( unsigned char ucAddr,unsigned char *pucInData,unsigned int uiInLen )
{
    unsigned char ucStatus = 0;

    JN24G_SendCmd( ucAddr,JN24G_CMD_CARDCFG,pucInData,uiInLen );
    //ucStatus = JN24G_WaitRevEnd( 50 );

    return ucStatus;
}

unsigned char JN24G_CheckCardEx( unsigned char *pucOutData )
{
    unsigned char ucStatus = 0;
    unsigned char aucInData[10];
    unsigned int uiInLen = 10;
    JN24G_REVCMD *pRevCmd;

    pRevCmd = &JN24gRevCmd;
    memset( pRevCmd,0,sizeof(JN24G_REVCMD) );
//	先发12指令；
    memset( aucInData,0x00,sizeof(aucInData) );
    memcpy( aucInData,"\x02\x01\x00\x00\x01\x12\x03\x12",8 );
    JN24G_SendCmd( 0x02,JN24G_POS_CMD_CHECKCARD,aucInData,uiInLen );
    //ucStatus = JN24G_CheckRev( 20 );
    if ( ucStatus||(pRevCmd->aucData[5]!=0x12) )
        return 0xFF;
//	发10指令；
    memset( aucInData,0x00,sizeof(aucInData) );
    memcpy( aucInData,"\x02\x01\x00\x00\x01\x10\x03\x10",8 );
    JN24G_SendCmd( 0x02,JN24G_POS_CMD_CHECKCARD,aucInData,uiInLen );
    //ucStatus = JN24G_CheckRev( 50 );
    if ( (!ucStatus)&&(pRevCmd->aucData[5]==0x10) )
        memcpy( pucOutData,&pRevCmd->aucData[6],4 );
    else
        return 0xFF;

    return ucStatus;
}

unsigned char JN24G_Proc_OLD( unsigned char ucAddr,unsigned char ucCmd,unsigned char *pucInData,unsigned int uiInLen,
                              unsigned char *pucRetData,unsigned int *puiRetLen )
{
    unsigned char ucStatus,ucI;
    JN24G_REVCMD *pRevCmd;

    pRevCmd = &JN24gRevCmd;
    memset( pRevCmd,0,sizeof(JN24gRevCmd) );
    *puiRetLen = 0;
    for ( ucI=0; ucI<uiMaxJN24GCmd; ucI++ )
    {
        if ( ucCmd==JN24G_CmdTable[ucI].ucCmd )
        {
            if ( JN24G_CmdTable[ucI].pFct )
            {
                ucStatus = JN24G_CmdTable[ucI].pFct( ucAddr,pucInData,uiInLen );
                pucRetData[0] = ucStatus;
                memcpy( &pucRetData[1],pRevCmd->aucData,pRevCmd->uiLen );
                *puiRetLen = pRevCmd->uiLen+1;
                ucStatus = JN24GERR_SUCCESS;
            }
            else
            {
                ucStatus = JN24GERR_COMMAND;
            }
            break;
        }
    }

    if (ucI == uiMaxJN24GCmd)
    {
        ucStatus = JN24GERR_COMMAND;
    }

    return ucStatus;
}
#endif

int JN24_init(void)
{
    fd_jn24 = OSUART_tty_open(0);
    if (fd_jn24<0)
    {
        Uart_Printf("!!!!! open jn24 serial port failed!\n");
        return -1;
    }
    return fd_jn24;
}




int JN24_Proc(unsigned char *pin, int in_len, unsigned char *pout, int *out_len)
{
    int fd, i=0, ret=0;
    JN24G_REVCMD *pRevCmd;

    pRevCmd = &JN24gRevCmd;
    memset( pRevCmd,0,sizeof(JN24gRevCmd) );

    fd = JN24_init();

#if 1
    Uart_Printf("\n\nS:------------------>\n");
    for (i=0; i<in_len; i++)
    {
        Uart_Printf("%02x ", pin[i]);
    }
    Uart_Printf("\n");
#endif
    ret = OSUART_tty_SendByte(fd, pin, in_len);
    if (ret<0)
    {
        Uart_Printf("%s sendbyte failed!(fd=%x)\n", __func__, fd);
        return fd;
    }

    //增加了对超时的处理
    ret = JN24G_WaitRevEnd(fd, 10);
    if (ret<0)
    {
        Uart_Printf("R: !!!!!!!!! failed!!!!!!!!!!!\n");
        return -1;
    }

    memcpy(pout, pRevCmd->aucData, pRevCmd->uiLen);
    pout[pRevCmd->uiLen]=0x0;
    *out_len = pRevCmd->uiLen;

#if 1
    Uart_Printf("R:----------%x-------->\n", pRevCmd->uiLen);
    for (i=0; i<pRevCmd->uiLen; i++)
    {
        Uart_Printf("%02x ", pout[i]);
    }
    Uart_Printf("\n");
#endif



out:
    JN24_close();

    if (pout[5] & 0x80)
        return -1;


    return 0;
}


int JN24_close(void)
{
    if (fd_jn24) close(fd_jn24);
    return 0;
}


/////////////////////// API ////////////////////////////////////////////

void GetLRC(unsigned char  *ucDataForLRC, unsigned  char *ucLRC,unsigned int uiLen)
{
    unsigned int i = 0;
    *ucLRC=0x00;

    for ( i=0; i<uiLen; i++)
        *ucLRC=*ucLRC^ucDataForLRC[i];
    return ;
}


int init_gn24gdata()
{
    memset(&g_jn24g_cmd, 0, sizeof(JN24G_CMD));
}


int Os__24g_mag_connect(unsigned char *pout, int  *out_len)
{
//    unsigned char pin[]={0x02, 0x01, 0x00, 0x00, 0x01, 0x10, 0x03, 0x10};
    unsigned char pin[]={0x02, 0x01, 0x00, 0x00, 0x01, 0x10, 0x03, 0x10};
    unsigned char x[255];
    int ret, i, len;

    ret = JN24_Proc(pin,8, x, &len);

    *out_len = len;
    memcpy(pout, x, len);
    pout[len]=0x0;

    return ret;
}

// data: 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00, 0x00
//	Data(1)~(6)：终端机编号（主机）。
//	Data(7)：密钥索引。 默认00H （保留，暂不处理）
//Data(8)：密钥算法标识符。 默认00H （保留，暂不处理）
int Os__24g_mag_init(unsigned char *data, unsigned char *pout, int  *out_len)
{
//    unsigned char pin[100]={	0x02, 0x01, 0x00, 0x01, 0x05,
//							0x11, 0x00, 0x11, 0x00, 0x00,
//							0x08, 0x01, 0x02, 0x03, 0x04,
//							0x05, 0x06, 0x00, 0x00, 0x0d,
//							0x03};
    unsigned char aucData[255];
    int ret, i, len, index=0, crc_len=0;

    JN24G_CMD *cmd=&g_jn24g_cmd;

    init_gn24gdata();

    memcpy(cmd->aucSendData, "\x02\x01\x00\x01\x05", 5);
    cmd->uiSendLen += 5;

    cmd->aucSendData[cmd->uiSendLen++]=0x11;	//cmd

    memcpy(cmd->aucSendData+cmd->uiSendLen, "\x00\x11\x00\x00\x08", 5);
    cmd->uiSendLen += 5;

    memcpy(cmd->aucSendData+cmd->uiSendLen, data, 8);	//pos
    cmd->uiSendLen += 8;
    cmd->aucSendData[cmd->uiSendLen++]=0x0d;	//data

    cmd->aucSendData[cmd->uiSendLen++]=0x03;	//data

    crc_len = cmd->aucSendData[2]*100+cmd->aucSendData[3]*10+cmd->aucSendData[4];
    GetLRC(cmd->aucSendData+5, &cmd->aucSendData[cmd->uiSendLen++], crc_len);


    ret = JN24_Proc(cmd->aucSendData,cmd->uiSendLen, aucData, &len);

    *out_len = len;
    memcpy(pout, aucData, len);
    pout[len]=0x0;
    return ret;
}



unsigned char TRACK_DATA[]=
{
    0x0,  // 0
    79, 40, 107,// 1, 2, 3
    119, // 4, 1+2
    186, // 5, 1+3
    147, // 6, 2+3
    226 // 7, 1+2+3
};



//termid = 0x01 .. 0x06
//track = 0x0 - 0x7
int Os__24g_mag_readtrack(unsigned char termid[6], unsigned char track, unsigned char *pout, int  *out_len)
{
//    unsigned char pin[100]={	0x02, 0x01, 0x00, 0x01, 0x04,
//							0x11, 0x00, 0x13, 0x00, 0x00,
//							0x07, 0x01, 0x02, 0x03, 0x04,
//							0x05, 0x06, 0x00, 0x??, 0x03}

    unsigned char aucData[255];
    int ret, i, len, index=0, crc_len=0;

    JN24G_CMD *cmd=&g_jn24g_cmd;

    init_gn24gdata();

    memcpy(cmd->aucSendData, "\x02\x01\x00\x01\x04", 5);
    cmd->uiSendLen += 5;

    cmd->aucSendData[cmd->uiSendLen++]=0x11;	//cmd

    memcpy(cmd->aucSendData+cmd->uiSendLen, "\x00\x13\x00\x00\x07", 5);
    cmd->uiSendLen += 5;

    memcpy(cmd->aucSendData+cmd->uiSendLen, termid, 6);	//pos
    cmd->uiSendLen += 6;

    cmd->aucSendData[cmd->uiSendLen++]=track;	//data

    if (track>0x29) track -= 0x30;
    cmd->aucSendData[cmd->uiSendLen++]=TRACK_DATA[track+0x30];	//data

    cmd->aucSendData[cmd->uiSendLen++]=0x03;	//data

    crc_len = cmd->aucSendData[2]*100+cmd->aucSendData[3]*10+cmd->aucSendData[4];
    GetLRC(cmd->aucSendData+5, &cmd->aucSendData[cmd->uiSendLen++], crc_len);


    ret = JN24_Proc(cmd->aucSendData,cmd->uiSendLen, aucData, &len);

    *out_len = len;
    memcpy(pout, aucData, len);
    pout[len]=0x0;

    return ret;
}


int Os__24g_mag_readphoto_init(unsigned char *pout, int  *out_len)
{
//    unsigned char pin[100]={	0x02, 0x01, 0x00, 0x00, 0x06,
//							0x11, 0x20, 0x0a, 0x01, 0x01,
//							0x00,
//							0x03}

    unsigned char aucData[255];
    int ret, i, len, index=0, crc_len=0;

    JN24G_CMD *cmd=&g_jn24g_cmd;

    init_gn24gdata();

    memcpy(cmd->aucSendData, "\x02\x01\x00\x00\x06", 5);
    cmd->uiSendLen += 5;

    cmd->aucSendData[cmd->uiSendLen++]=0x11;	//cmd

    memcpy(cmd->aucSendData+cmd->uiSendLen, "\x20\x0a\x01\x01\x00", 5);
    cmd->uiSendLen += 5;

    cmd->aucSendData[cmd->uiSendLen++]=0x03;	//data

    crc_len = cmd->aucSendData[2]*100+cmd->aucSendData[3]*10+cmd->aucSendData[4];
    GetLRC(cmd->aucSendData+5, &cmd->aucSendData[cmd->uiSendLen++], crc_len);


    ret = JN24_Proc(cmd->aucSendData,cmd->uiSendLen, aucData, &len);

    *out_len = len;
    memcpy(pout, aucData, len);
    pout[len]=0x0;
    return ret;
}


// xxH	所要读取的签名数据总包数
// yyH	所要读取的签名数据当前包序号
// data_type  	03H 图像图片数据   04H 签名图片数据
int Os__24g_mag_readphoto(unsigned char xx, unsigned char yy,  unsigned char data_type,
                          unsigned char *pout, int  *out_len)
{
//    unsigned char pin[100]={	0x02, 0x01, 0x00, 0x00, 0x07,
//							0x11, 0x20, 0x0b, 0x??, 0x??,
//							0x01, 0x??
//							0x03}

    unsigned char aucData[255];
    int ret, i, len, index=0, crc_len=0;

    JN24G_CMD *cmd=&g_jn24g_cmd;

    init_gn24gdata();

    memcpy(cmd->aucSendData, "\x02\x01\x00\x00\x07", 5);
    cmd->uiSendLen += 5;

    cmd->aucSendData[cmd->uiSendLen++]=0x11;	//cmd

    memcpy(cmd->aucSendData+cmd->uiSendLen, "\x20\x0b", 2);
    cmd->uiSendLen += 2;

    cmd->aucSendData[cmd->uiSendLen++]=xx;	//xx
    cmd->aucSendData[cmd->uiSendLen++]=yy;	//yy
    cmd->aucSendData[cmd->uiSendLen++]=0x01;	//lc
    cmd->aucSendData[cmd->uiSendLen++]=data_type;	//cmd

    cmd->aucSendData[cmd->uiSendLen++]=0x03;	//data

    crc_len = cmd->aucSendData[2]*100+cmd->aucSendData[3]*10+cmd->aucSendData[4];
    GetLRC(cmd->aucSendData+5, &cmd->aucSendData[cmd->uiSendLen++], crc_len);


    ret = JN24_Proc(cmd->aucSendData,cmd->uiSendLen, aucData, &len);

    *out_len = len;
    memcpy(pout, aucData, len);
    pout[len]=0x0;

    return ret;
}

/*
	Data(1)~(15)： 商家号。 （15字节 ASCII码）
	Data(16)~(19)：终端编号。（4字节 ASCII码）
	Data(20)：    卡号长度。（1字节 BCD码）
	Data(21)~(30)：卡号内容。（10字节）不足补00H
	Data(31)~(36)：交易金额。（6字节ASCII码）每次消费金额不得超出9999.99元。
	Data(37)~(39)：日期。（3字节 BCD码）
	Data(40)~(42)：时间。（3字节 BCD码）
*/
int Os__24g_mag_consumenote(unsigned char salerID[15], unsigned char termID[4],
                            unsigned char cardid_len, unsigned char cardid[10], unsigned char amount[6],
                            unsigned char date[3], unsigned char time[3], unsigned char *pout, int  *out_len)
{
//    unsigned char pin[100]={	0x02, 0x01, 0x00, 0x04, 0x08,
//							0x11, 0x00, 0x14, 0x00, 0x00,
//							0x2a, 0x?? .... 42个
//							0x03}

    unsigned char aucData[255];
    int ret, i, len, index=0, crc_len=0;

    JN24G_CMD *cmd=&g_jn24g_cmd;

    init_gn24gdata();

    memcpy(cmd->aucSendData, "\x02\x01\x00\x04\x08", 5);
    cmd->uiSendLen += 5;

    cmd->aucSendData[cmd->uiSendLen++]=0x11;	//cmd

    memcpy(cmd->aucSendData+cmd->uiSendLen, "\x00\x14\x00\x00\x2a", 5);
    cmd->uiSendLen += 5;


    memcpy(cmd->aucSendData+cmd->uiSendLen, salerID, 15);	//商户号
    cmd->uiSendLen += 15;

    memcpy(cmd->aucSendData+cmd->uiSendLen, termID, 4);	//终端号
    cmd->uiSendLen += 4;

    cmd->aucSendData[cmd->uiSendLen++]=cardid_len;	//卡号长度

    memcpy(cmd->aucSendData+cmd->uiSendLen, cardid, 10);	//卡号
    cmd->uiSendLen += 10;

    memcpy(cmd->aucSendData+cmd->uiSendLen, amount, 6);	//金额
    cmd->uiSendLen += 6;

    memcpy(cmd->aucSendData+cmd->uiSendLen, date, 3);	//日期
    cmd->uiSendLen += 3;

    memcpy(cmd->aucSendData+cmd->uiSendLen, time, 3);	//时间
    cmd->uiSendLen += 3;


    cmd->aucSendData[cmd->uiSendLen++]=0x03;	//data

    crc_len = cmd->aucSendData[2]*100+cmd->aucSendData[3]*10+cmd->aucSendData[4];
    GetLRC(cmd->aucSendData+5, &cmd->aucSendData[cmd->uiSendLen++], crc_len);


    ret = JN24_Proc(cmd->aucSendData,cmd->uiSendLen, aucData, &len);

    *out_len = len;
    memcpy(pout, aucData, len);
    pout[len]=0x0;

    return ret;
}

int Os__24g_mag_disconnect( unsigned char *pout, int  *out_len)
{
    unsigned char pin[]={0x02, 0x01, 0x00, 0x00, 0x01, 0x12, 0x03, 0x12};
    unsigned char x[255];
    int ret, i, len;

    ret = JN24_Proc(pin,8, x, &len);

    *out_len = len;
    memcpy(pout, x, len);
    pout[len]=0x0;
    return ret;
}


//electronic purse
int Os__24g_ep_connect(unsigned char *pout, int  *out_len)
{
    unsigned char pin[]={0x02, 0x01, 0x00, 0x00, 0x01, 0x10, 0x03, 0x10};
    unsigned char x[255];
    int ret, i, len;

    ret = JN24_Proc(pin,8, x, &len);

    *out_len = len;
    memcpy(pout, x, len);
    pout[len]=0x0;

    return ret;
}

int Os__24g_ep_disconnect(unsigned char *pout, int  *out_len)
{
    unsigned char pin[]={0x02, 0x01, 0x00, 0x00, 0x01, 0x12, 0x03, 0x12};
    unsigned char x[255];
    int ret, i, len;

    ret = JN24_Proc(pin,8, x, &len);

    *out_len = len;
    memcpy(pout, x, len);
    pout[len]=0x0;
    return ret;
}

/*
*P1： 01H：消费   02H：充值
*
*
*
*/
int Os__24g_ep_trade(unsigned char p1, unsigned char salerID[15], unsigned char termID[4],
                     unsigned char cardid_len, unsigned char cardid[10], unsigned char amount[6],
                     unsigned char date[3], unsigned char time[3],
                     unsigned char *pout, int  *out_len)
{
//    unsigned char pin[100]={	0x02, 0x01, 0x00, 0x04, 0x08,
//							0x11, 0x00, 0x14, 0x??, 0x00,
//							0x2a, data 42字节,
//							0x03};
    unsigned char aucData[255];
    int ret, i, len, index=0, crc_len=0;

    JN24G_CMD *cmd=&g_jn24g_cmd;

    init_gn24gdata();

    memcpy(cmd->aucSendData, "\x02\x01\x00\x04\x08", 5);
    cmd->uiSendLen += 5;

    cmd->aucSendData[cmd->uiSendLen++]=0x11;	//cmd

    memcpy(cmd->aucSendData+cmd->uiSendLen, "\x00\x14", 2);
    cmd->uiSendLen += 2;
    cmd->aucSendData[cmd->uiSendLen++]=p1;	//消费or充值？
    memcpy(cmd->aucSendData+cmd->uiSendLen, "\x00\x2a", 2);
    cmd->uiSendLen += 2;

    memcpy(cmd->aucSendData+cmd->uiSendLen, salerID, 15);	//商户号
    cmd->uiSendLen += 15;

    memcpy(cmd->aucSendData+cmd->uiSendLen, termID, 4);	//终端号
    cmd->uiSendLen += 4;

    cmd->aucSendData[cmd->uiSendLen++]=cardid_len;	//卡号长度

    memcpy(cmd->aucSendData+cmd->uiSendLen, cardid, 10);	//卡号
    cmd->uiSendLen += 10;

    memcpy(cmd->aucSendData+cmd->uiSendLen, amount, 6);	//金额
    cmd->uiSendLen += 6;

    memcpy(cmd->aucSendData+cmd->uiSendLen, date, 3);	//日期
    cmd->uiSendLen += 3;

    memcpy(cmd->aucSendData+cmd->uiSendLen, time, 3);	//时间
    cmd->uiSendLen += 3;


    cmd->aucSendData[cmd->uiSendLen++]=0x03;	//data

    crc_len = cmd->aucSendData[2]*100+cmd->aucSendData[3]*10+cmd->aucSendData[4];
    GetLRC(cmd->aucSendData+5, &cmd->aucSendData[cmd->uiSendLen++], crc_len);


    ret = JN24_Proc(cmd->aucSendData,cmd->uiSendLen, aucData, &len);

    *out_len = len;
    memcpy(pout, aucData, len);
    pout[len]=0x0;

    return ret;
}


//voucher
int Os__24g_voucher_connect(unsigned char *pout, int  *out_len)
{
    unsigned char pin[]={0x02, 0x01, 0x00, 0x00, 0x01, 0x10, 0x03, 0x10};
    unsigned char x[255];
    int ret, i, len;

    ret = JN24_Proc(pin,8, x, &len);
    if (ret<0) goto out;

    *out_len = len;
    memcpy(pout, x, len);
    pout[len]=0x0;

out:
    return ret;
}

int Os__24g_voucher_disconnect(unsigned char *pout, int  *out_len)
{
    unsigned char pin[]={0x02, 0x01, 0x00, 0x00, 0x01, 0x12, 0x03, 0x12};
    unsigned char x[255];
    int ret, i, len;

    ret = JN24_Proc(pin,8, x, &len);
    if (ret<0) goto out;

    *out_len = len;
    memcpy(pout, x, len);
    pout[len]=0x0;

out:
    return ret;
}


int Os__24g_voucher_init(unsigned char *termID, unsigned char *pout, int  *out_len)
{
//    unsigned char pin[100]={0x02, 0x01, 0x00, 0x01, 0x02,
//	0x11, 0x30, 0x01, 0x00, 0x00, 0x06, 0x01, 0x02, 0x03, 0x04, 0x05,0x06, 0x03};
    unsigned char aucData[255];
    int ret, i, len, index=0, crc_len=0;

    JN24G_CMD *cmd=&g_jn24g_cmd;

    init_gn24gdata();

    memcpy(cmd->aucSendData, "\x02\x01\x00\x01\x02", 5);
    cmd->uiSendLen += 5;

    cmd->aucSendData[cmd->uiSendLen++]=0x11;	//cmd

    memcpy(cmd->aucSendData+cmd->uiSendLen, "\x30\x01\x00\x00\x06", 5);
    cmd->uiSendLen += 5;

    //memcpy(cmd->aucSendData+cmd->uiSendLen, "\x01\x02\x03\x04\x05\x06", 6);	//pos
    memcpy(cmd->aucSendData+cmd->uiSendLen, termID, 6);	//pos
    cmd->uiSendLen += 6;

    cmd->aucSendData[cmd->uiSendLen++]=0x03;	//data

    crc_len = cmd->aucSendData[2]*100+cmd->aucSendData[3]*10+cmd->aucSendData[4];
    GetLRC(cmd->aucSendData+5, &cmd->aucSendData[cmd->uiSendLen++], crc_len);


    ret = JN24_Proc(cmd->aucSendData,cmd->uiSendLen, aucData, &len);

    *out_len = len;
    memcpy(pout, aucData, len);
    pout[len]=0x0;

    return ret;
}

//p1=0x1, p2=0x1
//vid = 0x00...0xff
int Os__24g_voucher_read(unsigned char p1, unsigned char p2, unsigned char vid[16],
                         unsigned char *pout, int  *out_len)
{
//    unsigned char pin[100]={0x02, 0x01, 0x00, 0x02, 0x02,
//                            0x11, 0x30, 0x02, 0x01, 0x01, 0x10,
//                            0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
//                            0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
//                            0xcc, 0xdd,0xee,0xff, 0x03};

    unsigned char aucData[255];
    int ret, i, len, index=0, crc_len=0, data_len=0;

    JN24G_CMD *cmd=&g_jn24g_cmd;

    init_gn24gdata();

    memcpy(cmd->aucSendData, "\x02\x01\x00\x02\x02", 5);
    cmd->uiSendLen += 5;

    cmd->aucSendData[cmd->uiSendLen++]=0x11;	//cmd

    memcpy(cmd->aucSendData+cmd->uiSendLen, "\x30\x02", 2);
    cmd->uiSendLen += 2;

    cmd->aucSendData[cmd->uiSendLen++]=p1;	//所要读取优惠卷信息总包数
    cmd->aucSendData[cmd->uiSendLen++]=p2;	//所要读取优惠卷信息当前包序号
    cmd->aucSendData[cmd->uiSendLen++]=0x10;	//DATA字段数据长度


    memcpy(cmd->aucSendData+cmd->uiSendLen, vid, 16);	//pos
    cmd->uiSendLen += 16;

    cmd->aucSendData[cmd->uiSendLen++]=0x03;	//data

    crc_len = cmd->aucSendData[2]*100+cmd->aucSendData[3]*10+cmd->aucSendData[4];
    GetLRC(cmd->aucSendData+5, &cmd->aucSendData[cmd->uiSendLen++], crc_len);

    ret = JN24_Proc(cmd->aucSendData,cmd->uiSendLen, aucData, &len);

    //Uart_Printf("%s aucData[7]=%x ret=%x\n", __func__, aucData[7], ret);
    if (!ret)
    {
        if (aucData[7]==0x00)
        {	//EN
            Uart_Printf("%s read voucher in English\n", __func__);
            data_len = aucData[6];	//数据区的长度，不包括
            *out_len = data_len/2+6+1+2+2;

            memcpy(pout, aucData, 7);
            for (i=0; i<data_len/2; i++)
            {
                pout[7+i] = aucData[7+(i*2+1)];
            }
            memcpy(pout+7+data_len/2, &aucData[7+data_len], 4);
            pout[7+data_len/2+4]=0x0;

        }
        else
        {	//NOT EN
            Uart_Printf("%s read voucher in CHN\n", __func__);
            *out_len = len;
            memcpy(pout, aucData, len);
            pout[len]=0x0;
        }
    }
    return ret;
}

int Os__24g_voucher_readover(unsigned char *vid, unsigned char *pout, int  *out_len)
{
//    unsigned char pin[100]={0x02, 0x01, 0x00, 0x02, 0x02,
//                            0x11, 0x30, 0x03, 0x00, 0x00, 0x10,
//                            0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
//                            0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
//                            0xcc, 0xdd,0xee,0xff, 0x03};

    unsigned char aucData[255];
    int ret, i, len, index=0, crc_len=0;

    JN24G_CMD *cmd=&g_jn24g_cmd;

    init_gn24gdata();

    memcpy(cmd->aucSendData, "\x02\x01\x00\x02\x02", 5);
    cmd->uiSendLen += 5;

    cmd->aucSendData[cmd->uiSendLen++]=0x11;	//cmd



    memcpy(cmd->aucSendData+cmd->uiSendLen, "\x30\x03\x00\x00\x10", 5);
    cmd->uiSendLen += 5;

    memcpy(cmd->aucSendData+cmd->uiSendLen, vid, 16);	//pos
    cmd->uiSendLen += 16;

    cmd->aucSendData[cmd->uiSendLen++]=0x03;	//data

    crc_len = cmd->aucSendData[2]*100+cmd->aucSendData[3]*10+cmd->aucSendData[4];
    GetLRC(cmd->aucSendData+5, &cmd->aucSendData[cmd->uiSendLen++], crc_len);

    ret = JN24_Proc(cmd->aucSendData,cmd->uiSendLen, aucData, &len);

    *out_len = len;
    memcpy(pout, aucData, len);
    pout[len]=0x0;

    return ret;
}


/////////////////////// API END ////////////////////////////////////////////


typedef struct
{
    int len;
    unsigned char data[4096];
} PIC_INFO;

int SEIMMA_DownloadPIC()
{
    unsigned char pout[255],  prn_data[100];
    int ret, i, j=5, out_len;
    int sign_counts=0, photo_counts=0, idx=0;
    unsigned char le=0;
    PIC_INFO pic_sign, pic_photo;
    FILE *fp;

    ret = Os__24g_mag_readphoto_init(pout, &out_len);
    if (ret<0)
    {
        Uart_Printf("readphoto init failed!\n");
        return -1;
    }

    le = pout[6+2];
    if (le==0x00)
    {
        return -2;
    }

    photo_counts = pout[11];
    sign_counts = pout[12];
    Uart_Printf("%s photo_counts = %d, sign_counts=%d\n",__func__, photo_counts, sign_counts);
    if (!photo_counts || !sign_counts) return -3;

    i=0;
    pic_photo.len=0;
    memset((unsigned char *)pic_photo.data, 0, sizeof(PIC_INFO));
    while (i<photo_counts)
    {
        ret = Os__24g_mag_readphoto(photo_counts, i+1, 0x03, pout, &out_len);
        if (ret<0)
        {
            Uart_Printf("%s read photo data failed!\n", __func__);
            return -1;
        }

        memcpy(pic_photo.data+pic_photo.len, pout+10, out_len-10-4);
        pic_photo.len+=out_len-10-4;
        i++;
    }

    i=0;
    pic_sign.len=0;
    memset((unsigned char *)pic_sign.data, 0, sizeof(PIC_INFO));
    while (i<sign_counts)
    {
        ret = Os__24g_mag_readphoto(sign_counts, i+1, 0x04, pout, &out_len);
        if (ret<0)
        {
            Uart_Printf("%s read photo data failed!\n", __func__);
            return -1;
        }
        memcpy(pic_sign.data+pic_sign.len, pout+10, out_len-10-4);
        pic_sign.len+=out_len-10-4;

        i++;
    }


    if (sign_counts)
    {
        if ((fp=fopen("/mnt/pos/app/lcd/jn24_sign1.bmp","w"))==NULL)
        {
            Uart_Printf("Can't open jn24_sign1.bmp\n");
            return -1;
        }
        fwrite(pic_sign.data,pic_sign.len,1,fp);
        fclose(fp);
    }

    if (photo_counts)
    {

        if ((fp=fopen("/mnt/pos/app/lcd/jn24_pic1.jpg","w"))==NULL)
        {
            Uart_Printf("Can't open jn24_pic1.jpg\n");
            return -1;
        }
        fwrite(pic_photo.data,pic_photo.len,1,fp);
        fclose(fp);
    }

    if (photo_counts && sign_counts)
        Os__lcd_image(64,0, "null.bmp");

    if (photo_counts)
        Os__lcd_image(64,0, "jn24_pic1.jpg");

    if (sign_counts)
        Os__lcd_image(64+180,50, "jn24_sign1.bmp");

    return 0;
}


int jn24_test(char argc, char **argv)
{
    unsigned char pout[255],  prn_data[100];
    int ret, i, j=5, out_len;
    unsigned char vid[16]={0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd,0xee,0xff};
    unsigned char termID[6]={0x01, 0x02, 0x03, 0x04, 0x05,0x06};
    int case_select=0;


    Os__lcd_image(64,0, "null.bmp");
    Os__lcd_image(64,0, "jn24_pic.jpg");
    Os__lcd_image(64+180,50, "jn24_sign.bmp");

    if (argc==1)
    {
        Uart_Printf("plz use as  ** demo argv1 **\n");
        return 0;
    }

    switch (atoi(argv[1]))
    {

    case 1:
        //优惠券
    {
        Os__24g_voucher_connect(pout, &out_len);
        sleep(1);

        Os__24g_voucher_init(termID, pout, &out_len);
        sleep(1);


        Os__24g_voucher_read(0x1, 0x1, vid, pout, &out_len);
        sleep(1);

        //print data
        memset(prn_data, 0, 100);
        memcpy(prn_data, pout+7, pout[6]);
//        Os__GB2312_xprint(prn_data, 0x01);
//        Os__GB2312_xprint(prn_data, 0x1c);
        Os__GB2312_xprint(prn_data, 0x1d);
//        Os__GB2312_xprint(prn_data, 0x1e);
        Os__linefeed(160);

        //   Os__24g_voucher_readover(vid, pout, &out_len);


        Os__24g_voucher_disconnect(pout, &out_len);
        break;
    }

    case 2:
    {
        //银行卡
        Os__24g_mag_connect(pout, &out_len);

        Os__24g_mag_init("\x01\x02\x03\x04\x05\x06\x00\x00", pout, &out_len);

        Os__24g_mag_readtrack("\x01\x02\x03\x04\x05\x06", 0x01,pout, &out_len);

        SEIMMA_DownloadPIC();

        memset(prn_data, 0, 100);
        memcpy(prn_data, "1234567890abcef", 15);
        memcpy(prn_data+15, "1111", 4);
        prn_data[19]=0x10;
        memcpy(prn_data+20, "\x11\x22\x33\x44\x55\x55\x44\x33\x22\x11", 10);
        memcpy(prn_data+30, "222222", 6);
        memcpy(prn_data+36, "\x10\x03\x11", 3);
        memcpy(prn_data+39, "\x21\x38\x00", 3);
        Os__24g_mag_consumenote("1234567890abcef", "\x01\x01\x01\x01", 0x10,
                                "\x11\x22\x33\x44\x55\x55\x44\x33\x22\x11", "222222",
                                "\x10\x03\x11","\x21\x38\x00",
                                pout, &out_len);
        break;
    }


    case 3:
    {
        Os__24g_ep_disconnect(pout, &out_len);

        //电子钱包
        Os__24g_ep_connect(pout, &out_len);

        //卡号和时间存在问题
        Os__24g_ep_trade(0x01, "\x01\x02\x03\0x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
                         "\x11\x11\x11\x11",0x3, "\x00\x00\x00\x00\x00\x00\x00\x31\x32\x32",
//                         "\x00\x00\x00\x00\x00\x01", "\x10\x03\x12", "\x11\x22\x33",
                         "001630", "\x10\x03\x15", "\x13\x20\x11",
                         pout, &out_len
                        );
        break;
    }

    }
    return 0;
}


//////////// end ////////////////////

