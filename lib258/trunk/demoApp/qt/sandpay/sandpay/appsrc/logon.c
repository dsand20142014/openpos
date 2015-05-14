#include <include.h>
#include <global.h>
#include <xdata.h>


unsigned char LOGON_IniProcess(void)
{
	unsigned char ucResult=SUCCESS;


	if(RunData.ucInputKeyFlag&&DataSave0.ConstantParamData.ucYTJFlag==2)
	{
		//Os__clr_display(255);
        //Os__GB2312_display(0,0,(uchar *)"请由收银机发起");
       // UTIL_GetKey(3);
		return SUCCESS;
	}
	
	NORMALTRANS_ucTransType=TRANS_S_INIONLINE;
	DataSave0.ChangeParamData.ucInilizeFlag=FLAG_INIT_UNDO;
	DataSave0.ChangeParamData.ucIsLogonFlag=FLAG_LOGON_UNDO;
	DataSave0.ChangeParamData.ucIsCashierLogonFlag=FLAG_LOGON_UNDO;

    Uart_Printf("\n LOGON_IniProcess :%02x\n",ucResult);

    ucResult=XDATA_Write_Vaild_File(DataSaveChange);
    Uart_Printf("\n 2012 07 15 - LOGON_IniProcess XDATA_Write_Vaild_File\:%02x=\n",ucResult);

	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_PreComm();

	Uart_Printf("111COMMS_PreCommCOMMS_PreCommCOMMS_PreCommCOMMS_PreComm\n");
	
	if(ucResult==SUCCESS)
		ucResult=LOGON_IniOnline();

	Uart_Printf("222COMMS_PreCommCOMMS_PreCommCOMMS_PreCommCOMMS_PreComm:%02x, rst:%02x\n",DataSave0.ConstantParamData.ucTranskeyflag,ucResult);

	if(ucResult==SUCCESS)
	{
		if(!DataSave0.ConstantParamData.ucTranskeyflag)
		{		
			Uart_Printf("1111LOGON_DownTransKeyLOGON_DownTransKeyLOGON_DownTransKey");

			ucResult=LOGON_DownTransKey();

			Uart_Printf("2222LOGON_DownTransKeyLOGON_DownTransKeyLOGON_DownTransKey:%02x\n",ucResult);
			
			if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
			{
				Uart_Printf("COMMS_PreCommCOMMS_PreComm\n");
				COMMS_PreComm();
			}
		}
	}
	Uart_Printf("\nlogon 00049:%02x\n",ucResult);

	
	if(ucResult==SUCCESS)
		ucResult=LOGON_DownKey();
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_FinComm();
	if(ucResult==SUCCESS)
	{
		DataSave0.ChangeParamData.ucInilizeFlag=FLAG_INIT_OK;
		DataSave0.ChangeParamData.uiCardTableNum=0;
		DataSave0.ChangeParamData.uiBlackFeildNum=0;
		DataSave0.ChangeParamData.uiBlackNum=0;
		DataSave0.ChangeParamData.uiRateTableNum=0;
		memset(DataSave0.ChangeParamData.aucBlackDateTime,0,7);
		memset(DataSave0.ChangeParamData.aucTableDateTime,0,7);
		memset(DataSave0.ChangeParamData.aucZoneDateTime,0,7);
		memset(DataSave0.ChangeParamData.aucRateRenewDateTime,0,7);
		memset(DataSave0.ChangeParamData.aucIssueKeyRenewDateTime,0,7);
		ucResult=XDATA_Write_Vaild_File(DataSaveChange);
		DataSave0.ConstantParamData.ulMult=0;
		ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
		XDATA_Clear_ZoneCtrls_File();
        XDATA_Clear_CardTables_File();
		XDATA_Clear_Rate_File();
		XDATA_Clear_Black_File();
	
	}
	if(ucResult==SUCCESS)
	{
#ifdef GUI_PROJECT
        memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
        ProUiFace.ProToUi.uiLines=1;
        memcpy(ProUiFace.ProToUi.aucLine1,"初始化成功",20);
        sleep(1);
#else
		//Os__clr_display(255);
        //Os__GB2312_display(0,0,(uchar *)"初始化成功");
        //PRT_PrintInitData();
#endif
	}
	if(Commun_flag==0x32&&DataSave0.ConstantParamData.ucConnectType=='2')
		TCP_Close(RunData.iTCPHandle);

	Uart_Printf("LOGON_IniProcess:%02x\n",ucResult);
	return ucResult;
}

unsigned char LOGON_IniOnline(void)
{
	unsigned char ucResult,aucProcessCode[6];
	unsigned char aucBuf[400],t,transbuf[20];
	short 		iLen;
	int i,iI;
	memset(aucProcessCode,0,sizeof(aucProcessCode));
	memcpy(aucProcessCode,"900000",6);

	DataSave0.ChangeParamData.uiBlackNum = 0;
	DataSave0.ChangeParamData.uiBlackFeildNum = 0;
	DataSave0.ChangeParamData.uiBlackSrlFeildNum = 0;
	DataSave0.ChangeParamData.uiCardTableNum = 0;
	DataSave0.ChangeParamData.uiZoneCodeNum = 0;
	DataSave0.ChangeParamData.uiRateTableNum = 0;
	memset(DataSave0.ChangeParamData.aucIssueKeyIndex,0,18*3);
	memset(&DataSave0.ChangeParamData.uiCTNum,0,24*sizeof(unsigned int));
	Uart_Printf("DataSave0.ChangeParamData.uiCTNum:\n");
	for(iI=0;iI<24;iI++)
	{
		Uart_Printf("%d ",DataSave0.ChangeParamData.uiCTNum[iI/8][iI%8]);
		if(iI==7||iI==15||iI==23)
			Uart_Printf("\n");
	}
	
	ucResult=XDATA_Write_Vaild_File(DataSaveChange);
	if(ucResult!=SUCCESS)
		return ucResult;

	ISO8583_Clear();
	
	ISO8583_SetMsgID(800);
	
	ISO8583_SetBitValue(3,aucProcessCode,6);
	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);
	
	UTIL_IncreaseTraceNumber(0);

	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);

	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);

	//ISO8583_SetBitValue(44,COM_VERSION,5);

	memset(aucBuf,0,sizeof(aucBuf));
	UTIL_PinVersionAnalyze(DataSave0.ConstantParamData.aucPinpadVersion,14,&aucBuf[60]);
	memcpy(&aucBuf[80],CURRENT_PRJ,sizeof(CURRENT_PRJ));
	memcpy(&aucBuf[110],CURRENT_VER,sizeof(CURRENT_VER));
	memcpy(&aucBuf[140],COM_VERSION,5);
	ISO8583_SetBitValue(60,aucBuf,145);
	
	ucResult = COMMS_TransSendReceive();
	
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(60,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult==SUCCESS)
		{
			memcpy(DataSave0.ConstantParamData.aucMerchantName,aucBuf,30);
			memcpy(DataSave0.ConstantParamData.aucHostTelNumber1,&aucBuf[30],15);
			memcpy(DataSave0.ConstantParamData.aucHostTelNumber2,&aucBuf[45],15);
			memcpy(DataSave0.ConstantParamData.aucHostTelNumber3,&aucBuf[60],15);	
			memcpy(DataSave0.ConstantParamData.aucHostTelNumber4,&aucBuf[75],15);	
			DataSave0.ConstantParamData.ucAccessType=aucBuf[90];
			DataSave0.ConstantParamData.ucConnectType=aucBuf[91];
			memset(transbuf,0,sizeof(transbuf));
			memcpy(transbuf,&aucBuf[92],15);
			UTIL_IPAnalyze(transbuf,&DataSave0.ConstantParamData.ulHostIPAdd1);
			DataSave0.ConstantParamData.uiHostPort1=asc_long(&aucBuf[107],8);
			memset(transbuf,0,sizeof(transbuf));
			memcpy(transbuf,&aucBuf[115],15);
			UTIL_IPAnalyze(transbuf,&DataSave0.ConstantParamData.ulHostIPAdd2);
			DataSave0.ConstantParamData.uiHostPort2=asc_long(&aucBuf[130],8);
			memcpy(DataSave0.ConstantParamData.aucHostDomain,&aucBuf[138],30);
			memcpy(RunData.aucDownKey,&aucBuf[168],10);
			if(DataSave0.TransInfoData.TransTotal.uiTotalNb==0)
				memcpy(DataSave0.ConstantParamData.aucTreat,&aucBuf[168],10);
			memcpy(DataSave0.ConstantParamData.aucMid,&aucBuf[178],24);
			memcpy(DataSave0.ConstantParamData.aucTid,&aucBuf[202],12);
			ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
		}
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult = ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
		}
		if(ucResult==SUCCESS)
		{
			desm1(&aucBuf[0],&aucBuf[0],DataSave0.ConstantParamData.MacMasterKey[0]);
			des(&aucBuf[0],&aucBuf[0],DataSave0.ConstantParamData.MacMasterKey[1]);
			desm1(&aucBuf[0],DataSave0.ConstantParamData.MacMasterUseKey[0],
				DataSave0.ConstantParamData.MacMasterKey[0]);
			desm1(&aucBuf[8],&aucBuf[8],DataSave0.ConstantParamData.MacMasterKey[0]);
			des(&aucBuf[8],&aucBuf[8],DataSave0.ConstantParamData.MacMasterKey[1]);
			desm1(&aucBuf[8],DataSave0.ConstantParamData.MacMasterUseKey[1],
				DataSave0.ConstantParamData.MacMasterKey[0]);
			ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
		}
	}
	else 
	{
		return ucResult;
	}
	
	return ucResult;
}

