#include <include.h>
#include <global.h>
#include <util.h>
#include <xdata.h>
#include <osicc.h>
#ifdef TMS
#include "tmsapp.h"
#include "tms.h"
#endif


#pragma arm section rwdata ,code = "manager"
/*
int main(int args,char *argv[])
{
	//setlocale(LC_ALL, "C");//2012-01-05
	main_entry(args,argv);
}
*/
 
void manager(struct seven *EventInfo)
{
	unsigned char ucResult ;
	unsigned char ucKey;
	unsigned char EventType;
	unsigned char EventNum;
	unsigned char ucInputMode ,ucCardSelect;
	unsigned char ucICCardType,ucResetBufLen,aucResetBuf[50];
	unsigned short usI;

	uchar ucTransFlag=0;

	EventType = (unsigned char )EventInfo->event_type;
	EventNum = (unsigned char )EventInfo->event_nr;

	util_Printf("\n中国银联PBOC版 P400\n事件类型=%02x,事件序号=%02x\n",EventType,EventNum);
	util_Printf("项目编号:%s\n",CURRENT_PRJ);
	util_Printf("项目版本:%s\n",SOFTWARE_VER);

	ucResult = SUCCESS;

	Os__light_on();
	EventInfo->pt_eve_out->treatment_status = NOK;
	switch ( EventType+EventNum)
	{
		case EVECIR + EVE_MAG_CARD:
			UTIL_ClearGlobalData();
			UTIL_GetTerminalInfo();
			MAG_Read_LoadTransType();
			if(DataSave0.ChangeParamData.ucTerminalLockFlag)
			{
				ucResult =UTIL_UnLockKeyBoard();
				if(ucResult)    break;
			}
			if (DataSave0.ConstantParamData.ucSwiptEvent == 0x30)
				break;
			if (DataSave0.ConstantParamData.ucCashFlag == 0x31)
				break;
			if(UTIL_Check_BasetSet() == SUCCESS)
				break;
			ucResult = MAG_EMVSetInputMode(&ucInputMode);
			if(ucResult)
			{
				MSG_DisplayErrMsg(ucResult);
			}
			ucResult = MAG_CardSelect(ucInputMode ,
								&ucCardSelect ,
								&(EventInfo->pt_eve_in->e_mag_card.mag_card[0]));
		       if (ucResult == ERR_SANDMAGCARD)
		       {
		            EventInfo->pt_eve_out->treatment_status = NOK;
		            break;
		       }
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
			if((ucResult == ERR_USEICCARDFIRST||ucResult ==SUCCESS)
				&&(ucCardSelect == CARDSPECIES_MAG)
				&&(G_NORMALTRANS_ucInputMode==TRANS_INPUTMODE_SWIPECARD))
			{
				ucResult = MAG_DispCardNo();
			}
			util_Printf("manager=%02x\n",ucResult);
			if((ucResult==SUCCESS&&G_RUNDATA_ucReadCardFlag == EMVFIRSTMAG)||(ucResult==SUCCESS)||(ucResult==ERR_FALLBACK))
			{
				if(G_RUNDATA_ucReadCardFlag==EMVFIRSTMAG)
					G_NORMALTRANS_ucFallBackFlag =2;
				G_RUNDATA_ucDefOperation=true;
				ucResult=Trans_Process(0);
			}
			if((ucResult != SUCCESS)
				&&(ucResult != ERR_END)
				&&(ucResult != ERR_CANCEL)
				&&(ucResult != ERR_USEICCARDFIRST))
			{
				MSG_DisplayErrMsg(ucResult);
			}
			Os__light_off();
			EventInfo->pt_eve_out->treatment_status = OK;
			UTIL_WaitPullOutICC();
			break;
		case EVECIR + EVE_ICC:
			UTIL_ClearGlobalData();
			UTIL_GetTerminalInfo();
			MAG_Read_LoadTransType();
			if(DataSave0.ChangeParamData.ucTerminalLockFlag)
			{
				ucResult =UTIL_UnLockKeyBoard();
				if(ucResult)    break;
			}
			if (DataSave0.ConstantParamData.ucSwiptEvent == 0x30)
				break;
			if(DataSave0.ConstantParamData.ucCashFlag == 0x31)
				break;
			if(UTIL_Check_BasetSet() == SUCCESS)
				break;
			
			ucResetBufLen = sizeof(aucResetBuf);
			ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);
			util_Printf("卡类型ucICCardType=%02x\n",ucICCardType);
			if(ucICCardType!=ICC_ASY 	&& ucICCardType!=ICC_SYN)				//异步卡 , 同步卡
			{
				G_NORMALTRANS_ucFallBackFlag =2;
				G_RUNDATA_ucDefOperation=true;
				ucResult=MASAPP_Event(true ,&ucKey);

				ucTransFlag = 0x01;
			}else 
			if(ucICCardType!=ICC_SYN)
			{
				G_RUNDATA_ucDefOperation=true;
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_EMV;

				ucTransFlag = 0x01;
			}
			if(ucTransFlag == 0x01){
				if(ucResult==0x00)
				{
					ucResult=Trans_Process(0);
				}
				if((ucResult != SUCCESS)
				&&(ucResult != ERR_CANCEL)
				&&(ucResult != ERR_END))
				{
					MSG_DisplayErrMsg(ucResult);
				}
				Os__light_off();
				DispPINPAD_AD();
				EventInfo->pt_eve_out->treatment_status = OK;
				UTIL_WaitPullOutICC();
			}
			break;
		case EVECOM + EVE_POWER_ON:
			OnEve_power_on();
			DispPINPAD_AD();
			break;
		case EVESEL + 0xff:
			UTIL_ClearGlobalData();
			UTIL_GetTerminalInfo();
			MAG_Read_LoadTransType();
			if(DataSave0.ChangeParamData.ucTerminalLockFlag)
			{
				ucResult =UTIL_UnLockKeyBoard();
				if(ucResult)    break;
			}
			Os__light_on();
			//Os__clr_display(255);
			UTIL_CheckSettlePrint();
			//Os__GB2312_display(0,0,(uchar *)"1.消费  2.撤  销");
			//Os__GB2312_display(1,0,(uchar *)"3.退货  4.预授权");
			//Os__GB2312_display(2,0,(uchar *)"5.离线  6.打  印");
			//Os__GB2312_display(3,0,(uchar *)"7.管理  8.其  它");
			Os__light_off();
			break;
		case EVECOM+EVE_AWAKE:
			UTIL_ClearGlobalData();
			UTIL_GetTerminalInfo();
			MAG_Read_LoadTransType();
			COMMS_FinComm();
			if(DataSave0.ChangeParamData.ucTerminalLockFlag)
			{
				ucResult =UTIL_UnLockKeyBoard();
				if(ucResult)    break;
			}
			if (DataSave0.ConstantParamData.ucDisAutoTiming== '1')
			{
				UTIL_ClearGlobalData();
				UTIL_GetTerminalInfo();
				MAG_Read_LoadTransType();
				UTIL_AutoSettleOn();
			}
			if(DataSave0.ConstantParamData.ucDisAutoSettleFlag== '1')
			{
				UTIL_ClearGlobalData();
				UTIL_GetTerminalInfo();
				MAG_Read_LoadTransType();
				AutoSettle();
			}
			if(!DataSave0.ConstantParamData.BackLightFlag)
			{
				Os__light_off();
				Os__light_off_pp();
			}
			util_Printf("\nReaderSupport=%d,ReaderType=%d,READER_HONGBAO=%d",ReaderSupport,ReaderType,READER_HONGBAO);
			if(ReaderSupport&&(ReaderType==READER_HONGBAO))
			{
				util_Printf("\nINTERFACE_RefurbishReaderLCD(1)");
				INTERFACE_RefurbishReaderLCD(1);//欢迎使用
			}
			break;
#ifdef TMS
		case 0x02 + 0x27:
			UTIL_ClearGlobalData();
			UTIL_GetTerminalInfo();
			MAG_Read_LoadTransType();
			SEL_DOWNTTMSPARA();
			Os__light_off();
			break;
			
		case EVESEL + EVE_GETINFO:
			util_Printf("+++++++++++事件EVESEL + EVE_GETINFO\n");
#if 0
			memcpy(EventInfo->pt_eve_in->e_cash_register.ecr_msg,(unsigned char *)CURRENT_PRJ,sizeof(CURRENT_PRJ));
			memcpy(&EventInfo->pt_eve_in->e_cash_register.ecr_msg[20],SOFTWARE_VER,sizeof(SOFTWARE_VER));
#else
			UTIL_ClearGlobalData();
			UTIL_GetTerminalInfo();
			SEL_GetParam();

			memset(EventInfo->pt_eve_in->e_cash_register.ecr_msg,0x00,sizeof(EventInfo->pt_eve_in->e_cash_register.ecr_msg));
			memcpy(EventInfo->pt_eve_in->e_cash_register.ecr_msg,(unsigned char *)CURRENT_PRJ,sizeof(CURRENT_PRJ));
			memcpy(&EventInfo->pt_eve_in->e_cash_register.ecr_msg[20],(unsigned char *)SOFTWARE_VER,sizeof(SOFTWARE_VER));
			if((ucResult = UTIL_Is_Trans_Empty()) == 0)
				EventInfo->pt_eve_in->e_cash_register.ecr_msg[50] = 1;
			else
				EventInfo->pt_eve_in->e_cash_register.ecr_msg[50] = 0;
			int_bcd(&EventInfo->pt_eve_in->e_cash_register.ecr_msg[51],2,&uiUpdateLen);
			memcpy(&EventInfo->pt_eve_in->e_cash_register.ecr_msg[53],aucUpdateBuff,uiUpdateLen);
#endif
			break;

		case EVESEL + EVE_DOWNINFO:
			util_Printf("+++++++++++事件EVESEL + EVE_DOWNINFO\n");
			UTIL_ClearGlobalData();
			UTIL_GetTerminalInfo();
#if 0
			memset(aucUpdateBuff,0x00,sizeof(aucUpdateBuff));
			memcpy(aucUpdateBuff,&EventInfo->pt_eve_in->e_cash_register.ecr_msg[0],2);
			util_Printf("中国银联PBOC\n");
			uiUpdateLen=(unsigned short)tab_long(&EventInfo->pt_eve_in->e_cash_register.ecr_msg[2],2);
			util_Printf("+++++++++++++++++===数据长度:%ld\n",uiUpdateLen);
			memset(aucUpdateBuff,0x00,sizeof(aucUpdateBuff));
			memcpy(aucUpdateBuff,&EventInfo->pt_eve_in->e_cash_register.ecr_msg[4],uiUpdateLen);
			TMSAPP_UpdateTMSData_ADS(TMS_WriteFile);
#endif
			memset(aucUpdateBuff,0x00,sizeof(aucUpdateBuff));
			memcpy(aucUpdateBuff,&EventInfo->pt_eve_in->e_cash_register.ecr_msg[0],2);
			if(!memcmp(&aucUpdateBuff[0],AppSoftId,2))
			{
				uiUpdateLen = (unsigned short)tab_long(&EventInfo->pt_eve_in->e_cash_register.ecr_msg[2],2);
				memset(aucUpdateBuff,0x00,sizeof(aucUpdateBuff));
				memcpy(aucUpdateBuff,&EventInfo->pt_eve_in->e_cash_register.ecr_msg[4],uiUpdateLen);
				//Dump((uchar *)"ReceParam",aucUpdateBuff,uiUpdateLen);
				TMSAPP_UpdateTMSData_ADS(TMS_WriteFile);
			}
			break;
#endif
		case EVESEL + EVE_RESELECT_LOGON://签到
			UTIL_ClearGlobalData();
			UTIL_GetTerminalInfo();
			if(DataSave0.ChangeParamData.ucTerminalLockFlag)
			{
				ucResult =UTIL_UnLockKeyBoard();
				if(ucResult)    break;
			}
			MAG_Read_LoadTransType();
			G_NORMALTRANS_ucTransType = TRANS_LOGONON;
			ucResult = LOGON_CashierProcess(0);
			Os__light_off();
			break;
		case EVESEL + EVE_MAN:						//结算
			UTIL_ClearGlobalData();
			UTIL_GetTerminalInfo();
			if(DataSave0.ChangeParamData.ucTerminalLockFlag)
			{
				ucResult =UTIL_UnLockKeyBoard();
				if(ucResult)    break;
			}
			MAG_Read_LoadTransType();
			if(DataSave0.ConstantParamData.ucDisplay == 0x31)
			{
				ucResult=SERV_DispTotal();
				util_Printf("ucResult=%02x",ucResult);
				if (ucResult == SUCCESS)
				    break;
			}
			ucResult = SERV_Settle(true);
			Os__light_off();
			break;
		case EVECIR +0x25:
			EventInfo->pt_eve_out->treatment_status = NOK;
			break;
		case EVESEL + '1':
		case EVESEL + '2':
		case EVESEL + '3':
		case EVESEL + '4':
		case EVESEL + '5':
		case EVESEL + '6':
		case EVESEL + '7':
		case EVESEL + '8':
		case EVESEL + '9':
		case EVESEL +	KEY_F:
		case EVESEL +	KEY_R:
		case EVESEL +	KEY_F1:
		case EVESEL +	KEY_F2:
		case EVESEL +	KEY_F3:
		case EVESEL +	KEY_F4:
		case EVESEL +	KEY_ENTER:
		case EVESEL +	KEY_PAPER_FEED:
           if(DataSave0.ChangeParamData.ucTerminalLockFlag)
			{
				ucResult =UTIL_UnLockKeyBoard();
				if(ucResult)    break;
			}
			Mag_Menu(EventNum);
			Os__light_off();
			COMMS_FinComm();
		break;
	default:
		break;
	}
}
#pragma arm section code ,rwdata

