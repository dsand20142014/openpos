/*
	SAND China
--------------------------------------------------------------------------
	FILE  set.c										(Copyright SAND 2001)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2001-07-02		Xiaoxi Jiang
    Last modified :	2001-07-02		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network
    Purpose :
        Source file for set system parameter.

    List of routines in file :

    File history :

*/

#include <include.h>
#include <global.h>
#include <py_input.h>
#include <xdata.h>

unsigned char CFG_CalcDynamicPSW(void)
{
	unsigned char aucDynamicPSW[8],aucBuf[7];
	unsigned char ucResult=SUCCESS;
		
	memset(aucDynamicPSW,0,sizeof(aucDynamicPSW));
	UTIL_GetDynamicPSW(aucDynamicPSW);

	util_Printf("\n动态密码:%s\n",aucDynamicPSW);
	memset(aucBuf,0,sizeof(aucBuf));
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(UCHAR*)"请输入系统密码:");
	if (UTIL_Input(1,true,6,6,'P',aucBuf,0) != KEY_ENTER)
	{
		ucResult = ERR_CANCEL;
	}
	else
	{
		if( memcmp(aucBuf,aucDynamicPSW,6) )
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,1,(UCHAR*)" 系统密码错");
			//Os__GB2312_display(1,1,(UCHAR*)" 功能未开启");
			UTIL_GetKey(3);
			ucResult = ERR_CASH_PASS;
		}
	}
	return(ucResult);
}
void CFG_ChangParaFlag(void)
{
	uchar ucParamFlag[50];

	memset(ucParamFlag,0x01,sizeof(ucParamFlag));
//	OSAPP_WritePrivateFile(ucParamFlag);
}
unsigned char CFG_TermIDAndMarchID(void)
{
	unsigned char 	ucResult;
	unsigned char 	aucBuf[50];

	if(DataSave0.ChangeParamData.ucCashierLogonFlag==CASH_LOGONFLAG)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(unsigned char *)" 终端为签到状态");
		//Os__GB2312_display(2,0,(unsigned char *)" 请签退后再试！");
		MSG_WaitKey(5);
		return(SUCCESS);
	}
	if(DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb
		+DataSave0.TransInfoData.TransTotal.uiTotalNb)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(unsigned char *)" 终端尚有交易");
		//Os__GB2312_display(2,0,(unsigned char *)" 请结算后再试！");
		MSG_WaitKey(5);
		return(SUCCESS);
	}

	/* Terminal ID */
	if(!ucResult)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		MSG_DisplayMenuMsg( true,0,0,MSG_TERMINALID);
		memcpy(aucBuf,DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
		if (UTIL_Input(1,true,8,8,'N',aucBuf,0) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}else
		{
			Os__saved_copy(aucBuf,
				DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
		}
	}
	/* Merchant ID */
	if(!ucResult)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		MSG_DisplayMenuMsg( true,0,0,MSG_MERCHANTID);
		memcpy(aucBuf,DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
		if (UTIL_Input(1,true,15,15,'N',aucBuf,0) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}else
		{
			Os__saved_copy(aucBuf,
				DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
		}
	}
	return(SUCCESS);
}

unsigned char CFG_SetHideCardNo(void)
{
	unsigned char aucBuf[3];
	unsigned char ucResult,ucKey,ucChar;

	util_Printf("Before Hide Card=%02x\n",DataSave0.ConstantParamData.ucPrintCardNo);

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"打印签购单");
	//Os__GB2312_display(1,0,(unsigned char *)"是否屏蔽卡号");
	//Os__GB2312_display(3,0,(unsigned char *)"1:是  0:不是");

	ucResult = SUCCESS;
	memset(aucBuf,0,sizeof(aucBuf));
	aucBuf[0] = DataSave0.ConstantParamData.ucPrintCardNo;
	//Os__GB2312_display(2,0,aucBuf);

	while(1)
	{
		ucKey = Os__xget_key();
		switch(ucKey)
		{
			case '0':
			case '1':
				Os__saved_copy(&ucKey,&DataSave0.ConstantParamData.ucPrintCardNo,sizeof(unsigned char));
				XDATA_Write_Vaild_File(DataSaveConstant);
				break;
			default:
				break;
		}
		aucBuf[0] = DataSave0.ConstantParamData.ucPrintCardNo;
		//Os__GB2312_display(2,0,aucBuf);
		if (ucKey == KEY_ENTER)
			break;
	}
	util_Printf("After Hide Card=%02x\n",DataSave0.ConstantParamData.ucPrintCardNo);

	if (DataSave0.ConstantParamData.ucPrintCardNo == 0x31)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"预授权");
		//Os__GB2312_display(1,0,(unsigned char *)"是否屏蔽卡号");
		//Os__GB2312_display(3,0,(unsigned char *)"1:是  0:不是");

		memset(aucBuf,0,sizeof(aucBuf));
		aucBuf[0] = DataSave0.ConstantParamData.ucDisAuth;
		//Os__GB2312_display(2,0,aucBuf);

		while(1)
		{
			ucKey=Os__xget_key();
			switch(ucKey)
			{
				case '0':
				case '1':
					Os__saved_copy(&ucKey,&DataSave0.ConstantParamData.ucDisAuth,sizeof(unsigned char));
					XDATA_Write_Vaild_File(DataSaveConstant);
					break;
				default:
					break;
			}
			aucBuf[0] = DataSave0.ConstantParamData.ucDisAuth;
			//Os__GB2312_display(2,0,aucBuf);
			if (ucKey == KEY_ENTER)
				break;
		}

		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"选择屏蔽卡种:");
		//Os__GB2312_display(1,0,(unsigned char *)"0.内卡");
		//Os__GB2312_display(2,0,(unsigned char *)"1.外卡");
		//Os__GB2312_display(3,0,(unsigned char *)"2.内外卡");
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,&DataSave0.ConstantParamData.ucCardtype,1);
		//Os__GB2312_display(3,6,aucBuf);

		while(1)
		{
			ucKey=Os__xget_key();
			switch(ucKey)
			{
				case '0':
				case '1':
				case '2':
					ucChar = ucKey;
					Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucCardtype,
						sizeof(unsigned char));
					XDATA_Write_Vaild_File(DataSaveConstant);
					break;
				default :
					break;
			}
			aucBuf[0] = DataSave0.ConstantParamData.ucCardtype;
			//Os__GB2312_display(3,6,aucBuf);
			util_Printf("卡号标志 = %02x\n",DataSave0.ConstantParamData.ucCardtype);
			if(ucKey == KEY_ENTER)
				break;
		}
	}
	return(ucResult);
}

