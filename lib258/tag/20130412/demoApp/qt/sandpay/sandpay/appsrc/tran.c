#include <include.h>
#include <global.h>
#include <xdata.h>
#include <stdio.h>

unsigned char TRS_CheckReversal(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char aucBuf[10];
	
	if(RunData.ucComType)
		return SUCCESS;
	if(DataSave0.ReversalISO8583Data.ucValid==DATASAVEVALIDFLAG)
	{
		ISO8583_RestoreISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.ReversalISO8583Data.BackupISO8583Data);
							
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
   		ISO8583_SetBitValue(11,aucBuf,6);
   		UTIL_IncreaseTraceNumber(0);
   		NORMALTRANS_ulTraceNumber = DataSave0.ChangeParamData.ulTraceNumber;
   		
   		ucResult = COMMS_TransSendReceive();
   		if(ucResult==SUCCESS)
   		{
#ifdef GUI_PROJECT
            memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
            ProUiFace.ProToUi.uiLines=1;
            memcpy(ProUiFace.ProToUi.aucLine1,"冲正之后交易继续",32);
            sleep(1);
#else
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"֮׼");
#endif
			Os__xdelay(50);
   			Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0, sizeof(unsigned char));
			ucResult=XDATA_Write_Vaild_File(DataSaveReversalISO8583);
			if(ucResult!=SUCCESS)
				return ucResult;
		}else
			return ucResult;
	}
	return SUCCESS;
}

unsigned char Trans_OnlinePurchase(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen,iBarLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	if(DataSave1.CardTable.ucCountType=='1')
	{
		if(NormalTransData.ucTransType==TRANS_S_ONLINEPURCHASE)
		{
			if(NormalTransData.ucReturnflag==true)
				ISO8583_SetBitValue(3,(uchar *)"476000",6);
			else
				ISO8583_SetBitValue(3,(uchar *)"706000",6);
		}
		if(NormalTransData.ucTransType==TRANS_S_ONLINEPURCHASE_P)
			ISO8583_SetBitValue(3,(uchar *)"196000",6);
		if(NormalTransData.ucTransType==TRANS_S_PAYONLINE)
			ISO8583_SetBitValue(3,(uchar *)"726000",6);
		if(NormalTransData.ucTransType==TRANS_S_MOBILECHARGEONLINE)
			ISO8583_SetBitValue(3,(uchar *)"766000",6);
	}
	else
	{
		if(NormalTransData.ucTransType==TRANS_S_ONLINEPURCHASE)
		{
			if(NormalTransData.ucReturnflag==true)
				ISO8583_SetBitValue(3,(uchar *)"474000",6);
			else
				ISO8583_SetBitValue(3,(uchar *)"704000",6);
		}
		if(NormalTransData.ucTransType==TRANS_S_ONLINEPURCHASE_P)
			ISO8583_SetBitValue(3,(uchar *)"194000",6);
		if(NormalTransData.ucTransType==TRANS_S_PAYONLINE)
			ISO8583_SetBitValue(3,(uchar *)"724000",6);
		if(NormalTransData.ucTransType==TRANS_S_MOBILECHARGEONLINE)
			ISO8583_SetBitValue(3,(uchar *)"764000",6);
	}
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	if(NormalTransData.ucTransType==TRANS_S_ONLINEPURCHASE)
		memset(NormalTransData.aucExpiredDate,0,sizeof(NormalTransData.aucExpiredDate));
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	if(NormalTransData.ucHandFlag)
		aucBuf[1]='1';
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(NormalTransData.ucTransType,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	if(NormalTransData.ucTransType==TRANS_S_PAYONLINE)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		iBarLen=bcd_long(&NormalTransData.ucBarLenth,2);
		bcd_asc(aucBuf,NormalTransData.aucBarCode,iBarLen);
		ISO8583_SetBitValue(33,aucBuf,iBarLen);
	}
	if( EXTRATRANS_uiISO2Len )
	{
	    ISO8583_SetBitValue(35,EXTRATRANS_aucISO2,EXTRATRANS_uiISO2Len);
	}
	if( EXTRATRANS_uiISO3Len )
	{
	    ISO8583_SetBitValue(36,EXTRATRANS_aucISO3,EXTRATRANS_uiISO3Len);
	}
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	// Set bit 47
	ISO8583_SetBitValue(47,(uchar*)"111111",6);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//bit 58 
	memset(aucBuf,0,sizeof(aucBuf));
	ulAmount=NormalTransData.ulAmount-NormalTransData.ulRateAmount;
	long_asc(aucBuf,12,&ulAmount);
	ISO8583_SetBitValue(58,aucBuf,12);
	//Set bit 61
	memset(aucBuf,0,sizeof(aucBuf));
	long_bcd(aucBuf,6,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(61,aucBuf,6);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)	
		return ucResult;
	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/*	
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
			return ucResult;
		NORMALTRANS_ulAmount=asc_long(aucBuf,12);
	   */
		if(NormalTransData.ucTransType==TRANS_S_ONLINEPURCHASE)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			ISO8583_GetBitValue(14,aucBuf,&iLen,sizeof(aucBuf));
			if(memcmp(aucBuf,"\x00\x00\x00",3))
			{
				NormalTransData.aucExpiredDate[0]=0x20;
				asc_bcd(&NormalTransData.aucExpiredDate[1],3,aucBuf,6);
			}
		}	
		if(NormalTransData.ucTransType!=TRANS_S_ONLINEPURCHASE_P)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
			if(ucResult!=SUCCESS)	
				return ucResult;
			RunData.ulValueble=asc_long(aucBuf,12);
			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(5,aucBuf,&iLen,sizeof(aucBuf));
			if(ucResult!=SUCCESS)	
				return ucResult;
			RunData.ulCredit=asc_long(aucBuf,12);

			if(RunData.ulValueble>=RunData.ulCredit)
				NormalTransData.ulAfterEDBalance=RunData.ulValueble-RunData.ulCredit;
			else
			{
				NormalTransData.ulAfterEDBalance=RunData.ulCredit-RunData.ulValueble;
				NormalTransData.ucBalanceFlag=1;
				
			}
		}
		if(ISO8583_GetBitValue(62,aucBuf,&iLen,sizeof(aucBuf))==SUCCESS)
		{
			if(aucBuf[0]==1)
				NormalTransData.ucReturnflag=true;
		}
		
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}

