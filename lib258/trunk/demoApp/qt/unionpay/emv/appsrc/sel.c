/*
	SAND China
--------------------------------------------------------------------------
	FILE  sel.c										(Copyright SAND 2001)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2001-07-02		Xiaoxi Jiang
    Last modified :	2001-07-02		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network
    Purpose :
        Source file for select event process.

    List of routines in file :

    File history :

*/

#include <include.h>
#include <global.h>
#include <xdata.h>
#include <emvinterface.h>
#include <py_input.h>
#ifdef TMS
#include <TMSApp.h>
#endif
//#include <emvparam.h>
//#include <emvinterface.h>
//#include <emvtrans.h>

unsigned char SEL_GetCardMsg(unsigned char ucKey)
{
	unsigned char aucCardNo[TRANS_ACCLEN*2+1];


	if((!DataSave0.ConstantParamData.ucInputCardNo)
		&&(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)
	  )
	{
		return(ERR_NOFUNCT);
	}

	if((DataSave0.ConstantParamData.ucInputCardNo ==0x31)//全不支持
	  )
	{
		return(ERR_NOFUNCT);
	}
	else if((DataSave0.ConstantParamData.ucInputCardNo ==0x32)
		&&G_NORMALTRANS_ucTransType != TRANS_PREAUTHFINISH)
	{
		return(ERR_NOFUNCT);
	}
		util_Printf("\n .20120615. SEL_GetCardMsg.......\n");
	
	MSG_DisplayMsg( true,0,0,MSG_CARDNO);
	memset(aucCardNo,0,sizeof(aucCardNo));
	G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_MANUALINPUT;
	aucCardNo[0] = ucKey;
	if (UTIL_Input(2,false,12,19,'N',aucCardNo,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}else
	{
		G_NORMALTRANS_ucSourceAccLen = strlen((char *)aucCardNo);
		asc_bcd(G_NORMALTRANS_aucCardPan,6,&aucCardNo[G_NORMALTRANS_ucSourceAccLen-13],12);
#ifdef TEST
		util_Printf("G_NORMALTRANS_ucSourceAccLen=%02x\n",G_NORMALTRANS_ucSourceAccLen);
		util_Printf("aucCardNo=%s\n",aucCardNo);
#endif
		if(G_NORMALTRANS_ucSourceAccLen%2)
		{
			asc_bcd(G_NORMALTRANS_aucSourceAcc,(G_NORMALTRANS_ucSourceAccLen+1)/2,
				aucCardNo,G_NORMALTRANS_ucSourceAccLen+1);
		}else
		{
			asc_bcd(G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen/2,
				aucCardNo,G_NORMALTRANS_ucSourceAccLen);
		}
#ifdef TEST
		for(ucI=0;ucI<(G_NORMALTRANS_ucSourceAccLen+1)/2;ucI++)
			util_Printf("%02x",G_NORMALTRANS_aucSourceAcc[ucI]);
		util_Printf("\n");
#endif

	}
	if(G_NORMALTRANS_ucTransType != TRANS_QUERYBAL)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"请输入卡有效期");
		//Os__GB2312_display(1,0,(unsigned char *)"(年年月月) :");
		//Os__GB2312_display(3,0,(unsigned char *)"跳过请按[确认]键");
		memset(aucCardNo,0,sizeof(aucCardNo));
		G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_MANUALINPUT;
		if (UTIL_Input(2,true,0,4,'N',aucCardNo,NULL) != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}else
		{
			asc_bcd(G_NORMALTRANS_ucExpiredDate,2,aucCardNo,4);
		}
	}
	return(SUCCESS);

}


