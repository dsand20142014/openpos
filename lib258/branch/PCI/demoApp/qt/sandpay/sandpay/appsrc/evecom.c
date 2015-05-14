#include <include.h>
#include <global.h>
#include <xdata.h>
unsigned char TMS_WriteFile(void);

unsigned char OnEve_power_on(void)
{
    unsigned char ucI,ucResult,ucKey,i,j,aucBuf[20],ucStatus,aucOutData[50],ucLen;
	int iFileSize;
    int ret;
	
	Uart_Printf("**************************In check all file **************************\n",ucResult);
	Uart_Printf("ɼ֧\n");
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,CURRENT_PRJ,sizeof(CURRENT_PRJ));
	Uart_Printf("%s\n",aucBuf);
	for(i=0;i<15;i++)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,FileInfo[i].aucFileName,10);
        Uart_Printf("%s\n",aucBuf);
        iFileSize=OSAPP_FileSize((char *)FileInfo[i].aucFileName);
        Uart_Printf("%d\n",iFileSize);
	}
	if(xDATA_ConstantSize>=BaseSize||xDATA_ChangeSize>=BaseSize
		||xDATA_TransInfoSize>=BaseSize||xDATA_BackupISO8583Size>=BaseSize
		||xDATA_ReversalISO8583Size>=BaseSize||xDATA_SaveTransSize>=BaseSize
		||xDATA_BatchFlagsSize>=BaseSize||xDATA_BlackFeildsSize>=BaseSize
		||xDATA_ZoneCtrlsSize>=BaseSize||xDATA_CardTablesSize>=BaseSize
		||xDATA_RedoTransSize>=BaseSize||xDATA_RedoTransSize>=BaseSize
		||xDATA_RateSize>=BaseSize)
	{
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(uchar *)"ļ");
        Uart_Printf("*********************有文件过大*********************\n");
		while(1);
	}
    //Os__clr_display(255);
    //Os__GB2312_display(0,0,"ļ...");
	ucResult = FILE_CheckConstantParamData();
	
	if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"ļһ!");
        //Os__xget_key();
        Uart_Printf("*********************Constant参数文件一有误!*********************\n");
        return;
    }
	 
    ucResult = FILE_CheckChangeParamData();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"ļ!");
        //Os__xget_key();
        Uart_Printf("*********************Constant参数文件二有误!*********************\n");

        return;
    }
		
	ucResult = FILE_CheckTransInfoData();
	if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"ͳļ!");
        //Os__xget_key();
        Uart_Printf("*********************统计文件有误!*********************\n");

        return;
    }
	  
    ucResult = FILE_CheckSaveISO8583();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"SaveISOļ");
        //Os__xget_key();
        Uart_Printf("*********************SaveISO文件有误!*********************\n");

        return;
    }
    
    ucResult = FILE_CheckSendISO8583();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"SendISOļ");
        //Os__xget_key();
        Uart_Printf("*********************SendISO文件有误!*********************\n");

        return;
    }
    
    ucResult = FILE_CheckReversalISO8583();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"ReverISOļ");
        //Os__xget_key();
        Uart_Printf("*********************ReverISO文件有误!*********************\n");

        return;
    }
    
    ucResult = FILE_CheckSaveTrans();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"ױļ");
        //Os__xget_key();
        Uart_Printf("*********************交易保存文件有误!*********************\n");

        return;
    }
    
    ucResult = FILE_CheckBatchFlags();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"BatchFlaļ");
        //Os__xget_key();
        Uart_Printf("*********************BatchFla文件有误!*********************\n");

        return;
    }

    ucResult = FILE_CheckBlackField();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"BlackFeļ");
        //Os__xget_key();
        Uart_Printf("*********************BlackFe文件有误!*********************\n");

        return;
    }

    ucResult = FILE_CheckZoneCtrls();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"ZoneCtrlļ");
        //Os__xget_key();
        Uart_Printf("*********************ZoneCtrl文件有误!*********************\n");

        return;
    }
    
    ucResult = FILE_CheckCardTables();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"CardTablļ");
        //Os__xget_key();
        Uart_Printf("*********************CardTabl文件有误!*********************\n");

        return;
    }
    
    ucResult = FILE_CheckRedo();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"Redoļ");
        //Os__xget_key();
        Uart_Printf("*********************Redo文件有误!*********************\n");

        return;
    }
    ucResult = FILE_CheckRateTables();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"Rateļ");
        //Os__xget_key();
        Uart_Printf("*********************Rate文件有误!*********************\n");

        return;
    }
    ucResult = FILE_CheckTemp();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"Tempļ");
        //Os__xget_key();
        Uart_Printf("*********************Temp文件有误!*********************\n");

        return;
    }
    ucResult = FILE_CheckRateTables();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"Rateļ");
        //Os__xget_key();
        Uart_Printf("*********************Rate文件有误!*********************\n");

        return;
    }
    ucResult = FILE_CheckTemp();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"Tempļ");
        //Os__xget_key();
        Uart_Printf("*********************Temp文件有误!*********************\n");

        return;
    }
    ucResult = FILE_CheckIssueInfo();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"IssueInfoļ");
        //Os__xget_key();
        Uart_Printf("*********************IssueInfo文件有误!*********************\n");

        return;
    }
    ucResult = FILE_CheckLastIssueInfo();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"LastIssueInfoļ");
        //Os__xget_key();
        Uart_Printf("*********************LastIssueInfo文件有误!*********************\n");

        return;
    }
    ucResult = FILE_CheckExpInfo();
    if( ucResult != SUCCESS )
    {
        //Os__GB2312_display(1,0,(unsigned char*)"ExpInfoļ");
        //Os__xget_key();
        Uart_Printf("*********************ExpInfo文件有误!*********************\n");

        return;
    }
    //NETCOMMS_Load_Default_Key();
    //SANDPAY_LoadKey(1,4);
  	MAC_Load_Key();
    //if(ucResult == SUCCESS )
    //    TMSAPP_CREATETMSFile();
    //Uart_Printf("TMSAPP_CREATETMSFile=\n");
    //if( ucResult == SUCCESS )
    //    TMSAPP_UpdateTMSData(TMS_WriteFile);

    if(ucResult == SUCCESS )
        ucResult = MIFARE_GetVersion();
    {
        Uart_Printf("\nThis Is PinPad Receive Data [ DrvOut.xxdata ]\n");
        for(i = 0; i <  11; i++)
        {
            Uart_Printf("%02x ",MifareCommandData.aucCommandOut[i]);
        }
        Uart_Printf("\n");
    }
    if(ucResult==SUCCESS)
    {
        Uart_Printf("Check version success\n");
        memcpy(DataSave0.ConstantParamData.aucPinpadVersion,&MifareCommandData.aucCommandOut[1],2);
        memcpy(&DataSave0.ConstantParamData.aucPinpadVersion[10],&MifareCommandData.aucCommandOut[3],4);
        ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
    }
    else
    {
        Uart_Printf("Check version failed");
        memset(DataSave0.ConstantParamData.aucPinpadVersion,0,14);
        ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
        //Os__clr_display(255);
        //Os__GB2312_display(1,0,(unsigned char*)"ǽģ!");
        Uart_Printf("*********************PINPAD版本有误!*********************\n");

        //while(1);
    }
    return(ucResult);
}

