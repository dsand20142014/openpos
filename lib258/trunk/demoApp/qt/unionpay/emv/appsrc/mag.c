#include <include.h>
#include <global.h>
#include <xdata.h>


unsigned char Untouch_ReadCardNo(unsigned char *ucStatus, unsigned char *pucOutData,
												unsigned char *ucOutDataLength)
{
	unsigned char ucResult = SUCCESS;

	DrvIn.type      = 0x6F;
	DrvIn.length	= 0;

	ucResult = SendReceivePinpad();//00 80 0C 2C 86 04
#ifdef UnTest
	util_Printf("取卡:%02x\n",ucResult);
	{
		unsigned char ucI;
		for(ucI=0;ucI<6;ucI++)
			util_Printf("%02x ",DrvOut.xxdata[ucI]);
		util_Printf("\n");
	}
#endif
	if (ucResult == SUCCESS)
	{
		if ((DrvOut.xxdata[0] == 0x00) && DrvOut.xxdata[5] == 0x04)
		{
			*ucStatus = DrvOut.xxdata[0];
			*ucOutDataLength = 5;
			memcpy(pucOutData,&DrvOut.xxdata[1],5);
		}
		else
		{
//			if (G_NORMALTRANS_ucTransType != TRANS_VOIDCUPMOBILE)
//			{
//				//Os__clr_display(255);
//				//Os__GB2312_display(1,1,(uchar *)" 此卡不处理");
//				//Os__GB2312_display(2,0,(uchar *)" 非CUPMOBILE卡");
//				//Os__GB2312_display(3,2,(uchar *)" 请取卡");
//				UTIL_GetKey(2);
//			}
			return(ERR_CANCEL);
		}
    }
    else
    {
    	return (ERR_CANCEL);
    }
    return(SUCCESS);
}
unsigned char MAG_ReadUnCard(unsigned char *ucStatus, unsigned char *pucOutData,
												unsigned char *ucOutDataLength)
{
	unsigned char aucOutData[30],aucBuf[30];
 	unsigned char ucLen;
 	unsigned char ucResult = SUCCESS;
	int i;

	memset(aucOutData,0,sizeof(aucOutData));
	memset(aucBuf,0,sizeof(aucBuf));

 	ucResult = Untouch_ReadCardNo(ucStatus,aucOutData,&ucLen);
#ifdef UnTest
	util_Printf("\n");
	util_Printf("卡号+卡类型:\n");
	for(i=0;i<5;i++)
		util_Printf("aucOutData[%d]=%02x\n",i,aucOutData[i]);
	util_Printf("卡状态:%02x\n",ucStatus);
	util_Printf("卡长（卡类型）%02x:\n",ucLen);
	util_Printf("ucResult:%02x\n",ucResult);
	util_Printf("\n");
#endif
	if (ucResult == SUCCESS)
	{
		if (aucOutData[4] == 0x04)
		{
			memcpy(pucOutData,&aucOutData[0],4); //此处为纯卡号不包括卡类型
		}
		else
		{
			return(ERR_CANCEL);
		}
	}
	else
	{
		return (ucResult);
	}
	return(SUCCESS);
}

unsigned char MAG_ReadUnTouchCard(void)
{
	unsigned char ucResult ;
	unsigned char ucStatus,ucLen,aucOutData[300];

	/*读卡且取卡号(0x6F)*/
	ucResult = SUCCESS;
	memset(aucOutData,0,sizeof(aucOutData));
	ucResult = MAG_ReadUnCard(&ucStatus,aucOutData,&ucLen);
	return ucResult;
}
unsigned char MAG_ReadFile(void)
{
	unsigned char ucResult;
	unsigned char aucSeqNo[11];

	DrvIn.type = 0x89;
	DrvIn.length = 6;

	DrvIn.xxdata[0] = 0x05;
	DrvIn.xxdata[1] = 0x00;
	DrvIn.xxdata[2] = 0xB0;
	DrvIn.xxdata[3] = 0x82;
	DrvIn.xxdata[4] = 0x00;
	DrvIn.xxdata[5] = 0x10;

	ucResult = SendReceivePinpad();
	if (ucResult == SUCCESS)
	{
		if (DrvOut.xxdata[0] == 0x00)
		{
			int i;
			memset(aucSeqNo,0,sizeof(aucSeqNo));
			memcpy(aucSeqNo,&DrvOut.xxdata[2],10);

			memcpy(G_NORMALTRANS_aucUntouchSeq,aucSeqNo,10);
#ifdef UnTest
			util_Printf("卡片序列号:");
			for(i=0;i<10;i++)
			{
				util_Printf("%02x ",G_NORMALTRANS_aucUntouchSeq[i]);
			}
			util_Printf("\nEF02 File Data:");
			for (i=0;i<DrvOut.xxdata[1];i++)
			{
				util_Printf("%02x ",DrvOut.xxdata[i]);
			}
			util_Printf("\n");
#endif
		}
		else
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,0,(uchar *)" 读EF02文件失败");
			//Os__GB2312_display(2,2,(uchar *)" 请重试");
			UTIL_GetKey(2);
			return (ERR_CANCEL);
		}
	}else
	{
		return (ERR_CANCEL);
	}
	return (SUCCESS);
}
unsigned char MAG_ProcessUntouch(unsigned char *ucStatus, unsigned char *pucOutData,
												unsigned char *ucOutDataLength)
{
 	unsigned char ucResult = SUCCESS;
	unsigned char aucAID[16];

	DrvIn.type      = 0x89;
	DrvIn.length	= 21;

	DrvIn.xxdata[0]=0x14;
	DrvIn.xxdata[1]=0x00;
	DrvIn.xxdata[2]=0xA4;
	DrvIn.xxdata[3]=0x04;
	DrvIn.xxdata[4]=0x00;
	DrvIn.xxdata[5]=0x0F;

	memset(aucAID,0,sizeof(aucAID));
	memcpy(aucAID,"\xA0\x00\x00\x03\x33\x43\x55\x50\x2D\x4D\x4F\x42\x49\x4C\x45",15);
	memcpy(&DrvIn.xxdata[6],aucAID,15);

	{
		int i;
		util_Printf("Sand PIN Data:");
		for(i=0;i<21;i++)
			util_Printf("%02x ",DrvIn.xxdata[i]);
		util_Printf("\n");
	}

	ucResult = SendReceivePinpad();
	{
		unsigned char ucI;
		util_Printf("\n*******处理AID事件*******\n");
		util_Printf("ucResult:%02x\n",ucResult);
		util_Printf("xxdata[0]:%02x\n",DrvOut.xxdata[0]);
		util_Printf("xxdata[0]:%02x\n",DrvOut.xxdata[1]);
		if (DrvOut.xxdata[0] == 0x00)
		{
			for(ucI=0;ucI<DrvOut.xxdata[1];ucI++)
			{
				util_Printf("%02x ",DrvOut.xxdata[ucI]);
			}
		}
		util_Printf("\n");
	}

	if (ucResult == SUCCESS)
	{
		if (DrvOut.xxdata[0] == 0x00 && DrvOut.xxdata[1]>3)
		{
			*ucStatus = DrvOut.xxdata[0];
			*ucOutDataLength = DrvOut.xxdata[1];
			memcpy(pucOutData,&DrvOut.xxdata[2],15);
		}
		else
		{
//			//Os__clr_display(255);
//			//Os__GB2312_display(1,0,(uchar *)"读CMCPA应用失败");
//			//Os__GB2312_display(2,2,(uchar *)" 请重试");
//			UTIL_GetKey(2);
			return (ERR_NOTPHONECARD);
		}
	}
	else
	{
		return (ERR_CANCEL);
	}
	return(SUCCESS);
}
unsigned char MAG_MoveUntouchC(void)
{
	unsigned char ucResult;

	DrvIn.type      = 0x8A;
	DrvIn.length	= 0;
	ucResult = SendReceivePinpad();
	util_Printf("UR=%02x",ucResult);
	if (DrvOut.xxdata[0]!=0x00)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2,2,(uchar *)" 请取卡");
		UTIL_GetKey(2);
	}
	return(SUCCESS);
}

unsigned char MAG_Menu_KEYEIGHT(void)
{
	unsigned char ucKey;
	unsigned char ucResult;

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"1.回响测试");
	//Os__GB2312_display(1,0,(uchar *)"2.移动支付撤销");
	//Os__GB2312_display(2,0,(uchar *)"3.切换通讯模式");
	//Os__GB2312_display(3,0,(uchar *)"4.修改主管密码");

	while(1)
	{
		ucKey = UTIL_WaitGetKey(20);
		switch(ucKey)
		{
			case '1':
				ucResult = Trans_TestSingal();
				COMMS_FinComm();
				ucKey = KEY_CLEAR;
				break;
			case '2':
				ucResult = Trans_VoidCUP();
				ucKey = KEY_CLEAR;
				break;
			case '3':
				ucResult = CFG_SwitchComm();
				ucKey = KEY_CLEAR;
				break;
			case '4':
				ucResult = SYSCASH_ModiCashier();
				ucKey = KEY_CLEAR;
				break;
			case '5':
				if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
				ucResult = Trans_EnqBalance();
				COMMS_FinComm();
				ucKey = KEY_CLEAR;
				break;
			case KEY_F3:
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"1.回响测试");
				//Os__GB2312_display(1,0,(uchar *)"2.移动支付撤销");
				//Os__GB2312_display(2,0,(uchar *)"3.切换通讯模式");
				//Os__GB2312_display(3,0,(uchar *)"4.修改主管密码");
				break;
			case KEY_F4:
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"5.余额查询");
				break;
			default:
				break;
		}
		util_Printf("case '8':[%02x]\n",ucResult);
		if(ucKey == KEY_CLEAR)
		break;
	}
	return(ucResult);
}