unsigned char CFG_ConstantParamBaseData(void)
{
	unsigned char 	ucResult,aucIp[2];
	unsigned char 	aucBuf[50],aucTmpBuf[5];
	unsigned short 	uiSpeed,usI;
	unsigned long	traceNum;
	unsigned char	ucNextKey,ucIpType,ucChar;
	unsigned char ucInputFlag,ucCompare;

	util_Printf("\n DataSave0.ChangeParamData.ucCashierLogonFlag=%02x ",DataSave0.ChangeParamData.ucCashierLogonFlag);
	util_Printf("\n DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb=%d ",DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb);
	util_Printf("\n DataSave0.TransInfoData.TransTotal.uiTotalNb=%d ",DataSave0.TransInfoData.TransTotal.uiTotalNb);
	util_Printf("\n通讯方式:%02x\n",DataSave0.ConstantParamData.ucCommMode);
	ucResult = SUCCESS;
	ucNextKey=0x00;

	CFG_ChangParaFlag();
	ucResult=UTIL_SetReader();
	if (ucResult)	return(ucResult);
	if((DataSave0.ChangeParamData.ucCashierLogonFlag!=CASH_LOGONFLAG)
      		&&!(DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb+DataSave0.TransInfoData.TransTotal.uiTotalNb))
      {
		  	//Select Terminal Type
			ucResult = CFG_TermTypeConfig();
			util_Printf("-----11---ucResult = %02x\n",ucResult);
			if (ucResult)	return(ucResult);
				ucResult = CFG_SetHideCardNo();
			if(!ucResult)
				ucResult = CFG_SetMarchName();
			util_Printf("商户名称:%02x\n",ucResult);
      			/* Terminal ID */
			if(!ucResult)
			{
				memset(aucBuf,0,sizeof(aucBuf));
				MSG_DisplayMenuMsg(true,0,0,MSG_TERMINALID);
				ucInputFlag = PY_INPUT_NUMBER
								|PY_INPUT_UPPER_CASE;
				memcpy(aucBuf,DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);

				ucResult=PY_Main(aucBuf,1,8,8,ucInputFlag,300);
				if(ucResult != SUCCESS)
				{
					return(ERR_CANCEL);
				}
				else
				{
					Os__saved_copy(aucBuf,
						DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
					XDATA_Write_Vaild_File(DataSaveConstant);
				}
			}
			/* Merchant ID */
			if(!ucResult)
			{
				memset(aucBuf,0,sizeof(aucBuf));
				MSG_DisplayMenuMsg( true,0,0,MSG_MERCHANTID);
				memcpy(aucBuf,DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
				if (UTIL_Input(1,true,15,15,'N',aucBuf,0) != KEY_ENTER )
				{
					return(ERR_CANCEL);
				}else
				{
					Os__saved_copy(aucBuf,
						DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
					XDATA_Write_Vaild_File(DataSaveConstant);
				}
			}
	}
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"是否支持手机无卡");
	//Os__GB2312_display(1,0,(unsigned char *)"预约交易");
	//Os__GB2312_display(3,0,(unsigned char *)"1:是  0:否");
	memset(aucBuf,0,sizeof(aucBuf));
	aucBuf[0] = DataSave0.ConstantParamData.ucMobileFlag;
	ucCompare=DataSave0.ConstantParamData.ucMobileFlag;
	if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
		return(ERR_CANCEL);
	else
	{
		ucChar = aucBuf[0];
		if (ucCompare != ucChar)
		{
			if (!CFG_CalcDynamicPSW())
			{
				Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucMobileFlag,
							sizeof(unsigned char));
				XDATA_Write_Vaild_File(DataSaveConstant);
			}
		}
	}
	
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"是否支持分期付款");
	//Os__GB2312_display(1,0,(unsigned char *)"交易");
	//Os__GB2312_display(3,0,(unsigned char *)"1:是  0:否");
	memset(aucBuf,0,sizeof(aucBuf));
	aucBuf[0] = DataSave0.ConstantParamData.ucInstallment;
	ucCompare =  DataSave0.ConstantParamData.ucInstallment;
	if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
		ucResult=ERR_CANCEL;
	else
	{
		ucChar = aucBuf[0];
		if (ucCompare != ucChar)
		{	
			if (!CFG_CalcDynamicPSW())
			{
				Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucInstallment,
					sizeof(unsigned char));
				XDATA_Write_Vaild_File(DataSaveConstant);
			}
		}
	}

	/* Comm mode */
	if( ucResult == SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char*)"1.RS232 2.拨号");
		Os__display(1,0,(unsigned char*)"3.GPRS  4.CDMA");
		Os__display(2,0,(unsigned char*)"5.TCP/IP");

		memset(aucBuf,0,sizeof(aucBuf));
		aucBuf[0] = DataSave0.ConstantParamData.ucCommMode + '0';
		if (UTIL_Input(3,true,1,1,'N',aucBuf,(uchar *)"12345") != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}else
		{
			aucBuf[0] = aucBuf[0] - '0';
			Os__saved_copy(aucBuf,
				&DataSave0.ConstantParamData.ucCommMode,1);
		}
	}

	/* Dial connect wait seconds*/
	if( ucResult == SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,"30",2);
		uiSpeed = (unsigned short)str_long(aucBuf);
		Os__saved_copy((unsigned char *)&uiSpeed,
			(unsigned char *)&DataSave0.ConstantParamData.uiDialWaitTime,
			sizeof(unsigned short));
		XDATA_Write_Vaild_File(DataSaveConstant);
	}
	util_Printf("商户名称11:%02x %02x\n",ucResult,DataSave0.ConstantParamData.ucCommMode);
	if(ucResult==SUCCESS
		&&(DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_GPRS)
			||(DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_CDMA)
			)
		{
			memset(aucIp,0x01,sizeof(aucIp));
			memcpy(&DataSave0.ConstantParamData.ucTerIPType,aucIp,1);
			ucIpType=DataSave0.ConstantParamData.ucTerIPType;
			XDATA_Write_Vaild_File(DataSaveConstant);
			ucResult=SEL_HostIP(ucIpType);
		}
		if(ucResult==SUCCESS&&(DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_TCPIP))
		{
			memset(aucIp,0x02,sizeof(aucIp));
			memcpy(&DataSave0.ConstantParamData.ucTerIPType,aucIp,1);
			ucIpType=DataSave0.ConstantParamData.ucTerIPType;
			XDATA_Write_Vaild_File(DataSaveConstant);
			ucResult=SEL_HostIP(ucIpType);
		}

		if(ucResult==SUCCESS&&(DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_TCPIP)
			||(DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_GPRS)
			||(DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_CDMA))
		{
					ucResult=SEL_HostCopyIP(ucIpType);
					util_Printf("CopyIP---ucResult=%02x\n",ucResult);
					if(!ucResult)
						ucResult=SEL_HostPorts();
					util_Printf("\nPort---ucResult=%02x\n",ucResult);
					if (!ucResult)
						ucResult=SEL_HostBackPorts();
		}
		util_Printf("www.ucResult=%02x\n",ucResult);
		if( ucResult == SUCCESS
			&& ((DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_TCPIP)
			||(DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_GPRS)
			||(DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_CDMA))
			)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			//Os__clr_display(255);
			//Os__GB2312_display(0, 0, (unsigned char *)"是否开启");
			//Os__GB2312_display(1, 0, (unsigned char *)"数据加密功能");
			//Os__GB2312_display(3, 0, (unsigned char *)"0:否        1:是");

			aucBuf[0] = DataSave0.ConstantParamData.ucEncryptDataFlag;
	   		if (UTIL_Input(2,true,1,1,'N',aucBuf,(uchar *)"01") != KEY_ENTER )
			{
				return(ERR_CANCEL);
			}else
			{
				ucChar = aucBuf[0];
				Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucEncryptDataFlag,
					sizeof(unsigned char));
			}
			if (DataSave0.ConstantParamData.ucEncryptDataFlag==0x30)
			    DataSave0.ConstantParamData.ucMBFEFlag=0x30;
			XDATA_Write_Vaild_File(DataSaveConstant);
		}

		if((ucResult==SUCCESS)
			&&(DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_GPRS))
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar*)"请输入APN:");
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,DataSave0.ConstantParamData.aucAPN,PARAM_APN);
			if( (ucResult = OSUTIL_Input(0,1,0,PARAM_APN,'A',aucBuf,NULL)) != SUCCESS )
			{
				return(ERR_CANCEL);
			}
			else
			{
				memset(DataSave0.ConstantParamData.aucAPN,0,sizeof(DataSave0.ConstantParamData.aucAPN));
				memcpy(DataSave0.ConstantParamData.aucAPN,aucBuf,PARAM_APN);
			}
		}
		if (!ucResult)
		{
			XDATA_Write_Vaild_File(DataSaveConstant);
		}

	/* Receive Data Timeout */

	if( (ucResult == SUCCESS)
	&&( (DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_MODEM)
	||(DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_HDLC) )
	)
	{
		/* HOST Tel 1 */
		if( ucResult == SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			MSG_DisplayMenuMsg( true,0,0,MSG_HOSTTEL1);
			memcpy(aucBuf,DataSave0.ConstantParamData.aucHostTelNumber1,PARAM_TELNUMBERLEN);
			for(usI=0;usI<PARAM_TELNUMBERLEN;usI++)
			{
				if(aucBuf[usI] == '.')
				{
					aucBuf[usI] = 0;
					break;
				}
			}
	        memset(&aucBuf[usI],0,PARAM_TELNUMBERLEN-usI);
			if (UTIL_Input(1,true,1,PARAM_TELNUMBERLEN,'N',aucBuf,0) != KEY_ENTER )
			{
				ucResult = ERR_CANCEL;
			}else
			{
				aucBuf[strlen((char *)aucBuf)] = '.';
				Os__saved_copy(aucBuf,
					DataSave0.ConstantParamData.aucHostTelNumber1,PARAM_TELNUMBERLEN);
			}
		}
		/* HOST Tel 2 */
		if( ucResult == SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			MSG_DisplayMenuMsg( true,0,0,MSG_HOSTTEL2);
			memcpy(aucBuf,DataSave0.ConstantParamData.aucHostTelNumber2,PARAM_TELNUMBERLEN);
			for(usI=0;usI<PARAM_TELNUMBERLEN;usI++)
			{
				if(aucBuf[usI] == '.')
				{
					aucBuf[usI] = 0;
					break;
				}
			}
	        	memset(&aucBuf[usI],0,PARAM_TELNUMBERLEN-usI);
			if (UTIL_Input(1,true,1,PARAM_TELNUMBERLEN,'N',aucBuf,0) != KEY_ENTER )
			{
				ucResult = ERR_CANCEL;
			}else
			{
				aucBuf[strlen((char *)aucBuf)] = '.';
				Os__saved_copy(aucBuf,
					DataSave0.ConstantParamData.aucHostTelNumber2,PARAM_TELNUMBERLEN);
			}
		}
		/* HOST Tel 3 */
		if( ucResult == SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			MSG_DisplayMenuMsg( true,0,0,MSG_HOSTTEL3);
			memcpy(aucBuf,DataSave0.ConstantParamData.aucHostTelNumber3,PARAM_TELNUMBERLEN);
			for(usI=0;usI<PARAM_TELNUMBERLEN;usI++)
			{
				if(aucBuf[usI] == '.')
				{
					aucBuf[usI] = 0;
					break;
				}
			}
	        	memset(&aucBuf[usI],0,PARAM_TELNUMBERLEN-usI);
			if (UTIL_Input(1,true,1,PARAM_TELNUMBERLEN,'N',aucBuf,0) != KEY_ENTER )
			{
				ucResult = ERR_CANCEL;
			}else
			{
				aucBuf[strlen((char *)aucBuf)] = '.';
				Os__saved_copy(aucBuf,
					DataSave0.ConstantParamData.aucHostTelNumber3,PARAM_TELNUMBERLEN);
			}
		}

		/* HOST Tel 4 */
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			MSG_DisplayMenuMsg( true,0,0,MSG_HOSTTEL4);
			memcpy(aucBuf,DataSave0.ConstantParamData.aucHostTelNumber4,PARAM_TELNUMBERLEN);
			for(usI=0;usI<PARAM_TELNUMBERLEN;usI++)
			{
				if(aucBuf[usI] == '.')
				{
					aucBuf[usI] = 0;
					break;
				}
			}
	        	memset(&aucBuf[usI],0,PARAM_TELNUMBERLEN-usI);
			if (UTIL_Input(1,true,1,PARAM_TELNUMBERLEN,'N',aucBuf,0) != KEY_ENTER )
			{
				ucResult = ERR_CANCEL;
			}else
			{
				aucBuf[strlen((char *)aucBuf)] = '.';
				Os__saved_copy(aucBuf,
					DataSave0.ConstantParamData.aucHostTelNumber4,PARAM_TELNUMBERLEN);
			}
			XDATA_Write_Vaild_File(DataSaveConstant);
		}
	}

	if(ucResult == SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		MSG_DisplayMenuMsg( true,0,0,MSG_TPDU);
		bcd_asc(aucBuf,DataSave0.ConstantParamData.aucTpdu,4);
		if (UTIL_Input(1,true,4,4,'N',aucBuf,0) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}else
		{
			asc_bcd(aucTmpBuf,2,aucBuf,4);
			Os__saved_copy(aucTmpBuf,(unsigned char *)&DataSave0.ConstantParamData.aucTpdu,2);
		}
	}
	XDATA_Write_Vaild_File(DataSaveConstant);
	if( ucResult == SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		MSG_DisplayMenuMsg( true,0,0,MSG_TRACENUM);
		long_asc(aucBuf,6,&DataSave0.ChangeParamData.ulTraceNumber);
		if (UTIL_Input(1,true,6,6,'N',aucBuf,0) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}else
		{
			traceNum = asc_long(aucBuf,6);
			Os__saved_copy((unsigned char *)&traceNum,(unsigned char *)&DataSave0.ChangeParamData.ulTraceNumber,sizeof(unsigned long));
			XDATA_Write_Vaild_File(DataSaveChange);
		}
	}
    if(	(DataSave0.ChangeParamData.ucCashierLogonFlag!=CASH_LOGONFLAG)
      &&(ucResult == SUCCESS)
      )
	{
		memset(aucBuf,0,sizeof(aucBuf));
		MSG_DisplayMenuMsg( true,0,0,MSG_BATCHNUM);
		long_asc(aucBuf,6,&DataSave0.ChangeParamData.ulBatchNumber);
		if (UTIL_Input(1,true,6,6,'N',aucBuf,0) != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}else
		{
			traceNum = asc_long(aucBuf,6);
			Os__saved_copy((unsigned char *)&traceNum,(unsigned char *)&DataSave0.ChangeParamData.ulBatchNumber,sizeof(unsigned long));
			XDATA_Write_Vaild_File(DataSaveChange);
		}
	}
	if( !ucResult
		&& !(DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb+DataSave0.TransInfoData.TransTotal.uiTotalNb)
		)
	{
		while(1)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"最大交易笔数:");
			int_asc(aucBuf,3,&TRANS_MAXNB);
			if (UTIL_Input(1,true,3,3,'N',aucBuf,0) != KEY_ENTER )
			{
				ucResult = ERR_CANCEL;
			}else
			{
				traceNum = asc_long(aucBuf,3);
				if(traceNum>TRANS_SAVEMAXNB)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(1,0,(unsigned char *)"超出最大交易笔数");
					//Os__GB2312_display(2,0,(unsigned char *)"    请重新输入");
					MSG_WaitKey(2);
					continue;
				}else
				if(traceNum<300)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(1,0,(unsigned char *)"  交易笔数不足");
					//Os__GB2312_display(2,0,(unsigned char *)"    请重新输入");
					MSG_WaitKey(2);
					continue;
				}
				else
				{
					Os__saved_copy((unsigned char *)&traceNum,(unsigned char *)&TRANS_MAXNB,sizeof(unsigned int));
					XDATA_Write_Vaild_File(DataSaveChange);
					break;
				}
			}
		}
	}

	util_Printf("333333DataSave0.ConstantParamData.aucGlobalTransEnable = [%02x][%02x]",
			DataSave0.ConstantParamData.aucGlobalTransEnable[0],
			DataSave0.ConstantParamData.aucGlobalTransEnable[1]);
	return(ucResult);
}

