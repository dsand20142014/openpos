#include	<test/include.h>
#include	<EMVError.h>
#include	<test/Global.h>
#include	<test/Util.h>
#include	<test/EMVParam.h>
#include "tools.h"


OPTION  const LanguageOption[]=
{
	{1 , "1 - English"},
	{2 , "2 - Chinese"}
};

OPTION  const TransOption[]=
{
	{1 , "1 - CASH"},
	{2 , "2 - GOODS"},
	{3 , "3 - SERVICE"},
	{4 , "4 - CASHBACK"}
};

OPTION  const KernelOption[]=
{
	{1 , "1 - X"},
	{2 , "2 - X and Y"},
	{3 , "3 - X per AID"},
	{4 , "4 - X and Y per AID"}
		

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



	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,6,6,'N',aucDate,NULL)!=KEY_ENTER)	return EMVERROR_CANCEL;

	

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


	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,4,4,'N',aucTime,NULL)!=KEY_ENTER) return EMVERROR_CANCEL;

	
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),&aucDate[4],2);
	memcpy(aucBuff+strlen((char*)aucBuff),&aucDate[2],2);
	memcpy(aucBuff+strlen((char*)aucBuff),aucDate,2);
	Os__write_date(aucBuff);
     system("sync");

	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff,aucTime,4);
	memcpy(&aucBuff[4],"00",2); //��˰�صײ㻷�������������ϴ���
	Os__write_time(aucBuff);
    system("sync");

    Os__clr_display11(255);
    Os__GB2312_display11(2,0,(PUCHAR)"Date&Time Set OK!");

    emitSignal();


	return EMVERROR_SUCCESS;
	
}


UCHAR	TERMSETTING_TermInfo(void)
{
	UCHAR	aucTerminalID[TERMINALIDLEN+1],aucMerchantID[MERCHANTIDLEN+1];
	UCHAR	ucResult;

	ucResult = EMVERROR_SUCCESS;
	
	
    Os__clr_display11(255);
    Os__GB2312_display11(0,2,"Terminal ID");
    emitSignal();

	memset(aucTerminalID,0x00,sizeof(aucTerminalID));
	memcpy(aucTerminalID,ConstParam.aucTerminalID,TERMINALIDLEN);
    if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,TERMINALIDLEN,TERMINALIDLEN,'N',aucTerminalID,NULL)!=KEY_ENTER)
        return EMVERROR_CANCEL;
	

	
    Os__clr_display11(255);
    Os__GB2312_display11(0,2,"Merchant ID");
    emitSignal();

	memset(aucMerchantID,0x00,sizeof(aucMerchantID));
	memcpy(aucMerchantID,ConstParam.aucMerchantID,MERCHANTIDLEN);
    if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,MERCHANTIDLEN,MERCHANTIDLEN,'N',aucMerchantID,NULL)!=KEY_ENTER)
        return EMVERROR_CANCEL;


	memcpy(ConstParam.aucTerminalID,aucTerminalID,TERMINALIDLEN);
	memcpy(ConstParam.aucMerchantID,aucMerchantID,MERCHANTIDLEN);
	strcpy((char*)ConstParam.aucMerchantName,"EMV41 TEST MERCHANT");
	ucResult = UTIL_WriteConstParamFile(&ConstParam);

	EMVERROR_DisplayMsg(ucResult);
	
		
	return EMVERROR_SUCCESS;
	
}

UCHAR	TERMSETTING_LanguageSetlect(void)
{
	POPTION    pOption;
	UCHAR		szTitle[30],ucResult;

	memset(szTitle,0x00,sizeof(szTitle));
	strcpy((char*)szTitle,"  Language Menu [");
	szTitle[strlen((char*)szTitle)]=ConstParam.ucLanguage;
	szTitle[strlen((char*)szTitle)]=']';
	pOption = (POPTION)UTIL_SelectOption(szTitle,ASCII_FONT,(POPTION)LanguageOption,sizeof(LanguageOption)/sizeof(OPTION));
	if(pOption)
		ConstParam.ucLanguage=pOption->ucOptionValue+0x30;
	return ucResult;
	
}

