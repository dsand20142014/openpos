#include	<Msg.h>
#include	<QPBOC.h>
#include	<Global.h>
extern const QDATAPROPERTY QTermDataProperty[];
extern UCHAR 	MagTrack2[40];
const	PARAMINDICATE	ParamIndicate[]=
{
	{0x01,	QAPPVERNUMBERLEN,		NULL,	(PUCHAR)&QConfig.aucAppVerNumber},
	{0x02,	4,						NULL,	(PUCHAR)&QConfig.uiFloorLimit},
	{0x03,	1,						NULL,	(PUCHAR)&QConfig.ucTermType},	
	{0x04,	4,						NULL,	(PUCHAR)&QConfig.uiThreshold},	
	{0x05,	999,						(PUCHAR)&QConfig.ucDefaultDOLLen,(PUCHAR)&QConfig.aucDefaultDOL},	
	{0x06,	1,						NULL,	(PUCHAR)&QConfig.ucMaxTragetPercent},	
	{0x07,	QTERMCOUNTRYCODELEN,		NULL,	(PUCHAR)&QConfig.aucTermCountryCode},	
	{0x08,	1,						NULL,	(PUCHAR)&QConfig.ucTragetPercent},
	{0x09,	QTRANSCURRENCYCODELEN,	NULL,	(PUCHAR)&QConfig.aucTransCurrencyCode},	
	{0x0A,	1,						NULL,	(PUCHAR)&QConfig.ucTransCurrencyExp},	
	{0x0B,	QTERMCAPABLEN,			NULL,	(PUCHAR)&QConfig.aucTermCapab},	
	{0x0C,	QTERMADDCAPABLEN,		NULL,	(PUCHAR)&QConfig.aucTermAddCapab},	
	{0x0D,	999,						(PUCHAR)&QConfig.ucDefaultTDOLLen,(PUCHAR)&QConfig.aucDefaultTDOL},
	{0x0E,	5,						NULL,	(PUCHAR)&QConfig.TAC.aucDenial},
	{0x0F,	5,						NULL,	(PUCHAR)&QConfig.TAC.aucOnline},
	{0x10,	5,						NULL,	(PUCHAR)&QConfig.TAC.aucDefault}
};

UCHAR	MSG_OpenCom(void)
{
	UCHAR	ucResult;
	
	COM_PARAM CommParam =
	{
		1,     /* Stop bit */
		9600, /* Speed */
		1,	   /* MODEM 0 RS232 1 HDLC 2 */
		8,     /* Data bit */
	   'N'     /* No parity */
	};
//	MSG_ComClose();
	if(QConstParam.ucCommID ==0)
		ucResult = Os__init_com(&CommParam);
	else
//		ucResult = Os__init_com3(0x0303, 0x1400, 0x14);
		ucResult = Os__init_com3(0x0303, 0x0c00, 0x0c);
	if(ucResult) ucResult =QERROR_OPENCOM;
	
	return ucResult;
	
}

UCHAR MSG_ComSendData(PUCHAR pucBuff, USHORT uiLen)
{
	USHORT 	uiI;
	UCHAR	ucHigh,ucLow,ucLRC;


    Os__clr_display11(255);
    Os__GB2312_display11(3,2,(PUCHAR)"Send Data...");
    emitSignal();


	Os__com_flush2();
//	Os__xdelay(50);
//	Os__xdelay(5);
	if(MSG_ComSendByte(CHAR_STX))
		return QERROR_SENDDATA;
	
	ucLRC = 0x00;
	ucLow = uiLen&0xFF;
	ucHigh = (uiLen>>8)&0xFF;

	ucLRC ^= ucHigh;
	if(MSG_ComSendByte(ucHigh))
		return QERROR_SENDDATA;
	

	ucLRC^=ucLow;
	if(MSG_ComSendByte(ucLow))
		return QERROR_SENDDATA;
	

	for(uiI=0;uiI<uiLen;uiI++)
	{
		ucLRC^=*(pucBuff+uiI);
		if(MSG_ComSendByte(*(pucBuff+uiI)))
			return QERROR_SENDDATA;
		
	}

	ucLRC ^= 0x03;
	
	if(MSG_ComSendByte(CHAR_ETX))
		return QERROR_SENDDATA;

	if(MSG_ComSendByte(ucLRC))
		return QERROR_SENDDATA;
	
	
	return QERROR_SUCCESS;
}
UCHAR MSG_ComSendByte(UCHAR	ucSendChar)
{
	if(QConstParam.ucCommID ==0)
		Os__txcar(ucSendChar);
	else
		Os__txcar3(ucSendChar);
//	Os__xdelay(1);
	return QERROR_SUCCESS;
	
}

UCHAR MSG_ComReceiveByte(PUCHAR pucByte,USHORT uiTimeout)
{
	USHORT uiResult;
#if 0
	uiResult = Os__rxcar(uiTimeout);
#else

	
	if(QConstParam.ucCommID ==0)
		uiResult = Os__rxcar(uiTimeout);
	else
		uiResult = Os__rxcar3(uiTimeout);
#endif

	switch	( uiResult / 256 )
	{
	case QERROR_SUCCESS :
		*pucByte = uiResult % 256;
//		Uart_Printf("%02X ",*pucByte);
		return(QERROR_SUCCESS);
	default:
//		Uart_Printf("\nConstParam.ucCommID %02x",QConstParam.ucCommID);
		return QERROR_RECEIVEDATA;
	
	}
}

UCHAR MSG_ComRecvData(PUCHAR pBuff, USHORT *puiLen)
{
	
	USHORT uiI,uiDataLen,uiMaxLen;
	UCHAR  ucTemp,ucLRC;

    Os__clr_display11(255);

    Os__GB2312_display11(3,2,(PUCHAR)"Receive Data...");
    emitSignal();
	uiMaxLen =*puiLen;
	for(uiI=0;uiI<500;uiI++)
	{
		if(!MSG_ComReceiveByte(pBuff,10))
			if(pBuff[0]==0x02) break;
	}
	if(uiI>=500) 
		return QERROR_RECEIVEDATA;
	
	ucLRC = 0;
	for(uiI=0;uiI<2;uiI++)
	{
		if(MSG_ComReceiveByte(pBuff+uiI+1,500)!=0)
			return QERROR_RECEIVEDATA;
		
		ucLRC^=*(pBuff+uiI+1);
	}
	
	uiDataLen=(unsigned int)(*(pBuff+1))*256 + *(pBuff+2);
	if(uiDataLen>uiMaxLen) return QERROR_OVERFLOW;
	
	for(uiI=0;uiI<uiDataLen;uiI++)
	{
		if(MSG_ComReceiveByte(pBuff+uiI,500)!=0)
			return QERROR_RECEIVEDATA;
		
		ucLRC^=*(pBuff+uiI);
	}
	
	
		
	if(MSG_ComReceiveByte(&ucTemp,500)!=0)
		return QERROR_RECEIVEDATA;
	
	if(ucTemp!=0x03)
		return QERROR_RECEIVEDATA;
	ucLRC ^= ucTemp;

	if(MSG_ComReceiveByte(&ucTemp,500)!=0)
		return QERROR_RECEIVEDATA;

	if(ucLRC!=ucTemp)
		return QERROR_RECEIVEDATA;

	*puiLen = uiDataLen;

	return QERROR_SUCCESS;
}


UCHAR MSG_ComClose()
{
	
//	OSUART_Close1();
#if 1
	if(QConstParam.ucCommID ==0)
		OSUART_Close1();
	else
		OSUART_Close2();
#endif
	return QERROR_SUCCESS;
}