void CFG_DefaultParam(void)
{
#if PS400
	Os__saved_set(&DataSave0.ConstantParamData.ucCommMode,3,1);
#else
	DataSave0.ConstantParamData.ucCommMode =PARAM_COMMMODE_HDLC;
#endif
	if(DataSave0.ConstantParamData.ucTerminalType  == TYPE_SHOP )//百货
	{
		DataSave0.ConstantParamData.uiReceiveTimeout = 30;
		DataSave0.ConstantParamData.ucXorTipFee = 0;
		DataSave0.ConstantParamData.usTipFeePre = 0;
		DataSave0.ConstantParamData.ulRefundMaxAmount=999999999;
		DataSave0.ConstantParamData.ucDialRetry=3;

		DataSave0.ConstantParamData.ucTransRetry = 3;
		DataSave0.ConstantParamData.ucOFFLineSendType = '1';
		DataSave0.ConstantParamData.ucPrnErrTicket = '0';
		DataSave0.ConstantParamData.ucUnSaleSwipe = '1';
		DataSave0.ConstantParamData.ucUnManagePwd = '0';
		DataSave0.ConstantParamData.ucUnAuthFinSwipe = '1';
		DataSave0.ConstantParamData.ucVoidInputPin = '1';
		DataSave0.ConstantParamData.ucPREAUTHFINISHInputPin = '1';
		DataSave0.ConstantParamData.ucDefaultTransParam=TRANS_PURCHASE;
		DataSave0.ConstantParamData.ENCRYPTTYPEParam=ENCRYPTTYPE_SINGLE;
		DataSave0.ConstantParamData.ucMagPreauthFinish='3';
		DataSave0.ConstantParamData.ucUndoOff = '0';
		memcpy(DataSave0.ConstantParamData.aucGlobalTransEnable,
					"\x86\x10" , PARAM_GLOBALTRANSENABLELEN);//"\x07\x10"
#if 1
		Os__saved_copy((unsigned char *)"4008200358.",//33034500
					DataSave0.ConstantParamData.aucHostTelNumber1,10);
		Os__saved_copy((unsigned char *)"66694500.",
					DataSave0.ConstantParamData.aucHostTelNumber2,9);
		Os__saved_copy((unsigned char *)"66674500.",
					DataSave0.ConstantParamData.aucHostTelNumber3,9);
		memset(DataSave0.ConstantParamData.aucHostTelNumber4,0,PARAM_TELNUMBERLEN);
		Os__saved_copy((unsigned char *)"123456.",
					DataSave0.ConstantParamData.aucHostTelNumber4,7);

		Os__saved_copy((unsigned char *)"\x04\x01",
					DataSave0.ConstantParamData.aucTpdu,2);
#endif
		XDATA_Write_Vaild_File(DataSaveConstant);
	}
	else
	if(DataSave0.ConstantParamData.ucTerminalType == TYPE_HOTEL)//宾馆
	{
		DataSave0.ConstantParamData.uiReceiveTimeout = 30;
		DataSave0.ConstantParamData.ucXorTipFee = 0;
		DataSave0.ConstantParamData.usTipFeePre = 0;
		DataSave0.ConstantParamData.ulRefundMaxAmount=999999999;
		DataSave0.ConstantParamData.ucDialRetry=3;

		DataSave0.ConstantParamData.ucTransRetry = 3;
		DataSave0.ConstantParamData.ucOFFLineSendType = '1';
		DataSave0.ConstantParamData.ucPrnErrTicket = '0';
		DataSave0.ConstantParamData.ucUnSaleSwipe = '1';
		DataSave0.ConstantParamData.ucUnManagePwd = '0';
		DataSave0.ConstantParamData.ucUnAuthFinSwipe = '1';
		DataSave0.ConstantParamData.ucVoidInputPin = '1';
		DataSave0.ConstantParamData.ucPREAUTHFINISHInputPin = '1';
		DataSave0.ConstantParamData.ucDefaultTransParam=TRANS_PURCHASE;
		DataSave0.ConstantParamData.ENCRYPTTYPEParam=ENCRYPTTYPE_SINGLE;
		DataSave0.ConstantParamData.ucMagPreauthFinish='3';
		DataSave0.ConstantParamData.ucUndoOff = '0';

		memcpy(DataSave0.ConstantParamData.aucGlobalTransEnable,
					"\xFE\xB4" , PARAM_GLOBALTRANSENABLELEN);//"\x7F\x14"
#if 1
		Os__saved_copy((unsigned char *)"4008200358.",//33034500
					DataSave0.ConstantParamData.aucHostTelNumber1,10);
		Os__saved_copy((unsigned char *)"66694500.",
					DataSave0.ConstantParamData.aucHostTelNumber2,9);
		Os__saved_copy((unsigned char *)"66674500.",
					DataSave0.ConstantParamData.aucHostTelNumber3,9);
		memset(DataSave0.ConstantParamData.aucHostTelNumber4,0,PARAM_TELNUMBERLEN);
		Os__saved_copy((unsigned char *)"123456.",
					DataSave0.ConstantParamData.aucHostTelNumber4,7);
		Os__saved_copy((unsigned char *)"\x04\x01",
					DataSave0.ConstantParamData.aucTpdu,2);
#endif
		XDATA_Write_Vaild_File(DataSaveConstant);
	}
	else
	if(DataSave0.ConstantParamData.ucTerminalType == RESTAURANT)//餐饮
	{
		DataSave0.ConstantParamData.uiReceiveTimeout = 30;
		DataSave0.ConstantParamData.ucXorTipFee = 0;
		DataSave0.ConstantParamData.usTipFeePre = 0;
		DataSave0.ConstantParamData.ulRefundMaxAmount=999999999;
		DataSave0.ConstantParamData.ucDialRetry=3;

		DataSave0.ConstantParamData.ucTransRetry = 3;
		DataSave0.ConstantParamData.ucOFFLineSendType = '1';
		DataSave0.ConstantParamData.ucPrnErrTicket = '0';
		DataSave0.ConstantParamData.ucUnSaleSwipe = '1';
		DataSave0.ConstantParamData.ucUnManagePwd = '0';
		DataSave0.ConstantParamData.ucUnAuthFinSwipe = '1';
		DataSave0.ConstantParamData.ucVoidInputPin = '1';
		DataSave0.ConstantParamData.ucPREAUTHFINISHInputPin = '1';
		DataSave0.ConstantParamData.ucDefaultTransParam=TRANS_PURCHASE;
		DataSave0.ConstantParamData.ENCRYPTTYPEParam=ENCRYPTTYPE_SINGLE;
		DataSave0.ConstantParamData.ucMagPreauthFinish='3';
		DataSave0.ConstantParamData.ucUndoOff = '0';

		memcpy(DataSave0.ConstantParamData.aucGlobalTransEnable,
					"\x86\x10" , PARAM_GLOBALTRANSENABLELEN);  //"\x06\x50"
#if 1
		Os__saved_copy((unsigned char *)"4008200358.",
					DataSave0.ConstantParamData.aucHostTelNumber1,10);
		Os__saved_copy((unsigned char *)"66694500.",
					DataSave0.ConstantParamData.aucHostTelNumber2,9);
		Os__saved_copy((unsigned char *)"66674500.",
					DataSave0.ConstantParamData.aucHostTelNumber3,9);
		memset(DataSave0.ConstantParamData.aucHostTelNumber4,0,PARAM_TELNUMBERLEN);
		Os__saved_copy((unsigned char *)"123456.",
					DataSave0.ConstantParamData.aucHostTelNumber4,7);

		Os__saved_copy((unsigned char *)"\x04\x01",
					DataSave0.ConstantParamData.aucTpdu,2);
#endif
		XDATA_Write_Vaild_File(DataSaveConstant);
		//XDATA_Read_Vaild_File(DataSaveConstant);
	}
		util_Printf("DataSave0.ConstantParamData.aucGlobalTransEnable = [%02x][%02x]",
			DataSave0.ConstantParamData.aucGlobalTransEnable[0],
			DataSave0.ConstantParamData.aucGlobalTransEnable[1]);
}

unsigned char CFG_ConstantParamPBOCData(void)
{
	unsigned char ucResult;
	unsigned char aucBuf[50];

	ucResult = SUCCESS;
	/* PBOC Application Select Flag */
#if 1
	if( ucResult == SUCCESS)
	{
		MSG_DisplayMenuMsg( true,0,0,MSG_DOAPPSELECT);
		memset(aucBuf,0,sizeof(aucBuf));
		aucBuf[0] = DataSave0.ConstantParamData.ucPBOCAppSelectFlag+'0';
		if (UTIL_Input(1,true,1,1,'N',aucBuf,(uchar *)"01") != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}else
		{
			aucBuf[0] = aucBuf[0] - '0';
			Os__saved_copy(aucBuf,
				&DataSave0.ConstantParamData.ucPBOCAppSelectFlag,1);
		}
	}
#endif
	/* PBOC PSAM Reader */
	if( ucResult == SUCCESS)
	{
		MSG_DisplayMenuMsg( true,0,0,MSG_PSAMREADER);
		memset(aucBuf,0,sizeof(aucBuf));
		aucBuf[0] = DataSave0.ConstantParamData.ucPBOCPSAMReader+'0';
		if (UTIL_Input(1,true,1,1,'N',aucBuf,(uchar *)"1234") != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}else
		{
			aucBuf[0] = aucBuf[0] - '0';
			Os__saved_copy(aucBuf,
				&DataSave0.ConstantParamData.ucPBOCPSAMReader,1);
		}
	}

	XDATA_Write_Vaild_File(DataSaveConstant);

	return(ucResult);
}
unsigned char CFG_ConstantParamDateTime(void)
{
	unsigned char Date_Time[17];
	unsigned char tmp_date[17];
	unsigned char ucChar;

	memset(Date_Time,0,sizeof(Date_Time));
	memset(tmp_date,0,sizeof(tmp_date));

	MSG_DisplayMenuMsg( true,0,0,MSG_DATETIME);
	if(UTIL_Input(1,true,10,10,'N',tmp_date,0) != KEY_ENTER)
	{
		return(ERR_CANCEL);
	}

	Date_Time[0] = tmp_date[4];
	Date_Time[1] = tmp_date[5];
	Date_Time[2] = tmp_date[2];
	Date_Time[3] = tmp_date[3];
	Date_Time[4] = tmp_date[0];
	Date_Time[5] = tmp_date[1];

#ifdef TEST
	util_Printf("Date_Time=%s\n",Date_Time);
#endif

	ucChar=Os__write_date(Date_Time);
#ifdef TEST
	util_Printf("ucChar=%02x\n",ucChar);
#endif

	memset(Date_Time,0,sizeof(Date_Time));

	memcpy(Date_Time,&tmp_date[6],4);

#ifdef TEST
	util_Printf("Date_Time1=%s\n",Date_Time);
#endif

	ucChar=Os__write_time(Date_Time);
#ifdef TEST
	util_Printf("ucChar1=%02x\n",ucChar);
#endif
	return(OK);

}