unsigned char Trans_Void(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];
	short 			iLen;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	if(DataSave1.CardTable.ucCountType=='1')
	{
		if(NormalTransData.ucTransType==TRANS_S_VOID)
			ISO8583_SetBitValue(3,(uchar *)"716000",6);
		if(NormalTransData.ucTransType==TRANS_S_VOIDPAYONLINE)
			ISO8583_SetBitValue(3,(uchar *)"736000",6);
		if(NormalTransData.ucTransType==TRANS_S_VOID_P)
			ISO8583_SetBitValue(3,(uchar *)"206000",6);

	}
	else
	{
		if(NormalTransData.ucTransType==TRANS_S_VOID)
			ISO8583_SetBitValue(3,(uchar *)"714000",6);
		if(NormalTransData.ucTransType==TRANS_S_VOIDPAYONLINE)
			ISO8583_SetBitValue(3,(uchar *)"734000",6);
		if(NormalTransData.ucTransType==TRANS_S_VOID_P)
			ISO8583_SetBitValue(3,(uchar *)"204000",6);
	}
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	if(NormalTransData.ucTransType==TRANS_S_VOID)
	{
		memset(NormalTransData.aucExpiredDate,0,sizeof(NormalTransData.aucExpiredDate));
	}
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	if(NormalTransData.ucHandFlag)
		aucBuf[1]='1';
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 inputmodel
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 inputmodel
	if(NormalTransData.ucTransType==TRANS_S_VOID)
		STL_GetTotalDomain(TRANS_S_ONLINEPURCHASE,aucBuf);
	if(NormalTransData.ucTransType==TRANS_S_VOIDPAYONLINE)
		STL_GetTotalDomain(TRANS_S_PAYONLINE,aucBuf);
	if(NormalTransData.ucTransType==TRANS_S_VOID_P)
		STL_GetTotalDomain(TRANS_S_ONLINEPURCHASE_P,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	if( EXTRATRANS_uiISO2Len )
	{
	    ISO8583_SetBitValue(35,EXTRATRANS_aucISO2,EXTRATRANS_uiISO2Len);
	}
	if( EXTRATRANS_uiISO3Len )
	{
	    ISO8583_SetBitValue(36,EXTRATRANS_aucISO3,EXTRATRANS_uiISO3Len);
	}
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//Set bit 57
	 memset(aucBuf,0,sizeof(aucBuf));
	asc_bcd(aucBuf,4,DataSave0.ConstantParamData.aucTerminalID,8);
	memcpy(&aucBuf[4],DataSave0.ChangeParamData.aucBatchNumber,12);
	long_bcd(&aucBuf[16],3,&RunData.ulOldTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	

		if(NormalTransData.ucTransType!=TRANS_S_VOID_P)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
			if(ucResult!=SUCCESS)	
				return ucResult;
			RunData.ulValueble=asc_long(aucBuf,12);
			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(5,aucBuf,&iLen,sizeof(aucBuf));
			if(ucResult!=SUCCESS)	
				return ucResult;
			RunData.ulCredit=asc_long(aucBuf,12);

			if(RunData.ulValueble>=RunData.ulCredit)
				NormalTransData.ulAfterEDBalance=RunData.ulValueble-RunData.ulCredit;
			else
			{
				NormalTransData.ulAfterEDBalance=RunData.ulCredit-RunData.ulValueble;
				NormalTransData.ucBalanceFlag=1;
			}
		}
		
		if(NormalTransData.ucTransType==TRANS_S_VOID)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			ISO8583_GetBitValue(14,aucBuf,&iLen,sizeof(aucBuf));
			if(memcmp(aucBuf,"\x00\x00\x00",3))
			{
				NormalTransData.aucExpiredDate[0]=0x20;
				asc_bcd(&NormalTransData.aucExpiredDate[1],3,aucBuf,6);
			}
		}
				
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}
unsigned char Trans_Preload(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];
	short 			iLen;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	if(DataSave1.CardTable.ucCountType=='1')
	{
		if(NormalTransData.ucTransType==TRANS_S_REFUND)
			ISO8583_SetBitValue(3,(uchar *)"646600",6);
		else if(NormalTransData.ucTransType==TRANS_S_PRELOAD)
			ISO8583_SetBitValue(3,(uchar *)"626600",6);
		else if(NormalTransData.ucTransType==TRANS_S_LOADONLINE)
			ISO8583_SetBitValue(3,(uchar *)"676600",6);
		else if(NormalTransData.ucTransType==TRANS_S_CASHLOAD)
			ISO8583_SetBitValue(3,(uchar *)"596600",6);
	}
	else
	{
		if(NormalTransData.ucTransType==TRANS_S_REFUND)
			ISO8583_SetBitValue(3,(uchar *)"646600",6);
		else if(NormalTransData.ucTransType==TRANS_S_PRELOAD)
			ISO8583_SetBitValue(3,(uchar *)"624600",6);
		else if(NormalTransData.ucTransType==TRANS_S_LOADONLINE)
		{
			if(NormalTransData.ucCardLoadFlag==1)
				ISO8583_SetBitValue(3,(uchar *)"676000",6);
			else
				ISO8583_SetBitValue(3,(uchar *)"674000",6);
		}
		else if(NormalTransData.ucTransType==TRANS_S_CASHLOAD)
			ISO8583_SetBitValue(3,(uchar *)"594600",6);
	}
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	if(NormalTransData.ucTransType==TRANS_S_REFUND)
		ulAmount=0;
	else
		ulAmount=NormalTransData.ulAmount;
	long_asc(aucBuf,12,&ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(NormalTransData.ucTransType,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	if( EXTRATRANS_uiISO2Len )
	{
	    ISO8583_SetBitValue(35,EXTRATRANS_aucISO2,EXTRATRANS_uiISO2Len);
	}
	if( EXTRATRANS_uiISO3Len )
	{
	    ISO8583_SetBitValue(36,EXTRATRANS_aucISO3,EXTRATRANS_uiISO3Len);
	}
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	ISO8583_SetBitValue(47,DataSave0.ChangeParamData.aucCashierNo,6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulTrueBalance);
	ISO8583_SetBitValue(45,aucBuf,12);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//Set bit 62 card status
	if(NormalTransData.ucTransType==TRANS_S_LOADONLINE)
	{
		if(SleTransData.ucStatus==CARD_RECYCLED)
			aucBuf[0]='1';
		else
			aucBuf[0]='0';
		aucBuf[1]=NormalTransData.ucPackFlag+'0';
		memcpy(&aucBuf[2],SendTransData.aucSysRefNo,12);
		if(SendTransData.aucCardId[17])
			memcpy(&aucBuf[14],"19",2);
		else
			memcpy(&aucBuf[14],"16",2);
		memcpy(&aucBuf[16],SendTransData.aucCardId,19);
		ISO8583_SetBitValue(62,aucBuf,35);
	}
	//Set bit 63
	memset(aucBuf,0,sizeof(aucBuf));
	if(NormalTransData.ucCardType==CARD_CPU)
	{
		aucBuf[0]=0;
		memcpy(&aucBuf[1],NormalTransData.aucCardSerial,4);
		memcpy(&aucBuf[5],&NormalTransData.aucUserCardNo[4],4);
		aucBuf[11]=RunData.ucKeyVersion;
		aucBuf[12]=RunData.ucArithId;
		memcpy(&aucBuf[13],RunData.aucRandom,4);
		memcpy(&aucBuf[17],RunData.aucTrace,2);
		memcpy(&aucBuf[19],RunData.aucPreBalance,4);
		long_tab(&aucBuf[23],4,&NormalTransData.ulAmount);
		aucBuf[27]=2;
		asc_bcd(&aucBuf[28],4,DataSave0.ConstantParamData.aucOnlytermcode,8);	
		memcpy(&aucBuf[34],RunData.aucMAC1,4);
		ISO8583_SetBitValue(63,aucBuf,38);
	}
	if(NormalTransData.ucCardType==CARD_M1)
	{
		memcpy(&aucBuf[0],NormalTransData.aucIssueDate,4);
		memcpy(&aucBuf[4],NormalTransData.aucCardSerial,4);
		memcpy(&aucBuf[8],NormalTransData.aucVerifyCode,4);
		ISO8583_SetBitValue(63,aucBuf,12);
	}
	if(NormalTransData.ucCardType==CARD_MEM)
	{
		memcpy(&aucBuf[0],NormalTransData.aucUserCardNo,8);
		ISO8583_SetBitValue(63,aucBuf,8);
	}
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{		
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
			return ucResult;
		NORMALTRANS_ulAmount=asc_long(aucBuf,12);
	   

		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(14,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		NormalTransData.aucExpiredDate[0]=0x20;
		asc_bcd(&NormalTransData.aucExpiredDate[1],3,aucBuf,6);
		/**/
		 if(NormalTransData.ucTransType==TRANS_S_LOADONLINE&&NormalTransData.ucCardLoadFlag!=1)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
			if(ucResult!=SUCCESS)	
				return ucResult;
			NormalTransData.ulAfterEDBalance=asc_long(aucBuf,12);
		}		
		else
		{
			ucResult=ISO8583_GetBitValue(63,aucBuf,&iLen,sizeof(aucBuf));
			if(ucResult!=SUCCESS)	
				return ucResult;
			if(NormalTransData.ucCardType==CARD_CPU)
			{
				memcpy(RunData.aucMAC2,aucBuf,4);
				memcpy(NormalTransData.aucDateTime,&aucBuf[4],7);
			}
			if(NormalTransData.ucCardType==CARD_M1)
			{
				desm1(aucBuf,aucBuf,DataSave0.ConstantParamData.MacMasterUseKey[0]);
				des(aucBuf,aucBuf,DataSave0.ConstantParamData.MacMasterUseKey[1]);
				desm1(aucBuf,aucBuf,DataSave0.ConstantParamData.MacMasterUseKey[0]);
				memcpy(RunData.aucLoadKey,aucBuf,6);
				desm1(&aucBuf[8],&aucBuf[8],DataSave0.ConstantParamData.MacMasterUseKey[0]);
				des(&aucBuf[8],&aucBuf[8],DataSave0.ConstantParamData.MacMasterUseKey[1]);
				desm1(&aucBuf[8],&aucBuf[8],DataSave0.ConstantParamData.MacMasterUseKey[0]);
				memcpy(RunData.aucWriteKey1,&aucBuf[8],6);
			}
		}		
		 if(NormalTransData.ucTransType==TRANS_S_PRELOAD)
	 	{
			ucResult=ISO8583_GetBitValue(62,aucBuf,&iLen,sizeof(aucBuf));
			if(ucResult!=SUCCESS)	
				return ucResult;
			NormalTransData.ucMemFlag=aucBuf[0];
	 	}
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}

unsigned char Trans_Query(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	if(NormalTransData.ucTransType==TRANS_S_QUERY&&DataSave1.CardTable.ucCountType!='1')
		ISO8583_SetBitValue(3,(uchar *)"804000",6);
	else
		ISO8583_SetBitValue(3,(uchar *)"806000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);

	if( EXTRATRANS_uiISO2Len )
	{
	    ISO8583_SetBitValue(35,EXTRATRANS_aucISO2,EXTRATRANS_uiISO2Len);
	}
	if( EXTRATRANS_uiISO3Len )
	{
	    ISO8583_SetBitValue(36,EXTRATRANS_aucISO3,EXTRATRANS_uiISO3Len);
	}

	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulPrevEDBalance);
	ISO8583_SetBitValue(46,aucBuf,12);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/**/	
		ucResult=ISO8583_GetBitValue(59,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
			return ucResult;
		RunData.ulPoint=asc_long(aucBuf,12);
	   

		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		RunData.ulValueble=asc_long(aucBuf,12);
	
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(5,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		RunData.ulCredit=asc_long(&aucBuf[0],12);

		ucResult=ISO8583_GetBitValue(62,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		RunData.ucZfFlag=aucBuf[2];
		RunData.ulBalance=asc_long(&aucBuf[3],12);
		RunData.ulFreeze=asc_long(&aucBuf[16],12);
				
	}
	
	return ucResult;
}

unsigned char Trans_M1KeyQuery(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(800);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"480000",6);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulPrevEDBalance);
	ISO8583_SetBitValue(46,aucBuf,12);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 62
	aucBuf[0]=1;
	ISO8583_SetBitValue(62,aucBuf,1);
	//bit 63
	memcpy(&aucBuf[0],NormalTransData.aucIssueDate,4);
	memcpy(&aucBuf[4],NormalTransData.aucCardSerial,4);
	memcpy(&aucBuf[8],NormalTransData.aucVerifyCode,4);
	ISO8583_SetBitValue(63,aucBuf,12);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/**/	

		ucResult=ISO8583_GetBitValue(63,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		desm1(aucBuf,aucBuf,DataSave0.ConstantParamData.MacMasterUseKey[0]);
		des(aucBuf,aucBuf,DataSave0.ConstantParamData.MacMasterUseKey[1]);
		desm1(aucBuf,aucBuf,DataSave0.ConstantParamData.MacMasterUseKey[0]);
		memcpy(RunData.aucWriteKey1,aucBuf,6);
				
	}
	
	return ucResult;
}

unsigned char Trans_RefundQuery(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	if(DataSave1.CardTable.ucCountType=='1')
		ISO8583_SetBitValue(3,(uchar *)"796000",6);
	else
		ISO8583_SetBitValue(3,(uchar *)"794000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulTrueBalance);
	ISO8583_SetBitValue(45,aucBuf,12);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/**/	
		
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		NormalTransData.ulAmount=asc_long(aucBuf,12);
				
	}
	
	return ucResult;
}

unsigned char Trans_PreloadQuery(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"636000",6);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulTrueBalance);
	ISO8583_SetBitValue(45,aucBuf,12);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/**/	
		
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		NormalTransData.ulAmount=asc_long(aucBuf,12);
				
	}
	
	return ucResult;
}

