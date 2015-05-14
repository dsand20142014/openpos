#include	<test/Msg.h>
#include	<EMV41.h>
#include	<test/Global.h>
#include "test/Util.h"
#include "tools.h"

extern const DATAPROPERTY TermDataProperty[];
extern UCHAR 	MagTrack2[40];
const	PARAMINDICATE	ParamIndicate[]=
{
	{0x01,	APPVERNUMBERLEN,		NULL,	(PUCHAR)&EMVConfig.aucAppVerNumber},
	{0x02,	4,						NULL,	(PUCHAR)&EMVConfig.uiFloorLimit},
	{0x03,	1,						NULL,	(PUCHAR)&EMVConfig.ucTermType},	
	{0x04,	4,						NULL,	(PUCHAR)&EMVConfig.uiThreshold},	
	{0x05,	999,						(PUCHAR)&EMVConfig.ucDefaultDOLLen,(PUCHAR)&EMVConfig.aucDefaultDOL},	
	{0x06,	1,						NULL,	(PUCHAR)&EMVConfig.ucMaxTragetPercent},	
	{0x07,	TERMCOUNTRYCODELEN,		NULL,	(PUCHAR)&EMVConfig.aucTermCountryCode},	
	{0x08,	1,						NULL,	(PUCHAR)&EMVConfig.ucTragetPercent},
	{0x09,	TRANSCURRENCYCODELEN,	NULL,	(PUCHAR)&EMVConfig.aucTransCurrencyCode},	
	{0x0A,	1,						NULL,	(PUCHAR)&EMVConfig.ucTransCurrencyExp},	
	{0x0B,	TERMCAPABLEN,			NULL,	(PUCHAR)&EMVConfig.aucTermCapab},	
	{0x0C,	TERMADDCAPABLEN,		NULL,	(PUCHAR)&EMVConfig.aucTermAddCapab},	
	{0x0D,	999,						(PUCHAR)&EMVConfig.ucDefaultTDOLLen,(PUCHAR)&EMVConfig.aucDefaultTDOL},
	{0x0E,	5,						NULL,	(PUCHAR)&EMVConfig.TAC.aucDenial},
	{0x0F,	5,						NULL,	(PUCHAR)&EMVConfig.TAC.aucOnline},
	{0x10,	5,						NULL,	(PUCHAR)&EMVConfig.TAC.aucDefault}
};

UCHAR	MSG_OpenCom(void)
{
	UCHAR	ucResult;
     printf("ssssssssssss\n");
	COM_PARAM CommParam =
	{
		1,     /* Stop bit */
		9600, /* Speed */
		1,	   /* MODEM 0 RS232 1 HDLC 2 */
		8,     /* Data bit */
	   'N'     /* No parity */
	};
//	MSG_ComClose();
    printf("0000ssssssssssssuConstParam.ucCommID== %02x\n",ConstParam.ucCommID);

	if(ConstParam.ucCommID ==0)
		ucResult = Os__init_com(&CommParam);
    else
        ucResult = Os__init_com3(0x0303, 0x0c00, 0x0c);
    printf("ssssssssssssucResult == %02x\n",ucResult);

//		ucResult = Os__init_com3(0x0303, 0x1400, 0x14);
    if(ucResult) ucResult = EMVERROR_OPENCOM;
    printf("end ssssssssssssucResult == %02x\n",ucResult);

	return ucResult;
	
}

UCHAR MSG_ComSendData(PUCHAR pucBuff, USHORT uiLen)
{
	USHORT 	uiI;
	UCHAR	ucHigh,ucLow,ucLRC;

    Os__clr_display11(255);
	if(ConstParam.bPBOC20==FALSE)
        Os__GB2312_display11(2,2,(PUCHAR)"Send Data...");
    else
        Os__GB2312_display11(2,2,(PUCHAR)"发送数据...");

    emitSignal();

	
	Os__com_flush2();
//	Os__xdelay(50);
//	Os__xdelay(5);
	if(MSG_ComSendByte(CHAR_STX))
		return EMVERROR_SENDDATA;
	
	ucLRC = 0x00;
	ucLow = uiLen&0xFF;
	ucHigh = (uiLen>>8)&0xFF;

	ucLRC ^= ucHigh;
	if(MSG_ComSendByte(ucHigh))
		return EMVERROR_SENDDATA;
	

	ucLRC^=ucLow;
	if(MSG_ComSendByte(ucLow))
		return EMVERROR_SENDDATA;
	

	for(uiI=0;uiI<uiLen;uiI++)
	{
		ucLRC^=*(pucBuff+uiI);
		if(MSG_ComSendByte(*(pucBuff+uiI)))
			return EMVERROR_SENDDATA;
		
	}

	ucLRC ^= 0x03;
	
	if(MSG_ComSendByte(CHAR_ETX))
		return EMVERROR_SENDDATA;

	if(MSG_ComSendByte(ucLRC))
		return EMVERROR_SENDDATA;
	
	
	return EMVERROR_SUCCESS;
}
UCHAR MSG_ComSendByte(UCHAR	ucSendChar)
{
	if(ConstParam.ucCommID ==0)
		Os__txcar(ucSendChar);
	else
		Os__txcar3(ucSendChar);
//	Os__xdelay(1);
	return EMVERROR_SUCCESS;
	
}

UCHAR MSG_ComReceiveByte(PUCHAR pucByte,USHORT uiTimeout)
{
	USHORT uiResult;
#if 0
	uiResult = Os__rxcar(uiTimeout);
#else

	
	if(ConstParam.ucCommID ==0)
		uiResult = Os__rxcar(uiTimeout);
	else
		uiResult = Os__rxcar3(uiTimeout);
#endif

	switch	( uiResult / 256 )
	{
	case EMVERROR_SUCCESS :
		*pucByte = uiResult % 256;
//		Uart_Printf("%02X ",*pucByte);
		return(EMVERROR_SUCCESS);
	default:
//		Uart_Printf("\nConstParam.ucCommID %02x",ConstParam.ucCommID);
		return EMVERROR_RECEIVEDATA;
	
	}
}