unsigned char OnEve_KeyPress(unsigned char ucKey)
{
    unsigned char ucResult;
    unsigned char aucBuf[10];
    unsigned int uiTimesOut;

    unsigned long 	ulResetAmount;
    unsigned char aucBuf1[7];
    unsigned char ucPassWd[5];
    
    ucResult = SUCCESS;
    RunData.ucInputKeyFlag=1;
    switch(ucKey)
    {
    case '1':
        //ǩ
	 if(DataSave0.ConstantParamData.ucKFCFlag)
	 {
	        ucResult = STL_Settle();
		 if(Commun_flag==0x32&&DataSave0.ConstantParamData.ucConnectType=='2')
			TCP_Close(RunData.iTCPHandle);
	 }
	 else
        	ucResult = MENU_OnlineManage();
        break;
    case '2':
        // 
	 if(DataSave0.ConstantParamData.ucKFCFlag)
        	ucResult = SEL_ReprintMenu();
	 else
	 {
		if(DataSave0.ConstantParamData.ucYTJFlag==2)
		{
        //	//Os__clr_display(255);
        //	//Os__GB2312_display(0,0,(uchar *)"");
			UTIL_GetKey(3);
			break;
		}
        	ucResult = STL_Settle();
		if(Commun_flag==0x32&&DataSave0.ConstantParamData.ucConnectType=='2')
			TCP_Close(RunData.iTCPHandle);
	 }
        break;
    case '3':
        // ش
	 if(DataSave0.ConstantParamData.ucKFCFlag)
        	ucResult = SEL_ServiceMenu();
	 else
         	ucResult = SEL_ReprintMenu();
        break;
    case '4':
        //ͳ
	 if(DataSave0.ConstantParamData.ucKFCFlag)
        	ucResult = MENU_OnlineManage();
	 else
        	ucResult = SEL_ServiceMenu();
        break;
    case '5':
        //
	 if(DataSave0.ConstantParamData.ucKFCFlag)
        	ucResult = Untouch_CardAdminSelectPt();
	 else
 	{
	        ucResult = SEL_ManaMenu();
		if(Commun_flag==0x32&&DataSave0.ConstantParamData.ucConnectType=='2')
			TCP_Close(RunData.iTCPHandle);
 	}
        break;
    case '6':
        //
	 if(DataSave0.ConstantParamData.ucKFCFlag)
 	 {
	        ucResult = SEL_ManaMenu();
		if(Commun_flag==0x32&&DataSave0.ConstantParamData.ucConnectType=='2')
			TCP_Close(RunData.iTCPHandle);
 	 }
	 else
 	{
		if(DataSave0.ConstantParamData.ucYTJFlag)
		{
            ////Os__clr_display(255);
    //		//Os__GB2312_display(0,0,(uchar *)"");
			UTIL_GetKey(3);
			break;
		}
	        ucResult = Untouch_CardAdminSelectPt();
 	}
        break;
    case '7':
		/**/if(DataSave0.ChangeParamData.aucTermFlag[6]
			&&!DataSave0.ConstantParamData.ucKFCFlag)
	       	 ucResult = Untouch_HandChoice();
		else
		{
			ucResult=Untouch_DX_Process();
		}
        break;
    case '8':
        //ϵͳ
          //  //Os__clr_display(255);
        //    //Os__GB2312_display(0,2,(unsigned char *)"ϵͳ˵");
//            //Os__GB2312_display(1,0,(unsigned char *)":");
            memset(ucPassWd,0,sizeof(ucPassWd));
            if( UTIL_Input(2,true,4,4,'P',ucPassWd,NULL) == KEY_ENTER )
            {
                if(!memcmp(ucPassWd,"\x38\x38\x38\x38",4))
                {
                    ucResult = MenuSys_Appli();
                }else if(!memcmp(ucPassWd,"2273",4))
                {
                    ucResult = CFG_ClearRom();
                }
            }
        break;
    case '9':
        //ucResult = CFG_ConstantParamVersion();
        ucResult = UTIL_SetFixAmount();
        break;
    default:
        break;
    }
   if(( ucResult != SUCCESS)&&( ucResult != ERR_CANCEL))	
		{
			MSG_DisplayErrMsg(ucResult);
		}
    return (SUCCESS);
}