unsigned char MAG_QuickUndo(void)
{
	unsigned char ucResult;
	unsigned char ucKey;
	unsigned short	uiIndex;

	if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"一体机模式下");
		//Os__GB2312_display(1,0,(uchar *)"该交易不支持!");
		MSG_WaitKey(3);
		return(ERR_CANCEL);
	}

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"   (消费撤销)");
	//Os__GB2312_display(1,0,(unsigned char *)"  撤销上笔交易");
	//Os__GB2312_display(2,0,(unsigned char *)"  请按[确定]键");
	//Os__GB2312_display(3,0,(unsigned char *)"[确定]    [取消]");

	while(1)
	{
		ucKey = UTIL_GetKey(15);

		ucResult = UTIL_Is_Trans_Empty();
		if(ucResult != SUCCESS)
		{
			return ucResult;
		}
		break;
	}
	if (ucKey == KEY_ENTER)
	{
		uiIndex = DataSave0.ChangeParamData.uiLastTransIndex;
		if( DataSave0.TransInfoData.auiTransIndex[uiIndex])
		{
			GetOldTransData(uiIndex);
			OSMEM_Memcpy((unsigned char *)&(DataSave1.SaveTrans),(unsigned char *)&NormalTransData,sizeof(NORMALTRANS));
		}
		if (G_NORMALTRANS_ucTransType != TRANS_PURCHASE)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,0,(uchar *)"原交易非消费交易");
			//Os__GB2312_display(2,0,(uchar *)"  按任意键退出");
			UTIL_GetKey(0);
			return(SUCCESS);
		}
		long_asc(G_RUNDATA_aucOriginalTraceNum,6,&G_NORMALTRANS_ulTraceNumber);
		UTIL_IncreaseTraceNumber();
		util_Printf("流水号 = %s [%d] \n",G_RUNDATA_aucOriginalTraceNum,G_NORMALTRANS_ulTraceNumber);
		if(!I_UnSale)
			ucResult = ERR_TRANSNOTSUP;
		else
		{
			G_NORMALTRANS_ucTransType = TRANS_VOIDPURCHASE;
			if(!ucResult) ucResult = Trans_Process(1);
		}
	}
	else if (ucKey == KEY_CLEAR)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2,0,(uchar *)"  按任意键退出");
		UTIL_GetKey(0);
		return(SUCCESS);
	}
	else
	{
		if(!I_UnSale)
			ucResult = ERR_TRANSNOTSUP;
		else
		{
			G_NORMALTRANS_ucTransType = TRANS_VOIDPURCHASE;
			if(!ucResult) ucResult = Trans_Process(0);
		}
	}
	util_Printf("ucResutl:%02x\n",ucResult);
	if (ucResult == SUCCESS
		||ucResult == ERR_CANCEL)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"  按任意键退出");
		COMMS_FinComm();
		UTIL_GetKey(0);
	}
	return(ucResult);
}

unsigned char MAG_COLLECT(void)
{
	unsigned char ucResult,ucPress;

	util_Printf("\n++++++++++需要上送数据:%02x\n",DataSave0.ChangeParamData.uiSendTotalNb);
	if (!DataSave0.ChangeParamData.uiSendTotalNb)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1, 1, (uchar *) "无可采集数据");
		return(UTIL_GetKey(5)?SUCCESS:SUCCESS);
	}
	//Os__clr_display(255);
	//Os__GB2312_display(0, 0, (uchar *) "是否上送采集数据");
	//Os__GB2312_display(2, 0, (uchar *) "上送-[确认]");
	//Os__GB2312_display(3, 0, (uchar *) "退出-[取消]");

	do
	{
		ucPress=UTIL_GetKey(30);
	}while(ucPress !=KEY_ENTER && ucPress != KEY_CLEAR && ucPress !=99);

	switch(ucPress)
	{
		case KEY_ENTER:
			G_NORMALTRANS_ucTransType=TRANS_SENDCOLLECTDATA;
			G_NORMALTRANS_euCardSpecies=0xFF;				//用于区分使用哪种ISO8583属性结构
			//Os__clr_display(255);
			//Os__GB2312_display(1, 1, (uchar *)" 数据处理中");
			COMMS_PreComm();
			ucResult=SERV_ProcessData();
			break;
		case 99:
		case KEY_CLEAR:
			return(SUCCESS);
	}
	return(ucResult);
}

unsigned char MAG_MOBILEMENU(void)
{
	unsigned char ucKey;
	unsigned char ucResult=SUCCESS;

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"1.预约消费");
	//Os__GB2312_display(1,0,(uchar *)"2.预约消费撤销");
	////Os__GB2312_display(2,0,(uchar *)"3.预约退货");
	while(1)
	{
		ucKey=UTIL_GetKey(15);
		switch(ucKey)
		{
			case '1':
				G_NORMALTRANS_ucTransType=TRANS_MOBILEAUTH;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_MANUALINPUT;
				ucResult = Trans_Process(0);
				break;
			case '2':
				G_NORMALTRANS_ucTransType=TRANS_VOIDMOBILEAUTH;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_MANUALINPUT;
				ucResult = Trans_Process(0);
				break;
#if 0
			case '3':
				G_NORMALTRANS_ucTransType=TRANS_MOBILEREFUND;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_MANUALINPUT;
				ucResult = Trans_Process(0);
				break;
#endif
			case 99:
			case KEY_CLEAR:
				break;
			default:
				continue;
		}
		break;
	}
	return(ucResult);
}

unsigned char MAG_CreditSale(void)
{
	unsigned char  ucResult;
	unsigned char ucKey;

	ucResult=SUCCESS;

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"1.分期付款消费");
	//Os__GB2312_display(1,0,(uchar *)"2.分期付款撤消");
	//Os__GB2312_display(2,0,(uchar *)"3.分期付款退货");

	while(1)
	{
		ucKey=UTIL_WaitGetKey(15);
		switch(ucKey)
		{
			case '1':
				G_NORMALTRANS_ucTransType=TRANS_CREDITSALE;
				ucResult=Trans_Process(0);
				break;
			case '2':
				G_NORMALTRANS_ucTransType=TRANS_CREDITSVOID;
				ucResult=Trans_Process(0);
				break;
			case '3':
				G_NORMALTRANS_ucTransType=TRANS_CREDITSREFUND;
				ucResult=Trans_Process(0);
				break;		
			case 99:
			case KEY_CLEAR:
				break;
			default:
				continue;
		}
		break;
	}
	if( (ucResult != SUCCESS)&&(ucResult != ERR_CANCEL) &&(ucResult != ERR_END))
	{
		MSG_DisplayErrMsg(ucResult);
		ucResult=ERR_CANCEL;
	}
	return(ucResult);
}

unsigned char MAG_SaleInstallment(void)
{
	unsigned char ucKey,ucResult;

	ucResult=SUCCESS;

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"1.普通消费");
	//Os__GB2312_display(1,0,(uchar *)"2.分期付款");
	while(1)
	{
		ucKey=UTIL_WaitGetKey(60);
		switch(ucKey)
		{
			case '1':
					if(!I_Sale)
						ucResult = ERR_TRANSNOTSUP;
					else
					{
						G_NORMALTRANS_ucTransType = TRANS_PURCHASE;
						if(!ucResult) ucResult = Trans_Process(0);
					}
				break;
			case '2':
				G_NORMALTRANS_ucTransType=TRANS_CREDITSALE;
				ucResult=Trans_Process(0);
				break;
			case 99:
			case KEY_CLEAR:
				break;
			default:
				continue;
		}
		break;
	}
	return(ucResult);
}