UCHAR MSG_ComRecvData(PUCHAR pBuff, USHORT *puiLen)
{
	
	USHORT uiI,uiDataLen,uiMaxLen;
	UCHAR  ucTemp,ucLRC;
    Os__clr_display11(255);
	if(ConstParam.bPBOC20==FALSE)
        Os__GB2312_display11(2,2,(PUCHAR)"Receive Data...");
	else
        Os__GB2312_display11(2,2,(PUCHAR)"接收数据...");

    emitSignal();


	uiMaxLen =*puiLen;
	for(uiI=0;uiI<100;uiI++)
	{
		if(!MSG_ComReceiveByte(pBuff,10))
			if(pBuff[0]==0x02) break;
	}
	if(uiI>=500) 
		return EMVERROR_RECEIVEDATA;
	
	ucLRC = 0;
	for(uiI=0;uiI<2;uiI++)
	{
		if(MSG_ComReceiveByte(pBuff+uiI+1,500)!=0)
			return EMVERROR_RECEIVEDATA;
		
		ucLRC^=*(pBuff+uiI+1);
	}
	
	uiDataLen=(unsigned int)(*(pBuff+1))*256 + *(pBuff+2);
	if(uiDataLen>uiMaxLen) return EMVERROR_OVERFLOW;
	
	for(uiI=0;uiI<uiDataLen;uiI++)
	{
		if(MSG_ComReceiveByte(pBuff+uiI,500)!=0)
			return EMVERROR_RECEIVEDATA;
		
		ucLRC^=*(pBuff+uiI);
	}
	
	
		
	if(MSG_ComReceiveByte(&ucTemp,500)!=0)
		return EMVERROR_RECEIVEDATA;
	
	if(ucTemp!=0x03)
		return EMVERROR_RECEIVEDATA;
	ucLRC ^= ucTemp;

	if(MSG_ComReceiveByte(&ucTemp,500)!=0)
		return EMVERROR_RECEIVEDATA;

	if(ucLRC!=ucTemp)
		return EMVERROR_RECEIVEDATA;

	*puiLen = uiDataLen;

	return EMVERROR_SUCCESS;
}


UCHAR MSG_ComClose()
{
	
//	OSUART_Close1();
#if 1
	if(ConstParam.ucCommID ==0)
		OSUART_Close1();
	else
		OSUART_Close2();
#endif
	return EMVERROR_SUCCESS;
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
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x82",ComPackSend+ComPackSendLen,&uiLen);//AIP
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x07",ComPackSend+ComPackSendLen,&uiLen);//ACU
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //ARQC
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x8E",ComPackSend+ComPackSendLen,&uiLen);  //CVM LIST
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1E",ComPackSend+ComPackSendLen,&uiLen);  //IFD
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x0D",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Default
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x0E",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Denial
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x0F",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Online
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x33",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Capabilities
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x35",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x37",ComPackSend+ComPackSendLen,&uiLen);  //Unpredictable Number
	ComPackSendLen += uiLen;
	
	
	if(((TermConfig.ucTermType&0xF0)==0x10)||((TermConfig.ucTermType&0xF0)==0x20))
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x01",ComPackSend+ComPackSendLen,&uiLen);  //Acquirer Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x02",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Authorised
	ComPackSendLen += uiLen;

	if(TransReqInfo.enTransType ==CASHBACK)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x03",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Other
		ComPackSendLen += uiLen;
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x25",ComPackSend+ComPackSendLen,&uiLen);  //Application Effective Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x24",ComPackSend+ComPackSendLen,&uiLen);  //Application Expiration Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5A",ComPackSend+ComPackSendLen,&uiLen);  //Primary Account Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x34",ComPackSend+ComPackSendLen,&uiLen);  //Application PAN Sequence Number
	ComPackSendLen += uiLen;
	/*
	if(EncryptPINLen>0)
		NP_Write(MV_EncPINData,EncryptPIN,EncryptPINLen);//max len 128 bytes
*/
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x28",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Country Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	if((TermConfig.ucTermType&0xF0)==0x20)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x15",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Category Code
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
		ComPackSendLen += uiLen;

		
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x39",ComPackSend+ComPackSendLen,&uiLen);  //POS Entry Mode
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1A",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Country Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x57",ComPackSend+ComPackSendLen,&uiLen);  //Track 2 Equivalent Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x81",ComPackSend+ComPackSendLen,&uiLen);  	//Transaction Amount
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x2A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Currency Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x21",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Time
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9C",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x4C",ComPackSend+ComPackSendLen,&uiLen); 	//TICC Dynamic Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x41",ComPackSend+ComPackSendLen,&uiLen); 	//Trans Sequensce Counter
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\xDF\x02",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x74",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x79",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;
	ucResult = MSG_OpenCom();

	if(!ucResult)
		ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
//	Uart_Printf("\nSend %02x",ucResult);
	if(!ucResult)
		ucResult =MSG_ComRecvData(pRecvBuff,puiLen);
	if(!ucResult)
		if(*pRecvBuff  !=Msg_EMV_AuthRP)
				ucResult = EMVERROR_RECEIVEDATA;
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
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x82",ComPackSend+ComPackSendLen,&uiLen);//AIP
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //ARQC
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1E",ComPackSend+ComPackSendLen,&uiLen);  //IFD
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x33",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Capabilities
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x35",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x37",ComPackSend+ComPackSendLen,&uiLen);  //Unpredictable Number
	ComPackSendLen += uiLen;
	
	
	if(((TermConfig.ucTermType&0xF0)==0x10)||((TermConfig.ucTermType&0xF0)==0x20))
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x01",ComPackSend+ComPackSendLen,&uiLen);  //Acquirer Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x02",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Authorised
	ComPackSendLen += uiLen;

	if(TransReqInfo.enTransType ==CASHBACK)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x03",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Other
		ComPackSendLen += uiLen;
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x25",ComPackSend+ComPackSendLen,&uiLen);  //Application Effective Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x24",ComPackSend+ComPackSendLen,&uiLen);  //Application Expiration Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5A",ComPackSend+ComPackSendLen,&uiLen);  //Primary Account Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x34",ComPackSend+ComPackSendLen,&uiLen);  //Application PAN Sequence Number
	ComPackSendLen += uiLen;
	/*
	if(EncryptPINLen>0)
		NP_Write(MV_EncPINData,EncryptPIN,EncryptPINLen);//max len 128 bytes
*/
	

	if((TermConfig.ucTermType&0xF0)==0x20)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x15",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Category Code
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
		ComPackSendLen += uiLen;

		
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x39",ComPackSend+ComPackSendLen,&uiLen);  //POS Entry Mode
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1A",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Country Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x57",ComPackSend+ComPackSendLen,&uiLen);  //Track 2 Equivalent Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x2A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Currency Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x21",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Time
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9C",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x4C",ComPackSend+ComPackSendLen,&uiLen); 	//ICC Dynamic Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x41",ComPackSend+ComPackSendLen,&uiLen); 	//Trans Sequensce Counter
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\xDF\x02",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x74",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x79",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;
	
	ucResult = MSG_OpenCom();

	if(!ucResult)
		ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
	
	if(!ucResult)
		ucResult =MSG_ComRecvData(pRecvBuff,puiLen);
	if(!ucResult)
		if(*pRecvBuff  !=Msg_EMV_AuthRP)
				ucResult = EMVERROR_RECEIVEDATA;
	MSG_ComClose();

	return ucResult;
}