UCHAR	MSG_FinaTransRequest(PUCHAR pRecvBuff,USHORT *puiLen)
{
	
	UCHAR ComPackSend[1024],ucResult;
	
	USHORT ComPackSendLen,VarReadLen,uiTagBuffLen,uiLen,uiBuffsize;

    Os__clr_display11(255);
    Os__GB2312_display11(2,0,"Fina Request");
    emitSignal();
	memset(ComPackSend,0,1024);
	ComPackSendLen=0;

	uiBuffsize = sizeof(ComPackSend);
	
	ComPackSend[0] = Msg_EMV_FinaRQ;
	ComPackSendLen ++;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x82",ComPackSend+ComPackSendLen,&uiLen);//AIP
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x07",ComPackSend+ComPackSendLen,&uiLen);//ACU
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //Q_ARQC
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x8E",ComPackSend+ComPackSendLen,&uiLen);  //CVM LIST
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1E",ComPackSend+ComPackSendLen,&uiLen);  //IFD
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x0D",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Default
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x0E",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Denial
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x0F",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Online
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x33",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Capabilities
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x35",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x37",ComPackSend+ComPackSendLen,&uiLen);  //Unpredictable Number
	ComPackSendLen += uiLen;
	
	
	if(((QTermConfig.ucTermType&0xF0)==0x10)||((QTermConfig.ucTermType&0xF0)==0x20))
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x01",ComPackSend+ComPackSendLen,&uiLen);  //Acquirer Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x02",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Authorised
	ComPackSendLen += uiLen;

	if(QTransReqInfo.enTransType ==QCASHBACK)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x03",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Other
		ComPackSendLen += uiLen;
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x25",ComPackSend+ComPackSendLen,&uiLen);  //Application Effective Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x24",ComPackSend+ComPackSendLen,&uiLen);  //Application Expiration Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5A",ComPackSend+ComPackSendLen,&uiLen);  //Primary Account Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x34",ComPackSend+ComPackSendLen,&uiLen);  //Application PAN Sequence Number
	ComPackSendLen += uiLen;
	/*
	if(EncryptPINLen>0)
		NP_Write(MV_EncPINData,EncryptPIN,EncryptPINLen);//max len 128 bytes
*/
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x28",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Country Code
	ComPackSendLen += uiLen;
	

	if((QTermConfig.ucTermType&0xF0)==0x20)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x15",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Category Code
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
		ComPackSendLen += uiLen;

		
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x39",ComPackSend+ComPackSendLen,&uiLen);  //POS Entry Mode
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1A",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Country Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x57",ComPackSend+ComPackSendLen,&uiLen);  //Track 2 Equivalent Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x81",ComPackSend+ComPackSendLen,&uiLen);  	//Transaction Amount
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x2A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Currency Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x21",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Time
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9C",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x4C",ComPackSend+ComPackSendLen,&uiLen); 	//TICC Dynamic Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x41",ComPackSend+ComPackSendLen,&uiLen); 	//Trans Sequensce Counter
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\xDF\x02",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;
#if 1
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x6E",ComPackSend+ComPackSendLen,&uiLen); 	//Form Factor Indicator
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x7C",ComPackSend+ComPackSendLen,&uiLen); 	//Customer Exclusive Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x66",ComPackSend+ComPackSendLen,&uiLen); 	//Terminal Transaction Qualifiers
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x6B",ComPackSend+ComPackSendLen,&uiLen); 	//Terminal Transaction Qualifiers
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x56",ComPackSend+ComPackSendLen,&uiLen); 	//Terminal Transaction Qualifiers
	ComPackSendLen += uiLen;
	
#endif	
	ucResult = MSG_OpenCom();

	if(!ucResult)
		ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
//	Uart_Printf("\nSend %02x",ucResult);
	if(!ucResult)
		ucResult =MSG_ComRecvData(pRecvBuff,puiLen);
	if(!ucResult)
		if(*pRecvBuff  !=Msg_EMV_AuthRP)
				ucResult = QERROR_RECEIVEDATA;
	MSG_ComClose();
	return ucResult;
}


UCHAR	MSG_AuthTransRequest(PUCHAR pRecvBuff,USHORT *puiLen)
{
	
	UCHAR ComPackSend[1024],ucResult;
	
    USHORT ComPackSendLen,VarReadLen,uiTagBuffLen,uiLen,uiBuffsize;


     Os__clr_display11(255);
     Os__GB2312_display11(2,0,"Auth Request");
    emitSignal();
	memset(ComPackSend,0,1024);
	ComPackSendLen=0;

	uiBuffsize = sizeof(ComPackSend);
	
	ComPackSend[0] = Msg_EMV_AuthRQ;
	ComPackSendLen ++;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x82",ComPackSend+ComPackSendLen,&uiLen);//AIP
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //Q_ARQC
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1E",ComPackSend+ComPackSendLen,&uiLen);  //IFD
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x33",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Capabilities
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x35",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x37",ComPackSend+ComPackSendLen,&uiLen);  //Unpredictable Number
	ComPackSendLen += uiLen;
	
	
	if(((QTermConfig.ucTermType&0xF0)==0x10)||((QTermConfig.ucTermType&0xF0)==0x20))
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x01",ComPackSend+ComPackSendLen,&uiLen);  //Acquirer Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x02",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Authorised
	ComPackSendLen += uiLen;

	if(QTransReqInfo.enTransType ==QCASHBACK)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x03",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Other
		ComPackSendLen += uiLen;
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x25",ComPackSend+ComPackSendLen,&uiLen);  //Application Effective Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x24",ComPackSend+ComPackSendLen,&uiLen);  //Application Expiration Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5A",ComPackSend+ComPackSendLen,&uiLen);  //Primary Account Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x34",ComPackSend+ComPackSendLen,&uiLen);  //Application PAN Sequence Number
	ComPackSendLen += uiLen;
	/*
	if(EncryptPINLen>0)
		NP_Write(MV_EncPINData,EncryptPIN,EncryptPINLen);//max len 128 bytes
*/
	

	if((QTermConfig.ucTermType&0xF0)==0x20)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x15",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Category Code
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
		ComPackSendLen += uiLen;

		
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x39",ComPackSend+ComPackSendLen,&uiLen);  //POS Entry Mode
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1A",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Country Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x57",ComPackSend+ComPackSendLen,&uiLen);  //Track 2 Equivalent Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x2A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Currency Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x21",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Time
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9C",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x4C",ComPackSend+ComPackSendLen,&uiLen); 	//ICC Dynamic Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x41",ComPackSend+ComPackSendLen,&uiLen); 	//Trans Sequensce Counter
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\xDF\x02",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;
#if 1
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x6E",ComPackSend+ComPackSendLen,&uiLen); 	//Form Factor Indicator
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x7C",ComPackSend+ComPackSendLen,&uiLen); 	//Customer Exclusive Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x66",ComPackSend+ComPackSendLen,&uiLen); 	//Terminal Transaction Qualifiers
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x6B",ComPackSend+ComPackSendLen,&uiLen); 	//Terminal Transaction Qualifiers
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x56",ComPackSend+ComPackSendLen,&uiLen); 	//Terminal Transaction Qualifiers
	ComPackSendLen += uiLen;
#endif
	ucResult = MSG_OpenCom();

	if(!ucResult)
		ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
	
	if(!ucResult)
		ucResult =MSG_ComRecvData(pRecvBuff,puiLen);
	if(!ucResult)
		if(*pRecvBuff  !=Msg_EMV_AuthRP)
				ucResult = QERROR_RECEIVEDATA;
	MSG_ComClose();

	return ucResult;
}