unsigned char Mag_Menu(unsigned char ucKey)
{
	unsigned char ucResult,ucFlag;
	unsigned char ucNextKey;

//	while(1)
	{
		util_Printf("\n++++++++++++Mag_Menu++++++++++\n");
		Os__light_on();

		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"1.消费  2.撤  销");
		//Os__GB2312_display(1,0,(uchar *)"3.退货  4.预授权");
		//Os__GB2312_display(2,0,(uchar *)"5.离线  6.打  印");
		//Os__GB2312_display(3,0,(uchar *)"7.管理  8.其  它");

		UTIL_ClearGlobalData();
		UTIL_GetTerminalInfo();
		MAG_Read_LoadTransType();
		ucResult = SUCCESS;
		ucNextKey = 0;

		switch(ucKey)
		{
			case '1':
				if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"一体机模式下");
					//Os__GB2312_display(1,0,(uchar *)"该交易不支持!");
					MSG_WaitKey(3);
					ucResult = ERR_CANCEL;
					break;
				}
				if(DataSave0.ConstantParamData.ucInstallment=='1')
				{
					ucResult=MAG_SaleInstallment();
				}
				else
				{
					if(!I_Sale)
						ucResult = ERR_TRANSNOTSUP;
					else
					{
						G_NORMALTRANS_ucTransType = TRANS_PURCHASE;
						if(!ucResult) ucResult = Trans_Process(0);
						util_Printf("\nucResult =103=%d",ucResult);
					}
				}
				break;
			case '2':
				if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"一体机模式下");
					//Os__GB2312_display(1,0,(uchar *)"该交易不支持!");
					MSG_WaitKey(3);
					ucResult = ERR_CANCEL;
					break;
				}

				if(!I_UnSale)
					ucResult = ERR_TRANSNOTSUP;
				else
				{
					G_NORMALTRANS_ucTransType = TRANS_VOIDPURCHASE;
					if(!ucResult) ucResult = Trans_Process(0);
				}
				break;
			case '3':
				if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"一体机模式下");
					//Os__GB2312_display(1,0,(uchar *)"该交易不支持!");
					MSG_WaitKey(3);
					ucResult = ERR_CANCEL;
					break;
				}

				if(!I_Refund)
					ucResult = ERR_TRANSNOTSUP;
				else
				{
					G_NORMALTRANS_ucTransType = TRANS_REFUND;
					if(!ucResult) ucResult = Trans_Process(0);
				}
				break;
			case '4':
				if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"一体机模式下");
					//Os__GB2312_display(1,0,(uchar *)"该交易不支持!");
					MSG_WaitKey(3);
					ucResult = ERR_CANCEL;
					break;
				}

				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"1.预授权 ");
				//Os__GB2312_display(1,0,(uchar *)"2.追加预授权");
				//Os__GB2312_display(2,0,(uchar *)"3.预授权完成联机");
				//Os__GB2312_display(3,0,(uchar *)"4.预授权完成离线");

				while(1)
				{
					ucNextKey = UTIL_WaitGetKey(20);
					switch(ucNextKey)
					{
						case '1':
							if(!I_Auth)
								ucResult = ERR_TRANSNOTSUP;
							else
							{
								G_NORMALTRANS_ucTransType = TRANS_PREAUTH;
								if(!ucResult) ucResult = Trans_Process(0);
							}
							ucNextKey = KEY_CLEAR;
							break;
						case '2':
							if(!I_AddAuth)
								ucResult = ERR_TRANSNOTSUP;
							else
							{
								G_NORMALTRANS_ucTransType = TRANS_PREAUTHADD;
								if(!ucResult) ucResult = Trans_Process(0);
							}
							ucNextKey = KEY_CLEAR;
							break;
						case '3':
							if(!I_AuthFin)
								ucResult = ERR_TRANSNOTSUP;
							else
							{
								G_NORMALTRANS_ucTransType = TRANS_PREAUTHFINISH;
								if(!ucResult) ucResult = Trans_Process(0);
							}
							ucNextKey = KEY_CLEAR;
							break;
						case '4':
							if(!I_AuthFinishOff)
								ucResult = ERR_TRANSNOTSUP;
							else
							{
								G_NORMALTRANS_ucTransType = TRANS_PREAUTHSETTLE;
								if(!ucResult) ucResult = Trans_Process(0);
							}
							ucNextKey = KEY_CLEAR;
							break;
						case '5':
							if(!I_UnAuth)
								ucResult = ERR_TRANSNOTSUP;
							else
							{
								G_NORMALTRANS_ucTransType = TRANS_VOIDPREAUTH;
								if(!ucResult) ucResult = Trans_Process(0);
							}
							ucNextKey = KEY_CLEAR;
							break;
						case '6':
							if(!I_UnAuthFin)
								ucResult = ERR_TRANSNOTSUP;
							else
							{
								G_NORMALTRANS_ucTransType = TRANS_VOIDPREAUTHFINISH;
								if(!ucResult) ucResult = Trans_Process(0);
							}
							ucNextKey = KEY_CLEAR;
							break;
						case KEY_F4:
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,(uchar *)"1.预授权 ");
							//Os__GB2312_display(1,0,(uchar *)"2.追加预授权 ");
							//Os__GB2312_display(2,0,(uchar *)"3.预授权完成联机");
							//Os__GB2312_display(3,0,(uchar *)"4.预授权完成离线");
							break;
						case KEY_F3:
							//Os__clr_display(255);
							//Os__GB2312_display(0,0,(uchar *)"5.预授权撤销    ");
							//Os__GB2312_display(1,0,(uchar *)"6.预授权完成撤销");
							break;
						default:
							break;
					}
					if(ucNextKey == KEY_CLEAR)
						break;
				}
				break;
			case '5'://离线
				if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"一体机模式下");
					//Os__GB2312_display(1,0,(uchar *)"该交易不支持!");
					MSG_WaitKey(3);
					ucResult = ERR_CANCEL;
					break;
				}

				while(1)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"1.离线结算");
					//Os__GB2312_display(1,0,(uchar *)"2.离线调整");
					//Os__GB2312_display(2,0,(uchar *)"3.离线撤销");

					ucNextKey = UTIL_WaitGetKey(20);
					switch(ucNextKey)
					{
						case '1':
							if(!I_Offline)
								ucResult = ERR_TRANSNOTSUP;
							else
							{
								G_NORMALTRANS_ucTransType = TRANS_OFFPREAUTHFINISH;
								G_RUNDATA_ucTransType=G_NORMALTRANS_ucTransType;
								G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
								ucResult = Trans_Process(0);
							}
							ucNextKey = KEY_CLEAR;
							break;
						case '2':
							if(!I_TipFee)
								ucResult = ERR_TRANSNOTSUP;
							if(ucResult == SUCCESS)
							{
								G_NORMALTRANS_ucTransType = TRANS_TIPADJUST;
								ucResult = Trans_Process(0);
							}
							ucNextKey = KEY_CLEAR;
							break;
						case '3':
							if(I_UndoOff == 0x30)
								ucResult = ERR_TRANSNOTSUP;
							else
							{
								G_NORMALTRANS_ucTransType = TRANS_UNDOOFF;
								G_NORMALTRANS_euCardSpecies = CARDSPECIES_EMV;
								G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_MANUALINPUT;
								ucResult=Trans_Process(0);
							}
							ucNextKey = KEY_CLEAR;
							break;
						default:
							break;
					}
					if(ucNextKey == KEY_CLEAR)
						break;
				}
				break;

			case '6':		//打印
				if(DataSave0.ConstantParamData.ucRePAuthority=='1')
					ucResult=MAG_InputCashierPass();
				if(!ucResult)
				{
					SEL_ServiceMenu();
				}
				break;
			case '7':		//管理
				while(1)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"1签到      2签退");
					//Os__GB2312_display(1,0,(uchar *)"3交易查询  4统计");
					//Os__GB2312_display(2,0,(uchar *)"5外线号码  6结算");
					//Os__GB2312_display(3,0,(uchar *)"7锁定终端  8版本");
					ucNextKey = UTIL_WaitGetKey(20);
					switch(ucNextKey)
					{
						case '1':
							G_NORMALTRANS_ucTransType = TRANS_LOGONON;
							ucResult = LOGON_CashierProcess(0);
							ucNextKey = KEY_CLEAR;
							break;
						case '2':
							G_NORMALTRANS_ucTransType = TRANS_LOGONOFF;
							ucResult = LOGON_CashierProcess(0);
							ucNextKey = KEY_CLEAR;
							break;
						case '3':
							Trans_Query();
							ucNextKey = KEY_CLEAR;
							break;
						case '4':
							SERV_DispTotal();
							ucNextKey = KEY_CLEAR;
							break;
						case '5':
							ucResult = CFG_ConstantParamPhoneNo();
							ucNextKey = KEY_CLEAR;
							break;
						case '6':
							if (DataSave0.ConstantParamData.ucDisplay == 0x31)
							{
								ucResult=SERV_DispTotal();
							}else
								ucResult = 0x01;
							util_Printf("ucResult=%02x",ucResult);
							if (ucResult!=SUCCESS)
							{
								ucResult = SERV_Settle(false);
								ucNextKey = KEY_CLEAR;
							}
							break;
						case '7':
							ucResult = SEL_LockKeyBoard();
							if( (ucResult != SUCCESS)&&(ucResult != ERR_CANCEL))
							{
								MSG_DisplayErrMsg(ucResult);
							}
							return SUCCESS ;
						case '8':
							CFG_ConstantParamVersion();
							ucNextKey = KEY_CLEAR;
							break;
						default:
							break;
					}
					if(ucNextKey == KEY_CLEAR)
						break;
				}
				break;
			case '8':		//其它
#ifndef UNTOUCH
				ucFlag =1;
				while(1)
				{
					//Os__clr_display(255);
                    //if(ucFlag==1)
                    //{
						//Os__GB2312_display(0,0,(uchar *)"1.电子现金");
						//Os__GB2312_display(1,0,(uchar *)"2.分期付款");
						//Os__GB2312_display(2,0,(uchar *)"3.联机余额查询");
						//Os__GB2312_display(3,0,(uchar *)"4.回响测试");
                    //}
                    //else
                    //{
					   	//Os__GB2312_display(0,0,(uchar *)"5.切换通讯模式");
						//Os__GB2312_display(1,0,(uchar *)"6.修改主管密码");
						//Os__GB2312_display(2,0,(uchar *)"7.预约支付");
                        //if (DataSave0.ConstantParamData.ucCollectFlag==0x31)
						//Os__GB2312_display(3,0,(uchar *)"8.数据采集");
                    //}
					ucNextKey = UTIL_WaitGetKey(20);
					switch(ucNextKey)
					{
						case '1':
							ucResult =Mag_ECMenu();
							COMMS_FinComm();
							ucNextKey =KEY_CLEAR;
							break;
						case '2':
							ucResult = MAG_CreditSale();			//分期付款-工行模式
						    //ucResult = Trans_VoidCUP();		//暂时屏蔽移动支付撤销交易
						    ucNextKey =KEY_CLEAR;
						    break;
						case '3':
							if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
							{
								//Os__clr_display(255);
								//Os__GB2312_display(0,0,(uchar *)"一体机模式下");
								//Os__GB2312_display(1,0,(uchar *)"该交易不支持!");
								MSG_WaitKey(3);
								ucResult = ERR_CANCEL;
								ucNextKey = KEY_CLEAR;
								break;
							}
							ucResult = Trans_EnqBalance();
							COMMS_FinComm();
							ucNextKey = KEY_CLEAR;
							break;
						case '4':
							ucResult = Trans_TestSingal();
							COMMS_FinComm();
							ucNextKey = KEY_CLEAR;
							break;
						case '5':
							ucResult = CFG_SwitchComm();
							ucNextKey = KEY_CLEAR;
							break;
						case '6':
							ucResult = SYSCASH_ModiCashier();
							ucNextKey = KEY_CLEAR;
							 break;
						case '7':
						    	if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
							{
  								//Os__clr_display(255);
  								//Os__GB2312_display(0,0,(uchar *)"一体机模式下");
  								//Os__GB2312_display(1,0,(uchar *)"该交易暂不支持!");
  								MSG_WaitKey(3);
  								ucResult = ERR_CANCEL;
  								ucNextKey = KEY_CLEAR;
								break;
							}
							 if (DataSave0.ConstantParamData.ucMobileFlag=='1')
								ucResult=MAG_MOBILEMENU();
							else
								ucResult = ERR_TRANSNOTSUP;
						    	ucNextKey =KEY_CLEAR;
						    	break;

					    	case '8':
					    		if (DataSave0.ConstantParamData.ucCollectFlag==0x31)
						    		ucResult=MAG_COLLECT();
						    	else
						    		ucResult=ERR_CANCEL;
						    	break;
						case KEY_F3:
							ucFlag =1;
							break;
						case KEY_F4:
							ucFlag =2;
							break;
						default:
							break;
					}
					util_Printf("case '8':[%02x]\n",ucResult);
					if(ucNextKey == KEY_CLEAR)
						break;
				}
