#define		DIGITAL_DATA
#include	<digital.h>
#undef		DIGITAL_DATA

UCHAR	DIGITAL_OpenSerial(void)
{
	UCHAR	ucResult;
	
	COM_PARAM CommParam =
	{
		1,     /* Stop bit */
		115200, /* Speed */
		1,	   /* MODEM 0 RS232 1 HDLC 2 */
		8,     /* Data bit */
	   'N'     /* No parity */
	};
	util_Printf("\nConstParam.ucReaderPortOS=%d",ConstParam.ucReaderPort);
	if(ConstParam.ucReaderPort ==1)
		ucResult = Os__init_com(&CommParam);
	else if(ConstParam.ucReaderPort ==2)
		ucResult = Os__init_com3(0x0303, 0x0c00, 0x0c);
	//else if(ConstParam.ucReaderPort ==3)
	//	ucResult = OSUART_Init3(&CommParam);
	if(ucResult) ucResult =EMVERROR_OPENCOM;
	
	return ucResult;
}

UCHAR DIGITAL_ComClose()
{

	util_Printf("\nDIGITAL_ComCloseConstParam.ucReaderPort=%d",ConstParam.ucReaderPort);
	if(ConstParam.ucReaderPort ==1)
		OSUART_Close1();
	else if(ConstParam.ucReaderPort ==2)
		OSUART_Close2();
	//else 	
	//	OSUART_Close3();
		
	return EMVERROR_SUCCESS;
}


void	DIGITAL_SendData(PUCHAR pucSend,USHORT uiSendLen)
{
	USHORT	uiI;

	//if(ConstParam.bDigitalDebug)
	{

		util_Printf("\nSend Data:\n");
		for(uiI =0;uiI<uiSendLen;uiI++)
			util_Printf("%02X ",*(pucSend+uiI));
	}	
	if(ConstParam.ucReaderPort ==1)
	{
		Os__com_flush1(0);
		for(uiI =0;uiI <uiSendLen;uiI++)
			Os__txcar(*(pucSend+uiI));
	}
//	else if(ConstParam.ucReaderPort ==2)
//	{
//		Os__com_flush2();
//		for(uiI =0;uiI <uiSendLen;uiI++)
//			Os__txcar3(*(pucSend+uiI));
//	}
//	else	
//	{
//		Os__com_flush3();
//		for(uiI =0;uiI <uiSendLen;uiI++)
//			OSUART_TxChar3(*(pucSend+uiI));
//	}
}
UCHAR DIGITAL_ReceiveByte(PUCHAR pucByte,USHORT uiTimeout)
{
	USHORT uiResult;
#if 0
	uiResult = Os__rxcar(uiTimeout);
#else

	
	if(ConstParam.ucReaderPort ==1)
		uiResult = Os__rxcar(uiTimeout);
	else if(ConstParam.ucReaderPort ==2)
		uiResult = Os__rxcar3(uiTimeout);
//	else
//		uiResult = OSUART_RxChar3(uiTimeout);
#endif

	switch	( uiResult / 256 )
	{
	case EMVERROR_SUCCESS :
		*pucByte = uiResult % 256;
	//if(ConstParam.bDigitalDebug)	
		util_Printf("%02X ",*pucByte);

		return(EMVERROR_SUCCESS);
	default:
		return EMVERROR_RECEIVEDATA;
	
	}
}

UCHAR	DIGITAL_RecvData(UCHAR bCheck ,UINT uiTimeout)
{
	UCHAR ucResult,ucLRC;
	USHORT	uiLen,uiI;

	util_Printf("\nConstParam.bDigitalDebug=DIGITAL_RecvData=%d",ConstParam.bDigitalDebug);
	if(ConstParam.bDigitalDebug)
		util_Printf("\nRecv Data:\n");

	memset(aucSendRecvBuf,0x00,sizeof(aucSendRecvBuf));
	uiSendRecvLen = 0;
	if(bCheck)
	{
		Os__timer_start(&uiTimeout );
		do{
			ucResult = DIGITAL_ReceiveByte(&aucSendRecvBuf[0],0xff);
			if(!ucResult)
			{
				if(aucSendRecvBuf[0] == STX) break;
			}
			
		}while(uiTimeout);
		Os__timer_stop(&uiTimeout);
		if(!uiTimeout) 
		{
			util_Printf("\nTime Out");
			return EMVERROR_RECEIVEDATA;
		}
	}
	else aucSendRecvBuf[uiSendRecvLen] =STX;
	
	uiSendRecvLen++;
	

	ucResult =EMVERROR_SUCCESS;
	for(uiI=0;uiI <2&&!ucResult ;uiI++)
		ucResult =DIGITAL_ReceiveByte(&aucSendRecvBuf[1+uiI],10);
	if(ucResult) return EMVERROR_RECEIVEDATA;
	uiSendRecvLen +=2;

	uiLen =(USHORT)CONV_HexLong(&aucSendRecvBuf[1],2);
	uiLen +=2;
	
	for(uiI=0;uiI< uiLen;uiI++)
		ucResult =DIGITAL_ReceiveByte(&aucSendRecvBuf[3+uiI],10);
	
	uiSendRecvLen +=uiLen;
	
	if(aucSendRecvBuf[uiSendRecvLen-2] !=ETX)
		return EMVERROR_RECEIVEDATA;
	ucLRC = 0xFF;
	for(uiI=1;uiI<uiSendRecvLen-2;uiI++)
	{
		ucLRC ^= aucSendRecvBuf[uiI];
	}
	
	if(ucLRC!=aucSendRecvBuf[uiSendRecvLen-1])
		return EMVERROR_RECEIVEDATA;
	
	return EMVERROR_SUCCESS;
	
	
}

UCHAR	DIGITAL_Poling(void)
{
	UCHAR	aucBuff[10],ucResult;

	memset(&PolingData,0x00,sizeof(POLINGDATA));
//	ucResult =DIGITAL_OpenSerial();
//	if(ucResult) return ucResult;
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	aucBuff[0] =CMD_Poling;
	DIGITAL_Package(aucBuff,1);
	DIGITAL_SendData(aucSendRecvBuf,uiSendRecvLen);
	return ucResult;

}
UCHAR	DIGITAL_PolingProcess(void)
{
	UCHAR ucResult;

	
	
	ucResult =DIGITAL_RecvData(false,10);
	util_Printf("\nDIGITAL_RecvData(false,10)===%d",ucResult);
	if(!ucResult)
	{
//		util_Printf("\nuiSendRecvLen %d ucResult %d",uiSendRecvLen,ucResult);
		if(aucSendRecvBuf[5]!=CMD_Poling)
			ucResult =EMVERROR_ICCCOMAND;
		else if(!aucSendRecvBuf[6])
			memcpy(&PolingData,&aucSendRecvBuf[7],uiSendRecvLen-9);
		else if(aucSendRecvBuf[6]==0x07)
			ucResult = 0x43;//EMVERROR_COLLISION->0x43TEST
		else
			ucResult =EMVERROR_ICCCOMAND;
	}
	else
		DIGITAL_PolingAbort();
//	DIGITAL_ComClose();
	
	return ucResult;

}

