#include <include.h>
#include <global.h>
#include <xdata.h>

unsigned char SYSCASH_ChangeSuperPasswd(void)
{
    unsigned char aucBuf[CASH_MAXSUPERPASSWDLEN+1];
    unsigned char aucBuf2[CASH_MAXSUPERPASSWDLEN+1];
    unsigned char ucResult;

	ucResult = SUCCESS;
   	/*Input Supervizor Password*/
//	MSG_DisplayMsg( true,0,0,MSG_SUPERPASS);
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
    //Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"管理员密码:");
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,CASH_MAXSUPERPASSWDLEN,CASH_MAXSUPERPASSWDLEN,'P',aucBuf,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	{
		int i;
		for(i=0;i<CASH_MAXSUPERPASSWDLEN;i++)
			util_Printf("%02x ",DataSave0.Cashier_SysCashier_Data.aucSuperPassword[i]);
		util_Printf("\n");
	}
	if( memcmp(aucBuf,DataSave0.Cashier_SysCashier_Data.aucSuperPassword,
				CASH_MAXSUPERPASSWDLEN))
	{
		util_Printf("ERR_CASH_PASS\n");
		return(ERR_CASH_PASS);
	}

    /*Input New Password*/
       //Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"新管理员密码:");
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,CASH_MAXSUPERPASSWDLEN,CASH_MAXSUPERPASSWDLEN,'P',aucBuf,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	
    /*Enter New Password*/
       //Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"确认新管理员密码");
	memset(aucBuf2,0,sizeof(aucBuf2));
	if( UTIL_Input(1,true,CASH_MAXSUPERPASSWDLEN,CASH_MAXSUPERPASSWDLEN,'P',aucBuf2,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	
    if (ucResult == SUCCESS)
    	{
		
		if(!memcmp( aucBuf,aucBuf2 , CASH_MAXSUPERPASSWDLEN))
		 {
				Os__saved_copy(aucBuf2,
						DataSave0.Cashier_SysCashier_Data.aucSuperPassword,
						CASH_MAXSUPERPASSWDLEN);
				XDATA_Write_Vaild_File(DataSaveCashier);
			       //Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"修改成功");
				MSG_WaitKey(3);
		 }
		else 
		{
 			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"两次密码不一致");
			MSG_WaitKey(3);
			 return(SUCCESS);
		}
    	}
    return(ucResult);       	
}

unsigned char SYSCASH_AddCashier(void)
{
	unsigned char aucCashierNo[CASH_CASHIERNOLEN+1];
	unsigned char aucCashierPass[CASH_CASHIERPASSLEN+1];
    unsigned char aucBuf[CASH_MAXSUPERPASSWDLEN+1];
	
	if(DataSave0.Cashier_SysCashier_Data.ucSYSCashierExitFlag)
	{
	    //Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"主管操作员已存在");
		MSG_WaitKey(3);
		return(SUCCESS);
	}
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
   	/*Input Supervizor Password*/
    	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"系统管理员密码");
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,CASH_MAXSUPERPASSWDLEN,CASH_MAXSUPERPASSWDLEN,'P',aucBuf,NULL) != KEY_ENTER )
	{
    	return(MSG_DisplayErrMsg(ERR_CANCEL));
	}
	if( memcmp(aucBuf,DataSave0.Cashier_SysCashier_Data.aucSuperPassword,
			CASH_MAXSUPERPASSWDLEN))
	{
		return(MSG_DisplayErrMsg(ERR_CASH_PASS));
	}
	MSG_DisplayMenuMsg( true,0,0,MSG_SYS_CASHIERNO);
	memset(aucCashierNo,0,sizeof(aucCashierNo));
	if( UTIL_Input(1,true,CASH_CASHIERNOLEN,CASH_CASHIERNOLEN,'N',aucCashierNo,NULL) != KEY_ENTER )
	{
        return(MSG_DisplayErrMsg(MSG_CANCEL));
	}
	Os__saved_copy(aucCashierNo,DataSave0.Cashier_SysCashier_Data.aucSYSCashierNo,CASH_SYSCASHIERNOLEN);

	MSG_DisplayMenuMsg( true,0,0,MSG_SYS_CASHIERPASS);
	memset(aucCashierPass,0,sizeof(aucCashierPass));
	if( UTIL_Input(1,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
	{
        return(MSG_DisplayErrMsg(MSG_CANCEL));
	}
	Os__saved_copy(aucCashierPass,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,CASH_SYSCASHIERPASSLEN);
	Os__saved_set(&DataSave0.Cashier_SysCashier_Data.ucSYSCashierExitFlag,1,1);
	XDATA_Write_Vaild_File(DataSaveCashier);
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"增加成功");
	MSG_WaitKey(3);
	return(SUCCESS);
}