unsigned char Trans_ChangeQuery(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];
	short 			iLen;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucOldCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"426600",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 32 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(32,aucBuf,16);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);

	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/*	
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
			return ucResult;
		NORMALTRANS_ulAmount=asc_long(aucBuf,12);
	   */

		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		NormalTransData.ulAmount=asc_long(aucBuf,12);
				

				
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}

unsigned char Trans_StatusQuery(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"AC4000",6);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,"\x65\x01",2);
	memset(&aucBuf[2],'0',10);
	ISO8583_SetBitValue(61,aucBuf,12);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/**/	
		
		ucResult=ISO8583_GetBitValue(5,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		RunData.ulCredit=asc_long(aucBuf,12);
				
	}
	
	return ucResult;
}

unsigned char Trans_Sale(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"246000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 15 discount
	ISO8583_SetBitHexValue(15,NormalTransData.aucDiscount,2);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	if(NormalTransData.ucHandFlag)
		aucBuf[1]='1';
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(TRANS_S_SALEONLINE,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	if( EXTRATRANS_uiISO2Len )
	{
	    ISO8583_SetBitValue(35,EXTRATRANS_aucISO2,EXTRATRANS_uiISO2Len);
	}
	if( EXTRATRANS_uiISO3Len )
	{
	    ISO8583_SetBitValue(36,EXTRATRANS_aucISO3,EXTRATRANS_uiISO3Len);
	}
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	ISO8583_SetBitValue(47,DataSave0.ChangeParamData.aucCashierNo,6);

	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//Set bit 61 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	bcd_asc(&aucBuf[16],NormalTransData.aucOldCardNo,16);
	ISO8583_SetBitValue(61,aucBuf,32);
	// Set bit 62 pay type
	ISO8583_SetBitValue(62,&NormalTransData.ucPackFlag,1);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();
	if(ucResult!=SUCCESS)
		return ucResult;
	ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
	if(ucResult!=SUCCESS)
		return ucResult;
	NORMALTRANS_ulAmount=asc_long(aucBuf,12);
	memset(aucBuf,0,sizeof(aucBuf));
	ucResult=ISO8583_GetBitValue(14,aucBuf,&iLen,sizeof(aucBuf));
	if(ucResult!=SUCCESS)	
		return ucResult;
	NormalTransData.aucExpiredDate[0]=0x20;
	asc_bcd(&NormalTransData.aucExpiredDate[1],3,aucBuf,6);

	return ucResult;
}

unsigned char Trans_BatchLoad(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	if(DataSave1.CardTable.ucCountType=='1')
		ISO8583_SetBitValue(3,(uchar *)"A36000",6);
	else
		ISO8583_SetBitValue(3,(uchar *)"A34000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	if(NormalTransData.ucHandFlag)
		aucBuf[1]='1';
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(TRANS_S_BATCHLOAD,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	//Set bit32 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucOldCardNo,16);
	ISO8583_SetBitValue(32,aucBuf,16);
	if( EXTRATRANS_uiISO2Len )
	{
	    ISO8583_SetBitValue(35,EXTRATRANS_aucISO2,EXTRATRANS_uiISO2Len);
	}
	if( EXTRATRANS_uiISO3Len )
	{
	    ISO8583_SetBitValue(36,EXTRATRANS_aucISO3,EXTRATRANS_uiISO3Len);
	}
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	ISO8583_SetBitValue(47,DataSave0.ChangeParamData.aucCashierNo,6);

	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//Set bit 61 Card NO
	/*memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	bcd_asc(&aucBuf[16],NormalTransData.aucOldCardNo,16);
	ISO8583_SetBitValue(61,aucBuf,32);*/
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();
	if(ucResult!=SUCCESS)
		return ucResult;
	ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
	if(ucResult!=SUCCESS)
		return ucResult;
	NormalTransData.ulAmount=asc_long(aucBuf,12);
/*	memset(aucBuf,0,sizeof(aucBuf));
	ucResult=ISO8583_GetBitValue(14,aucBuf,&iLen,sizeof(aucBuf));
	if(ucResult!=SUCCESS)	
		return ucResult;
	NormalTransData.aucExpiredDate[0]=0x20;
	asc_bcd(&NormalTransData.aucExpiredDate[1],3,aucBuf,6);*/

	return ucResult;
}

unsigned char Trans_OnlineRefund(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"A46000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	if(NormalTransData.ucHandFlag)
		aucBuf[1]='1';
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(TRANS_S_ONLINEREFUND,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	//Set bit32 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucOldCardNo,16);
	ISO8583_SetBitValue(32,aucBuf,16);
	if( EXTRATRANS_uiISO2Len )
	{
	    ISO8583_SetBitValue(35,EXTRATRANS_aucISO2,EXTRATRANS_uiISO2Len);
	}
	if( EXTRATRANS_uiISO3Len )
	{
	    ISO8583_SetBitValue(36,EXTRATRANS_aucISO3,EXTRATRANS_uiISO3Len);
	}
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	ISO8583_SetBitValue(47,DataSave0.ChangeParamData.aucCashierNo,6);

	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//Set bit 61 Card NO
	/*memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	bcd_asc(&aucBuf[16],NormalTransData.aucOldCardNo,16);
	ISO8583_SetBitValue(61,aucBuf,32);*/
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)	
		return ucResult;

	ucResult = COMMS_TransOnline();
	if(ucResult!=SUCCESS)
		return ucResult;

	if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
	if(ucResult!=SUCCESS)
		return ucResult;
	NormalTransData.ulAfterEDBalance=asc_long(aucBuf,12);
/*	memset(aucBuf,0,sizeof(aucBuf));
	ucResult=ISO8583_GetBitValue(14,aucBuf,&iLen,sizeof(aucBuf));
	if(ucResult!=SUCCESS)	
		return ucResult;
	NormalTransData.aucExpiredDate[0]=0x20;
	asc_bcd(&NormalTransData.aucExpiredDate[1],3,aucBuf,6);*/
	return ucResult;
}

unsigned char Trans_Mobile(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	if(NormalTransData.ucTransType==TRANS_S_MOBILECHARGE)
		ISO8583_SetBitValue(3,(uchar *)"776000",6);
	if(NormalTransData.ucTransType==TRANS_S_MOBILECHARGEONLINE)
		ISO8583_SetBitValue(3,(uchar *)"774000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	ulAmount=NormalTransData.ulAmount-NormalTransData.ulRateAmount;
	long_asc(aucBuf,12,&ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	//Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
/*
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
*/
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 33 
	ISO8583_SetBitValue(33,NormalTransData.aucBarCode,11);
	
	//Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	//Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
/*
	//Set bit 44
	ISO8583_SetBitValue(44,COM_VERSION,5);
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulPrevEDBalance);
	ISO8583_SetBitValue(46,aucBuf,12);
*/
	//Set bit 47 cashier id
	ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	//Set bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//Set bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
/*
	//Set bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
*/
	//Set bit 57
	memset(aucBuf,0,sizeof(aucBuf));
	asc_bcd(aucBuf,4,DataSave0.ConstantParamData.aucTerminalID,8);
	memcpy(&aucBuf[4],DataSave0.ChangeParamData.aucBatchNumber,12);
	long_bcd(&aucBuf[16],3,&RunData.ulOldTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);
	//Set bit 58 
	memset(aucBuf,0,sizeof(aucBuf));
	ulAmount=NormalTransData.ulAmount;
	long_asc(aucBuf,12,&ulAmount);
	ISO8583_SetBitValue(58,aucBuf,12);
	//Set bit 60
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,NormalTransData.aucProCode,15);
	memcpy(&aucBuf[20],NormalTransData.aucBarCode,11);
	ISO8583_SetBitValue(60,aucBuf,31);
/*
	//Set bit 61
	memset(aucBuf,0,sizeof(aucBuf));
	ulAmount=NormalTransData.ulAmount-NormalTransData.ulRateAmount;
	long_asc(aucBuf,12,&ulAmount);
	ISO8583_SetBitValue(61,aucBuf,6);
	//Set bit 62
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&RunData.ulOldTraceNumber);
	if(NormalTransData.ucTransType==TRANS_S_MOBILECHARGE)
		aucBuf[6]='0';
	if(NormalTransData.ucTransType==TRANS_S_MOBILECHARGEONLINE)
		aucBuf[6]='1';
	ISO8583_SetBitValue(62,aucBuf,7);
*/
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();
	
	return ucResult;
}

unsigned char Trans_PreAuth(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];
	short 			iLen;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"324000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(TRANS_S_PREAUTH,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//Set bit 61
	memset(aucBuf,0,sizeof(aucBuf));
	long_bcd(aucBuf,6,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(61,aucBuf,6);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/*	
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
			return ucResult;
		NORMALTRANS_ulAmount=asc_long(aucBuf,12);
	   */

		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		RunData.ulValueble=asc_long(aucBuf,12);
		
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(5,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		RunData.ulCredit=asc_long(aucBuf,12);

		if(RunData.ulValueble>=RunData.ulCredit)
			NormalTransData.ulAfterEDBalance=RunData.ulValueble-RunData.ulCredit;
		else
		{
			NormalTransData.ulAfterEDBalance=RunData.ulCredit-RunData.ulValueble;
			NormalTransData.ucBalanceFlag=1;
		}
		
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(62,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		memcpy(NormalTransData.aucAuthCode,aucBuf,14);
				
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}

unsigned char Trans_PreAuthVoid(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];
	short 			iLen;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"334000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	if(NormalTransData.ucHandFlag)
		aucBuf[1]='1';
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(TRANS_S_PREAUTHVOID,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//Set bit 62
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,NormalTransData.aucAuthCode,14);
	ISO8583_SetBitValue(62,aucBuf,14);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/**/	
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
			return ucResult;
		NORMALTRANS_ulAmount=asc_long(aucBuf,12);
	   

		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		NormalTransData.ulAfterEDBalance=asc_long(aucBuf,12);
		
				
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}

unsigned char Trans_PreAuthFinish(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];
	short 			iLen;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"344000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	if(NormalTransData.ucHandFlag)
		aucBuf[1]='1';
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(TRANS_S_PREAUTHFINISH,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//Set bit 62
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,NormalTransData.aucAuthCode,14);
	ISO8583_SetBitValue(62,aucBuf,14);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/*	
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
			return ucResult;
		NORMALTRANS_ulAmount=asc_long(aucBuf,12);
	   */

		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		RunData.ulValueble=asc_long(aucBuf,12);
		
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(5,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		RunData.ulCredit=asc_long(aucBuf,12);

		if(RunData.ulValueble>=RunData.ulCredit)
			NormalTransData.ulAfterEDBalance=RunData.ulValueble-RunData.ulCredit;
		else
		{
			NormalTransData.ulAfterEDBalance=RunData.ulCredit-RunData.ulValueble;
			NormalTransData.ucBalanceFlag=1;
		}
		
				
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}

unsigned char Trans_PreAuthFinishVoid(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];
	short 			iLen;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"354000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	if(NormalTransData.ucHandFlag)
		aucBuf[1]='1';
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(TRANS_S_PREAUTHFINISH,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//Set bit 57
	 memset(aucBuf,0,sizeof(aucBuf));
	asc_bcd(aucBuf,4,DataSave0.ConstantParamData.aucTerminalID,8);
	memcpy(&aucBuf[4],DataSave0.ChangeParamData.aucBatchNumber,12);
	long_bcd(&aucBuf[16],3,&RunData.ulOldTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);
	//Set bit 62
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,NormalTransData.aucAuthCode,14);
	ISO8583_SetBitValue(62,aucBuf,14);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/*	
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
			return ucResult;
		NORMALTRANS_ulAmount=asc_long(aucBuf,12);
	   */

		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		NormalTransData.ulAfterEDBalance=asc_long(aucBuf,12);
		
				
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}

unsigned char Trans_GetTrace(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(800);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"160000",6);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 22 inputmodel
	//memset(aucBuf,'0',3);
	//aucBuf[3]=NormalTransData.ucCardType;
	//ISO8583_SetBitValue(22,aucBuf,4);

	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 32 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucBarCode,20);
	ISO8583_SetBitValue(32,&aucBuf[1],19);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulPrevEDBalance);
	ISO8583_SetBitValue(46,aucBuf,12);
	
	// bit 48 Batchnumber
	//memset(aucBuf,0,sizeof(aucBuf));
	//bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	//ISO8583_SetBitValue(48,aucBuf,24);

	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/**/	
		ucResult=ISO8583_GetBitValue(60,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
			return ucResult;
		NormalTransData.ulTicketNumber=asc_long(aucBuf,6);
		iLen-=7;
		int_bcd(&NormalTransData.ucBarLenth,1,(uint *)&iLen);
		if(aucBuf[6]=='1')
			asc_bcd(NormalTransData.aucOldCardNo,10,&aucBuf[7],20);
				
	}
	
	return ucResult;
}

unsigned char Trans_ZshOnlinePurchase(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen,iBarLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	if(NormalTransData.ucTransType==TRANS_S_JSZSHONLINEPURCHASE)
		ISO8583_SetBitValue(3,(uchar *)"A24000",6);
	else
		ISO8583_SetBitValue(3,(uchar *)"A14000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(NormalTransData.ucTransType,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	//Set bit 32 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucBarCode,20);
	ISO8583_SetBitValue(32,&aucBuf[1],19);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	// Set bit 47
	ISO8583_SetBitValue(47,(uchar*)"111111",6);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	memset(aucBuf,0,sizeof(aucBuf));
	long_bcd(&aucBuf[16],3,&NormalTransData.ulTicketNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	//bit 58 
	memset(aucBuf,0,sizeof(aucBuf));
	ulAmount=NormalTransData.ulAmount-NormalTransData.ulRateAmount;
	long_asc(aucBuf,12,&ulAmount);
	ISO8583_SetBitValue(58,aucBuf,12);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)	
		return ucResult;
	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/*	
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
			return ucResult;
		NORMALTRANS_ulAmount=asc_long(aucBuf,12);
	   */

		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(46,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		RunData.ulValueble=asc_long(aucBuf,12);
		
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(5,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		RunData.ulCredit=asc_long(aucBuf,12);

		if(RunData.ulValueble>=RunData.ulCredit)
			NormalTransData.ulAfterEDBalance=RunData.ulValueble-RunData.ulCredit;
		else
		{
			NormalTransData.ulAfterEDBalance=RunData.ulCredit-RunData.ulValueble;
			NormalTransData.ucBalanceFlag=1;
		}
				
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}


unsigned char Trans_Transfer(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[300];
	short 			iLen,offset;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucOldCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"654000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	ulAmount=NormalTransData.ulAmount-NormalTransData.ulRateAmount;
	long_asc(aucBuf,12,&ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	if(NormalTransData.ucHandFlag)
		aucBuf[1]='1';
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(TRANS_S_TRANSFER,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	//Set bit 32 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(32,aucBuf,16);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	ISO8583_SetBitValue(44,(uchar*)"00008",5);
	ISO8583_SetBitValue(47,(uchar*)"111111",6);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//Set bit 57
	 memset(aucBuf,0,sizeof(aucBuf));
	asc_bcd(aucBuf,4,DataSave0.ConstantParamData.aucTerminalID,8);
	memcpy(&aucBuf[4],DataSave0.ChangeParamData.aucBatchNumber,12);
	long_bcd(&aucBuf[16],3,&RunData.ulOldTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);
	//Set bit 62
	memset(aucBuf,0,sizeof(aucBuf));
	offset=0;
	//ƽ̨
	bcd_asc(&aucBuf[offset],&RunData.ucPt,2);	offset +=2;
	//
	bcd_asc(&aucBuf[offset],RunData.aucZoneCode,6);	offset +=6;
	//ˮ
	long_asc(&aucBuf[offset],6,&RunData.ulOldTraceNumber);	offset += 6;
	ISO8583_SetBitValue(62,aucBuf,offset);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;

	UTIL_IncreaseTraceNumber(0);

	ucResult = COMMS_TransOnline();

	if(ucResult==SUCCESS)
	{	/**/	
				
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}

unsigned char Trans_ChangePin(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	if(DataSave1.CardTable.ucCountType=='1')
		ISO8583_SetBitValue(3,(uchar *)"816000",6);
	else
		ISO8583_SetBitValue(3,(uchar *)"814000",6);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);

	if( EXTRATRANS_uiISO2Len )
	{
	    ISO8583_SetBitValue(35,EXTRATRANS_aucISO2,EXTRATRANS_uiISO2Len);
	}
	if( EXTRATRANS_uiISO3Len )
	{
	    ISO8583_SetBitValue(36,EXTRATRANS_aucISO3,EXTRATRANS_uiISO3Len);
	}

	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	ISO8583_SetBitValue(44,(uchar*)"00008",5);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//bit 53 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucNewPin,8);
	ISO8583_SetBitHexValue(53,aucBuf,8);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	UTIL_IncreaseTraceNumber(0);

	ucResult = COMMS_TransOnline();
	
	return ucResult;
}
unsigned char Trans_ChangeCard(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];
	short 			iLen;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucOldCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"836600",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(TRANS_S_CHANGECD,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	//Set bit 32 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(32,aucBuf,16);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	if(RunData.aucFunFlag[11])
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,RunData.aucPin,8);
		ISO8583_SetBitHexValue(52,aucBuf,8);
	}
	else
	{
		//bit 61 
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,RunData.aucPin,6);
		ISO8583_SetBitValue(61,aucBuf,6);
	}
	//Set bit 63
	memset(aucBuf,0,sizeof(aucBuf));
	if(NormalTransData.ucCardType==CARD_CPU)
	{
		aucBuf[0]=0;
		memcpy(&aucBuf[1],NormalTransData.aucCardSerial,4);
		memcpy(&aucBuf[5],&NormalTransData.aucUserCardNo[4],4);
		aucBuf[11]=RunData.ucKeyVersion;
		aucBuf[12]=RunData.ucArithId;
		memcpy(&aucBuf[13],RunData.aucRandom,4);
		memcpy(&aucBuf[17],RunData.aucTrace,2);
		memcpy(&aucBuf[19],RunData.aucPreBalance,4);
		long_tab(&aucBuf[23],4,&NormalTransData.ulAmount);
		aucBuf[27]=2;
		asc_bcd(&aucBuf[28],4,DataSave0.ConstantParamData.aucOnlytermcode,8);	
		memcpy(&aucBuf[34],RunData.aucMAC1,4);
		ISO8583_SetBitValue(63,aucBuf,38);
	}
	if(NormalTransData.ucCardType==CARD_M1)
	{
		memcpy(&aucBuf[0],NormalTransData.aucIssueDate,4);
		memcpy(&aucBuf[4],NormalTransData.aucCardSerial,4);
		memcpy(&aucBuf[8],NormalTransData.aucVerifyCode,4);
		ISO8583_SetBitValue(63,aucBuf,12);
	}
	if(NormalTransData.ucCardType==CARD_MEM)
	{
		memcpy(&aucBuf[0],NormalTransData.aucUserCardNo,8);
		ISO8583_SetBitValue(63,aucBuf,8);
	}
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	/*	
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
			return ucResult;
		NORMALTRANS_ulAmount=asc_long(aucBuf,12);
	   */

		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		NormalTransData.ulAmount=asc_long(aucBuf,12);
				
		/*memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		bcd_asc(NormalTransData.aucLoadTrace,aucBuf,16);*/

		ucResult=ISO8583_GetBitValue(63,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)	
			return ucResult;
		if(NormalTransData.ucCardType==CARD_CPU)
		{
			memcpy(RunData.aucMAC2,aucBuf,4);
			memcpy(NormalTransData.aucDateTime,&aucBuf[4],7);
		}
		if(NormalTransData.ucCardType==CARD_M1)
		{
			desm1(aucBuf,aucBuf,DataSave0.ConstantParamData.MacMasterUseKey[0]);
			des(aucBuf,aucBuf,DataSave0.ConstantParamData.MacMasterUseKey[1]);
			desm1(aucBuf,aucBuf,DataSave0.ConstantParamData.MacMasterUseKey[0]);
			memcpy(RunData.aucLoadKey,aucBuf,6);
			desm1(&aucBuf[8],&aucBuf[8],DataSave0.ConstantParamData.MacMasterUseKey[0]);
			des(&aucBuf[8],&aucBuf[8],DataSave0.ConstantParamData.MacMasterUseKey[1]);
			desm1(&aucBuf[8],&aucBuf[8],DataSave0.ConstantParamData.MacMasterUseKey[0]);
			memcpy(RunData.aucWriteKey1,&aucBuf[8],6);
		}
				
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}

unsigned char Trans_ChangeExp(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[300];
	short 			iLen,offset=0,uiNum,i,auioffset[250];
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	for(i=0;i<TRANS_MAXNB;i++)	
	{
		if(DataSave0.TransInfoData.auiTransIndex[i])
		{
			ucResult=XDATA_Read_SaveTrans_File_2Datasave(i);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(DataSave0.SaveTransData.ucTransType!=TRANS_S_CHANGEEXP
				||DataSave0.SaveTransData.ucKYFlag==1
				||DataSave1.ucBatchFlag[i]==0x01)
				continue;
		}
		else
			continue;
		ISO8583_Clear();
		// Set Msgid 
		ISO8583_SetMsgID(600);
		//Set bit 2 Card NO
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,DataSave0.SaveTransData.aucUserCardNo,16);
		ISO8583_SetBitValue(2,aucBuf,16);
		//Set processCode
		ISO8583_SetBitValue(3,(uchar *)"436000",6);
		//Set bit 4 Amount
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,12,&DataSave0.SaveTransData.ulAmount);
		ISO8583_SetBitValue(4,aucBuf,12);
		
		// Set system trace audit number bit 11 	
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
		ISO8583_SetBitValue(11,aucBuf,6);

		
		CFG_GetDateTime();
		//Set bit 12 the Time
		ISO8583_SetBitHexValue(12,&DataSave0.SaveTransData.aucDateTime[4],3);
		//Set bit 13 the date
		ISO8583_SetBitHexValue(13,&DataSave0.SaveTransData.aucDateTime[1],3);
		//Set bit 14 expire date
		ISO8583_SetBitHexValue(14,&DataSave0.SaveTransData.aucExpiredDate[1],3);
		//Set bit 22 inputmodel
		memset(aucBuf,'0',3);
		aucBuf[3]=DataSave0.SaveTransData.ucCardType;
		ISO8583_SetBitValue(22,aucBuf,4);
		//Set bit 25 
		aucBuf[0]=DataSave0.SaveTransData.ucNewOrOld+1+'0';
		aucBuf[1]=DataSave0.SaveTransData.ucCardType;
		ISO8583_SetBitValue(25,aucBuf,2);
		//Set bit 26 
		ISO8583_SetBitHexValue(26,&DataSave0.SaveTransData.ucPtCode,1);
		//Set bit 27 
		STL_GetTotalDomain(TRANS_S_CHANGEEXP,aucBuf);
		ISO8583_SetBitHexValue(27,aucBuf,1);
		// Set Card Acceptor Terminal Identification bit 41
		ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
		// Set Card Acceptor Identification Code bit 42 
		ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
		// Set bit 47 cashier id
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,DataSave0.ChangeParamData.aucCashierNo,6);
		ISO8583_SetBitValue(47,aucBuf,6);

		// Set bit 44
		//ISO8583_SetBitValue(44,COM_VERSION,5);
		
		// bit 48 Batchnumber
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
		ISO8583_SetBitValue(48,aucBuf,24);
		//bit 49 zonecode
		ISO8583_SetBitHexValue(49,DataSave0.SaveTransData.aucZoneCode,3);
		//Set bit 61
		offset=0;
		//ʶ
		aucBuf[offset]=DataSave0.SaveTransData.ucPackFlag;	offset+=1;
		//ԭЧ
		memcpy(&aucBuf[offset],DataSave0.SaveTransData.aucExpiredDate,4);	offset +=4;
		//Ч
		memcpy(&aucBuf[offset],DataSave0.SaveTransData.aucNewExpiredDate,4);	offset +=4;
		ISO8583_SetBitValue(61,aucBuf,offset);
		//Set bit 62
		offset=0;
		aucBuf[offset]=1;offset+=1;
		aucBuf[offset]=DataSave0.SaveTransData.ucPtCode;offset+=1;
		STL_GetTotalDomain(DataSave0.SaveTransData.ucTransType,&aucBuf[offset]);
		offset +=1;				
		//
		aucBuf[offset]=0x16;offset+=1;
		memcpy(&aucBuf[offset],DataSave0.SaveTransData.aucUserCardNo,10);	offset += 10;
		//ǰ
		long_tab(&aucBuf[offset],4,&DataSave0.SaveTransData.ulPrevEDBalance);	offset +=4;
		//ˮ
		long_bcd(&aucBuf[offset],3,&DataSave0.SaveTransData.ulTraceNumber);		offset +=3;
		//ɢ
		if(DataSave0.SaveTransData.ucCardType==CARD_M1)
		{
			memcpy(&aucBuf[offset],&DataSave0.SaveTransData.aucIssueDate[2],2);
			memcpy(&aucBuf[offset+2],DataSave0.SaveTransData.aucCardSerial,4);
			memcpy(&aucBuf[offset+6],DataSave0.SaveTransData.aucVerifyCode,1);
			aucBuf[offset+7]=0x02;
		}
		else
		if(DataSave0.SaveTransData.ucCardType==CARD_CPU)
		{
			memcpy(&aucBuf[offset],DataSave0.SaveTransData.aucCardSerial,4);
			memcpy(&aucBuf[offset+4],&DataSave0.SaveTransData.aucUserCardNo[4],4);
		}
		else
			memcpy(&aucBuf[offset],DataSave0.SaveTransData.aucUserCardNo,8);
			
		offset +=8;
		//
		long_tab(&aucBuf[offset],4,&DataSave0.SaveTransData.ulAmount);	offset +=4;
		//Ӧͱʶ
		if(DataSave0.SaveTransData.ucCardType==CARD_CPU)
			aucBuf[offset]=0x06;
		else
			aucBuf[offset]=0x21;
		offset += 1;
		//ն˱
		asc_bcd(&aucBuf[offset],4,DataSave0.ConstantParamData.aucOnlytermcode,8);	
		offset +=6;
		//
		long_bcd(&aucBuf[offset+1],3,&DataSave0.SaveTransData.ulTraceNumber);
		offset +=4;
		//ʱ
		memcpy(&aucBuf[offset],DataSave0.SaveTransData.aucDateTime,7);	offset +=7;
		//TAC
		memcpy(&aucBuf[offset],DataSave0.SaveTransData.aucTraceTac,4);	offset +=4;
		//ֵˮ
		asc_bcd(&aucBuf[offset],8,DataSave0.SaveTransData.aucLoadTrace,16);	offset +=8;
		//
		memcpy(&aucBuf[offset],DataSave0.SaveTransData.aucZoneCode,3);	offset +=3;
		//
		ulAmount=DataSave0.SaveTransData.ulAmount;
		long_tab(&aucBuf[offset],4,&ulAmount);	offset +=4;
		//
		memcpy(&aucBuf[offset],DataSave0.SaveTransData.aucSamTace,4);offset +=4;
		//
		aucBuf[offset]=DataSave0.SaveTransData.ucNewOrOld+1+'0';
		offset+=1;
		aucBuf[offset]=DataSave0.SaveTransData.ucCardType;
		offset+=1;
		//׺
		ulAmount=DataSave0.SaveTransData.ulAfterEDBalance;
		long_tab(&aucBuf[offset],4,&ulAmount);	offset +=4;
		ISO8583_SetBitValue(62,aucBuf,offset);
		//Set MAC NULL first
		ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
		
		ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
								(unsigned char *)&DataSave0.SaveISO8583Data);
		XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		ucResult = UTIL_IncreaseTraceNumber(0);
		if(ucResult!=SUCCESS)	
			return ucResult;
		ucResult = COMMS_TransOnline();

		if(ucResult==SUCCESS)
		{
			Os__saved_set((unsigned char *)&DataSave1.ucBatchFlag[i],
							0x01,sizeof(unsigned char));
			ucResult=XDATA_Write_Vaild_File(DataSaveBatchFlags);
			if(ucResult!=SUCCESS)
				return ucResult;
		}
		if(ucResult!=SUCCESS)
		{
			return ucResult;
		}
	}
	return ucResult;
}