UCHAR	DIGITAL_Active(void)
{
	UCHAR	aucBuff[10],ucResult;
	
//	ucResult =DIGITAL_OpenSerial();
//	if(ucResult) return ucResult;
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	if(PolingData.ucCardType==0x01)
		aucBuff[0] =CMD_WUPTOTAT;
	else
		aucBuff[0] =CMD_WUPTOATTR;
	DIGITAL_Package(aucBuff,1);
	DIGITAL_SendData(aucSendRecvBuf,uiSendRecvLen);
	
	ucResult =DIGITAL_RecvData(true,10);
	if(!ucResult)
	{
		if(aucSendRecvBuf[5]!=aucBuff[0])
			ucResult =EMVERROR_ICCCOMAND;
		else if(aucSendRecvBuf[6])
			ucResult =EMVERROR_ICCCOMAND;
//			memcpy(&PolingData,&aucSendRecvBuf[7],uiSendRecvLen-9);
//		else

	}
//	DIGITAL_ComClose();
	if(!ucResult)
		ucResult =DIGITAL_Light(LIGHT_READ);
	return ucResult;
}
#if SANDREADER
UCHAR WKICC(QICCIN *pQICCIn ,QICCOUT  *pQICCOut)
{
	UCHAR	ucResult,ucStatus;
	USHORT	uiLen,uiI;

	util_Printf("\nSend ICC Data:\n");
	util_Printf("%02X %02X %02X %02X ",pQICCIn->ucCla,pQICCIn->ucIns,pQICCIn->ucP1,pQICCIn->ucP2);
	for(uiI=0;uiI<pQICCIn->uiLc;uiI++)
		util_Printf("%02X ",*(pQICCIn->aucDataIn+uiI));
	if(pQICCIn->uiLe) util_Printf("00");

	DrvIn.type      = 0x89;
	uiLen =1;
	memcpy(&DrvIn.xxdata[uiLen],&pQICCIn->ucCla,4);
	uiLen +=4;	
	if(pQICCIn->uiLc)
	{
		DrvIn.xxdata[uiLen++] =pQICCIn->uiLc;
		memcpy(&DrvIn.xxdata[uiLen],pQICCIn->aucDataIn,pQICCIn->uiLc);
		uiLen += pQICCIn->uiLc;
	}
	if(pQICCIn->uiLe)
		DrvIn.xxdata[uiLen++] =0x00;
	DrvIn.length =uiLen;
	DrvIn.xxdata[0]=DrvIn.length-1;
	
	ucResult =DIGITAL_SendReceive(30*100);
	if( ucResult == SUCCESS)
	{
		ucStatus = DrvOut.xxdata[0];
		if(!ucStatus)
		{
			pQICCOut->uiLenOut =DrvOut.xxdata[1];
			memcpy(pQICCOut->aucDataOut,&DrvOut.xxdata[2],pQICCOut->uiLenOut);
			pQICCOut->ucSWA = pQICCOut->aucDataOut[pQICCOut->uiLenOut-2];
			pQICCOut->ucSWB = pQICCOut->aucDataOut[pQICCOut->uiLenOut-1];
			pQICCOut->uiLenOut =DrvOut.xxdata[1]-2;
		}else ucResult =ERR_ICCARD;
	}
	if(!ucResult)
	{
		util_Printf("\nRecv ICC Data:\n");
		for(uiI=0;uiI<pQICCOut->uiLenOut;uiI++)
			util_Printf("%02X ",pQICCOut->aucDataOut[uiI]);
		util_Printf("\nSW: %02X %02X",pQICCOut->ucSWA,pQICCOut->ucSWB);
	}	

	util_Printf("\n digital  WKICC ucResult:[%02x]\n",ucResult );

	return ucResult;
}