UCHAR	MSG_TransConfirm(void)
{
	
	UCHAR ComPackSend[1024],ucResult,ComPackRecv[300];
	
	USHORT ComPackSendLen,uiTagBuffLen,uiLen,uiBuffsize,ComPackRecvLen;



    Os__clr_display11(255);
    Os__GB2312_display11(2,0,"Confirm");
     emitSignal();
	memset(ComPackSend,0,1024);
	ComPackSendLen=0;

	uiBuffsize = sizeof(ComPackSend);
	
	ComPackSend[0] = Msg_EMV_Confirm;
	ComPackSendLen ++;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;

	if(QEMVTransInfo.enTransResult ==QOFFLINEAPPROVED)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //Q_ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	else
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //Q_ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
	ComPackSendLen += uiLen;

	if(QEMVTransInfo.ucScriptResultNum)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\xDF\x31",ComPackSend+ComPackSendLen,&uiLen);  //Issure Script Result
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x41",ComPackSend+ComPackSendLen,&uiLen); 	//Trans Sequensce Counter
	ComPackSendLen += uiLen;

	ComPackRecvLen =sizeof(ComPackRecv);
	ucResult = MSG_OpenCom();

	if(!ucResult)
		ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
	
	if(!ucResult)
		ucResult =MSG_ComRecvData(ComPackRecv,&ComPackRecvLen);
	if(!ucResult)
		if(ComPackRecv[0] !=Msg_EMV_Confirm)
				ucResult = QERROR_RECEIVEDATA;
	MSG_ComClose();
	return ucResult;
	
}


UCHAR	MSG_ReversalRequest(void)
{
	UCHAR ComPackSend[1024],ComPackRecv[300],ucResult;
	
	USHORT ComPackSendLen,ComPackRecvLen,VarReadLen,uiTagBuffLen,uiLen,uiBuffsize;



    Os__clr_display11(255);
    Os__GB2312_display11(2,0,"Reversal Request");
     emitSignal();
	memset(ComPackSend,0,1024);
	ComPackSendLen=0;

	uiBuffsize = sizeof(ComPackSend);
	
	ComPackSend[0] = Msg_EMV_Reversal;
	ComPackSendLen ++;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x82",ComPackSend+ComPackSendLen,&uiLen);//AIP
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QSECONDGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //Q_ARQC
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QSECONDGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QSECONDGACPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QSECONDGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1E",ComPackSend+ComPackSendLen,&uiLen);  //IFD
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x33",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Capabilities
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x35",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;
	
	if(((QTermConfig.ucTermType&0xF0)==0x10)||((QTermConfig.ucTermType&0xF0)==0x20))
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x01",ComPackSend+ComPackSendLen,&uiLen);  //Acquirer Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x24",ComPackSend+ComPackSendLen,&uiLen);  //Application Expiration Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5A",ComPackSend+ComPackSendLen,&uiLen);  //Primary Account Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x34",ComPackSend+ComPackSendLen,&uiLen);  //Application PAN Sequence Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x8A",ComPackSend+ComPackSendLen,&uiLen);  //Auth Response Code
	ComPackSendLen += uiLen;

	if((QTermConfig.ucTermType&0xF0)==0x20)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x15",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Category Code
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x39",ComPackSend+ComPackSendLen,&uiLen);  //POS Entry Mode
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x02",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Authorised
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1A",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Country Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x57",ComPackSend+ComPackSendLen,&uiLen);  //Track 2 Equivalent Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x2A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Currency Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x21",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Time
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9C",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Type
	ComPackSendLen += uiLen;

	if(QEMVTransInfo.ucScriptResultNum)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\xDF\x31",ComPackSend+ComPackSendLen,&uiLen);  //Issure Script Result
		ComPackSendLen += uiLen;
	}
	ComPackRecvLen =sizeof(ComPackRecv);
	ucResult = MSG_OpenCom();

	if(!ucResult)
		ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
	
	if(!ucResult)
		ucResult =MSG_ComRecvData(ComPackRecv,&ComPackRecvLen);
	
	if(!ucResult)
		if(ComPackRecv[0]  !=Msg_EMV_Reversal)
				ucResult = QERROR_RECEIVEDATA;
			
	MSG_ComClose();
	return ucResult;
}


UCHAR	MSG_AdviceRequest(void)
{
	UCHAR ComPackSend[1024],ComPackRecv[300],ucResult;
	
	USHORT ComPackSendLen,ComPackRecvLen,VarReadLen,uiTagBuffLen,uiLen,uiBuffsize;



    Os__clr_display11(255);
    Os__GB2312_display11(2,0,"Online Advice");
   emitSignal();
	//Os__xdelay(100);
	memset(ComPackSend,0,1024);
	ComPackSendLen=0;

	uiBuffsize = sizeof(ComPackSend);
	
	ComPackSend[0] = Msg_EMV_OnlineAdvice;
	ComPackSendLen ++;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x82",ComPackSend+ComPackSendLen,&uiLen);//AIP
	ComPackSendLen += uiLen;
	

	if(QEMVTransInfo.enTransResult ==QOFFLINEDECLINED)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QFIRSTGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //Q_ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QFIRSTGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QFIRSTGACPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QFIRSTGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	else
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QSECONDGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //Q_ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QSECONDGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QSECONDGACPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QSECONDGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1E",ComPackSend+ComPackSendLen,&uiLen);  //IFD
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x33",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Capabilities
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x35",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;

	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x37",ComPackSend+ComPackSendLen,&uiLen);  //Unpredictable Number
	ComPackSendLen += uiLen;
	
	
	if(((QTermConfig.ucTermType&0xF0)==0x10)||((QTermConfig.ucTermType&0xF0)==0x20))
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x01",ComPackSend+ComPackSendLen,&uiLen);  //Acquirer Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x02",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Authorised
	ComPackSendLen += uiLen;

	if(QTransReqInfo.enTransType ==QCASHBACK)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x03",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Other
		ComPackSendLen += uiLen;
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x25",ComPackSend+ComPackSendLen,&uiLen);  //Application Effective Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x24",ComPackSend+ComPackSendLen,&uiLen);  //Application Expiration Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5A",ComPackSend+ComPackSendLen,&uiLen);  //Primary Account Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x34",ComPackSend+ComPackSendLen,&uiLen);  //Application PAN Sequence Number
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x8A",ComPackSend+ComPackSendLen,&uiLen);  //Auth Response Code
	ComPackSendLen += uiLen;

	if((QTermConfig.ucTermType&0xF0)==0x20)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x15",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Category Code
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x39",ComPackSend+ComPackSendLen,&uiLen);  //POS Entry Mode
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1A",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Country Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x57",ComPackSend+ComPackSendLen,&uiLen);  //Track 2 Equivalent Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x2A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Currency Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x21",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Time
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9C",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Type
	ComPackSendLen += uiLen;

	if(QEMVTransInfo.ucScriptResultNum)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\xDF\x31",ComPackSend+ComPackSendLen,&uiLen);  //Issure Script Result
		ComPackSendLen += uiLen;
	}
	ComPackRecvLen =sizeof(ComPackRecv);
	ucResult = MSG_OpenCom();

	if(!ucResult)
		ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
	
	if(!ucResult)
		ucResult =MSG_ComRecvData(ComPackRecv,&ComPackRecvLen);

	if(!ucResult)
		if(ComPackRecv[0]  !=Msg_EMV_OnlineAdvice)
				ucResult = QERROR_RECEIVEDATA;
				
	ucResult =QERROR_SUCCESS;
	MSG_ComClose();

	return ucResult;
}