unsigned char LOGON_LogonProcess(void)
{
	unsigned char ucResult=SUCCESS;
	if(RunData.ucInputKeyFlag&&DataSave0.ConstantParamData.ucYTJFlag==2)
	{
		//Os__clr_display(255);
        //Os__GB2312_display(0,0,(uchar *)"请由收银机发起");
        //UTIL_GetKey(3);
		return SUCCESS;
	}
	
	if(DataSave0.ChangeParamData.ucInilizeFlag!=FLAG_INIT_OK)
	{
		ucResult=LOGON_IniProcess();
		if(ucResult!=SUCCESS)
		{
			return ucResult;
		}
	}
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_PreComm();
	DataSave0.ChangeParamData.ucIsLogonFlag=FLAG_LOGON_UNDO;
	ucResult=XDATA_Write_Vaild_File(DataSaveChange);
	if(ucResult!=SUCCESS)
	{
		if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
			COMMS_FinComm();
		return ucResult;
	}
	ucResult=LOGON_LogonOnline();
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_FinComm();
	if(ucResult==SUCCESS)
	{
		DataSave0.ChangeParamData.ucIsLogonFlag=FLAG_LOGON_OK;
		ucResult=XDATA_Write_Vaild_File(DataSaveChange);
	}
	if(DataSave0.ChangeParamData.ucSwitchIPFlag==1)
	{
		Trans_SwitchInform();
	}
	if(ucResult==SUCCESS)
	{

#ifdef GUI_PROJECT
        memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
        ProUiFace.ProToUi.uiLines=1;
        memcpy(ProUiFace.ProToUi.aucLine1,"系统签到成功!",26);
        sleep(1);
#else
		//Os__clr_display(255);
        //Os__GB2312_display(0,0,(uchar *)"系统签到成功!");
		UTIL_GetKey(3);
#endif
	}else{
#ifdef GUI_PROJECT
		memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
		ProUiFace.ProToUi.uiLines=1;	
        memcpy(ProUiFace.ProToUi.aucLine1,"系统签到失败!",26);
        sleep(1);
#endif
	}
	
	if(Commun_flag==0x32&&DataSave0.ConstantParamData.ucConnectType=='2')
		TCP_Close(RunData.iTCPHandle);
	return ucResult;
}

unsigned char LOGON_LogonOnline(void)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[300],aucProcessCode[6],aucLogonDateTime[10],aucIssKeyDateTime[10];
	unsigned char 	*ptBlack,*ptCardtable,*ptZone;
	unsigned short  uiBlackNum,uiTableNum,uiZoneNum,i;
	
	
	ucResult = SUCCESS;
	RunData.uc1stIpTye=1;
	ucResult=LOGON_SyncOnline();
	RunData.uc1stIpTye=0;
	if(ucResult!=SUCCESS)
	{
		return ucResult;
	}
	memcpy(aucProcessCode,"920000",6);
	uiBlackNum=0;  
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(800);
	// Set process code bit 3
	ISO8583_SetBitValue(3,aucProcessCode,6);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);
	UTIL_IncreaseTraceNumber(0);	
	
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);

	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	// Set bit 47 cashier id
    //ISO8583_SetBitValue(47,(unsigned char *)"111111",6);
	
	// Set bit 60 last logon on time and sam card id
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(&aucBuf[0],DataSave0.ChangeParamData.aucZoneDateTime,7);
	memcpy(&aucBuf[7],DataSave0.ChangeParamData.aucBlackDateTime,7);
	memcpy(&aucBuf[14],DataSave0.ChangeParamData.aucTableDateTime,7);
	memcpy(&aucBuf[21],DataSave0.ChangeParamData.aucRateRenewDateTime,7);
	memcpy(&aucBuf[28],DataSave0.ChangeParamData.aucIssueKeyRenewDateTime,7);
	ISO8583_SetBitValue(60,aucBuf,35);
	
	ucResult = COMMS_TransSendReceive();
	if(ucResult==SUCCESS)
	{
		// Bit 12 TIME  
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(12,aucBuf,&iLen,sizeof(aucBuf));
		if( ucResult != SUCCESS )
			return(ucResult);
		asc_bcd(&aucLogonDateTime[4],3,aucBuf,6);
		ucResult = XDATA_Write_Vaild_File(DataSaveChange);		
		if( ucResult != SUCCESS )
			return(ucResult);
		memset(aucTime,0,sizeof(aucTime));
		memcpy(aucTime,aucBuf,6);	
		// Bit 13 Date 
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(13,aucBuf,&iLen,sizeof(aucBuf));
		if( ucResult != SUCCESS )
			return(ucResult);
		aucLogonDateTime[0]=0x20;
		asc_bcd(&aucLogonDateTime[1],3,aucBuf,6);
		ucResult = XDATA_Write_Vaild_File(DataSaveChange);		
		if( ucResult != SUCCESS )
			return(ucResult);
		Os__read_date(aucDate);
		memcpy(&aucDate[0],&aucBuf[4],2);
		memcpy(&aucDate[2],&aucBuf[2],2);
		memcpy(&aucDate[4],&aucBuf[0],2);
		Os__write_date(aucDate);
		Os__write_time(aucTime);
	
	}
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult!=SUCCESS)
		{
			return ucResult;
		}
		memset(DataSave0.ChangeParamData.aucPrintInfo,0,sizeof(DataSave0.ChangeParamData.aucPrintInfo));
		asc_bcd(DataSave0.ChangeParamData.aucSettleCycle,3,&aucBuf[16],6);
		DataSave0.ConstantParamData.ulMaxXiaoFeiAmount = asc_long(&aucBuf[22],3)*10000;
		memcpy(DataSave0.ChangeParamData.aucPrintInfo,&aucBuf[25],40);
		memcpy(DataSave0.ConstantParamData.aucOnlytermcode,&aucBuf[65],8);
		UTIL_BitFlagAnalyze(&aucBuf[73],2,DataSave0.ChangeParamData.aucTermFlag);
		ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
		if(ucResult!=SUCCESS)
			return ucResult;
		ucResult=XDATA_Write_Vaild_File(DataSaveChange);
		if(ucResult!=SUCCESS)
			return ucResult;
		ucResult=KEY_StoreNewKey(&aucBuf[0]);
		if(ucResult!=SUCCESS)				
			return ucResult;
	}
	
	if(ucResult==SUCCESS/*&&DataSave0.TransInfoData.TransTotal.uiTotalNb==0*/)
	{
		//get card table bit 60
		if((ISO8583_CheckBit(60)==SUCCESS)&&(ISO8583Data.Bit[60-1].uiLen !=0))
		{
			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(60,aucBuf,&iLen,sizeof(aucBuf));
			if(ucResult!=SUCCESS)
				return ucResult;
#ifdef TEST
		{
			unsigned short uiI;
			Uart_Printf("card tabel \n");
			for(uiI=0;uiI<iLen;uiI++)
				Uart_Printf("%02x ",aucBuf[uiI]);
			Uart_Printf("\n");
		}
#endif
			if(aucBuf[0]=='1')
			{
				ucResult=LOGON_DownZone();
				if(ucResult==SUCCESS)
				{
					memcpy(DataSave0.ChangeParamData.aucZoneDateTime,aucLogonDateTime,7);
					 ucResult=XDATA_Write_Vaild_File(DataSaveChange);		
				}
				if(ucResult!=SUCCESS)
				{
					return ucResult;
				}
			}
			
		        if(aucBuf[1]=='1')
			 {
			        ucResult = LOGON_DownBlackList();
				if(ucResult==SUCCESS)
				{
					memcpy(DataSave0.ChangeParamData.aucBlackDateTime,aucLogonDateTime,7);
					 ucResult=XDATA_Write_Vaild_File(DataSaveChange);	
				}
				if(ucResult!=SUCCESS)
				{
					return ucResult;
				}
			}
			if(aucBuf[2]=='1')
			{
				ucResult=LOGON_DownTable();
				if(ucResult==SUCCESS)
				{
					if(aucBuf[6]=='1')
						ucResult=LOGON_DownTableSps();
				}
				if(ucResult==SUCCESS)
				{
					memcpy(DataSave0.ChangeParamData.aucTableDateTime,aucLogonDateTime,7);
					 ucResult=XDATA_Write_Vaild_File(DataSaveChange);	
				}
				if(ucResult!=SUCCESS)
				{
					return ucResult;
				}
			}
			if(aucBuf[3]=='1')
			{
				ucResult=LOGON_DownRate();
				if(ucResult==SUCCESS)
				{
					memcpy(DataSave0.ChangeParamData.aucRateRenewDateTime,aucLogonDateTime,7);
					ucResult= XDATA_Write_Vaild_File(DataSaveChange);		
				}
				if(ucResult!=SUCCESS)
				{
					return ucResult;
				}
			}
			if(aucBuf[4]=='1'||aucBuf[7]=='1')
			{
				if(aucBuf[4]=='1')
				{
					ucResult=LOGON_DownMemberCardKey();
					if(ucResult!=SUCCESS)
					{
						return ucResult;
					}
				}
				if(aucBuf[7]=='1')
				{
					ucResult=LOGON_DownWKCardKey();
					if(ucResult!=SUCCESS)
					{
						return ucResult;
					}
				}
				if(ucResult==SUCCESS)
				{
					memcpy(DataSave0.ChangeParamData.aucIssueKeyRenewDateTime,aucLogonDateTime,7);
					ucResult= XDATA_Write_Vaild_File(DataSaveChange);		
				}
				if(ucResult!=SUCCESS)
				{
					return ucResult;
				}
			}
		}		
	}
	if(ucResult==SUCCESS)
	{
		memcpy(DataSave0.ChangeParamData.aucLogonDateTime,aucLogonDateTime,7);
		ucResult=XDATA_Write_Vaild_File(DataSaveChange);		
	}
	return ucResult;
}


unsigned char LOGON_DownTableSps(void)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[600],aucProcessCode[6];
	unsigned char 	*ptCardtable;
	unsigned short  	uiTableNum,i;
	
	
	ucResult = SUCCESS;
	memcpy(aucProcessCode,"140000",6);
	uiTableNum=0;  
	while(1)
	{
		ISO8583_Clear();
		// Set Msgid 
		ISO8583_SetMsgID(800);
		// Set process code bit 3
		ISO8583_SetBitValue(3,aucProcessCode,6);
		
		// Set system trace audit number bit 11 	
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
		ISO8583_SetBitValue(11,aucBuf,6);
		UTIL_IncreaseTraceNumber(0);	
		
		// Set Card Acceptor Terminal Identification bit 41
		ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
		// Set Card Acceptor Identification Code bit 42 
		ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);

		// Set bit 44
        //ISO8583_SetBitValue(44,COM_VERSION,5);
		// Set bit 47 cashier id
        //ISO8583_SetBitValue(47,(unsigned char *)"111111",6);
		
		// Set bit 60 last logon on time and sam card id
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,DataSave0.ChangeParamData.aucTableDateTime,14);
		ISO8583_SetBitValue(60,aucBuf,14);
		
		ucResult = COMMS_TransSendReceive();
		if(ucResult==SUCCESS)
		{
			//get card table bit 60
			if((ISO8583_CheckBit(61)==SUCCESS)&&(ISO8583Data.Bit[61-1].uiLen !=0))
			{
				
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
				if(ucResult!=SUCCESS)
					return ucResult;
#ifdef TEST
				{
					unsigned short uiI;
					Uart_Printf("card tabel \n");
					for(uiI=0;uiI<iLen;uiI++)
						Uart_Printf("%02x ",aucBuf[uiI]);
					Uart_Printf("\n");
				}
#endif
                ptCardtable=aucBuf;
				uiTableNum=(unsigned short)bcd_long(aucBuf,2);
				ucResult=XDATA_AppendRecords(DataSaveCardTables,ptCardtable,sizeof(CARDTABLE),uiTableNum);
				if(ucResult!=SUCCESS)
					return ucResult;
				ucResult=XDATA_Write_Vaild_File(DataSaveChange);
				if(ucResult!=SUCCESS)
					return ucResult;
			}
			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(3,aucBuf,&iLen,6);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(!memcmp(aucBuf,"140001",6))
            {
                memcpy(aucProcessCode,"140001",6);
                continue;
            }
            if(!memcmp(aucBuf,"140000",6))
                break;
		}
		else
			return ucResult;
		
	}
	return ucResult;	
}