unsigned char SYSCASH_DelCashier(void)
{
	unsigned char aucCashierNo[CASH_CASHIERNOLEN+1];
	unsigned char aucCashierPass[CASH_CASHIERPASSLEN+1];
    unsigned char aucBuf[CASH_MAXSUPERPASSWDLEN+1];

   	/*Input Supervizor Password*/
	if(!DataSave0.Cashier_SysCashier_Data.ucSYSCashierExitFlag)
	{
	    //Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"主管操作员不存在");
		MSG_WaitKey(3);
		return(SUCCESS);
	}
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
   	/*Input Supervizor Password*/
       //Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"系统管理员密码");
	memset(aucBuf,0,sizeof(aucBuf));
	if( UTIL_Input(1,true,CASH_MAXSUPERPASSWDLEN,CASH_MAXSUPERPASSWDLEN,'P',aucBuf,NULL) != KEY_ENTER )
	{
		return(MSG_DisplayErrMsg(MSG_CANCEL));
	}
	if( memcmp(aucBuf,DataSave0.Cashier_SysCashier_Data.aucSuperPassword,
				CASH_MAXSUPERPASSWDLEN))
	{
		return(MSG_DisplayErrMsg(ERR_CASH_PASS));
	}
	memset(aucCashierNo,0,sizeof(aucCashierNo));
	memset(aucCashierPass,0,sizeof(aucCashierPass));
	Os__saved_copy(aucCashierNo,DataSave0.Cashier_SysCashier_Data.aucSYSCashierNo,CASH_SYSCASHIERNOLEN);
	Os__saved_copy(aucCashierPass,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,CASH_SYSCASHIERPASSLEN);
	Os__saved_set(&DataSave0.Cashier_SysCashier_Data.ucSYSCashierExitFlag,0,1);
	XDATA_Write_Vaild_File(DataSaveCashier);
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"删除成功");
	MSG_WaitKey(3);
	return(SUCCESS);
}

unsigned char SYSCASH_ModiCashier(void)
{
	unsigned char aucCashierPass[CASH_SYSCASHIERPASSLEN+1];
	unsigned char aucCashierPass2[CASH_SYSCASHIERPASSLEN+1];

	if(!DataSave0.Cashier_SysCashier_Data.ucSYSCashierExitFlag)
	{
	    //Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"主管操作员不存在");
		MSG_WaitKey(3);
		return(SUCCESS);
	}
	MSG_DisplayMenuMsg( true,0,0,MSG_SYS_CASHIERPASS);
	memset(aucCashierPass,0,sizeof(aucCashierPass));
	if( UTIL_Input(1,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if( memcmp(DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,aucCashierPass,CASH_SYSCASHIERPASSLEN))
	{
		return(MSG_DisplayErrMsg(ERR_CASH_PASS));
	}else
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"新主管密码");
		memset(aucCashierPass,0,sizeof(aucCashierPass));
		if( UTIL_Input(1,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucCashierPass,NULL) != KEY_ENTER )
		{
			return(MSG_DisplayErrMsg(ERR_CANCEL));
		}
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"确定新主管密码");
		memset(aucCashierPass2,0,sizeof(aucCashierPass2));
		if( UTIL_Input(1,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucCashierPass2,NULL) != KEY_ENTER )
		{
			return(MSG_DisplayErrMsg(ERR_CANCEL));
		}
		if(!memcmp( aucCashierPass, aucCashierPass2 ,CASH_SYSCASHIERPASSLEN ))
		{
			Os__saved_copy(aucCashierPass,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,CASH_SYSCASHIERPASSLEN);
			XDATA_Write_Vaild_File(DataSaveCashier);
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"修改成功");
		}else
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"密码输入不一致");
		}
		MSG_WaitKey(3);
		return(SUCCESS);
	}						

}

