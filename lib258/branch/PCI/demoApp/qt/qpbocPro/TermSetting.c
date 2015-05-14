#include	<include.h>
#include	<QError.h>
#include	<Global.h>
#include	<Util.h>
#include	<EMVParam.h>

UCHAR	TERMSETTING_ReaderParam(void);


OPTION  const LanguageOption[]=
{
	{1 , "1 - English"},
	{2 , "2 - Chinese"}

};

OPTION  const TransOption[]=
{
	{1 , "1 - QCASH"},
	{2 , "2 - QGOODS"},
	{3 , "3 - SERVICE"},
	{4 , "4 - QCASHBACK"}
		

};

UCHAR	TERMSETTING_DateTime()
{
	UCHAR		aucBuff[20],aucDate[10],aucTime[10];
	UCHAR		ucResult;
	

	memset(aucBuff,0x00,sizeof(aucBuff));
	memset(aucDate,0x00,sizeof(aucDate));
	Os__read_date(aucDate);
	strcpy((char*)aucBuff," OLD:");
	memcpy(aucBuff+strlen((char*)aucBuff),&aucDate[4],2);
	memcpy(aucBuff+strlen((char*)aucBuff),&aucDate[2],2);
	memcpy(aucBuff+strlen((char*)aucBuff),aucDate,2);
	memcpy(aucDate,&aucBuff[5],6);
    Os__clr_display11(255);


    Os__GB2312_display11(0,2,(PUCHAR)"Cur Date:");
    Os__GB2312_display11(1,2,(PUCHAR)"(YYMMDD)");
    Os__GB2312_display11(2,0,aucBuff);
    emitSignal();

	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,6,6,'N',aucDate,NULL)!=KEY_ENTER)	return QERROR_CANCEL;

	

	memset(aucBuff,0x00,sizeof(aucBuff));
	memset(aucTime,0x00,sizeof(aucTime));
	Os__read_time(aucTime);
	strcpy((char*)aucBuff,"  OLD:");
	memcpy(aucBuff+strlen((char*)aucBuff),aucTime,4);
	

    Os__clr_display11(255);

    Os__GB2312_display11(0,2,(PUCHAR)"Cur Time:");
    Os__GB2312_display11(1,2,(PUCHAR)" (hhmm)");
    Os__GB2312_display11(2,0,aucBuff);
     emitSignal();
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,4,4,'N',aucTime,NULL)!=KEY_ENTER) return QERROR_CANCEL;

	
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),&aucDate[4],2);
	memcpy(aucBuff+strlen((char*)aucBuff),&aucDate[2],2);
	memcpy(aucBuff+strlen((char*)aucBuff),aucDate,2);
	Os__write_date(aucBuff);

	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff,aucTime,4);
	memcpy(&aucBuff[4],"00",2); //˰صײ㻷ϴ
	Os__write_time(aucBuff);
	

    Os__clr_display11(255);

    Os__GB2312_display11(2,0,(PUCHAR)"Date&Time Set OK!");
     emitSignal();
	return QERROR_SUCCESS;
	
}


UCHAR	TERMSETTING_TermInfo(void)
{
	UCHAR	aucTerminalID[QTERMINALIDLEN+1],aucMerchantID[QMERCHANTIDLEN+1];
	UCHAR	ucResult;

	ucResult = QERROR_SUCCESS;
	
	

    Os__clr_display11(255);

    Os__GB2312_display11(0,2,"Terminal ID");
     emitSignal();
	memset(aucTerminalID,0x00,sizeof(aucTerminalID));
	memcpy(aucTerminalID,QConstParam.aucTerminalID,QTERMINALIDLEN);
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,QTERMINALIDLEN,QTERMINALIDLEN,'N',aucTerminalID,NULL)!=KEY_ENTER) return QERROR_CANCEL;
	

	

    Os__clr_display11(255);

    Os__GB2312_display11(0,2,"Merchant ID");
    emitSignal();
	memset(aucMerchantID,0x00,sizeof(aucMerchantID));
	memcpy(aucMerchantID,QConstParam.aucMerchantID,QMERCHANTIDLEN);
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,QMERCHANTIDLEN,QMERCHANTIDLEN,'N',aucMerchantID,NULL)!=KEY_ENTER) return QERROR_CANCEL;


	memcpy(QConstParam.aucTerminalID,aucTerminalID,QTERMINALIDLEN);
	memcpy(QConstParam.aucMerchantID,aucMerchantID,QMERCHANTIDLEN);
	strcpy((char*)QConstParam.aucMerchantName,"VISA WAVE TEST MERCHANT");

    ucResult = UTIL_WriteConstParamFile(&QConstParam);

	QERROR_DisplayMsg(ucResult);

	return QERROR_SUCCESS;
	
}