UCHAR	TERMSETTING_TermConfig(void)
{
	UCHAR	ucResult,ucValue,aucBuff[10];

	ucResult = EMVERROR_SUCCESS;

	memset(aucBuff,0x00,sizeof(aucBuff));
    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(PUCHAR)"Force Online?");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");

    emitSignal();

	aucBuff[0] =ConstParam.bForceOnline? '1':'0';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01") ==KEY_ENTER)
		ConstParam.bForceOnline = aucBuff[0]-'0';
	else return EMVERROR_CANCEL;
		
	
	

    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(PUCHAR)"Batch Capture?");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");


    emitSignal();



	aucBuff[0]  =ConstParam.bBatchCapture?'1':'0';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01") ==KEY_ENTER)
		ConstParam.bBatchCapture = aucBuff[0]-'0';
	else return EMVERROR_CANCEL;
		
		
	
	
    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(PUCHAR)"Support PBOC20?");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
    emitSignal();



	aucBuff[0]  =ConstParam.bPBOC20?'1':'0';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
		ConstParam.bPBOC20 = aucBuff[0]-'0';
	else return EMVERROR_CANCEL;
	

	
    Os__clr_display11(255);

    Os__GB2312_display11(0,0,(PUCHAR)"Select Account?");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
    emitSignal();



	aucBuff[0] =ConstParam.bSelectAccount?'1':'0';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
		ConstParam.bSelectAccount = aucBuff[0]-'0';
	else return EMVERROR_CANCEL;	
	