UCHAR	MSG_AddBatchRecord(UCHAR ucRecordID)
{

	UCHAR ComPackSend[1024],ComPackRecv[1024],ucResult;
	
	USHORT ComPackSendLen,ComPackRecvLen,VarReadLen,uiTagBuffLen,uiLen,uiBuffsize;
	int		iHandle;
	long	FleSize;
	
    Os__clr_display11(255);

	memset(ComPackSend,0,1024);
	ComPackSendLen=0;

	uiBuffsize = sizeof(ComPackSend);

	ComPackSendLen =2;
	ComPackSend[ComPackSendLen] = ucRecordID;
	ComPackSendLen ++;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x82",ComPackSend+ComPackSendLen,&uiLen);//AIP
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x07",ComPackSend+ComPackSendLen,&uiLen);//ACU
	ComPackSendLen += uiLen;


	if(QEMVTransInfo.enTransResult ==QOFFLINEAPPROVED ||QEMVTransInfo.enTransResult ==QOFFLINEDECLINED)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //Q_ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	else
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //Q_ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x8E",ComPackSend+ComPackSendLen,&uiLen);  //CVM LIST
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1E",ComPackSend+ComPackSendLen,&uiLen);  //IFD
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x0D",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Default
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x0E",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Denial
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x0F",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Online
	ComPackSendLen += uiLen;
	
//	uiLen = uiBuffsize - ComPackSendLen;
//	QDATA_TagPackProcess(QALLPHASETAG,"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
//	ComPackSendLen += uiLen;

	if(QEMVTransInfo.ucScriptResultNum)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\xDF\x31",ComPackSend+ComPackSendLen,&uiLen);  //Issure Script Result
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x33",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Capabilities
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x35",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;
/*	
	if(QEMVTransInfo.enTransResult == QOFFLINEAPPROVED)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //Q_TC or Q_AAC
		ComPackSendLen += uiLen;
	}
	else 
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QSECONDGACPHASETAG,"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //Q_TC or Q_AAC
		ComPackSendLen += uiLen;
	}
*/
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x37",ComPackSend+ComPackSendLen,&uiLen);  //Unpredictable Number
	ComPackSendLen += uiLen;
	
	
	if(((QTermConfig.ucTermType&0xF0)==0x10)||((QTermConfig.ucTermType&0xF0)==0x20))
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x01",ComPackSend+ComPackSendLen,&uiLen);  //Acquirer Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x02",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Authorised
	ComPackSendLen += uiLen;

	if(QTransReqInfo.enTransType ==QCASHBACK)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x03",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Other
		ComPackSendLen += uiLen;
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x25",ComPackSend+ComPackSendLen,&uiLen);  //Application Effective Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x24",ComPackSend+ComPackSendLen,&uiLen);  //Application Expiration Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5A",ComPackSend+ComPackSendLen,&uiLen);  //Primary Account Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x34",ComPackSend+ComPackSendLen,&uiLen);  //Application PAN Sequence Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x28",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Country Code
	ComPackSendLen += uiLen;
	

	if((QTermConfig.ucTermType&0xF0)==0x20)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x15",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Category Code
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1A",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Country Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x57",ComPackSend+ComPackSendLen,&uiLen);  //Track 2 Equivalent Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x81",ComPackSend+ComPackSendLen,&uiLen);  	//Transaction Amount
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x5F\x2A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Currency Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x21",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Time
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9C",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x4C",ComPackSend+ComPackSendLen,&uiLen); 	//TICC Dynamic Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x8A",ComPackSend+ComPackSendLen,&uiLen);  //Auth Response Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x89",ComPackSend+ComPackSendLen,&uiLen);  //Auth  Code
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x41",ComPackSend+ComPackSendLen,&uiLen); 	//Trans Sequensce Counter
	ComPackSendLen += uiLen;
	
	CONV_LongHex(ComPackSend,sizeof(USHORT),&ComPackSendLen);

	iHandle = OSAPP_OpenFile(BATCHRECORD,O_CREATE|O_WRITE);
	if(iHandle>=0)
	{
			

		FleSize =OSAPP_FileSize(BATCHRECORD);
		ucResult =OSAPP_FileSeek(iHandle,FleSize,SEEK_SET);
		
		if(!ucResult)
			if(ComPackSendLen != OSAPP_FileWrite(iHandle,ComPackSend,ComPackSendLen))
				ucResult = QERROR_SAVEFILE;
		OSAPP_FileClose(iHandle);
	}
	

	return ucResult;
	
}

UCHAR	MSG_Reconciliation(void)
{
	
	UCHAR ComPackSend[1024],ComPackRecv[300],ucResult;
	
	USHORT ComPackSendLen,ComPackRecvLen,VarReadLen,uiTagBuffLen,uiLen,uiBuffsize;
	UCHAR	aucTotalAmount[6],aucBatchNumber[6];

	ComPackRecvLen = sizeof(ComPackRecv);
	
	uiBuffsize = sizeof(ComPackSend);
	memset(ComPackSend,0,1024);
	ComPackSendLen=0;
	ComPackSend[ComPackSendLen] = Msg_EMV_Reconci;
	ComPackSendLen ++;
	
	QTAG_INIT((QDATAPROPERTY *)QTermDataProperty,TRUE);
	QTAG_SetTagValue((PUCHAR)"\x9F\x1C",QConstParam.aucTerminalID,QTERMINALIDLEN);
	QTAG_SetTagValue((PUCHAR)"\x9F\x16",QConstParam.aucMerchantID,QMERCHANTIDLEN);
	if(((QConfig.ucTermType&0xF0)==0x10)||((QConfig.ucTermType&0xF0)==0x20))
		QTAG_SetTagValue((PUCHAR)"\x9F\x35",&QConfig.ucTermType,1);
	QTAG_SetTagValue((PUCHAR)"\x5F\x2A",QConfig.aucTransCurrencyCode,QTRANSCURRENCYCODELEN);
	CONV_LongBcd(aucTotalAmount,sizeof(aucTotalAmount),&QConstParam.uiTotalAmount);
	QTAG_SetTagValue((PUCHAR)"\x9F\x02",aucTotalAmount,sizeof(aucTotalAmount));
	CONV_LongBcd(aucBatchNumber,sizeof(aucBatchNumber),&QConstParam.uiBatchNumber);
	QTAG_SetTagValue((PUCHAR)"\xDF\x01",aucBatchNumber,sizeof(aucBatchNumber));

	VarReadLen =uiBuffsize -ComPackSendLen;
	ucResult = QTAG_Pack(&ComPackSend[ComPackSendLen],&VarReadLen);
	if(!ucResult)
	{
		ComPackSendLen += VarReadLen;
//		CONV_LongHex(ComPackSend,sizeof(USHORT),&ComPackSendLen);
		ComPackRecvLen =sizeof(ComPackRecv);
		ucResult = MSG_OpenCom();
		
		if(!ucResult)
			ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
//		Uart_Printf("\nSend %02x",ucResult);
		if(!ucResult)
			ucResult =MSG_ComRecvData(ComPackRecv,&ComPackRecvLen);
		if(!ucResult)
			if(ComPackRecv[0] !=Msg_EMV_Reconci)
				ucResult = QERROR_RECEIVEDATA;
		MSG_ComClose();

		if(!ucResult)
			ucResult = MSG_BatchUp();
	}
	else ucResult = QERROR_DATA;
	if(!ucResult)
	{
		UTIL_DeleteAllData(TRANSLOG);
		QConstParam.uiBatchNumber++;
		if(QConstParam.uiBatchNumber>999999)
			QConstParam.uiBatchNumber =1;
		QConstParam.uiTotalAmount =0;
		QConstParam.uiTotalNums =0;
		ucResult =UTIL_WriteConstParamFile(&QConstParam);
	}
	/*
	if(!ucResult)
	{
        Os__clr_display11(255);
        Os__GB2312_display11(2,0,(PUCHAR)"");
		UTIL_GetKey(3);
	}
	else
	*/
	QERROR_DisplayMsg(ucResult);
	return ucResult;
}