unsigned char SEL_TestOnlineTrans(void)
{
	unsigned char ucResult;
	ucResult = SUCCESS;
	util_Printf("\n****ucPackgeHeadEve111 %02x",DataSave0.ChangeParamData.ucPackgeHeadEve);
	if(!DataSave0.ChangeParamData.ucPackgeHeadEve) COMMS_PreComm();
	DataSave0.ChangeParamData.euLoadParamType=LOADPARAMTYPE_MAG;
	XDATA_Write_Vaild_File(DataSaveChange);
	ucResult = LOGON_Online();
	util_Printf("\n****ucPackgeHeadEve222 %02x",DataSave0.ChangeParamData.ucPackgeHeadEve);
//	if(!DataSave0.ChangeParamData.ucPackgeHeadEve)
       COMMS_FinComm();
	if(ucResult == SUCCESS)
	{
		//Os__clr_display(255);
		if(G_NORMALTRANS_ucTransType == TRANS_LOADPARAM )
			//Os__GB2312_display(1, 0, (uchar *)"  参数下载成功");
		if(G_NORMALTRANS_ucTransType == TRANS_ONLINETEST )
			//Os__GB2312_display(1, 0, (uchar *)" 回响测试成功");
		if(G_NORMALTRANS_ucTransType == TRANS_SENDSTATS )
			//Os__GB2312_display(1, 0, (uchar *)" 状态上送成功");
		UTIL_WaitGetKey(10);
	}
	DataSave0.ChangeParamData.ucPackgeHeadEve = 0;
	XDATA_Write_Vaild_File(DataSaveChange);

	return(ucResult);

}
unsigned char SEL_SETKEYPARA(void)
{
	unsigned char ucResult,ucI;
	unsigned char aucBuf[50],aucIPBuf[20],aucTmpBuf[10];


	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char*)"选择远程密钥通讯");
	//Os__GB2312_display(1,0,(unsigned char*)"1.拨号 2.GPRS");
	//Os__GB2312_display(2,0,(unsigned char*)"3.TCP/IP");

	memset(aucBuf,0,sizeof(aucBuf));
	aucBuf[0] = DataSave0.ConstantParamData.ucKEYCOMM + '0';
	if (UTIL_Input(3,true,1,1,'N',aucBuf,(uchar *)"123") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}else
	{
		aucBuf[0] = aucBuf[0] - '0';
		Os__saved_copy(aucBuf,
			&DataSave0.ConstantParamData.ucKEYCOMM,1);
	}

	if (DataSave0.ConstantParamData.ucKEYCOMM == PARAM_DOWNKEYCOMM_HDLC) //拨号
	{
		//Tel1
		memset(aucBuf,0,sizeof(aucBuf));
		MSG_DisplayMenuMsg( true,0,0,MSG_HOSTTEL1);
		memcpy(aucBuf,DataSave0.ConstantParamData.aucHostTel1,PARAM_TELNUMBERLEN);
		for(ucI=0;ucI<PARAM_TELNUMBERLEN;ucI++)
		{
			if(aucBuf[ucI] == '.')
			{
				aucBuf[ucI] = 0;
				break;
			}
		}
              memset(&aucBuf[ucI],0,PARAM_TELNUMBERLEN-ucI);
		if (UTIL_Input(1,true,1,PARAM_TELNUMBERLEN,'N',aucBuf,0) != KEY_ENTER )
		{
			return (ERR_CANCEL);
		}else
		{
			aucBuf[strlen((char *)aucBuf)] = '.';
			Os__saved_copy(aucBuf,
				DataSave0.ConstantParamData.aucHostTel1,PARAM_TELNUMBERLEN);
		}

		//Tel2
		memset(aucBuf,0,sizeof(aucBuf));
		MSG_DisplayMenuMsg( true,0,0,MSG_HOSTTEL2);
		memcpy(aucBuf,DataSave0.ConstantParamData.aucHostTel2,PARAM_TELNUMBERLEN);
		for(ucI=0;ucI<PARAM_TELNUMBERLEN;ucI++)
		{
			if(aucBuf[ucI] == '.')
			{
				aucBuf[ucI] = 0;
				break;
			}
		}
              memset(&aucBuf[ucI],0,PARAM_TELNUMBERLEN-ucI);
		if (UTIL_Input(1,true,1,PARAM_TELNUMBERLEN,'N',aucBuf,0) != KEY_ENTER )
		{
			return (ERR_CANCEL);
		}else
		{
			aucBuf[strlen((char *)aucBuf)] = '.';
			Os__saved_copy(aucBuf,
				DataSave0.ConstantParamData.aucHostTel2,PARAM_TELNUMBERLEN);
		}
		XDATA_Write_Vaild_File(DataSaveConstant);
	}
	/*GPRS & TCP/IP*/
	if (DataSave0.ConstantParamData.ucKEYCOMM == PARAM_DOWNKEYCOMM_GPRS
		|| DataSave0.ConstantParamData.ucKEYCOMM == PARAM_DOWNKEYCOMM_TCPIP)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char*)"当前IP:");
		//Os__GB2312_display(3,0,(unsigned char*)"按[清除]键修改");

		memcpy(aucIPBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostKEYIP,4);
		if (OSUTIL_InputIPV4Addr(0,0x31,1,aucIPBuf) != KEY_ENTER )
		{
			return (ERR_CANCEL);
		}
		else
		{
			memcpy(&DataSave0.ConstantParamData.ulHostKEYIP, aucIPBuf, 4);
			util_Printf("KEY_IP=%d\n",DataSave0.ConstantParamData.ulHostKEYIP);
		}
		//Set Port
		memset(aucBuf,0,sizeof(aucBuf));
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (unsigned char *)"主机端口:");
		int_asc(aucBuf,6,&DataSave0.ConstantParamData.uiHostKEYPort);
		if (UTIL_Input(1,true,6,6,'N',aucBuf,0) != KEY_ENTER )
		{
			return (ERR_CANCEL);
		}
		else
		{
			DataSave0.ConstantParamData.uiHostKEYPort=asc_long(aucBuf,6);
		}
		util_Printf("\nuiHostKEYPort=%d",DataSave0.ConstantParamData.uiHostKEYPort);
		XDATA_Write_Vaild_File(DataSaveConstant);
	}

	//Set GPRS APN
	if (DataSave0.ConstantParamData.ucKEYCOMM == PARAM_DOWNKEYCOMM_GPRS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar*)"请输入APN:");
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,DataSave0.ConstantParamData.aucKEYAPN,26);
		if(UTIL_Input(3,true,1,30,'A',aucBuf,NULL) != KEY_ENTER)
		{
			return (ERR_CANCEL);
		}
		else
		{
			memcpy(DataSave0.ConstantParamData.aucKEYAPN,aucBuf,26);
		}
		XDATA_Write_Vaild_File(DataSaveConstant);
	}

	memset(aucBuf,0,sizeof(aucBuf));
	MSG_DisplayMenuMsg( true,0,0,MSG_TPDU);
	bcd_asc(aucBuf,DataSave0.ConstantParamData.aucKEYTpdu,10);
	if (UTIL_Input(1,true,10,10,'N',aucBuf,0) != KEY_ENTER )
	{
		ucResult = ERR_CANCEL;
	}else
	{
		memset(aucTmpBuf,0,sizeof(aucTmpBuf));
		asc_bcd(aucTmpBuf,5,aucBuf,10);
		Os__saved_copy(aucTmpBuf,(unsigned char *)&DataSave0.ConstantParamData.aucKEYTpdu,5);
	}
	XDATA_Write_Vaild_File(DataSaveConstant);

	return (SUCCESS);
}
unsigned char SEL_ChangeStats(void)
{
	unsigned char aucBuf[5];

	if(DataSave0.ChangeParamData.ucCashierLogonFlag==CASH_LOGONFLAG)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"----状态设置----");
		//Os__GB2312_display(1,0,(unsigned char *)" 终端为签到状态");
		//Os__GB2312_display(2,0,(unsigned char *)" 请签退后再试！");
		MSG_WaitKey(5);
		return(SUCCESS);
	}
	if(DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb
		+DataSave0.TransInfoData.TransTotal.uiTotalNb)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"----状态设置----");
		//Os__GB2312_display(1,0,(unsigned char *)" 终端尚有交易");
		//Os__GB2312_display(2,0,(unsigned char *)" 请结算后再试！");
		MSG_WaitKey(5);
		return(SUCCESS);
	}

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"----状态设置----");
	//Os__GB2312_display(1,0,(unsigned char*)"请选择类型:");
	//Os__GB2312_display(3,0,(unsigned char*)"0:测试    1:正常");\

	memset(aucBuf,0,sizeof(aucBuf));
	util_Printf("状态 = %02x\n",DataSave0.ChangeParamData.ucSuperLogonFlag);
	if(DataSave0.ChangeParamData.ucSuperLogonFlag == 0)
		aucBuf[0] = 0x31;
	else
		aucBuf[0] = 0x30;
	if(UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") !=KEY_ENTER)
	{
		return(MSG_DisplayErrMsg(ERR_CANCEL));
	}
	if(aucBuf[0] == '0')
	{
		LOGON_WriteSuperLogonFlag(CASH_LOGONFLAG);		/*状态设为测试交易*/
	}else
	{
		LOGON_WriteSuperLogonFlag(0);		/*状态设为正常交易*/
	}
	return(SYSCASH_SetTransType());
}