unsigned char CFG_ConstantParamPhoneNo(void)
{
    unsigned char ucResult,ucKey;
    unsigned char aucBuf[PARAM_SWITCHLEN],ucCh;
    unsigned short uiTmp;
	unsigned long ulTmp;

    ucResult = SUCCESS;
    /* Save all constant param to mirror */

    if( ucResult == SUCCESS )
    {
        memset(aucBuf,0,sizeof(aucBuf));
        MSG_DisplayMenuMsg( true,0,0,MSG_SWITCHBOARD);
        memcpy(aucBuf,DataSave0.ConstantParamData.aucSwitchBoard,PARAM_SWITCHLEN);

        //if (UTIL_Input(1,true,0,PARAM_SWITCHLEN,'A',aucBuf,0) != KEY_ENTER )
        if( (ucResult = OSUTIL_Input(50,1,0,PARAM_SWITCHLEN,'A',aucBuf,NULL)) != SUCCESS )
        {
            return(ERR_CANCEL);
        }else
        {
            Os__saved_copy(aucBuf,DataSave0.ConstantParamData.aucSwitchBoard,PARAM_SWITCHLEN);
            XDATA_Write_Vaild_File(DataSaveConstant);
        }
    }
    //Os__clr_display(255);
    //Os__GB2312_display(0,0,(unsigned char *)"如果没有特殊要求");
    //Os__GB2312_display(1,0,(unsigned char *)"必须设成默认设置");
    //Os__GB2312_display(2,0,(unsigned char *)"是否设成默认设置");
    //Os__GB2312_display(3,0,(unsigned char *)"确认/取消？");

	ucKey=MSG_WaitKey(30);
    if(ucKey==KEY_ENTER)
    {
        unsigned char aucModemParam[10]={0,0,8,6,115,8,1,3,0,0};
        Os__saved_copy(aucModemParam,(uchar *)&DataSave0.ConstantParamData.ModemParam,10);
        XDATA_Write_Vaild_File(DataSaveConstant);
        return SUCCESS;
    }else
    {
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"你未选择默认设置");
        //Os__GB2312_display(1,0,(unsigned char *)"请设置参数");
        MSG_WaitKey(5);
    }
    while(1)
    {
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"音频/脉冲：");
        //Os__GB2312_display(1,0,(unsigned char *)"1.音频  2.脉冲");
        memset(aucBuf,0,sizeof(aucBuf));
		if(DataSave0.ConstantParamData.ModemParam.dp==0x00)
		{
		   aucBuf[0]=0x31;
		}else if (DataSave0.ConstantParamData.ModemParam.dp==0x01)
		{
		  aucBuf[0]=0x32;
		}
        if (UTIL_Input(2,false,1,1,'N',aucBuf,(uchar *)"12") != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
            break;
        }else
        {
            if(aucBuf[0] == '1')
                Os__saved_set(&DataSave0.ConstantParamData.ModemParam.dp,0,1);
            else
                Os__saved_set(&DataSave0.ConstantParamData.ModemParam.dp,1,1);
            XDATA_Write_Vaild_File(DataSaveConstant);
            break;
        }
    }
    while(1)
    {
        memset(aucBuf,0,sizeof(aucBuf));
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"检测拨号音：");
        //Os__GB2312_display(1,0,(unsigned char *)"1.是  2.否");
        //aucBuf[0] = '1';
		if(DataSave0.ConstantParamData.ModemParam.chdt==0x00)
		{
			aucBuf[0]=0x31;
		}
		else if(DataSave0.ConstantParamData.ModemParam.chdt==0x01)
		{
			aucBuf[0]=0x32;
		}
        if (UTIL_Input(2,true,1,1,'N',aucBuf,(uchar *)"12") != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
            break;
        }else
        {
            if(aucBuf[0] == '1')
                Os__saved_set(&DataSave0.ConstantParamData.ModemParam.chdt,0,1);
            else
                Os__saved_set(&DataSave0.ConstantParamData.ModemParam.chdt,1,1);
            ucResult = SUCCESS;
            XDATA_Write_Vaild_File(DataSaveConstant);
            break;
        }
    }
    while(1)
    {
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"摘机拨号等待时间");

        memset(aucBuf,0,sizeof(aucBuf));
		ulTmp=DataSave0.ConstantParamData.ModemParam.dt1;
		if(ulTmp/100>0)
			long_asc(aucBuf,3,&ulTmp);
		else if(ulTmp/10>0)
			long_asc(aucBuf,2,&ulTmp);
		else
			long_asc(aucBuf,1,&ulTmp);
        if (UTIL_Input(1,true,1,3,'N',(uchar *)aucBuf,0) != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
            break;
        }else
        {
            uiTmp = (unsigned short)asc_long(aucBuf,strlen((char *)aucBuf));
            if((uiTmp <4)||(uiTmp>255))
            {
                //Os__clr_display(255);
                //Os__GB2312_display(0,0,(unsigned char *)"数值必须在4到");
                //Os__GB2312_display(1,0,(unsigned char *)"255之间！");
                MSG_WaitKey(8);
                continue;
            }
            ucCh = uiTmp;
            Os__saved_set(&DataSave0.ConstantParamData.ModemParam.dt1,ucCh,1);
            ucResult = SUCCESS;
            XDATA_Write_Vaild_File(DataSaveConstant);
            break;
        }
    }

    while(1)
    {

        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"拨外线等待时间：");
        //aucBuf[0]='6';
		memset(aucBuf,0,sizeof(aucBuf));
		ulTmp=DataSave0.ConstantParamData.ModemParam.dt2;
		if(ulTmp/100>0)
			long_asc(aucBuf,3,&ulTmp);
		else if(ulTmp/10>0)
			long_asc(aucBuf,2,&ulTmp);
		else
			long_asc(aucBuf,1,&ulTmp);
        if (UTIL_Input(1,true,1,3,'N',aucBuf,0) != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
            break;
        }else
        {
            uiTmp = (unsigned short)asc_long(aucBuf,strlen((char *)aucBuf));
            if(uiTmp>255)
            {
                //Os__clr_display(255);
                //Os__GB2312_display(0,0,(unsigned char *)"数值必须小于");
                //Os__GB2312_display(1,0,(unsigned char *)"255！");
                MSG_WaitKey(8);
                continue;
            }
            ucCh = uiTmp;
            Os__saved_set(&DataSave0.ConstantParamData.ModemParam.dt2,ucCh,1);
            ucResult = SUCCESS;
            ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
            break;
        }
    }
    while(1)
    {
      //Os__clr_display(255);
      //Os__GB2312_display(0,0,(unsigned char *)"单一号码保持时间");
      ulTmp=DataSave0.ConstantParamData.ModemParam.ht;
      memset(aucBuf,0,sizeof(aucBuf));
		if(ulTmp/100>0)
			long_asc(aucBuf,3,&ulTmp);
		else if(ulTmp/10>0)
			long_asc(aucBuf,2,&ulTmp);
		else
			long_asc(aucBuf,1,&ulTmp);
        if (UTIL_Input(1,true,1,3,'N',aucBuf,0) != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
            break;
        }else
        {
            uiTmp = (unsigned short)asc_long(aucBuf,strlen((char *)aucBuf));
            if((uiTmp < 70)||(uiTmp>255))
            {
                //Os__clr_display(255);
                //Os__GB2312_display(0,0,(unsigned char *)"数值必须在70");
                //Os__GB2312_display(1,0,(unsigned char *)"到255之间！");
                MSG_WaitKey(8);
                continue;
            }
            ucCh = uiTmp;
			//util_Printf("uiTmp=%d\n",uiTmp);
			//util_Printf("ucCh=%d\n",ucCh);
            Os__saved_set(&DataSave0.ConstantParamData.ModemParam.ht,ucCh,1);
            ucResult = SUCCESS;
            XDATA_Write_Vaild_File(DataSaveConstant);
            break;
        }
    }
    while(1)
    {

        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"拨号间隔时间：");
        //aucBuf[0]='8';
		memset(aucBuf,0,sizeof(aucBuf));
		ulTmp=DataSave0.ConstantParamData.ModemParam.wt;
		if(ulTmp/100>0)
			long_asc(aucBuf,3,&ulTmp);
		else if(ulTmp/10>0)
			long_asc(aucBuf,2,&ulTmp);
		else
			long_asc(aucBuf,1,&ulTmp);
        if (UTIL_Input(1,true,1,2,'N',aucBuf,0) != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
            break;
        }else
        {
            uiTmp = (unsigned short)asc_long(aucBuf,strlen((char *)aucBuf));
			//util_Printf("uiTmp4=%d\n",uiTmp);
            if((uiTmp < 7)||(uiTmp>25))
            {
                //Os__clr_display(255);
                //Os__GB2312_display(0,0,(unsigned char *)"数值必须在7");
                //Os__GB2312_display(1,0,(unsigned char *)"到25之间！");
                MSG_WaitKey(8);
                continue;
            }
            ucCh = uiTmp;
			//util_Printf("uiTmp4=%d\n",ucCh);
            Os__saved_set(&DataSave0.ConstantParamData.ModemParam.wt,ucCh,1);
			//util_Printf("wt=%d \n",DataSave0.ConstantParamData.ModemParam.wt);
            ucResult = SUCCESS;
            XDATA_Write_Vaild_File(DataSaveConstant);
            break;
        }
    }

 /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	if(!ucResult)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		ucCh = 0;
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"同步异步选择：");
		//Os__GB2312_display(1,0,(unsigned char *)"1.同步  2.异步");
		memset(aucBuf,0x00,sizeof(aucBuf));
		aucBuf[0] = '1';
		if (UTIL_Input(2,true,1,1,'N',aucBuf,(uchar *)"12") != KEY_ENTER )
		{
			ucResult = ERR_CANCEL;
		}else
		{
			if(aucBuf[0] == '2')
				ucCh |=0x80;
			ucResult = SUCCESS;
			util_Printf("ucCh1=%02x \n",ucCh);
		}
	}
    if(ucResult == SUCCESS)
    {
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"波特率设置：");
        //Os__GB2312_display(1,0,(unsigned char *)"1-1200  2-2400");
		util_Printf("波特率设置aucBuf=%02x",aucBuf[0]);
        if (UTIL_Input(2,true,1,1,'N',aucBuf,(uchar *)"12") != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
        }else
        {
            if(aucBuf[0] == '1')
                ucCh |=0x00;
            else
                ucCh |=0x20;
            ucResult = SUCCESS;
        }
    }
    if(ucResult == SUCCESS)
    {
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"线路制式：");
        //Os__GB2312_display(1,0,(unsigned char *)"1.CCITT  2.BELL");
        memset(aucBuf,0x00,sizeof(aucBuf));
        aucBuf[0] = '1';
        if (UTIL_Input(2,true,1,1,'N',aucBuf,(uchar *)"12") != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
        }else
        {
            if(aucBuf[0] == '1')
                ucCh |=0x00;
            else
                ucCh |=0x10;
            ucResult = SUCCESS;
        }
    }
 /*++++++++++++++++++++++++++++++++++++++++end++++++++++++++++++++++++++++++++++++++++*/
    if(ucResult == SUCCESS)
    {
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"拨号响应超时时间");
        //Os__GB2312_display(1,0,(unsigned char *)"横线前代表按键");
        //Os__GB2312_display(2,0,(unsigned char *)"横线后代表秒数");
		//Os__GB2312_display(3,0,(unsigned char *)"按任意键继续");

        MSG_WaitKey(60);
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"1-5 2-8 3-11");
        //Os__GB2312_display(1,0,(unsigned char *)"4-14 5-17 6-20");
        //Os__GB2312_display(2,0,(unsigned char *)"7-23 8-26 ");
        memset(aucBuf,0x00,sizeof(aucBuf));
		switch(DataSave0.ConstantParamData.ModemParam.ssetup)
		{
			case 0x00:
				aucBuf[0]=0x31;
				break;
			case 0x01:
				aucBuf[0]=0x32;
				break;
			case 0x02:
				aucBuf[0]=0x33;
				break;
			case 0x03:
				aucBuf[0]=0x34;
				break;
			case 0x04:
				aucBuf[0]=0x35;
				break;
			case 0x05:
				aucBuf[0]=0x36;
				break;
			case 0x06:
				aucBuf[0]=0x37;
				break;
			case 0x07:
				aucBuf[0]=0x38;
				break;
			default:
				aucBuf[0]=0x32;
		}
        if (UTIL_Input(3,true,1,1,'N',aucBuf,(uchar *)"12345678") != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
        }else
        {
            switch(aucBuf[0])
            {

                case '1':
                    ucCh |= 0x00;
                    break;
                case '2':
                    ucCh |= 0x01;
                    break;
                case '3':
                    ucCh |= 0x02;
                    break;
                case '4':
                    ucCh |= 0x03;
                    break;
                case '5':
                    ucCh |= 0x04;
                    break;
                case '6':
                    ucCh |= 0x05;
                    break;
                case '7':
                    ucCh |= 0x06;
                    break;
                case '8':
                    ucCh |= 0x07;
                    break;
            }
            ucResult = SUCCESS;
        }
    }
    if(ucResult == SUCCESS)
    {
        Os__saved_set(&DataSave0.ConstantParamData.ModemParam.ssetup,ucCh,1);
        XDATA_Write_Vaild_File(DataSaveConstant);
    }

    while(1)
    {
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"拨号循环次数");
		 memset(aucBuf,0,sizeof(aucBuf));
		aucBuf[0]=DataSave0.ConstantParamData.ModemParam.dtimes+0x30;

        if (UTIL_Input(1,true,1,1,'N',aucBuf,0) != KEY_ENTER )
        {
            ucResult = ERR_CANCEL;
            break;
        }else
        {
            uiTmp = (unsigned short)asc_long(aucBuf,strlen((char *)aucBuf));
            ucCh = uiTmp;
            Os__saved_set(&DataSave0.ConstantParamData.ModemParam.dtimes,ucCh,1);
            ucResult = SUCCESS;
            XDATA_Write_Vaild_File(DataSaveConstant);
            break;
        }
    }
    Os__saved_set(&DataSave0.ConstantParamData.ModemParam.timeout,0,1);
    Os__saved_set(&DataSave0.ConstantParamData.ModemParam.asmode,0,1);

    {
        unsigned char ucI;
        util_Printf("\nend--ModemParam = ");
        for(ucI = 0;ucI<10;ucI++)
            util_Printf("%d ",*(&DataSave0.ConstantParamData.ModemParam.dp+ucI));
            util_Printf("\n%d ",DataSave0.ConstantParamData.ModemParam.dp);
            util_Printf("%d ",DataSave0.ConstantParamData.ModemParam.chdt);
            util_Printf("%d ",DataSave0.ConstantParamData.ModemParam.dt1);
            util_Printf("%d ",DataSave0.ConstantParamData.ModemParam.dt2);
            util_Printf("%d ",DataSave0.ConstantParamData.ModemParam.ht);
            util_Printf("%d ",DataSave0.ConstantParamData.ModemParam.wt);
            util_Printf("%d ",DataSave0.ConstantParamData.ModemParam.ssetup);
            util_Printf("%d ",DataSave0.ConstantParamData.ModemParam.dtimes);
            util_Printf("%d ",DataSave0.ConstantParamData.ModemParam.timeout);
            util_Printf("%d \n",DataSave0.ConstantParamData.ModemParam.asmode);
    }

    XDATA_Write_Vaild_File(DataSaveConstant);

    return(ucResult);
}