UCHAR	QICC(QICCIN *pQICCIn ,QICCOUT  *pQICCOut)
{
	
	UCHAR	aucBuff[300],ucResult;
	USHORT	uiLen;

	
//	ucResult =DIGITAL_OpenSerial();
//	if(ucResult) return ucResult;
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	uiLen =0;
	aucBuff[uiLen++] =CMD_Digital;
	memcpy(&aucBuff[uiLen],&pQICCIn->ucCla,4);
	uiLen += 4;
	if(pQICCIn->uiLc)
	{
		aucBuff[uiLen++] =pQICCIn->uiLc;
		memcpy(&aucBuff[uiLen],pQICCIn->aucDataIn,pQICCIn->uiLc);
		uiLen += pQICCIn->uiLc;
	}
	if(pQICCIn->uiLe)
		aucBuff[uiLen++] =0x00;
	
	DIGITAL_Package(aucBuff,uiLen);
#if 1
	DIGITAL_SendData(aucSendRecvBuf,uiSendRecvLen);
	
	ucResult =DIGITAL_RecvData(true,10000);
	if(!ucResult)
	{
		if(aucSendRecvBuf[5]!=aucBuff[0])
			ucResult =EMVERROR_ICCCOMAND;
		else if(!aucSendRecvBuf[6])
		{
			pQICCOut->uiLenOut =uiSendRecvLen-9-2;
			memcpy(pQICCOut->aucDataOut,&aucSendRecvBuf[7],pQICCOut->uiLenOut);
			pQICCOut->ucSWA = aucSendRecvBuf[7+pQICCOut->uiLenOut];
			pQICCOut->ucSWB= aucSendRecvBuf[7+pQICCOut->uiLenOut+1];
		}
		else
			ucResult =EMVERROR_ICCCOMAND;
	}
//	DIGITAL_ComClose();

#else
/*

//	DDA ERROR
	
	if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x0E\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\x00",20))
	{
		pEMVICCOut->uiLenOut =52;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x32\x84\x0E\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\xA5\x20\xBF\x0C\x1D\x61\x1B\x4F\x08\xA0\x00\x00\x00\x03\x10\x10\x03\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x31\x87\x01\x01",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x08\xA0\x00\x00\x00\x03\x10\x10\x03\x00",13))
	{
		pEMVICCOut->uiLenOut =95;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x5D\x84\x08\xA0\x00\x00\x00\x03\x10\x10\x03\xA5\x51\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x31\x87\x01\x01\x9F\x38\x23\x9F\x37\x04\x9F\x66\x04\x9F\x02\x06\x5F\x2A\x02\x9F\x01\x06\x9F\x40\x05\x81\x04\x9F\x04\x04\x9F\x03\x06\x9F\x3A\x04\x9F\x06\x10\x9F\x34\x03\x5F\x2D\x04\x65\x73\x65\x6E\x9F\x11\x01\x01\x9F\x12\x0C\x50\x72\x65\x66\x20\x4E\x61\x6D\x65\x20\x30\x31",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x80\xA8\x00\x00",4))
	{
		pEMVICCOut->uiLenOut =149;
		memcpy(pEMVICCOut->aucDataOut,"\x77\x81\x92\x82\x02\x70\x00\x94\x08\x08\x01\x02\x00\x10\x01\x02\x01\x9F\x36\x02\x00\x03\x57\x13\x47\x61\x73\x90\x01\x01\x00\x10\xD1\x01\x21\x20\x00\x12\x33\x99\x00\x03\x1F\x9F\x10\x07\x06\x01\x11\x03\x90\x00\x00\x9F\x26\x08\xAA\xBB\xCC\xDD\xEE\xFF\x11\x22\x5F\x34\x01\x01\x9F\x6C\x02\x20\x00\x9F\x5D\x06\x00\x00\x00\x01\x00\x00\x9F\x4B\x40\x53\xA1\x96\xAB\x02\xA6\xD3\x4C\x12\xA0\xAB\x99\xBD\x62\x30\x4B\xA8\xD4\xE5\xF1\x28\xDD\x23\xEF\xD0\x13\xC0\xD5\x56\x00\x70\xAB\x2A\x5F\x3B\x71\x87\xEC\x68\xF4\x43\xEF\xCB\xB3\xDF\x6F\x73\x3D\x1F\x96\xFA\x18\xFB\x08\x68\x2D\x58\xE8\x56\x21\xED\x48\x8F\x17",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =179;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\xB0\x8F\x01\x50\x90\x81\x80\x6F\xFD\x93\x0F\xE8\x43\x7A\x9C\x2E\x8B\x77\x1E\x93\x07\x49\x6B\x6C\x74\xC5\x08\xB8\xBC\x2C\xC5\x1E\x6F\xEE\xE6\x6C\x1D\x75\xE6\xA5\x46\x0E\x07\xCA\x42\x6B\x0D\x30\xDD\x5F\x78\x25\xAE\x38\xF2\x09\x94\x09\x76\x68\x9A\x0B\xEB\x18\x82\x3E\x26\xE1\xB1\x38\x1C\xA7\x22\x20\xF6\xBB\x99\xFD\x85\x05\xF3\xD7\x71\x4A\x26\x7A\xAE\xB1\xBC\x6E\x69\x0A\xD6\xC5\x7F\x98\xC9\xE9\x9C\x3D\x6D\x8B\x4D\x63\xAF\xA9\xEA\x85\x91\xF3\x47\x12\x94\x2F\x3F\x5E\xDC\xDA\x93\xB0\xC6\x02\x4D\x22\x67\x9C\x05\xD6\x60\x4E\xF9\xC3\xCB\xFC\xD7\x92\x24\xEA\xA4\x6C\x23\x7A\xDF\x99\x66\x36\xDF\xE2\x97\x15\xCA\x63\x7A\x95\xBE\x48\xF2\xC7\x7F\x93\xC8\x3F\x9F\xC6\xBF\xEB\x72\x30\x01\xF9\x8E\xE7\x23\x9F\x32\x01\x03",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =143;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\x8C\x9F\x46\x81\x80\x7B\x32\xE7\x7E\x6D\x81\x43\xE8\xA4\x4D\xA2\x0C\x60\x13\x37\xE4\xD9\x3D\x82\xCE\x6F\xEE\x70\xFF\x09\xBE\x3D\x1A\x8B\x03\xD5\x12\x6F\x82\x40\x62\xF8\xE9\x37\xCC\xEC\x7A\x9C\x47\x0C\x90\x0E\x2E\xC6\x49\x2E\xC7\x15\x90\x29\x45\x20\x6C\x37\xBF\xED\xFC\xAB\xD5\x8C\xC1\xA8\x8D\x2C\x64\xD6\x2A\xB1\xD8\x1F\xD2\xF1\xB5\x1B\xE7\xCB\xB3\xFD\x71\x09\xC3\xE3\x8D\xE2\xFF\xB0\x78\xD8\x06\x9E\xD8\x51\x8B\x3E\x5D\x7B\x60\xD2\x0A\x0B\x8A\x02\x39\xDA\x33\x90\xD0\x64\xF1\xFD\x63\x5A\xEB\x5B\x57\x90\x87\xB8\xCC\x2F\xA0\x0F\x73\x9F\x47\x01\x03\x9F\x4A\x01\x82",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =18;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x10\x5A\x08\x47\x61\x73\x90\x01\x01\x00\x10\x5F\x24\x03\x10\x12\x31",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =10;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x08\x9F\x69\x05\x01\x11\x22\x33\x44",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}

	


	//ICCPK ERROR
	if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x0E\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\x00",20))
	{
		pEMVICCOut->uiLenOut =81;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x4F\x84\x0E\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\xA5\x3D\xBF\x0C\x3A\x61\x1B\x4F\x08\xA0\x00\x00\x00\x03\x10\x10\x03\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x31\x87\x01\x01\x61\x1B\x4F\x08\xA0\x00\x00\x00\x03\x10\x10\x04\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x32\x87\x01\x02",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x08\xA0\x00\x00\x00\x03\x10\x10\x03\x00",13))
	{
		pEMVICCOut->uiLenOut =72;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x46\x84\x08\xA0\x00\x00\x00\x03\x10\x10\x03\xA5\x3A\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x31\x87\x01\x01\x9F\x38\x0C\x9F\x37\x04\x9F\x66\x04\x9F\x02\x06\x5F\x2A\x02\x5F\x2D\x04\x65\x73\x65\x6E\x9F\x11\x01\x01\x9F\x12\x0C\x50\x72\x65\x66\x20\x4E\x61\x6D\x65\x20\x30\x31",pEMVICCOut->uiLenOut);
	
	}
	else if(!memcmp(&aucBuff[1],"\x80\xA8\x00\x00",4))
	{
		pEMVICCOut->uiLenOut =113;
		memcpy(pEMVICCOut->aucDataOut,"\x77\x6F\x82\x02\x70\x00\x94\x28\x08\x01\x03\x00\x58\x01\x01\x00\x58\x01\x01\x00\x58\x01\x01\x00\x58\x01\x01\x00\x58\x01\x01\x00\x58\x01\x01\x00\x58\x01\x01\x00\x58\x01\x01\x00\x58\x01\x01\x00\x9F\x36\x02\x00\x03\x57\x13\x47\x61\x73\x90\x01\x01\x00\x10\xD1\x01\x21\x20\x00\x12\x33\x99\x00\x03\x1F\x9F\x10\x07\x06\x01\x11\x03\x90\x00\x00\x9F\x26\x08\xAA\xBB\xCC\xDD\xEE\xFF\x11\x22\x5F\x34\x01\x01\x9F\x6C\x02\x20\x00\x9F\x5D\x06\x00\x00\x00\x01\x00\x00",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =179;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\xB0\x8F\x01\x50\x90\x81\x80\x6F\xFD\x93\x0F\xE8\x43\x7A\x9C\x2E\x8B\x77\x1E\x93\x07\x49\x6B\x6C\x74\xC5\x08\xB8\xBC\x2C\xC5\x1E\x6F\xEE\xE6\x6C\x1D\x75\xE6\xA5\x46\x0E\x07\xCA\x42\x6B\x0D\x30\xDD\x5F\x78\x25\xAE\x38\xF2\x09\x94\x09\x76\x68\x9A\x0B\xEB\x18\x82\x3E\x26\xE1\xB1\x38\x1C\xA7\x22\x20\xF6\xBB\x99\xFD\x85\x05\xF3\xD7\x71\x4A\x26\x7A\xAE\xB1\xBC\x6E\x69\x0A\xD6\xC5\x7F\x98\xC9\xE9\x9C\x3D\x6D\x8B\x4D\x63\xAF\xA9\xEA\x85\x91\xF3\x47\x12\x94\x2F\x3F\x5E\xDC\xDA\x93\xB0\xC6\x02\x4D\x22\x67\x9C\x05\xD6\x60\x4E\xF9\xC3\xCB\xFC\xD7\x92\x24\xEA\xA4\x6C\x23\x7A\xDF\x99\x66\x36\xDF\xE2\x97\x15\xCA\x63\x7A\x95\xBE\x48\xF2\xC7\x7F\x93\xC8\x3F\x9F\xC6\xBF\xEB\x72\x30\x01\xF9\x8E\xE7\x23\x9F\x32\x01\x03",pEMVICCOut->uiLenOut);
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =143;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\x8C\x9F\x46\x81\x80\x7B\x32\xE7\x7E\x6D\x81\x43\xE8\xA4\x4D\xA2\x0C\x60\x13\x37\xE4\xD9\x3D\x82\xCE\x6F\xEE\x70\xFF\x09\xBE\x3D\x1A\x8B\x03\xD5\x12\x6F\x82\x40\x62\xF8\xE9\x37\xCC\xEC\x7A\x9C\x47\x0C\x90\x0E\x2E\xC6\x49\x2E\xC7\x15\x90\x29\x45\x20\x6C\x37\xBF\xED\xFC\xAB\xD5\x8C\xC1\xA8\x8D\x2C\x64\xD6\x2A\xB1\xD8\x1F\xD2\xF1\xB5\x1B\xE7\xCB\xB3\xFD\x71\x09\xC3\xE3\x8D\xE2\xFF\xB0\x78\xD8\x06\x9E\xD8\x51\x8B\x3E\x5D\x7B\x60\xD2\x0A\x0B\x8A\x02\x39\xDA\x33\x90\xD0\x64\xF1\xFD\x63\x5A\xEB\x5B\x57\x90\x87\xB8\xCC\x2F\xA0\x0F\x73\x9F\x47\x01\x03\x9F\x4A\x01\x82",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x03\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =18;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x10\x5A\x08\x47\x61\x73\x90\x01\x01\x00\x10\x5F\x24\x03\x10\x12\x31",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x5C\x00",5))
	{
		pEMVICCOut->uiLenOut =6;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x04\x85\x02\x12\x34",pEMVICCOut->uiLenOut);
		
	}



	// CAPK ERROR2
	if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x0E\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\x00",20))
	{
		pEMVICCOut->uiLenOut =81;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x4F\x84\x0E\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\xA5\x3D\xBF\x0C\x3A\x61\x1B\x4F\x08\xA0\x00\x00\x00\x03\x10\x10\x03\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x31\x87\x01\x01\x61\x1B\x4F\x08\xA0\x00\x00\x00\x03\x10\x10\x04\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x32\x87\x01\x02",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x08\xA0\x00\x00\x00\x03\x10\x10\x03\x00",13))
	{
		pEMVICCOut->uiLenOut =72;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x46\x84\x08\xA0\x00\x00\x00\x03\x10\x10\x03\xA5\x3A\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x31\x87\x01\x01\x9F\x38\x0C\x9F\x37\x04\x9F\x66\x04\x9F\x02\x06\x5F\x2A\x02\x5F\x2D\x04\x65\x73\x65\x6E\x9F\x11\x01\x01\x9F\x12\x0C\x50\x72\x65\x66\x20\x4E\x61\x6D\x65\x20\x30\x31",pEMVICCOut->uiLenOut);
	
	}
	else if(!memcmp(&aucBuff[1],"\x80\xA8\x00\x00",4))
	{
		pEMVICCOut->uiLenOut =149;
		memcpy(pEMVICCOut->aucDataOut,"\x77\x81\x92\x82\x02\x30\x00\x94\x08\x08\x01\x02\x00\x10\x01\x02\x00\x9F\x36\x02\x00\x03\x57\x13\x47\x61\x73\x90\x01\x01\x00\x10\xD1\x01\x21\x20\x00\x12\x33\x99\x00\x03\x1F\x9F\x10\x07\x06\x01\x11\x03\x90\x00\x00\x9F\x26\x08\xAA\xBB\xCC\xDD\xEE\xFF\x11\x22\x5F\x34\x01\x01\x9F\x6C\x02\x30\x00\x9F\x5D\x06\x00\x00\x00\x01\x00\x00\x9F\x4B\x40\x53\xA1\x96\xAB\x02\xA6\xD3\x4C\x12\xA0\xAB\x99\xBD\x62\x30\x4B\xA8\xD4\xE5\xF1\x28\xDD\x23\xEF\xD0\x13\xC0\xD5\x56\x00\x70\xAB\x2A\x5F\x3B\x71\x87\xEC\x68\xF4\x43\xEF\xCB\xB3\xDF\x6F\x73\x3D\x1F\x96\xFA\x18\xFB\x08\x68\x2D\x58\xE8\x56\x21\xED\x48\x8F\x17",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =179;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\xB0\x8F\x01\x50\x90\x81\x80\x6F\xFD\x93\x0F\xE8\x43\x7A\x9C\x2E\x8B\x77\x1E\x93\x07\x49\x6B\x6C\x74\xC5\x08\xB8\xBC\x2C\xC5\x1E\x6F\xEE\xE6\x6C\x1D\x75\xE6\xA5\x46\x0E\x07\xCA\x42\x6B\x0D\x30\xDD\x5F\x78\x25\xAE\x38\xF2\x09\x94\x09\x76\x68\x9A\x0B\xEB\x18\x82\x3E\x26\xE1\xB1\x38\x1C\xA7\x22\x20\xF6\xBB\x99\xFD\x85\x05\xF3\xD7\x71\x4A\x26\x7A\xAE\xB1\xBC\x6E\x69\x0A\xD6\xC5\x7F\x98\xC9\xE9\x9C\x3D\x6D\x8B\x4D\x63\xAF\xA9\xEA\x85\x91\xF3\x47\x12\x94\x2F\x3F\x5E\xDC\xDA\x93\xB0\xC6\x02\x4D\x22\x67\x9C\x05\xD6\x60\x4E\xF9\xC3\xCB\xFC\xD7\x92\x24\xEA\xA4\x6C\x23\x7A\xDF\x99\x66\x36\xDF\xE2\x97\x15\xCA\x63\x7A\x95\xBE\x48\xF2\xC7\x7F\x93\xC8\x3F\x9F\xC6\xBF\xEB\x72\x30\x01\xF9\x8E\xE7\x23\x9F\x32\x01\x03",pEMVICCOut->uiLenOut);
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =139;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\x88\x9F\x46\x81\x80\x8B\x55\x68\x22\x57\xAB\x43\xC8\x2D\xB3\x42\x10\x4F\xB7\xC6\x29\x82\x06\x3C\x06\xE1\x79\x56\xF3\x15\x15\x5B\xAE\xCB\x1B\x37\x91\x36\x54\xE8\xF9\x79\x11\x3D\xAD\xB8\x4B\xA6\xB3\x80\x8E\x85\xF2\x44\x0F\x38\x13\x45\x50\x69\x21\x3F\xED\xE0\x20\xB5\x1D\x72\x9F\x9A\x1C\xCB\x54\x27\xEC\x64\x86\xE4\x65\x19\xC6\xDC\xFC\x9C\xB2\xD6\xAC\x6C\xF7\x47\xDB\x07\x2E\x74\x1F\xFF\x43\x79\xAE\xD7\x9A\x72\x3B\x60\xAD\x64\xF5\x1E\xD5\xEB\xA1\x57\x04\x44\x85\xFD\x53\x74\xFE\x47\x77\xB0\xBE\x68\xB6\x55\x74\x6B\x1E\x26\x37\x17\xB6\x9F\x47\x01\x03",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =18;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x10\x5A\x08\x47\x61\x73\x90\x01\x01\x00\x10\x5F\x24\x03\x10\x12\x31",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =10;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x08\x9F\x69\x05\x01\x11\x22\x33\x44",pEMVICCOut->uiLenOut);
		
	}





	// CAPK ERROR2
	if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x0E\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\x00",20))
	{
		pEMVICCOut->uiLenOut =81;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x4F\x84\x0E\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\xA5\x3D\xBF\x0C\x3A\x61\x1B\x4F\x08\xA0\x00\x00\x00\x03\x10\x10\x03\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x31\x87\x01\x01\x61\x1B\x4F\x08\xA0\x00\x00\x00\x03\x10\x10\x04\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x32\x87\x01\x02",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x08\xA0\x00\x00\x00\x03\x10\x10\x03\x00",13))
	{
		pEMVICCOut->uiLenOut =72;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x46\x84\x08\xA0\x00\x00\x00\x03\x10\x10\x03\xA5\x3A\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x31\x87\x01\x01\x9F\x38\x0C\x9F\x37\x04\x9F\x66\x04\x9F\x02\x06\x5F\x2A\x02\x5F\x2D\x04\x65\x73\x65\x6E\x9F\x11\x01\x01\x9F\x12\x0C\x50\x72\x65\x66\x20\x4E\x61\x6D\x65\x20\x30\x31",pEMVICCOut->uiLenOut);
	
	}
	else if(!memcmp(&aucBuff[1],"\x80\xA8\x00\x00",4))
	{
		pEMVICCOut->uiLenOut =149;
		memcpy(pEMVICCOut->aucDataOut,"\x77\x81\x92\x82\x02\x70\x00\x94\x08\x08\x01\x02\x00\x10\x01\x02\x01\x9F\x36\x02\x00\x03\x57\x13\x47\x61\x73\x90\x01\x01\x00\x10\xD1\x01\x21\x20\x00\x12\x33\x99\x00\x03\x1F\x9F\x10\x07\x06\x01\x11\x03\x90\x00\x00\x9F\x26\x08\xAA\xBB\xCC\xDD\xEE\xFF\x11\x22\x5F\x34\x01\x01\x9F\x6C\x02\x20\x00\x9F\x5D\x06\x00\x00\x00\x01\x00\x00\x9F\x4B\x40\x55\x75\xAF\x0B\xC2\x03\xD9\xBD\x21\x11\xFD\x30\xFA\xDE\x8A\x56\xA9\x7B\xD3\xE4\xC8\xC9\xBA\xDF\xE2\xEB\x20\xC6\x1A\xDF\xF8\xD5\x26\x83\xD5\x24\x65\x2D\x7A\xFE\xAD\x94\xE1\x35\xED\xAD\xE0\x3A\x36\x8B\x3B\x04\x2A\xE2\x5E\xB5\x4B\x1F\x91\x5E\x21\x02\xFB\x82",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =179;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\xB0\x8F\x01\x50\x90\x81\x80\x6F\xFD\x93\x0F\xE8\x43\x7A\x9C\x2E\x8B\x77\x1E\x93\x07\x49\x6B\x6C\x74\xC5\x08\xB8\xBC\x2C\xC5\x1E\x6F\xEE\xE6\x6C\x1D\x75\xE6\xA5\x46\x0E\x07\xCA\x42\x6B\x0D\x30\xDD\x5F\x78\x25\xAE\x38\xF2\x09\x94\x09\x76\x68\x9A\x0B\xEB\x18\x82\x3E\x26\xE1\xB1\x38\x1C\xA7\x22\x20\xF6\xBB\x99\xFD\x85\x05\xF3\xD7\x71\x4A\x26\x7A\xAE\xB1\xBC\x6E\x69\x0A\xD6\xC5\x7F\x98\xC9\xE9\x9C\x3D\x6D\x8B\x4D\x63\xAF\xA9\xEA\x85\x91\xF3\x47\x12\x94\x2F\x3F\x5E\xDC\xDA\x93\xB0\xC6\x02\x4D\x22\x67\x9C\x05\xD6\x60\x4E\xF9\xC3\xCB\xFC\xD7\x92\x24\xEA\xA4\x6C\x23\x7A\xDF\x99\x66\x36\xDF\xE2\x97\x15\xCA\x63\x7A\x95\xBE\x48\xF2\xC7\x7F\x93\xC8\x3F\x9F\xC6\xBF\xEB\x72\x30\x01\xF9\x8E\xE7\x23\x9F\x32\x01\x03",pEMVICCOut->uiLenOut);
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =143;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\x8C\x9F\x46\x81\x80\x7B\x32\xE7\x7E\x6D\x81\x43\xE8\xA4\x4D\xA2\x0C\x60\x13\x37\xE4\xD9\x3D\x82\xCE\x6F\xEE\x70\xFF\x09\xBE\x3D\x1A\x8B\x03\xD5\x12\x6F\x82\x40\x62\xF8\xE9\x37\xCC\xEC\x7A\x9C\x47\x0C\x90\x0E\x2E\xC6\x49\x2E\xC7\x15\x90\x29\x45\x20\x6C\x37\xBF\xED\xFC\xAB\xD5\x8C\xC1\xA8\x8D\x2C\x64\xD6\x2A\xB1\xD8\x1F\xD2\xF1\xB5\x1B\xE7\xCB\xB3\xFD\x71\x09\xC3\xE3\x8D\xE2\xFF\xB0\x78\xD8\x06\x9E\xD8\x51\x8B\x3E\x5D\x7B\x60\xD2\x0A\x0B\x8A\x02\x39\xDA\x33\x90\xD0\x64\xF1\xFD\x63\x5A\xEB\x5B\x57\x90\x87\xB8\xCC\x2F\xA0\x0F\x73\x9F\x47\x01\x03\x9F\x4A\x01\x82",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =18;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x10\x5A\x08\x47\x61\x73\x90\x01\x01\x00\x10\x5F\x24\x03\x10\x12\x31",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =10;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x08\x9F\x69\x05\x01\x11\x22\x33\x44",pEMVICCOut->uiLenOut);
		
	}

		*/
	if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x0E\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\x00",20))
	{
		pEMVICCOut->uiLenOut =81;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x4F\x84\x0E\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\xA5\x3D\xBF\x0C\x3A\x61\x1B\x4F\x08\xA0\x00\x00\x00\x03\x10\x10\x03\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x31\x87\x01\x01\x61\x1B\x4F\x08\xA0\x00\x00\x00\x03\x10\x10\x04\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x32\x87\x01\x02",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x08\xA0\x00\x00\x00\x03\x10\x10\x03\x00",13))
	{
		pEMVICCOut->uiLenOut =72;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x46\x84\x08\xA0\x00\x00\x00\x03\x10\x10\x03\xA5\x3A\x50\x0C\x56\x49\x53\x41\x20\x54\x45\x53\x54\x20\x30\x31\x87\x01\x01\x9F\x38\x0C\x9F\x37\x04\x9F\x66\x04\x9F\x02\x06\x5F\x2A\x02\x5F\x2D\x04\x65\x73\x65\x6E\x9F\x11\x01\x01\x9F\x12\x0C\x50\x72\x65\x66\x20\x4E\x61\x6D\x65\x20\x30\x31",pEMVICCOut->uiLenOut);
	
	}
	else if(!memcmp(&aucBuff[1],"\x80\xA8\x00\x00",4))
	{
		pEMVICCOut->uiLenOut =81;
		memcpy(pEMVICCOut->aucDataOut,"\x77\x4F\x82\x02\x70\x00\x94\x08\x08\x01\x02\x00\x10\x01\x02\x01\x9F\x36\x02\x00\x03\x57\x13\x47\x61\x73\x90\x01\x01\x00\x10\xD1\x01\x21\x20\x00\x12\x33\x99\x00\x03\x1F\x9F\x10\x07\x06\x01\x11\x03\x90\x00\x00\x9F\x26\x08\xAA\xBB\xCC\xDD\xEE\xFF\x11\x22\x5F\x34\x01\x01\x9F\x6C\x02\x20\x00\x9F\x5D\x06\x00\x00\x00\x01\x00\x00",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =179;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\xB0\x8F\x01\x50\x90\x81\x80\x6F\xFD\x93\x0F\xE8\x43\x7A\x9C\x2E\x8B\x77\x1E\x93\x07\x49\x6B\x6C\x74\xC5\x08\xB8\xBC\x2C\xC5\x1E\x6F\xEE\xE6\x6C\x1D\x75\xE6\xA5\x46\x0E\x07\xCA\x42\x6B\x0D\x30\xDD\x5F\x78\x25\xAE\x38\xF2\x09\x94\x09\x76\x68\x9A\x0B\xEB\x18\x82\x3E\x26\xE1\xB1\x38\x1C\xA7\x22\x20\xF6\xBB\x99\xFD\x85\x05\xF3\xD7\x71\x4A\x26\x7A\xAE\xB1\xBC\x6E\x69\x0A\xD6\xC5\x7F\x98\xC9\xE9\x9C\x3D\x6D\x8B\x4D\x63\xAF\xA9\xEA\x85\x91\xF3\x47\x12\x94\x2F\x3F\x5E\xDC\xDA\x93\xB0\xC6\x02\x4D\x22\x67\x9C\x05\xD6\x60\x4E\xF9\xC3\xCB\xFC\xD7\x92\x24\xEA\xA4\x6C\x23\x7A\xDF\x99\x66\x36\xDF\xE2\x97\x15\xCA\x63\x7A\x95\xBE\x48\xF2\xC7\x7F\x93\xC8\x3F\x9F\xC6\xBF\xEB\x72\x30\x01\xF9\x8E\xE7\x23\x9F\x32\x01\x03",pEMVICCOut->uiLenOut);
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =164;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\xA1\x9F\x46\x81\x80\x7B\x32\xE7\x7E\x6D\x81\x43\xE8\xA4\x4D\xA2\x0C\x60\x13\x37\xE4\xD9\x3D\x82\xCE\x6F\xEE\x70\xFF\x09\xBE\x3D\x1A\x8B\x03\xD5\x12\x6F\x82\x40\x62\xF8\xE9\x37\xCC\xEC\x7A\x9C\x47\x0C\x90\x0E\x2E\xC6\x49\x2E\xC7\x15\x90\x29\x45\x20\x6C\x37\xBF\xED\xFC\xAB\xD5\x8C\xC1\xA8\x8D\x2C\x64\xD6\x2A\xB1\xD8\x1F\xD2\xF1\xB5\x1B\xE7\xCB\xB3\xFD\x71\x09\xC3\xE3\x8D\xE2\xFF\xB0\x78\xD8\x06\x9E\xD8\x51\x8B\x3E\x5D\x7B\x60\xD2\x0A\x0B\x8A\x02\x39\xDA\x33\x90\xD0\x64\xF1\xFD\x63\x5A\xEB\x5B\x57\x90\x87\xB8\xCC\x2F\xA0\x0F\x73\x9F\x47\x01\x03\x9F\x4A\x01\x82\x57\x13\x47\x61\x73\x90\x01\x01\x00\x10\xD1\x01\x21\x20\x00\x12\x33\x99\x00\x03\x1F",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =18;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x10\x5A\x08\x47\x61\x73\x90\x01\x01\x00\x10\x5F\x24\x03\x10\x12\x31",pEMVICCOut->uiLenOut);
		
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =10;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x08\x9F\x69\x05\x01\x11\x22\x33\x44",pEMVICCOut->uiLenOut);
		
	}
	pEMVICCOut->ucSWA = 0x90;
	pEMVICCOut->ucSWB= 0x00;
	
	ucResult =EMVERROR_SUCCESS;
	