unsigned char SEL_LockKeyBoard(void)
{
	unsigned char aucCashierPass[9];
	unsigned char ucCashierIndex;
	unsigned char ucResult;

	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(uchar *)"   是否锁屏?");
	ucResult = Os__xget_key();
	if (ucResult == KEY_ENTER)
	{
		if(DataSave0.ChangeParamData.ucCashierLogonFlag!=CASH_LOGONFLAG)
		{
			return(ERR_CASH_NOTLOGON);
		}

		if(DataSave0.ChangeParamData.ucTerminalLockFlag)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"键盘已锁定");
			MSG_WaitKey(3);
			return(SUCCESS);
		}
		MSG_DisplayMenuMsg( true,0,0,MSG_CASHIERPASS);
		memset(aucCashierPass,0,sizeof(aucCashierPass));
		if( UTIL_Input(1,true,CASH_CASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}
		ucCashierIndex = DataSave0.ChangeParamData.ucCashierLogonIndex;
		if(DataSave0.ChangeParamData.ucCashierLogonFlag == 0x55)
		{
			if( (memcmp(aucCashierPass,&DataSave0.Cashier_SysCashier_Data.aucCashierPass[ucCashierIndex],CASH_CASHIERPASSLEN) ==0)
			  ||(memcmp(aucCashierPass,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,CASH_SYSCASHIERPASSLEN) == 0)
			 )
			{
				Os__saved_set(&DataSave0.ChangeParamData.ucTerminalLockFlag,1,1);
				XDATA_Write_Vaild_File(DataSaveChange);
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(unsigned char *)"键盘已锁定");
				MSG_WaitKey(3);
				return(SUCCESS);
			}
		}
		if(memcmp(aucCashierPass,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,CASH_SYSCASHIERPASSLEN) == 0)
		{
			Os__saved_set(&DataSave0.ChangeParamData.ucTerminalLockFlag,1,1);
			XDATA_Write_Vaild_File(DataSaveChange);
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"键盘已锁定");
			MSG_WaitKey(3);
			return(SUCCESS);
		}
		else
		{
			return(ERR_CASH_PASS);
		}
	}else if (ucResult == KEY_CLEAR) return (ERR_CANCEL);
	return (SUCCESS);
}

unsigned char SEL_UnLockKeyBoard(void)
{
	unsigned char aucCashierPass[CASH_MAXSUPERPASSWDLEN] ,aucCashierNo[CASH_MAXSUPERNO];
	unsigned char ucCashierIndex;
	unsigned char ucResult ;
	unsigned char FLAG = 0;

	while(1)
	{
		if(ucResult != SUCCESS)
			MSG_DisplayErrMsg(ucResult);

		ucResult = SUCCESS ;
		ucCashierIndex = DataSave0.ChangeParamData.ucCashierLogonIndex;

		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)" 键盘锁定请解锁");
		//Os__GB2312_display(1,0,(unsigned char *)" 请输入操作员号:");

		memset(aucCashierNo,0,sizeof(aucCashierNo));
		if( UTIL_Input(2,true,CASH_MAXSUPERNO,CASH_MAXSUPERNO,'N',aucCashierNo,NULL) != KEY_ENTER )
		{
			continue;
		}
		if( ucResult == SUCCESS)
		{
			if(DataSave0.ChangeParamData.ucCashierLogonFlag == 0x55)
			{
				if((memcmp(DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucCashierIndex] ,aucCashierNo ,CASH_CASHIERNOLEN ))
				&&(memcmp(DataSave0.Cashier_SysCashier_Data.aucSYSCashierNo ,aucCashierNo , CASH_SYSCASHIERNOLEN ))
				)
				{
					ucResult = ERR_CASH_NOTEXIST;
					continue;
				}else
				{

					if(!memcmp(DataSave0.Cashier_SysCashier_Data.aucSYSCashierNo ,aucCashierNo , CASH_SYSCASHIERNOLEN ))
					{
						FLAG = 1;
					}
					if(!memcmp(DataSave0.Cashier_SysCashier_Data.aucCashierNo[ucCashierIndex],aucCashierNo , CASH_CASHIERNOLEN ))
					{
						FLAG = 0;
					}
				}
			}else
			{
				if(memcmp(DataSave0.Cashier_SysCashier_Data.aucSYSCashierNo ,aucCashierNo , CASH_SYSCASHIERNOLEN)  )
				{
					ucResult = ERR_CASH_NOTEXIST;
					continue;
				}else
				{
					FLAG = 1;
				}
			}
		}
		//-----------CASH PASS-----
		if( ucResult == SUCCESS)
		{
			//Os__clr_display(1);
			//Os__GB2312_display(1,0,(unsigned char *)"请输入操作员密码:");
			memset(aucCashierPass,0,sizeof(aucCashierPass));
			if(FLAG == 0)
			{
				if( UTIL_Input(2,true,CASH_CASHIERPASSLEN,CASH_CASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
				{
					continue;
				}
			}else
			{
				if( UTIL_Input(2,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
				{
					continue;
				}
			}
		}
		//------------------------
		if(FLAG == 0)
		{
			if( memcmp(aucCashierPass,&DataSave0.Cashier_SysCashier_Data.aucCashierPass[ucCashierIndex],CASH_CASHIERPASSLEN) ==0)
			{
				ucResult =SUCCESS;
				break;
			}
			else
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(unsigned char *)"密码错误");
				MSG_WaitKey(3);
				ucResult =SUCCESS;
				continue;
			}
		}else
		{
			if(memcmp(aucCashierPass,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,CASH_SYSCASHIERPASSLEN) == 0)
			{
				DataSave0.ChangeParamData.ucCashierLogonFlag= 0 ;
				ucResult =SUCCESS;
				break;
			}
			else
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(unsigned char *)"密码错误");
				MSG_WaitKey(3);
				ucResult =SUCCESS;
				continue;
			}
		}
	}
	Os__saved_set(&DataSave0.ChangeParamData.ucTerminalLockFlag,0,1);
	XDATA_Write_Vaild_File(DataSaveChange);
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"锁定解除!");
	MSG_WaitKey(5);

}

