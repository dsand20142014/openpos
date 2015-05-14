#include <include.h>
#include <global.h>
#include <xdata.h>

unsigned char EXTrans_ProHead_Ic(struct seven *EventInfo)
{
	unsigned char ucResult = 0x00;
	unsigned char ucKey;
	unsigned char EventType;
	unsigned char EventNum;
	unsigned char ucInputMode ,ucCardSelect;
	unsigned char ucICCardType,ucResetBufLen,aucResetBuf[50];
	unsigned short usI;
	uchar ucTransFlag=0;

	UTIL_ClearGlobalData();
	UTIL_GetTerminalInfo();

	util_Printf("\n 1.EXTrans_ProHead_Ic --------=%02x\n",ucResult);
	
	MAG_Read_LoadTransType();

	util_Printf("\n 2.EXTrans_ProHead_Ic --------=%02x\n",ucResult);
	
	if(DataSave0.ChangeParamData.ucTerminalLockFlag)
	{
		ucResult =UTIL_UnLockKeyBoard();
	}
	util_Printf("\n 3.EXTrans_ProHead_Ic --------=%02x\n",ucResult);
	
	if((ucResult)   
	||(DataSave0.ConstantParamData.ucSwiptEvent == 0x30)
	||(DataSave0.ConstantParamData.ucCashFlag == 0x31)
	||(UTIL_Check_BasetSet() == SUCCESS)
	)
		return ERR_TRANS_NOK;

	util_Printf("\n 4.EXTrans_ProHead_Ic --------=%02x\n",ucResult);
	
	ucResetBufLen = sizeof(aucResetBuf);
	//ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);

	util_Printf("\n 5.EXTrans_ProHead_Ic --------=%02x\n",ucICCardType);
	ucICCardType=ICC_ASY;
	if(ucICCardType!=ICC_ASY
	&&ucICCardType!=ICC_SYN)
	{
		G_NORMALTRANS_ucFallBackFlag =2;
		G_RUNDATA_ucDefOperation=true;
		ucResult=MASAPP_Event(true ,&ucKey);

		ucTransFlag = ERR_TRANS_OK;
	}
	else if(ucICCardType!=ICC_SYN)
	{
		G_RUNDATA_ucDefOperation=true;
		G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_INSERTCARD;
		G_NORMALTRANS_euCardSpecies=CARDSPECIES_EMV;
		ucTransFlag = ERR_TRANS_OK;
	}
	
	return ucResult;
}

unsigned char EXTrans_Process_IC(struct seven *EventInfo)
{
	unsigned char ucResult = 0x00;
	
	if(!ucResult)
	{
		ucResult=EXTrans_Process(0);
	}
	if((ucResult != SUCCESS)
		&&(ucResult != ERR_CANCEL)
		&&(ucResult != ERR_END))
	{
		MSG_DisplayErrMsg(ucResult);
	}
	Os__light_off();
	DispPINPAD_AD();
	UTIL_WaitPullOutICC();
	//EventInfo->pt_eve_out->treatment_status = OK;
	return 0;
}
unsigned char EXTrans_ProHead_Mag(struct seven *EventInfo)
{
	unsigned char ucResult = 0x00;
	unsigned char ucKey;
	unsigned char EventType;
	unsigned char EventNum;
	unsigned char ucInputMode ,ucCardSelect;
	unsigned char ucICCardType,ucResetBufLen,aucResetBuf[50];
	unsigned short usI;
	//FACEUITOPRO exPro;

	
	util_Printf("\n EXTrans_ProHead_Mag begin....[0001]");
	util_Printf("\n	1.MyThread ulAmount...[%d]", ProUiFace.UiToPro.ulAmount);
	util_Printf("\n	1.MyThread uiPwLen.....[%d]", ProUiFace.UiToPro.uiPwLen);
	util_Printf("\n	1.MyThread aucPasswd...[%s]", ProUiFace.UiToPro.aucPasswd);

	// ci chu hen guiyi-----
	//memset((char*)&exPro,0,sizeof(FACEUITOPRO));
	//memcpy((char*)&exPro, (char*)&ProUiFace.UiToPro, sizeof(FACEUITOPRO));

	UTIL_ClearGlobalData();	
	UTIL_GetTerminalInfo();
	MAG_Read_LoadTransType();
	// ci chu hen guiyi-----
    //memset((char*)&ProUiFace.UiToPro,0,sizeof(FACEUITOPRO));
    //memcpy((char*)&ProUiFace.UiToPro,(char*)&exPro,  sizeof(FACEUITOPRO));

	util_Printf("\n EXTrans_ProHead_Mag begin....[0002]");
	if(DataSave0.ChangeParamData.ucTerminalLockFlag)
	{
		ucResult =UTIL_UnLockKeyBoard();		
	}
	util_Printf("\n EXTrans_ProHead_Mag begin....[0002]");	
/*	if( (DataSave0.ConstantParamData.ucSwiptEvent == 0x30)
	|| (DataSave0.ConstantParamData.ucCashFlag == 0x31)
	|| (UTIL_Check_BasetSet() == SUCCESS)
	)
		ucResult = 0x03;
*/

	util_Printf("\n EXTrans_ProHead_Mag begin....[0003] [rst=%02x]",ucResult);		
	if(ucResult == 0x00){
		ucResult = MAG_EMVSetInputMode(&ucInputMode);
		if(ucResult)
		{
			MSG_DisplayErrMsg(ucResult);
		}
	}

	if(ucResult == 0x00){
        ucResult = MAG_CardSelect(ucInputMode , &ucCardSelect , ProUiFace.UiToPro.CardInfo.aucCardAcc);
		if (ucResult == ERR_SANDMAGCARD)
		{
			EventInfo->pt_eve_out->treatment_status = NOK;   
			return 0x03;         
		}
	}	
	
	if(ucResult == 0x00){
		if(ucResult==ERR_USEICCARDFIRST
		&&ucCardSelect==CARDSPECIES_EMV
		&&(ucInputMode&TRANS_INPUTMODE_INSERTCARD))
		{
			ucResult = MASAPP_WaitReadIC(ucInputMode );
			if(ucResult == SUCCESS)
			{
				G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_euCardSpecies = CARDSPECIES_EMV;
			}
			else if (ucResult != ERR_CANCEL)             /*插卡后失败做磁条卡fallback*/
			{
				G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_SWIPECARD;
				G_NORMALTRANS_euCardSpecies = CARDSPECIES_MAG;
				G_NORMALTRANS_ucFallBackFlag =2;
				ucResult =ERR_FALLBACK;
				ucResult = MAG_DispCardNo();
			}
		}
		util_Printf("\n EXTrans_ProHead_Mag begin....[0006] [rst=%02x]",ucResult);				
		if((ucResult == ERR_USEICCARDFIRST||ucResult ==SUCCESS)
			&&(ucCardSelect == CARDSPECIES_MAG)
			&&(G_NORMALTRANS_ucInputMode==TRANS_INPUTMODE_SWIPECARD))
		{
			ucResult = MAG_DispCardNo();
		}
		
		util_Printf("\n EXTrans_ProHead_Mag begin....[0007] [rst=%02x]",ucResult);				
		util_Printf("manager=%02x\n",ucResult);

		
		if((ucResult==SUCCESS&&G_RUNDATA_ucReadCardFlag == EMVFIRSTMAG)
			||(ucResult==SUCCESS)
			||(ucResult==ERR_FALLBACK)
		)
		{
		util_Printf("\n EXTrans_ProHead_Mag ....[0007.001] [rst=%02x]",ucResult);				
			if(G_RUNDATA_ucReadCardFlag==EMVFIRSTMAG)
				G_NORMALTRANS_ucFallBackFlag =2;
			
			G_RUNDATA_ucDefOperation=true;
			ucResult=EXTrans_Process(0);
			
		util_Printf("\n EXTrans_ProHead_Mag ....[0007.002] [rst=%02x]\n",ucResult);				
		}
	}

	util_Printf("\n EXTrans_ProHead_Mag begin....[0008] [rst=%02x]\n",ucResult);			
	if((ucResult != SUCCESS)
		&&(ucResult != ERR_END)
		&&(ucResult != ERR_CANCEL)
		&&(ucResult != ERR_USEICCARDFIRST))
	{
		MSG_DisplayErrMsg(ucResult);
	}
	util_Printf("\n=====EXTrans_ProHead_Mag=======End.\n");
	
	//Os__light_off();
	//EventInfo->pt_eve_out->treatment_status = OK;
	UTIL_WaitPullOutICC();

	util_Printf("\n=====EXTrans_ProHead_Mag=======End.\n");

	return 0x04;
}

