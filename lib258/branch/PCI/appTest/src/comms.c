#include <include.h>
#include <Global.h>
#include <xdata.h>
#include <oslib.h>
#include <sand_pci.h>
#include <menu.h>



unsigned char COMMS_GenSendReceive(void)
{
	unsigned char aucMAC[8];
	unsigned short uiLen;
	unsigned char ucResult;
	unsigned short SendMsgID,RecvMsgID;
	int i;//debug

	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
						(unsigned char *)&DataSave0.Trans_8583Data.SendISO8583Data);

	
	SendMsgID = (unsigned short)bcd_long(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN*2);

	ucResult = ISO8583_PackData(ISO8583Data.aucCommBuf,&ISO8583Data.uiCommBufLen,
					ISO8583_MAXCOMMBUFLEN);
	if(ucResult!= SUCCESS)
	{
		return ucResult;
	}
	if (ISO8583_CheckBit(64) == SUCCESS)
	{
		uiLen = ISO8583Data.uiCommBufLen-8;
		ucResult = UTIL_CalcGoldenCardSHMAC_Single(ISO8583Data.aucCommBuf,uiLen,
									aucMAC);
		memcpy( &ISO8583Data.aucCommBuf[uiLen],aucMAC,8);
	}
	uiLen = sizeof(ISO8583Data.aucCommBuf);

	ucResult = COMMS_RS232_SendReceive(ISO8583Data.aucCommBuf,ISO8583Data.uiCommBufLen,ISO8583Data.aucCommBuf,&uiLen);
	if( ucResult != SUCCESS)
	{
		Uart_Printf("COMMS_RS232_SendReceive error\r\n");
		return(ucResult);
	}

	Uart_Printf("uiLen = %d\r\n",uiLen);
	Uart_Printf("\r\n");
	for(i=0;i<500;i++)
	{
		Uart_Printf("%02x ",ISO8583Data.aucCommBuf[i]);
	}
	Uart_Printf("\r\n");

	ISO8583Data.uiCommBufLen = uiLen;
	ISO8583_UnpackData(ISO8583Data.aucCommBuf,ISO8583Data.uiCommBufLen);

	RecvMsgID = (unsigned short)bcd_long(ISO8583Data.aucMsgID,ISO8583_MSGIDLEN*2);


	if( RecvMsgID != SendMsgID + 10)
	{
		Uart_Printf("RecvMsgID not equals\r\n");
		return(ERR_UNKNOWNTRANTYPE);
	}

	

	ucResult = ISO8583_CheckResponseValid();
	if( ucResult != SUCCESS)
	{
		Uart_Printf("IOS8583 check error\r\n");
		return(ucResult);
	}

	return(SUCCESS);
}




unsigned char COMMS_RS232_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen)
{
	int ucResult;
	unsigned char aucBuf[16];
	unsigned short uiI;
	unsigned short uiDataLen;
	unsigned short uiPacketLen;
	unsigned char ucTPDULen,ucPackgeHeadLen;
	unsigned short uiTimeout,ReceiveBytetime=50;
	unsigned char ucByte;
	unsigned char *pucData;
	unsigned char aucTPDU[6];
	unsigned char aucSandServer[3];
	unsigned char aucPackgeHead[7];
	unsigned char aucIOBuff[ISO8583_MAXCOMMBUFLEN];
	unsigned char ucLRC;
	unsigned int uiLen;
	
	aucTPDU[0] = 0x60;
	aucTPDU[1] = 0x04;
	aucTPDU[2] = 0x01;
	aucTPDU[3] = 0x00;
	aucTPDU[4] = 0x00;


	memset(aucBuf,0,sizeof(aucBuf));
	memset(aucPackgeHead,0,sizeof(aucPackgeHead));
	memcpy(aucBuf,"60",2);
	memcpy(&aucBuf[2],"22",2);
	aucBuf[4] = '0';

	memcpy(&aucBuf[5],"0000000",7);

	asc_bcd(aucPackgeHead,6,aucBuf,12);

	//Os__com_flush2();
								
        memset(aucIOBuff,0x00,sizeof(aucIOBuff));
        ucTPDULen=sizeof(aucTPDU)-1;
        ucPackgeHeadLen = sizeof(aucPackgeHead)-1;
	Uart_Printf("ucTUDULen = %d\r\n",ucTPDULen);
	Uart_Printf("ucPackgeHeadLen = %d\r\n",ucPackgeHeadLen);
	Uart_Printf("uiInLen = %d\r\n",uiInLen);

        uiPacketLen= ucTPDULen+ucPackgeHeadLen+uiInLen;
	Uart_Printf("uiPacketLen = %02x\r\n",uiPacketLen);

	clear_display();
      	Os__GB2312_display(1, 0, (uchar *) "已连接收单中心\r\n");
	Os__GB2312_display(2, 0, (uchar *) "处理中...\r\n");
	Os__GB2312_display(3, 0, (uchar *) "数据发送中...\r\n");
	
        memcpy(aucIOBuff,aucTPDU,ucTPDULen);
        memcpy(&aucIOBuff[ucTPDULen],aucPackgeHead,ucPackgeHeadLen);
        memcpy(&aucIOBuff[ucTPDULen+ucPackgeHeadLen],pucInData,uiInLen);
	ucResult = serial_write(aucIOBuff,uiPacketLen);
	if(ucResult < 0)
	{
		Uart_Printf("serial write error\r\n");
		return ucResult;
	}

	memset(aucIOBuff,0,sizeof(aucIOBuff));
	uiTimeout = 60*100;
	ucResult = serial_read(aucIOBuff,ISO8583_MAXCOMMBUFLEN,uiTimeout);
	if(ucResult < 0)
	{
		return ucResult;
	}
	memcpy(pucOutData,aucIOBuff+11,ucResult-11);
	clear_display();
	Os__GB2312_display(1, 0, (uchar *) "已连接收单中心\r\n");
	Os__GB2312_display(2, 0, (uchar *) "处理中...\r\n");
	Os__GB2312_display(3, 0, (uchar *) "接收返回...\r\n");

	*puiOutLen = ucResult-11;
	return(SUCCESS);
}

unsigned char COMMS_ReceiveByte(unsigned char *pucByte,unsigned short *puiTimeout)
{
	unsigned short ucResult,ucReCOMM;

      ucResult = Os__rxcar3(*puiTimeout);
	switch	( ucResult / 256 )
	{
		case OK :
			*pucByte = ucResult % 256;
			ucReCOMM = OK;
			break;
		case COM_ERR_TO:
			ucReCOMM = ERR_COMMS_RECVTIMEOUT;
			break;
		default :
			ucReCOMM = ERR_COMMS_RECVCHAR;
			break;
	}
	return (ucReCOMM);
}