unsigned char SEL_ServiceMenu(void)
{
	unsigned char ucResult;
	SELMENU ServiceMenu;
	memset(&ServiceMenu,0,sizeof(SELMENU));

	UTIL_GetMenu_Value(TRANS_REPRINT,	MSG_REPRINT,		NULL,	SERV_Reprint,&ServiceMenu);
	UTIL_GetMenu_Value(NULL,			MSG_REPRINTBYTRACE,	NULL,	SERV_ReprintByTrace,&ServiceMenu);
	UTIL_GetMenu_Value(NULL,			MSG_REPRINTSETTLE,		NULL,	PRINT_RePrint_List,&ServiceMenu);
	UTIL_GetMenu_Value(NULL,			MSG_LISTPRINT,				NULL,	PRINT_Detail_Menu,&ServiceMenu);
	UTIL_GetMenu_Value(NULL,			MSG_TANSTOTAL,			NULL,	SERV_ReprintSettle,&ServiceMenu);

	ucResult = UTIL_DisplayMenu(&ServiceMenu);

	if(( ucResult != SUCCESS)&&(ucResult != ERR_CANCEL)&&(ucResult != ERR_END))
	{
		MSG_DisplayErrMsg(ucResult);
	}

	return(SUCCESS);

}

unsigned char SEL_ManagementMenu(void)
{
	unsigned  char ucResult;
	unsigned  char aucBuf[CASH_MAXSUPERPASSWDLEN+1];
	SELMENU ManagementMenu;
	memset(&ManagementMenu,0,sizeof(SELMENU));

	UTIL_GetMenu_Value(NULL,				MSG_CONFIGBASEDATA,		NULL,	CFG_ConstantParamBaseData,	&ManagementMenu);
	UTIL_GetMenu_Value(NULL,				MSG_INPUTMASTERKEY, 		NULL,	SEL_SetMasterKey,			&ManagementMenu);
	UTIL_GetMenu_Value(NULL,				MSG_SET_SYSTRANS, 			NULL,	KEY_InputSYSTrans,			&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_LOADPARAM,MSG_LOADPARAM,			NULL,	SEL_ParamUpdata,			&ManagementMenu);

	UTIL_GetMenu_Value(NULL,				MSG_RATESET,						NULL,	UITL_Input_Rate,			&ManagementMenu);
	UTIL_GetMenu_Value(NULL,             MSG_PKDOWNLOAD,						NULL,   EMV_LoadCAPK,				&ManagementMenu);
	UTIL_GetMenu_Value(NULL,             MSG_EMVPARAMDOWNLOAD,		NULL,   EMV_LoadAIDParam,			&ManagementMenu);
	UTIL_GetMenu_Value(NULL,				MSG_CHANGESTATUS,		NULL,	SEL_ChangeStats,			&ManagementMenu);

	//UTIL_GetMenu_Value(NULL,				MSG_SETKEYPARA,				NULL,	SEL_SETKEYPARA,			&ManagementMenu);
#if 1
#if 0//不需要管理员号
	ucResult = SUCCESS;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"管理员号:");
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,CASH_MAXSUPERNO,CASH_MAXSUPERNO,'N',aucBuf,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	else
	{
		if(memcmp(aucBuf ,DataSave0.Cashier_SysCashier_Data.aucSuperNo ,CASH_MAXSUPERNO ))
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,0,(uchar *)" 管理员号错误!");
			UTIL_WaitGetKey(5);
			return ERR_CANCEL;
		}
	}
#endif
	memset(aucBuf,0,sizeof(aucBuf));
    MSG_DisplayMenuMsg( true,0,0,MSG_SUPERPASS);
    if (UTIL_Input(1,true,CASH_MAXSUPERPASSWDLEN,CASH_MAXSUPERPASSWDLEN,'P',aucBuf,0) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}else
	{
		util_Printf("DataSave0.Cashier_SysCashier_Data.aucSuperPassword = %s \n",DataSave0.Cashier_SysCashier_Data.aucSuperPassword);
		if( memcmp(aucBuf,DataSave0.Cashier_SysCashier_Data.aucSuperPassword,
				CASH_MAXSUPERPASSWDLEN))
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"管理员密码错");
			MSG_WaitKey(3);
			return(SUCCESS);
		}
	}
#endif
	while(ucResult == SUCCESS||ucResult==ERR_CANCEL)
	{
		ucResult = UTIL_DisplayMenu(&ManagementMenu);
		if(ucResult == ERR_END) break;
	}

	util_Printf("44444DataSave0.ConstantParamData.aucGlobalTransEnable = [%02x][%02x]",
			DataSave0.ConstantParamData.aucGlobalTransEnable[0],
			DataSave0.ConstantParamData.aucGlobalTransEnable[1]);
	return(SUCCESS);
}
unsigned char SYSCASH_SuperKey(void)
{
	unsigned char ucResult;
	unsigned char aucBuf[7];

	ucResult=SUCCESS;

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(UCHAR*)"请输入超级密码:");

	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,6,6,'P',aucBuf,NULL) != KEY_ENTER )
	{
    	return(ERR_CANCEL);
	}

	if(memcmp(aucBuf,DataSave0.Cashier_SysCashier_Data.aucSuperPassWord,6))
	{
		return(MSG_DisplayErrMsg(ERR_SUPERKEY));
	}
	Os__saved_copy((unsigned char*)"123456",DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,CASH_SYSCASHIERPASSLEN);
	XDATA_Write_Vaild_File(DataSaveCashier);

	//Os__clr_display(255);
	//Os__GB2312_display(1,1,(UCHAR*)"密码恢复成功");
	MSG_WaitKey(3);
	return(SUCCESS);
}


unsigned char SEL_SYSMenu(void)
{
	unsigned char ucResult;
	SELMENU SysMenu;

	memset(&SysMenu,0,sizeof(SELMENU));

	UTIL_GetMenu_Value(NULL,	MSG_SUPERKEY,				NULL,	SYSCASH_SuperKey,&SysMenu);//超级密码用于对原始密码的恢复
	UTIL_GetMenu_Value(NULL,	MSG_CHG_SUPVCASHIERNO,		NULL,	SYSCASH_ModiCashier,&SysMenu);
	UTIL_GetMenu_Value(NULL,	MSG_CASHIERQUERY,			NULL,	CASH_CashierQuery,&SysMenu);
	UTIL_GetMenu_Value(NULL,	MSG_ADDCASHIER, 		NULL,	CASH_AddCashier,&SysMenu);
	UTIL_GetMenu_Value(NULL,	MSG_DELCASHIER,		NULL,	CASH_DelCashier,&SysMenu);
	UTIL_GetMenu_Value(NULL,	MSG_MODICASHIER,		NULL,	CASH_ModiCashier,&SysMenu);

	ucResult = SUCCESS;
	while(ucResult== SUCCESS||ucResult==ERR_CANCEL)
	{
		ucResult = UTIL_DisplayMenu(&SysMenu);
		if(ucResult == ERR_END) break;
	}
	return(SUCCESS);
}

