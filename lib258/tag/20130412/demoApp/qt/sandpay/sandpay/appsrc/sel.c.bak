#include <include.h>
#include <global.h>
#include <xdata.h>
#include <osgraph.h>

unsigned char SEL_ManaMenu(void)
{
	unsigned  char ucResult =SUCCESS;
	unsigned  char aucBuf[7];		
	if((Printer_flag)&&(Commun_flag))
	{
		memset(aucBuf,0,sizeof(aucBuf));
		MSG_DisplayMsg( true,0,0,MSG_SYSPASS);
		if (UTIL_Input(1,true,6,6,'P',aucBuf,0) != KEY_ENTER ) 
		{
			ucResult = ERR_CANCEL;
			return ucResult;
		}else
		{        
			if( memcmp(aucBuf,"918273",6) )
			{
				ucResult = ERR_SYSPASS;
				return ucResult;
			}
		}
		SEL_InitProcess();	
	}else
	{
		//Os__clr_display(255);		
		//Os__GB2312_display(0,0,( uchar *)"һγʼ");
		//Os__GB2312_display(1,0,( uchar *)"ôӡ");
		//Os__GB2312_display(2,0,( uchar *)"PINPADͨѶģʽ");
		UTIL_GetKey(10);
		UTIL_ClearGlobalData();
		UTIL_GetTerminalInfo();
		RunData.ucInputKeyFlag=1;
		SEL_InitProcess();	
	}
	return ucResult;
}

unsigned char SEL_InitProcess(void)
{
	unsigned char ucResult=SUCCESS,flag1=1,flag2=1,aucZDFunFlag[64];
	int i,j;
	SELMENU ManagementMenu;
	for(i=0;i<DataSave0.ChangeParamData.uiZoneCodeNum;i++)
	{
		ucResult=XDATA_Read_ZoneCtrls_File(i);
		if(ucResult!=SUCCESS)
			return ucResult;
		UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucFunction,8,aucZDFunFlag);
		for(j=0;j<64;j++)
			RunData.aucZDFunFlag[j]|=aucZDFunFlag[j];
	}
	if(RunData.aucZDFunFlag[44]!=1)
		flag2=0;
	while(1)
	{
		memset(&ManagementMenu,0,sizeof(SELMENU));
		if(DataSave0.ConstantParamData.ucTranskeyflag)
			flag1=0;
		else
			flag1=1;
			
		UTIL_GetMenu_Value(MSG_NULL,MSG_CONFIGBASEDATA,CFG_ConstantParamBaseData,NULL,&ManagementMenu);
		UTIL_GetMenu_Value(MSG_NULL,MSG_INIONLIE,LOGON_IniProcess,NULL,&ManagementMenu);
		UTIL_GetMenu_Value(MSG_NULL,SET_PRINTER,UTIL_SetPrinter,NULL,&ManagementMenu);
		//UTIL_GetMenu_Value(MSG_NULL,SET_PINPAD,UTIL_SetPinpad,NULL,&ManagementMenu);
		UTIL_GetMenu_Value(MSG_NULL,MSG_LOADAB,KEY_DownloadKeysFromCpuCard,NULL,&ManagementMenu);
		UTIL_GetMenu_Value(MSG_NULL,MSG_AMOUNTINPUT,CFG_SetAmountInput,NULL,&ManagementMenu);		
		UTIL_GetMenu_Value(MSG_NULL,MSG_CLEARSETTLE,CFG_ClearSettleProcess,NULL,&ManagementMenu);
		UTIL_GetMenu_Value(MSG_NULL,MSG_CLEARREVERSAL,CFG_ClearReversalProcess,NULL,&ManagementMenu);
		UTIL_GetMenu_Value(MSG_NULL,MSG_CLEARTRANSKEY,CFG_ClearTransKeyFlag,NULL,&ManagementMenu);
 
 
 
 		UTIL_GetMenu_Value(TRANS_S_INIONLINE,MSG_TRANSKEYDOWN,LOGON_DownTransKeyProcess,&flag1,&ManagementMenu);
	
	      ucResult = UTIL_DisplayMenu(&ManagementMenu);

		if(RunData.ucWVFlag==1)
		{
			uchar ucParamFlag[50];

			memset(ucParamFlag,0x01,sizeof(ucParamFlag));
//			OSAPP_WritePrivateFile(ucParamFlag);
         }
		if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
			COMMS_FinComm();
	      if(ucResult != SUCCESS)
	      {
			if(ucResult == ERR_CANCEL)
			{
				if((Printer_flag)&&(Commun_flag))
					return SUCCESS;
			}else if(ucResult!=SUCCESS)
			{		
				MSG_DisplayErrMsg(ucResult);
				if((Printer_flag)&&(Commun_flag))
				return SUCCESS;
			}
	      }
	}
	
	return SUCCESS;
}

unsigned char SEL_ReprintMenu(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[8];
	
	SELMENU ManagementMenu;
	memset(&ManagementMenu,0,sizeof(SELMENU));
	

	UTIL_GetMenu_Value(MSG_NULL,MSG_REPRINT,PRT_Reprint,NULL,&ManagementMenu);
	UTIL_GetMenu_Value(MSG_NULL,MSG_REPBYTRACE,PRT_ReprintbyTrace,NULL,&ManagementMenu);
    UTIL_GetMenu_Value(MSG_NULL,MSG_REPSETTLE,	PRT_ReprintSettle	,NULL,&ManagementMenu);
	
	if(DataSave0.ChangeParamData.aucTermFlag[4])
	{
		memset(aucBuf,0,sizeof(aucBuf));
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"");
		if (UTIL_Input(1,true,6,6,'P',aucBuf,0) != KEY_ENTER ) 
		{
			ucResult = ERR_CANCEL;
			return ucResult;
		}else
		{        
			if( memcmp(aucBuf,DataSave0.ConstantParamData.aucSuperPassword,6) )
			{
				ucResult = ERR_SYSPASS;
				return ucResult;
			}
		}
	}
	ucResult = UTIL_DisplayMenu(&ManagementMenu);
	
	
	return ucResult;	
}

unsigned char SEL_ServiceMenu(void)
{
	unsigned char ucResult=SUCCESS;
	
	SELMENU ManagementMenu;
	memset(&ManagementMenu,0,sizeof(SELMENU));
	

	UTIL_GetMenu_Value(MSG_NULL,MSG_LISTPRINT,PRT_Detail,NULL,&ManagementMenu);
	UTIL_GetMenu_Value(MSG_NULL,MSG_ENQURY_LIST,	CFG_QueryList	,NULL,&ManagementMenu);
	UTIL_GetMenu_Value(MSG_NULL,MSG_TERMINALTOTAL,	PRT_TerminalTt	,NULL,&ManagementMenu);
	
	ucResult = UTIL_DisplayMenu(&ManagementMenu);
	return ucResult;
}
unsigned char TMS_GetParam(void)
{
	unsigned char ucResult;
	
	ucResult = SUCCESS;
	//ucResult = TMS_Getparam_Manage();
	return(ucResult);
}