unsigned char LOGON_DownTable(void)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[600],aucProcessCode[6];
	unsigned char 	*ptCardtable;
	unsigned short  	uiTableNum,i;
	
	
	ucResult = SUCCESS;
	memcpy(aucProcessCode,"120000",6);
	uiTableNum=0;  
	while(1)
	{
		ISO8583_Clear();
		// Set Msgid 
		ISO8583_SetMsgID(800);
		// Set process code bit 3
		ISO8583_SetBitValue(3,aucProcessCode,6);
		
		// Set system trace audit number bit 11 	
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
		ISO8583_SetBitValue(11,aucBuf,6);
		UTIL_IncreaseTraceNumber(0);	
		
		// Set Card Acceptor Terminal Identification bit 41
		ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
		// Set Card Acceptor Identification Code bit 42 
		ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);

		// Set bit 44
        //ISO8583_SetBitValue(44,COM_VERSION,5);
		// Set bit 47 cashier id
        //ISO8583_SetBitValue(47,(unsigned char *)"111111",6);
		
		// Set bit 60 last logon on time and sam card id
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,DataSave0.ChangeParamData.aucTableDateTime,14);
		ISO8583_SetBitValue(60,aucBuf,14);
		
		ucResult = COMMS_TransSendReceive();
		if(ucResult==SUCCESS)
		{
			//get card table bit 60
			if((ISO8583_CheckBit(61)==SUCCESS)&&(ISO8583Data.Bit[61-1].uiLen !=0))
			{
				
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
				if(ucResult!=SUCCESS)
					return ucResult;
#ifdef TEST
				{
					unsigned short uiI;
					Uart_Printf("card tabel \n");
					for(uiI=0;uiI<iLen;uiI++)
						Uart_Printf("%02x ",aucBuf[uiI]);
					Uart_Printf("\n");
				}
#endif
                ptCardtable=aucBuf;
				uiTableNum=(unsigned short)bcd_long(aucBuf,2);
				ucResult=XDATA_AppendRecords(DataSaveCardTables,ptCardtable,sizeof(CARDTABLE),uiTableNum);
				if(ucResult!=SUCCESS)
					return ucResult;
				ucResult=XDATA_Write_Vaild_File(DataSaveChange);
				if(ucResult!=SUCCESS)
					return ucResult;
			}
			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(3,aucBuf,&iLen,6);
			if(ucResult!=SUCCESS)
				return ucResult;
            if(!memcmp(aucBuf,"120001",6))
            {
                memcpy(aucProcessCode,"120001",6);
                continue;
            }
            if(!memcmp(aucBuf,"120000",6))
                break;

		}
		else
			return ucResult;
	}
	return ucResult;	
}

unsigned char LOGON_DownZone(void)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[600],aucProcessCode[6];
	unsigned char 	*ptZone;
	unsigned short  	uiZoneNum,i;
	
	
	ucResult = SUCCESS;
	memcpy(aucProcessCode,"100000",6);
	uiZoneNum=0;  
	while(1)
	{
		ISO8583_Clear();
		// Set Msgid 
		ISO8583_SetMsgID(800);
		// Set process code bit 3
		ISO8583_SetBitValue(3,aucProcessCode,6);
		
		// Set system trace audit number bit 11 	
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
		ISO8583_SetBitValue(11,aucBuf,6);
		UTIL_IncreaseTraceNumber(0);	
		
		// Set Card Acceptor Terminal Identification bit 41
		ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
		// Set Card Acceptor Identification Code bit 42 
		ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);
		// Set bit 47 cashier id
		//ISO8583_SetBitValue(47,(unsigned char *)"111111",6);
		// Set bit 44
        //ISO8583_SetBitValue(44,COM_VERSION,5);
		
		// Set bit 60 last logon on time and sam card id
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,DataSave0.ChangeParamData.aucZoneDateTime,14);
		ISO8583_SetBitValue(60,aucBuf,14);
		
		memcpy(RunData.aucComVersion,(uchar *)"00002",5);
		ucResult = COMMS_TransSendReceive();
		memcpy(RunData.aucComVersion,COM_VERSION,5);
		if(ucResult==SUCCESS)
		{
			//get blackcard list bit 62 Zone control code
			if((ISO8583_CheckBit(61)==SUCCESS)&&(ISO8583Data.Bit[61-1].uiLen !=0))
			{
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
				if(ucResult!=SUCCESS)
					return ucResult;
#ifdef TEST
			{
				unsigned short uiI;
				Uart_Printf("zone control code\n");
				for(uiI=0;uiI<iLen;uiI++)
					Uart_Printf("%02x ",aucBuf[uiI]);
				Uart_Printf("\n");
			}
#endif
            ptZone=aucBuf;
            uiZoneNum=(unsigned short)bcd_long(aucBuf,2);
            ucResult=XDATA_AppendRecords(DataSaveZoneCtrls,ptZone,sizeof(CARDZONE),uiZoneNum);
            if(ucResult!=SUCCESS)
                return ucResult;
            ucResult=XDATA_Write_Vaild_File(DataSaveChange);
            if(ucResult!=SUCCESS)
                return ucResult;
			}
			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(3,aucBuf,&iLen,6);
			if(ucResult!=SUCCESS)
				return ucResult;
            if(!memcmp(aucBuf,"100001",6))
            {
                memcpy(aucProcessCode,"100001",6);
                continue;
            }
            if(!memcmp(aucBuf,"100000",6))
                break;
		}
		else
			return ucResult;
		
	}
	return ucResult;	
}

unsigned char LOGON_DownBlackList(void)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[600],aucProcessCode[6];
	unsigned char 	*ptBlackField;
	unsigned short  	uiTableNum,i;
	
	
	ucResult = SUCCESS;
	memcpy(aucProcessCode,"110000",6);
	uiTableNum=0;  
	while(1)
	{
		ISO8583_Clear();
		// Set Msgid 
		ISO8583_SetMsgID(800);
		// Set process code bit 3
		ISO8583_SetBitValue(3,aucProcessCode,6);
		
		// Set system trace audit number bit 11 	
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
		ISO8583_SetBitValue(11,aucBuf,6);
		UTIL_IncreaseTraceNumber(0);	
		
		// Set Card Acceptor Terminal Identification bit 41
		ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
		// Set Card Acceptor Identification Code bit 42 
		ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);

		// Set bit 44
	      //ISO8583_SetBitValue(44,COM_VERSION,5);
		// Set bit 47 cashier id
	//	ISO8583_SetBitValue(47,(unsigned char *)"111111",6);
		
		// Set bit 60 last logon on time and sam card id
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,DataSave0.ChangeParamData.aucBlackDateTime,14);
		ISO8583_SetBitValue(60,aucBuf,14);
		
		ucResult = COMMS_TransSendReceive();
		if(ucResult==SUCCESS)
		{
			//get card table bit 60
			if((ISO8583_CheckBit(61)==SUCCESS)&&(ISO8583Data.Bit[61-1].uiLen !=0))
			{
				
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
				if(ucResult!=SUCCESS)
					return ucResult;
				{
					unsigned short uiI;
					Uart_Printf("black field \n");
					for(uiI=0;uiI<iLen;uiI++)
						Uart_Printf("%02x ",aucBuf[uiI]);
					Uart_Printf("\n");
				}
			        ptBlackField=aucBuf;
				uiTableNum=(unsigned short)bcd_long(aucBuf,2);
				ucResult=XDATA_AppendRecords(DataSaveBlackFeilds,ptBlackField,sizeof(BLACKFIELD),uiTableNum);
				if(ucResult!=SUCCESS)
					return ucResult;
				ucResult=XDATA_Write_Vaild_File(DataSaveChange);
				if(ucResult!=SUCCESS)
					return ucResult;
			}
			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(3,aucBuf,&iLen,6);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(!memcmp(aucBuf,"110001",6))
		    	{
		    	 	memcpy(aucProcessCode,"110001",6);
			       		continue;
		 	}
			if(!memcmp(aucBuf,"110000",6))
		          break;	
		}
		else
			return ucResult;
		
	}
	return ucResult;	
}


unsigned char LOGON_DownRate(void)
{
	short 		  	iLen,uiBlackNum;
	unsigned char 	aucTime[6],aucDate[6],ucResult = SUCCESS;
	unsigned char 	aucBuf[300],aucProcessCode[6],aucBuf1[50];
	unsigned char 	*ptCardtable,*ptZone,*ptRate;
	unsigned short       uiTableNum,uiZoneNum,i,uiRateNum;
	unsigned char        ucNum = 0;//chenzx add 2005-9-20 14:34
	
	memset(aucProcessCode,0,sizeof(aucProcessCode));
	memcpy(aucProcessCode,"130000",6);
	uiBlackNum=0;  
	
	while(1)
	{
		ISO8583_Clear();
		// Set Msgid 
		ISO8583_SetMsgID(800);
		// Set process code bit 3
		ISO8583_SetBitValue(3,aucProcessCode,6);
		
		// Set system trace audit number bit 11 	
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
		ISO8583_SetBitValue(11,aucBuf,6);
		ucResult=UTIL_IncreaseTraceNumber(0);	
		if(ucResult!=SUCCESS)
			return ucResult;
		
		// Set Card Acceptor Terminal Identification bit 41
		ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
		// Set Card Acceptor Identification Code bit 42 
		ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);
		// Set bit 44
		ISO8583_SetBitValue(44,(uchar*)"00008",5);
		// Set bit 47 cashier id
		ISO8583_SetBitValue(47,(unsigned char *)"111111",6);

		//SMC_GetDateTimeDeliverCardRenew();
		
		// Set bit 60 last logon on time and sam card id
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,DataSave0.ChangeParamData.aucRateRenewDateTime,14);
		ISO8583_SetBitValue(60,aucBuf,14);

		//SMC_GetDateTimeDeliverCardRenew();
		
		ucResult = COMMS_TransSendReceive();
		
		if(ucResult==SUCCESS)
		{			
			//get rate table bit 61
			if((ISO8583_CheckBit(61)==SUCCESS)&&(ISO8583Data.Bit[61-1].uiLen !=0))
			{
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
				if(ucResult!=SUCCESS)
					return ucResult;

				ptRate=aucBuf;
				uiRateNum=(unsigned short)bcd_long(aucBuf,2);

				ucResult=XDATA_AppendRecords(DataSaveRate,ptRate,sizeof(RATETABLE),uiRateNum);
				if(ucResult!=SUCCESS)
					return ucResult;
				ucResult=XDATA_Write_Vaild_File(DataSaveChange);
		
				if(ucResult!=SUCCESS)
					return ucResult;					
						
			}

			//get blackcard list bit 62 Zone control code
			
								
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(3,aucBuf,&iLen,6);
			if(ucResult!=SUCCESS)
			{
				return ucResult;
			}
			
			if(!memcmp(aucBuf,"130001",6))
	    	       {
				memcpy(aucProcessCode,"130001",6);
				continue;
		 	}
			
			if(!memcmp(aucBuf,"130000",6))
			{
		              break;	
			}
		}
		else
		{
			return ucResult;
		}
		
	}

	
	return ucResult;	
}