unsigned char CFG_ConstantParamTotal(void)
{
	unsigned char KEY,ucReFile;

	//Os__clr_display(255);
	//Os__GB2312_display(0, 0, (uchar * )"是否清除交易记录！");
	//Os__GB2312_display(3, 0, (uchar * )"清除按[确认]键");
	KEY = UTIL_WaitGetKey(50);
	if(KEY != KEY_ENTER)
		return SUCCESS;

/*
	Os__saved_copy((unsigned char *)&ulBatchNumber,
				(unsigned char *)&DataSave0.ChangeParamData.ulBatchNumber,
				sizeof(ULONG_C51));
*/
	Os__saved_set((unsigned char *)DataSave0.TransInfoData.auiTransIndex,
				0,sizeof(unsigned short)*TRANS_MAXNB);
	Os__saved_copy((unsigned char *)&DataSave0.TransInfoData.TransTotal,
				(unsigned char *)&DataSave0.TransInfoData.LastTransTotal,
				sizeof(TRANSTOTAL));
	Os__saved_copy((unsigned char *)&DataSave0.TransInfoData.ForeignTransTotal,
				(unsigned char *)&DataSave0.TransInfoData.LastForeignTransTotal,
				sizeof(TRANSTOTAL));
	Os__saved_set((unsigned char *)&DataSave0.TransInfoData.ForeignTransTotal,
				0,sizeof(TRANSTOTAL));
	Os__saved_set((unsigned char *)&DataSave0.TransInfoData.TransTotal,
				0,sizeof(TRANSTOTAL));
	Os__saved_set(&DataSave0.ChangeParamData.ucCashierLogonFlag,0,1);
	Os__saved_set((unsigned char *)&DataSave0.ConstantParamData.aucSettleFlag,
				0,1);
	Os__saved_set((unsigned char *)&DataSave0.ConstantParamData.ucFunctStep,
		0,sizeof(unsigned char));
	Os__saved_set((unsigned char *)&DataSave0.ChangeParamData.uiTotalNumber,0,
	sizeof(unsigned int));
	Os__saved_set((unsigned char *)&DataSave0.ChangeParamData.uiEMVICLogNum,0,
	sizeof(unsigned short));

	ucReFile = XDATA_Write_Vaild_File(DataSaveConstant);
	if (!ucReFile)
	ucReFile = XDATA_Write_Vaild_File(DataSaveChange);
	if (!ucReFile)
	ucReFile = XDATA_Write_Vaild_File(DataSaveTransInfo);
	if (ucReFile)
	{
		MSG_DisplayErrMsg(ucReFile);
	}
	if (!ucReFile)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char*)"清除成功");
	}
	else
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char*)"清除失败");
	}
	MSG_WaitKey(3);

	return(SUCCESS);
}
unsigned char CFG_ConstantParamVersion(void)
{
	unsigned char DispBuf[30];
	unsigned char len;


	memset(DispBuf,0,sizeof(DispBuf));
	memcpy(DispBuf , "硬件版本:", 9);
	memcpy(&DispBuf[9],HARDWARE_VER,sizeof(HARDWARE_VER));
#ifdef GUI_PROJECT
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=8;	
	memcpy(ProUiFace.ProToUi.aucLine1,DispBuf, strlen((char *)DispBuf) );
#else
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,DispBuf);
#endif

	memset(DispBuf,0,sizeof(DispBuf));
	memcpy(DispBuf , "软件版本:", 9);
	memcpy(&DispBuf[9],SOFTWARE_VER,sizeof(SOFTWARE_VER));
#ifdef GUI_PROJECT
	memcpy(ProUiFace.ProToUi.aucLine2,DispBuf, strlen((char *)DispBuf) );
#else
	//Os__GB2312_display(1,0,DispBuf);
#endif
	

	memset(DispBuf,0,sizeof(DispBuf));
#ifdef GUI_PROJECT
	memcpy(ProUiFace.ProToUi.aucLine3,"最后修改时间:", 13);
#else
	//Os__GB2312_display(2,0,(unsigned char *)"最后修改时间:");
#endif

	memcpy( DispBuf,__DATE__,sizeof(__DATE__) );
#ifdef GUI_PROJECT
	memcpy(ProUiFace.ProToUi.aucLine4,DispBuf, strlen((char*)DispBuf));
#else
	Os__display(3,0,DispBuf);
	MSG_WaitKey(30);
#endif

	memset(DispBuf,0,sizeof(DispBuf));
	EMVDATA_KernelVer(DispBuf);
#ifdef GUI_PROJECT
	memcpy(ProUiFace.ProToUi.aucLine5,"EMV/非接内核版本:", strlen((char*)"EMV/非接内核版本:"));
	memcpy(ProUiFace.ProToUi.aucLine6,DispBuf, strlen((char*)DispBuf));
#else
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(UCHAR *)"EMV/非接内核版本:");
	//Os__GB2312_display(1,0,(UCHAR *)DispBuf);
#endif


	memset(DispBuf,0,sizeof(DispBuf));
	QDATA_KernelVer(DispBuf);
#ifdef GUI_PROJECT
	memcpy(ProUiFace.ProToUi.aucLine7,DispBuf, strlen((char*)DispBuf));
	memcpy(ProUiFace.ProToUi.aucLine8,"应用号:104(68)", strlen((char*)"应用号:104(68)"));
// sleep(1);
    sem_post(&binSem1);
    sem_wait(&binSem2);
#else
	//Os__GB2312_display(2,0,(UCHAR *)DispBuf);
	//Os__GB2312_display(3,0,(unsigned char *)"应用号:104(68)");
	MSG_WaitKey(30);
#endif
	