UCHAR	TERMSETTING_LanguageSetlect(void)
{
	POPTION    pOption;
	UCHAR		szTitle[30],ucResult;

	memset(szTitle,0x00,sizeof(szTitle));
	strcpy((char*)szTitle,"  Language Menu [");
	szTitle[strlen((char*)szTitle)]=QConstParam.ucLanguage;
	szTitle[strlen((char*)szTitle)]=']';
	pOption = (POPTION)UTIL_SelectOption(szTitle,ASCII_FONT,(POPTION)LanguageOption,sizeof(LanguageOption)/sizeof(OPTION));
	if(pOption)
		QConstParam.ucLanguage=pOption->ucOptionValue+0x30;
	return ucResult;
	
}

UCHAR	TERMSETTING_TermConfig(void)
{
	UCHAR	ucResult,ucValue,aucBuff[10];

	ucResult = QERROR_SUCCESS;

		
	
	memset(aucBuff,0x00,sizeof(aucBuff));


    Os__clr_display11(255);

    Os__GB2312_display11(0,0,(PUCHAR)"Batch Capture?");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
     emitSignal();
	aucBuff[0]  =QConstParam.bBatchCapture?'1':'0';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01") ==KEY_ENTER)
		QConstParam.bBatchCapture = aucBuff[0]-'0';
	else return QERROR_CANCEL;
		
		
	
	
    Os__clr_display11(255);

    Os__GB2312_display11(0,0,(PUCHAR)"Support PBOC20?");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
     emitSignal();
	aucBuff[0]  =QConstParam.bPBOC20?'1':'0';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
		QConstParam.bPBOC20 = aucBuff[0]-'0';
	else return QERROR_CANCEL;
	
	
    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(PUCHAR)"Communication Port");
    Os__GB2312_display11(1,0,(PUCHAR)"1-COM1  2-COM2");
    Os__GB2312_display11(2,0,(PUCHAR)"3-COM3");
     emitSignal();
	if(QConstParam.ucCommID ==0)
		aucBuff[0] ='1';
	else if(QConstParam.ucCommID ==1)
		aucBuff[0] ='2';
	else
		aucBuff[0] ='3';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"123")==KEY_ENTER)
	{
		QConstParam.ucCommID= aucBuff[0]-'1';
	}
	else return QERROR_CANCEL;	
	

    Os__clr_display11(255);

    Os__GB2312_display11(0,0,(PUCHAR)"Reader Port");
    Os__GB2312_display11(1,0,(PUCHAR)"1-COM1 2-COM2");
    Os__GB2312_display11(2,0,(PUCHAR)"3-COM3");
     emitSignal();
	if(QConstParam.ucReaderPort==0)
		aucBuff[0] ='1';
	else if(QConstParam.ucReaderPort ==1)
		aucBuff[0] ='2';
	else
		aucBuff[0] ='3';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"123")==KEY_ENTER)
	{
		QConstParam.ucReaderPort= aucBuff[0]-'1';
	}
	else return QERROR_CANCEL;	
	

    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(PUCHAR)"Ticket Print");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
     emitSignal();
	if(!QConstParam.bPrintTicket)
		aucBuff[0] ='0';
	else
		aucBuff[0] ='1';
	
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
	{
		if(aucBuff[0] =='1')
			QConstParam.bPrintTicket= TRUE;
		else
			QConstParam.bPrintTicket= FALSE;
	}
	else return QERROR_CANCEL;	


    Os__clr_display11(255);

    Os__GB2312_display11(0,0,(PUCHAR)"Auto Remove");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
     emitSignal();
	if(!QConstParam.bAutoRemove)
		aucBuff[0] ='0';
	else
		aucBuff[0] ='1';
	
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
	{
		if(aucBuff[0] =='1')
			QConstParam.bAutoRemove= TRUE;
		else
			QConstParam.bAutoRemove= FALSE;
	}
	else return QERROR_CANCEL;	


    Os__clr_display11(255);

    Os__GB2312_display11(0,0,(PUCHAR)"ICC Debug");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
     emitSignal();
	if(!QConstParam.bICCDebug)
		aucBuff[0] ='0';
	else
		aucBuff[0] ='1';
	
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
	{
		if(aucBuff[0] =='1')
			QConstParam.bICCDebug= TRUE;
		else
			QConstParam.bICCDebug= FALSE;
	}
	else return QERROR_CANCEL;	


    Os__clr_display11(255);

    Os__GB2312_display11(0,0,(PUCHAR)"Digital Debug");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
     emitSignal();
	if(!QConstParam.bDigitalDebug)
		aucBuff[0] ='0';
	else
		aucBuff[0] ='1';
	
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
	{
		if(aucBuff[0] =='1')
			QConstParam.bDigitalDebug= TRUE;
		else
			QConstParam.bDigitalDebug= FALSE;
	}
	else return QERROR_CANCEL;	
	
	if(!ucResult)
	{
		if(QConstParam.bDDOL)
		{
			memcpy(QConfig.aucDefaultDOL,"\x9F\x37\x04",3);
			QConfig.ucDefaultDOLLen = 0x03;
		}
		else QConfig.ucDefaultDOLLen = 0x00;

		if(QConstParam.bDTOL)
		{
			memcpy(QConfig.aucDefaultTDOL,"\x9F\x02\x06\x5F\x2A\x02\x9A\x03\x9C\x01\x95\x05\x9F\x37\x04",15);
			QConfig.ucDefaultTDOLLen = 15;
		}
		else	QConfig.ucDefaultTDOLLen = 0x00;
		
		if(QConstParam.bPBOC20)
		{
			QConfig.aucTermCapab[1] &=~QTERMCAPAB_ENCIPHEREDOFFLINEPIN;
			QConfig.aucTermCapab[1] |=QTERMCAPAB_IDCARD;
		}
		ucResult = UTIL_WriteEMVConfigFile(&QConfig);
		if(!ucResult)
			ucResult = UTIL_WriteConstParamFile(&QConstParam);
	}
	
	QERROR_DisplayMsg(ucResult);

	return ucResult;

}