unsigned char Trans_RefApl(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];
	short 			iLen;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"456000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',4);
	aucBuf[1]='1';
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(TRANS_S_REFAPL,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,DataSave0.ChangeParamData.aucCashierNo,6);
	ISO8583_SetBitValue(47,aucBuf,6);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//Set bit 62 Cashier Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(62,aucBuf,16);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)	
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	
	
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}


unsigned char Trans_VoidRefApl(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];
	short 			iLen;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(600);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"466000",6);
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',4);
	aucBuf[1]='1';
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(TRANS_S_REFAPL,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,DataSave0.ChangeParamData.aucCashierNo,6);
	ISO8583_SetBitValue(47,aucBuf,6);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//Set bit 57
	 memset(aucBuf,0,sizeof(aucBuf));
	asc_bcd(aucBuf,4,DataSave0.ConstantParamData.aucTerminalID,8);
	memcpy(&aucBuf[4],DataSave0.ChangeParamData.aucBatchNumber,12);
	long_bcd(&aucBuf[16],3,&RunData.ulOldTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);
	//Set bit 62 Cashier Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(62,aucBuf,16);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
	/* Set reversal trans data */
	ISO8583_SetMsgID(400);

	ISO8583_RemoveBit(11);
	//ISO8583_RemoveBit(63);
	memset(aucBuf,0,sizeof(aucBuf));
	Uart_Printf("S_NORMALTRANS_ulTraceNumber=%d\n",NORMALTRANS_ulTraceNumber);
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	/* Save new reversal */
	ucResult = ISO8583_SaveReversalISO8583Data();
	if(ucResult!=SUCCESS)	
		return ucResult;
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)	
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{	
	
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}
unsigned char Trans_ReversalPreload(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];
	short 			iLen;
	unsigned long   ulAmount;
	
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(400);
	//Set bit 2 Card NO
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
	ISO8583_SetBitValue(2,aucBuf,16);
	//Set processCode
	if(DataSave1.CardTable.ucCountType=='1')
	{
		if(NormalTransData.ucTransType==TRANS_S_REFUND)
			ISO8583_SetBitValue(3,(uchar *)"646600",6);
		else if(NormalTransData.ucTransType==TRANS_S_PRELOAD)
			ISO8583_SetBitValue(3,(uchar *)"626600",6);
		else if(NormalTransData.ucTransType==TRANS_S_LOADONLINE)
			ISO8583_SetBitValue(3,(uchar *)"676600",6);
		else if(NormalTransData.ucTransType==TRANS_S_CASHLOAD)
			ISO8583_SetBitValue(3,(uchar *)"596600",6);
	}
	else
	{
		if(NormalTransData.ucTransType==TRANS_S_REFUND)
			ISO8583_SetBitValue(3,(uchar *)"646600",6);
		else if(NormalTransData.ucTransType==TRANS_S_PRELOAD)
			ISO8583_SetBitValue(3,(uchar *)"624600",6);
		else if(NormalTransData.ucTransType==TRANS_S_LOADONLINE)
		{
			if(NormalTransData.ucCardLoadFlag==1)
				ISO8583_SetBitValue(3,(uchar *)"676000",6);
			else
				ISO8583_SetBitValue(3,(uchar *)"674000",6);
		}
		else if(NormalTransData.ucTransType==TRANS_S_CASHLOAD)
			ISO8583_SetBitValue(3,(uchar *)"594600",6);
	}
	//Set bit 4 Amount
	memset(aucBuf,0,sizeof(aucBuf));
	if(NormalTransData.ucTransType==TRANS_S_REFUND)
		ulAmount=0;
	else
		ulAmount=NormalTransData.ulAmount;
	long_asc(aucBuf,12,&ulAmount);
	ISO8583_SetBitValue(4,aucBuf,12);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&DataSave0.ChangeParamData.ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	
	CFG_GetDateTime();
	//Set bit 12 the Time
	ISO8583_SetBitHexValue(12,&NormalTransData.aucDateTime[4],3);
	//Set bit 13 the date
	ISO8583_SetBitHexValue(13,&NormalTransData.aucDateTime[1],3);
	//Set bit 14 expire date
	ISO8583_SetBitHexValue(14,&NormalTransData.aucExpiredDate[1],3);
	//Set bit 22 inputmodel
	memset(aucBuf,'0',3);
	aucBuf[3]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 25 
	aucBuf[0]=NormalTransData.ucNewOrOld+1+'0';
	aucBuf[1]=NormalTransData.ucCardType;
	ISO8583_SetBitValue(25,aucBuf,2);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&NormalTransData.ucPtCode,1);
	//Set bit 27 
	STL_GetTotalDomain(NormalTransData.ucTransType,aucBuf);
	ISO8583_SetBitHexValue(27,aucBuf,1);
	if( EXTRATRANS_uiISO2Len )
	{
	    ISO8583_SetBitValue(35,EXTRATRANS_aucISO2,EXTRATRANS_uiISO2Len);
	}
	if( EXTRATRANS_uiISO3Len )
	{
	    ISO8583_SetBitValue(36,EXTRATRANS_aucISO3,EXTRATRANS_uiISO3Len);
	}
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	// Set bit 47 cashier id
	ISO8583_SetBitValue(47,DataSave0.ChangeParamData.aucCashierNo,6);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,12,&NormalTransData.ulTrueBalance);
	ISO8583_SetBitValue(45,aucBuf,12);
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//bit 49 zonecode
	ISO8583_SetBitHexValue(49,NormalTransData.aucZoneCode,3);
	//bit 52 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	//Set bit 57
	memset(aucBuf,0,sizeof(aucBuf));
	long_bcd(&aucBuf[16],3,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(57,aucBuf,22);	
	//Set bit 62 card status
	if(NormalTransData.ucTransType==TRANS_S_LOADONLINE)
	{
		if(SleTransData.ucStatus==CARD_RECYCLED)
			aucBuf[0]='1';
		else
			aucBuf[0]='0';
		ISO8583_SetBitValue(62,aucBuf,1);
	}
	//Set bit 63
	memset(aucBuf,0,sizeof(aucBuf));
	if(NormalTransData.ucCardType==CARD_CPU)
	{
		aucBuf[0]=0;
		memcpy(&aucBuf[1],NormalTransData.aucCardSerial,4);
		memcpy(&aucBuf[5],&NormalTransData.aucUserCardNo[4],4);
		aucBuf[11]=RunData.ucKeyVersion;
		aucBuf[12]=RunData.ucArithId;
		memcpy(&aucBuf[13],RunData.aucRandom,4);
		memcpy(&aucBuf[17],RunData.aucTrace,2);
		memcpy(&aucBuf[19],RunData.aucPreBalance,4);
		long_tab(&aucBuf[23],4,&NormalTransData.ulAmount);
		aucBuf[27]=2;
		asc_bcd(&aucBuf[28],4,DataSave0.ConstantParamData.aucOnlytermcode,8);	
		memcpy(&aucBuf[34],RunData.aucMAC1,4);
		ISO8583_SetBitValue(63,aucBuf,38);
	}
	if(NormalTransData.ucCardType==CARD_M1)
	{
		memcpy(&aucBuf[0],NormalTransData.aucIssueDate,4);
		memcpy(&aucBuf[4],NormalTransData.aucCardSerial,4);
		memcpy(&aucBuf[8],NormalTransData.aucVerifyCode,4);
		ISO8583_SetBitValue(63,aucBuf,12);
	}
	if(NormalTransData.ucCardType==CARD_MEM)
	{
		memcpy(&aucBuf[0],NormalTransData.aucUserCardNo,8);
		ISO8583_SetBitValue(63,aucBuf,8);
	}
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();

	
	
	if(ucResult==SUCCESS)
	{		
	}else if(ucResult==ERR_HOSTCODE&&RUNDATA_ucErrorExtCode!=27)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	return ucResult;
}