#ifdef GUI_PROJECT
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=8;	
	memcpy(ProUiFace.ProToUi.aucLine1,HARDWARE_NAME, strlen((char *)HARDWARE_NAME) );
	memcpy(ProUiFace.ProToUi.aucLine2,"软件项目编号:", 13 );	
#else
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(UCHAR *)HARDWARE_NAME);
	//Os__GB2312_display(1,0,(unsigned char *)"软件项目编号:");
#endif

	memset(DispBuf,0,sizeof(DispBuf));
	memcpy(DispBuf,CURRENT_PRJ,sizeof(CURRENT_PRJ));
	len = strlen((char*)CURRENT_PRJ);

#ifdef GUI_PROJECT
	memcpy(ProUiFace.ProToUi.aucLine3,DispBuf, strlen((char *)DispBuf) );
// sleep(1);
    sem_post(&binSem1);
    sem_wait(&binSem2);
#else
	if(len<16)
	{
		//Os__GB2312_display(2,0,DispBuf);
	}else
	{
		//Os__GB2312_display(2,0,DispBuf);
		//Os__GB2312_display(3,0,&DispBuf[16]);
	}
	MSG_WaitKey(30);	
#endif

	return(SUCCESS);
}
void CFG_SetTestGroup(void)
{
	//Os__clr_display(255);
	//Os__GB2312_display(0,0," 请选择卡组织");
	//Os__GB2312_display(1,0,"1.CUP   2.VISA");
	//Os__GB2312_display(1,0,"3.JCB   4.MAST");
	//Os__GB2312_display(1,0,"5.NORMAL");

	ucTestGroup=Os__xget_key();

}


unsigned char CFG_CheckCashierData(void)
{
	unsigned char ucResult=SUCCESS;
	int iFileSize;
	int iFileHandel;
	int iFileErrNo;

	util_Printf("Checking [Cashier] File..\n");

	if((iFileSize=OSAPP_FileSize((char *)FileInfo[DataSaveCashier].aucFileName))<0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveCashier].aucFileName,O_CREATE);
		if(iFileHandel>=0)
		{
			OSMEM_Memset(&xDATA_Cashier,0,xDATA_CashierSize);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo)
				iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_Cashier,xDATA_CashierSize);
			OSAPP_FileClose(iFileHandel);
		}
	}else
	{
		util_Printf("CashierSize = %d\n",xDATA_CashierSize);
		if(iFileSize>=xDATA_CashierSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveCashier].aucFileName,O_READ);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo)
				iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Cashier,xDATA_CashierSize);
			OSAPP_FileClose(iFileHandel);
		}else if(iFileSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveCashier].aucFileName,O_READ);
			OSAPP_FileTrunc(iFileHandel,0);
		}
	}
	util_Printf("文件大小:%d \nCashier结构大小:%d\n",iFileSize,xDATA_CashierSize);
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}


unsigned char CFG_CheckTransInfoData()
{
	int iFileSize;
	int iFileHandel;
	int iFileErrNo;
       unsigned char ucResult=SUCCESS;

	util_Printf("Checking [TransInformation] File...\n");

	if((iFileSize=OSAPP_FileSize((char *)FileInfo[DataSaveTransInfo].aucFileName))<0)
	{
		util_Printf("TransInfoN\n");
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveTransInfo].aucFileName,O_CREATE);
		if(iFileHandel>=0)
		{
			OSMEM_Memset(&xDATA_TransInfo,0,xDATA_TransInfoSize);
			OSAPP_FileClose(iFileHandel);
		}
	}else
	{	util_Printf("TransInfo1 = %d\n",xDATA_TransInfoSize);
		if(iFileSize==xDATA_TransInfoSize)
		{
			util_Printf("TransInfo2\n");
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveTransInfo].aucFileName,O_READ);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo)
				iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_TransInfo,xDATA_TransInfoSize);
			OSAPP_FileClose(iFileHandel);
		}else if(iFileSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveTransInfo].aucFileName,O_READ);
			OSAPP_FileTrunc(iFileHandel,0);
		}
	}
	util_Printf("文件大小:%d \n TransInfo 结构大小:%d\n",iFileSize,xDATA_CashierSize);
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}

unsigned char CFG_CheckTrans8583Data()
{
	int iFileSize;
	int iFileHandel;
	int iFileErrNo;
	unsigned char ucResult=SUCCESS;

	util_Printf("Checking [ISO8583] File...\n");
	if((iFileSize=OSAPP_FileSize((char *)FileInfo[DataSaveTrans8583].aucFileName))<0)
	{
		util_Printf("Trans8583N\n");
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveTrans8583].aucFileName,O_CREATE);
		if(iFileHandel>=0)
		{
			OSMEM_Memset(&xDATA_TransInfo,0,xDATA_Trans8583Size);
			OSAPP_FileClose(iFileHandel);
		}
	}else
	{	util_Printf("ISO8583结构大小 = %d\n",xDATA_Trans8583Size);
		if(iFileSize==xDATA_Trans8583Size)
		{
			util_Printf("TransInfo2\n");
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveTrans8583].aucFileName,O_READ);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo)
				iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Trans8583,xDATA_Trans8583Size);
			OSAPP_FileClose(iFileHandel);
		}else if(iFileSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveTrans8583].aucFileName,O_READ);
			OSAPP_FileTrunc(iFileHandel,0);
		}
	}
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}

unsigned char CFG_CheckSaveTrans(void)
{
	long iFileSize;
	int iFileHandel;
	unsigned char ucResult=SUCCESS;

	util_Printf("Checking [SaveTrans] File...\n");
	if((iFileSize=OSAPP_FileSize((char *)FileInfo[DataSaveSaveTrans].aucFileName))<0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveSaveTrans].aucFileName,O_CREATE);
		if(iFileHandel>=0)
		{
			OSAPP_FileClose(iFileHandel);
		}
	}
	else util_Printf("()---1-----iFileSize = %ld \n",iFileSize);

	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}

unsigned char CFG_CheckTempFile(void)
{
	int iFileSize;
	int iFileHandel;
	unsigned char ucResult=SUCCESS;

       util_Printf("(CFG_CheckTempFile)---1--\n");
	if((iFileSize=OSAPP_FileSize((char *)FileInfo[TempData].aucFileName))<0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[TempData].aucFileName,O_CREATE);
		if(iFileHandel>=0)
		{
			OSAPP_FileClose(iFileHandel);
		}
	}
	else util_Printf("(CFG_CheckTempFile)---1-----iFileSize = %ld \n",iFileSize);
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}
unsigned char CFG_CHECKFILE_Collect(void)
{
		int iFileSize;
		int iFileHandel;

		if((iFileSize=OSAPP_FileSize((char *)FileInfo[SaveCollectData].aucFileName))<0)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[SaveCollectData].aucFileName,O_CREATE);
			if(iFileHandel>=0)
			{
				OSAPP_FileClose(iFileHandel);
			}
		}
		else
			util_Printf("***0*****第[%d]号文件大小= %d*************\n",SaveCollectData,iFileSize);
		util_Printf("****1****第[%d]号文件大小= %d*************\n",SaveCollectData,iFileSize);
		return(OSAPP_FileGetLastError());
}
unsigned char CFG_Check_File_All(void)
{
	unsigned char i,ucResult;

	ucResult = CFG_CheckConstantParamData();

	if(!ucResult)
		ucResult = CFG_CheckChangeParamData();
	else util_Printf("CFG_CheckConstantParamData()---1-----ucResult = %02x \n",ucResult);

	if(!ucResult)
		ucResult = CFG_CheckCashierData();
	else util_Printf("CFG_CheckChangeParamData()-----2---ucResult = %02x \n",ucResult);

	if(!ucResult)
		ucResult = CFG_CheckTransInfoData();
	else util_Printf("CFG_CheckCashierData()----3----ucResult = %02x \n",ucResult);

	if(!ucResult)
		ucResult = CFG_CheckTrans8583Data();
	else util_Printf("CFG_CheckTransInfoData()----4----ucResult = %02x \n",ucResult);

	if(!ucResult)
		ucResult = CFG_CheckSaveTrans();
	else util_Printf("CFG_CheckTrans8583Data----5----ucResult = %02x \n",ucResult);

   	if (!ucResult)
            ucResult = CFG_CheckTempFile();
       else util_Printf("CFG_CheckSaveTrans----6----ucResult = %02x \n",ucResult);

	if(!ucResult)
		ucResult = EMV_Check_file();
	else
		util_Printf("CFG_CheckTempFile()-----7---ucResult = %02x \n",ucResult);

	if(!ucResult)
		ucResult=CFG_CHECKFILE_Collect();
	else
		util_Printf("EMV_Check_file()--------ucResult = %02x \n",ucResult);

	util_Printf("数据采集文件_Collect()--------ucResult = %02x \n",ucResult);

	for(i=0;i<DataSaveMaxIndex;i++)
	{
		util_Printf("=====[File %d Size =%d]==========\n",i,OSAPP_FileSize((char *)FileInfo[i].aucFileName));
	}
    if (ucResult != SUCCESS)
    {
        MSG_DisplayErrMsg(ucResult);
    }
	return(SUCCESS);
}


unsigned char CFG_CheckConstantParamData(void)
{
	int iFileSize;
	int iFileHandel;
	int iFileErrNo;
	unsigned char ucResult=SUCCESS;

	util_Printf("\nChecking [constant] File..\n");

	if((iFileSize=OSAPP_FileSize((char *)FileInfo[DataSaveConstant].aucFileName))<0)
	{
		util_Printf("ConFGN\n");
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveConstant].aucFileName,O_CREATE);
		if(iFileHandel>=0)
		{
			OSMEM_Memset(&xDATA_Constant,0,xDATA_ConstantSize);
			OSAPP_FileClose(iFileHandel);
		}
	}else
	{
		util_Printf("iFileSize:%d \nConstant=%d\n",iFileSize,xDATA_ConstantSize);
		if(iFileSize==xDATA_ConstantSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveConstant].aucFileName,O_READ);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo)
				iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Constant,xDATA_ConstantSize);
			OSAPP_FileClose(iFileHandel);
		}else if(iFileSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveConstant].aucFileName,O_READ);
			OSAPP_FileTrunc(iFileHandel,0);
		}
	}
	util_Printf("文件大小:%d \nConstant结构大小:%d\n",iFileSize,xDATA_ConstantSize);

	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}