#endif
	return ucResult;
}
#endif

void DIGITAL_Package(PUCHAR  pucData,unsigned int uiLen)
{
	unsigned int iLg;
	int iI;

	memset(aucSendRecvBuf,0,sizeof(aucSendRecvBuf));
	uiSendRecvLen = 0;

	/*STX*/
	aucSendRecvBuf[uiSendRecvLen++] = STX;

	/*Data Length*/
	iLg = uiLen+3;
	CONV_IntHex((unsigned char *)&aucSendRecvBuf[uiSendRecvLen],2,&iLg);
	uiSendRecvLen += 2;

	/*Data*/
	aucSendRecvBuf[uiSendRecvLen++] = 0x60;
	aucSendRecvBuf[uiSendRecvLen++] = 0x02;
	aucSendRecvBuf[uiSendRecvLen++] = 0x01;
	memcpy(&aucSendRecvBuf[uiSendRecvLen],pucData,uiLen);
	uiSendRecvLen += uiLen;

	/*ETX*/
	aucSendRecvBuf[uiSendRecvLen++] = ETX;

	/*LRC*/
	aucSendRecvBuf[uiSendRecvLen] = (char)0xff;
	for(iI=1;iI<uiSendRecvLen-1;iI++)
	{
		aucSendRecvBuf[uiSendRecvLen] ^= aucSendRecvBuf[iI];
	}
	uiSendRecvLen ++;
}