unsigned char SEL_CashierMenu(void)
{
	unsigned char ucResult;
	SELMENU CashierMenu;
	memset(&CashierMenu,0,sizeof(SELMENU));

UTIL_GetMenu_Value(NULL,	MSG_ADDCASHIER, 		NULL,	CASH_AddCashier,&CashierMenu);
UTIL_GetMenu_Value(NULL,	MSG_DELCASHIER,		NULL,	CASH_DelCashier,&CashierMenu);
UTIL_GetMenu_Value(NULL,	MSG_MODICASHIER,		NULL,	CASH_ModiCashier,&CashierMenu);
UTIL_GetMenu_Value(NULL,	MSG_CASHIERQUERY,		NULL,	CASH_CashierQuery,&CashierMenu);
UTIL_GetMenu_Value(NULL,	MSG_CASHIERPINRELOAD,	NULL,	CASH_PinReload,&CashierMenu);

	if(!DataSave0.Cashier_SysCashier_Data.ucSYSCashierExitFlag)
	{
	    //Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"主管操作员不存在");
		MSG_WaitKey(3);
		return(SUCCESS);
	}

	ucResult = UTIL_DisplayMenu(&CashierMenu);

	if(( ucResult != SUCCESS)&&( ucResult != ERR_CANCEL))
	{
		MSG_DisplayErrMsg(ucResult);
	}

	return(SUCCESS);
}

unsigned char SEL_AccumulatRunTime( void )
{
unsigned char aucBuf[32],aucTmp[32];
unsigned long ulTemp;

	Os__read_time(aucBuf);
	asc_bcd(aucTmp,2,aucBuf,4);
	if( aucTmp[0]!=DataSave0.ChangeParamData.ucDialStartStamp )
	{
		Os__saved_set(&DataSave0.ChangeParamData.ucDialStartStamp,
						aucTmp[0],1);
		ulTemp=DataSave0.ChangeParamData.ulDialTimeTotal;
		ulTemp++;
		if( ulTemp>=9999 )
		{
			ulTemp=0;
			Os__saved_copy(	(unsigned char *)&ulTemp,
				(unsigned char *)&DataSave0.ChangeParamData.ulDialConnectCnt,
				sizeof(unsigned long));
			Os__saved_copy(	(unsigned char *)&ulTemp,
					(unsigned char *)&DataSave0.ChangeParamData.ulDialTotalCount,
					sizeof(unsigned long));
			Os__saved_copy(	(unsigned char *)&ulTemp,
					(unsigned char *)&DataSave0.ChangeParamData.ulDialConnectRate,
					sizeof(unsigned long));
		}
		Os__saved_copy(	(unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ChangeParamData.ulDialTimeTotal,
			sizeof(unsigned long));
		XDATA_Write_Vaild_File(DataSaveChange);
//		util_Printf( "ulDialTimeTotal=%d ucDialStartStamp=%d aucTmp[0]=%2x\n",
//				DataSave0.ChangeParamData.ulDialTimeTotal,
//				DataSave0.ChangeParamData.ucDialStartStamp,
//				aucTmp[0] );
	}
	return 1;
}

unsigned char SEL_IncDialTotalCount( void )
{
	unsigned long ulTemp;

	ulTemp=DataSave0.ChangeParamData.ulDialTotalCount;
	ulTemp++;
	Os__saved_copy(	(unsigned char *)&ulTemp,
					(unsigned char *)&DataSave0.ChangeParamData.ulDialTotalCount,
	sizeof(unsigned long));
	XDATA_Write_Vaild_File(DataSaveChange);
	util_Printf( "ulDialTotalCount=%d \n",
			DataSave0.ChangeParamData.ulDialTotalCount );

	return 1;
}

unsigned char SEL_IncDialConnectCnt( void )
{
	unsigned long ulTemp;

	ulTemp=DataSave0.ChangeParamData.ulDialConnectCnt;
	ulTemp++;
	Os__saved_copy(	(unsigned char *)&ulTemp,
		(unsigned char *)&DataSave0.ChangeParamData.ulDialConnectCnt,
		sizeof(unsigned long));
	if( DataSave0.ChangeParamData.ulDialTotalCount!=0 )
	{
		ulTemp=DataSave0.ChangeParamData.ulDialConnectCnt*100/DataSave0.ChangeParamData.ulDialTotalCount;
		Os__saved_copy(	(unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ChangeParamData.ulDialConnectRate,
			sizeof(unsigned long));

			util_Printf( "ulDialConnectCnt=%d ulDialTotalCount=%d ulDialConnectRate=%d\n",
			DataSave0.ChangeParamData.ulDialConnectCnt,
			DataSave0.ChangeParamData.ulDialTotalCount,
			DataSave0.ChangeParamData.ulDialConnectRate );

	}
	XDATA_Write_Vaild_File(DataSaveChange);
	util_Printf( "ulDialConnectCnt=%d \n",
			DataSave0.ChangeParamData.ulDialConnectCnt );
	return 1;
}

unsigned char SEL_ClrAccumulatDial( void )
{
unsigned long ulTemp;

	//Os__saved_set(&DataSave0.ChangeParamData.ucDialStartStamp,0,1);
	ulTemp=0;
	Os__saved_copy(	(unsigned char *)&ulTemp,
		(unsigned char *)&DataSave0.ChangeParamData.ulDialConnectCnt,
		sizeof(unsigned long));
	Os__saved_copy(	(unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ChangeParamData.ulDialTotalCount,
			sizeof(unsigned long));
	Os__saved_copy(	(unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ChangeParamData.ulDialTimeTotal,
			sizeof(unsigned long));
	Os__saved_copy(	(unsigned char *)&ulTemp,
			(unsigned char *)&DataSave0.ChangeParamData.ulDialConnectRate,
			sizeof(unsigned long));
	XDATA_Write_Vaild_File(DataSaveChange);
	return 1;
}

unsigned char SEL_DisplayEMVInfo(void)
{
	unsigned char 	ucKey;

	while(1)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"1.AID 参数信息");
		//Os__GB2312_display(1,0,(uchar *)"2.公钥信息");
		ucKey = Os__xget_key();
		switch(ucKey)
		{
			case '1':
				EMV_DisplayAIDParam();
				break;
			case '2':
				EMV_DisplayPKInfo();
				break;
			case KEY_CLEAR:
				return SUCCESS;
			default:
				break;
		}
	}
}

#ifdef TMS
unsigned char SEL_DOWNTTMSPARA(void)
{
	unsigned char ucResult;

	ucResult = SUCCESS;

	util_Printf("开始TMS操作\n");
	ucResult = UTIL_Is_Trans_Empty();
	util_Printf("\n是否有交易=%02x\n",ucResult);

	if(!ucResult)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(UCHAR*)" POS有交易数据");
		//Os__GB2312_display(2,0,(UCHAR*)"请先结算或清交易");
		TMSAPP_GetKey(3);
	}
	else
	{
		ucResult = TMSAPP_Manage((unsigned char*)CURRENT_PRJ,(unsigned char*)SOFTWARE_VER,
			xDATA_Constant.aucTerminalID, xDATA_Constant.aucMerchantID);
		if (ucResult == TMSAPPERR_NOPARAM)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,0,(uchar*)"无需更新参数");
			TMSAPP_GetKey(1);
		}
	}
}