UCHAR	MSG_TransConfirm(void)
{
	
	UCHAR ComPackSend[1024],ucResult,ComPackRecv[300];
	
	USHORT ComPackSendLen,uiTagBuffLen,uiLen,uiBuffsize,ComPackRecvLen;


    Os__clr_display11(255);

//    Os__GB2312_display11(2,0,"Confirm");
    emitSignal();


	memset(ComPackSend,0,1024);
	ComPackSendLen=0;

	uiBuffsize = sizeof(ComPackSend);
	
	ComPackSend[0] = Msg_EMV_Confirm;
	ComPackSendLen ++;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;

	if(EMVTransInfo.enTransResult ==OFFLINEAPPROVED)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	else
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5A",ComPackSend+ComPackSendLen,&uiLen);  //Primary Account Number
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x34",ComPackSend+ComPackSendLen,&uiLen);  //Application PAN Sequence Number
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x02",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Authorised
	ComPackSendLen += uiLen;

	if(TransReqInfo.enTransType ==CASHBACK)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x03",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Other
		ComPackSendLen += uiLen;
		
	}
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
	ComPackSendLen += uiLen;

	if(EMVTransInfo.ucScriptResultNum)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\xDF\x31",ComPackSend+ComPackSendLen,&uiLen);  //Issure Script Result
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x41",ComPackSend+ComPackSendLen,&uiLen); 	//Trans Sequensce Counter
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x74",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x79",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;
	
	ComPackRecvLen =sizeof(ComPackRecv);
	ucResult = MSG_OpenCom();

	if(!ucResult)
		ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
	
	if(!ucResult)
		ucResult =MSG_ComRecvData(ComPackRecv,&ComPackRecvLen);
	if(!ucResult)
		if(ComPackRecv[0] !=Msg_EMV_Confirm)
				ucResult = EMVERROR_RECEIVEDATA;
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
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x82",ComPackSend+ComPackSendLen,&uiLen);//AIP
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //ARQC
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1E",ComPackSend+ComPackSendLen,&uiLen);  //IFD
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x33",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Capabilities
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x35",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;
	
	if(((TermConfig.ucTermType&0xF0)==0x10)||((TermConfig.ucTermType&0xF0)==0x20))
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x01",ComPackSend+ComPackSendLen,&uiLen);  //Acquirer Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x24",ComPackSend+ComPackSendLen,&uiLen);  //Application Expiration Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5A",ComPackSend+ComPackSendLen,&uiLen);  //Primary Account Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x34",ComPackSend+ComPackSendLen,&uiLen);  //Application PAN Sequence Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x8A",ComPackSend+ComPackSendLen,&uiLen);  //Auth Response Code
	ComPackSendLen += uiLen;

	if((TermConfig.ucTermType&0xF0)==0x20)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x15",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Category Code
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x39",ComPackSend+ComPackSendLen,&uiLen);  //POS Entry Mode
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x02",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Authorised
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1A",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Country Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x57",ComPackSend+ComPackSendLen,&uiLen);  //Track 2 Equivalent Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x2A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Currency Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x21",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Time
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9C",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Type
	ComPackSendLen += uiLen;
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x74",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x79",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;
	
	if(EMVTransInfo.ucScriptResultNum)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\xDF\x31",ComPackSend+ComPackSendLen,&uiLen);  //Issure Script Result
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
				ucResult = EMVERROR_RECEIVEDATA;
			
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
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x82",ComPackSend+ComPackSendLen,&uiLen);//AIP
	ComPackSendLen += uiLen;
	

	if(EMVTransInfo.enTransResult ==OFFLINEDECLINED)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	else
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1E",ComPackSend+ComPackSendLen,&uiLen);  //IFD
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x33",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Capabilities
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x35",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;

	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x37",ComPackSend+ComPackSendLen,&uiLen);  //Unpredictable Number
	ComPackSendLen += uiLen;
	
	
	if(((TermConfig.ucTermType&0xF0)==0x10)||((TermConfig.ucTermType&0xF0)==0x20))
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x01",ComPackSend+ComPackSendLen,&uiLen);  //Acquirer Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x02",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Authorised
	ComPackSendLen += uiLen;

	if(TransReqInfo.enTransType ==CASHBACK)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x03",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Other
		ComPackSendLen += uiLen;
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x25",ComPackSend+ComPackSendLen,&uiLen);  //Application Effective Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x24",ComPackSend+ComPackSendLen,&uiLen);  //Application Expiration Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5A",ComPackSend+ComPackSendLen,&uiLen);  //Primary Account Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x34",ComPackSend+ComPackSendLen,&uiLen);  //Application PAN Sequence Number
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x8A",ComPackSend+ComPackSendLen,&uiLen);  //Auth Response Code
	ComPackSendLen += uiLen;

	if((TermConfig.ucTermType&0xF0)==0x20)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x15",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Category Code
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x39",ComPackSend+ComPackSendLen,&uiLen);  //POS Entry Mode
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1A",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Country Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x57",ComPackSend+ComPackSendLen,&uiLen);  //Track 2 Equivalent Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x2A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Currency Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x21",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Time
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9C",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x74",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x79",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;
	
	if(EMVTransInfo.ucScriptResultNum)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\xDF\x31",ComPackSend+ComPackSendLen,&uiLen);  //Issure Script Result
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
				ucResult = EMVERROR_RECEIVEDATA;
				
	ucResult =EMVERROR_SUCCESS;
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
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x82",ComPackSend+ComPackSendLen,&uiLen);//AIP
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x07",ComPackSend+ComPackSendLen,&uiLen);//ACU
	ComPackSendLen += uiLen;


	if(EMVTransInfo.enTransResult ==OFFLINEAPPROVED ||EMVTransInfo.enTransResult ==OFFLINEDECLINED)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	else
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //ARQC
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x27",ComPackSend+ComPackSendLen,&uiLen);  //CID
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
		ComPackSendLen += uiLen;
		
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,(PUCHAR)"\x9F\x36",ComPackSend+ComPackSendLen,&uiLen);//ATC
		ComPackSendLen += uiLen;
	}
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x8E",ComPackSend+ComPackSendLen,&uiLen);  //CVM LIST
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x34",ComPackSend+ComPackSendLen,&uiLen);  //CVM RESULT
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1E",ComPackSend+ComPackSendLen,&uiLen);  //IFD
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x0D",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Default
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x0E",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Denial
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x0F",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Action Code - Online
	ComPackSendLen += uiLen;
	