unsigned char LOGON_DownKey(void)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6],ucKeyArrey;
	unsigned char 	aucBuf[600],aucProcessCode[6],aucBase[2];
	unsigned char 	*ptCardtable,aucBit25[2];
	unsigned short  	uiTableNum,i;
	int j;
	Uart_Printf("RunData.aucDownKey:\n");
	for(i=0;i<4;i++)
		Uart_Printf("%02x ",RunData.aucDownKey[i]);
	Uart_Printf("\n");

	ucResult = SUCCESS;
	memcpy(aucProcessCode,"970000",6);
	uiTableNum=0;  
	i=0;j=0;
	for(i=0;i<2;i++)
	{
		uiTableNum=RunData.aucDownKey[i]-'0';
		for(j=0;j<uiTableNum;j++)
		{
			ISO8583_Clear();
			// Set Msgid 
			ISO8583_SetMsgID(800);
			// Set process code bit 3
			ISO8583_SetBitValue(3,aucProcessCode,6);
			
			// Set system trace audit number bit 11 	
			memset(aucBuf,0,sizeof(aucBuf));
			long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
			ISO8583_SetBitValue(11,aucBuf,6);
			UTIL_IncreaseTraceNumber(0);	
			//Set bit 25 
			if(uiTableNum==1||(uiTableNum==3&&j==0))
				memcpy(aucBit25,"15",2);
			if((uiTableNum==2&&j==0)||(uiTableNum==3&&j==1))
				memcpy(aucBit25,"21",2);
			if((uiTableNum==2&&j==1)||(uiTableNum==3&&j==2))
				memcpy(aucBit25,"20",2);
			ISO8583_SetBitValue(25,aucBit25,2);
			//Set bit 26 
			memset(aucBuf,0,sizeof(aucBuf));
			aucBuf[0]='0';
			aucBuf[1]=i+'0'+1;
			ISO8583_SetBitValue(26,aucBuf,2);
			// Set Card Acceptor Terminal Identification bit 41
			ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
			// Set Card Acceptor Identification Code bit 42 
			ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);

			// Set bit 44
		      //ISO8583_SetBitValue(44,COM_VERSION,5);
			
			
			ucResult = COMMS_TransSendReceive();
			if(ucResult==SUCCESS)
			{
				memset(aucBase,0,sizeof(aucBase));
				ucResult=ISO8583_GetBitValue(26,aucBase,&iLen,sizeof(aucBase));
				if(ucResult!=SUCCESS)
					return ucResult;
				
				
				//get card table bit 60
				if((ISO8583_CheckBit(61)==SUCCESS)&&(ISO8583Data.Bit[61-1].uiLen !=0))
				{
					
					memset(aucBuf,0,sizeof(aucBuf));
					ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
					if(ucResult!=SUCCESS)
						return ucResult;
					if(aucBase[1]=='1'&&aucBit25[0]=='1'&&aucBit25[1]==CARD_MEM)/*Ρ*/
					{
						ucResult=LoadKey(&aucBuf[0],KEYARRAY_PURSECARD2,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_WKCARD1_CRCKEY1);
	                                      ucResult=LoadKey(&aucBuf[8],KEYARRAY_PURSECARD2,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_WKCARD1_CRCKEY1+1);
						ucResult=CheckKeyVeriCode(&aucBuf[16],KEYARRAY_PURSECARD2,
												KEYINDEX_WKCARD1_CRCKEY1);
						if(ucResult!=SUCCESS)
						{
						Uart_Printf("CheckKeyVeriCodeErr1\n");
							return ucResult;	
						}
						ucResult=LoadKey(&aucBuf[54],KEYARRAY_PURSECARD2,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_WKCARD1_MESSAGEKEYA);
	                                      ucResult=LoadKey(&aucBuf[62],KEYARRAY_PURSECARD2,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_WKCARD1_MESSAGEKEYA+1);
						ucResult=CheckKeyVeriCode(&aucBuf[70],KEYARRAY_PURSECARD2,
												KEYINDEX_WKCARD1_MESSAGEKEYA);
						if(ucResult!=SUCCESS)
						{
						Uart_Printf("CheckKeyVeriCodeErr2\n");
							return ucResult;	
						}
						ucResult=LoadKey(&aucBuf[90],KEYARRAY_PURSECARD2,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_WKCARD1_PURCHASEKEY);
	                                      ucResult=LoadKey(&aucBuf[98],KEYARRAY_PURSECARD2,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_WKCARD1_PURCHASEKEY+1);
						ucResult=CheckKeyVeriCode(&aucBuf[106],KEYARRAY_PURSECARD2,
												KEYINDEX_WKCARD1_PURCHASEKEY);
						if(ucResult!=SUCCESS)
						{
						Uart_Printf("CheckKeyVeriCodeErr3\n");
							return ucResult;	
						}
					}
					else
					if(aucBase[1]=='2'&&aucBit25[0]=='1'&&aucBit25[1]==CARD_MEM)/*˹ؿ*/
					{
						ucResult=LoadKey(&aucBuf[0],KEYARRAY_PURSECARD3,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD3_AMOUNTCHECK);
	                                      ucResult=LoadKey(&aucBuf[8],KEYARRAY_PURSECARD3,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD3_AMOUNTCHECK+1);
						ucResult=CheckKeyVeriCode(&aucBuf[16],KEYARRAY_PURSECARD3,
												KEYINDEX_PURSECARD3_AMOUNTCHECK);
						if(ucResult!=SUCCESS)
						{
						Uart_Printf("CheckKeyVeriCodeErr4\n");
							return ucResult;	
						}
						ucResult=LoadKey(&aucBuf[18],KEYARRAY_PURSECARD3,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD3_CARDCHECK);
						ucResult=LoadKey(&aucBuf[26],KEYARRAY_PURSECARD3,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD3_CARDCHECK+1);
						ucResult=CheckKeyVeriCode(&aucBuf[34],KEYARRAY_PURSECARD3,
												KEYINDEX_PURSECARD3_CARDCHECK);
						if(ucResult!=SUCCESS)
						{
						Uart_Printf("CheckKeyVeriCodeErr5\n");
							return ucResult;	
						}
					}
					else
					if(aucBit25[0]=='2'&&aucBit25[1]==CARD_CPU)/*ΡCPU*/
					{
						switch(aucBase[1])
						{
							case '1':ucKeyArrey=KEYARRAY_PURSECARD0;break;
							case '2':ucKeyArrey=KEYARRAY_PURSECARD3;break;
							default:ucKeyArrey=KEYARRAY_PURSECARD5;break;
						}
						ucResult=LoadKey(&aucBuf[0],ucKeyArrey,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD0_PURCHASEKEY);
	                                      ucResult=LoadKey(&aucBuf[8],ucKeyArrey,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD0_PURCHASEKEY+1);
						ucResult=CheckKeyVeriCode(&aucBuf[16],ucKeyArrey,
												KEYINDEX_PURSECARD0_PURCHASEKEY);
						if(ucResult!=SUCCESS)
						{
						Uart_Printf("CheckKeyVeriCodeErr6\n");
							return ucResult;	
						}
						/*ucResult=LoadKey(&aucBuf[126],ucKeyArrey,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD0_APPCTRLKEY);
	                                      ucResult=LoadKey(&aucBuf[134],ucKeyArrey,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD0_APPCTRLKEY+1);
						ucResult=CheckKeyVeriCode(&aucBuf[142],ucKeyArrey,
												KEYINDEX_PURSECARD0_APPCTRLKEY);
						if(ucResult!=SUCCESS)
						{
						Uart_Printf("CheckKeyVeriCodeErr7\n");
							return ucResult;	
						}
						ucResult=LoadKey(&aucBuf[144],ucKeyArrey,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD0_APPMACKEY);
	                                      ucResult=LoadKey(&aucBuf[152],ucKeyArrey,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD0_APPMACKEY+1);
						ucResult=CheckKeyVeriCode(&aucBuf[160],ucKeyArrey,
												KEYINDEX_PURSECARD0_APPMACKEY);
						if(ucResult!=SUCCESS)
						{
						Uart_Printf("CheckKeyVeriCodeErr8\n");
							return ucResult;	
						}*/
						ucResult=LoadKey(&aucBuf[180],ucKeyArrey,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD0_UPDATEKEY);
	                                      ucResult=LoadKey(&aucBuf[188],ucKeyArrey,
										KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD0_UPDATEKEY+1);
						ucResult=CheckKeyVeriCode(&aucBuf[196],ucKeyArrey,
												KEYINDEX_PURSECARD0_UPDATEKEY);
						if(ucResult!=SUCCESS)
						{
						Uart_Printf("CheckKeyVeriCodeErr9\n");
							return ucResult;	
						}
					}
					else
					if(aucBit25[0]=='2'&&aucBit25[1]==CARD_M1)/*¿*/
					{
						if(aucBase[1]=='1')
						{
							//УԿ1
							ucResult=LoadKey(&aucBuf[0],KEYARRAY_PURSECARD1,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD1_CRCKEY);
		                                      ucResult=LoadKey(&aucBuf[8],KEYARRAY_PURSECARD1,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD1_CRCKEY+1);
												 
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[16],KEYARRAY_PURSECARD1,
													KEYINDEX_PURSECARD1_CRCKEY);
							if(ucResult!=SUCCESS)
							{
						Uart_Printf("CheckKeyVeriCodeErra\n");
								return ucResult;	
							}
							
							//ϢKeyAԿ
							ucResult=LoadKey(&aucBuf[18],KEYARRAY_PURSECARD1,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD1_MESSAGEKEYA);
							ucResult=LoadKey(&aucBuf[26],KEYARRAY_PURSECARD1,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD1_MESSAGEKEYA+1);
							
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[34],KEYARRAY_PURSECARD1,
													KEYINDEX_PURSECARD1_MESSAGEKEYA);
							if(ucResult!=SUCCESS)
							{
						Uart_Printf("CheckKeyVeriCodeErrb\n");
								return ucResult;	
							}
							//KeyAԿ
							ucResult=LoadKey(&aucBuf[36],KEYARRAY_PURSECARD1,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD1_PURCHASEKEY);
							ucResult=LoadKey(&aucBuf[44],KEYARRAY_PURSECARD1,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD1_PURCHASEKEY+1);
							
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[52],KEYARRAY_PURSECARD1,
													KEYINDEX_PURSECARD1_PURCHASEKEY);
							if(ucResult!=SUCCESS)
							{
						Uart_Printf("CheckKeyVeriCodeErrb.5\n");
								return ucResult;	
							}
							/*MACԿ*/
							ucResult=LoadKey(&aucBuf[54],KEYARRAY_PURSECARD1,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD1_LOADMACKEY);
							ucResult=LoadKey(&aucBuf[62],KEYARRAY_PURSECARD1,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD1_LOADMACKEY+1);
							
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[70],KEYARRAY_PURSECARD1,
													KEYINDEX_PURSECARD1_LOADMACKEY);
							if(ucResult!=SUCCESS)
							{
								return ucResult;	
							}
						}
						if(aucBase[1]=='2')
						{
							//УԿ1
							ucResult=LoadKey(&aucBuf[0],KEYARRAY_PURSECARD4,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD4_CRCKEYS);
		                                      ucResult=LoadKey(&aucBuf[8],KEYARRAY_PURSECARD4,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD4_CRCKEYS+1);
												 
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[16],KEYARRAY_PURSECARD4,
													KEYINDEX_PURSECARD4_CRCKEYS);
							if(ucResult!=SUCCESS)
							{
						Uart_Printf("CheckKeyVeriCodeErrc\n");
								return ucResult;	
							}
							
							//ϢKeyAԿ
							ucResult=LoadKey(&aucBuf[18],KEYARRAY_PURSECARD4,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD4_MESSAGEKEYAS);
							ucResult=LoadKey(&aucBuf[26],KEYARRAY_PURSECARD4,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD4_MESSAGEKEYAS+1);
							
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[34],KEYARRAY_PURSECARD4,
													KEYINDEX_PURSECARD4_MESSAGEKEYAS);
							if(ucResult!=SUCCESS)
							{
						Uart_Printf("CheckKeyVeriCodeErrd\n");
								return ucResult;	
							}
							//KeyAԿ
							ucResult=LoadKey(&aucBuf[36],KEYARRAY_PURSECARD4,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD4_PURCHASEKEY);
							ucResult=LoadKey(&aucBuf[44],KEYARRAY_PURSECARD4,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD4_PURCHASEKEY+1);
							
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[52],KEYARRAY_PURSECARD4,
													KEYINDEX_PURSECARD4_PURCHASEKEY);
							if(ucResult!=SUCCESS)
							{
						Uart_Printf("CheckKeyVeriCodeErrb.5\n");
								return ucResult;	
							}
							/*MACԿ*/
							ucResult=LoadKey(&aucBuf[54],KEYARRAY_PURSECARD4,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD4_LOADMACKEY);
							ucResult=LoadKey(&aucBuf[62],KEYARRAY_PURSECARD4,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD4_LOADMACKEY+1);
							
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[70],KEYARRAY_PURSECARD4,
													KEYINDEX_PURSECARD4_LOADMACKEY);
							if(ucResult!=SUCCESS)
							{
								return ucResult;	
							}
						}
						if(aucBase[1]=='3')
						{
							//УԿ1
							ucResult=LoadKey(&aucBuf[0],KEYARRAY_PURSECARD5,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD5_CRCKEYM);
		                                      ucResult=LoadKey(&aucBuf[8],KEYARRAY_PURSECARD5,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD5_CRCKEYM+1);
												 
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[16],KEYARRAY_PURSECARD5,
													KEYINDEX_PURSECARD5_CRCKEYM);
							if(ucResult!=SUCCESS)
							{
						Uart_Printf("CheckKeyVeriCodeErre\n");
								return ucResult;	
							}
							
							//ϢKeyAԿ
							ucResult=LoadKey(&aucBuf[18],KEYARRAY_PURSECARD5,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD5_MESSAGEKEYAM);
							ucResult=LoadKey(&aucBuf[26],KEYARRAY_PURSECARD5,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD5_MESSAGEKEYAM+1);
							
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[34],KEYARRAY_PURSECARD5,
													KEYINDEX_PURSECARD5_MESSAGEKEYAM);
							if(ucResult!=SUCCESS)
							{
						Uart_Printf("CheckKeyVeriCodeErrf\n");
								return ucResult;	
							}
							//KeyAԿ
							ucResult=LoadKey(&aucBuf[36],KEYARRAY_PURSECARD5,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD5_PURCHASEKEY);
							ucResult=LoadKey(&aucBuf[44],KEYARRAY_PURSECARD5,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD5_PURCHASEKEY+1);
							
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[52],KEYARRAY_PURSECARD5,
													KEYINDEX_PURSECARD4_PURCHASEKEY);
							if(ucResult!=SUCCESS)
							{
						Uart_Printf("CheckKeyVeriCodeErrb.5\n");
								return ucResult;	
							}
							/*MACԿ*/
							ucResult=LoadKey(&aucBuf[54],KEYARRAY_PURSECARD5,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD5_LOADMACKEY);
							ucResult=LoadKey(&aucBuf[62],KEYARRAY_PURSECARD5,
											KEYINDEX_PURSECARD_MASTERKEY,KEYINDEX_PURSECARD5_LOADMACKEY+1);
							
		                                         //ȶУ	
							ucResult=CheckKeyVeriCode(&aucBuf[70],KEYARRAY_PURSECARD5,
													KEYINDEX_PURSECARD5_LOADMACKEY);
							if(ucResult!=SUCCESS)
							{
								return ucResult;	
							}
						}
					}