#else
				ucResult = MAG_Menu_KEYEIGHT();
#endif
				break;
			case '9':
				Menu_KEYNINE();
				break;
			case KEY_F1:
				util_Printf("\nKEY_F1");
				G_NORMALTRANS_ucTransType = TRANS_LOGONON;
				ucResult = LOGON_CashierProcess(0);
				ucNextKey = KEY_CLEAR;
				break;
			case KEY_F3:
				util_Printf("\nKEY_F3");
				if (DataSave0.ConstantParamData.ucQuickUndo == 0x31)
				{
					ucResult = MAG_QuickUndo();
				}
				else
				{
					ucResult = SEL_LockKeyBoard();
					if (ucResult == ERR_CANCEL)
						return(ucResult);
				}
				break;
			case KEY_F4:
				ucResult=CFG_SwitchTransType();
				if(ucResult==ERR_CANCEL)
					return(ucResult);
				break;
			case KEY_PAPER_FEED:
				PRINT_xlinefeed(LINENUM*2);
				break;
			case KEY_F:
				PRINT_xlinefeed(LINENUM*1);
				break;
			case KEY_R:
				PRINT_XReverseFeed(LINENUM*1);
				break;
			default:
				break;
		}
		COMMS_FinComm();
		util_Printf("F3键1:%02x\n",ucResult);
		if( (ucResult != SUCCESS)&&(ucResult != ERR_CANCEL) &&(ucResult != ERR_END))
		{
			MSG_DisplayErrMsg(ucResult);
		}
		UTIL_WaitPullOutICC();
	}
	Os__light_off();
	DispPINPAD_AD();
	return(SUCCESS);
}

unsigned char Mag_AuthMenu(void)
{
	unsigned char ucResult;
	unsigned char ucNextKey;


	while(1)
	{
		if(ucNextKey != KEY_F3)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"1.预授权 ");
			//Os__GB2312_display(1,0,(uchar *)"2.追加预授权");
			//Os__GB2312_display(2,0,(uchar *)"3.预授权完成联机");
			//Os__GB2312_display(3,0,(uchar *)"4.预授权完成离线");
		}
		if(DataSave0.ConstantParamData.Pinpadflag ==1)
		{
			//if((!ReaderSupport)||(ReaderType!=READER_SAND))
			{
				Os__clr_display_pp(255);
				//Os__GB2312_display_pp(0,0,(uchar *)"   杉德银卡通");
				Os__GB2312_display_pp(0,0,(uchar *)"    欢迎使用");
			}
		}

		ucNextKey = UTIL_WaitGetKey(20);
		switch(ucNextKey)
		{
			case '1':
				if(!I_Auth)
					ucResult = ERR_TRANSNOTSUP;
				else
				{
					G_NORMALTRANS_ucTransType = TRANS_PREAUTH;
					if(!ucResult) ucResult = Trans_Process(0);
				}
				ucNextKey = KEY_CLEAR;
				break;
			case '2':
				if(!I_AddAuth)
					ucResult = ERR_TRANSNOTSUP;
				else
				{
					G_NORMALTRANS_ucTransType = TRANS_PREAUTHADD;
					if(!ucResult) ucResult = Trans_Process(0);
				}
				ucNextKey = KEY_CLEAR;
				break;
			case '3':
				if(!I_AuthFin)
					ucResult = ERR_TRANSNOTSUP;
				else
				{
					G_NORMALTRANS_ucTransType = TRANS_PREAUTHFINISH;
					if(!ucResult) ucResult = Trans_Process(0);
				}
				ucNextKey = KEY_CLEAR;
				break;
			case '4':
				if(!I_AuthFinishOff)
					ucResult = ERR_TRANSNOTSUP;
				else
				{
					G_NORMALTRANS_ucTransType = TRANS_PREAUTHSETTLE;
					if(!ucResult) ucResult = Trans_Process(0);
				}
				ucNextKey = KEY_CLEAR;
				break;
			case '5':
				if(!I_UnAuth)
					ucResult = ERR_TRANSNOTSUP;
				else
				{
					G_NORMALTRANS_ucTransType = TRANS_VOIDPREAUTH;
					if(!ucResult) ucResult = Trans_Process(0);
				}
				ucNextKey = KEY_CLEAR;
				break;
			case '6':
				if(!I_UnAuthFin)
					ucResult = ERR_TRANSNOTSUP;
				else
				{
					G_NORMALTRANS_ucTransType = TRANS_VOIDPREAUTHFINISH;
					if(!ucResult) ucResult = Trans_Process(0);
				}
				ucNextKey = KEY_CLEAR;
				break;
			case KEY_F3:
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"5.预授权撤销    ");
				//Os__GB2312_display(1,0,(uchar *)"6.预授权完成撤销");
				break;
			default:
				break;
		}
		if(ucNextKey == KEY_CLEAR)
			break;
	}

	return ucResult;

}
unsigned char Mag_ECMenu(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char ucKey,ucFlag;

	ucFlag =1;
	while(1)
	{
		UTIL_ClearGlobalData();
		UTIL_GetTerminalInfo();
		MAG_Read_LoadTransType();
		//Os__clr_display(255);

		//Os__GB2312_display(0,0,(uchar *)"1.快速支付");
		//Os__GB2312_display(1,0,(uchar *)"2.圈存");
		//Os__GB2312_display(2,0,(uchar *)"3.余额查询");
		//Os__GB2312_display(3,0,(uchar *)"4.明细查询");
		ucKey = UTIL_WaitGetKey(60);
		switch(ucKey)
		{
			case '1':
				G_NORMALTRANS_ucTransType = TRANS_PURCHASE;
				G_NORMALTRANS_ucECTrans =TRANS_Q_PURCHASE;
				if(!ReaderSupport)
				{
						//Os__clr_display(255);
						//Os__GB2312_display(1,0,(uchar *)"暂不支持该交易!");
						UTIL_WaitGetKey(3);
						return ERR_CANCEL;
				}
				ucResult = Trans_Process(0);
				ucKey =KEY_CLEAR;
				break;
			case '2':
				while(1)
				{
					UTIL_ClearGlobalData();
					UTIL_GetTerminalInfo();
					MAG_Read_LoadTransType();
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"1.现金充值");
					//Os__GB2312_display(1,0,(uchar *)"2.指定账户圈存");
					//Os__GB2312_display(2,0,(uchar *)"3.非指定账户圈存");
					ucKey = UTIL_WaitGetKey(60);
					switch (ucKey)
					{
						case '1':
							G_NORMALTRANS_ucTransType = TRANS_EC_CASHLOAD;
							ucResult = CASH_InputAndChackManagerPass(0);
							if(ucResult == SUCCESS)
							{
								//Os__clr_display(255);
								//Os__GB2312_display(1,0,(uchar *)"    请注意");
								//Os__GB2312_display(2,0,(uchar *)" 该交易要收现金");
								UTIL_WaitGetKey(3);
								G_NORMALTRANS_ucTransType = TRANS_EC_CASHLOAD;
								ucResult = Trans_Process(0);
							}
							ucKey =KEY_CLEAR;
							break;
						case '2':
							G_NORMALTRANS_ucTransType =TRANS_EC_ASSIGNLOAD;
							ucResult = Trans_Process(0);
							ucKey =KEY_CLEAR;
							break;
						case '3':
							G_NORMALTRANS_ucTransType = TRANS_EC_UNASSIGNLOAD;
							ucResult = Trans_Process(0);
							ucKey =KEY_CLEAR;
							break;
						default:
							ucKey =KEY_CLEAR;
							break;
					}
					if(ucKey==KEY_CLEAR)
						break;
				}
				break;
			case '3':
				ucResult =ReadICBalance();
				ucKey =KEY_CLEAR;
				break;
			case '4':
				ucResult =ReadICTransLog();
				ucKey =KEY_CLEAR;
				break;
#if 0
			case '5':
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult =EMV_GetTagValueFromStr((uchar *)"\x9F\x11\x03\x31\x32\x33\x82\x02\x39\x38",10,(uchar *)"\x9F\x11",aucBuf,&usLen);
				util_Printf("\naucBuf11=%s,%d\n",aucBuf,ucResult);
				for(i=0;i<usLen;i++)
		 			util_Printf("%02x ",*(aucBuf+i));
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult =EMV_GetTagValueFromStr((uchar *)"\x9F\x11\x03\x31\x32\x33\x82\x02\x39\x38",10,(uchar *)"\x82",aucBuf,&usLen);
				util_Printf("\naucBuf22=%s,%d\n",aucBuf,ucResult);
				for(i=0;i<usLen;i++)
		 			util_Printf("%02x ",*(aucBuf+i));
				ucKey =KEY_CLEAR;
				break;
#endif
			default:
				ucKey =KEY_CLEAR;
				break;
		}
		COMMS_FinComm();
		if(ucKey == KEY_CLEAR)
			break;
	}

	return ucResult;

}