unsigned char Trans_SwitchInform(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[500],aucIPV4Addr1[5],aucIPV4Addr2[5];
	short 			iLen;
	unsigned long   ulAmount;
	int i;
	
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		return SUCCESS;
	ucResult=TRS_CheckReversal();
	
	if(ucResult!=SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		UTIL_GetKey(3);
		return ucResult;
	}
	
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(200);
	//Set processCode
	ISO8583_SetBitValue(3,(uchar *)"C30000",6);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);

	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);	
	
	// bit 48 Batchnumber
	memset(aucBuf,0,sizeof(aucBuf));
	bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBatchNumber,24);
	ISO8583_SetBitValue(48,aucBuf,24);
	//Set bit 62 card status
	memcpy(aucIPV4Addr1,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPAdd1,4);
	memcpy(aucIPV4Addr2,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPAdd2,4);
	memset(aucBuf,0,sizeof(aucBuf));
	aucBuf[0]='0';
	aucBuf[1]='1';
	if(DataSave0.ChangeParamData.ucCurrentIPFlag==1)
		aucBuf[2]='0';
	else
		aucBuf[2]='1';
	bcd_asc(&aucBuf[3],&NormalTransData.aucDateTime[1],12);
	if(DataSave0.ChangeParamData.ucCurrentIPFlag==1)
	{
            sprintf((char *)&aucBuf[15],"%3d.%3d.%3d.%3d",
                aucIPV4Addr1[0],aucIPV4Addr1[1],aucIPV4Addr1[2],aucIPV4Addr1[3]);
            sprintf((char *)&aucBuf[30],"%3d.%3d.%3d.%3d",
                aucIPV4Addr2[0],aucIPV4Addr2[1],aucIPV4Addr2[2],aucIPV4Addr2[3]);
	}
	else
	{
            sprintf((char *)&aucBuf[15],"%3d.%3d.%3d.%3d",
                aucIPV4Addr2[0],aucIPV4Addr2[1],aucIPV4Addr2[2],aucIPV4Addr2[3]);
            sprintf((char *)&aucBuf[30],"%3d.%3d.%3d.%3d",
                aucIPV4Addr1[0],aucIPV4Addr1[1],aucIPV4Addr1[2],aucIPV4Addr1[3]);
	}
	ISO8583_SetBitValue(62,aucBuf,45);
	//Set MAC NULL first
	ISO8583_SetBitValue(64,(uchar *)"000000000000",8);
	
	ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.SaveISO8583Data);
	XDATA_Write_Vaild_File(DataSaveSaveISO8583);	
		
	ucResult = UTIL_IncreaseTraceNumber(0);
	if(ucResult!=SUCCESS)
		return ucResult;

	ucResult = COMMS_TransOnline();
	if(ucResult==SUCCESS)
	{
		DataSave0.ChangeParamData.ucSwitchIPFlag=0;
		XDATA_Write_Vaild_File(DataSaveChange);
	}

	return ucResult;
}