UCHAR	MSG_UpdateParam(void)
{
	
	UCHAR ComPackSend[1024],ComPackRecv[1024],ucResult;
	
	USHORT ComPackSendLen,ComPackRecvLen,VarReadLen,uiTagBuffLen,uiLen,uiBuffsize;
	

    Os__clr_display11(255);

	memset(ComPackSend,0,1024);
	ComPackSendLen=0;
	ComPackRecvLen =sizeof(ComPackRecv);
	uiBuffsize = sizeof(ComPackSend);

	
	ComPackSend[0] = Msg_EMV_UpdatePara;
	ComPackSendLen ++;

	ucResult = MSG_OpenCom();

	if(!ucResult)
		ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
	
	if(!ucResult)
		ucResult =MSG_ComRecvData(ComPackRecv,&ComPackRecvLen);

	if(ComPackRecv[0]!=Msg_EMV_UpdatePara)
		ucResult =QERROR_PARAMUPDATE;
	if(!ucResult)
		ucResult = MSG_ProcessParamUpdate(&ComPackRecv[1],ComPackRecvLen-1);

	if(!ucResult)
		ucResult = UTIL_WriteEMVConfigFile(&QConfig);

	MSG_ComClose();
	if(!ucResult)
	{

        Os__clr_display11(255);
        Os__GB2312_display11(2,0,"Param Update OK!");
        emitSignal();
		UTIL_GetKey(3);
	}
	else 
		QERROR_DisplayMsg(ucResult);
		
	return ucResult;
	
	
}


UCHAR	MSG_ProcessParamUpdate(UCHAR	*pucParamData,USHORT uiParamLen)
{
	USHORT	uiI,uiLen,uiDataLen;
	UCHAR	ucResult,ucMsg1,ucMsg2;
	BOOL	bBaseParam,bCAPKParam,bAIDParam,bExceptFileParam,bIPKRevokeParam;
	
	ucResult =QERROR_SUCCESS;
	bBaseParam = FALSE;
	bCAPKParam = FALSE;
	bAIDParam = FALSE;
	bExceptFileParam = FALSE;
	bIPKRevokeParam = FALSE;
	for(uiI =0; !ucResult && uiI <uiParamLen;)
	{
		if(uiI+2<uiParamLen)
		{
			ucMsg1 =*pucParamData;
			ucMsg2 =*(pucParamData+1);
			pucParamData +=2;
			switch(ucMsg1)
			{
			case BASEPARAM:
				uiDataLen = (USHORT)CONV_HexLong(pucParamData,2);
				if(uiI +4+uiDataLen <=uiParamLen)
				{
					ucResult = MSG_ProcessBaseParmUpdate(pucParamData+2,uiDataLen);
					if(!ucResult) bBaseParam = TRUE;
				}
				else
					ucResult = QERROR_PARAMUPDATE;
				break;
			case CAPKPARAM:
				uiDataLen = (USHORT)CONV_HexLong(pucParamData,2);
				if(uiI +4+uiDataLen <=uiParamLen)
				{
					ucResult = MSG_ProcessCAPKUpdate(ucMsg2,pucParamData+2,uiDataLen);
					if(!ucResult) bCAPKParam = TRUE;
				}
				else
					ucResult = QERROR_PARAMUPDATE;
				break;
			case AIDPARAM:
				uiDataLen = (USHORT)CONV_HexLong(pucParamData,2);
				if(uiI +4+uiDataLen <=uiParamLen)
				{
					ucResult = MSG_ProcessAIDUpdate(ucMsg2,pucParamData+2,uiDataLen);
					if(!ucResult) bAIDParam = TRUE;
				}
				else
					ucResult = QERROR_PARAMUPDATE;
				break;
			case EXCEPTFILEPARAM:
				uiDataLen = (USHORT)CONV_HexLong(pucParamData,2);
				if(uiI +4+uiDataLen <=uiParamLen)
				{
					ucResult = MSG_ProcessExceptFileUpdate(ucMsg2,pucParamData+2,uiDataLen);
					if(!ucResult) bExceptFileParam = TRUE;
				}
				else
					ucResult = QERROR_PARAMUPDATE;
				break;
				break;
			case IPKREVOKEPARAM:
				uiDataLen = (USHORT)CONV_HexLong(pucParamData,2);
				if(uiI +4+uiDataLen <=uiParamLen)
				{
					ucResult = MSG_ProcessIPKRevokeUpdate(ucMsg2,pucParamData+2,uiDataLen);
					if(!ucResult) bIPKRevokeParam = TRUE;
				}
				else
					ucResult = QERROR_PARAMUPDATE;
				break;
			default:
				ucResult =QERROR_PARAMUPDATE;
				break;
			}
			uiI += uiDataLen+4;
			pucParamData += uiDataLen+2;
			
		}
		else ucResult =QERROR_PARAMUPDATE;
		
	}
	if(!ucResult)
	{
		if(bBaseParam)
			ucResult = UTIL_WriteEMVConfigFile(&QConfig);
		if(!ucResult&&bCAPKParam)
			ucResult = UTIL_SaveCAPKFile();
		if(!ucResult&&bAIDParam)
			ucResult = UTIL_SaveAIDFile();
		if(!ucResult&&bExceptFileParam)
			ucResult = UTIL_SaveExceptFile();
		if(!ucResult&&bIPKRevokeParam)
			ucResult = UTIL_SaveIPKRevokeFile();
	}
//	Uart_Printf("\n Update %02x",ucResult);
	return ucResult;

	
}


UCHAR	MSG_ProcessBaseParmUpdate(PUCHAR	pBaseParam,USHORT uiParamLen)
{
	USHORT	uiI,uiLen;
	UCHAR	ucResult,ucIndex,ucEntrys,ucLen;
	
	ucResult = QERROR_SUCCESS;
	uiI = 0;
	ucEntrys =sizeof(ParamIndicate)/sizeof(PARAMINDICATE);
	
	while(!ucResult && uiI <uiParamLen)
	{
		
		ucIndex =pBaseParam[uiI];
		ucLen = QTAG_GetTagLenAttr(pBaseParam+uiI+1,&uiLen);
		if(ucLen+uiLen+1+uiI <=uiParamLen)
		{
			if(ucIndex <= ucEntrys)
			{
				if(ParamIndicate[ucIndex-1].uiParamLen==999)
				{
					*ParamIndicate[ucIndex-1].pParamLenIndicate =uiLen;
					memcpy(ParamIndicate[ucIndex-1].pParamIndicate,&pBaseParam[uiI+ucLen+1],uiLen);
				}
				
				else if(uiLen == ParamIndicate[ucIndex-1].uiParamLen)
				{
					if(ucIndex != 0x02 && ucIndex !=0x04)
						memcpy(ParamIndicate[ucIndex-1].pParamIndicate,&pBaseParam[uiI+ucLen+1],uiLen);
					else
						*(UINT*)ParamIndicate[ucIndex-1].pParamIndicate=CONV_HexLong(&pBaseParam[uiI+ucLen+1],uiLen);
				}
				else ucResult = QERROR_PARAMUPDATE;
			}
			else
				ucResult = QERROR_PARAMUPDATE;
			if(!ucResult) 
				uiI += ucLen+uiLen+1;
		}
		else ucResult = QERROR_PARAMUPDATE;
		
	}

	return ucResult;
	
}