UCHAR	TERMSETTING_TransSetlect(void)
{
	POPTION    pOption;
	UCHAR		szTitle[30],ucResult;

	memset(szTitle,0x00,sizeof(szTitle));
	strcpy((char*)szTitle,"   Trans   Menu   [");
	szTitle[strlen((char*)szTitle)]=QConstParam.ucTransType;
	szTitle[strlen((char*)szTitle)]=']';
	pOption = (POPTION)UTIL_SelectOption(szTitle,ASCII_FONT,(POPTION)TransOption,sizeof(TransOption)/sizeof(OPTION));
	if(pOption)
	{
		QConstParam.ucTransType=pOption->ucOptionValue+0x30;
		ucResult = UTIL_WriteConstParamFile(&QConstParam);
	}
	else ucResult = QERROR_CANCEL;
	
	QERROR_DisplayMsg(ucResult);
	return ucResult;

	
}



UCHAR	TERMSETTING_EMVTransParam(void)
{
	UCHAR	ucResult,aucBuff[40],ucTermType,aucTermCapab[QTERMCAPABLEN],aucTermAddCapab[QTERMADDCAPABLEN];
	UINT	uiFloorLimit,uiThreshold;
	UCHAR	ucTP,ucMTP,aucTermCountryCode[QTERMCOUNTRYCODELEN],aucTransCurCode[QTRANSCURRENCYCODELEN],aucAppVerNum[QAPPVERNUMBERLEN];
	BOOL	bSupportPSE;
	
	ucResult = QERROR_SUCCESS;

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"Terminal Type");
        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:   ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),&QConfig.ucTermType,2);

        Os__GB2312_display11(1,0,aucBuff);

        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,&QConfig.ucTermType,2);
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,2,2,'N',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(&ucTermType,sizeof(ucTermType),aucBuff,strlen((char*)aucBuff));
		else ucResult = QERROR_CANCEL;
		
	}

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"PSE Support?");
        Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
         emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		aucBuff[0]  =QConfig.bSupportPSE?'1':'0';
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
			bSupportPSE = aucBuff[0]-'0';
		else return QERROR_CANCEL;	
	}

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"Terminal Capab");
         emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:      ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),QConfig.aucTermCapab,QTERMCAPABLEN*2);

        Os__GB2312_display11(1,0,aucBuff);
        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,QConfig.aucTermCapab,QTERMCAPABLEN*2);
		if(UTIL_Input(RIGHT_ALIGN,3,TRUE,QTERMCAPABLEN*2,QTERMCAPABLEN*2,'H',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(aucTermCapab,QTERMCAPABLEN,aucBuff,QTERMCAPABLEN*2);
		else ucResult = QERROR_CANCEL;
		
	}

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"Terminal AddCapab");
         emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),QConfig.aucTermAddCapab,QTERMADDCAPABLEN*2);


        Os__GB2312_display11(1,0,aucBuff);
         emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,QConfig.aucTermAddCapab,QTERMADDCAPABLEN*2);
		if(UTIL_Input(RIGHT_ALIGN,3,TRUE,QTERMADDCAPABLEN*2,QTERMADDCAPABLEN*2,'H',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(aucTermAddCapab,QTERMADDCAPABLEN,aucBuff,QTERMADDCAPABLEN*2);
		else ucResult = QERROR_CANCEL;
		
	}

	if(!ucResult)
	{

        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"Country Code");
        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
        CONV_HexAsc(aucBuff+strlen((char*)aucBuff),QConfig.aucTermCountryCode,QTERMCOUNTRYCODELEN*2);

        Os__GB2312_display11(1,0,aucBuff);
       emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,QConfig.aucTermCountryCode,QTERMCOUNTRYCODELEN*2);
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,QTERMCOUNTRYCODELEN*2,QTERMCOUNTRYCODELEN*2,'N',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(aucTermCountryCode,QTERMCOUNTRYCODELEN,aucBuff,QTERMCOUNTRYCODELEN*2);
		else ucResult = QERROR_CANCEL;
		
	}

	if(!ucResult)
	{

        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"Currency Code");
        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),QConfig.aucTransCurrencyCode,QTRANSCURRENCYCODELEN*2);

        Os__GB2312_display11(1,0,aucBuff);
       emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,QConfig.aucTransCurrencyCode,QTRANSCURRENCYCODELEN*2);
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,QTRANSCURRENCYCODELEN*2,QTRANSCURRENCYCODELEN*2,'N',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(aucTransCurCode,QTRANSCURRENCYCODELEN,aucBuff,QTRANSCURRENCYCODELEN*2);
		else ucResult = QERROR_CANCEL;
		
	}
	
	if(!ucResult)
	{

        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"App Ver");
        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),QConfig.aucAppVerNumber,QAPPVERNUMBERLEN*2);

        Os__GB2312_display11(1,0,aucBuff);
       emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,QConfig.aucAppVerNumber,QAPPVERNUMBERLEN*2);
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,QAPPVERNUMBERLEN*2,QAPPVERNUMBERLEN*2,'H',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(aucAppVerNum,QAPPVERNUMBERLEN,aucBuff,QAPPVERNUMBERLEN*2);
		else ucResult = QERROR_CANCEL;
		
	}
	if(!ucResult)
	{

        Os__clr_display11(255);
        Os__GB2312_display11(0,2,(PUCHAR)"Floor Limit");
       emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD: ");
		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),QConfig.uiFloorLimit,2);
        Os__GB2312_display11(1,0,aucBuff);

       emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiFloorLimit = QConfig.uiFloorLimit;
		ucResult = UTIL_InputAmount(3,(unsigned long*)&uiFloorLimit,0,0xFFFFFF);
	}

	if(!ucResult)
	{
        Os__clr_display11(255);
        Os__GB2312_display11(0,2,(PUCHAR)"  Threshold");
        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD: ");
		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),QConfig.uiThreshold,2);

        Os__GB2312_display11(1,0,aucBuff);
      emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiThreshold = QConfig.uiThreshold;
		ucResult = UTIL_InputAmount(3,(unsigned long*)&uiThreshold,0,uiFloorLimit);
	}

	if(!ucResult)
	{

        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"Traget Percent");
        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
		sprintf(aucBuff+strlen((char*)aucBuff)," %02d",QConfig.ucTragetPercent);

        Os__GB2312_display11(1,0,aucBuff);

        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		sprintf(aucBuff,"%02d",QConfig.ucTragetPercent);
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,2,2,'N',aucBuff,NULL)==KEY_ENTER)
			ucTP = CONV_AscLong(aucBuff,strlen((char*)aucBuff));
		else ucResult = QERROR_CANCEL;
		
	}

	if(!ucResult)
	{

        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"Max Traget Percent");
        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
		sprintf(aucBuff+strlen((char*)aucBuff)," %02d",QConfig.ucMaxTragetPercent);
        Os__GB2312_display11(1,0,aucBuff);

       emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		sprintf(aucBuff,"%02d",QConfig.ucMaxTragetPercent);
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,2,2,'N',aucBuff,NULL)==KEY_ENTER)
			ucMTP = CONV_AscLong(aucBuff,strlen((char*)aucBuff));
		else ucResult = QERROR_CANCEL;
		
	}

	if(!ucResult) 
	{
		QConfig.ucTermType = ucTermType;
		QConfig.bSupportPSE = bSupportPSE;
		memcpy(QConfig.aucTermCapab,aucTermCapab,QTERMCAPABLEN);
		memcpy(QConfig.aucTermAddCapab,aucTermAddCapab,QTERMADDCAPABLEN);
		memcpy(QConfig.aucTermCountryCode,aucTermCountryCode,QTERMCOUNTRYCODELEN);
		memcpy(QConfig.aucTransCurrencyCode,aucTransCurCode,QTRANSCURRENCYCODELEN);
		memcpy(QConfig.aucAppVerNumber,aucAppVerNum,QAPPVERNUMBERLEN);
		QConfig.uiFloorLimit = uiFloorLimit;
		QConfig.uiThreshold = uiThreshold;
		QConfig.ucTragetPercent =ucTP;
		QConfig.ucMaxTragetPercent = ucMTP;
		
		memcpy(QConfig.aucDefaultDOL,"\x9F\x37\x04",3);
		QConfig.ucDefaultDOLLen = 0x03;
		memcpy(QConfig.aucDefaultTDOL,"\x9F\x02\x06\x5F\x2A\x02\x9A\x03\x9C\x01\x95\x05\x9F\x37\x04",15);
		QConfig.ucDefaultTDOLLen = 15;
		ucResult = UTIL_WriteEMVConfigFile(&QConfig);
	}
	if(!ucResult)
	{

        Os__clr_display11(255);
        Os__GB2312_display11(2,0,(PUCHAR)"Param Set OK");
       emitSignal();
		UTIL_GetKey(3);
	}
	else
		QERROR_DisplayMsg(ucResult);
		
	return ucResult;
	
}