//	uiLen = uiBuffsize - ComPackSendLen;
//	EMVDATA_TagPackProcess(ALLPHASETAG,"\x9F\x10",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Application Data
//	ComPackSendLen += uiLen;

	if(EMVTransInfo.ucScriptResultNum)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\xDF\x31",ComPackSend+ComPackSendLen,&uiLen);  //Issure Script Result
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x33",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Capabilities
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x35",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x95",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Verification Results
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9B",ComPackSend+ComPackSendLen,&uiLen);  //Transaction Status Information
	ComPackSendLen += uiLen;
/*	
	if(EMVTransInfo.enTransResult == OFFLINEAPPROVED)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //TC or AAC
		ComPackSendLen += uiLen;
	}
	else 
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(SECONDGACPHASETAG,"\x9F\x26",ComPackSend+ComPackSendLen,&uiLen);  //TC or AAC
		ComPackSendLen += uiLen;
	}
*/
//????record
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x39",ComPackSend+ComPackSendLen,&uiLen);  //POS Entry Mode
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x37",ComPackSend+ComPackSendLen,&uiLen);  //Unpredictable Number
	ComPackSendLen += uiLen;
	
	
	if(((TermConfig.ucTermType&0xF0)==0x10)||((TermConfig.ucTermType&0xF0)==0x20))
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x01",ComPackSend+ComPackSendLen,&uiLen);  //Acquirer Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x02",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Authorised
	ComPackSendLen += uiLen;

	if(TransReqInfo.enTransType ==CASHBACK)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x03",ComPackSend+ComPackSendLen,&uiLen);  //Amount, Other
		ComPackSendLen += uiLen;
		
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x25",ComPackSend+ComPackSendLen,&uiLen);  //Application Effective Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x24",ComPackSend+ComPackSendLen,&uiLen);  //Application Expiration Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5A",ComPackSend+ComPackSendLen,&uiLen);  //Primary Account Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x34",ComPackSend+ComPackSendLen,&uiLen);  //Application PAN Sequence Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x28",ComPackSend+ComPackSendLen,&uiLen);  //Issuer Country Code
	ComPackSendLen += uiLen;
	

	if((TermConfig.ucTermType&0xF0)==0x20)
	{
		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x15",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Category Code
		ComPackSendLen += uiLen;

		uiLen = uiBuffsize - ComPackSendLen;
		EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x16",ComPackSend+ComPackSendLen,&uiLen);  //Merchant Identifier
		ComPackSendLen += uiLen;
	}

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1A",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Country Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x1C",ComPackSend+ComPackSendLen,&uiLen);  //Terminal Identification
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x57",ComPackSend+ComPackSendLen,&uiLen);  //Track 2 Equivalent Data
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x81",ComPackSend+ComPackSendLen,&uiLen);  	//Transaction Amount
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x5F\x2A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Currency Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9A",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Date
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x21",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Time
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9C",ComPackSend+ComPackSendLen,&uiLen); 	//Transaction Type
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x4C",ComPackSend+ComPackSendLen,&uiLen); 	//TICC Dynamic Number
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x8A",ComPackSend+ComPackSendLen,&uiLen);  //Auth Response Code
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x89",ComPackSend+ComPackSendLen,&uiLen);  //Auth  Code
	ComPackSendLen += uiLen;
	
	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x41",ComPackSend+ComPackSendLen,&uiLen); 	//Trans Sequensce Counter
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x74",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;

	uiLen = uiBuffsize - ComPackSendLen;
	EMVDATA_TagPackProcess(ALLPHASETAG,(PUCHAR)"\x9F\x79",ComPackSend+ComPackSendLen,&uiLen); 	//Online Encipher Pin 
	ComPackSendLen += uiLen;
	
	CONV_LongHex(ComPackSend,sizeof(USHORT),&ComPackSendLen);

	iHandle = OSAPP_OpenFile(BATCHRECORD,O_CREATE|O_WRITE);
	if(iHandle>=0)
	{
			

		FleSize =OSAPP_FileSize(BATCHRECORD);
		ucResult =OSAPP_FileSeek(iHandle,FleSize,SEEK_SET);
		
		if(!ucResult)
			if(ComPackSendLen != OSAPP_FileWrite(iHandle,ComPackSend,ComPackSendLen))
				ucResult = EMVERROR_SAVEFILE;
		OSAPP_FileClose(iHandle);
	}
	

	return ucResult;
	
}