unsigned char CFG_CheckChangeParamData(void)
{
	unsigned char ucResult=SUCCESS;
	int iFileSize;
	int iFileHandel;
	int iFileErrNo;

	util_Printf("Checking [Change] File..\n");

	if((iFileSize=OSAPP_FileSize((char *)FileInfo[DataSaveChange].aucFileName))<0)
	{
		iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveChange].aucFileName,O_CREATE);
		if(iFileHandel>=0)
		{
			OSMEM_Memset(&xDATA_Change,0,xDATA_ChangeSize);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo)
				iFileErrNo = OSAPP_FileWrite(iFileHandel,(unsigned char *)&xDATA_Change,xDATA_ChangeSize);
			OSAPP_FileClose(iFileHandel);
		}
	}else
	{
		util_Printf("DataChange1=%d\n",xDATA_ChangeSize);
		if(iFileSize==xDATA_ChangeSize)
		{
			util_Printf("DataChange2\n");
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveChange].aucFileName,O_READ);
			iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
			if(!iFileErrNo)
				iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)&xDATA_Change,xDATA_ChangeSize);
			OSAPP_FileClose(iFileHandel);
		}else if(iFileSize)
		{
			iFileHandel = OSAPP_OpenFile((char *)FileInfo[DataSaveChange].aucFileName,O_READ);
			OSAPP_FileTrunc(iFileHandel,0);
		}
	}
	util_Printf("文件大小:%d \nChange结构大小:%d\n",iFileSize,xDATA_ChangeSize);
	ucResult = OSAPP_FileGetLastError();
        if (ucResult)
        {
            G_RUNDATA_ucErrorFileCode = ucResult;
            ucResult = ERR_WRITEFILE;
        }
        return(ucResult);
}


unsigned char CFG_TermTypeConfig(void)
{
	unsigned char ucResult;
	unsigned char aucTransTAB[3];
	unsigned char ucKey,ucResetKey;

	ucResult = SUCCESS;

	while(1)
	{
        	//Os__clr_display(255);
        	//Os__GB2312_display(0,0,(unsigned char *)"请选择终端类别：");
        	//Os__GB2312_display(1,0,(unsigned char *)"1：百货2：宾馆");
        	//Os__GB2312_display(2,0,(unsigned char *)"3：餐饮");
        	//Os__GB2312_display(3,0,(unsigned char *)"按[确认]键跳过");
        	memset(aucTransTAB,0,sizeof(aucTransTAB));
        	aucTransTAB[0] = DataSave0.ConstantParamData.ucTerminalType;
        	Os__display(2,13,aucTransTAB);
		ucKey = Os__xget_key();
		if(ucKey == '1'||ucKey == '2'||ucKey == '3')
		{
			if (aucTransTAB[0] != 0x31 && aucTransTAB[0] != 0x32 && aucTransTAB[0] != 0x33)
				break;
			else
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(unsigned char *)"已初始化终端类别");
				//Os__GB2312_display(1,0,(unsigned char *)"是否重置?");
				//Os__GB2312_display(3,0,(unsigned char *)"[确定]    [取消]");
				ucResetKey = Os__xget_key();
				if (ucResetKey == KEY_ENTER)
					break;
				else if(ucResetKey == KEY_CLEAR)
					return(SUCCESS);
			}
		}
		else if (ucKey == KEY_ENTER)
                {
                    if (aucTransTAB[0] != 0x31 && aucTransTAB[0] != 0x32 && aucTransTAB[0] != 0x33)
                    {
                        //Os__clr_display(255);
                        //Os__GB2312_display(1,0,(unsigned char *)"终端类别尚未设置");
                        //Os__GB2312_display(2,1,(unsigned char *)" 请务必选择");
                        UTIL_WaitGetKey(10);
                    }
                    else
                        break;
                }
		else if (ucKey == KEY_CLEAR)
			break;
	}

	if(ucKey == '1')
	{
		DataSave0.ConstantParamData.ucTerminalType = TYPE_SHOP;
		XDATA_Write_Vaild_File(DataSaveConstant);
		CFG_DefaultParam();
	}else
	if(ucKey == '2')
	{
		DataSave0.ConstantParamData.ucTerminalType = TYPE_HOTEL;
		XDATA_Write_Vaild_File(DataSaveConstant);
		CFG_DefaultParam();
	}else
	if(ucKey == '3')
	{
		DataSave0.ConstantParamData.ucTerminalType = RESTAURANT;
		XDATA_Write_Vaild_File(DataSaveConstant);
		CFG_DefaultParam();
	}else
	if(ucKey == KEY_ENTER)
		ucResult = SUCCESS;
	else
	if(ucKey == KEY_CLEAR)
		ucResult = ERR_CANCEL;

	return ucResult;
}
/*-----------------------------------------------------
功能说明:
	在屏幕上面显示多行汉字；
参数说明:
	ptSrc:		需要显示的数据
	uiDataLen:	显示数据的长度
	ucLine:		从第几行开始显示
-------------------------------------------------------*/
unsigned char CFG_DisplayGB2312Info( uchar *ptSrc,uint uiDataLen,uchar ucLine)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned int  uiI;
	unsigned int  uiPrintLen,uiLen,uiNum;
	unsigned char ucASCNum;
	unsigned char ucRow;

	if(uiDataLen == 0)
	{
		return 0;
	}
	for(uiPrintLen=0;uiPrintLen<uiDataLen;uiPrintLen++)
	{
		if( ptSrc[uiPrintLen] == 0 )
		break;
	}
	util_Printf("uiPrintLen = %d\n",uiPrintLen);

	uiLen = 0;
	ucRow = ucLine;
	do
	{
		for(uiNum = 0;((uiLen+uiNum) < uiPrintLen)&&(uiNum < 16);uiNum++);

		if(uiNum >= 16)
		{
			uiI = 0;
			ucASCNum = 0;
			while(uiI < uiNum)
			{
				if(ptSrc[uiLen + uiI] < 0x80)
				{
					ucASCNum++;
					uiI++;
				}
				else
				{
					uiI+=2;
				}
			}
			if((ucASCNum%2) != 0)
			{
				uiNum--;
			}
		}
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,&ptSrc[uiLen],uiNum);
		//Os__clr_display(ucRow);
		//Os__GB2312_display(ucRow, 0, aucPrintBuf);
		uiLen += uiNum;
		ucRow++;

		if(ucRow == 0x04)
			break;
	}while(uiLen < uiPrintLen);

	return 0;
}
unsigned char CFG_SetMarchName(void)
{
	unsigned char ucResult;
	unsigned char aucBuf[PARAM_MERCHANTNAMELEN+1];
	unsigned char aucDispBuf[41];
	unsigned char ucInputFlag;
	unsigned char ucKey;

	memset(aucBuf,0,sizeof(aucBuf));
   	memcpy(aucBuf,DataSave0.ConstantParamData.aucMerchantName,PARAM_MERCHANTNAMELEN);

	if(!strlen((char *)aucBuf))
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"请输入商户名：");
		ucInputFlag = PY_INPUT_CHINESE
						|PY_INPUT_LOWER_CASE
						|PY_INPUT_UPPER_CASE
						|PY_INPUT_NUMBER
						|PY_INPUT_QUWEI;
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = PY_Main(aucBuf ,1,0,40, ucInputFlag ,300);
		if(!ucResult)
		{
			memcpy(DataSave0.ConstantParamData.aucMerchantName,
					aucBuf,PARAM_MERCHANTNAMELEN);
			XDATA_Write_Vaild_File(DataSaveConstant);
		}
	}else
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"商户名：");
		memset(aucDispBuf,0,sizeof(aucDispBuf));
		memcpy(aucDispBuf,&aucBuf[0],PARAM_MERCHANTNAMELEN);
		CFG_DisplayGB2312Info(aucDispBuf,40,1);
#if 0
		memset(aucDispBuf_1,0,sizeof(aucDispBuf_1));
		memset(aucDispBuf_2,0,sizeof(aucDispBuf_2));
		memset(aucDispBuf_3,0,sizeof(aucDispBuf_3));
		memcpy( aucDispBuf_1,&aucBuf[0], 16);
		memcpy( aucDispBuf_2,&aucBuf[16] , 16);
		memcpy( aucDispBuf_3,&aucBuf[32] , 8);
		if(strlen((char *)aucDispBuf_1))
			CFG_DisplayGB2312Info(aucDispBuf_1,16,1);
			////Os__GB2312_display(1,0,aucDispBuf_1);
		if(strlen((char *)aucDispBuf_2))
			CFG_DisplayGB2312Info(aucDispBuf_2,16,2);
			////Os__GB2312_display(2,0,aucDispBuf_2);
		if(strlen((char *)aucDispBuf_3))
			CFG_DisplayGB2312Info(aucDispBuf_3,16,3);
			////Os__GB2312_display(3,0,aucDispBuf_3);
#endif
		Os__xget_key();
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(unsigned char *)"是否更改商户名？");
		//Os__GB2312_display(2,0,(unsigned char *)"按确认键进行更改");

		ucKey = Os__xget_key();

		if(ucKey == KEY_ENTER)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char *)"请输入新商户名：");
			ucInputFlag = PY_INPUT_CHINESE
						|PY_INPUT_LOWER_CASE
						|PY_INPUT_UPPER_CASE
						|PY_INPUT_NUMBER
						|PY_INPUT_QUWEI;
			memset(aucBuf,0,sizeof(aucBuf));
			ucResult = PY_Main(aucBuf ,1,0,40, ucInputFlag ,300);

			if(!ucResult)
			{
				memcpy(DataSave0.ConstantParamData.aucMerchantName,
						aucBuf,PARAM_MERCHANTNAMELEN);
				XDATA_Write_Vaild_File(DataSaveConstant);
			}
		}
	}
	return ucResult;
}
unsigned char CFG_SwitchTransType(void)
{
	unsigned char ucChar;
	unsigned char ucDispCh;
	unsigned char ucKey,aucBuf[8];

#if PS100
		ucResult = CASH_InputAndChackManagerPass(1);
		if (ucResult && ucResult != ERR_CANCEL)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,1,(uchar *)"主管密码错");
			return(SUCCESS);
		}
		if (!ucResult)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"切换交易模式");
				//Os__GB2312_display(1,0,(uchar *)"1.一体机");
				//Os__GB2312_display(2,0,(uchar *)"2.非一体机");

				memset(aucBuf,0,sizeof(aucBuf));
				ucDispCh = 0x30;
				util_Printf("模式:%02x\n",DataSave0.ConstantParamData.ucCashFlag);
				memcpy(aucBuf,&DataSave0.ConstantParamData.ucCashFlag,sizeof(unsigned char));
				//Os__GB2312_display(3,0,(unsigned char *)"当前为:");
				//Os__GB2312_display(3,4,aucBuf);

				while(1)
				{
					ucKey = Os__xget_key();
					switch(ucKey)
					{
						case '1':
						case '2':
							ucChar = ucKey;
							Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucCashFlag,
								sizeof(unsigned char));
							XDATA_Write_Vaild_File(DataSaveConstant);
							break;
						case KEY_CLEAR:
							return(ERR_CANCEL);
						case KEY_ENTER:
							ucDispCh = 0x31;
							break;
						default :
							continue;
					}
					aucBuf[0] = DataSave0.ConstantParamData.ucCashFlag;
					//Os__GB2312_display(3,4,aucBuf);
					util_Printf("交易模式 = %02x\n",DataSave0.ConstantParamData.ucCashFlag);
					if (ucDispCh == 0x31)
						break;
				}
			}