UCHAR	TERMSETTING_TAC(void)
{
	QTACCODE		tac;
	UCHAR		ucResult,aucBuff[20],ucSize,ucLen;
	
	memcpy(&tac,&QConfig.TAC,sizeof(QTACCODE));
	ucResult =QERROR_SUCCESS;

	
	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));

        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"Denial Code Exist");
        emitSignal();
		aucBuff[0] =tac.bDenialExist? '1':'0';
		if(UTIL_Input(MIDDLE_ALIGN,1,TRUE,1,1,'N',aucBuff,"01") ==KEY_ENTER)
			tac.bDenialExist = aucBuff[0]-'0';
		else 
			return QERROR_CANCEL;
		
		if(tac.bDenialExist)
		{
			ucSize =sizeof(tac.aucDenial);
            ////*Os__GB2312_display11(3,0,(PUCHAR)"Denial Code");
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"OLD:  ");
			CONV_HexAsc(aucBuff+strlen((char*)aucBuff),tac.aucDenial,ucSize*2);
            Os__GB2312_display11(2,0,aucBuff);

           emitSignal();
			memset(aucBuff,0x00,sizeof(aucBuff));
			CONV_HexAsc(aucBuff,tac.aucDenial,ucSize*2);
			if(UTIL_Input(RIGHT_ALIGN,3,TRUE,ucSize*2,ucSize*2,'H',aucBuff,NULL)==KEY_ENTER)
				CONV_AscHex(&tac.aucDenial,ucSize,aucBuff,ucSize*2);
			else 
				ucResult = QERROR_CANCEL;
		}
	}

	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));

        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"Online Code Exist");
         emitSignal();
		aucBuff[0] =tac.bOnlineExist? '1':'0';
		if(UTIL_Input(MIDDLE_ALIGN,1,TRUE,1,1,'N',aucBuff,"01") ==KEY_ENTER)
			tac.bOnlineExist = aucBuff[0]-'0';
		else 
			return QERROR_CANCEL;
		
		if(tac.bOnlineExist)
		{
			
			ucSize =sizeof(tac.aucOnline);
            ////*Os__GB2312_display11(3,0,(PUCHAR)"Online Code");
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"OLD:  ");
			CONV_HexAsc(aucBuff+strlen((char*)aucBuff),tac.aucOnline,ucSize*2);

            Os__GB2312_display11(2,0,aucBuff);
           emitSignal();
			memset(aucBuff,0x00,sizeof(aucBuff));
			CONV_HexAsc(aucBuff,tac.aucOnline,ucSize*2);
			if(UTIL_Input(RIGHT_ALIGN,3,TRUE,ucSize*2,ucSize*2,'H',aucBuff,NULL)==KEY_ENTER)
				CONV_AscHex(&tac.aucOnline,ucSize,aucBuff,ucSize*2);
			else 
				ucResult = QERROR_CANCEL;
		}
		
	}
	if(!ucResult)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));

        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"Default Code Exist");
       emitSignal();
		aucBuff[0] =tac.bDefaultExist? '1':'0';
		if(UTIL_Input(MIDDLE_ALIGN,1,TRUE,1,1,'N',aucBuff,"01") ==KEY_ENTER)
			tac.bDefaultExist= aucBuff[0]-'0';
		else 
			return QERROR_CANCEL;
		
		if(tac.bDefaultExist)
		{
			ucSize =sizeof(tac.aucDefault);

			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"OLD:  ");
			CONV_HexAsc(aucBuff+strlen((char*)aucBuff),tac.aucDefault,ucSize*2);

            Os__GB2312_display11(2,0,aucBuff);
           emitSignal();
			memset(aucBuff,0x00,sizeof(aucBuff));
			CONV_HexAsc(aucBuff,tac.aucDefault,ucSize*2);
			if(UTIL_Input(RIGHT_ALIGN,3,TRUE,0,ucSize*2,'H',aucBuff,NULL)==KEY_ENTER)
				CONV_AscHex(&tac.aucDefault,ucSize,aucBuff,ucSize*2);
			else 
				ucResult = QERROR_CANCEL;
		}
	}

	if(!ucResult)
	{
		memcpy(&QConfig.TAC,&tac,sizeof(QTACCODE));
		ucResult = UTIL_WriteEMVConfigFile(&QConfig);
	}

	if(!ucResult)
	{

        Os__clr_display11(255);
        Os__GB2312_display11(2,2,(PUCHAR)"TAC Set OK!");
         emitSignal();
		UTIL_GetKey(3);
	}
	else
		QERROR_DisplayMsg(ucResult);
		
	return ucResult;
}
UCHAR	TERMSETTING_Func(void)
{
	do
	{
        Os__clr_display11(255);
		if(QConstParam.bPBOC20==FALSE)
		{    
            Os__GB2312_display11(0,0,"1.Terminal");
            Os__GB2312_display11(1,0,"2.Function");
            Os__GB2312_display11(2,0,"3.Date and Time");
            Os__GB2312_display11(3,0,"4.Default Setting");
		}
		else
		{
            Os__GB2312_display11(0,0,"1.终端设置");
            Os__GB2312_display11(1,0,"2.功能选项");
            Os__GB2312_display11(2,0,"3.日期与时间");
            Os__GB2312_display11(3,0,"4.缺省设置");
		}
        emitSignal();

        switch(Os__xget_key11())
		{
		case '1':
			TERMSETTING_TermInfo();
			break;
		case '2':
			TERMSETTING_TermConfig();
			break;
		case '3':
			TERMSETTING_DateTime();
			break;
		case '4':
			TERMSETTING_LoadDefaultSetting();
			break;
		case KEY_CLEAR:
			return QERROR_SUCCESS;
		default:
			break;
		}
	}while(1);
	
}