unsigned char MAG_EMVSetInputMode(unsigned char * ucReadMode)
{
	unsigned char ucResult,ucInputMode;

	ucResult=SUCCESS;

#ifdef USEINSERTCARD
		ucInputMode=TRANS_INPUTMODE_SWIPECARD
					|TRANS_INPUTMODE_MANUALINPUT
					|TRANS_INPUTMODE_INSERTCARD ;
#else
		ucInputMode=TRANS_INPUTMODE_SWIPECARD
					|TRANS_INPUTMODE_MANUALINPUT;
#endif

	switch(G_NORMALTRANS_ucTransType)
	{
		case TRANS_VOIDPURCHASE:
			if( DataSave0.ConstantParamData.ucUnSaleSwipe=='1')
				ucInputMode|=TRANS_INPUTMODE_SWIPECARD;
			else
				ucInputMode&=~TRANS_INPUTMODE_SWIPECARD;
			break;
		case TRANS_VOIDPREAUTHFINISH:
			if( DataSave0.ConstantParamData.ucUnAuthFinSwipe=='1')
				ucInputMode|=TRANS_INPUTMODE_SWIPECARD;
			else
				ucInputMode&=~TRANS_INPUTMODE_SWIPECARD;
			break;
		default:
			break;
	}
#ifdef USEINSERTCARD
	switch(G_NORMALTRANS_ucTransType)
	{
		case TRANS_VOIDPURCHASE:
			if( DataSave0.ConstantParamData.ucUnSaleSwipe=='1')
				ucInputMode|=TRANS_INPUTMODE_INSERTCARD;
			else
				ucInputMode&=~TRANS_INPUTMODE_INSERTCARD;
			break;
		case TRANS_PREAUTHADD:
		case TRANS_PREAUTHFINISH:       //EMV IC不支持联机预授权
			ucInputMode&=~TRANS_INPUTMODE_INSERTCARD;
			break;
		default:
			break;
	}
#endif
	util_Printf("手输功能 = %02x %02x\n",ucInputMode,DataSave0.ConstantParamData.ucInputCardNo);
	if(DataSave0.ConstantParamData.ucInputCardNo == 0x30)//全都支持手输卡号功能
	{
		ucInputMode|=TRANS_INPUTMODE_MANUALINPUT;
		switch(G_NORMALTRANS_ucTransType)
		{
			case TRANS_VOIDPURCHASE:
			case TRANS_PREAUTH:
			case TRANS_VOIDPREAUTH:
			case TRANS_PREAUTHFINISH:
			case TRANS_VOIDPREAUTHFINISH:
			case TRANS_PREAUTHSETTLE:
			case TRANS_QUERYBAL:
			case TRANS_OFFPREAUTHFINISH:	//离线结算
			case TRANS_PREAUTHADD:
				break;
			case TRANS_REFUND:
			case TRANS_PURCHASE:
			case TRANS_CREDITSALE:
			case TRANS_CREDITSREFUND:
				ucInputMode&=~TRANS_INPUTMODE_MANUALINPUT;
				break;
			default:
				break;
		}
	}
	else if(DataSave0.ConstantParamData.ucInputCardNo ==0x31)//全不支持手输卡号功能
	{
		ucInputMode&=~TRANS_INPUTMODE_MANUALINPUT;
	}else if(DataSave0.ConstantParamData.ucInputCardNo ==0x32)//仅支持预授权完成
	{
		ucInputMode|=TRANS_INPUTMODE_MANUALINPUT;
		switch(G_NORMALTRANS_ucTransType)
		{
			case TRANS_PREAUTHFINISH:
				break;
			default:
				ucInputMode&=~TRANS_INPUTMODE_MANUALINPUT;
				break;
		}
	}

	util_Printf("输入模式 = %02x\n",ucInputMode);
	if(!ucInputMode)
		return ERR_NOTPROC;

	if(ucInputMode==TRANS_INPUTMODE_FULLBACK)
		ucInputMode=TRANS_INPUTMODE_SWIPECARD;
	switch(G_NORMALTRANS_ucTransType)
	{
		case TRANS_EC_ASSIGNLOAD:
		case TRANS_EC_UNASSIGNLOAD:
		case TRANS_EC_CASHLOAD:
			ucInputMode =TRANS_INPUTMODE_INSERTCARD;
			break;
		default:
			break;
	}

	*ucReadMode = ucInputMode;
	util_Printf("ucInputMode = %02X\n",ucInputMode);
	return SUCCESS ;
}


void MAG_Read_LoadTransType(void)
{
	unsigned char ucTransType1;
	unsigned char ucTransType2;
	ucTransType1 = DataSave0.ConstantParamData.aucGlobalTransEnable[0];
	ucTransType2 = DataSave0.ConstantParamData.aucGlobalTransEnable[1];
	if(!DataSave0.ConstantParamData.ucLoadFlag)
	{
		ucTransType1 = 0xFF;
		ucTransType2 = 0xF0;
	}
	I_Inquiry = ucTransType1&0x80;
	I_Auth = ucTransType1&0x40;
	I_UnAuth = ucTransType1&0x20;
	I_AuthFin = ucTransType1&0x10;
	I_UnAuthFin = ucTransType1&0x08;
	I_Sale = ucTransType1&0x04;
	I_UnSale = ucTransType1&0x02;
	I_Refund = ucTransType1&0x01;

	I_Offline = ucTransType2&0x80;
	I_TipFee = ucTransType2&0x40;
	I_AuthFinishOff = ucTransType2&0x20;
	I_ICScript = ucTransType2&0x10;
	I_OffSale = ucTransType2&0x08;
	I_AddAuth = ucTransType2&0x04;
	I_PurseDeposit = ucTransType2&0x02;

	G_NORMALTRANS_ucTransType=DataSave0.ConstantParamData.ucDefaultTransParam;
#if 0
util_Printf("\aucGlobalTransEnable ---000---= %02x \n", DataSave0.ConstantParamData.aucGlobalTransEnable[0]);
util_Printf("aucGlobalTransEnable ---000---= %02x \n",DataSave0.ConstantParamData.aucGlobalTransEnable[1]);

util_Printf("\nucTransType1 ---000---= %02x \n", ucTransType1);
util_Printf("ucTransType2 ---000---= %02x \n",ucTransType1);

util_Printf("I_Inquiry = %02x \n",I_Inquiry);
util_Printf("I_Auth = %02x \n",I_Auth);
util_Printf("I_UnAuth = %02x \n",I_UnAuth);
util_Printf("I_AuthFin = %02x \n",I_AuthFin);
util_Printf("I_UnAuthFin = %02x \n",I_UnAuthFin);
util_Printf("I_Sale = %02x \n",I_Sale);
util_Printf("I_UnSale = %02x \n",I_UnSale);
util_Printf("I_Refund = %02x \n",I_Refund);

util_Printf("I_Offline = %02x \n",I_Offline);
util_Printf("I_TipFee = %02x \n",I_TipFee);
util_Printf("I_AuthFinishOff = %02x \n",I_AuthFinishOff);
util_Printf("I_ICScript = %02x \n",I_ICScript);
util_Printf("I_OffSale = %02x \n",I_OffSale);
util_Printf("I_AddAuth = %02x \n",I_AddAuth);
util_Printf("I_PurseDeposit = %02x \n",I_PurseDeposit);
#endif
}


unsigned char  MAG_DispCardNo(void)
{
	unsigned char ucResult,ucCardKey;
	unsigned short ucJ;

	while(1)
	{
		Os__light_on();
		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (unsigned char * )"卡号:");
		Os__display(1, 0, G_NORMALTRANS_ucDispSourceAcc);
		Os__display(2, 0, &G_NORMALTRANS_ucDispSourceAcc[16]);
		//Os__GB2312_display(3, 0, (unsigned char * )"按[确认]键继续");
#ifndef GUI_PROJECT
		ucCardKey = UTIL_GetKey(3);
#else
		ucCardKey = KEY_ENTER;
#endif
		util_Printf("\nucCardKey=%02x\n",ucCardKey);

		switch(ucCardKey)
		{
			case KEY_ENTER:
				ucResult = SUCCESS;
				break;
			case 99:
			case KEY_CLEAR:
				ucResult = ERR_CANCEL;
				break;
			default:
				continue;
		}
		break;
	}
	if((!ucResult)
		&& (G_NORMALTRANS_ucTransType == TRANS_OFFPREAUTHFINISH))
	{
		G_NORMALTRANS_ucSourceAccLen=G_NORMALTRANS_ucDispSourceAccLen;
		asc_bcd(G_NORMALTRANS_aucSourceAcc,(G_NORMALTRANS_ucDispSourceAccLen+1)/2,G_NORMALTRANS_ucDispSourceAcc,
			((G_NORMALTRANS_ucDispSourceAccLen+1)/2)*2
				);
		for(ucJ=0;ucJ<G_EXTRATRANS_uiISO2Len;ucJ++)
		{
			if(G_EXTRATRANS_aucISO2[ucJ] == '=')
			{
				asc_bcd(G_NORMALTRANS_ucExpiredDate,2,&G_EXTRATRANS_aucISO2[ucJ+1],4);
				util_Printf("有效期 = %02x %02x %02x %02x\n",G_EXTRATRANS_aucISO2[ucJ+1],G_EXTRATRANS_aucISO2[ucJ+2],G_EXTRATRANS_aucISO2[ucJ+3],G_EXTRATRANS_aucISO2[ucJ+4]);
				util_Printf("有效期 = %02x %02x \n",G_NORMALTRANS_ucExpiredDate[0],G_NORMALTRANS_ucExpiredDate[1]);
			}
		}
	}
	return ucResult;
}

unsigned char MAG_CardSelect(unsigned char ucInputMode ,unsigned char *PucCardSelect,unsigned char *pucTrack)
{
	unsigned char ucResult=0;
	unsigned char ucCardSelect;


	ucCardSelect=CARDSPECIES_MAG;
	//util_Printf("Sel:\n");
	//for(i=0;i<100;i++)
	//	util_Printf(" %02x",pucTrack[i]);
	//util_Printf("\n");
	util_Printf("\n 		MAG_CardSelect begin....[0001] [rst=%02x]\n",ucResult);				
	ucResult= MAG_GetTrackInfo(pucTrack);

	util_Printf("\n 		MAG_CardSelect begin....[0002] [rst=%02x]\n",ucResult);				
	
       if (ucResult==ERR_SANDMAGCARD)  return(ERR_SANDMAGCARD);
	if(!ucResult||ucResult==ERR_USEICCARDFIRST)
	{
		G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
		G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
		if(ucInputMode&TRANS_INPUTMODE_FULLBACK) ucResult=SUCCESS;
		util_Printf("MAG_CardSelect()  ---1---    ucResult = %02X\n",ucResult);
	}
	if(ucInputMode&TRANS_INPUTMODE_INSERTCARD&&ucResult==ERR_USEICCARDFIRST)
	{
		ucCardSelect=MAG_ManuSelectCard(G_NORMALTRANS_ucTransType);
	}
	util_Printf("ucCardSelect=%02x\n",ucCardSelect);
	*PucCardSelect = ucCardSelect;
	return ucResult;
}