/*	
  Os__clr_display11(255);
    //Os__GB2312_display11(0,0,(PUCHAR)"Show Rand Num?");
    //Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
	aucBuff[0] =ConstParam.bShowRandNum?'1':'0';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
		ConstParam.bShowRandNum = aucBuff[0]-'0';
	else return EMVERROR_CANCEL;		
	
*/
    Os__clr_display11(255);

    Os__GB2312_display11(0,0,(PUCHAR)"Issuer Referral?");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
    emitSignal();



	aucBuff[0] =ConstParam.bIssuerReferal?'1':'0';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
		ConstParam.bIssuerReferal = aucBuff[0]-'0';
	else return EMVERROR_CANCEL;		

		
    Os__clr_display11(255);

    Os__GB2312_display11(0,0,(PUCHAR)"Terminal DDOL?");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
    emitSignal();


	aucBuff[0]  =ConstParam.bDDOL?'1':'0';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
		ConstParam.bDDOL = aucBuff[0]-'0';
	else return EMVERROR_CANCEL;	

    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(PUCHAR)"Terminal DTOL?");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");


    emitSignal();


	aucBuff[0]  =ConstParam.bDTOL?'1':'0';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
		ConstParam.bDTOL = aucBuff[0]-'0';
	else return EMVERROR_CANCEL;	
	
    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(PUCHAR)"Port Select");
    Os__GB2312_display11(1,0,(PUCHAR)"(1-COM1 2-COM2)");

    emitSignal();


	if(ConstParam.ucCommID ==0)
		aucBuff[0] ='1';
	else
		aucBuff[0] ='2';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"12")==KEY_ENTER)
	{
		ConstParam.ucCommID= aucBuff[0]-'1';
	}
	else return EMVERROR_CANCEL;	
	

    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(PUCHAR)"Ticket Print");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");

    emitSignal();

	if(!ConstParam.bPrintTicket)
		aucBuff[0] ='0';
	else
		aucBuff[0] ='1';
	
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
	{
		if(aucBuff[0] =='1')
			ConstParam.bPrintTicket= TRUE;
		else
			ConstParam.bPrintTicket= FALSE;
	}
	else return EMVERROR_CANCEL;
	

    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(PUCHAR)"Support Debug?");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
    emitSignal();

	if(!ConstParam.bDEBUG)
		aucBuff[0] ='0';
	else
		aucBuff[0] ='1';
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
	{
		if(aucBuff[0] =='1')
			ConstParam.bDEBUG= TRUE;
		else
			ConstParam.bDEBUG= FALSE;
	}
	else return EMVERROR_CANCEL;
	
	

    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(PUCHAR)"EC Support");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");
    emitSignal();


	if(!ConstParam.bECSupport)
		aucBuff[0] ='0';
	else
		aucBuff[0] ='1';
	
	if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
	{
		if(aucBuff[0] =='1')
			ConstParam.bECSupport= TRUE;
		else
			ConstParam.bECSupport= FALSE;
	}	
	else return EMVERROR_CANCEL;	
	

    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(PUCHAR)"Auto Support");
    Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");

    emitSignal();


     if(!ConstParam.bAutoSupport)
         aucBuff[0] ='0';
     else
         aucBuff[0] ='1';

     if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,2,'N',aucBuff,"01")==KEY_ENTER)
     {
         if(aucBuff[0] =='1'){
             ConstParam.bAutoSupport= TRUE;

              unsigned char delayTime[15];
              memset(delayTime,0x00,sizeof(delayTime));
             Os__clr_display11(255);
             Os__GB2312_display11(0,0,(PUCHAR)"Delay time");

            emitSignal();

              printf("11~~~~~~~ConstParam.uiDelayTime = %d \n",ConstParam.uiDelayTime);

              CONV_LongAsc(delayTime,1,&ConstParam.uiDelayTime);

              printf("~~~delayTime = %s \n",delayTime);

              if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',delayTime,NULL)==KEY_ENTER){
                  printf("@@@@@@@ delayTime = %s \n",delayTime);

                   ConstParam.uiDelayTime = CONV_AscLong(delayTime,1);
                   printf("@@@@@@~~~~~~~ConstParam.uiDelayTime = %d \n",ConstParam.uiDelayTime);

              }
              else
                  return EMVERROR_CANCEL;


              UINT transAmount = 0;
              Os__clr_display11(255);
              Os__GB2312_display11(0,0,(PUCHAR)"Transtion Amount");

              emitSignal();
              printf("00~~~~~~ConstParam.uiTransAmount = %d \n", ConstParam.uiTransAmount);

              if(!UTIL_InputAmount(3,&transAmount,1,9999999999)){
                  ConstParam.uiTransAmount = transAmount;
                  printf("11~~~~~~~ConstParam.uiTransAmount = %d \n", ConstParam.uiTransAmount);

              }
              else
                  return EMVERROR_CANCEL;

//              UINT otherAmount = 0;
//              Os__clr_display11(255);
//              Os__GB2312_display11(0,0,(PUCHAR)"Other Amount");

//              emitSignal();


//              if(!UTIL_InputAmount(3,&otherAmount,1,9999999999))
//                 ConstParam.uiOtherAmount = otherAmount;
//               else
//                  return EMVERROR_CANCEL;

              char pinNum[15];
              Os__clr_display11(255);
              Os__GB2312_display11(0,0,(PUCHAR)"Pin Password");

              emitSignal();

              memset(pinNum,0x00,sizeof(pinNum));
              memcpy(pinNum,ConstParam.ucPinNum,15);
              if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,6,6,'p',pinNum,NULL)==KEY_ENTER)
                  memcpy(ConstParam.ucPinNum,pinNum,15);
               else
                  return EMVERROR_CANCEL;

         }
         else
             ConstParam.bAutoSupport= FALSE;
     }
     else return EMVERROR_CANCEL;


	if(!ucResult)
	{
		if(ConstParam.bDDOL)
		{
			memcpy(EMVConfig.aucDefaultDOL,"\x9F\x37\x04",3);
			EMVConfig.ucDefaultDOLLen = 0x03;
		}
		else EMVConfig.ucDefaultDOLLen = 0x00;

		if(ConstParam.bDTOL)
		{
			memcpy(EMVConfig.aucDefaultTDOL,"\x9F\x02\x06\x5F\x2A\x02\x9A\x03\x9C\x01\x95\x05\x9F\x37\x04",15);
			EMVConfig.ucDefaultTDOLLen = 15;
		}
		else	EMVConfig.ucDefaultTDOLLen = 0x00;
		
		if(ConstParam.bPBOC20)
		{
			EMVConfig.aucTermCapab[1] &=~TERMCAPAB_ENCIPHEREDOFFLINEPIN;
			EMVConfig.aucTermCapab[1] |=TERMCAPAB_IDCARD;
		}
		ucResult = UTIL_WriteEMVConfigFile(&EMVConfig);
        printf("@123@@@@ucResult  = %02x \n",ucResult);
        if(!ucResult){
			ucResult = UTIL_WriteConstParamFile(&ConstParam);
            printf("~~132~~~~~~ ucResult  = %02x\n",ucResult);
        }
	}

	EMVERROR_DisplayMsg(ucResult);
	return ucResult;

}