#ifdef TEST
					{
						unsigned short uiI;
						Uart_Printf("downkey\n");
						for(uiI=0;uiI<iLen;uiI++)
							Uart_Printf("%02x ",aucBuf[uiI]);
						Uart_Printf("\n");
					}
#endif
				}
				
			}
			else
				return ucResult;
		}
	}
	return ucResult;	
}
unsigned char LOGON_DownMemberCardKey(void)
{
	short 		  	iLen,ioffset;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[600],aucProcessCode[6];
	unsigned short  	uiZoneNum,i,uiNowNum;
	
	
	ucResult = SUCCESS;
	memcpy(aucProcessCode,"990000",6);
	uiZoneNum=0;  
	while(1)
	{
		ISO8583_Clear();
		// Set Msgid 
		ISO8583_SetMsgID(800);
		// Set process code bit 3
		ISO8583_SetBitValue(3,aucProcessCode,6);
		
		// Set system trace audit number bit 11 	
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
		ISO8583_SetBitValue(11,aucBuf,6);
		UTIL_IncreaseTraceNumber(0);	
		
		ISO8583_SetBitValue(26,(uchar *)"03",2);
		// Set Card Acceptor Terminal Identification bit 41
		ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
		// Set Card Acceptor Identification Code bit 42 
		ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);
		// Set bit 60 last logon on time and sam card id
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,DataSave0.ChangeParamData.aucIssueKeyRenewDateTime,14);
		ISO8583_SetBitValue(60,aucBuf,14);
		
		ucResult = COMMS_TransSendReceive();
		if(ucResult==SUCCESS)
		{
			//get blackcard list bit 61 Zone control code
			if((ISO8583_CheckBit(61)==SUCCESS)&&(ISO8583Data.Bit[61-1].uiLen !=0))
			{
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
				if(ucResult!=SUCCESS)
					return ucResult;
				{
					unsigned short uiI;
					Uart_Printf("zone Key\n");
					for(uiI=0;uiI<iLen;uiI++)
						Uart_Printf("%02x ",aucBuf[uiI]);
					Uart_Printf("\n");
				}
				uiZoneNum=bcd_long(aucBuf,2);
				ioffset=1;
				for(i=0;i<uiZoneNum;i++)
				{
					ucResult=ZoneKey_Insert(&aucBuf[ioffset]);
					if(ucResult)return ucResult;
					Uart_Printf("aucBuf[ioffset+5]=%d\n",aucBuf[ioffset+5]);
					ioffset=ioffset+(6+aucBuf[ioffset+5]);
					Uart_Printf("ioffset=%d\n",ioffset);
				}
			}
			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(3,aucBuf,&iLen,6);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(!memcmp(aucBuf,"990001",6))
		    	{
		    	 	memcpy(aucProcessCode,"990001",6);
			       		continue;
		 	}
			if(!memcmp(aucBuf,"990000",6))
		          break;	
		}
		else
			return ucResult;
		
	}
	return ucResult;	
}
unsigned char LOGON_DownWKCardKey(void)
{
	short 		  	iLen,ioffset;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[600],aucProcessCode[6];
	unsigned short  	uiZoneNum,i,uiNowNum;
	
	
	ucResult = SUCCESS;
	memcpy(aucProcessCode,"990000",6);
	uiZoneNum=0;  
	while(1)
	{
		ISO8583_Clear();
		// Set Msgid 
		ISO8583_SetMsgID(800);
		// Set process code bit 3
		ISO8583_SetBitValue(3,aucProcessCode,6);
		
		// Set system trace audit number bit 11 	
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
		ISO8583_SetBitValue(11,aucBuf,6);
		UTIL_IncreaseTraceNumber(0);	
		
		ISO8583_SetBitValue(26,(uchar *)"01",2);
		// Set Card Acceptor Terminal Identification bit 41
		ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
		// Set Card Acceptor Identification Code bit 42 
		ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);
		// Set bit 60 last logon on time and sam card id
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,DataSave0.ChangeParamData.aucIssueKeyRenewDateTime,14);
		ISO8583_SetBitValue(60,aucBuf,14);
		
		ucResult = COMMS_TransSendReceive();
		if(ucResult==SUCCESS)
		{
			//get blackcard list bit 61 Zone control code
			if((ISO8583_CheckBit(61)==SUCCESS)&&(ISO8583Data.Bit[61-1].uiLen !=0))
			{
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
				if(ucResult!=SUCCESS)
					return ucResult;
				{
					unsigned short uiI;
					Uart_Printf("zone Key\n");
					for(uiI=0;uiI<iLen;uiI++)
						Uart_Printf("%02x ",aucBuf[uiI]);
					Uart_Printf("\n");
				}
				uiZoneNum=bcd_long(aucBuf,2);
				ioffset=1;
				for(i=0;i<uiZoneNum;i++)
				{
					ucResult=WKZoneKey_Insert(&aucBuf[ioffset]);
					if(ucResult)return ucResult;
					Uart_Printf("aucBuf[ioffset+5]=%d\n",aucBuf[ioffset+5]);
					ioffset=ioffset+(6+aucBuf[ioffset+5]);
					Uart_Printf("ioffset=%d\n",ioffset);
				}
			}
			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(3,aucBuf,&iLen,6);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(!memcmp(aucBuf,"990001",6))
		    	{
		    	 	memcpy(aucProcessCode,"990001",6);
			       		continue;
		 	}
			if(!memcmp(aucBuf,"990000",6))
		          break;	
		}
		else
			return ucResult;
		
	}
	return ucResult;	
}
unsigned char LOGON_LogoffProcess(void)
{
	unsigned short  uiIndex;
	unsigned char ucResult;

    for(uiIndex = 0;uiIndex < TRANS_MAXNB;uiIndex++)
    {
        if(DataSave0.TransInfoData.auiTransIndex[uiIndex])
            break;
    }
    
    if(uiIndex==TRANS_MAXNB)
    {
    	DataSave0.ChangeParamData.ucIsLogonFlag=FLAG_LOGON_UNDO;
        DataSave0.ChangeParamData.ucIsCashierLogonFlag=FLAG_LOGON_UNDO;
    	ucResult=XDATA_Write_Vaild_File(DataSaveChange);	
        if(ucResult!=SUCCESS)
        {
        #ifdef GUI_PROJECT
            memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
           // ProUiFace.ProToUi.uiLines=1;
            memcpy(ProUiFace.ProToUi.aucLine1,"签退失败",16);
            sleep(1);
        #endif
            return ucResult;
        }
    }
    else
    {
    #ifdef GUI_PROJECT
        memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
        //ProUiFace.ProToUi.uiLines=1;
        memcpy(ProUiFace.ProToUi.aucLine1,"请先结算",16);
        sleep(1);
    #endif
    	return ERR_SETTLE_FIRST;
    }
#ifdef GUI_PROJECT
    memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
   // ProUiFace.ProToUi.uiLines=1;
    memcpy(ProUiFace.ProToUi.aucLine1,"签退成功",16);
    sleep(1);
#else
   //Os__clr_display(255);
   //Os__GB2312_display(0,0, "签退成功");
   MSG_WaitKey(2);

#endif
    return SUCCESS;
}