unsigned char EXTrans_Process(unsigned char Flag)
{
	unsigned char	ucResult,ucI,ucResult1,ucCopy;
	unsigned long	ulAmount;
	unsigned long	AuthLen;
	unsigned char	aucBuf[200],DateTimeBuf[13];
	short iLen;
	unsigned char ucInputFlag;
	unsigned char aucRefNoBuf[13];
	unsigned char ucTransType,ucInputType,ucNeedInputPIN;
	unsigned char ucKey;
	unsigned char	 ucPindata[8], ucPinlen;
	EXTRATRANS    ExtraTransOnline;
	DRV_OUT  aucDRVOut;

	util_Printf("\n EXTrans_Process ....[EX-0007.000]");
	//Os__light_on();
/*	if (DataSave0.ConstantParamData.ucSendCollectFlag=='1')
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"请先上送采集数据");
		UTIL_GetKey(5);
		return(ERR_CANCEL);
	}

	util_Printf("交易参数:\n交易類型:[%02x] 輸入模式:[%02x] 卡種:[%02x]\n",G_NORMALTRANS_ucTransType,G_NORMALTRANS_ucInputMode,G_NORMALTRANS_euCardSpecies);
	util_Printf("流水AAA [%d]\n",G_NORMALTRANS_ulTraceNumber);
	if (G_NORMALTRANS_ucInputMode != TRANS_INPUTMODE_PUTCARD)
	{
		if(UTIL_Check_BasetSet() == SUCCESS)    //检查终端状态
		{
			if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
				return(DataSave0.ChangeParamData.ucResult);
			else
				return SUCCESS;
		}
	}*/
	ucResult = SUCCESS;
	G_RUNDATA_ucTransType=G_NORMALTRANS_ucTransType;

	if((ucResult = SAV_CheckTransMaxNb()) != SUCCESS)		//检查交易是否已满
		return(ucResult);
	
	util_Printf("\n EXTrans_Process ....[EX-0007.001] [rst=%02x]",ucResult);

    	/*Cashier[柜员]*/
	memcpy(G_NORMALTRANS_aucCashierNo,
		DataSave0.Cashier_SysCashier_Data.aucCashierNo[DataSave0.ChangeParamData.ucCashierLogonIndex],
			CASH_CASHIERNOLEN);

	if(  (G_NORMALTRANS_ucTransType != TRANS_TIPADJUST)
	   &&(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)
	   &&(G_NORMALTRANS_ucTransType != TRANS_UNDOOFF)
	   &&(G_NORMALTRANS_ucTransType != TRANS_REFUND)
	  )
	{
		COMMS_PreComm();
		ucTransType=G_NORMALTRANS_ucTransType;
		util_Printf("\nDataSave0.ChangeParamData.ucDownLoadFlag=%d",DataSave0.ChangeParamData.ucDownLoadFlag);
		if( DataSave0.ChangeParamData.ucDownLoadFlag==1 )
		{
			G_NORMALTRANS_ucTransType=TRANS_LOADPARAM;
			ucResult=LOGON_Online();
			if( ucResult==SUCCESS )
			{
				DataSave0.ChangeParamData.ucDownLoadFlag=0;
				XDATA_Write_Vaild_File(DataSaveChange);
			}
		}
		G_NORMALTRANS_ucTransType=ucTransType;
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.002] [rst=%02x]",ucResult);

	/* Input Super Password 主管密码*/
	if( ucResult == SUCCESS )
	{
		switch( G_NORMALTRANS_ucTransType )
		{
		case TRANS_CREDITSVOID:
		case TRANS_VOIDPURCHASE:
			if(DataSave0.ConstantParamData.ucUnManagePwd=='1')
			{
				ucResult = CASH_InputAndChackManagerPass(0);
				if(ucResult != SUCCESS)
					return ucResult;
			}
			break;
		case TRANS_REFUND:
		case TRANS_UNDOOFF:
		case TRANS_CREDITSREFUND:
		case TRANS_VOIDPREAUTH:
		case TRANS_VOIDCUPMOBILE:
		case TRANS_VOIDMOBILEAUTH:
		case TRANS_VOIDPREAUTHFINISH:
			ucResult = CASH_InputAndChackManagerPass(0);
			if(ucResult != SUCCESS)
				return ucResult;
			break;
		default:
			break;
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.003] [rst=%02x]",ucResult);

	/* Input Original Trace Number*/
	if( ucResult == SUCCESS && !Flag)
	{
		switch( G_NORMALTRANS_ucTransType )
		{
			case TRANS_UNDOOFF:
			case TRANS_TIPADJUST:
			case TRANS_CREDITSVOID:
			case TRANS_VOIDPURCHASE:
			case TRANS_VOIDCUPMOBILE:
			case TRANS_VOIDMOBILEAUTH:
			case TRANS_VOIDPREAUTHFINISH:
				if((DataSave0.ConstantParamData.ucCashFlag == 0x31)&&(G_NORMALTRANS_ucTransType ==TRANS_VOIDPURCHASE ))
				  break;
				MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
				//Os__GB2312_display(1, 0, (uchar * )"请输入原凭证号:");
				if( UTIL_Input(2,true,1,TRANS_TRACENUMLEN,'N',G_RUNDATA_aucOriginalTraceNum,NULL) != KEY_ENTER )
				{
					return(ERR_CANCEL);
				}
				break;
	       	 default:
				break;
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.004] [rst=%02x]",ucResult);

	/* Search old trans or find new trans location */
	if( ucResult == SUCCESS )  //查询存储空间
	{
		switch( G_NORMALTRANS_ucTransType )
		{
		case TRANS_QUERYBAL:
			break;
		case TRANS_UNDOOFF://离线撤消
			ucResult = UTIL_Is_Trans_Empty();
			util_Printf("\nucResult=%02x\n",ucResult);
			if(ucResult != SUCCESS)
				return ucResult;
			ucResult=SAV_SavedTransIndex(3);
			util_Printf("SAV_SavedTransIndex(3)---ucResult=%02x",ucResult);
			break;
		case TRANS_VOIDPURCHASE:
		case TRANS_CREDITSVOID:
		case TRANS_VOIDCUPMOBILE:
		case TRANS_VOIDMOBILEAUTH:
		case TRANS_VOIDPREAUTHFINISH:
			ucResult =  SAV_SavedTransIndex(1);
			if(ucResult == SUCCESS)
			{
				ucResult =  SAV_SavedTransIndex(0);		/*将撤销交易作为单独交易存储*/
			}
			break;
		case TRANS_TIPADJUST:
			ucResult =  SAV_SavedTransIndex(2);
			util_Printf("返回值:%02x\n",ucResult);
			if(ucResult == SUCCESS)
			{
				if(G_NORMALTRANS_ucOldTransType == TRANS_OFFPREAUTHFINISH)
				{
					/*不保留小费额*/
					if(DataSave0.TransInfoData.auiTransIndex[G_RUNDATA_uiOldTransIndex]==TRANS_NIINORMAL)
					{
						G_RUNDATA_ucTIPSaving = 0;		/*离线交易未上送调整，覆盖原交易存放*/
						G_NORMALTRANS_ulTraceNumber = G_NORMALTRANS_ulOldTraceNumber;
					}else
					{
						G_RUNDATA_ucTIPSaving = 1;		/*离线已上送调整，产生新的交易存放*/
						ucResult =  SAV_SavedTransIndex(0);
					}
				}else
			 	{
			 		if(!DataSave0.ConstantParamData.ucXorTipFee)
			 		{
						return ERR_TRANSNOTSUP;
			 		}
					/*保留小费额*/
					G_RUNDATA_ucTIPSaving = 2;		/*其他交易小费，产生新的交易存放*/
					ucResult =  SAV_SavedTransIndex(0);
				}
			}
			break;
		default:
			ucResult =  SAV_SavedTransIndex(0);
			break;
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.005] [rst=%02x]",ucResult);

	if (!ucResult && DataSave0.ConstantParamData.ucORDERFLAG == 0x31)//订单号
	{
		ucResult =Trans_InputOrder();
		if (ucResult)
			return(ucResult);
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.006] [rst=%02x]",ucResult);

	if(!ucResult && G_NORMALTRANS_ucTransType==TRANS_EC_UNASSIGNLOAD)
	{
		  ucResult=MASAPP_SwipeCard();/*转出卡输入*/
		  if(!ucResult)
		  	ucResult = UTIL_InputEncryptPIN_EC(ucPindata ,&ucPinlen);
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.007] [rst=%02x]",ucResult);

	//读取片卡信息
	if( (ucResult == SUCCESS)
		&&(G_NORMALTRANS_ucTransType !=TRANS_UNDOOFF)
		&&(G_NORMALTRANS_ucTransType != TRANS_TIPADJUST)
		&&(G_NORMALTRANS_ucTransType != TRANS_CUPMOBILE)
		&&(G_NORMALTRANS_ucTransType != TRANS_VOIDCUPMOBILE)
		&&(G_NORMALTRANS_ucTransType != TRANS_MOBILEAUTH)
		&&(G_NORMALTRANS_ucTransType != TRANS_VOIDMOBILEAUTH)
	  )
	{
		if( ( (G_NORMALTRANS_ucTransType == TRANS_VOIDPURCHASE)
				&&(DataSave0.ConstantParamData.ucUnSaleSwipe=='0'))
		  	||((G_NORMALTRANS_ucTransType == TRANS_VOIDPREAUTHFINISH)
		  		&&(DataSave0.ConstantParamData.ucUnAuthFinSwipe=='0'))
		  	||((G_NORMALTRANS_ucTransType == DataSave0.ConstantParamData.ucDefaultTransParam)
		  	   	&&(G_RUNDATA_ucDefOperation))
		  )
		{
			util_Printf("\n标志1G_RUNDATA_ucDefOperation=%02x\n",G_RUNDATA_ucDefOperation);
		}
		else
		{
			util_Printf("\n标志2=%02x\n",G_RUNDATA_ucDefOperation);

			if(((G_NORMALTRANS_ucTransType == TRANS_PURCHASE)
				||(G_NORMALTRANS_ucTransType == TRANS_QUERYBAL))
			&&(ReaderSupport)&&(ReaderType==READER_HONGBAO))
			{
				util_Printf("\nHBReader");
				if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
				    ucResult=UTIL_EMVInputTransAmount();//非接交易先输金额
				if(ucResult == SUCCESS )
				{
					memset(&aucDRVOut,0,sizeof(DRV_OUT));
					if(G_NORMALTRANS_ucECTrans==TRANS_Q_PURCHASE)
					{
						ucInputType=0x04;
						MAG_DisplaySwipeCardMode(G_NORMALTRANS_ucTransType,0x18);
					}
					else
					{
						ucInputType=0x07;
						MAG_DisplaySwipeCardMode(G_NORMALTRANS_ucTransType,0x15);
					}

					memset((unsigned char *)&InterfaceTransData,0,sizeof(InterfaceTransData));
                		      INTERFACE_TransToReader(G_NORMALTRANS_ulAmount,&InterfaceTransData,&aucDRVOut,&ucInputType);
                					util_Printf("\nRead返回刷卡模式ucInputType= %02x\n",ucInputType);
                        	if(((ucInputType== 0x00)||(ucInputType== 0x04)||(ucInputType== 0x07))
                        		&&(InterfaceTransData.unTransResult!=SUCCESS))//IC卡和磁卡不要判断
                        	{
                        		ucResult = InterfaceTransData.unTransResult;
                        		util_Printf("非接卡器返回ucResult= %02x\n",ucResult);
                        	}
                        	else
                        	{
                        		//BIT 0  IC;  BIT 1  mag ; BIT 2 interface
                	        	util_Printf("\nINTERFACE_TransToReader ucInputType=%02x",ucInputType);
                	        	if(ucInputType == 0x01)	     //IC卡
                	        		ucResult =MASAPP_External_Event(&aucDRVOut,0x01,&ucKey);
                	        	else if(ucInputType == 0x02) //磁卡
                	        		ucResult =MASAPP_External_Event(&aucDRVOut,0x02,&ucKey);
                	        	else if(ucInputType == 0x04) //非接卡
                	        	{
                	        		util_Printf("\nInterfaceTransData.bCheckOnlinePin =%d",InterfaceTransData.bCheckOnlinePin);
                	        		if((InterfaceTransData.bCheckOnlinePin == 0)&&(G_NORMALTRANS_ucTransType != TRANS_QUERYBAL))
                	        			ucNeedInputPIN =0;
                	        		else
                	        			ucNeedInputPIN =1;
                	        		if(InterfaceTransData.bForceOnline == 0)
                	        			COMMS_FinComm();
                	        		util_Printf("\nInterfaceTransData.bForceOnline=abc=%d",InterfaceTransData.bForceOnline);
                	        		ucResult = MASAPP_UnTouchCard_Event();
                	        		util_Printf("\nMASAPP_UnTouchCard_Event=%d",ucResult);
            		     		}
            		     		else
            				{
            					ucResult = MASAPP_Event(true ,&ucKey);
            					if(ucResult!=SUCCESS)
            						return(ucResult);
            				}
                	     	}
			    }
			}
			#if SANDREADER
			else if((G_NORMALTRANS_ucTransType == TRANS_PURCHASE
				|| G_NORMALTRANS_ucTransType == TRANS_CREDITSALE
				|| G_NORMALTRANS_ucTransType == TRANS_QUERYBAL)
			&&(ReaderSupport)&&(ReaderType==READER_SAND))
			{
				util_Printf("\nSANDReader");
				if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
				    ucResult=UTIL_EMVInputTransAmount();    //非接交易先输金额

				if ((!ucResult)&&(G_NORMALTRANS_ucTransType == TRANS_CREDITSALE))
				{
					ucResult=Trans_InputNumOfStages();
					if (ucResult)
						return(ucResult);
				}

				if(!ucResult)
				{
					ucResult = MASAPP_Event_SAND(true ,&ucKey);
					util_Printf("\nMASAPP_Event_SAND=%d",ucResult);
				}
				if(ucResult)
				    return(ucResult);
			}
			#endif
			else
			{
				ucResult = MASAPP_Event(true ,&ucKey);
				if(ucResult!=SUCCESS)
					return(ucResult);
			}
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.008] [rst=%02x]",ucResult);

	if(ucResult==SUCCESS
		&&G_NORMALTRANS_ucTransType == TRANS_OFFPREAUTHFINISH)
	{
		ucResult=Trans_OFFLineTrans();
		util_Printf("离线结算ucResult2=%02x\n",ucResult);
		if(ucResult!=SUCCESS)
			return(ucResult);
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.009] [rst=%02x]",ucResult);

	//手机预约消费
	if (!ucResult && G_NORMALTRANS_ucTransType==TRANS_MOBILEAUTH)
	{
		ucResult=Trans_MobileTProcess();
		if (ucResult){
			COMMS_FinComm();
			return(ucResult);
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.010] [rst=%02x]",ucResult);

	/* Void trans & tipadjust trans */
	if(ucResult == SUCCESS )									//输入原参考号
	{
		switch( G_NORMALTRANS_ucTransType )
		{
			case TRANS_REFUND:
				if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
					  break;
				MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
				//Os__GB2312_display(1, 0, (uchar * )"请输入原参考号:");

				ucInputFlag = PY_INPUT_NUMBER|PY_INPUT_UPPER_CASE;
				memset(aucRefNoBuf,0,sizeof(aucRefNoBuf));
				memcpy(aucRefNoBuf,G_NORMALTRANS_aucOldRefNumber,12);
				ucResult=PY_Main(aucRefNoBuf,2,TRANS_REFNUMLEN,TRANS_REFNUMLEN,ucInputFlag,300);
				if(ucResult!=SUCCESS)
				{
					return(ERR_CANCEL);
				}
				else
				{
					memcpy(G_NORMALTRANS_aucOldRefNumber,aucRefNoBuf,12);
					util_Printf("\naucRefNoBuf=%s\n",aucRefNoBuf);
					util_Printf("G_NORMALTRANS_aucOldRefNumber=%s\n",G_NORMALTRANS_aucOldRefNumber);
				}
        		break;
			default:
				break;
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.011] [rst=%02x]",ucResult);
	/*Input Original Data*/
	if( ucResult == SUCCESS )								//输入原交易日期
	{
		switch( G_NORMALTRANS_ucTransType )
		{
			case TRANS_REFUND:
			case TRANS_VOIDPREAUTH:
			case TRANS_PREAUTHFINISH:
			case TRANS_PREAUTHSETTLE:
		 	if(DataSave0.ConstantParamData.ucCashFlag != 0x31)
		 	{
				memset(DateTimeBuf ,0 ,sizeof(DateTimeBuf));
				MSG_DisplayMsg( true,0,0,MSG_TRANSDATE);
				//Os__GB2312_display(2, 0, (uchar * )"(月月日日):");
				if( UTIL_Input(3,true,TRANS_DATELEN,TRANS_DATELEN,'N',DateTimeBuf,NULL) != KEY_ENTER )
				{
					return(ERR_CANCEL);
				}
				asc_bcd(&G_NORMALTRANS_aucOldDate[2], 2, DateTimeBuf, 4);
		 	}
			G_NORMALTRANS_ulOldTraceNumber = G_NORMALTRANS_ulTraceNumber;
			break;
	     default:
			break;
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.011] [rst=%02x]",ucResult);
	/* Input AuthCode */
	if( ucResult == SUCCESS )
	{
		switch( G_NORMALTRANS_ucTransType )
		{
			case TRANS_PREAUTHFINISH:
			case TRANS_PREAUTHSETTLE:
			case TRANS_VOIDPREAUTH:
			case TRANS_PREAUTHADD:
        		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
				//Os__GB2312_display(1, 0, (uchar * )"请输入原授权码:");

				ucInputFlag = PY_INPUT_NUMBER|PY_INPUT_UPPER_CASE;

				ucResult=PY_Main(G_NORMALTRANS_aucAuthCode,2,2,TRANS_AUTHCODELEN,ucInputFlag,300);
				util_Printf("ucResult=%02x\n",ucResult);

				if(ucResult!=SUCCESS)
				{
					return(ERR_CANCEL);
				}else
				{
	        		memcpy(G_NORMALTRANS_aucOldAuthCode,G_NORMALTRANS_aucAuthCode,6);
					util_Printf("G_NORMALTRANS_aucOldAuthCode=%s,\nG_NORMALTRANS_aucAuthCode=%s\n",G_NORMALTRANS_aucOldAuthCode,G_NORMALTRANS_aucAuthCode);
				}
				AuthLen = strlen((char*)G_NORMALTRANS_aucOldAuthCode);
				util_Printf("原授权号长度= %d\n",AuthLen);
				util_Printf("补[%d]空格\n",(6-AuthLen));
				if(AuthLen < 6)
				{
					memset(&G_NORMALTRANS_aucOldAuthCode[AuthLen],' ',(6-AuthLen));
				}
				util_Printf("原授权号 =%s\n",G_NORMALTRANS_aucOldAuthCode);
		        	break;
        	default:
				break;
		}
	}

	util_Printf("\n EXTrans_Process ....[EX-0007.012] [rst=%02x]",ucResult);

	/*输入交易金额*/
	if( (ucResult == SUCCESS)
		&& (G_NORMALTRANS_ucTransType != TRANS_CUPMOBILE )
		&& (G_NORMALTRANS_ucTransType != TRANS_MOBILEAUTH ))
	{
		 if((DataSave0.ConstantParamData.ucCashFlag == 0x31)&&((G_NORMALTRANS_ucTransType == TRANS_PURCHASE)
			  	||(G_NORMALTRANS_ucTransType == TRANS_REFUND)
				||(G_NORMALTRANS_ucTransType == TRANS_CREDITSREFUND)))
		{
			    ucResult = UTIL_CheckTransAmount();
		}
		if(G_NORMALTRANS_ulAmount==0||G_NORMALTRANS_ucTransType==TRANS_TIPADJUST)
		{
			if (DataSave0.ConstantParamData.ucCashFlag == 0x31)
			{
				return (ERR_NOZEOR);
			}
			ucResult=UTIL_EMVInputTransAmount();
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.013] [rst=%02x]",ucResult);
	
	#if 1
	if((ucResult == SUCCESS)&&(G_NORMALTRANS_ucTransType == TRANS_REFUND))
	{
		util_Printf("退货前\n");
		ucTransType = G_NORMALTRANS_ucTransType;
		G_NORMALTRANS_ucTransType = TRANS_LOGONON;
		ucResult = LOGON_CashierProcess(1);
		util_Printf("退货前需签到[%02x]\n",ucResult);
		if (ucResult != SUCCESS)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,2,(unsigned char *)"交易失败");
			//Os__GB2312_display(2,1,(unsigned char *)"请重新签到!");
			UTIL_GetKey(2);
			return (ERR_CANCEL);
		}
		G_NORMALTRANS_ucTransType = ucTransType;
		G_RUNDATA_ucHostConnectFlag = true;

		util_Printf("预拨号:%02x\n",G_RUNDATA_ucDialFlag);
	}
	#endif
	util_Printf("\n EXTrans_Process ....[EX-0007.014] [rst=%02x]",ucResult);
	
	if(  (ucResult == SUCCESS)
		&&(( (G_NORMALTRANS_ucTransType == TRANS_VOIDPURCHASE)
			&&(DataSave0.ConstantParamData.ucUnSaleSwipe=='0'))
	  	||((G_NORMALTRANS_ucTransType == TRANS_VOIDPREAUTHFINISH)
	  		&&(DataSave0.ConstantParamData.ucUnAuthFinSwipe=='0'))
	  ))
	{
		G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_MANUALINPUT;
	}

#if 1
	util_Printf("[+读卡方式 --qq--= %02x\n",G_NORMALTRANS_ucInputMode);
	util_Printf("卡类型 --qq--= %02x\n",G_NORMALTRANS_euCardSpecies);
	util_Printf("读卡标识 --qq--= %02x\n",G_RUNDATA_ucReadCardFlag);
	util_Printf("交易类型 --qq--= %02x\n",G_NORMALTRANS_ucTransType);
	util_Printf("处理结果:%d+]",ucResult);
#endif
	util_Printf("\n EXTrans_Process ....[EX-0007.015] [rst=%02x]",ucResult);

	//银联芯片卡处理
	if( ucResult == SUCCESS
		&&G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV
		&&(G_NORMALTRANS_ucInputMode!=TRANS_INPUTMODE_HB_PUTCARD)
		&&(G_NORMALTRANS_ucInputMode!=TRANS_INPUTMODE_SAND_PUTCARD))//	&&G_NORMALTRANS_ucInputMode==TRANS_INPUTMODE_INSERTCARD
	{
		util_Printf("\nG_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV");
		if(G_RUNDATA_ucReadCardFlag == EMVFIRSTMAG)
		{
			util_Printf("\nG_RUNDATA_ucReadCardFlag == EMVFIRSTMAG");
			memcpy(&ExtraTransOnline ,&ExtraTransData, sizeof(EXTRATRANS));
		}
		if(G_NORMALTRANS_ucTransType == TRANS_QUERYBAL
			||G_NORMALTRANS_ucTransType == TRANS_PURCHASE
			||G_NORMALTRANS_ucTransType == TRANS_PREAUTH
			||G_NORMALTRANS_ucTransType == TRANS_PREAUTHFINISH
			||G_NORMALTRANS_ucTransType == TRANS_EC_ASSIGNLOAD
			||G_NORMALTRANS_ucTransType == TRANS_EC_UNASSIGNLOAD
			||G_NORMALTRANS_ucTransType == TRANS_EC_CASHLOAD
			||G_NORMALTRANS_ucTransType == TRANS_CREDITSALE
			)/*判断交易类型*/
			{
				util_Printf("流水WWW [%d]\n",G_NORMALTRANS_ulTraceNumber);
				ucResult = EMVTransProcess();/*ICC卡EMV处理*/
				util_Printf("EMV=%02x\n",ucResult);
			}
		else
		if(G_NORMALTRANS_ucTransType == TRANS_VOIDPURCHASE
			||G_NORMALTRANS_ucTransType == TRANS_VOIDPREAUTHFINISH
			||G_NORMALTRANS_ucTransType == TRANS_REFUND
			||G_NORMALTRANS_ucTransType == TRANS_VOIDPREAUTH
			||G_NORMALTRANS_ucTransType == TRANS_PREAUTHSETTLE
			||G_NORMALTRANS_ucTransType==TRANS_OFFPREAUTHFINISH
			||G_NORMALTRANS_ucTransType==TRANS_PREAUTHADD
			||G_NORMALTRANS_ucTransType==TRANS_CREDITSREFUND
			||G_NORMALTRANS_ucTransType==TRANS_CREDITSVOID
			)
		{
			if(G_NORMALTRANS_ucInputMode==TRANS_INPUTMODE_INSERTCARD)/**/
				ucResult = EMV_SimpleProcess();
			if(!ucResult)
			{
				switch( G_NORMALTRANS_ucTransType )
				{
					case TRANS_CREDITSVOID:
					case TRANS_VOIDPURCHASE:
					case TRANS_VOIDPREAUTH:
					case TRANS_VOIDPREAUTHFINISH:
						if(DataSave0.ConstantParamData.ucVoidInputPin=='1')
						{
							ucResult = UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
							if(ucResult==EMVERROR_BYPASS)
								ucResult =SUCCESS;
						}
						break;
					default:
						break;
				}
			}
			if(!ucResult&&G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)
			{
				ucResult = TRANS_OnlineProcess();
			}
		}
		util_Printf("FallBack＝%02x %02x\n",G_NORMALTRANS_ucFallBackFlag,ucResult);
		if(G_NORMALTRANS_ucTransType == TRANS_EC_ASSIGNLOAD
			||G_NORMALTRANS_ucTransType == TRANS_EC_UNASSIGNLOAD
			||G_NORMALTRANS_ucTransType == TRANS_EC_CASHLOAD
			)
			G_NORMALTRANS_ucFallBackFlag =0;
		if(G_NORMALTRANS_ucFallBackFlag)
		{
			if(G_RUNDATA_ucReadCardFlag != EMVFIRSTMAG)
				ucResult=MASAPP_Event(true ,&ucKey);
			else
			{
				memcpy(&ExtraTransData, &ExtraTransOnline ,sizeof(EXTRATRANS));
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
				ucResult = SUCCESS ;
			}
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.016] [rst=%02x]",ucResult);

#if 1
	util_Printf("\n=============流程结果========%02x\n",ucResult);
	util_Printf("读卡方式= %02x\n",G_NORMALTRANS_ucInputMode);
	util_Printf("卡类型 --qq2--= %02x\n",G_NORMALTRANS_euCardSpecies);
	util_Printf("读卡标识 --qq2--= %02x\n",G_RUNDATA_ucReadCardFlag);
	util_Printf("交易类型 --qq2--= %02x\n",G_NORMALTRANS_ucTransType);
#endif

    //银联非接卡处理
	if(( ucResult == SUCCESS&&G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV)
		&&(G_NORMALTRANS_ucInputMode==TRANS_INPUTMODE_HB_PUTCARD))
	{
		if(ucNeedInputPIN ==1)
			ucResult = UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
		if((!ucResult)&&(InterfaceTransData.bForceOnline !=0x00))
			ucResult = TRANS_OnlineProcess();
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.017] [rst=%02x]",ucResult);

	//银联非接卡处理，杉德读卡器(处理信用支付交易)
	if(( ucResult == SUCCESS&&G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV)
		&&(G_NORMALTRANS_ucInputMode==TRANS_INPUTMODE_SAND_PUTCARD)
		&&(G_NORMALTRANS_ucTransType==TRANS_CREDITSALE
				||G_NORMALTRANS_ucTransType==TRANS_CREDITSVOID))
	{
		ucPinlen=0;
		memset(ucPindata,0,sizeof(ucPindata));
		ucResult = UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
		if (!ucResult)
			ucResult = TRANS_OnlineProcess();
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.018] [rst=%02x]",ucResult);

	//银联磁条卡处理
	if( (ucResult == SUCCESS )
		&&(G_NORMALTRANS_euCardSpecies!=CARDSPECIES_EMV)
		&&(G_NORMALTRANS_euCardSpecies!=CARDSPECIES_UNTOUCH)
		)
	{
		switch( G_NORMALTRANS_ucTransType )
		{
		case TRANS_QUERYBAL:
		case TRANS_PURCHASE:
		case TRANS_PREAUTH:
		case TRANS_CREDITSALE:
		case TRANS_MOBILEAUTH:		//手机预约消费
			ucResult = UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
			if(!ucResult)
			{
				ucResult = TRANS_OnlineProcess();
			}
			break;
		case TRANS_UNDOOFF:
		case TRANS_TIPADJUST:
		case TRANS_OFFPREAUTHFINISH:
			break;
		default:
			if(G_NORMALTRANS_ucOldTransType==TRANS_OFFPURCHASE)
			{
				G_NORMALTRANS_ucTransType=TRANS_VOIDOFFPURCHASE;
			}
			switch( G_NORMALTRANS_ucTransType )
			{
				case TRANS_PURCHASE:
				case TRANS_TIPADJUST:
				case TRANS_OFFPREAUTHFINISH:
				case TRANS_REFUND:
				case TRANS_PREAUTHSETTLE:
					break;
				case TRANS_CREDITSVOID:
				case TRANS_VOIDPURCHASE:
				case TRANS_VOIDPREAUTH:
				case TRANS_VOIDMOBILEAUTH:
				case TRANS_VOIDPREAUTHFINISH:
					if(DataSave0.ConstantParamData.ucVoidInputPin=='1')
						ucResult = UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
					break;
				case TRANS_PREAUTHFINISH:
					if( DataSave0.ConstantParamData.ucPREAUTHFINISHInputPin=='1')
						ucResult = UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
					break;
				default:
					ucResult = UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
					break;
			}
			if(!ucResult)
			{
				ucResult = TRANS_OnlineProcess();
			}
			if(ucResult == SUCCESS)
			{
				memset(aucBuf,0,sizeof(aucBuf));
				ucResult = ISO8583_GetBitValue(4,aucBuf,&iLen,sizeof(aucBuf));
				if( ucResult == SUCCESS )
				{
					ulAmount = asc_long(aucBuf,12);

					if(G_NORMALTRANS_ucOldTransType != TRANS_OFFPREAUTHFINISH)
					{
						if(ulAmount != G_NORMALTRANS_ulAmount)
							ucResult = ERR_AMOUNTERR;
					}
					else
						if(ulAmount != (G_NORMALTRANS_ulFeeAmount + G_NORMALTRANS_ulAmount) )
							ucResult = ERR_AMOUNTERR;
				}
			}
			break;
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.019] [rst=%02x]",ucResult);

	//银联手机芯片卡处理
	if ((ucResult == SUCCESS)
		&&(G_NORMALTRANS_euCardSpecies == CARDSPECIES_UNTOUCH))
	{
		util_Printf("非接触交易参数:類型:%02x 模式:%02x 卡種:%02x\n",G_NORMALTRANS_ucTransType,G_NORMALTRANS_ucInputMode,G_NORMALTRANS_euCardSpecies);
		switch( G_NORMALTRANS_ucTransType )
		{
			case TRANS_CUPMOBILE:
				ucResult = UTIL_InputMobileAmount();
				if (ucResult == SUCCESS)
				{
					memset(ucPindata,0,sizeof(ucPindata));
					ucResult = UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
					if (ucResult == SUCCESS)
					{
						ucResult = TRANS_OnlineProcess();
					}
				}
				break;
			case TRANS_VOIDCUPMOBILE:
				ucResult = MAG_ProUntouchCard();
				if (ucResult == SUCCESS)
				{
					memset(ucPindata,0,sizeof(ucPindata));
					ucResult = UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
					if (ucResult == SUCCESS)
						ucResult = TRANS_OnlineProcess();
				}
				else
				{
					return (ERR_CANCEL);
				}
				break;
			default:
				break;
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.020] [rst=%02x]",ucResult);

	/*GET the current data and time*/
	if((ucResult == SUCCESS ))
	{
		switch( G_NORMALTRANS_ucTransType )
		{
		case TRANS_UNDOOFF:
		case TRANS_TIPADJUST:
		case TRANS_OFFPREAUTHFINISH:
		case TRANS_OFFPURCHASE:
#ifdef USEUPDATADATATIME
			memset(DateTimeBuf,0,sizeof(DateTimeBuf));
			Os__read_date(DateTimeBuf);
			asc_bcd(aucBuf,3,DateTimeBuf,6);
			G_NORMALTRANS_aucDate[0] = 0x20;
			G_NORMALTRANS_aucDate[1] = aucBuf[2];
			G_NORMALTRANS_aucDate[2] = aucBuf[1];
			G_NORMALTRANS_aucDate[3] = aucBuf[0];
			memset(DateTimeBuf,0,sizeof(DateTimeBuf));
			Os__read_time_sec(DateTimeBuf);
			asc_bcd(aucBuf,3,DateTimeBuf,6);
			aucBuf[3]=0;
			memcpy(G_NORMALTRANS_aucTime,aucBuf,3);
#else
			memset(DateTimeBuf,0,sizeof(DateTimeBuf));
			Os__read_date(aucBuf);
			DateTimeBuf[0] = 0x20;
			asc_bcd(&DateTimeBuf[1],1,&aucBuf[4],2);
			LOGON_Dispatch_DateTime(&DateTimeBuf[2]);
			memcpy(G_NORMALTRANS_aucDate,DateTimeBuf,4);
			Os__read_time_sec(aucBuf);
			memcpy(G_NORMALTRANS_aucTime,&DateTimeBuf[4],2);
			asc_bcd(&G_NORMALTRANS_aucTime[2],1,&aucBuf[4],2);
			util_Printf("\n--2--G_NORMALTRANS_aucDate Time =");
			{
				int i;
				for(i=0;i<13;i++)
					util_Printf("%02x ",DateTimeBuf[i] );
				util_Printf("\n");
			}
#endif
			break;
		default:
			break;
		}
	}
	util_Printf("\n EXTrans_Process ....[EX-0007.021] [rst=%02x]",ucResult);

	/* Store Trans */
	util_Printf("交易类型 12=%02x\n",G_NORMALTRANS_ucTransType);
	if( ucResult == SUCCESS )
	{
		switch( G_NORMALTRANS_ucTransType )
		{
		case TRANS_QUERYBAL:
			break;
		case TRANS_CREDITSVOID:
		case TRANS_CREDITSALE:
		case TRANS_MOBILEAUTH:
		case TRANS_VOIDMOBILEAUTH:
		case TRANS_REFUND:
		case TRANS_UNDOOFF:
		case TRANS_PREAUTH:
		case TRANS_PURCHASE:
		case TRANS_TIPADJUST:
		case TRANS_CUPMOBILE:
		case TRANS_PREAUTHADD:
		case TRANS_OFFPURCHASE:
		case TRANS_VOIDPREAUTH:
		case TRANS_VOIDPURCHASE:
		case TRANS_VOIDCUPMOBILE:
		case TRANS_PREAUTHFINISH:
		case TRANS_PREAUTHSETTLE:
		case TRANS_VOIDOFFPURCHASE:
		case TRANS_OFFPREAUTHFINISH:
		case TRANS_VOIDPREAUTHFINISH:
		case TRANS_EC_ASSIGNLOAD:
		case TRANS_EC_UNASSIGNLOAD:
		case TRANS_EC_CASHLOAD:
			ucResult = SAV_TransSave();
			break;
		default:
			break;
		}
	}
	if(!DataSave0.ChangeParamData.ucPackgeHeadEve)
		COMMS_FinComm();
	util_Printf("流水1111 [%d]\n",G_NORMALTRANS_ulTraceNumber);

	util_Printf("\n==1===ucResult = %02X\n",ucResult);
	if(( ucResult == SUCCESS )&&(DataSave0.ConstantParamData.ucCashFlag != 0x31))
	{
		switch( G_NORMALTRANS_ucTransType )
		{
		case TRANS_QUERYBAL:
			break;
		case TRANS_CREDITSVOID:
		case TRANS_CREDITSALE:
		case TRANS_MOBILEAUTH:
		case TRANS_VOIDMOBILEAUTH:
		case TRANS_UNDOOFF:
		case TRANS_PURCHASE:
		case TRANS_VOIDPURCHASE:
		case TRANS_PREAUTH:
		case TRANS_VOIDPREAUTH:
		case TRANS_PREAUTHFINISH:
		case TRANS_PREAUTHSETTLE:
		case TRANS_VOIDPREAUTHFINISH:
		case TRANS_OFFPREAUTHFINISH:
		case TRANS_REFUND:
		case TRANS_TIPADJUST:
		case TRANS_VOIDOFFPURCHASE:
		case TRANS_OFFPURCHASE:
		case TRANS_PREAUTHADD:
		case TRANS_CUPMOBILE:
		case TRANS_VOIDCUPMOBILE:
		case TRANS_EC_ASSIGNLOAD:
		case TRANS_EC_UNASSIGNLOAD:
		case TRANS_EC_CASHLOAD:
#ifdef GUI_PROJECT		
	    		PRINT_Ticket(NORMAL_PRINT_FLAG,ucCopy);
#else
			if( G_NORMALTRANS_ucTransType == TRANS_TIPADJUST )
			{
				//Os__clr_display(255);
				//Os__GB2312_display(1,0,(uchar *)"    调整成功");
				//Os__GB2312_display(2,0,(uchar *)"    正在打印");
			}
			else
			{
				if(G_NORMALTRANS_ucInputMode != TRANS_INPUTMODE_INSERTCARD
					&&G_NORMALTRANS_ucInputMode != TRANS_INPUTMODE_PUTCARD)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(1,0,(uchar *)"    交易成功");
					//Os__GB2312_display(2,0,(uchar *)"    正在打印");
				}
			}

			if (G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_HB_PUTCARD
				||G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_SAND_PUTCARD)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(1,0,(uchar *)"    交易成功");
				//Os__GB2312_display(2,0,(uchar *)"    正在打印");
				//Os__GB2312_display(3,0,(uchar *)"     请取卡");
			}
			if(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_INSERTCARD)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(1,0,(uchar *)"    交易成功");
				//Os__GB2312_display(2,0,(uchar *)"    正在打印");
				//Os__GB2312_display(3,0,(uchar *)"     请拔卡");
			}
			util_Printf("打印次数3333 = %02x,%d\n",PRINTTIMES,ucCopy);
			for(ucCopy=0,ucI=0;ucI<PRINTTIMES;ucI++,ucCopy++)
			{
				if(ucI)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(1,0,(uchar *)"    请撕小票");
					//Os__GB2312_display(2,0,(uchar *)"按任意键继续打印");
					MSG_WaitKey(5);
				}
				if(ucI)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(1,0,(uchar *)" 正在打印....");
				}
	    		       PRINT_Ticket(NORMAL_PRINT_FLAG,ucCopy);
			}
#endif				
			
			break;
		default:
			break;
		}
	}
	util_Printf("===2==ucResult = %02X\n",ucResult);
	if( (G_NORMALTRANS_ucTransType == TRANS_OFFPREAUTHFINISH)		/*离线未上送调整交易,流水号不变*/
	      &&(G_NORMALTRANS_ucTIPAdjustFlag))
      {
      }
	else
	{
		if (DataSave0.ConstantParamData.ucCashFlag != 0x31)
		{
			UTIL_IncreaseTraceNumber();	/*离线及调整在存储时流水号已加，上送时则不加*/
		}
	}
	util_Printf("\na. G_NORMALTRANS_ulTraceNumber: [%6d]\n",G_NORMALTRANS_ulTraceNumber);
	
	if (!ucResult
	&&(G_NORMALTRANS_ucTransType != TRANS_TIPADJUST)
	  &&(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)
	  &&(G_NORMALTRANS_ucTransType != TRANS_OFFPURCHASE))
	{
		ucResult1 = Trans_SendICOfflineTrans();
	}

	if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
	{
		   if(  (G_NORMALTRANS_ucTransType != TRANS_TIPADJUST)
			   &&(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH))
			{
				if(DataSave0.ChangeParamData.ucPackgeHeadEve)
				{
					ucResult1 = EMV_PackgeHeadEveProcess(DataSave0.ChangeParamData.ucPackgeHeadEve);
					if(ucResult1 == SUCCESS)
					{
						Os__saved_set(&DataSave0.ChangeParamData.ucPackgeHeadEve,0,1);
					}
					XDATA_Write_Vaild_File(DataSaveChange);
				}
			}
	}else
	{
	   if(  (G_NORMALTRANS_ucTransType != TRANS_TIPADJUST)
		   &&(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH))
		{
			if(DataSave0.ChangeParamData.ucPackgeHeadEve)
			{
				ucResult1=EMV_PackgeHeadEveProcess(DataSave0.ChangeParamData.ucPackgeHeadEve);
				if(ucResult1 == SUCCESS)
				{
					Os__saved_set(&DataSave0.ChangeParamData.ucPackgeHeadEve,0,1);
				}
				XDATA_Write_Vaild_File(DataSaveChange);
			}
		}
	}
	/*处理报头下发要求*/
	COMMS_FinComm();
	if(ReaderSupport&&(ReaderType==READER_HONGBAO)&&(G_NORMALTRANS_ucInputMode==TRANS_INPUTMODE_HB_PUTCARD))
	{
		util_Printf("\nINTERFACE_RefurbishReaderLCD(4)");
		INTERFACE_RefurbishReaderLCD(4);//交易完成
	}
	if((ucResult != SUCCESS)
		&&(ucResult != ERR_CANCEL)
		&&(ucResult != ERR_END)&&(DataSave0.ConstantParamData.ucCashFlag != 0x31))
	{
		MSG_DisplayErrMsg(ucResult);
		ucResult = ERR_CANCEL;
	}
	util_Printf("\nb. G_NORMALTRANS_ulTraceNumber: [%6d]\n",G_NORMALTRANS_ulTraceNumber);	return ucResult;
}