unsigned char Main_Logon(void)
{
	unsigned char key;

	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(unsigned char *)"    是否签到?");
	//Os__GB2312_display(2,0,(unsigned char *)" 按[确认]键继续");
	key=MSG_WaitKey(10);
	if(key == KEY_ENTER)
	{
		G_NORMALTRANS_ucTransType = TRANS_LOGONON;
		LOGON_CashierProcess(0);
	}else
	{
		return(ERR_CANCEL);
	}
	return(SUCCESS);
}




unsigned char DispDateAndTime(unsigned char lin,unsigned col )
{
	unsigned char DispBuf[17];
	unsigned char Buf[11];

	memset(Buf,0,sizeof(Buf));
	memset(DispBuf,0,sizeof(DispBuf));

	UTIL_READ_DateTimeAndFormat(Buf/*YYMMDDhhmm*/);
	memcpy(DispBuf,"20",2);
	memcpy(&DispBuf[2],Buf,2);//YY
	memcpy(&DispBuf[4],"/",1);
	memcpy(&DispBuf[5],&Buf[2],2);//MM
	memcpy(&DispBuf[7],"/",1);
	memcpy(&DispBuf[8],&Buf[4],2);//DD
	memcpy(&DispBuf[10]," ",1);
	memcpy(&DispBuf[11],&Buf[6],2);//hh
	memcpy(&DispBuf[13],":",1);
	memcpy(&DispBuf[14],&Buf[8],2);//mm

	//Os__GB2312_display(lin,col,DispBuf);
	return SUCCESS;
}
void DispPINPAD_AD(void)
{
	if(DataSave0.ConstantParamData.Pinpadflag ==1)
	{
		Os__light_on_pp();
		Os__clr_display_pp(255);
		//Os__GB2312_display_pp(1,0,(uchar *)"   杉德银卡通");
		Os__GB2312_display_pp(0,0,(uchar *)"    欢迎使用");
	}
}