UCHAR	TERMSETTING_TransSetlect(void)
{
	POPTION    pOption;
	UCHAR		szTitle[30],ucResult;

	memset(szTitle,0x00,sizeof(szTitle));
    strcpy((char*)szTitle,"   Trans   Menu   [");
	szTitle[strlen((char*)szTitle)]=ConstParam.ucTransType;
	szTitle[strlen((char*)szTitle)]=']';
	pOption = (POPTION)UTIL_SelectOption(szTitle,ASCII_FONT,(POPTION)TransOption,sizeof(TransOption)/sizeof(OPTION));
	if(pOption)
	{
		ConstParam.ucTransType=pOption->ucOptionValue+0x30;
		ucResult = UTIL_WriteConstParamFile(&ConstParam);
	}
	else ucResult = EMVERROR_CANCEL;
	
	EMVERROR_DisplayMsg(ucResult);
	return ucResult;

	
}

UCHAR	TERMSETTING_EMVTransParam(void)
{
	UCHAR	ucResult,aucBuff[40],ucTermType,aucTermCapab[TERMCAPABLEN],aucTermAddCapab[TERMADDCAPABLEN];
	UINT	uiFloorLimit,uiThreshold;
	UCHAR	ucTP,ucMTP,aucTermCountryCode[TERMCOUNTRYCODELEN],aucTransCurCode[TRANSCURRENCYCODELEN],aucAppVerNum[APPVERNUMBERLEN];
	BOOL	bSupportPSE;
	
	ucResult = EMVERROR_SUCCESS;

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"Terminal Type");
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:   ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),&EMVConfig.ucTermType,2);
        Os__GB2312_display11(1,0,aucBuff);

       emitSignal();

		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,&EMVConfig.ucTermType,2);
        unsigned char rec = UTIL_Input(MIDDLE_ALIGN,3,TRUE,2,2,'N',aucBuff,NULL);
        printf("rec  = %02x \n",rec);
        if(rec ==KEY_ENTER)
			CONV_AscHex(&ucTermType,sizeof(ucTermType),aucBuff,strlen((char*)aucBuff));
		else ucResult = EMVERROR_CANCEL;
		
	}

    printf("ucResult===%02x\n",ucResult);
	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"PSE Support?");
        Os__GB2312_display11(1,0,(PUCHAR)"(0-No 1-Yes)");

      emitSignal();


		memset(aucBuff,0x00,sizeof(aucBuff));
        aucBuff[0]  = EMVConfig.bSupportPSE?'1':'0';
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,1,1,'N',aucBuff,"01")==KEY_ENTER)
			bSupportPSE = aucBuff[0]-'0';
		else return EMVERROR_CANCEL;	
	}

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"Terminal Capab");
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:      ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),EMVConfig.aucTermCapab,TERMCAPABLEN*2);
        Os__GB2312_display11(1,0,aucBuff);

      emitSignal();

		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,EMVConfig.aucTermCapab,TERMCAPABLEN*2);
		if(UTIL_Input(RIGHT_ALIGN,3,TRUE,TERMCAPABLEN*2,TERMCAPABLEN*2,'H',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(aucTermCapab,TERMCAPABLEN,aucBuff,TERMCAPABLEN*2);
		else ucResult = EMVERROR_CANCEL;
		
	}

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"Terminal AddCapab");
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),EMVConfig.aucTermAddCapab,TERMADDCAPABLEN*2);
        Os__GB2312_display11(1,0,aucBuff);

       emitSignal();

		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,EMVConfig.aucTermAddCapab,TERMADDCAPABLEN*2);
		if(UTIL_Input(RIGHT_ALIGN,3,TRUE,TERMADDCAPABLEN*2,TERMADDCAPABLEN*2,'H',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(aucTermAddCapab,TERMADDCAPABLEN,aucBuff,TERMADDCAPABLEN*2);
		else ucResult = EMVERROR_CANCEL;
		
	}

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"Country Code");
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),EMVConfig.aucTermCountryCode,TERMCOUNTRYCODELEN*2);

        Os__GB2312_display11(1,0,aucBuff);
        emitSignal();

		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,EMVConfig.aucTermCountryCode,TERMCOUNTRYCODELEN*2);
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,TERMCOUNTRYCODELEN*2,TERMCOUNTRYCODELEN*2,'N',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(aucTermCountryCode,TERMCOUNTRYCODELEN,aucBuff,TERMCOUNTRYCODELEN*2);
		else ucResult = EMVERROR_CANCEL;
		
	}

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"Currency Code");
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),EMVConfig.aucTransCurrencyCode,TRANSCURRENCYCODELEN*2);
        Os__GB2312_display11(1,0,aucBuff);

        emitSignal();

		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,EMVConfig.aucTransCurrencyCode,TRANSCURRENCYCODELEN*2);
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,TRANSCURRENCYCODELEN*2,TRANSCURRENCYCODELEN*2,'N',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(aucTransCurCode,TRANSCURRENCYCODELEN,aucBuff,TRANSCURRENCYCODELEN*2);
		else ucResult = EMVERROR_CANCEL;
		
	}
	
	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"App Ver");
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
		CONV_HexAsc(aucBuff+strlen((char*)aucBuff),EMVConfig.aucAppVerNumber,APPVERNUMBERLEN*2);

        Os__GB2312_display11(1,0,aucBuff);
        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		CONV_HexAsc(aucBuff,EMVConfig.aucAppVerNumber,APPVERNUMBERLEN*2);
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,APPVERNUMBERLEN*2,APPVERNUMBERLEN*2,'H',aucBuff,NULL)==KEY_ENTER)
			CONV_AscHex(aucAppVerNum,APPVERNUMBERLEN,aucBuff,APPVERNUMBERLEN*2);
		else ucResult = EMVERROR_CANCEL;
		
	}
	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,2,(PUCHAR)"Floor Limit");
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD: ");
		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),EMVConfig.uiFloorLimit,2);

        Os__GB2312_display11(1,0,aucBuff);
        emitSignal();
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiFloorLimit = EMVConfig.uiFloorLimit;
		ucResult = UTIL_InputAmount(3,(unsigned long*)&uiFloorLimit,0,0xFFFFFF);
	}

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,2,(PUCHAR)"  Threshold");
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD: ");
		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),EMVConfig.uiThreshold,2);
        Os__GB2312_display11(1,0,aucBuff);

        emitSignal();

		memset(aucBuff,0x00,sizeof(aucBuff));
		uiThreshold = EMVConfig.uiThreshold;
		ucResult = UTIL_InputAmount(3,(unsigned long*)&uiThreshold,0,uiFloorLimit);
	}

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"Traget Percent");
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
		sprintf(aucBuff+strlen((char*)aucBuff)," %02d",EMVConfig.ucTragetPercent);

        Os__GB2312_display11(1,0,aucBuff);
        emitSignal();

		memset(aucBuff,0x00,sizeof(aucBuff));
		sprintf(aucBuff,"%02d",EMVConfig.ucTragetPercent);
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,2,2,'N',aucBuff,NULL)==KEY_ENTER)
			ucTP = CONV_AscLong(aucBuff,strlen((char*)aucBuff));
		else ucResult = EMVERROR_CANCEL;
		
	}

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"Max Traget Percent");
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD:  ");
		sprintf(aucBuff+strlen((char*)aucBuff)," %02d",EMVConfig.ucMaxTragetPercent);

        Os__GB2312_display11(1,0,aucBuff);
        emitSignal();

		memset(aucBuff,0x00,sizeof(aucBuff));
		sprintf(aucBuff,"%02d",EMVConfig.ucMaxTragetPercent);
		if(UTIL_Input(MIDDLE_ALIGN,3,TRUE,2,2,'N',aucBuff,NULL)==KEY_ENTER)
			ucMTP = CONV_AscLong(aucBuff,strlen((char*)aucBuff));
		else ucResult = EMVERROR_CANCEL;
		
	}

	if(!ucResult) 
	{
		EMVConfig.ucTermType = ucTermType;
		EMVConfig.bSupportPSE = bSupportPSE;
		memcpy(EMVConfig.aucTermCapab,aucTermCapab,TERMCAPABLEN);
		memcpy(EMVConfig.aucTermAddCapab,aucTermAddCapab,TERMADDCAPABLEN);
		memcpy(EMVConfig.aucTermCountryCode,aucTermCountryCode,TERMCOUNTRYCODELEN);
		memcpy(EMVConfig.aucTransCurrencyCode,aucTransCurCode,TRANSCURRENCYCODELEN);
		memcpy(EMVConfig.aucAppVerNumber,aucAppVerNum,APPVERNUMBERLEN);
		EMVConfig.uiFloorLimit = uiFloorLimit;
		EMVConfig.uiThreshold = uiThreshold;
		EMVConfig.ucTragetPercent =ucTP;
		EMVConfig.ucMaxTragetPercent = ucMTP;
		
		memcpy(EMVConfig.aucDefaultDOL,"\x9F\x37\x04",3);
		EMVConfig.ucDefaultDOLLen = 0x03;
		memcpy(EMVConfig.aucDefaultTDOL,"\x9F\x02\x06\x5F\x2A\x02\x9A\x03\x9C\x01\x95\x05\x9F\x37\x04",15);
		EMVConfig.ucDefaultTDOLLen = 15;
		ucResult = UTIL_WriteEMVConfigFile(&EMVConfig);
	}
	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(2,0,(PUCHAR)"Param Set OK");
        emitSignal();


		UTIL_GetKey(3);
	}
	else
		EMVERROR_DisplayMsg(ucResult);
	return ucResult;
	
}