UCHAR DIGITAL_PolingAbort(void)
{
	UCHAR	aucBuff[10],ucResult=0;
	USHORT	uiLen;
#if 1
	
//	ucResult =DIGITAL_OpenSerial();
//	if(!ucResult)
	{
	
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiLen =0;
		aucBuff[uiLen++] =00;
		
		DIGITAL_AbortPackage(aucBuff,uiLen);
		DIGITAL_SendData(aucSendRecvBuf,uiSendRecvLen);

//		DIGITAL_ComClose();
	}
#endif
	return ucResult;
	
}

void DIGITAL_AbortPackage(PUCHAR  pucData,unsigned int uiLen)
{
	unsigned int iLg;
	int iI;

	memset(aucSendRecvBuf,0,sizeof(aucSendRecvBuf));
	uiSendRecvLen = 0;

	/*STX*/
	aucSendRecvBuf[uiSendRecvLen++] = STX;

	/*Data Length*/
	iLg = uiLen+2;
	CONV_IntHex((unsigned char *)&aucSendRecvBuf[uiSendRecvLen],2,&iLg);
	uiSendRecvLen += 2;

	/*Data*/
	aucSendRecvBuf[uiSendRecvLen++] = 0x60;
	aucSendRecvBuf[uiSendRecvLen++] = 0x02;
//	aucSendRecvBuf[uiSendRecvLen++] = 0x01;
	memcpy(&aucSendRecvBuf[uiSendRecvLen],pucData,uiLen);
	uiSendRecvLen += uiLen;

	/*ETX*/
	aucSendRecvBuf[uiSendRecvLen++] = ETX;

	/*LRC*/
	aucSendRecvBuf[uiSendRecvLen] = (char)0xff;
	for(iI=1;iI<uiSendRecvLen-1;iI++)
	{
		aucSendRecvBuf[uiSendRecvLen] ^= aucSendRecvBuf[iI];
	}
	uiSendRecvLen ++;
}