UCHAR	MSG_Reconciliation(void)
{
    printf("AAAAAAAAAAA\n");
	UCHAR ComPackSend[1024],ComPackRecv[300],ucResult;
	
	USHORT ComPackSendLen,ComPackRecvLen,VarReadLen,uiTagBuffLen,uiLen,uiBuffsize;
	UCHAR	aucTotalAmount[6],aucBatchNumber[6];

	ComPackRecvLen = sizeof(ComPackRecv);
    printf("bbbbbb\n");

	uiBuffsize = sizeof(ComPackSend);
	memset(ComPackSend,0,1024);
	ComPackSendLen=0;
	ComPackSend[ComPackSendLen] = Msg_EMV_Reconci;
	ComPackSendLen ++;
	
	EMVTAG_INIT((DATAPROPERTY *)TermDataProperty,TRUE);
	EMVTAG_SetTagValue((PUCHAR)"\x9F\x1C",ConstParam.aucTerminalID,TERMINALIDLEN);
	EMVTAG_SetTagValue((PUCHAR)"\x9F\x16",ConstParam.aucMerchantID,MERCHANTIDLEN);
	if(((EMVConfig.ucTermType&0xF0)==0x10)||((EMVConfig.ucTermType&0xF0)==0x20))
		EMVTAG_SetTagValue((PUCHAR)"\x9F\x35",&EMVConfig.ucTermType,1);
	EMVTAG_SetTagValue((PUCHAR)"\x5F\x2A",EMVConfig.aucTransCurrencyCode,TRANSCURRENCYCODELEN);
	CONV_LongBcd(aucTotalAmount,sizeof(aucTotalAmount),&ConstParam.uiTotalAmount);
	EMVTAG_SetTagValue((PUCHAR)"\x9F\x02",aucTotalAmount,sizeof(aucTotalAmount));
	CONV_LongBcd(aucBatchNumber,sizeof(aucBatchNumber),&ConstParam.uiBatchNumber);
	EMVTAG_SetTagValue((PUCHAR)"\xDF\x01",aucBatchNumber,sizeof(aucBatchNumber));
    printf("ccccc\n");

	VarReadLen =uiBuffsize -ComPackSendLen;
	ucResult = EMVTAG_Pack(&ComPackSend[ComPackSendLen],&VarReadLen);
    printf("ddddddddducResult ==%02x \n",ucResult);

	if(!ucResult)
	{
		ComPackSendLen += VarReadLen;
//		CONV_LongHex(ComPackSend,sizeof(USHORT),&ComPackSendLen);
		ComPackRecvLen =sizeof(ComPackRecv);
		ucResult = MSG_OpenCom();
        printf("111ucResult ==%02x \n",ucResult);

		if(!ucResult)
			ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
        Uart_Printf("\nSend %02x",ucResult);
		if(!ucResult)
			ucResult =MSG_ComRecvData(ComPackRecv,&ComPackRecvLen);
        printf("222ucResult ==%02x \n",ucResult);

		if(!ucResult)
			if(ComPackRecv[0] !=Msg_EMV_Reconci)
				ucResult = EMVERROR_RECEIVEDATA;
		MSG_ComClose();
        printf("333ucResult ==%02x \n",ucResult);

		if(!ucResult)
			ucResult = MSG_BatchUp();
	}
	else ucResult = EMVERROR_DATA;
    printf("444ucResult ==%02x \n",ucResult);

	if(!ucResult)
	{
		UTIL_DeleteAllData(TRANSLOG);
		ConstParam.uiBatchNumber++;
		if(ConstParam.uiBatchNumber>999999)
			ConstParam.uiBatchNumber =1;
		ConstParam.uiTotalAmount =0;
		ConstParam.uiTotalNums =0;
		ucResult =UTIL_WriteConstParamFile(&ConstParam);
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
    printf("eeeeeeeeeucResult ==%02x \n",ucResult);

	EMVERROR_DisplayMsg(ucResult);
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

    printf("1111111111\n");

	ComPackSend[0] = Msg_EMV_UpdatePara;
    printf("222222222\n");
    ComPackSendLen ++;
    printf("aaaa\n");
    ucResult = MSG_OpenCom();
  printf("333333333ucResult ==%02x \n",ucResult);
	if(!ucResult)
		ucResult = MSG_ComSendData(ComPackSend,ComPackSendLen);
    printf("44444ucResult ==%02x \n",ucResult);
    if(!ucResult)
		ucResult =MSG_ComRecvData(ComPackRecv,&ComPackRecvLen);
    printf("55555ucResult ==%02x \n",ucResult);

	if(ComPackRecv[0]!=Msg_EMV_UpdatePara)
		ucResult =EMVERROR_PARAMUPDATE;
	if(!ucResult)
		ucResult = MSG_ProcessParamUpdate(&ComPackRecv[1],ComPackRecvLen-1);
    printf("66666ucResult ==%02x \n",ucResult);

	if(!ucResult)
		ucResult = UTIL_WriteEMVConfigFile(&EMVConfig);
    printf("7777ucResult ==%02x \n",ucResult);

	MSG_ComClose();
    printf("88888ucResult ==%02x \n",ucResult);

	if(!ucResult)
    {  printf("99999ucResult ==%02x \n",ucResult);

        Os__clr_display11(255);

        Os__GB2312_display11(2,0,"Param Update OK!");
        emitSignal();

		UTIL_GetKey(3);
	}
	else 
		EMVERROR_DisplayMsg(ucResult);
    printf("enducResult ==%02x \n",ucResult);

	return ucResult;
	
	
}


UCHAR	MSG_ProcessParamUpdate(UCHAR	*pucParamData,USHORT uiParamLen)
{
	USHORT	uiI,uiLen,uiDataLen;
	UCHAR	ucResult,ucMsg1,ucMsg2;
	BOOL	bBaseParam,bCAPKParam,bAIDParam,bExceptFileParam,bIPKRevokeParam;
	
	ucResult =EMVERROR_SUCCESS;
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
					ucResult = EMVERROR_PARAMUPDATE;
				break;
			case CAPKPARAM:
				uiDataLen = (USHORT)CONV_HexLong(pucParamData,2);
				if(uiI +4+uiDataLen <=uiParamLen)
				{
					ucResult = MSG_ProcessCAPKUpdate(ucMsg2,pucParamData+2,uiDataLen);
					if(!ucResult) bCAPKParam = TRUE;
				}
				else
					ucResult = EMVERROR_PARAMUPDATE;
				break;
			case AIDPARAM:
				uiDataLen = (USHORT)CONV_HexLong(pucParamData,2);
				if(uiI +4+uiDataLen <=uiParamLen)
				{
					ucResult = MSG_ProcessAIDUpdate(ucMsg2,pucParamData+2,uiDataLen);
					if(!ucResult) bAIDParam = TRUE;
				}
				else
					ucResult = EMVERROR_PARAMUPDATE;
				break;
			case EXCEPTFILEPARAM:
				uiDataLen = (USHORT)CONV_HexLong(pucParamData,2);
				if(uiI +4+uiDataLen <=uiParamLen)
				{
					ucResult = MSG_ProcessExceptFileUpdate(ucMsg2,pucParamData+2,uiDataLen);
					if(!ucResult) bExceptFileParam = TRUE;
				}
				else
					ucResult = EMVERROR_PARAMUPDATE;
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
					ucResult = EMVERROR_PARAMUPDATE;
				break;
			default:
				ucResult =EMVERROR_PARAMUPDATE;
				break;
			}
			uiI += uiDataLen+4;
			pucParamData += uiDataLen+2;
			
		}
		else ucResult =EMVERROR_PARAMUPDATE;
		
	}
	if(!ucResult)
	{
		if(bBaseParam)
			ucResult = UTIL_WriteEMVConfigFile(&EMVConfig);
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
	
	ucResult = EMVERROR_SUCCESS;
	uiI = 0;
	ucEntrys =sizeof(ParamIndicate)/sizeof(PARAMINDICATE);
	
	while(!ucResult && uiI <uiParamLen)
	{
		
		ucIndex =pBaseParam[uiI];
		ucLen = EMVTAG_GetTagLenAttr(pBaseParam+uiI+1,&uiLen);
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
				else ucResult = EMVERROR_PARAMUPDATE;
			}
			else
				ucResult = EMVERROR_PARAMUPDATE;
			if(!ucResult) 
				uiI += ucLen+uiLen+1;
		}
		else ucResult = EMVERROR_PARAMUPDATE;
		
	}

	return ucResult;
	
}