UCHAR	MSG_CAPKAdd(PQCAPK	pCapk)
{
	UCHAR	ucI,ucResult;
	PQCAPK	pTermCAPK;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQTermCAPKNum;ucI++)
	{
		pTermCAPK =&QTermCAPK[ucI];
		if(!memcmp(pTermCAPK->aucRID,pCapk->aucRID,QRIDDATALEN) 
			&&(pTermCAPK->ucCAPKI==pCapk->ucCAPKI))
			break;
	}
	if(ucI <ucQTermCAPKNum)
		memcpy(pTermCAPK,pCapk,sizeof(QCAPK));
	else if(ucQTermCAPKNum<QMAXTERMCAPKNUMS)
	{
		memcpy(&QTermCAPK[ucQTermCAPKNum],pCapk,sizeof(QCAPK));
		ucQTermCAPKNum ++;
	}
	else ucResult = QERROR_OVERFLOW;

	return ucResult;
	
		
}


UCHAR	MSG_CAPKModify(PQCAPK 	pCapk)
{
	UCHAR	ucI,ucResult;
	PQCAPK	pTermCAPK;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQTermCAPKNum;ucI++)
	{
		pTermCAPK =&QTermCAPK[ucI];
		if(!memcmp(pTermCAPK->aucRID,pCapk->aucRID,QRIDDATALEN) 
			&&(pTermCAPK->ucCAPKI==pCapk->ucCAPKI))
			break;
	}
	
	if(ucI <ucQTermCAPKNum)
		memcpy(pTermCAPK,pCapk,sizeof(QCAPK));
	

	return ucResult;
	
		
}

UCHAR	MSG_CAPKDelete(PQCAPK		pCapk)
{
	UCHAR	ucI,ucJ,ucResult;
	PQCAPK	pTermCAPK;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQTermCAPKNum;ucI++)
	{
		pTermCAPK =&QTermCAPK[ucI];
		if(!memcmp(pTermCAPK->aucRID,pCapk->aucRID,QRIDDATALEN) 
			&&(pTermCAPK->ucCAPKI==pCapk->ucCAPKI))
			break;
	}
	
	if(ucI <ucQTermCAPKNum&&ucQTermCAPKNum)
	{
		for(ucJ =ucI; ucJ <ucQTermCAPKNum-1;ucJ++)
			memcpy(&QTermCAPK[ucJ],&QTermCAPK[ucJ+1],sizeof(QCAPK));
		ucQTermCAPKNum--;
	}
	

	return ucResult;
	
		
}

UCHAR	MSG_ProcessCAPKUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen)
{
	USHORT	uiI,uiLen;
	UCHAR	ucResult,ucIndex,ucLen,auchashBuff[QHASHDATALEN];
	UCHAR	aucBuff[1024];
	PQCAPK	pCapk;
	
	ucResult = QERROR_SUCCESS;
	
	if(uiParamLen !=sizeof(QCAPK))
		ucResult = QERROR_PARAMUPDATE;

	pCapk =(PQCAPK)pBaseParam;
	uiLen =0;
//	aucBuff[uiLen++] = pCapk->ucModulLen;
	memcpy(&aucBuff[uiLen],pCapk->aucRID,QRIDDATALEN);
	uiLen += QRIDDATALEN;
	aucBuff[uiLen++] = pCapk->ucIndex;
//	memcpy(aucBuff+uiLen,&pCapk->ucCAPKI,4);
//	uiLen +=4;
	memcpy(aucBuff+uiLen,&pCapk->aucModul,pCapk->ucModulLen);
	uiLen +=pCapk->ucModulLen;
	memcpy(aucBuff+uiLen,&pCapk->aucExponent,pCapk->ucExponentLen);
	uiLen +=pCapk->ucExponentLen;
	
	
	SHA1_Compute(aucBuff,uiLen,auchashBuff);
	if(memcmp(auchashBuff,pCapk->aucCheckSum,QHASHDATALEN))
		ucResult = QERROR_HASHVALUE;
	if(!ucResult)
	{
		switch(ucAction)
		{
		case ADDACTION:
			ucResult = MSG_CAPKAdd((PQCAPK)pBaseParam);
			break;
		case DELETEACTION:
			ucResult = MSG_CAPKDelete((PQCAPK)pBaseParam);
			break;
		case MODIFYACTION:
			ucResult = MSG_CAPKModify((PQCAPK)pBaseParam);
			break;
		}
	}
	return ucResult;
}

UCHAR	MSG_IPKRevokeAdd(PQIPKREVOKE	pIPKRevoke)
{
	UCHAR	ucI,ucResult;
	PQIPKREVOKE	pTermIPKRevoke;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQIPKRevokeNum;ucI++)
	{
		pTermIPKRevoke =&QIPKRevoke[ucI];
		if(!memcmp(pTermIPKRevoke->aucRID,pIPKRevoke->aucRID,QRIDDATALEN) 
			&&(pTermIPKRevoke->ucCAPKI==pIPKRevoke->ucCAPKI))
			break;
	}
	if(ucI <ucQIPKRevokeNum)
		memcpy(pTermIPKRevoke,pIPKRevoke,sizeof(QIPKREVOKE));
	else if(ucQIPKRevokeNum<QMAXIPKREVOKENUMS)
	{
		memcpy(&QIPKRevoke[ucQIPKRevokeNum],pIPKRevoke,sizeof(QIPKREVOKE));
		ucQIPKRevokeNum ++;
	}
	else ucResult = QERROR_PARAMUPDATE;

	return ucResult;
	
		
}


UCHAR	MSG_IPKRevokeModify(PQIPKREVOKE 	pIPKRevoke)
{
	UCHAR	ucI,ucResult;
	PQIPKREVOKE	pTermIPKRevoke;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQIPKRevokeNum;ucI++)
	{
		pIPKRevoke =&QIPKRevoke[ucI];
		if(!memcmp(pTermIPKRevoke->aucRID,pIPKRevoke->aucRID,QRIDDATALEN) 
			&&(pTermIPKRevoke->ucCAPKI==pIPKRevoke->ucCAPKI))
			break;
	}
	
	if(ucI <ucQIPKRevokeNum)
		memcpy(pIPKRevoke,pIPKRevoke,sizeof(QIPKRevoke));
	

	return ucResult;
	
		
}

UCHAR	MSG_IPKRevokeDelete(PQIPKREVOKE		pIPKRevoke)
{
	UCHAR	ucI,ucJ,ucResult;
	PQIPKREVOKE	pTermIPKRevoke;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQIPKRevokeNum;ucI++)
	{
		pTermIPKRevoke =&QIPKRevoke[ucI];
		if(!memcmp(pTermIPKRevoke->aucRID,pIPKRevoke->aucRID,QRIDDATALEN) 
			&&(pTermIPKRevoke->ucCAPKI==pIPKRevoke->ucCAPKI))
			break;
	}
	
	if(ucI <ucQIPKRevokeNum&&ucQIPKRevokeNum)
	{
		for(ucJ =ucI; ucJ <ucQIPKRevokeNum-1;ucJ++)
			memcpy(&QIPKRevoke[ucJ],&QIPKRevoke[ucJ+1],sizeof(QIPKRevoke));
		ucQIPKRevokeNum--;
	}
	

	return ucResult;
	
		
}

UCHAR	MSG_ProcessIPKRevokeUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen)
{
	USHORT	uiI;
	UCHAR	ucResult,ucIndex,ucLen;
	
	ucResult = QERROR_SUCCESS;
	
	if(uiParamLen !=sizeof(QIPKREVOKE))
		ucResult = QERROR_PARAMUPDATE;
	if(!ucResult)
	{
		switch(ucAction)
		{
		case ADDACTION:
			ucResult = MSG_IPKRevokeAdd((PQIPKREVOKE)pBaseParam);
			break;
		case DELETEACTION:
			ucResult = MSG_IPKRevokeDelete((PQIPKREVOKE)pBaseParam);
			break;
		case MODIFYACTION:
			ucResult = MSG_IPKRevokeModify((PQIPKREVOKE)pBaseParam);
			break;
		}
	}
	return ucResult;
}