UCHAR	DIGITAL_DeSelect(void)
{
	UCHAR	aucBuff[10],ucResult;

//	ucResult =DIGITAL_OpenSerial();
//	if(ucResult) return ucResult;
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	aucBuff[0] = CMD_DeSelect;
	DIGITAL_Package(aucBuff,1);
	DIGITAL_SendData(aucSendRecvBuf,uiSendRecvLen);
	
	ucResult =DIGITAL_RecvData(true,1);
	if(!ucResult)
	{
		if(aucSendRecvBuf[5]!=aucBuff[0])
			ucResult =EMVERROR_ICCCOMAND;
		else if(aucSendRecvBuf[6])
			ucResult =EMVERROR_ICCCOMAND; 
	}
//	DIGITAL_ComClose();
	
	return ucResult;
}

UCHAR	DIGITAL_Light(UCHAR ucLightCtr)
{
	UCHAR	aucBuff[10],ucResult;

//	ucResult =DIGITAL_OpenSerial();
//	if(ucResult) return ucResult;
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	aucBuff[0] = CMD_Light;
	aucBuff[1] = ucLightCtr;
	DIGITAL_Package(aucBuff,2);
	DIGITAL_SendData(aucSendRecvBuf,uiSendRecvLen);
	
	ucResult =DIGITAL_RecvData(true,20);
	if(!ucResult)
	{
		if(aucSendRecvBuf[5]!=aucBuff[0])
			ucResult =EMVERROR_ICCCOMAND;
		else if(aucSendRecvBuf[6])
			ucResult =EMVERROR_ICCCOMAND;
	}
//	DIGITAL_ComClose();
	
	return ucResult;
}