unsigned char SEL_GetParam(void)
{
	unsigned char ucResult;

	ucResult = SUCCESS;
	ucResult = TMSApp_GetParam_Manage();

	return(ucResult);
}
#endif

unsigned char SEL_ParamUpdata(void)
{
	unsigned char ucResult;
	unsigned char ucKey;

	ucResult=SUCCESS;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"1.从PC更新参数");
	//Os__GB2312_display(1,0,(uchar *)"2.从主机下载参数");

	while(1)
	{
		ucKey = Os__xget_key();
		switch(ucKey)
		{
			case '1':
				ucResult = PCDown_UpdataParam();
				break;
			case '2':
				COMMS_PreComm();
				G_NORMALTRANS_ucTransType =TRANS_LOADPARAM;
				ucResult = SEL_TestOnlineTrans();
				break;
			case KEY_CLEAR:
				break;
			default:
				continue;
		}
		if(( ucResult != SUCCESS)&&( ucResult != ERR_CANCEL))
		{
			MSG_DisplayErrMsg(ucResult);
		}
		break;
	}
	return(SUCCESS);
}

unsigned char SEL_SetMasterKey(void)
{
	unsigned  char ucResult;
	SELMENU ManagementMenu;

	memset(&ManagementMenu,0,sizeof(SELMENU));


	UTIL_GetMenu_Value(NULL,	MSG_MUNUALSETKEY, 		NULL,	KEY_InputMasterKey,		&ManagementMenu);
	UTIL_GetMenu_Value(NULL,	MSG_INPUTKEYINDEX, 	 	NULL,	KEY_InputKeyIndex,			&ManagementMenu);
	UTIL_GetMenu_Value(NULL,	MSG_DOWNKEY, 	 			NULL,	KEY_DownKeyFromPOS,	&ManagementMenu);
	UTIL_GetMenu_Value(NULL,	MSG_DKFROMC, 	 			NULL,	KEY_DownKeyFromCard,		&ManagementMenu);

	UTIL_GetMenu_Value(NULL,	MSG_DEFAULTKEY, 			NULL,	UTIL_Set_BankID,				&ManagementMenu);

    ucResult = SUCCESS;
	while(ucResult == SUCCESS||ucResult==ERR_CANCEL)
	{
		ucResult = UTIL_DisplayMenu(&ManagementMenu);
		if(ucResult == ERR_END) break;
	}
	return(SUCCESS);
}


unsigned char SEL_HostIP(const unsigned char ucTerIp)
{
	unsigned char ucKey,ucNextKey,ucResult;
	unsigned char aucBuf[30],aucDisp[5];
	unsigned char ucFlag;

	ucResult=SUCCESS;
	ucNextKey=0x00;
	ucFlag=0x00;

	memset(aucBuf,0x00,sizeof(aucBuf));
	util_Printf("HostIp[ucTerIp=%02x]\n",ucTerIp);
	OSMMI_ClrDisplay(0x30,255);
	//Os__GB2312_display(0,0,(unsigned char*)"请选择交易IP:");
	if(ucTerIp==0x01)
	{
		memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPAddress1,4);
		sprintf((char *)aucDisp,"1.IP1:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,2,0,aucDisp);

	  memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPAddress2,4);
		sprintf((char *)aucDisp,"2.IP2:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,3,0,aucDisp);

	  memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPAddress3,4);
		sprintf((char *)aucDisp,"3.IP3:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,4,0,aucDisp);
		//OSMMI_DisplayASCII(0x30,0,0,(unsigned char*)"1.IP1: 172.16.10.71");
		//OSMMI_DisplayASCII(0x30,1,0,(unsigned char*)"2.IP2: 172.16.10.72");
		//OSMMI_DisplayASCII(0x30,2,0,(unsigned char*)"3.IP3: 172.16.10.73");
	}
	else if(ucTerIp==0x02)
	{
		memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPBack1,4);
		sprintf((char *)aucDisp,"1.IP1:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,2,0,aucDisp);

	  memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPBack2,4);
		sprintf((char *)aucDisp,"2.IP2:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,3,0,aucDisp);

	  memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPBack3,4);
		sprintf((char *)aucDisp,"3.IP3:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,4,0,aucDisp);
		//OSMMI_DisplayASCII(0x30,0,0,(unsigned char*)"1.IP1: 144.16.35.21");
		//OSMMI_DisplayASCII(0x30,1,0,(unsigned char*)"2.IP2: 144.16.35.22");
		//OSMMI_DisplayASCII(0x30,2,0,(unsigned char*)"3.IP3: 144.16.35.23");
	}

	while(1)
	{
		ucKey=Os__xget_key();
		if(ucKey=='1')
		{
			ucFlag=0x01;
			if(ucTerIp==0x01)
			{
				util_Printf("case1 ucTerIp1=%02x\n",ucTerIp);
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPAddress1,4);
			}
			else if(ucTerIp==0x02)
			{
				util_Printf("case1 ucTerIp2=%02x\n",ucTerIp);
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPBack1,4);
			}
			ucResult=SEL_SelectIP(ucFlag,ucTerIp,aucBuf);
			break;
		}
		else if	(ucKey=='2')
		{
			ucFlag=0x02;
			if(ucTerIp==0x01)
			{
				util_Printf("case2 ucTerIp1=%02x\n",ucTerIp);
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPAddress2,4);
			}
			else if(ucTerIp==0x02)
			{
				util_Printf("case2 ucTerIp2=%02x\n",ucTerIp);
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPBack2,4);
			}
			ucResult=SEL_SelectIP(ucFlag,ucTerIp,aucBuf);
			break;
		}
		else if(ucKey=='3')
		{
			ucFlag=0x03;
			if(ucTerIp==0x01)
			{
				util_Printf("case3 ucTerIp1=%02x\n",ucTerIp);
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPAddress3,4);
			}
			else if(ucTerIp==0x02)
			{
				util_Printf("case3 ucTerIp2=%02x\n",ucTerIp);
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPBack3,4);
			}
			ucResult=SEL_SelectIP(ucFlag,ucTerIp,aucBuf);
			break;
		}
	}
	util_Printf("ucSEL_HostIP-Result=%02x",ucResult);
	return(ucResult);
}