unsigned char CashierLogonOnline(void)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[300],aucProcessCode[6];
	unsigned char 	*ptBlack,*ptCardtable,*ptZone;
	unsigned short  uiBlackNum,uiTableNum,uiZoneNum,i;
	
	
	ucResult = SUCCESS;
	memcpy(aucProcessCode,"950000",6);
	uiBlackNum=0;  
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(800);
	//Set bit 2 Card NO
	if(!NormalTransData.ucHandFlag)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
		ISO8583_SetBitValue(2,aucBuf,16);
	}
	// Set process code bit 3
	ISO8583_SetBitValue(3,aucProcessCode,6);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);
	UTIL_IncreaseTraceNumber(0);	
	//Set bit 22
	memset(aucBuf,0,sizeof(aucBuf));
	if(NormalTransData.ucHandFlag)
		aucBuf[1]='1';
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&RunData.ucPtcode,1);
	//Set bit 27
	if(NormalTransData.ucTransType==TRANS_S_SALE)
		ISO8583_SetBitHexValue(27,(uchar *)"\x04",1);
	if(NormalTransData.ucTransType==TRANS_S_VOID||NormalTransData.ucTransType==TRANS_S_ONLINEPURCHASE)
		ISO8583_SetBitHexValue(27,(uchar *)"\x21",1);
	if(NormalTransData.ucTransType==TRANS_S_ONLINEREFUND)
		ISO8583_SetBitHexValue(27,(uchar *)"\x12",1);
	if(NormalTransData.ucTransType==TRANS_S_LOADONLINE)
		ISO8583_SetBitHexValue(27,(uchar *)"\x05",1);
	if(NormalTransData.ucTransType==TRANS_S_CHANGEEXP)
		ISO8583_SetBitHexValue(27,(uchar *)"\x45",1);
	if(NormalTransData.ucTransType==TRANS_S_CHANGECD)
		ISO8583_SetBitHexValue(27,(uchar *)"\x43",1);
	if(NormalTransData.ucTransType==TRANS_S_BATCHLOAD)
		ISO8583_SetBitHexValue(27,(uchar *)"\x10",1);
	if(NormalTransData.ucTransType==TRANS_S_CASHLOAD)
		ISO8583_SetBitHexValue(27,(uchar *)"\x31",1);
	if(NormalTransData.ucTransType==TRANS_S_REFAPL||NormalTransData.ucTransType==TRANS_S_VOIDREFAPL)
		ISO8583_SetBitHexValue(27,(uchar *)"\x30",1);
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);
	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	// Set bit 47 cashier id
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucCashierNo,6);
	ISO8583_SetBitValue(47,aucBuf,6);
	// Set bit 47 cashier password
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);
	
	ucResult = COMMS_TransSendReceive();

	return ucResult;
}





unsigned char ExpLogonOnline(void)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[300],aucProcessCode[6];
	unsigned short  	uiZoneNum,i;
	
	
	ucResult = SUCCESS;
	memcpy(aucProcessCode,"ca0000",6);
	while(1)
	{
		ISO8583_Clear();
		// Set Msgid 
		ISO8583_SetMsgID(800);
		// Set process code bit 3
		ISO8583_SetBitValue(3,aucProcessCode,6);
		
		// Set system trace audit number bit 11 	
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
		ISO8583_SetBitValue(11,aucBuf,6);
		UTIL_IncreaseTraceNumber(0);	
		// Set Card Acceptor Terminal Identification bit 41
		ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
		// Set Card Acceptor Identification Code bit 42 
		ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);
		//Set 60 exp type
		if(NormalTransData.ucTransType==TRANS_S_SALEONLINE)
			aucBuf[0]=1;
		else
			aucBuf[0]=0;
		ISO8583_SetBitValue(60,aucBuf,1);
			
		ucResult = COMMS_TransSendReceive();
		if(ucResult==SUCCESS)
		{
			if(ucResult==SUCCESS)
			{
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult=ISO8583_GetBitValue(60,aucBuf,&iLen,sizeof(aucBuf));
				if(ucResult!=SUCCESS)
					return ucResult;
				uiZoneNum=bcd_long(aucBuf,2);
				for(i=0;i<20;i++)
				{
					if(ExpTransData.ExpFeeData[i].ucPt==0)
						break;
				}
				memcpy(&ExpTransData.ExpFeeData[i],&aucBuf[1],uiZoneNum*sizeof(EXPFEE));
			}
			
			if(ucResult==SUCCESS)
			{
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
				if(ucResult!=SUCCESS)
					return ucResult;
				uiZoneNum=bcd_long(aucBuf,2);
				for(i=0;i<30;i++)
				{
					if(ExpTransData.ExpParamData[i].ucPt==0)
						break;
				}
				memcpy(&ExpTransData.ExpParamData[i],&aucBuf[1],uiZoneNum*sizeof(EXPPARAM));
			}
			ucResult=XDATA_Write_Vaild_File(DataSaveExpInfo);
			if(ucResult!=SUCCESS)
				return ucResult;

			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(3,aucBuf,&iLen,6);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(!memcmp(&aucBuf[2],"0001",4))
		    	{
		    	 	memcpy(&aucProcessCode[2],"0001",4);
			       		continue;
		 	}
			if(!memcmp(&aucBuf[2],"0000",4))
		          break;	
		}
		else
			return ucResult;
		
	}
	return ucResult;
}
unsigned char ExpLogonProcess(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char aucBuf[10],aucCashierNo[10],aucCashierPass[20];
	unsigned char strBuf[17];
	short iLen;
	DataSave0.ChangeParamData.ucExpFlag = FLAG_LOGON_UNDO;
	DataSave0.ChangeParamData.ucExpFlag1 = FLAG_LOGON_UNDO;
	XDATA_Write_Vaild_File(DataSaveChange);
	ucResult=ExpLogonOnline();
	if( ucResult == SUCCESS)			
	{
		if(NormalTransData.ucTransType==TRANS_S_SALEONLINE)
			DataSave0.ChangeParamData.ucExpFlag1 = FLAG_LOGON_OK;
		else
			DataSave0.ChangeParamData.ucExpFlag = FLAG_LOGON_OK;
		XDATA_Write_Vaild_File(DataSaveChange);
	}
	return	ucResult;
}	

unsigned char CashierLogonProcess(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char aucBuf[10],aucCashierNo[10],aucCashierPass[20];
	unsigned char strBuf[17],uctHf,uctCT;
	
	if(DataSave0.ChangeParamData.ucInilizeFlag!=FLAG_INIT_OK)
	{
		return ERR_NOTINIT;
	}

	if(DataSave0.ChangeParamData.ucIsLogonFlag!=FLAG_LOGON_OK)
	{
		return ERR_NOTLOGON;
	}
	uctHf=NormalTransData.ucHandFlag;
	uctCT=NormalTransData.ucCardType;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"Ա");
	//Os__GB2312_display(1,0,(unsigned char *)"[ȷ]");
	ucResult = SLE4442_WaitInsert();
	if(ucResult==KEY_ENTER)
	{
		NormalTransData.ucHandFlag=1;
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"Ա");
		memset(aucCashierNo,0,sizeof(aucCashierNo));
		if( UTIL_Input(1,true,6,6,'N',aucCashierNo,NULL) != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}
		memcpy(RunData.aucCashierNo,aucCashierNo,6);
		ucResult = SUCCESS;
	}
	else
	{
		if(ucResult == SUCCESS )
		{
			SLE4442_ATR(strBuf);
		}

		//ǲǷǷ
		if(ucResult==SUCCESS)
		{
			ucResult=SMC_CheckCard();
		}

		if(ucResult==SUCCESS)
		{
			ucResult = SLE4442_ReadCardData();
		}
		if(ucResult == SUCCESS)
		{
			if(!memcmp(NormalTransData.aucUserCardNo,"\x44\x44\x44\x44",4))
				bcd_asc(RunData.aucCashierNo,&NormalTransData.aucUserCardNo[5],6);
			else
				ucResult=ERR_CARDNOFUN;
		}
	}
	if(ucResult == SUCCESS)
		ucResult = UTIL_InputCashierPIN();

	if(ucResult == SUCCESS)
		ucResult = CashierLogonOnline();
	
	if(ucResult==SUCCESS)
	{
		DataSave0.ChangeParamData.ucIsCashierLogonFlag = FLAG_LOGON_OK;
		memcpy(DataSave0.ChangeParamData.aucCashierNo,RunData.aucCashierNo,6);
		memcpy(NormalTransData.aucCashierNo,RunData.aucCashierNo,6);
		//memcpy(DataSave0.ChangeParamData.aucCashierPin,RunData.aucPin,8);
		XDATA_Write_Vaild_File(DataSaveChange);
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"Աǩɹ!");
		UTIL_GetKey(3);
	}
	if( ucResult != SUCCESS)			
	{
		DataSave0.ChangeParamData.ucIsCashierLogonFlag = FLAG_LOGON_UNDO;
		XDATA_Write_Vaild_File(DataSaveChange);
	}
	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(uchar *)"γ");
	Os__xdelay(10);
	Os__ICC_remove();

	NormalTransData.ucHandFlag=uctHf;
	NormalTransData.ucCardType=uctCT;
	//return SUCCESS;
	return	ucResult;
}