UCHAR	DIGITAL_Remove(void)
{
	UCHAR	aucBuff[10],ucResult=0;
	
	memset(&PolingData,0x00,sizeof(POLINGDATA));
//	ucResult =DIGITAL_OpenSerial();
//	if(ucResult) return ucResult;
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	aucBuff[0] =CMD_Remove;
	DIGITAL_Package(aucBuff,1);
	DIGITAL_SendData(aucSendRecvBuf,uiSendRecvLen);
	
	return ucResult;

}


UCHAR	DIGITAL_RemoveProcess(void)
{
	UCHAR	ucResult;

	ucResult =DIGITAL_RecvData(false,10);
	
	if(!ucResult)
	{
		if(aucSendRecvBuf[5]!=CMD_Remove)
			ucResult =EMVERROR_ICCCOMAND;
		else if(!aucSendRecvBuf[6])
			ucResult = EMVERROR_SUCCESS;
		else
			ucResult =EMVERROR_ICCCOMAND;
	}
	else
		DIGITAL_PolingAbort();
//	DIGITAL_ComClose();
	
	return ucResult;

}


UCHAR	DIGITAL_Beep(UCHAR ucNums)
{
	UCHAR	aucBuff[10],ucResult;

//	ucResult =DIGITAL_OpenSerial();
//	if(ucResult) return ucResult;
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	aucBuff[0] = CMD_Beep;
	aucBuff[1] = ucNums;
	DIGITAL_Package(aucBuff,2);
	DIGITAL_SendData(aucSendRecvBuf,uiSendRecvLen);
	
	ucResult =DIGITAL_RecvData(true,20);
	if(!ucResult)
	{
		if(aucSendRecvBuf[5]!=aucBuff[0])
			ucResult =EMVERROR_ICCCOMAND;
		else if(aucSendRecvBuf[6])
			ucResult =EMVERROR_ICCCOMAND;
	}
//	DIGITAL_ComClose();
	
	return ucResult;
}