//#define TrackTest
//#define In
unsigned char MAG_GetTrackInfo(unsigned char *pucTrack)
{
	unsigned short uiI,usJJ,ucJ;
	unsigned char  *pucPtr,ucAccType;

//	util_Printf("\n			1.MAG_GetTrackInfo ..................\n\n [ulAmount :%d] [uiPwLen :%d] [aucPasswd :%s]\n", ProUiFace.UiToPro.ulAmount,ProUiFace.UiToPro.uiPwLen,ProUiFace.UiToPro.aucPasswd);
	util_Printf("\n		MAG_GetTrackInfo ucResult = [0001]\n");
	pucPtr = pucTrack;
	
	pucPtr += 2;
	/* Track2 information */
	G_EXTRATRANS_ucISO2Status = *(pucPtr);
	pucPtr ++;
	G_EXTRATRANS_uiISO2Len = *(pucPtr);

	util_Printf("		MAG_GetTrackInfo ucResult = [0002] --uiISO2Len: [%d]\n",G_EXTRATRANS_uiISO2Len);
	/***判断磁道长度是否正确***/
	if(G_EXTRATRANS_uiISO2Len > TRANS_ISO2LEN || G_EXTRATRANS_uiISO2Len<TRANS_ISO2MINLEN)
		return(ERR_MAG_TRACKDATA);

	util_Printf("		MAG_GetTrackInfo ucResult = [0003]\n");
		
	pucPtr ++;

	util_Printf("G_EXTRATRANS_ucISO2Status=%02x,%02x\n",G_EXTRATRANS_ucISO2Status,G_EXTRATRANS_uiISO2Len);

	for( ucJ=0; ucJ<G_EXTRATRANS_uiISO2Len; ucJ++,pucPtr ++)
	{
    		G_EXTRATRANS_aucISO2[ucJ] =  *(pucPtr) | 0x30;
	}
	util_Printf("Track2 Data:%s\n",G_EXTRATRANS_aucISO2);

       if (!memcmp(&G_EXTRATRANS_aucISO2[20],"===",3)){
	util_Printf("retrun ERR_SANDMAGCARD.\n");
		return (ERR_SANDMAGCARD);
       }
    /* Track3 information */
   	G_EXTRATRANS_ucISO3Status = *(pucPtr);

	pucPtr ++;
	G_EXTRATRANS_uiISO3Len = *(pucPtr);

	/***判断磁道长度是否正确***/
	if(G_EXTRATRANS_uiISO3Len > TRANS_ISO3LEN)
		return(ERR_MAG_TRACKDATA);
	pucPtr ++;

	util_Printf("Track3 ISO3Status=%02x,%02x\n",G_EXTRATRANS_ucISO3Status,G_EXTRATRANS_uiISO3Len);
//	util_Printf("\n			2.MAG_GetTrackInfo ..................\n\n [ulAmount :%d] [uiPwLen :%d] [aucPasswd :%s]\n", ProUiFace.UiToPro.ulAmount,ProUiFace.UiToPro.uiPwLen,ProUiFace.UiToPro.aucPasswd);
	for( uiI=0; uiI<G_EXTRATRANS_uiISO3Len; uiI++,pucPtr ++)
        {
            G_EXTRATRANS_aucISO3[uiI] =  *(pucPtr) | 0x30;
        }
//util_Printf("Track3 Data:%s\n",G_EXTRATRANS_aucISO3);
//util_Printf("%s\n",G_EXTRATRANS_aucISO3);
//util_Printf("\n			3.MAG_GetTrackInfo ..................\n\n [ulAmount :%d] [uiPwLen :%d] [aucPasswd :%s]\n", ProUiFace.UiToPro.ulAmount,ProUiFace.UiToPro.uiPwLen,ProUiFace.UiToPro.aucPasswd);
	if( (G_EXTRATRANS_ucISO2Status == SUCCESS_TRACKDATA)
	  &&( (G_EXTRATRANS_ucISO3Status == SUCCESS_TRACKDATA)
	    ||(G_EXTRATRANS_ucISO3Status == DRV_FULL)
		||(G_EXTRATRANS_ucISO3Status == DRV_FREE))
	  )
	{
		/**************取卡号后12位减一位******************/
		for(ucJ=0;ucJ<G_EXTRATRANS_uiISO2Len;ucJ++)
		{
			if(G_EXTRATRANS_aucISO2[ucJ] == '=')
			{
				/*************取卡号***************/
				//---------FANGBO MODIFY DATE:2006/01/05------
	util_Printf("\n	4.MAG_GetTrackInfo ..................\n\n 		[ulAmount :%d] [uiPwLen :%d] [aucPasswd :%s]\n", 
	ProUiFace.UiToPro.ulAmount,ProUiFace.UiToPro.uiPwLen,ProUiFace.UiToPro.aucPasswd);				
				//util_Printf("\nG_EXTRATRANS_aucISO2 =[%02x]\n",ucJ);
				util_Printf("\nG_EXTRATRANS_aucISO2 =[%d]\n",ucJ);
				memset(G_NORMALTRANS_ucDispSourceAcc,0,20);
				memcpy(G_NORMALTRANS_ucDispSourceAcc, G_EXTRATRANS_aucISO2, ucJ);// max 20
				//util_Printf("G_NORMALTRANS_ucDispSourceAcc = %s\n",G_NORMALTRANS_ucDispSourceAcc);
	util_Printf("\n	5.MAG_GetTrackInfo ..................\n\n 		[ulAmount :%d] [uiPwLen :%d] [aucPasswd :%s]\n", 
	ProUiFace.UiToPro.ulAmount,ProUiFace.UiToPro.uiPwLen,ProUiFace.UiToPro.aucPasswd);
				
				G_NORMALTRANS_ucDispSourceAccLen = ucJ;
				asc_bcd(G_NORMALTRANS_ucExpiredDate,2,&G_EXTRATRANS_aucISO2[ucJ+1],4);
				//------------end----------------
				if(ucJ>=13)
					asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO2[ucJ-13],12);
				else
					asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO2[0],12);
				break;
			}
		}
		/***************************/
		if ((G_EXTRATRANS_ucISO3Status == DRV_FULL) ||
			(G_EXTRATRANS_ucISO3Status == DRV_FREE)
			)
		{
			G_NORMALTRANS_ucSourceAccType = '0';
			G_EXTRATRANS_uiISO3Len = 0;
		}else
		{
			for(uiI=0;uiI<G_EXTRATRANS_uiISO3Len;uiI++)
			{
				if(G_EXTRATRANS_aucISO3[uiI]=='=')
				{
					if(!G_EXTRATRANS_uiISO2Len)
					{
						if(uiI>=13)
							asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO3[uiI-13],12);
						else
							asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO3[0],12);
					}
				}
				break;
			}

			if(uiI>=G_EXTRATRANS_uiISO3Len)
				G_NORMALTRANS_ucSourceAccType = '0';
			else
			{
				usJJ = (unsigned short)asc_long(&G_EXTRATRANS_aucISO3[uiI+1],3);

				util_Printf("usJJ=%d\n",usJJ);

				if(usJJ == 156)
					uiI+=3;
				else
					uiI+=1;
				ucAccType = G_EXTRATRANS_aucISO3[uiI+27];
				switch(ucAccType)
				{
					case 0x30:
					case 0x31:
					case 0x32:
					case 0x33:
					case 0x34:
						G_NORMALTRANS_ucSourceAccType = '0';
						break;
					default:
						G_NORMALTRANS_ucSourceAccType = '0';
						break;
				}
			}
		}

		util_Printf("G_NORMALTRANS_ucSourceAccType=%02x\n",G_NORMALTRANS_ucSourceAccType);

		asc_bcd(G_NORMALTRANS_aucISO2,(G_EXTRATRANS_uiISO2Len+1)/2,G_EXTRATRANS_aucISO2,((G_EXTRATRANS_uiISO2Len*2)+1)/2);
		asc_bcd(G_NORMALTRANS_aucISO3,(G_EXTRATRANS_uiISO3Len+1)/2,G_EXTRATRANS_aucISO3,((G_EXTRATRANS_uiISO3Len*2)+1)/2);
		G_NORMALTRANS_ucISO2Len = G_EXTRATRANS_uiISO2Len;
		G_NORMALTRANS_ucISO3Len = G_EXTRATRANS_uiISO3Len;

		util_Printf("G_EXTRATRANS_aucISO2 ucJ = %d \n",ucJ) ;
#ifdef USEINSERTCARD
		if(G_EXTRATRANS_aucISO2[ucJ+5]=='2' ||(G_EXTRATRANS_aucISO2[ucJ+5]=='6'))
			return ERR_USEICCARDFIRST ;
		else if(G_NORMALTRANS_ucFallBackFlag==2)//如果磁条卡service code不为2,6,不能当做fallback交易
			G_NORMALTRANS_ucFallBackFlag=0;

#endif
			return(SUCCESS);
	}else
	{
		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
		//Os__GB2312_display(1, 0, (uchar *)"刷卡错误");
		//Os__GB2312_display(2, 0, (uchar *)"请继续刷卡-->");
		//Os__GB2312_display(3, 0, (uchar *)"或按取消键退出");
		return(ERR_END);
	}
}