unsigned char SEL_HostCopyIP(const unsigned char ucIpTY)
{
	unsigned char ucKey,ucNextKey,ucResult;
	unsigned char aucBuf[30],aucDisp[5];
	unsigned char ucFlag;

	ucResult=SUCCESS;
	ucNextKey=0x00;
	memset(aucBuf,0x00,sizeof(aucBuf));

	OSMMI_ClrDisplay(0x30,255);
	//Os__GB2312_display(0,0,(unsigned char*)"请选择交易备份IP");
	if(ucIpTY==0x01)
	{
		memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPGPRS1,4);
		sprintf((char *)aucDisp,"1.IP1:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,2,0,aucDisp);

	  memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPGPRS2,4);
		sprintf((char *)aucDisp,"2.IP2:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,3,0,aucDisp);

	  memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPGPRS3,4);
		sprintf((char *)aucDisp,"3.IP3:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,4,0,aucDisp);
		//OSMMI_DisplayASCII(0x30,0,0,(unsigned char*)"1.IP1: 172.16.10.61");
		//OSMMI_DisplayASCII(0x30,1,0,(unsigned char*)"2.IP2: 172.16.10.62");
		//OSMMI_DisplayASCII(0x30,2,0,(unsigned char*)"3.IP3: 172.16.10.63");
	}
	if(ucIpTY==0x02)
	{
		memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPMIS1,4);
		sprintf((char *)aucDisp,"1.IP1:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,2,0,aucDisp);

	  memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPMIS2,4);
		sprintf((char *)aucDisp,"2.IP2:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,3,0,aucDisp);

	  memset(aucDisp,0,sizeof(aucDisp));
		memset(aucBuf,0x00,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPMIS3,4);
		sprintf((char *)aucDisp,"3.IP3:%d.%d.%d.%d",aucBuf[0],aucBuf[1],aucBuf[2],aucBuf[3]);
		OSMMI_DisplayASCII(0x30,4,0,aucDisp);
		//OSMMI_DisplayASCII(0x30,0,0,(unsigned char*)"1.IP1: 144.16.34.21");
		//OSMMI_DisplayASCII(0x30,1,0,(unsigned char*)"2.IP2: 144.16.34.22");
		//OSMMI_DisplayASCII(0x30,2,0,(unsigned char*)"3.IP3: 144.16.34.23");
	}


	while(1)
	{
		ucKey=Os__xget_key();
		if(ucKey=='1')
		{
			ucFlag=0x01;
			if(ucIpTY==0x01)
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPGPRS1,4);
			else if(ucIpTY==0x02)
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPMIS1,4);
			ucResult=SEL_SelectBackIP(ucFlag,ucIpTY,aucBuf);
			break;
		}
		else if(ucKey=='2')
		{
			ucFlag=0x02;
			if(ucIpTY==0x01)
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPGPRS2,4);
			else if(ucIpTY==0x02)
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPMIS2,4);
			ucResult=SEL_SelectBackIP(ucFlag,ucIpTY,aucBuf);
			break;
		}
		else if(ucKey=='3')
		{
			ucFlag=0x03;
			if(ucIpTY==0x01)
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPGPRS3,4);
			else if(ucIpTY==0x02)
				memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulHostIPMIS3,4);
			ucResult=SEL_SelectBackIP(ucFlag,ucIpTY,aucBuf);
			break;
		}
	}
	util_Printf("\nSEL_HostCopyIP[ucResult]=%02x\n",ucResult);
	return(ucResult);
}
unsigned char SEL_HostBackPorts(void)
{
	unsigned char aucBuf[30];
	unsigned char ucResult;

	ucResult=SUCCESS;

	if(ucResult == SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (unsigned char *)"主机备份端口:");
		int_asc(aucBuf,6,&DataSave0.ConstantParamData.uiHostBackPort);
		if (UTIL_Input(1,true,6,6,'N',aucBuf,0) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}else
		{
			DataSave0.ConstantParamData.uiHostBackPort=asc_long(aucBuf,6);
		}
		util_Printf("\nDataSave0.ConstantParamData.uiHostPort=%d",
						DataSave0.ConstantParamData.uiHostBackPort);
	}
	return(ucResult);
}
unsigned char SEL_HostPorts(void)
{
	unsigned char aucBuf[30];
	unsigned char ucResult;

	ucResult=SUCCESS;

	if(ucResult == SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (unsigned char *)"主机端口:");
		int_asc(aucBuf,6,&DataSave0.ConstantParamData.uiHostPort);
		if (UTIL_Input(1,true,6,6,'N',aucBuf,0) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}else
		{
			DataSave0.ConstantParamData.uiHostPort=asc_long(aucBuf,6);
		}
		util_Printf("\nDataSave0.ConstantParamData.uiHostPort=%d",
						DataSave0.ConstantParamData.uiHostPort);
	}
	return(ucResult);
}
unsigned char SEL_SelectIP(const unsigned char ucIpFlag,const unsigned char ucHostIp,unsigned char aucIpBuf[30])
{
	unsigned char ucResult;

	ucResult=SUCCESS;

	util_Printf("\nucFlag2=%02x,ucHostIp=%02x\n",ucIpFlag,ucHostIp);
	if(!ucResult)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char*)"当前IP:");
		//Os__GB2312_display(3,0,(unsigned char*)"按[清除]键修改");
		if (OSUTIL_InputIPV4Addr(0,0x31,1,aucIpBuf) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}
		else
		{
			util_Printf("\nucFlag3=%02x,ucHostIp=%02x\n",ucIpFlag,ucHostIp);
			if(ucIpFlag==0x01&&ucHostIp==0x01)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPAddress1, aucIpBuf, 4);
			}
			else if(ucIpFlag==0x01&&ucHostIp==0x02)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPBack1, aucIpBuf, 4);
			}
			else if(ucIpFlag==0x02&&ucHostIp==0x01)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPAddress2, aucIpBuf, 4);
			}
			else if(ucIpFlag==0x02&&ucHostIp==0x02)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPBack2, aucIpBuf, 4);
			}
			else if(ucIpFlag==0x03&&ucHostIp==0x01)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPAddress3, aucIpBuf, 4);
			}
			else if(ucIpFlag==0x03&&ucHostIp==0x02)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPBack3, aucIpBuf, 4);
			}
			memcpy(&DataSave0.ConstantParamData.ulHostIP, aucIpBuf, 4);
			util_Printf("HIP=%d\n",DataSave0.ConstantParamData.ulHostIP);
		}
		XDATA_Write_Vaild_File(DataSaveConstant);
	}
	return(ucResult);
}