UCHAR	TERMSETTING_TAC(void)
{
	TACCODE		tac;
	UCHAR		ucResult,aucBuff[20],ucSize,ucLen;
	
	memcpy(&tac,&EMVConfig.TAC,sizeof(TACCODE));
	ucResult =EMVERROR_SUCCESS;

	
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
			return EMVERROR_CANCEL;
		
		if(tac.bDenialExist)
		{
			ucSize =sizeof(tac.aucDenial);
            ////Os__GB2312_display11(3,0,(PUCHAR)"Denial Code");
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
				ucResult = EMVERROR_CANCEL;
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
			return EMVERROR_CANCEL;
		
		if(tac.bOnlineExist)
		{
			
			ucSize =sizeof(tac.aucOnline);
            ////Os__GB2312_display11(3,0,(PUCHAR)"Online Code");
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
				ucResult = EMVERROR_CANCEL;
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
			return EMVERROR_CANCEL;
		
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
				ucResult = EMVERROR_CANCEL;
		}
	}

	if(!ucResult)
	{
		memcpy(&EMVConfig.TAC,&tac,sizeof(TACCODE));
		ucResult = UTIL_WriteEMVConfigFile(&EMVConfig);
	}

	if(!ucResult)
	{
        Os__clr_display11(255);
        Os__GB2312_display11(2,2,(PUCHAR)"TAC Set OK!");

        emitSignal();

		UTIL_GetKey(3);
	}
	else
		EMVERROR_DisplayMsg(ucResult);
	return ucResult;
}