unsigned char SYSCASH_SetTransType(void)
{
	unsigned char aucTransTAB[3];
	unsigned char aucBuf[8];
	unsigned char ucChar,ucDispCh;

	util_Printf("\n6666DataSave0.ConstantParamData.aucGlobalTransEnable = %02x %02x \n",
		DataSave0.ConstantParamData.aucGlobalTransEnable[0],	
		DataSave0.ConstantParamData.aucGlobalTransEnable[1]);

    memset(aucTransTAB,0,sizeof(aucTransTAB));
	memcpy(aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,PARAM_GLOBALTRANSENABLELEN);

	util_Printf("aucTransTAB = %02x%02x\n",aucTransTAB[0],aucTransTAB[1]);
	
	util_Printf("Tab=%s",aucTransTAB);
	ucChar = aucTransTAB[0];
	util_Printf("ucChar=%02x",ucChar);

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"是否支持查询");
	//Os__GB2312_display(3,0,(unsigned char *)"0:不支持  1:支持");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[0]&0x80)?0x31:0x30;
	util_Printf("ucDispCh=%02x",ucDispCh);
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	if(aucBuf[0] == '1')
	{
		ucChar |= 0x80;
		util_Printf("ucChar1=%02x",ucChar);
	}
	else
		ucChar &= 0x7F;
	util_Printf("ucChar2=%02x",ucChar);
	aucTransTAB[0] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);

	//Os__clr_display(0);
	//Os__GB2312_display(0,0,(unsigned char *)"是否支持预授权");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[0]&0x40)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x40;
	else
		ucChar &= 0xBF;
	aucTransTAB[0] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);

	//Os__clr_display(0);
	//Os__GB2312_display(0,0,(unsigned char *)"支持预授权撤销");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[0]&0x20)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x20;
	else
		ucChar &= 0xDF;		 
	aucTransTAB[0] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);

	//Os__clr_display(0);
	//Os__GB2312_display(0,0,(unsigned char *)"预授权完成联机");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[0]&0x10)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x10;
	else
		ucChar &= 0xEF;		 
	aucTransTAB[0] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);

	//Os__clr_display(0);
	//Os__GB2312_display(0,0,(unsigned char *)"预授权完成");
	//Os__GB2312_display(1,0,(unsigned char *)"联机撤销");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[0]&0x08)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x08;
	else
		ucChar &= 0xF7;		 
	aucTransTAB[0] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);

	//Os__clr_display(0);
	//Os__clr_display(2);
	//Os__GB2312_display(0,0,(unsigned char *)"是否支持消费");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[0]&0x04)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x04;
	else
		ucChar &= 0xFB;		 
	aucTransTAB[0] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);

	//Os__clr_display(0);
	//Os__GB2312_display(0,0,(unsigned char *)"支持消费撤销");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[0]&0x02)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x02;
	else
		ucChar &= 0xFD;		 
	aucTransTAB[0] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);
	
	//Os__clr_display(0);
	//Os__GB2312_display(0,0,(unsigned char *)"是否支持退货");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[0]&0x01)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x01;
	else
		ucChar &= 0xFE;		 
	aucTransTAB[0] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);
	//-------------------------------1----------------------------------------
	ucChar = aucTransTAB[1];
	//Os__clr_display(0);
	//Os__GB2312_display(0,0,(unsigned char *)"支持离线结算");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[1]&0x80)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x80;
	else
		ucChar &= 0x7F;		 
	aucTransTAB[1] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);

	//Os__clr_display(0);
	//Os__GB2312_display(0,0,(unsigned char *)"支持离线结算调整");
	memset(aucBuf,0,sizeof(aucBuf));
	util_Printf("\naucTransTAB[1]=%02x\n",aucTransTAB[1]);
	ucDispCh = (aucTransTAB[1]&0x40)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	util_Printf("ucDispCh=%02x\n",ucDispCh);
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	util_Printf("ucChar1=%02x\n",ucChar);
	if(aucBuf[0] == '1')
		ucChar |= 0x40;
	else
		ucChar &= 0xBF;	
	util_Printf("ucChar2=%02x\n",ucChar);
	aucTransTAB[1] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);

	//Os__clr_display(0);
	//Os__GB2312_display(0,0,(unsigned char *)"是否支持 ");
	//Os__GB2312_display(1,0,(unsigned char *)"预授权完成离线");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[1]&0x20)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x20;
	else
		ucChar &= 0xDF;
	aucTransTAB[1] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);

	//Os__clr_display(0);
	//Os__clr_display(2);
	//Os__GB2312_display(0,0,(unsigned char *)"是否支持 IC卡");
	//Os__GB2312_display(1,0,(unsigned char *)"脚本处理结果通知");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[1]&0x10)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x10;
	else
		ucChar &= 0xEF;
	aucTransTAB[1] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);

	//Os__clr_display(0);
	//Os__clr_display(2);
	//Os__GB2312_display(0,0,(unsigned char *)"支持离线消费");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[1]&0x08)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x08;
	else
		ucChar &= 0xF7;
	aucTransTAB[1] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);