UCHAR	MSG_CAPKAdd(PCAPK	pCapk)
{
	UCHAR	ucI,ucResult;
	PCAPK	pTermCAPK;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucTermCAPKNum;ucI++)
	{
		pTermCAPK =&TermCAPK[ucI];
		if(!memcmp(pTermCAPK->aucRID,pCapk->aucRID,RIDDATALEN) 
			&&(pTermCAPK->ucCAPKI==pCapk->ucCAPKI))
			break;
	}
	if(ucI <ucTermCAPKNum)
		memcpy(pTermCAPK,pCapk,sizeof(CAPK));
	else if(ucTermCAPKNum<MAXTERMCAPKNUMS)
	{
		memcpy(&TermCAPK[ucTermCAPKNum],pCapk,sizeof(CAPK));
		ucTermCAPKNum ++;
	}
	else ucResult = EMVERROR_OVERFLOW;

	return ucResult;
	
		
}


UCHAR	MSG_CAPKModify(PCAPK 	pCapk)
{
	UCHAR	ucI,ucResult;
	PCAPK	pTermCAPK;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucTermCAPKNum;ucI++)
	{
		pTermCAPK =&TermCAPK[ucI];
		if(!memcmp(pTermCAPK->aucRID,pCapk->aucRID,RIDDATALEN) 
			&&(pTermCAPK->ucCAPKI==pCapk->ucCAPKI))
			break;
	}
	
	if(ucI <ucTermCAPKNum)
		memcpy(pTermCAPK,pCapk,sizeof(CAPK));
	

	return ucResult;
	
		
}

UCHAR	MSG_CAPKDelete(PCAPK		pCapk)
{
	UCHAR	ucI,ucJ,ucResult;
	PCAPK	pTermCAPK;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucTermCAPKNum;ucI++)
	{
		pTermCAPK =&TermCAPK[ucI];
		if(!memcmp(pTermCAPK->aucRID,pCapk->aucRID,RIDDATALEN) 
			&&(pTermCAPK->ucCAPKI==pCapk->ucCAPKI))
			break;
	}
	
	if(ucI <ucTermCAPKNum&&ucTermCAPKNum)
	{
		for(ucJ =ucI; ucJ <ucTermCAPKNum-1;ucJ++)
			memcpy(&TermCAPK[ucJ],&TermCAPK[ucJ+1],sizeof(CAPK));
		ucTermCAPKNum--;
	}
	

	return ucResult;
	
		
}

UCHAR	MSG_ProcessCAPKUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen)
{
	USHORT	uiI,uiLen;
	UCHAR	ucResult,ucIndex,ucLen,auchashBuff[HASHDATALEN];
	UCHAR	aucBuff[1024];
	PCAPK	pCapk;
	
	ucResult = EMVERROR_SUCCESS;
	
	if(uiParamLen !=sizeof(CAPK))
		ucResult = EMVERROR_PARAMUPDATE;

	pCapk =(PCAPK)pBaseParam;
	uiLen =0;
//	aucBuff[uiLen++] = pCapk->ucModulLen;
	memcpy(&aucBuff[uiLen],pCapk->aucRID,RIDDATALEN);
	uiLen += RIDDATALEN;
	aucBuff[uiLen++] = pCapk->ucIndex;
//	memcpy(aucBuff+uiLen,&pCapk->ucCAPKI,4);
//	uiLen +=4;
	memcpy(aucBuff+uiLen,&pCapk->aucModul,pCapk->ucModulLen);
	uiLen +=pCapk->ucModulLen;
	memcpy(aucBuff+uiLen,&pCapk->aucExponent,pCapk->ucExponentLen);
	uiLen +=pCapk->ucExponentLen;
	
	SHA1_Compute(aucBuff,uiLen,auchashBuff);
	if(memcmp(auchashBuff,pCapk->aucCheckSum,HASHDATALEN))
		ucResult = EMVERROR_HASHVALUE;
	if(!ucResult)
	{
		switch(ucAction)
		{
		case ADDACTION:
			ucResult = MSG_CAPKAdd((PCAPK)pBaseParam);
			break;
		case DELETEACTION:
			ucResult = MSG_CAPKDelete((PCAPK)pBaseParam);
			break;
		case MODIFYACTION:
			ucResult = MSG_CAPKModify((PCAPK)pBaseParam);
			break;
		}
	}
	return ucResult;
}

UCHAR	MSG_IPKRevokeAdd(PIPKREVOKE	pIPKRevoke)
{
	UCHAR	ucI,ucResult;
	PIPKREVOKE	pTermIPKRevoke;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucIPKRevokeNum;ucI++)
	{
		pTermIPKRevoke =&IPKRevoke[ucI];
		if(!memcmp(pTermIPKRevoke->aucRID,pIPKRevoke->aucRID,RIDDATALEN) 
			&&(pTermIPKRevoke->ucCAPKI==pIPKRevoke->ucCAPKI))
			break;
	}
	if(ucI <ucIPKRevokeNum)
		memcpy(pTermIPKRevoke,pIPKRevoke,sizeof(IPKREVOKE));
	else if(ucIPKRevokeNum<MAXIPKREVOKENUMS)
	{
		memcpy(&IPKRevoke[ucIPKRevokeNum],pIPKRevoke,sizeof(IPKREVOKE));
		ucIPKRevokeNum ++;
	}
	else ucResult = EMVERROR_PARAMUPDATE;

	return ucResult;
	
		
}