UCHAR	TERMSETTING_ECLimit(void)
{
	
	UCHAR		ucResult,aucBuff[20],ucSize,ucLen;
	BOOL		bECFLExist;
	UINT		uiECFL;

	bECFLExist = EMVConfig.bECFLExist;
	uiECFL = EMVConfig.uiECFloorLimit;

	ucResult =EMVERROR_SUCCESS;
	
	memset(aucBuff,0x00,sizeof(aucBuff));
    Os__clr_display11(255);

    Os__GB2312_display11(0,0,(PUCHAR)"EC Limit Exist");
    emitSignal();


	aucBuff[0] =bECFLExist? '1':'0';
	if(UTIL_Input(MIDDLE_ALIGN,1,TRUE,1,1,'N',aucBuff,"01") ==KEY_ENTER)
		bECFLExist = aucBuff[0]-'0';
	else 
		return EMVERROR_CANCEL;
	
	if(bECFLExist)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(0,0,(PUCHAR)"EC Floor Limit");
		memset(aucBuff,0x00,sizeof(aucBuff));
		strcpy((char*)aucBuff,"OLD: ");
		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),uiECFL,2);

        Os__GB2312_display11(1,0,aucBuff);
        emitSignal();


		memset(aucBuff,0x00,sizeof(aucBuff));
		ucResult = UTIL_InputAmount(3,(unsigned long*)&uiECFL,0,0xFFFFFF);
		

	}
	if(!ucResult)
	{
		EMVConfig.bECFLExist =bECFLExist;
		EMVConfig.uiECFloorLimit =uiECFL;
		ucResult = UTIL_WriteEMVConfigFile(&EMVConfig);
	}

	if(!ucResult)
	{
        Os__clr_display11(255);

        Os__GB2312_display11(2,0,(PUCHAR)"EC PARAM OK!");
        emitSignal();

		UTIL_GetKey(3);
	}
	else
		EMVERROR_DisplayMsg(ucResult);
	return ucResult;
}