void AutoSettle(void)
{
	unsigned char	aucDateTime[17],aucBuf[10],aucTransDate[10];
	unsigned short uiYear,uiH,uiD,uiM,uiY,uiMm;
	unsigned short uiDays,uiHours;



	if(DataSave0.ChangeParamData.ucCashierLogonFlag==CASH_LOGONFLAG)
	{
		if((DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb == 0)
		  &&(DataSave0.TransInfoData.TransTotal.uiTotalNb == 0)
		)
		{
			return;
		}
		uiDays=00;
		uiHours=23;
		util_Printf("uiHours=%d\n",uiHours);
#ifdef Hour
			for(i=1;i<=sizeof(DataSave0.ChangeParamData.ucSettleTime);i++)
			{util_Printf("SettleDateTime=%02x\n",DataSave0.ChangeParamData.ucSettleTime[i]);}
			for(i=1;i<=sizeof(DataSave0.ChangeParamData.ucSettleData);i++)
			{util_Printf("SettleDateData=%02x\n",DataSave0.ChangeParamData.ucSettleData[i]);}
#endif
		if(!memcmp(&DataSave0.ChangeParamData.ucSettleData[1],"\x00\x00\x00\x00",4)
			&&!memcmp(&DataSave0.ChangeParamData.ucSettleTime[1],"\x00\x00\x00",3))
		//第1次无结算时间,获取系统当前时间为第一次结算时间
		{
			util_Printf("The SettleCycle\n");
			memset(aucBuf,0,sizeof(aucBuf));
			/* 获取系统日期*/
			Os__read_date(aucBuf);	//\DDMMYY;
			DataSave0.ChangeParamData.ucSettleData[0]=0x20;
			asc_bcd(&DataSave0.ChangeParamData.ucSettleData[1],1,&aucBuf[4],2);
			asc_bcd(&DataSave0.ChangeParamData.ucSettleData[2],1,&aucBuf[2],2);
			asc_bcd(&DataSave0.ChangeParamData.ucSettleData[3],1,&aucBuf[0],2);
			/*获取系统时间*/
			Os__read_time(aucBuf);
			asc_bcd(&DataSave0.ChangeParamData.ucSettleTime[0],2,&aucBuf[0],4);
			XDATA_Write_Vaild_File(DataSaveChange);
		}

		uiYear=(unsigned short)bcd_long(DataSave0.ChangeParamData.ucSettleData,4);/*上批次结算日期[ 年]*/
		util_Printf("uiYear=%d\n",uiYear);

		//yymmddhh
#ifdef Lote
		for( uiI=0;uiI<4;uiI++)
		{
			util_Printf("11=======aucDateTime=%02x\n",DataSave0.ChangeParamData.ucSettleTime[uiI]);
		}
#endif
		uiH=(unsigned short)bcd_long(&DataSave0.ChangeParamData.ucSettleTime[0],2);/* 时*/
		uiMm=(unsigned short)bcd_long(&DataSave0.ChangeParamData.ucSettleTime[1],2);

		memset(aucDateTime,0,sizeof(aucDateTime));
		memcpy(aucDateTime,DataSave0.ChangeParamData.ucSettleData,8);/*上批次结算日期*/
		uiD=(unsigned short)bcd_long(&aucDateTime[3],2);/*  日*/
		uiM=(unsigned short)bcd_long(&aucDateTime[2],2);/*  月*/
		uiY=(unsigned short)bcd_long(&aucDateTime[1],2);/*  年*/

		util_Printf("uiY=%d uiM=%d uiD=%d  uiH=%d uiMm=%d\n",uiY,uiM,uiD,uiH,uiMm);

		uiH += uiHours;
		util_Printf("uiH=%d\n",uiH);
		if(uiH>24)
		{
			uiD=uiD+uiDays+1;
			uiH=uiH-24;
		}
		else
			uiD += uiDays;
		switch(uiM)
		{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			while(uiD>31)
			{
				uiD -= 31;
				uiM += 1;
			}
			break;
		case 2:
			if((uiYear%4==0 && uiYear%100!=0)||(uiYear%400==0))
			{
				while(uiD>29)
				{
					uiD -= 29;
					uiM += 1;
				}
			}else
			{
				while(uiD>28)
				{
					uiD -= 28;
					uiM += 1;
				}
			}
			break;
		default:
			while(uiD>30)
			{
				uiD -= 30;
				uiM += 1;
			}
			break;
		}

		if(uiM>12)
		{
			uiY +=1;
			uiM -=12;
		}

		memset(aucBuf,0,sizeof(aucBuf));
		Os__read_time_sec(aucBuf);//hhmmss/
		asc_bcd(&aucTransDate[4],3,aucBuf,6);

		aucTransDate[0]=0x20;
		memset(aucBuf,0,sizeof(aucBuf));
		Os__read_date(aucBuf);	//*DDMMYY/;
		asc_bcd(&aucTransDate[1],1,&aucBuf[4],2);
		asc_bcd(&aucTransDate[2],1,&aucBuf[2],2);
		asc_bcd(&aucTransDate[3],1,&aucBuf[0],2);

		memset(aucDateTime,0,sizeof(aucDateTime));
		short_bcd(aucDateTime,1,&uiY);
		short_bcd(&aucDateTime[1],1,&uiM);
		short_bcd(&aucDateTime[2],1,&uiD);
		short_bcd(&aucDateTime[3],1,&uiH);

		memcpy(&aucDateTime[4],&DataSave0.ChangeParamData.ucSettleTime[1],1);

		util_Printf("SettleDateTime=%02x %02x %02x %02x %02x\n",aucDateTime[0],aucDateTime[1],aucDateTime[2],aucDateTime[3],aucDateTime[4]);
		util_Printf("LocalDateTime=%02x %02x %02x %02x %02x\n",aucTransDate[1],aucTransDate[2],aucTransDate[3],aucTransDate[4],aucTransDate[5]);
		if(memcmp(&aucTransDate[1],aucDateTime,5)>=0)
		{
			SERV_Settle(true);
		}

	}
	return;
}