unsigned char CashierChangepinOnline(void)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[300],aucProcessCode[6];
	unsigned char 	*ptBlack,*ptCardtable,*ptZone;
	unsigned short  uiBlackNum,uiTableNum,uiZoneNum,i;
	
	
	ucResult = SUCCESS;
	memcpy(aucProcessCode,"960000",6);
	uiBlackNum=0;  
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(800);
	if(!NormalTransData.ucHandFlag)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
		ISO8583_SetBitValue(2,aucBuf,16);
	}
	// Set process code bit 3
	ISO8583_SetBitValue(3,aucProcessCode,6);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);
	UTIL_IncreaseTraceNumber(0);	
	//Set bit 22
	memset(aucBuf,0,sizeof(aucBuf));
	if(NormalTransData.ucHandFlag)
		aucBuf[1]='1';
	ISO8583_SetBitValue(22,aucBuf,4);
	//Set bit 26 
	ISO8583_SetBitHexValue(26,&RunData.ucPtcode,1);
	
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);
	// Set bit 44
	//ISO8583_SetBitValue(44,COM_VERSION,5);
	// Set bit 47 cashier id
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucCashierNo,6);
	ISO8583_SetBitValue(47,aucBuf,6);
	// Set bit 47 cashier password
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucPin,8);
	ISO8583_SetBitHexValue(52,aucBuf,8);

	// Set bit 61 cashier password
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,RunData.aucNewPin,8);
	ISO8583_SetBitHexValue(53,aucBuf,8);
	
	ucResult = COMMS_TransSendReceive();

	return ucResult;
}


unsigned char Empty(void)
{
return SUCCESS;
}

unsigned char CashierChangepin(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char aucBuf[10],aucCashierNo[10],aucCashierPass[20],aucPt[5],aucZDFunFlag[64];
	unsigned char strBuf[17];
	int i;
	SELMENU ManagementMenu;
	ucResult =CFG_CheckFlag();
	if(ucResult != SUCCESS)
	{
		return ucResult;
	}
	memset(aucPt,0,sizeof(aucPt));
	for(i=0;i<DataSave0.ChangeParamData.uiZoneCodeNum;i++)
	{
		ucResult=XDATA_Read_ZoneCtrls_File(i);
		if(ucResult!=SUCCESS)
			return ucResult;
		UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucFunction,8,aucZDFunFlag);
		if(DataSave1.ZoneCtrl.ucPtCode>0&&DataSave1.ZoneCtrl.ucPtCode<10)
		{
			if(aucZDFunFlag[52])
				aucPt[DataSave1.ZoneCtrl.ucPtCode-1]=1;
		}
	}
	memset(&ManagementMenu,0,sizeof(SELMENU));
	UTIL_GetMenu_Value(1,MSG_WKCHPIN,Empty,&aucPt[0],&ManagementMenu);
	UTIL_GetMenu_Value(2,MSG_SMTCHPIN,Empty,&aucPt[1],&ManagementMenu);
	UTIL_GetMenu_Value(3,MSG_HYCHPIN,Empty,&aucPt[2],&ManagementMenu);
	
	if(!memcmp(aucPt,"\x00\x00\x00",3))
		return ERR_NOFUNCTION;
	if(!memcmp(aucPt,"\x01\x00\x00",3))
	{
		RunData.ucPtcode=1;
	}else
	if(!memcmp(aucPt,"\x00\x01\x00",3))
	{
		RunData.ucPtcode=2;
	}else
	if(!memcmp(aucPt,"\x00\x00\x01",3))
	{
		RunData.ucPtcode=3;
	}else
	{
		ucResult = UTIL_DisplayMenu(&ManagementMenu);
		RunData.ucPtcode=NormalTransData.ucTransType;
	}
	if(ucResult != SUCCESS)
	{
		return ucResult;
	}
	
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_PreComm();
	}
#ifdef GUI_PROJECT
    memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
    ProUiFace.ProToUi.uiLines=2;
    memcpy(ProUiFace.ProToUi.aucLine1,"Ա",13);
    memcpy(ProUiFace.ProToUi.aucLine2,"[ȷ]",13);
    sleep(1);
#else
	//Os__clr_display(255);
    //Os__GB2312_display(0,0,(unsigned char *)"Ա");s
	//Os__GB2312_display(1,0,(unsigned char *)"[ȷ]");
#endif
	ucResult = SLE4442_WaitInsert();
	if(ucResult==KEY_ENTER)
	{
		NormalTransData.ucHandFlag=1;
#ifdef GUI_PROJECT
    memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
    ProUiFace.ProToUi.uiLines=1;
    memcpy(ProUiFace.ProToUi.aucLine1,"Ա",13);
    sleep(1);
#else
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"Ա");
#endif
		memset(aucCashierNo,0,sizeof(aucCashierNo));
		if( UTIL_Input(1,true,6,6,'N',aucCashierNo,NULL) != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}
		memcpy(RunData.aucCashierNo,aucCashierNo,6);
		ucResult = SUCCESS;
	}
	else
	{
		if(ucResult == SUCCESS )
		{
			SLE4442_ATR(strBuf);
		}

		//ǲǷǷ
		if(ucResult==SUCCESS)
		{
			ucResult=SMC_CheckCard();
		}

		if(ucResult==SUCCESS)
		{
			ucResult = SLE4442_ReadCardData();
		}
		if(ucResult == SUCCESS)
		{
			if(!memcmp(NormalTransData.aucUserCardNo,"\x44\x44\x44\x44",4))
				bcd_asc(RunData.aucCashierNo,&NormalTransData.aucUserCardNo[5],6);
			else
				ucResult=ERR_CARDNOFUN;
		}
	}
	if(ucResult == SUCCESS)
	{
		ucResult = UTIL_ChangeCashierPIN();
	}
	
	if(ucResult == SUCCESS)
		ucResult = CashierChangepinOnline();
	
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_FinComm();
	if(ucResult==SUCCESS)
	{
#ifdef GUI_PROJECT
    memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
    ProUiFace.ProToUi.uiLines=1;
    memcpy(ProUiFace.ProToUi.aucLine1,"Ա",14);
    sleep(1);
#else
		//DataSave0.ChangeParamData.ucIsCashierLogonFlag = FLAG_LOGON_OK;
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"Աܳɹ!");
		UTIL_GetKey(3);
#endif
	}
#ifdef GUI_PROJECT
    memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
    ProUiFace.ProToUi.uiLines=2;
    memcpy(ProUiFace.ProToUi.aucLine1,"Ա",8);
    sleep(1);
#else
	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(uchar *)"γ");
#endif
	Os__xdelay(10);
	Os__ICC_remove();
	
	
	return ucResult;
}
unsigned char LOGON_DownTransKey(void)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[600],aucProcessCode[6],aucBase[2];
	unsigned char 	*ptCardtable,aucBit25[2];
	unsigned short  	uiTableNum,i;
	int j;
	//if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	//	COMMS_PreComm();

	ucResult = SUCCESS;
	memcpy(aucProcessCode,"980000",6);
	uiTableNum=0;  
	i=0;j=0;
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(800);
	// Set process code bit 3
	ISO8583_SetBitValue(3,aucProcessCode,6);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);
	UTIL_IncreaseTraceNumber(0);	
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);

	ucResult = COMMS_TransSendReceive();
	if(ucResult==SUCCESS)
	{
		//get card table bit 60
		if((ISO8583_CheckBit(61)==SUCCESS)&&(ISO8583Data.Bit[61-1].uiLen !=0))
		{			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
			if(ucResult!=SUCCESS)
				return ucResult;
			if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
				COMMS_FinComm();
			for(i=KEYARRAY_PURSECARD0;i<KEYARRAY_PURSECARD0+20;i++)
			{
				ucResult=PINPAD_46_LoadKey(i,KEYINDEX_PURSECARD_MASTERKEY, aucBuf);

	Uart_Printf("\n 2012-06-25 0001 PINPAD_46_LoadKey:i=%d ucRst:%02x\n",i, ucResult);
					
				if(ucResult!=SUCCESS)
				{
					Uart_Printf("\n 2012-06-25 0001.01 PINPAD_46_LoadKey: ucRst:%02x\n", ucResult);		
					ucResult=LoadKey(&aucBuf[0],i,KEYINDEX_PURSECARD_PROTECTKEY,KEYINDEX_PURSECARD_MASTERKEY);
		                   ucResult=LoadKey(&aucBuf[8],i,KEYINDEX_PURSECARD_PROTECTKEY,KEYINDEX_PURSECARD_MASTERKEY+1);
				}
				ucResult=CheckKeyVeriCode(&aucBuf[16],i,KEYINDEX_PURSECARD_MASTERKEY);

	Uart_Printf("\n 2012-06-25 0002 PINPAD_46_LoadKey:i=%d ucRst:%02x\n",i, ucResult);
	Uart_Printf("\n 2012-06-25 0003 PINPAD_46_LoadKey:i=%d ucRst:%02X %02X\n",i, aucBuf[16],aucBuf[17]);
	
				if(ucResult!=SUCCESS)
				{
					return ucResult;	
				}
			}
		}
	}
	if(ucResult==SUCCESS)
	{
		DataSave0.ConstantParamData.ucTranskeyflag=1;
		XDATA_Write_Vaild_File(DataSaveConstant);
		/*memset(aucBuf,0,sizeof(aucBuf));
		PINPAD_47_Encrypt8ByteSingleKey(KEYARRAY_PURSECARD0, KEYINDEX_PURSECARD_MASTERKEY, aucBuf, aucBuf);
		for(i=KEYARRAY_PURSECARD0;i<KEYARRAY_PURSECARD0+20;i++)
		{
			PINPAD_42_LoadSingleKeyUseSingleKey(i, KEYINDEX_PURSECARD_MASTERKEY, KEYINDEX_PURSECARD_PROTECTKEY, aucBuf);
			PINPAD_42_LoadSingleKeyUseSingleKey(i, KEYINDEX_PURSECARD_MASTERKEY, KEYINDEX_PURSECARD_PROTECTKEY+1, aucBuf);
		}*/

	}
	return ucResult;
}
unsigned char LOGON_DownTransKeyProcess(void)
{
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
		COMMS_PreComm();
	return LOGON_DownTransKey();
}