unsigned char MAG_ManuSelectCard(unsigned char ucTransType)
{
	unsigned char ucKey,ucResult;

	Os__light_on();

	MSG_DisplyTransType(ucTransType ,0,0,true,255);
	//Os__GB2312_display(1,0,(unsigned char*)"是否为IC卡?");
	//Os__GB2312_display(2,0,(unsigned char*)"0.否");
	//Os__GB2312_display(3,0,(unsigned char*)"1.是");

	do
	{
		ucKey=UTIL_GetKey(30);
		if(ucKey=='0'||ucKey=='1'||ucKey==KEY_CLEAR||ucKey==99)
			break;
	}while(1);

	switch(ucKey)
	{
		case '0':
			ucResult=CARDSPECIES_MAG;
			break;
		case '1':
			G_RUNDATA_ucReadCardFlag = EMVFIRSTMAG;
			ucResult=CARDSPECIES_EMV;
			break;
		default:
			ucResult=ERR_CANCEL;
			break;
	}
	return ucResult;
}

unsigned char MAG_DisplaySwipeCardMode(uchar ucTransType,uchar ucMOde)
{
#ifdef GUI_PROJECT

#else
	MSG_DisplyTransType(ucTransType ,0,0,true,255);

	switch(ucMOde)
	{
		case 0x01:
			//Os__GB2312_display(2,0,(uchar*)"    请刷卡-->");
			break;
		case 0x02:
			//Os__GB2312_display(2,0,(uchar*)"   请输入卡号");
			break;
		case 0x03:
			//Os__GB2312_display(2,0,(uchar*)"请刷卡或输入卡号");
			break;
		case 0x04:
			//Os__GB2312_display(2,0,(uchar*)"请插入IC卡");
			break;
		case 0x05:
			//Os__GB2312_display(2,0,(uchar*)"请插入IC卡或刷卡");
			break;
		case 0x06:
			//Os__GB2312_display(1,0,(uchar*)"请插入IC卡");
			//Os__GB2312_display(2,0,(uchar*)"或手工输入卡号");
			break;
		case 0x07:
			//Os__GB2312_display(1,0,(uchar*)"请插入IC卡或刷卡");
			//Os__GB2312_display(2,0,(uchar*)"或输入卡号");
			break;
		case 0x15:
			//Os__GB2312_display(1,0,(uchar*)"请插入IC卡或刷卡");
			//Os__GB2312_display(2,0,(uchar*)"或放卡");
			break;
		case 0x16:
			//Os__GB2312_display(1,0,(uchar*)"请插入IC卡或刷卡");
			//Os__GB2312_display(2,0,(uchar*)"或输入卡号或挥卡");
			break;
		case 0x17:
			//Os__GB2312_display(2,0,(uchar*)"请插入IC卡或挥卡");
			break;
		case 0x18:
			//Os__GB2312_display(2,2,(uchar*)"请挥卡");
			break;
		case 0x81:
			//Os__GB2312_display(1,0,(uchar*)"刷卡错误");
			//Os__GB2312_display(2,0,(uchar*)"请继续刷卡-->");
			//Os__GB2312_display(3,0,(uchar*)"或按取消键退出");
			break;
		case 0x84:
			//Os__GB2312_display(1,0,(uchar*)"读IC卡错");
			//Os__GB2312_display(2,0,(uchar*)"请刷卡-->");
			//Os__GB2312_display(3,0,(uchar*)"或按取消键退出");
			break;

		default:
			//Os__GB2312_display(2,0,(uchar*)"刷卡方式错误");
			break;
	}
#endif
	return SUCCESS;
}

unsigned char MAG_UerICCardFirstProcess(unsigned char ucCardSelect,unsigned char ucFullBackFlag,unsigned char ucLastResult)
{
	unsigned char ucResult;

	util_Printf("\n--------MAG_UerICCardFirstProcess()------------\n");
	util_Printf("ucCardSelect = %02x\n",ucCardSelect);
	util_Printf("ucFullBackFlag = %02x\n",ucFullBackFlag);
	util_Printf("ucLastResult = %02x\n",ucLastResult);
	util_Printf("\n------------------end----------------------\n");

	if(ucFullBackFlag)
		ucResult = SUCCESS;
	else if(ucCardSelect==CARDSPECIES_EMV)
	{
#if 1
		if(ucLastResult)
		{
				ucResult = ERR_ICCARD;
		}
#else
		if(ucResult)
			ucResult=ERR_CANCEL;
#endif
	}
	else
	{
		if(ucCardSelect==CARDSPECIES_MAG)
			ucResult=SUCCESS;
		else
			ucResult = ERR_CANCEL;
	}
	return ucResult;
}
unsigned char MAG_SetMBFEParam(unsigned char ucFlag)
{
    unsigned char ucChar,aucBuf[2];

    //Os__clr_display(255);
    if(!ucFlag)
        //Os__GB2312_display(0,0,(unsigned char *)"--前置转发设置--");

    MSG_DisplayMenuMsg(false,1,0,MSG_SUPERPASS);
    memset(aucBuf,0,sizeof(aucBuf));
    if (UTIL_Input(2,true,CASH_MAXSUPERPASSWDLEN,CASH_MAXSUPERPASSWDLEN,'P',aucBuf,0) != KEY_ENTER )
    {
        return(ERR_CANCEL);
    }
    else
    {
        if( memcmp(aucBuf,DataSave0.Cashier_SysCashier_Data.aucSuperPassword,CASH_MAXSUPERPASSWDLEN))
        {
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(unsigned char *)" 管理员密码错!");
            MSG_WaitKey(3);
            return(SUCCESS);
        }
    }
        //Os__clr_display(255);
        //Os__GB2312_display(0,0,(unsigned char *)"是否支持佐川");
        //Os__GB2312_display(1,0,(unsigned char *)"模式");
        //Os__GB2312_display(3,0,(unsigned char *)"1:是  0:否");
        memset(aucBuf,0,sizeof(aucBuf));
        aucBuf[0] = DataSave0.ConstantParamData.ucLogistics;
        if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
            return(ERR_CANCEL);
        else
        {
            ucChar = aucBuf[0];
            Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucLogistics,sizeof(unsigned char));
            XDATA_Write_Vaild_File(DataSaveConstant);
        }

        if (DataSave0.ConstantParamData.ucLogistics =='1')
        {
            ReaderSupport = false;
            DataSave0.ConstantParamData.ucLoadEMVParam='0';
            DataSave0.ConstantParamData.ucMBFEFlag ='1';
            DataSave0.ConstantParamData.ucORDERFLAG ='1';
            DataSave0.ConstantParamData.ucSendOrder ='1';
            DataSave0.ConstantParamData.ucEncryptDataFlag = '1';	//主IP是否加密
		     memcpy(&DataSave0.ConstantParamData.ulHostIPAddress1, (unsigned char *)"\x74\xE4\xEA\x23", 4);
		     DataSave0.ConstantParamData.uiHostPort=asc_long((unsigned char *)"009076",6);
		     memcpy(&DataSave0.ConstantParamData.ulHostIPGPRS1, (unsigned char *)"\xDE\x42\x01\x86", 4);
		     DataSave0.ConstantParamData.uiHostBackPort=asc_long((unsigned char *)"009076",6);
		     memcpy(&DataSave0.ConstantParamData.ulHostIPBack1,(unsigned char *)"\x74\xE4\xEA\x23", 4);
		     memcpy(&DataSave0.ConstantParamData.ulHostIPMIS1,(unsigned char *)"\xDE\x42\x01\x86", 4);
		     if (DataSave0.ConstantParamData.ucCommMode == PARAM_COMMMODE_GPRS)
		     {
		        memset(DataSave0.ConstantParamData.aucAPN,0,PARAM_APN);
		        memcpy(DataSave0.ConstantParamData.aucAPN,"CMNET",5);
		     }
		     XDATA_Write_Vaild_File(DataSaveConstant);
        }
        else
        {
            //Os__clr_display(255);
             //Os__GB2312_display(0,0,(unsigned char *)"是否支持前置转发");
            //Os__GB2312_display(3,0,(unsigned char *)"1:是  0:否");

            memset(aucBuf,0,sizeof(aucBuf));
            aucBuf[0] = DataSave0.ConstantParamData.ucMBFEFlag;
            if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
            	return(ERR_CANCEL);
            else
            {
            	ucChar = aucBuf[0];
            	Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucMBFEFlag,
            		sizeof(unsigned char));
               if (DataSave0.ConstantParamData.ucMBFEFlag=='0')
                    DataSave0.ConstantParamData.ucSendOrder='0';
            	XDATA_Write_Vaild_File(DataSaveConstant);
            }
            
             //Os__clr_display(255);
            //Os__GB2312_display(0,0,(unsigned char *)"是否支持陆交模式");
            //Os__GB2312_display(1,0,(unsigned char *)"客户名称打印");
            //Os__GB2312_display(3,0,(unsigned char *)"1:是  0:否");

            memset(aucBuf,0,sizeof(aucBuf));
            aucBuf[0] = DataSave0.ConstantParamData.uclujiaomc;
            if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
            	return(ERR_CANCEL);
            else
            {
            	ucChar = aucBuf[0];
            	Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.uclujiaomc,
            		sizeof(unsigned char));
            	XDATA_Write_Vaild_File(DataSaveConstant);
            }

            //Os__clr_display(255);
        	//Os__GB2312_display(0,0,(unsigned char *)"是否支持输入订单");
        	//Os__GB2312_display(1,0,(unsigned char *)"编号");
        	//Os__GB2312_display(3,0,(unsigned char *)"1:是  0:否");
        	memset(aucBuf,0,sizeof(aucBuf));
        	aucBuf[0] = DataSave0.ConstantParamData.ucORDERFLAG;
        	if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
        		return(ERR_CANCEL);
        	else
        	{
        		ucChar = aucBuf[0];
        		Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucORDERFLAG,
        			sizeof(unsigned char));
        		XDATA_Write_Vaild_File(DataSaveConstant);
        	}

        	if (DataSave0.ConstantParamData.ucMBFEFlag=='1')
        	{
        	       //Os__clr_display(255);
                	//Os__GB2312_display(0,0,(unsigned char *)"是否上送订单");
                	//Os__GB2312_display(1,0,(unsigned char *)"编号");
                	//Os__GB2312_display(3,0,(unsigned char *)"1:是  0:否");
                	memset(aucBuf,0,sizeof(aucBuf));
                	aucBuf[0] = DataSave0.ConstantParamData.ucSendOrder;
                	if( UTIL_Input(2,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
                		return(ERR_CANCEL);
                	else
                	{
                		ucChar = aucBuf[0];
                		Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucSendOrder,
                			sizeof(unsigned char));
                		XDATA_Write_Vaild_File(DataSaveConstant);
                	}
              }
        }

    return(SUCCESS);
}
unsigned char MAG_SetCollectData(void)
{
	unsigned char ucKey;
	unsigned char ucDispCh,aucBuf[12],ucChar;


	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"--数据采集设置--");
	MSG_DisplayMenuMsg(false,1,0,MSG_SUPERPASS);
	memset(aucBuf,0,sizeof(aucBuf));
	if (UTIL_Input(2,true,CASH_MAXSUPERPASSWDLEN,CASH_MAXSUPERPASSWDLEN,'P',aucBuf,0) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}else
	{
		if( memcmp(aucBuf,DataSave0.Cashier_SysCashier_Data.aucSuperPassword,CASH_MAXSUPERPASSWDLEN))
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,0,(unsigned char *)" 管理员密码错!");
			MSG_WaitKey(3);
			return(SUCCESS);
		}
	}
	//Set CollectData Funcation
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"是否支持");
	//Os__GB2312_display(1,0,(uchar *)"数据采集功能");
	//Os__GB2312_display(2,0,(uchar *)"1.支持  0.不支持");
	memset(aucBuf,0,sizeof(aucBuf));
	ucDispCh = 0x30;
	memcpy(aucBuf,&DataSave0.ConstantParamData.ucCollectFlag,1);
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
				Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucCollectFlag,
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
		aucBuf[0] = DataSave0.ConstantParamData.ucCollectFlag;
		//Os__GB2312_display(3,4,aucBuf);
		if(ucDispCh == 0x31)
			break;
	}
	if (DataSave0.ConstantParamData.ucCollectFlag=='1')
	{
		DataSave0.ConstantParamData.ucORDERFLAG='1';
		XDATA_Write_Vaild_File(DataSaveConstant);
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char*)"当前采集主机IP:");
		//Os__GB2312_display(3,0,(unsigned char*)"按[清除]键修改");
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,(unsigned char *)&DataSave0.ConstantParamData.ulSendIPAddress,4);
		if (OSUTIL_InputIPV4Addr(0,0x31,1,aucBuf) != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}
		else
		{
			memcpy(&DataSave0.ConstantParamData.ulSendIPAddress, aucBuf, 4);
			XDATA_Write_Vaild_File(DataSaveConstant);
		}

		//Os__clr_display(255);
		//Os__GB2312_display(0, 0, (unsigned char *)"采集主机端口:");
		memset(aucBuf,0,sizeof(aucBuf));
		int_asc(aucBuf,6,&DataSave0.ConstantParamData.uiSendPort);
		if (UTIL_Input(1,true,6,6,'N',aucBuf,0) != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}else
		{
			DataSave0.ConstantParamData.uiSendPort=asc_long(aucBuf,6);
			XDATA_Write_Vaild_File(DataSaveConstant);
		}

		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(unsigned char *)"采集主机APN:");
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,DataSave0.ConstantParamData.aucSendAPN,20);
		if(aucBuf[0] == 0x00)
		{
			memcpy(aucBuf,"CMNET",5);
		}
		if (UTIL_Input(1,true,5,20,'A',aucBuf,0) != KEY_ENTER )
		{
			return(ERR_CANCEL);
		}
		else
		{
			memset(DataSave0.ConstantParamData.aucSendAPN,0,20);
			memcpy(DataSave0.ConstantParamData.aucSendAPN,aucBuf,20);
			XDATA_Write_Vaild_File(DataSaveConstant);
		}
	}
	else
	{
		DataSave0.ConstantParamData.ucORDERFLAG='0';
		XDATA_Write_Vaild_File(DataSaveConstant);
	}
	return(SUCCESS);
}
unsigned char Menu_KEYNINE(void)
{
	unsigned char ucResult;
	unsigned char ucKey,ucPKey;

	Os__light_on();
	while(1)
	{
		ucResult = SUCCESS;
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"1.主管密码管理");
		//Os__GB2312_display(1,0,(uchar *)"2.终端设置");
		//Os__GB2312_display(2,0,(uchar *)"3.初始化");
		//Os__GB2312_display(3,0,(uchar *)"4.EMV参数信息");

		ucKey = UTIL_GetKey(30);
		switch(ucKey)
		{
			case '1':
				ucResult = SEL_SYSMenu();
				break;
			case '2':
				ucResult = MenuSys_TerminalSet();
				break;
			case '3':
				ucResult = SEL_ManagementMenu();
				COMMS_FinComm();
				break;
			case '4':
				ucResult=SEL_DisplayEMVInfo();
				break;
#if 0
			case '5':
				G_NORMALTRANS_ulAmount=1000000;
				G_NORMALTRANS_ucSourceAccLen=16;
				memcpy(G_NORMALTRANS_aucSourceAcc,"\x99\x09\x09\x09\x09\x09\x09\x09",8);
				G_NORMALTRANS_ucTransType=TRANS_PURCHASE;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
				memcpy(G_NORMALTRANS_aucAuthGroup,"CUP",3);
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_INSERTCARD;
				PRINT_Ticket(NORMAL_PRINT_FLAG,0);
				break;
#endif
			case '9'://远程密钥管理
		       	ucPKey = UTIL_GetKey(1);
				if (ucPKey=='6')
				{
				    ucResult = MAG_SetMBFEParam(0);
				    util_Printf("前置转发设置:%02x\n",ucResult);
				}else if (ucPKey=='7')
				{
					ucResult=MAG_SetCollectData();
					util_Printf("数据采集设置:%02x\n",ucResult);
				}
				break;
			default:
				break;
		}
		ucPKey=0;
		if(ucKey == KEY_CLEAR||ucKey==99)
			break;
		if ((ucResult != SUCCESS)&&(ucResult != ERR_CANCEL) &&(ucResult != ERR_END)&&(ucResult != 99))
		{
			MSG_DisplayErrMsg(ucResult);
		}
	}
	return (SUCCESS);
}