UCHAR	TERMSETTING_TransParam(void)
{
	do
	{
        Os__clr_display11(255);
		if(QConstParam.bPBOC20==FALSE)
		{
            Os__GB2312_display11(1,0,"1.EMV Param");
            Os__GB2312_display11(2,0,"2.Reader Param");
		}
		else
		{

            Os__GB2312_display11(1,0,"1.EMV参数");
            Os__GB2312_display11(2,0,"2.READER参数");
		}

         emitSignal();
        switch(Os__xget_key11())
		{
		case '1':
			TERMSETTING_EMVTransParam();
			break;
		case '2':
			TERMSETTING_ReaderParam();
			break;
			/*
		case '4':
			TERMSETTING_LoadDefault();
			break;
			*/
		case KEY_CLEAR:
			return QERROR_SUCCESS;
		default:
			break;
		}
	}while(1);
	
}

void	TERMSETTING_LoadDefaultSetting(void)
{
	UCHAR	ucI,ucResult;

	
	memset(&QConfig,0x00,sizeof(QTERMCONFIG));
	memset(&QConstParam,0x00,sizeof(QEMVCONSTPARAM));

	
	
	for(ucI =0;ucI<19;ucI++)
		memcpy(&QTermCAPK[ucI],&TermSupportCAPKDefault[ucI],sizeof(QCAPK));
	ucQTermCAPKNum =19;

	for(ucI =0;ucI<4;ucI++)
       {
		memcpy(&QTermAID[ucI],&TermSupportAppDefault[ucI],sizeof(QTERMSUPPORTAPP));
        memcpy(&QTermAID[ucI].QReaderParam,&ConstReaderDefault,sizeof(QREADERPARAM));
    }
	ucQTermAIDNum=4;

	ucQIPKRevokeNum=0;
	ucQExceptFileNum =0;
	
	memcpy(&QConfig,&TermConfigDefault,sizeof(QTERMCONFIG));
	QConfig.pfnQICC = QICC;
	QConfig.pfnOfflinePIN = UTIL_OfflinePIN;
	QConfig.pfnOnlinePIN = UTIL_OnlinePIN;
	QConfig.pfnEMVInformMsg =UTIL_EMVInformMsg;
	memcpy(&QConstParam,&ConstParamDefault,sizeof(QEMVCONSTPARAM));
	memcpy(&QReaderParam,&ConstReaderDefault,sizeof(QREADERPARAM));
	UTIL_DeleteAllData((PUCHAR)CONSTPARAMFILE);
	ucResult = UTIL_WriteConstParamFile(&QConstParam);
	if(!ucResult)
	{
		UTIL_DeleteAllData((PUCHAR)EMVCONFIGFILE);
		ucResult = UTIL_WriteEMVConfigFile(&QConfig);
	}

	if(!ucResult)
		ucResult = UTIL_SaveCAPKFile();
	if(!ucResult)
		ucResult = UTIL_SaveAIDFile();
	if(!ucResult)
		ucResult = UTIL_SaveExceptFile();
	if(!ucResult)
		ucResult = UTIL_SaveIPKRevokeFile();
	if(!ucResult)
		ucResult = UTIL_WriteReaderParamFile(&QReaderParam);
	if(!ucResult)
		ucResult = UTIL_DeleteAllData((PUCHAR)TRANSLOG);
	if(!ucResult)
		ucResult = UTIL_DeleteAllData((PUCHAR)BATCHRECORD);
	
	if(!ucResult)
	{

        Os__clr_display11(255);
        Os__GB2312_display11(2,0,"Default Load OK!");
         emitSignal();
		UTIL_GetKey(3);
	}
	else 
		QERROR_DisplayMsg(ucResult);
		
	return ;
}
/*
typedef struct
{
	BOOL		bRCFLExist;				//Reader Contactless Floor Limit Exist Flag
	ULONG		ulRCFLimit;				//Reader Contactless Floor Limit
	BOOL		bRCTLExist;				//Reader Contactless Transaction Limit  Exist Flag
	ULONG		ulRCTLimit;				//Reader Contactless Transaction Limit
	BOOL		bRCVReqLExist;				//Reader CVM Required Limit  Exist Flag
	ULONG		ulRCVMReqLimit;			//Reader CVM Required Limit
	UCHAR		aucTermTransQ[4];			//Terminal Transaction Qualifiers
	ULONG		ulVLPAFunds;				//VLP Available Funds
	ULONG		ulVLPFLimit;				//VLP Funds Limit
	ULONG		ulVLPRThreshold;			//VLP Reset Threshold
	ULONG		ulVLPSTransLimit;			//VLP Single Transaction Limit
	BOOL		bStatusCheck;				//Status Check Flag
}QREADERPARAM;
*/