UCHAR	MSG_BatchUp(void)
{
	int		iHandle;
	UCHAR	aucLenBuff[2],ComPackSend[1024],ComPackRecv[300],ucResult;
	USHORT	uiRecordLen,ComPackSendLen,uiReadLen;
	
	ucResult = MSG_OpenCom();
	if(ucResult) return ucResult;
	
	iHandle = OSAPP_OpenFile(BATCHRECORD,O_WRITE);
	if(iHandle>=0)
	{
		OSAPP_FileSeek(iHandle,0,SEEK_SET);

		do
		{
			uiReadLen = OSAPP_FileRead(iHandle,aucLenBuff,sizeof(USHORT));
			if(uiReadLen ==sizeof(USHORT))
			{
				uiRecordLen = (USHORT)CONV_HexLong(aucLenBuff,sizeof(USHORT));
				if(uiRecordLen)
				{
					uiReadLen = OSAPP_FileRead(iHandle,ComPackSend,uiRecordLen-2);
					if(uiRecordLen-2 ==uiReadLen)
					{
						ucResult = MSG_ComSendData(ComPackSend,uiReadLen);
						if(!ucResult)
						{
							uiRecordLen =sizeof(ComPackRecv);
							ucResult =MSG_ComRecvData(ComPackRecv,&uiRecordLen);
						}
					}
					else 	ucResult= QERROR_READFILE;
				}
				else 	ucResult= QERROR_READFILE;
			}
			else 	break;
			
		}while(!ucResult);
		OSAPP_FileTrunc(iHandle,0);
		OSAPP_FileClose(iHandle);
	}
	MSG_ComClose();
	
	

	return ucResult;
	
}


UCHAR	MSG_AIDAdd(QTERMSUPPORTAPP* pAID)
{
	UCHAR	ucI,ucResult;
	QTERMSUPPORTAPP*	pTermAID;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQTermAIDNum;ucI++)
	{
		pTermAID =&QTermAID[ucI];
		if((pTermAID->ucAIDLen==pAID->ucAIDLen)
			&&!memcmp(pTermAID->aucAID,pAID->aucAID,pAID->ucAIDLen))
			break;
	}
	if(ucI <ucQTermAIDNum)
		memcpy(pTermAID,pAID,sizeof(QTERMSUPPORTAPP));
	else if(ucQTermAIDNum<QMAXTERMAIDNUMS)
	{
		memcpy(&QTermAID[ucQTermAIDNum],pAID,sizeof(QTERMSUPPORTAPP));
		ucQTermAIDNum ++;
	}
	else ucResult = QERROR_PARAMUPDATE;

	return ucResult;
	
		
}


UCHAR	MSG_AIDModify(QTERMSUPPORTAPP* pAID)
{
	UCHAR	ucI,ucResult;
	QTERMSUPPORTAPP*	pTermAID;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQTermAIDNum;ucI++)
	{
		pTermAID =&QTermAID[ucI];
		if((pTermAID->ucAIDLen==pAID->ucAIDLen)
			&&!memcmp(pTermAID->aucAID,pAID->aucAID,pAID->ucAIDLen))
			break;
	}
	
	if(ucI <ucQTermAIDNum)
		memcpy(pTermAID,pAID,sizeof(QTERMSUPPORTAPP));
	

	return ucResult;
	
		
}

UCHAR	MSG_AIDDelete(QTERMSUPPORTAPP* pAID)
{
	UCHAR	ucI,ucJ,ucResult;
	QTERMSUPPORTAPP*	pTermAID;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQTermAIDNum;ucI++)
	{
		pTermAID =&QTermAID[ucI];
		if((pTermAID->ucAIDLen==pAID->ucAIDLen)
			&&!memcmp(pTermAID->aucAID,pAID->aucAID,pAID->ucAIDLen))
			break;
	}
	
	if(ucI <ucQTermAIDNum&&ucQTermAIDNum)
	{
		for(ucJ =ucI; ucJ <ucQTermAIDNum-1;ucJ++)
			memcpy(&QTermAID[ucJ],&QTermAID[ucJ+1],sizeof(QTERMSUPPORTAPP));
		ucQTermAIDNum--;
	}
	

	return ucResult;
	
		
}

UCHAR	MSG_ProcessAIDUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen)
{
	USHORT	uiI;
	UCHAR	ucResult,ucIndex,ucLen;
	
	ucResult = QERROR_SUCCESS;
	
	if(uiParamLen !=sizeof(QTERMSUPPORTAPP))
		ucResult = QERROR_PARAMUPDATE;
	if(!ucResult)
	{
		switch(ucAction)
		{
		case ADDACTION:
			ucResult = MSG_AIDAdd((QTERMSUPPORTAPP*)pBaseParam);
			break;
		case DELETEACTION:
			ucResult = MSG_AIDDelete((QTERMSUPPORTAPP*)pBaseParam);
			break;
		case MODIFYACTION:
			ucResult = MSG_AIDModify((QTERMSUPPORTAPP*)pBaseParam);
			break;
		}
	}
	return ucResult;
}



UCHAR	MSG_ExceptFileAdd(PQEXCEPTPAN	pFile)
{
	UCHAR	ucI,ucResult;
	PQEXCEPTPAN	pTermFile;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQExceptFileNum;ucI++)
	{
		pTermFile =&QExceptFile[ucI];
		if((pTermFile->ucPANSeq ==pFile->ucPANSeq)
			&&!memcmp(pTermFile->aucPAN,pFile->aucPAN,QMAXPANDATALEN))
			break;
	}
	if(ucI <ucQExceptFileNum)
		memcpy(pTermFile,pFile,sizeof(QExceptFile));
	else if(ucQExceptFileNum<QMAXEXCEPTFILENUMS)
	{
		memcpy(&QExceptFile[ucQExceptFileNum],pFile,sizeof(QExceptFile));
		ucQExceptFileNum ++;
	}
	else ucResult = QERROR_PARAMUPDATE;

	return ucResult;
	
		
}


UCHAR	MSG_ExceptFileModify(PQEXCEPTPAN	pFile)
{
	UCHAR	ucI,ucResult;
	PQEXCEPTPAN	pTermFile;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQExceptFileNum;ucI++)
	{
		pTermFile =&QExceptFile[ucI];
		if((pTermFile->ucPANSeq ==pFile->ucPANSeq)
			&&!memcmp(pTermFile->aucPAN,pFile->aucPAN,QMAXPANDATALEN))
			break;
	}
	
	if(ucI <ucQExceptFileNum)
		memcpy(pTermFile,pFile,sizeof(QEXCEPTPAN));
	

	return ucResult;
	
		
}

UCHAR	MSG_ExceptFileDelete(PQEXCEPTPAN pFile)
{
	UCHAR	ucI,ucJ,ucResult;
	PQEXCEPTPAN	pTermFile;

	ucResult = QERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucQExceptFileNum;ucI++)
	{
		pTermFile =&QExceptFile[ucI];
		if((pTermFile->ucPANSeq ==pFile->ucPANSeq)
			&&!memcmp(pTermFile->aucPAN,pFile->aucPAN,QMAXPANDATALEN))
			break;
	}
	
	if(ucI <ucQExceptFileNum&&ucQExceptFileNum)
	{
		for(ucJ =ucI; ucJ <ucQExceptFileNum-1;ucJ++)
			memcpy(&QExceptFile[ucJ],&QExceptFile[ucJ+1],sizeof(QEXCEPTPAN));
		ucQExceptFileNum--;
	}
	

	return ucResult;
	
		
}