#endif
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"是否支持主界面");
	//Os__GB2312_display(1,0,(uchar *)"刷卡或插卡");
	//Os__GB2312_display(2,0,(uchar *)"1.支持  0.不支持");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = 0x30;
	memcpy(aucBuf,&DataSave0.ConstantParamData.ucSwiptEvent,1);
	//Os__GB2312_display(3,0,(unsigned char *)"当前为:");
	//Os__GB2312_display(3,4,aucBuf);
	while(1)
	{
		ucKey=Os__xget_key();
		switch(ucKey)
		{
			case '0':
			case '1':
				ucChar = ucKey;
				Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucSwiptEvent,
					sizeof(unsigned char));
				XDATA_Write_Vaild_File(DataSaveConstant);
				break;
			case KEY_CLEAR:
				return(ERR_CANCEL);
			case KEY_ENTER:
				ucDispCh = 0x31;
				break;
			default :
				continue;
		}
		aucBuf[0] = DataSave0.ConstantParamData.ucSwiptEvent;
		//Os__GB2312_display(3,4,aucBuf);
		if(ucDispCh == 0x31)
			break;
	}
	return(SUCCESS);
}

unsigned char CFG_WriteDateTimetoPOS(unsigned char * ucInBuf)
{
	unsigned char Date_Time[17];
	unsigned char tmp_date[17];
	unsigned char ucChar;

	memset(Date_Time,0,sizeof(Date_Time));
	memset(tmp_date,0,sizeof(tmp_date));

	memcpy(tmp_date , ucInBuf ,10);

	Date_Time[0] = tmp_date[4];
	Date_Time[1] = tmp_date[5];
	Date_Time[2] = tmp_date[2];
	Date_Time[3] = tmp_date[3];
	Date_Time[4] = tmp_date[0];
	Date_Time[5] = tmp_date[1];

#ifdef TEST
	util_Printf("Date_Time=%s\n",Date_Time);
#endif

	ucChar=Os__write_date(Date_Time);

#ifdef TEST
	util_Printf("ucChar=%02x\n",ucChar);
#endif

	memset(Date_Time,0,sizeof(Date_Time));

	memcpy(Date_Time,&tmp_date[6],4);

#ifdef TEST
	util_Printf("Date_Time1=%s\n",Date_Time);
#endif
	ucChar=Os__write_time(Date_Time);
#ifdef TEST
	util_Printf("ucChar1=%02x\n",ucChar);
#endif
	return(OK);

}

void CFG_DispJustCommMode(unsigned char ucLin)
{
	//Os__clr_display(ucLin);
    //if(DataSave0.ConstantParamData.ucCommMode == 1)
		//Os__GB2312_display(ucLin,0,(unsigned char*)"当前为串口通讯");
    //else
    //if(DataSave0.ConstantParamData.ucCommMode == 2)
		//Os__GB2312_display(ucLin,0,(unsigned char*)"当前为拨号通讯");
    //else
    //if(DataSave0.ConstantParamData.ucCommMode == 3)
		//Os__GB2312_display(ucLin,0,(unsigned char*)"当前为GPRS通讯");
    //else
    //if(DataSave0.ConstantParamData.ucCommMode == 4)
		//Os__GB2312_display(ucLin,0,(unsigned char*)"当前为CDMA通讯");
    //else
    //if(DataSave0.ConstantParamData.ucCommMode == 5)
		//Os__GB2312_display(ucLin,0,(unsigned char*)"当前为TCP/IP通讯");
    //else
		//Os__GB2312_display(ucLin,0,(unsigned char*)"通讯尚未设置");
}

unsigned char CFG_SwitchComm(void)
{
	unsigned char ucResult;
	unsigned char aucBuf[50];

	ucResult = SUCCESS;
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char*)"1.拨号方式");
	//Os__GB2312_display(1,0,(unsigned char*)"2.TCP/IP方式");
	CFG_DispJustCommMode(2);
	memset(aucBuf,0,sizeof(aucBuf));
	if (UTIL_Input(3,true,0,1,'N',aucBuf,(uchar *)"12") != KEY_ENTER )
	{
		ucResult = ERR_CANCEL;
	}else
	{
		if(aucBuf[0]  == '1')
			aucBuf[0] = 2;
		else
		if(aucBuf[0]  == '2')
			aucBuf[0] = 5;
		else
		if(aucBuf[0]!='1'||aucBuf[0]!='2')
		{
			//Os__clr_display(255);
			//Os__GB2312_display(2,0,(uchar *)" 您尚未切换通讯");
			MSG_WaitKey(3);
			return(SUCCESS);
		}
		CFG_DispJustCommMode(2);
		Os__saved_copy(aucBuf,
			&DataSave0.ConstantParamData.ucCommMode,1);
		ucResult=XDATA_Write_Vaild_File(DataSaveConstant);
		if(ucResult==SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"----通讯方式----");
			//Os__GB2312_display(2,0,(uchar *)"    切换成功!");
			MSG_WaitKey(3);
		}else if(ucResult!=SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"----通讯方式----");
			//Os__GB2312_display(2,0,(uchar *)"    切换失败");
			MSG_WaitKey(3);
		}
	}
	return ucResult;
}

unsigned char CFG_ChangeReaderParamData_HongBao(void)
{
	unsigned char ucResult = SUCCESS;
	BASICCONFIG pBasicConfig;
	unsigned char ucCOMFlag[2],KEY;

	ucResult = File_ReadBasicConfigFile(&pBasicConfig);
	util_Printf("\n File_ReadBasicConfigFile ucResult:[%02x]",ucResult);
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"请选择POS串口： ");
	//Os__GB2312_display(1,0,(unsigned char *)"1:串口1  2:串口2");

	memset(ucCOMFlag,0,sizeof(ucCOMFlag));
	ucCOMFlag[0] = pBasicConfig.ulCOMType+0x30;
	if( UTIL_Input(3,true,0,1,'N',ucCOMFlag,(uchar *)"12") != KEY_ENTER )
		return(ERR_CANCEL);

	KEY = ucCOMFlag[0];
	if(KEY == COM1)
		pBasicConfig.ulCOMType = ucCOMFlag[0] - 0x30;
	else if(KEY == COM2)
		pBasicConfig.ulCOMType = ucCOMFlag[0] - 0x30;
	else
		return(ERR_CANCEL);

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"监控信息是否开启");
	//Os__GB2312_display(1,0,(unsigned char *)"1:显示  0:不显示");

	memset(ucCOMFlag,0,sizeof(ucCOMFlag));
	ucCOMFlag[0] = pBasicConfig.ucDisplayDebugDataFlag+0x30;
	if( UTIL_Input(3,true,0,1,'N',ucCOMFlag,(uchar *)"01") != KEY_ENTER )
		return(ERR_CANCEL);
	pBasicConfig.ucDisplayDebugDataFlag = ucCOMFlag[0] - 0x30;

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"是否显示错误信息");
	//Os__GB2312_display(1,0,(unsigned char *)"1:显示  0:不显示");

	memset(ucCOMFlag,0,sizeof(ucCOMFlag));
	ucCOMFlag[0] = pBasicConfig.ucDisplayErrMsgData+0x30;
	if( UTIL_Input(3,true,0,1,'N',ucCOMFlag,(uchar *)"01") != KEY_ENTER )
		return(ERR_CANCEL);
	pBasicConfig.ucDisplayErrMsgData = ucCOMFlag[0] - 0x30;

	if(!ucResult)INTERFACE_ConfigBasicData(&pBasicConfig);
	util_Printf("\n INTERFACE_ConfigBasicData ucResult:[%02x]",ucResult);
	if(!ucResult) ucResult=File_WriteBasicConfigFile(&pBasicConfig);
	util_Printf("\n File_WriteBasicConfigFile ucResult:[%02x]",ucResult);
	INTERFACE_RefurbishReaderLCD(1);
	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(unsigned char *)"  配置参数完成");
	UTIL_GetKey(10);
	return(ucResult);
}
unsigned char CFG_ChangeReaderParamData_Sand(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[16];

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"读卡器串口设置 ");
	//Os__GB2312_display(1,0,(unsigned char *)"1:串口1  2:串口2");
	//Os__GB2312_display(2,0,(unsigned char *)"3:串口3  ");
	memset(aucBuf,0,sizeof(aucBuf));
	aucBuf[0] =ConstParam.ucReaderPort +0x30;
	if( UTIL_Input(3,true,1,1,'N',aucBuf,(uchar *)"123") != KEY_ENTER )
		return(ERR_CANCEL);
	else
	{
		ConstParam.ucReaderPort  =aucBuf[0]-0x30;
		UTIL_WriteConstParamFile(&ConstParam);
	}
	return ucResult;
}

unsigned char CFG_SetMarchName_Cust_Single(unsigned char ucIndex)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucKey = 0x00,ucInputFlag=0;
	unsigned char aucDisp[20],ucShowIndex=0,aucBuf[40];
	unsigned char aucDispBuf_1[20],aucDispBuf_2[20];
	
	memset(aucDisp,0x00,sizeof(aucDisp));
	ucShowIndex = ucIndex+'1';
	memcpy(aucDisp,(unsigned char *)"序号:",5);
	aucDisp[5]=ucShowIndex;
	memcpy(&aucDisp[6],(unsigned char *)"请输商户名",10);
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,aucDisp);

	memset(aucDispBuf_1,0,sizeof(aucDispBuf_1));
	memset(aucDispBuf_2,0,sizeof(aucDispBuf_2));
	memset(aucBuf,0x00,sizeof(aucBuf));
	memcpy(aucBuf,DataSave0.ConstantParamData.aucMerchantName_cust[ucIndex],20);
	memcpy( aucDispBuf_1,&aucBuf[0],16);
	memcpy( aucDispBuf_2,&aucBuf[16] , 4);
	UTIL_Tiaoshi(aucDispBuf_1,16);
	if(strlen((char *)aucDispBuf_1))
		//Os__GB2312_display(2,0,aucDispBuf_1);
	if(strlen((char *)aucDispBuf_2))
		//Os__GB2312_display(3,0,aucDispBuf_2);	
	ucKey = Os__xget_key();
	if(ucKey == KEY_ENTER)
	{
		if(strlen((char *)aucDispBuf_2))
			//Os__clr_display(3);
		ucInputFlag = PY_INPUT_CHINESE
					|PY_INPUT_LOWER_CASE
					|PY_INPUT_UPPER_CASE
					|PY_INPUT_NUMBER
					|PY_INPUT_QUWEI;
		memset(aucBuf,0,sizeof(aucBuf));
		ucResult = PY_Main(aucBuf ,1,0,20, ucInputFlag ,300);
		if(!ucResult)
		{
			memcpy(DataSave0.ConstantParamData.aucMerchantName_cust[ucIndex],
					aucBuf, 20);
			ucResult = XDATA_Write_Vaild_File(DataSaveConstant);
			if(ucResult!=SUCCESS)
				ucResult = ERR_WRITEFILE;
		}
	}
	else
	{
		ucResult = ERR_CANCEL;
	}
		
	return ucResult;
}
unsigned char CFG_SetMarchName_Cust()
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucIndex = 0;
	
	for(ucIndex=0;(ucIndex<9);ucIndex++)
	{
		ucResult = CFG_SetMarchName_Cust_Single(ucIndex);
		
	}
		
	return ucResult;
}