UCHAR	TERMSETTING_Func(void)
{
    UCHAR uKey=0;
   do
    {
        printf("in TERMSETTING_Func \n");
        Os__clr_display11(255);
		if(ConstParam.bPBOC20==FALSE)
		{
            Os__GB2312_display11(0,0,"1.Terminal");
            Os__GB2312_display11(1,0,"2.Function");
            Os__GB2312_display11(2,0,"3.Date and Time");
            Os__GB2312_display11(3,0,"4.Default Setting");


            emitSignal();

		}
		else
		{
            Os__GB2312_display11(0,0,"1.终端设置");
            Os__GB2312_display11(1,0,"2.功能选项");
            Os__GB2312_display11(2,0,"3.日期与时间");
            Os__GB2312_display11(3,0,"4.缺省设置");

            emitSignal();

		}
        printf("222222in TERMSETTING_Func \n");

        uKey = Os__xget_key11();
        printf("ukey===%02x\n",uKey);
        switch(uKey)
        {
        case '1':
            printf("1111111\n");
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
            return EMVERROR_SUCCESS;
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
		if(ConstParam.bPBOC20==FALSE)
		{

            Os__GB2312_display11(0,0,"1.Trans");
            Os__GB2312_display11(1,0,"2.Param");
            Os__GB2312_display11(2,0,"3.TAC");
			if(ConstParam.bECSupport)
                Os__GB2312_display11(3,0,"4.EC LIMIT");

            emitSignal();

		}
		else
		{

            Os__GB2312_display11(0,0,"1.交易类型");
            Os__GB2312_display11(1,0,"2.交易参数");
            Os__GB2312_display11(2,0,"3.TAC");
			if(ConstParam.bECSupport)
                Os__GB2312_display11(3,0,"4.EC LIMIT");

            emitSignal();

		}

        switch(Os__xget_key11())
		{
		case '1':
			TERMSETTING_TransSetlect();
			break;
		case '2':
			TERMSETTING_EMVTransParam();
			break;
		case '3':
			TERMSETTING_TAC();
			break;
		case '4':
			if(ConstParam.bECSupport)
				TERMSETTING_ECLimit();
			break;
		case KEY_CLEAR:
			return EMVERROR_SUCCESS;
		default:
			break;
		}
	}while(1);
	
}

void	TERMSETTING_LoadDefaultSetting(void)
{
	UCHAR	ucI,ucResult;

	
	memset(&EMVConfig,0x00,sizeof(TERMCONFIG));
	memset(&ConstParam,0x00,sizeof(EMVCONSTPARAM));

	
	
	for(ucI =0;ucI<28;ucI++)
		memcpy(&TermCAPK[ucI],&TermSupportCAPKDefault[ucI],sizeof(CAPK));
	ucTermCAPKNum =28;

	for(ucI =0;ucI<14;ucI++)
		memcpy(&TermAID[ucI],&TermSupportAppDefault[ucI],sizeof(TERMSUPPORTAPP));
	ucTermAIDNum=14;

	ucIPKRevokeNum=0;
	ucExceptFileNum =0;
	
	memcpy(&EMVConfig,&TermConfigDefault,sizeof(TERMCONFIG));
	EMVConfig.pfnEMVICC = EMVICC;
	EMVConfig.pfnOfflinePIN = UTIL_OfflinePIN;
	EMVConfig.pfnOnlinePIN = UTIL_OnlinePIN;
	EMVConfig.pfnEMVInformMsg =UTIL_EMVInformMsg;
	memcpy(&ConstParam,&ConstParamDefault,sizeof(EMVCONSTPARAM));
	ConstParam.bPBOC20 = TRUE;
	UTIL_DeleteAllData((PUCHAR)CONSTPARAMFILE);
	ucResult = UTIL_WriteConstParamFile(&ConstParam);
	if(!ucResult)
	{
		UTIL_DeleteAllData((PUCHAR)EMVCONFIGFILE);
		ucResult = UTIL_WriteEMVConfigFile(&EMVConfig);
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
		EMVERROR_DisplayMsg(ucResult);
	return ;
}