UCHAR	MSG_IPKRevokeModify(PIPKREVOKE 	pIPKRevoke)
{
	UCHAR	ucI,ucResult;
	PIPKREVOKE	pTermIPKRevoke;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucIPKRevokeNum;ucI++)
	{
		pIPKRevoke =&IPKRevoke[ucI];
		if(!memcmp(pTermIPKRevoke->aucRID,pIPKRevoke->aucRID,RIDDATALEN) 
			&&(pTermIPKRevoke->ucCAPKI==pIPKRevoke->ucCAPKI))
			break;
	}
	
	if(ucI <ucIPKRevokeNum)
		memcpy(pIPKRevoke,pIPKRevoke,sizeof(IPKREVOKE));
	

	return ucResult;
	
		
}

UCHAR	MSG_IPKRevokeDelete(PIPKREVOKE		pIPKRevoke)
{
	UCHAR	ucI,ucJ,ucResult;
	PIPKREVOKE	pTermIPKRevoke;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucIPKRevokeNum;ucI++)
	{
		pTermIPKRevoke =&IPKRevoke[ucI];
		if(!memcmp(pTermIPKRevoke->aucRID,pIPKRevoke->aucRID,RIDDATALEN) 
			&&(pTermIPKRevoke->ucCAPKI==pIPKRevoke->ucCAPKI))
			break;
	}
	
	if(ucI <ucIPKRevokeNum&&ucIPKRevokeNum)
	{
		for(ucJ =ucI; ucJ <ucIPKRevokeNum-1;ucJ++)
			memcpy(&IPKRevoke[ucJ],&IPKRevoke[ucJ+1],sizeof(IPKREVOKE));
		ucIPKRevokeNum--;
	}
	

	return ucResult;
	
		
}

UCHAR	MSG_ProcessIPKRevokeUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen)
{
	USHORT	uiI;
	UCHAR	ucResult,ucIndex,ucLen;
	
	ucResult = EMVERROR_SUCCESS;
	
	if(uiParamLen !=sizeof(IPKREVOKE))
		ucResult = EMVERROR_PARAMUPDATE;
	if(!ucResult)
	{
		switch(ucAction)
		{
		case ADDACTION:
			ucResult = MSG_IPKRevokeAdd((PIPKREVOKE)pBaseParam);
			break;
		case DELETEACTION:
			ucResult = MSG_IPKRevokeDelete((PIPKREVOKE)pBaseParam);
			break;
		case MODIFYACTION:
			ucResult = MSG_IPKRevokeModify((PIPKREVOKE)pBaseParam);
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
					else 	ucResult= EMVERROR_READFILE;
				}
				else 	ucResult= EMVERROR_READFILE;
			}
			else 	break;
			
		}while(!ucResult);
		OSAPP_FileTrunc(iHandle,0);
		OSAPP_FileClose(iHandle);
	}
	MSG_ComClose();
	
	

	return ucResult;
	
}


UCHAR	MSG_AIDAdd(TERMSUPPORTAPP* pAID)
{
	UCHAR	ucI,ucResult;
	TERMSUPPORTAPP*	pTermAID;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucTermAIDNum;ucI++)
	{
		pTermAID =&TermAID[ucI];
		if((pTermAID->ucAIDLen==pAID->ucAIDLen)
			&&!memcmp(pTermAID->aucAID,pAID->aucAID,pAID->ucAIDLen))
			break;
	}
	if(ucI <ucTermAIDNum)
		memcpy(pTermAID,pAID,sizeof(TERMSUPPORTAPP));
	else if(ucTermAIDNum<MAXTERMAIDNUMS)
	{
		memcpy(&TermAID[ucTermAIDNum],pAID,sizeof(TERMSUPPORTAPP));
		ucTermAIDNum ++;
	}
	else ucResult = EMVERROR_PARAMUPDATE;

	return ucResult;
	
		
}


UCHAR	MSG_AIDModify(TERMSUPPORTAPP* pAID)
{
	UCHAR	ucI,ucResult;
	TERMSUPPORTAPP*	pTermAID;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucTermAIDNum;ucI++)
	{
		pTermAID =&TermAID[ucI];
		if((pTermAID->ucAIDLen==pAID->ucAIDLen)
			&&!memcmp(pTermAID->aucAID,pAID->aucAID,pAID->ucAIDLen))
			break;
	}
	
	if(ucI <ucTermAIDNum)
		memcpy(pTermAID,pAID,sizeof(TERMSUPPORTAPP));
	

	return ucResult;
	
		
}

UCHAR	MSG_AIDDelete(TERMSUPPORTAPP* pAID)
{
	UCHAR	ucI,ucJ,ucResult;
	TERMSUPPORTAPP*	pTermAID;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucTermAIDNum;ucI++)
	{
		pTermAID =&TermAID[ucI];
		if((pTermAID->ucAIDLen==pAID->ucAIDLen)
			&&!memcmp(pTermAID->aucAID,pAID->aucAID,pAID->ucAIDLen))
			break;
	}
	
	if(ucI <ucTermAIDNum&&ucTermAIDNum)
	{
		for(ucJ =ucI; ucJ <ucTermAIDNum-1;ucJ++)
			memcpy(&TermAID[ucJ],&TermAID[ucJ+1],sizeof(TERMSUPPORTAPP));
		ucTermAIDNum--;
	}
	

	return ucResult;
	
		
}

UCHAR	MSG_ProcessAIDUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen)
{
	USHORT	uiI;
	UCHAR	ucResult,ucIndex,ucLen;
	
	ucResult = EMVERROR_SUCCESS;
	
	if(uiParamLen !=sizeof(TERMSUPPORTAPP))
		ucResult = EMVERROR_PARAMUPDATE;
	if(!ucResult)
	{
		switch(ucAction)
		{
		case ADDACTION:
			ucResult = MSG_AIDAdd((TERMSUPPORTAPP*)pBaseParam);
			break;
		case DELETEACTION:
			ucResult = MSG_AIDDelete((TERMSUPPORTAPP*)pBaseParam);
			break;
		case MODIFYACTION:
			ucResult = MSG_AIDModify((TERMSUPPORTAPP*)pBaseParam);
			break;
		}
	}
	return ucResult;
}



UCHAR	MSG_ExceptFileAdd(PEXCEPTPAN	pFile)
{
	UCHAR	ucI,ucResult;
	PEXCEPTPAN	pTermFile;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucExceptFileNum;ucI++)
	{
		pTermFile =&ExceptFile[ucI];
		if((pTermFile->ucPANSeq ==pFile->ucPANSeq)
			&&!memcmp(pTermFile->aucPAN,pFile->aucPAN,MAXPANDATALEN))
			break;
	}
	if(ucI <ucExceptFileNum)
		memcpy(pTermFile,pFile,sizeof(EXCEPTPAN));
	else if(ucExceptFileNum<MAXEXCEPTFILENUMS)
	{
		memcpy(&ExceptFile[ucExceptFileNum],pFile,sizeof(EXCEPTPAN));
		ucExceptFileNum ++;
	}
	else ucResult = EMVERROR_PARAMUPDATE;

	return ucResult;
	
		
}