UCHAR	DIGITAL_InputPin(unsigned char* paucPin)
{
	UCHAR	aucRandNums[8],aucBuff[20],aucWKPin[8],ucResult;
	const char  aucPinKey[]="sandsand";

	
	memset(paucPin,0x00,12);
	ucResult =DIGITAL_OpenSerial();
	util_Printf("\n Open Result %d",ucResult);
	if(ucResult) return ucResult;
	//DIGITAL_Light(TRUE);
	//DIGITAL_Beep(1);
	EMVVERIFY_RandNumGen(aucRandNums,sizeof(aucRandNums));
	memset(aucBuff,0x00,sizeof(aucBuff));
	aucBuff[0] = CMD_InputPIn;
	memcpy(&aucBuff[1] ,aucRandNums,sizeof(aucRandNums));
	DIGITAL_Package(aucBuff,9);
	DIGITAL_SendData(aucSendRecvBuf,uiSendRecvLen);
	
	ucResult =DIGITAL_RecvData(true,60000);
	
	if(!ucResult)
	{
//		util_Printf("\nuiSendRecvLen %d ucResult %d",uiSendRecvLen,ucResult);
		if(aucSendRecvBuf[5]!=CMD_InputPIn)
			ucResult =EMVERROR_ICCCOMAND;
		else if(!aucSendRecvBuf[6])
		{
			des(aucRandNums,aucWKPin,aucPinKey);
			desm1(&aucSendRecvBuf[7],aucBuff,aucWKPin);
			desm1(&aucSendRecvBuf[15],&aucBuff[8],aucWKPin);
			if(aucBuff[0]>12) return EMVERROR_ICCCOMAND;
			if(aucBuff[0]==0) return  EMVERROR_BYPASS;
			memcpy(paucPin,&aucBuff[1],aucBuff[0]);
		}
		else
			ucResult =EMVERROR_CANCEL;
	}
	else
		DIGITAL_PolingAbort();
	DIGITAL_Light(false);	
	DIGITAL_ComClose();
	
	return ucResult;

}
#if SANDREADER
unsigned char DIGITAL_SendReceive(unsigned int uiTime)
{

	unsigned int uiTimesOut;
	DRV_OUT *ptDrvOut;
	unsigned char ucAbort = false;
/*the drive will be used ,and the action type*/	
	Drv.drv_nr = drv_pad;
	Drv.type = execute;
/*clear the global data DrvOut */
	memset(&DrvOut, 0, sizeof(DRV_OUT));
	Drv.pt_drv_in = &DrvIn;
	Drv.pt_drv_out = &DrvOut;
	while(1)
	{
		Os__xdelay(1);
		if(!ucAbort)
		ptDrvOut = OSDRV_CallAsync(&Drv);
		Os__xdelay(1);
		uiTimesOut = uiTime;
		
	 	/*decrease time start*/
		Os__timer_start(&uiTimesOut);
		while((uiTimesOut)&&(ptDrvOut->gen_status == DRV_RUNNING));
		/*stop timer*/
		Os__timer_stop(&uiTimesOut);
		/*overtime*/
	 	if(!uiTimesOut)
		{			
			Os__abort_drv(drv_pad);
			//Os__abort_drv(drv_mmi);
			return 0x04;
		}
		Os__xdelay(1);
		Os__xdelay(1);

		if(  ( ptDrvOut->gen_status == DRV_ABSENT )
	       ||(  ( ptDrvOut->gen_status == DRV_ENDED ) 
	          &&( ptDrvOut->drv_status == DRV_ABSENT ) 
	         )
	       ||( DRV_RUNNING == ptDrvOut->gen_status ))
		{
			Os__abort_drv(drv_pad);
			//Os__abort_drv(drv_mmi);
			return(ERR_DRIVER);
		}
		else
		{
			memcpy(&DrvOut,ptDrvOut,sizeof(DRV_OUT));
			//Os__abort_drv(drv_mmi);
			return(SUCCESS);
		}		
	}
}
unsigned char DIGITAL_CheckCard(void)
{
	unsigned char ucResult = SUCCESS,ucStatus;
	
	DrvIn.type      = 0x6F;
	DrvIn.length	= 0;
	util_Printf("\nDIGITAL_SendReceive");
	ucResult = DIGITAL_SendReceive(50);
	util_Printf("\nDIGITAL_SendReceive==123=%d",ucResult);
	if( ucResult == SUCCESS)
	{
		ucStatus = DrvOut.xxdata[0];
		util_Printf("\nDIGITAL_SendReceive==Status=%d",ucStatus);
		if(!memcmp(&DrvOut.xxdata[1],"\x00\x00\x00\x00",4))
			ucStatus=0x04;
		ucResult =ucStatus;	
	}
	if(ucResult&&ucResult!=0x04)
	{
		if(ucResult==0x02)
		   ucResult =ERR_MULTICARD;
		else
			ucResult =ERR_ICCARD;
	}
	util_Printf("\nDIGITAL_SendReceive==456=%d",ucResult);
	return(ucResult);
}
unsigned char DIGITAL_GetWKCardInfo(void)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucStatus;
	unsigned char ucLen;
	
	DrvIn.type      = 0x18;
	DrvIn.length	= 6;
	util_Printf("\nSend 18\n");
	memcpy(DrvIn.xxdata,"\xA0\xA1\xA2\xA3\xA4\xA5",6);	
	util_Printf("\nDIGITAL_SendReceive");
	ucResult = DIGITAL_SendReceive(50);
	util_Printf("\nDIGITAL_SendReceive==123=%d\n",ucResult);
	if( ucResult == SUCCESS)
	{
		for(ucLen=0;ucLen<32;ucLen++)
		util_Printf("%02x ",DrvOut.xxdata[ucLen]);
		ucStatus = DrvOut.xxdata[0];
		if(!memcmp(&DrvOut.xxdata[1],"\x00\x00\x00\x00",4))
			ucStatus=0x04;
		ucResult =ucStatus;	
	}
	util_Printf("\nDIGITAL_SendReceive==456=%d",ucResult);
	return(ucResult);
}
unsigned char DIGITAL_SendReceive_tmp(void)
{

	unsigned int uiTimesOut;
	DRV_OUT *ptKey;
	DRV_OUT *ptDrvOut;
	unsigned char ucAbort = false;
/*the drive will be used ,and the action type*/	
	Drv.drv_nr = drv_pad;
	Drv.type = execute;
/*clear the global data DrvOut */
	memset(&DrvOut, 0, sizeof(DRV_OUT));
	Drv.pt_drv_in = &DrvIn;
	Drv.pt_drv_out = &DrvOut;
	
		
	while(1)
	{

		ptKey = Os__get_key();
		if(!ucAbort)
		ptDrvOut = OSDRV_CallAsync(&Drv);	
		uiTimesOut = 500;
	 	/*decrease time start*/
		Os__timer_start(&uiTimesOut);
		while((uiTimesOut)&&(ptDrvOut->gen_status == DRV_RUNNING)&&(ptKey->gen_status != DRV_ENDED));
		/*stop timer*/
		Os__timer_stop(&uiTimesOut);
		/*overtime*/
	 	if(!uiTimesOut)
		{			
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			/*S_RUNDATA_ucDialRetry++;
			if(S_RUNDATA_ucDialRetry == 5)
				return ERR_APP_TIMEOUT;
			return ERR_JTK_TMPDEFINE;*/
			return 0x04;
		}
		/*key pressed*/
		if(ptKey->gen_status == DRV_ENDED)
	 	{
	 		if(ptKey->xxdata[1] == KEY_CLEAR)
	 		{
				Os__xdelay(100);
	 			Os__abort_drv(drv_pad);
	 			return ERR_CANCEL;
	 		}
	 		else
	 		{
	 			ucAbort = true;
 				continue;
 			}
	 	}
	 	/*drv end*/

		if(  ( ptDrvOut->gen_status == DRV_ABSENT )
	       ||(  ( ptDrvOut->gen_status == DRV_ENDED ) 
	          &&( ptDrvOut->drv_status == DRV_ABSENT ) 
	         )
	       ||( DRV_RUNNING == ptDrvOut->gen_status ))
		{
			Os__abort_drv(drv_pad);
			Os__abort_drv(drv_mmi);
			return(ERR_DRIVER);
		}else
		{
			memcpy(&DrvOut,ptDrvOut,sizeof(DRV_OUT));
			Os__abort_drv(drv_mmi);
			return(SUCCESS);
		}
	}

}
unsigned char DIGITAL_RemoveCard(uchar ucDisFlag)
{
		unsigned char ucResult,ucFlag=ucDisFlag;
		
		while(1)
		{
			ucResult=DIGITAL_CheckCard();
			if(ucResult ==0x04)
			{
				ucResult= SUCCESS;
				break;
			}
			if(ucFlag)
			{
				//Os__clr_display(3);
				//Os__GB2312_display(3,2,"«Î»°ø®");
				ucFlag =0;
			}
		}			
		return ucResult;		
}
#endif