UCHAR	MSG_ProcessExceptFileUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen)
{
	USHORT	uiI;
	UCHAR	ucResult,ucIndex,ucLen;
	
	ucResult = QERROR_SUCCESS;
	
	if(uiParamLen !=sizeof(QEXCEPTPAN))
		ucResult = QERROR_PARAMUPDATE;
	if(!ucResult)
	{
		switch(ucAction)
		{
		case ADDACTION:
			ucResult = MSG_ExceptFileAdd((PQEXCEPTPAN)pBaseParam);
			break;
		case DELETEACTION:
			ucResult = MSG_ExceptFileDelete((PQEXCEPTPAN)pBaseParam);
			break;
		case MODIFYACTION:
			ucResult = MSG_ExceptFileModify((PQEXCEPTPAN)pBaseParam);
			break;
		}
	}
	return ucResult;
}


UCHAR	MSG_ProcessIccFailBack(void)
{
	
	UCHAR ComPackSend[1024],ComPackRecv[300],ucResult;
	
	USHORT ComPackSendLen,ComPackRecvLen,VarReadLen,uiTagBuffLen,uiLen,uiBuffsize;
	UCHAR	aucTotalAmount[6],aucOtherAmount[6],aucBatchNumber[6],ucEntryMode,aucTracerNumber[6];
	UCHAR	aucTrack2Data[40],ucTrack2Len;
	UINT	uiTotalAmount;
	int		iHandle;
	long	FleSize;
	
	ucEntryMode = 0x92;
	
	ComPackRecvLen = sizeof(ComPackRecv);
	
	uiBuffsize = sizeof(ComPackSend);
	memset(ComPackSend,0,1024);
	ComPackSendLen=2;
	if(!QConstParam.bBatchCapture)
		ComPackSend[ComPackSendLen] = Msg_EMV_FinaRQ;
	else
		ComPackSend[ComPackSendLen] = Msg_EMV_AuthRQ;
	ComPackSendLen ++;
	
	QTAG_INIT((QDATAPROPERTY *)QTermDataProperty,TRUE);
	QTAG_SetTagValue((PUCHAR)"\x9F\x1C",QConstParam.aucTerminalID,QTERMINALIDLEN);
	QTAG_SetTagValue((PUCHAR)"\x9F\x16",QConstParam.aucMerchantID,QMERCHANTIDLEN);
	if(((QConfig.ucTermType&0xF0)==0x10)||((QConfig.ucTermType&0xF0)==0x20))
		QTAG_SetTagValue((PUCHAR)"\x9F\x35",&QConfig.ucTermType,1);
	QTAG_SetTagValue((PUCHAR)"\x9C",(PUCHAR)&QTransReqInfo.enTransType,1);
	QTAG_SetTagValue((PUCHAR)"\x5F\x2A",QConfig.aucTransCurrencyCode,QTRANSCURRENCYCODELEN);
	ucTrack2Len =strlen((char*)MagTrack2);
	CONV_AscBcd(aucTrack2Data,ucTrack2Len/2,MagTrack2,ucTrack2Len);
	QTAG_SetTagValue((PUCHAR)"\x57",aucTrack2Data,ucTrack2Len/2);	 //Track 2 Equivalent Data

	CONV_LongBcd(aucTotalAmount,sizeof(aucTotalAmount),&QTransReqInfo.uiAmount);
	QTAG_SetTagValue((PUCHAR)"\x9F\x02",aucTotalAmount,sizeof(aucTotalAmount));
	if(QTransReqInfo.enTransType ==QCASHBACK)
	{
		CONV_LongBcd(aucOtherAmount,sizeof(aucOtherAmount),&QTransReqInfo.uiOtherAmount);
		QTAG_SetTagValue((PUCHAR)"\x9F\x03",aucOtherAmount,sizeof(aucOtherAmount));
	}
	QTAG_SetTagValue((PUCHAR)"\x9F\x39",&ucEntryMode,1);
	CONV_LongBcd(aucBatchNumber,sizeof(aucBatchNumber),&QConstParam.uiBatchNumber);
	QTAG_SetTagValue((PUCHAR)"\xDF\x01",aucBatchNumber,sizeof(aucBatchNumber));

	CONV_LongHex(aucTracerNumber,sizeof(UINT),&QTransReqInfo.uiTraceNumber);							//Trans sequence Counter
	QTAG_SetTagValue((PUCHAR)"\x9F\x41",aucTracerNumber,sizeof(UINT));
	
	
	VarReadLen =uiBuffsize -ComPackSendLen;
	ucResult = QTAG_Pack(&ComPackSend[ComPackSendLen],&VarReadLen);
	if(!ucResult)
	{
		ComPackSendLen += VarReadLen;
//		if(!QConstParam.bBatchCapture)
//		{
			ucResult = MSG_OpenCom();
			if(!ucResult)
				ucResult = MSG_ComSendData(&ComPackSend[2],ComPackSendLen-2);
			if(!ucResult)
			{
				ComPackRecvLen =sizeof(ComPackRecv);
				ucResult =MSG_ComRecvData(ComPackRecv,&ComPackRecvLen);
			}
			/*
			if(!ucResult)
				if(ComPackRecv[0] !=Msg_EMV_Reconci)
					ucResult = QERROR_RECEIVEDATA;
			*/
			if(!ucResult)
				ucResult = QAPPCRYPT_OnlineRespDataProcess(NULL,&ComPackRecv[1],ComPackRecvLen-1);
			MSG_ComClose();
//		}
#if 0
		else
		{
			CONV_LongHex(ComPackSend,sizeof(USHORT),&ComPackSendLen);

			iHandle = OSAPP_OpenFile(BATCHRECORD,O_CREATE|O_WRITE);
			if(iHandle>=0)
			{
					

				FleSize =OSAPP_FileSize(BATCHRECORD);
				ucResult =OSAPP_FileSeek(iHandle,FleSize,SEEK_SET);
				
				if(!ucResult)
					if(ComPackSendLen != OSAPP_FileWrite(iHandle,ComPackSend,ComPackSendLen))
						ucResult = QERROR_SAVEFILE;
				OSAPP_FileClose(iHandle);
			}
		}
#endif
	}
	
	return ucResult;
	
}


UCHAR	MSG_TransResult(void)
{
	
	UCHAR ComPackSend[1024],ComPackRecv[300],ucResult;
	
	USHORT ComPackSendLen,ComPackRecvLen,VarReadLen,uiTagBuffLen,uiLen,uiBuffsize;
	UCHAR	aucTotalAmount[6],aucBatchNumber[6];

	ucResult = QERROR_SUCCESS;
	ComPackRecvLen = sizeof(ComPackRecv);
	
	uiBuffsize = sizeof(ComPackSend);
	memset(ComPackSend,0,1024);
	ComPackSendLen=0;
	ComPackSend[ComPackSendLen] = Msg_EMV_TransResult;
	ComPackSendLen ++;
	
	
	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\xDF\x32",ComPackSend+ComPackSendLen,&uiLen);  //TRANSACTION RESULT
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	QDATA_TagPackProcess(QALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;
	
	if(!ucResult)
	{
		ComPackRecvLen =sizeof(ComPackRecv);
		ucResult = MSG_OpenCom();
		
		if(!ucResult)
			ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
		if(!ucResult)
			ucResult =MSG_ComRecvData(ComPackRecv,&ComPackRecvLen);
		if(!ucResult)
			if(ComPackRecv[0] !=Msg_EMV_TransResult)
				ucResult = QERROR_RECEIVEDATA;
		MSG_ComClose();
	}
	else ucResult = QERROR_DATA;
	return ucResult;
}