UCHAR	MSG_ExceptFileModify(PEXCEPTPAN	pFile)
{
	UCHAR	ucI,ucResult;
	PEXCEPTPAN	pTermFile;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucExceptFileNum;ucI++)
	{
		pTermFile =&ExceptFile[ucI];
		if((pTermFile->ucPANSeq ==pFile->ucPANSeq)
			&&!memcmp(pTermFile->aucPAN,pFile->aucPAN,MAXPANDATALEN))
			break;
	}
	
	if(ucI <ucExceptFileNum)
		memcpy(pTermFile,pFile,sizeof(EXCEPTPAN));
	

	return ucResult;
	
		
}

UCHAR	MSG_ExceptFileDelete(PEXCEPTPAN pFile)
{
	UCHAR	ucI,ucJ,ucResult;
	PEXCEPTPAN	pTermFile;

	ucResult = EMVERROR_SUCCESS;
		
	for(ucI =0 ;ucI <ucExceptFileNum;ucI++)
	{
		pTermFile =&ExceptFile[ucI];
		if((pTermFile->ucPANSeq ==pFile->ucPANSeq)
			&&!memcmp(pTermFile->aucPAN,pFile->aucPAN,MAXPANDATALEN))
			break;
	}
	
	if(ucI <ucExceptFileNum&&ucExceptFileNum)
	{
		for(ucJ =ucI; ucJ <ucExceptFileNum-1;ucJ++)
			memcpy(&ExceptFile[ucJ],&ExceptFile[ucJ+1],sizeof(EXCEPTPAN));
		ucExceptFileNum--;
	}
	

	return ucResult;
	
		
}

UCHAR	MSG_ProcessExceptFileUpdate(UCHAR ucAction,PUCHAR	pBaseParam,USHORT uiParamLen)
{
	USHORT	uiI;
	UCHAR	ucResult,ucIndex,ucLen;
	
	ucResult = EMVERROR_SUCCESS;
	
	if(uiParamLen !=sizeof(EXCEPTPAN))
		ucResult = EMVERROR_PARAMUPDATE;
	if(!ucResult)
	{
		switch(ucAction)
		{
		case ADDACTION:
			ucResult = MSG_ExceptFileAdd((PEXCEPTPAN)pBaseParam);
			break;
		case DELETEACTION:
			ucResult = MSG_ExceptFileDelete((PEXCEPTPAN)pBaseParam);
			break;
		case MODIFYACTION:
			ucResult = MSG_ExceptFileModify((PEXCEPTPAN)pBaseParam);
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
	if(!ConstParam.bBatchCapture)
		ComPackSend[ComPackSendLen] = Msg_EMV_FinaRQ;
	else
		ComPackSend[ComPackSendLen] = Msg_EMV_AuthRQ;
	ComPackSendLen ++;
	
	EMVTAG_INIT((DATAPROPERTY *)TermDataProperty,TRUE);
	EMVTAG_SetTagValue((PUCHAR)"\x9F\x1C",ConstParam.aucTerminalID,TERMINALIDLEN);
	EMVTAG_SetTagValue((PUCHAR)"\x9F\x16",ConstParam.aucMerchantID,MERCHANTIDLEN);
	if(((EMVConfig.ucTermType&0xF0)==0x10)||((EMVConfig.ucTermType&0xF0)==0x20))
		EMVTAG_SetTagValue((PUCHAR)"\x9F\x35",&EMVConfig.ucTermType,1);
	EMVTAG_SetTagValue((PUCHAR)"\x9C",(PUCHAR)&TransReqInfo.enTransType,1);
	EMVTAG_SetTagValue((PUCHAR)"\x5F\x2A",EMVConfig.aucTransCurrencyCode,TRANSCURRENCYCODELEN);
	ucTrack2Len =strlen((char*)MagTrack2);
	CONV_AscBcd(aucTrack2Data,ucTrack2Len/2,MagTrack2,ucTrack2Len);
	EMVTAG_SetTagValue((PUCHAR)"\x57",aucTrack2Data,ucTrack2Len/2);	 //Track 2 Equivalent Data

	CONV_LongBcd(aucTotalAmount,sizeof(aucTotalAmount),&TransReqInfo.uiAmount);
	EMVTAG_SetTagValue((PUCHAR)"\x9F\x02",aucTotalAmount,sizeof(aucTotalAmount));
	if(TransReqInfo.enTransType ==CASHBACK)
	{
		CONV_LongBcd(aucOtherAmount,sizeof(aucOtherAmount),&TransReqInfo.uiOtherAmount);
		EMVTAG_SetTagValue((PUCHAR)"\x9F\x03",aucOtherAmount,sizeof(aucOtherAmount));
	}
	EMVTAG_SetTagValue((PUCHAR)"\x9F\x39",&ucEntryMode,1);
	CONV_LongBcd(aucBatchNumber,sizeof(aucBatchNumber),&ConstParam.uiBatchNumber);
	EMVTAG_SetTagValue((PUCHAR)"\xDF\x01",aucBatchNumber,sizeof(aucBatchNumber));

	CONV_LongHex(aucTracerNumber,sizeof(UINT),&TransReqInfo.uiTraceNumber);							//Trans sequence Counter
	EMVTAG_SetTagValue((PUCHAR)"\x9F\x41",aucTracerNumber,sizeof(UINT));
	
	
	VarReadLen =uiBuffsize -ComPackSendLen;
	ucResult = EMVTAG_Pack(&ComPackSend[ComPackSendLen],&VarReadLen);
	if(!ucResult)
	{
		ComPackSendLen += VarReadLen;
//		if(!ConstParam.bBatchCapture)
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
					ucResult = EMVERROR_RECEIVEDATA;
			*/
			if(!ucResult)
				ucResult = APPCRYPT_OnlineRespDataProcess(NULL,&ComPackRecv[1],ComPackRecvLen-1);
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
						ucResult = EMVERROR_SAVEFILE;
				OSAPP_FileClose(iHandle);
			}
		}
#endif
	}
	
	return ucResult;
	
}