/*Define OffLine Undo*/
	//Os__clr_display(0);
	//Os__clr_display(2);
	//Os__GB2312_display(0,0,(unsigned char *)"支持离线撤销");
	memset(aucBuf,0,sizeof(aucBuf));
	aucBuf[0] = DataSave0.ConstantParamData.ucUndoOff; 
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	Os__saved_copy(	aucBuf,&DataSave0.ConstantParamData.ucUndoOff,1); 	
	XDATA_Write_Vaild_File(DataSaveConstant);
	util_Printf("\n离线撤销 = %02x\n",DataSave0.ConstantParamData.ucUndoOff);
/*End*/
	//Os__clr_display(0);
	//Os__clr_display(2);
	//Os__GB2312_display(0,0,(unsigned char *)"支持预授权追加");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[1]&0x04)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(1,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x04;
	else
		ucChar &= 0xFB;
	aucTransTAB[1] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);

	//Os__clr_display(0);
	//Os__clr_display(2);
	//Os__GB2312_display(0,0,(unsigned char *)"是否支持 ");
	//Os__GB2312_display(1,0,(unsigned char *)"电子钱包的圈存");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = (aucTransTAB[1]&0x02)?0x31:0x30;
	aucBuf[0] = ucDispCh;
	if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}	
	if(aucBuf[0] == '1')
		ucChar |= 0x02;
	else
		ucChar &= 0xFD;
	aucTransTAB[1] = ucChar;	
	Os__saved_copy(	aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
		PARAM_GLOBALTRANSENABLELEN); 
	XDATA_Write_Vaild_File(DataSaveConstant);
	
	util_Printf("\nDataSave0.ConstantParamData.aucGlobalTransEnable = %02x %02x \n",DataSave0.ConstantParamData.aucGlobalTransEnable[0],	DataSave0.ConstantParamData.aucGlobalTransEnable[1]);

	return(SUCCESS);
}	