UCHAR	TERMSETTING_ReaderParam(void)
{
	unsigned char aucBuff[30],ucResult;
	QREADERPARAM	  rm;
	
	ucResult = QERROR_SUCCESS;

	memcpy(&rm,&QReaderParam,sizeof(QREADERPARAM));
	
	if(!ucResult)
	{
        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"RCFL EXIST?");			//Reader Contactless Floor Limit Exist Flag
        Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		aucBuff[0]  =rm.bRCFLExist?'1':'0';
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
			rm.bRCFLExist = aucBuff[0]-'0';
		else return QERROR_CANCEL;	
	}

	if(!ucResult&&rm.bRCFLExist)					//Reader Contactless Floor Limit
	{		
        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"RCFLimit Value");
		memset(aucBuff,0x00,sizeof(aucBuff));
		sprintf(aucBuff,"%d.%02d",rm.ulRCFLimit/100,rm.ulRCFLimit%100);
        Os__GB2312_display11(1,0,aucBuff);
         emitSignal();
		ucResult = UTIL_InputAmount(3,(unsigned long*)&rm.ulRCFLimit,0,0xFFFFFF);
	}

	if(!ucResult)
	{	
        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"RCTL Exist?");					//Reader Contactless Transaction Limit  Exist Flag
        Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
         emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		aucBuff[0]  =rm.bRCTLExist?'1':'0';
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
			rm.bRCTLExist = aucBuff[0]-'0';
		else return QERROR_CANCEL;	
	}

	if(!ucResult&&rm.bRCTLExist)
	{
        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"RCTLimit Value");				//Reader Contactless Transaction Limit
		memset(aucBuff,0x00,sizeof(aucBuff));
		sprintf(aucBuff,"%d.%02d",rm.ulRCTLimit/100,rm.ulRCTLimit%100);
        Os__GB2312_display11(1,0,aucBuff);
         emitSignal();

		
		ucResult = UTIL_InputAmount(3,(unsigned long*)&rm.ulRCTLimit,0,0xFFFFFF);
	}

	if(!ucResult)
	{
        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"CVM Limit EXIST?");					//Reader CVM Required Limit  Exist Flag
        Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
		memset(aucBuff,0x00,sizeof(aucBuff));
		aucBuff[0]  =rm.bRCVReqLExist?'1':'0';
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
			rm.bRCVReqLExist = aucBuff[0]-'0';
		else return QERROR_CANCEL;	
	}

	if(!ucResult&&rm.bRCVReqLExist)
	{
        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"CVM Limit Value");				//Reader Contactless Transaction Limit
		memset(aucBuff,0x00,sizeof(aucBuff));
		sprintf(aucBuff,"%d.%02d",rm.ulRCVMReqLimit/100,rm.ulRCVMReqLimit%100);

        Os__GB2312_display11(1,0,aucBuff);
		ucResult = UTIL_InputAmount(3,(unsigned long*)&rm.ulRCVMReqLimit,0,0xFFFFFF);
	}

	if(!ucResult)
	{
        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"Status Check?");					//Status Check Flag
        Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
		memset(aucBuff,0x00,sizeof(aucBuff));
		aucBuff[0]  =rm.bStatusCheck?'1':'0';
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
			rm.bStatusCheck = aucBuff[0]-'0';
		else return QERROR_CANCEL;	
	}
	
	if(!ucResult)
	{
        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(PUCHAR)"TTQ Value");					//Terminal Transaction Qualifiers
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:    ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),rm.aucTermTransQ,sizeof(rm.aucTermTransQ)*2);
        Os__GB2312_display11(1,0,aucBuff);

		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,rm.aucTermTransQ,sizeof(rm.aucTermTransQ)*2);
		if(UTIL_Input(RIGHT_ALIGN,3,TRUE,8,8,'H',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(rm.aucTermTransQ,4,aucBuff,8);
		else ucResult = QERROR_CANCEL;
	}
	
	if(!ucResult) 
	{
		memcpy(&QReaderParam,&rm,sizeof(QREADERPARAM));
		ucResult = UTIL_WriteReaderParamFile(&QReaderParam);
	}
	if(!ucResult)
	{
        Os__clr_display11(255);
        Os__GB2312_display11(2,0,(PUCHAR)"READER Param OK");
         emitSignal();
		UTIL_GetKey(3);
	}
	else
		QERROR_DisplayMsg(ucResult);
	return ucResult;
	
}