unsigned char LOGON_SyncOnline(void)
{
	unsigned char ucResult=SUCCESS,aucProcessCode[6];
	unsigned char aucBuf[400],t,transbuf[20],aucTreat[10];
	short 		iLen;
	int i;
	memset(aucProcessCode,0,sizeof(aucProcessCode));
	memcpy(aucProcessCode,"910000",6);

	ISO8583_Clear();
	
	ISO8583_SetMsgID(800);
	
	ISO8583_SetBitValue(3,aucProcessCode,6);
	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);
	
	UTIL_IncreaseTraceNumber(0);

	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);

	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);

	//ISO8583_SetBitValue(44,COM_VERSION,5);

	memset(aucBuf,0,sizeof(aucBuf));
	UTIL_PinVersionAnalyze(DataSave0.ConstantParamData.aucPinpadVersion,14,&aucBuf[60]);
	memcpy(&aucBuf[80],CURRENT_PRJ,sizeof(CURRENT_PRJ));
	memcpy(&aucBuf[110],CURRENT_VER,sizeof(CURRENT_VER));
	memcpy(&aucBuf[140],COM_VERSION,5);
	ISO8583_SetBitValue(60,aucBuf,145);
	
	ucResult = COMMS_TransSendReceive();
	
	if(ucResult==SUCCESS)
	{
		memcpy(aucTreat,DataSave0.ConstantParamData.aucTreat,10);
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = ISO8583_GetBitValue(60,aucBuf,&iLen,sizeof(aucBuf));
		if(ucResult==SUCCESS)
		{
			memcpy(DataSave0.ConstantParamData.aucMerchantName,aucBuf,30);
			memcpy(DataSave0.ConstantParamData.aucHostTelNumber1,&aucBuf[30],15);
			memcpy(DataSave0.ConstantParamData.aucHostTelNumber2,&aucBuf[45],15);
			memcpy(DataSave0.ConstantParamData.aucHostTelNumber3,&aucBuf[60],15);	
			memcpy(DataSave0.ConstantParamData.aucHostTelNumber4,&aucBuf[75],15);	
			DataSave0.ConstantParamData.ucAccessType=aucBuf[90];
			DataSave0.ConstantParamData.ucConnectType=aucBuf[91];
			memset(transbuf,0,sizeof(transbuf));
			memcpy(transbuf,&aucBuf[92],15);
			UTIL_IPAnalyze(transbuf,&DataSave0.ConstantParamData.ulHostIPAdd1);
			DataSave0.ConstantParamData.uiHostPort1=asc_long(&aucBuf[107],8);
			memset(transbuf,0,sizeof(transbuf));
			memcpy(transbuf,&aucBuf[115],15);
			UTIL_IPAnalyze(transbuf,&DataSave0.ConstantParamData.ulHostIPAdd2);
			DataSave0.ConstantParamData.uiHostPort2=asc_long(&aucBuf[130],8);
			memcpy(DataSave0.ConstantParamData.aucHostDomain,&aucBuf[138],30);
			if(DataSave0.TransInfoData.TransTotal.uiTotalNb==0)
			{
				memcpy(RunData.aucDownKey,&aucBuf[168],10);
				memcpy(DataSave0.ConstantParamData.aucTreat,&aucBuf[168],10);
			}else
			{
				if(memcmp(aucTreat,&aucBuf[168],10))
					DataSave0.ChangeParamData.ucSettleFlag=FLAG_SETTLE_FAIL;
			}
			memcpy(DataSave0.ConstantParamData.aucMid,&aucBuf[178],24);
			memcpy(DataSave0.ConstantParamData.aucTid,&aucBuf[202],12);
			ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
			if(ucResult!=SUCCESS)return ucResult;
			ucResult=XDATA_Write_Vaild_File(DataSaveChange);
			if(ucResult!=SUCCESS)return ucResult;
			
		}

	}
	if(ucResult==SUCCESS)
	{
		if(memcmp(aucTreat,DataSave0.ConstantParamData.aucTreat,10))
		{
			ucResult=LOGON_DownKey();
		}
	}
	return ucResult;
}
unsigned char LOGON_ComfirmApp(uchar successflag,ulong offset)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[600],aucProcessCode[6];

	ucResult = SUCCESS;
	memcpy(aucProcessCode,"090000",6);
	ISO8583_Clear();
	// Set Msgid 
	ISO8583_SetMsgID(800);
	// Set process code bit 3
	ISO8583_SetBitValue(3,aucProcessCode,6);
	
	// Set  bit 9
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,CURRENT_PRJ,sizeof(CURRENT_PRJ));
	ISO8583_SetBitValue(9,aucBuf,20);
	// Set  bit 10 
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,CURRENT_VER,sizeof(CURRENT_VER));
	ISO8583_SetBitValue(10,aucBuf,10);
	
	// Set system trace audit number bit 11 	
	memset(aucBuf,0,sizeof(aucBuf));
	long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
	ISO8583_SetBitValue(11,aucBuf,6);
	UTIL_IncreaseTraceNumber(0);	
	
	// Set Card Acceptor Terminal Identification bit 41
	ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
	// Set Card Acceptor Identification Code bit 42 
	ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);
	
	memset(aucBuf,0,sizeof(aucBuf));
	aucBuf[0]='1';
	long_bcd(&aucBuf[1],4,&offset);
	aucBuf[5]=successflag;
	ISO8583_SetBitValue(61,aucBuf,6);

	
	ucResult = COMMS_TransSendReceive();
	if(ucResult==SUCCESS)
	{
	}
	return ucResult;
}
unsigned char LOGON_DownApp(void)
{
	short 		  	iLen;
	unsigned char 	ucResult,aucTime[6],aucDate[6];
	unsigned char 	aucBuf[600],aucProcessCode[6];
	unsigned short  	uiZoneNum,i,uiNowNum;
	unsigned char 	*Ptr;
	unsigned long		uloffset,uli,ulAddr;
	void(*Reboot)(void)=(void(*)(void))0x00000000;
	ucResult = SUCCESS;
	memcpy(aucProcessCode,"090000",6);
	uiZoneNum=0;  
	Ptr=(uchar *)0xC500004;
	uloffset=0;
	while(1)
	{
		ISO8583_Clear();
		// Set Msgid 
		ISO8583_SetMsgID(800);
		// Set process code bit 3
		ISO8583_SetBitValue(3,aucProcessCode,6);
		// Set  bit 9
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,CURRENT_PRJ,sizeof(CURRENT_PRJ));
		ISO8583_SetBitValue(9,aucBuf,20);
		// Set  bit 10 
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,CURRENT_VER,sizeof(CURRENT_VER));
		ISO8583_SetBitValue(10,aucBuf,10);
		
		// Set system trace audit number bit 11 	
		memset(aucBuf,0,sizeof(aucBuf));
		long_asc(aucBuf,6,&NORMALTRANS_ulTraceNumber);
		ISO8583_SetBitValue(11,aucBuf,6);
		UTIL_IncreaseTraceNumber(0);	
		
		// Set Card Acceptor Terminal Identification bit 41
		ISO8583_SetBitValue(41,DataSave0.ConstantParamData.aucTerminalID,8);
		// Set Card Acceptor Identification Code bit 42 
		ISO8583_SetBitValue(42,DataSave0.ConstantParamData.aucMerchantID,15);
		
		memset(aucBuf,0,sizeof(aucBuf));
		aucBuf[0]='0';
		long_bcd(&aucBuf[1],4,&uloffset);
		ISO8583_SetBitValue(61,aucBuf,6);

		ucResult = COMMS_TransSendReceive();
		if(ucResult==SUCCESS)
		{
			//get blackcard list bit 61 Zone control code
			if((ISO8583_CheckBit(61)==SUCCESS)&&(ISO8583Data.Bit[61-1].uiLen !=0))
			{
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult=ISO8583_GetBitValue(61,aucBuf,&iLen,sizeof(aucBuf));
				if(ucResult!=SUCCESS)
					return ucResult;
				uli=5;
				while(1)
				{
					if(aucBuf[uli]=='E')
					{
						if(aucBuf[uli+1]=='N'&&aucBuf[uli+2]=='D')
						break;
					}
					uli++;
				}
				for(ulAddr=0;ulAddr<uli-5;ulAddr++)
				{
					*Ptr=aucBuf[ulAddr+5];
					Ptr++;
				}
				uloffset+=(uli-5);
			}
			
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult=ISO8583_GetBitValue(3,aucBuf,&iLen,6);
			if(ucResult!=SUCCESS)
				return ucResult;
			if(!memcmp(aucBuf,"090001",6))
		    	{
		    	 	memcpy(aucProcessCode,"090001",6);
			       		continue;
		 	}
			if(!memcmp(aucBuf,"090000",6))
		          break;	
		}
		else
			return ucResult;
		
	}
	if(ucResult==SUCCESS)
	{
		ucResult=LOGON_ComfirmApp('0',uloffset);
	}	
	else
	{
		LOGON_ComfirmApp('1',uloffset);
	}	
	if(ucResult==SUCCESS)
	{
		for(ulAddr=0;ulAddr<uloffset;ulAddr++)
		{
			if(ulAddr%16==0)
				Uart_Printf("\n");
			Uart_Printf("%02x ",*((uchar *)(0xC500004+ulAddr)));
		}
		Uart_Printf("\nuloffset=%d\n",uloffset);
//		ucResult=OSTMS_Save_File(1,1,uloffset);
		Uart_Printf("\nucResult=%02x\n",ucResult);
	}
	return ucResult;	
}