unsigned char MAG_InputCashierPass(void)
{
	unsigned char aucBuf[6];

	memset(aucBuf,0,sizeof(aucBuf));
	MSG_DisplayMenuMsg(true,0,0,MSG_SYS_CASHIERPASS);
	if (UTIL_Input(1,true,CASH_SYSCASHIERPASSLEN,CASH_SYSCASHIERPASSLEN,'P',aucBuf,0) != KEY_ENTER )
	{
		return(ERR_CANCEL);
	}
	else
	{
		util_Printf("DataSave0.Cashier_SysCashier_Data.aucSuperPassword = %s \n",DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass);
		if( memcmp(aucBuf,DataSave0.Cashier_SysCashier_Data.aucSYSCashierPass,CASH_SYSCASHIERPASSLEN))
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,1,(unsigned char *)"主管密码错");
			MSG_WaitKey(3);
			return(ERR_CANCEL);
		}
		else
		{
			return(SUCCESS);
		}
	}
	return(ERR_CANCEL);
}
unsigned char MAG_PutCard(void)
{
	unsigned char ucResult,ucI;
	int i;

	ucI = 0;
	do{
			DrvIn.type      = 0x6F;
			DrvIn.length	= 0;
			ucResult = SendReceivePinpad();
#ifdef UnTest
			util_Printf("返回:[%02x]\n",ucResult);
			for(i=0;i<6;i++)
				util_Printf("%02x ",DrvOut.xxdata[i]);
			util_Printf("\n");
#endif
			if (!ucResult)
			{
				if ((DrvOut.xxdata[0] == 0x00)
					&& DrvOut.xxdata[5] == 0x04)
				{
					break;
				}
				else
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar*)" (移动支付撤销)");
					//Os__GB2312_display(2,2,(uchar*)" 请放卡");
					ucResult = ERR_CANCEL;
				}
			}
			else
			{
				//Os__clr_display(255);
				//Os__GB2312_display(1,1,(uchar *)" 此卡不处理");
				//Os__GB2312_display(2,0,(uchar *)" 非CUPMOBILE卡");
				//Os__GB2312_display(3,2,(uchar *)" 请取卡");
				UTIL_GetKey(1);
				ucResult = ERR_END;
				break;
			}
			util_Printf("while:[%02x]\n",ucResult);
		}while(ucResult == ERR_CANCEL);
	util_Printf("移动撤销:%02x\n",ucResult);
	return(ucResult);
}
unsigned char MAG_ProUntouchCard(void)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucStatus,ucLen,aucOutData[300];

	if (G_NORMALTRANS_ucTransType == TRANS_VOIDCUPMOBILE)
	{
		ucResult = MAG_PutCard();
#ifdef UnTest
		util_Printf("MAG_PutCard:[%02x]\n",ucResult);
#endif
	}
#ifdef UnTest
	util_Printf("MAG_PutCard  OUT:[%02x]\n",ucResult);
#endif
	if (!ucResult)
		ucResult = MAG_ReadUnTouchCard();

	if (ucResult == SUCCESS)
	{
		memset(aucOutData,0,sizeof(aucOutData));
		ucResult = MAG_ProcessUntouch(&ucStatus,aucOutData,&ucLen);
#ifdef UnTest
		{
			unsigned char ucI;
			for(ucI=0;ucI<ucLen;ucI++)
				util_Printf("%02x ",aucOutData[ucI]);
			util_Printf("\n");
		}
#endif
		if (ucResult == SUCCESS)
		{
			ucResult = MAG_ReadFile();  //Read card seq
		}
#ifdef UnTest
		util_Printf("读文件:%02x\n",ucResult);
#endif
		if (ucResult == SUCCESS)
		{
			ucResult = TRANS_SetUntouch55Field();
		}
	}
	return(ucResult);
}