unsigned char SEL_SelectBackIP(const unsigned char ucIpFlag,const unsigned char ucIp,unsigned char aucIpBuf[30])
{
	unsigned char ucResult;
	unsigned char ucIPAddress[5];

	ucResult=SUCCESS;

	util_Printf("\nucFlag2=%02x,%02x\n",ucIpFlag,ucIp);
	if(!ucResult)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char*)"当前备份IP:");
		//Os__GB2312_display(3,0,(unsigned char*)"按[清除]键修改");

		memset(ucIPAddress,0x00,sizeof(ucIPAddress));
		memcpy(ucIPAddress,&DataSave0.ConstantParamData.ulBackIP,sizeof(unsigned long));
		asc_long(ucIPAddress,4);
		util_Printf("IPADD=%3d,%3d,%3d,%3d\n",ucIPAddress[0],ucIPAddress[1],ucIPAddress[2],ucIPAddress[3]);

		if (OSUTIL_InputIPV4Addr(0,0x31,1,aucIpBuf) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}
		else
		{
			util_Printf("\nucFlag3=%02x,ucHostIp=%02x,Ip=%3d,%3d,%3d,%3d\n",ucIpFlag,ucIp,aucIpBuf[0],aucIpBuf[1],aucIpBuf[2],aucIpBuf[3]);
			if(ucIpFlag==0x01&&ucIp==0x01)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPGPRS1, aucIpBuf, 4);
			}
			else if(ucIpFlag==0x01&&ucIp==0x02)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPMIS1, aucIpBuf, 4);
			}
			else if(ucIpFlag==0x02&&ucIp==0x01)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPGPRS2, aucIpBuf, 4);
			}
			else if(ucIpFlag==0x02&&ucIp==0x02)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPMIS2, aucIpBuf, 4);
			}
			else if(ucIpFlag==0x03&&ucIp==0x01)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPGPRS3, aucIpBuf, 4);
			}
			else if(ucIpFlag==0x03&&ucIp==0x02)
			{
				memcpy(&DataSave0.ConstantParamData.ulHostIPMIS3, aucIpBuf, 4);
			}
			memcpy(&DataSave0.ConstantParamData.ulBackIP, aucIpBuf, 4);
		}
		XDATA_Write_Vaild_File(DataSaveConstant);
	}
	return(ucResult);
}

unsigned char SEL_ReaderMenu_HongBao(void)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucKey = 0x00;
	unsigned char ucKeyIndex;
	unsigned char aucInData[17];
	BASICCONFIG pBasicConfig;

 	while(1)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"1配置读卡器参数");
		//Os__GB2312_display(1,0,(uchar *)"2设与POS接口密钥");
		//Os__GB2312_display(2,0,(uchar *)"3设读卡器EMV信息");
		//Os__GB2312_display(3,0,(uchar *)"4读卡器公钥信息");
		ucKey = Os__xget_key();
		switch(ucKey)
		{
			case '1':
				ucResult = CFG_ChangeReaderParamData_HongBao();
				break;
			case '2':
				util_Printf("\nabc");
			 	ucResult = INTERFACE_ChangeConnectState();
			 	util_Printf("\nINTERFACE_ChangeConnectState=ABC=%d",ucResult);
				if(!ucResult) ucResult = File_ReadBasicConfigFile(&pBasicConfig);
				if(!ucResult)
				{
					memset(aucInData,0,sizeof(aucInData));
					ucKeyIndex=pBasicConfig.ucPowerUpFlag;
					if(	ucKeyIndex == 0x01)
						memcpy(aucInData,IMEKMDKDEFAULTVALUE,16);
					else if(ucKeyIndex == 0x02)
						memcpy(aucInData,IAEKMDKDEFAULTVALUE,16);
					else if(ucKeyIndex == 0x03)
						memcpy(aucInData,IMEKDEFAULTVALUE,16);
					else if(ucKeyIndex == 0x04)
						memcpy(aucInData,IAEKDEFAULTVALUE,16);
					ucResult = INTERFACE_ConfigKeyData(ucKeyIndex,aucInData);
				}
				break;
			case '3':
				ucResult = VerifyReaderCAPK(1);
				break;
			case '4':
				EMV_DisplayReaderPKInfo();
				break;
/////////////////////////////测试用///////////////
#if 1
			case '5':
				ucKey =Os__xget_key();
				if((ucKey>='0')&&(ucKey<='9'))
				INTERFACE_RefurbishReaderLCD(ucKey-0x30);
				break;
			case '6':
				ucKey =Os__xget_key();
				if((ucKey>='0')&&(ucKey<='9'))
					INTERFACE_ShowStatusToReader(ucKey-0x30);
				break;
			case '7':
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"TEST");
				UTIL_WaitGetKey(3);
				break;
#endif
/////////////////////////////////////////////
			case KEY_CLEAR:
				return  SUCCESS;
				break;
			default:
				break;
		}
	}
}
unsigned char SEL_ReaderMenu_Sand(void)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucKey = 0x00;


 	while(1)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"1配置读卡器参数");
		ucKey = Os__xget_key();
		switch(ucKey)
		{
			case '1':
				ucResult = CFG_ChangeReaderParamData_Sand();
				break;
			case KEY_CLEAR:
				return  SUCCESS;
				break;
			default:
				break;
		}
	}
}
