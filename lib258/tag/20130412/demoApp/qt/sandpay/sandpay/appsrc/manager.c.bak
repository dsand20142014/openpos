#include <include.h>
#include <global.h>
#include <xdata.h>
#pragma arm section rwdata ,code = "manager"
#ifndef GUI_PROJECT
//int main(int args,char *argv[])
//{
//	main_entry(args,argv);
//}
void manager(struct seven *EventInfo)
{
	unsigned char ucResult = SUCCESS;
	unsigned char EventType;
	unsigned char EventNum,ucKey;
	unsigned int uiTimesOut;
	unsigned char pCust_Card[300],strBuf[17],aucZDFunFlag[64];
	int i,ucI,ucJ,uiLen,settleresult;
	
	EventType = (unsigned char )EventInfo->event_type;
	EventNum = (unsigned char )EventInfo->event_nr;
	Uart_Printf("\nEventType=%02x,EventNum=%02x\n",EventType,EventNum);
	if(  (EventNum >= '0')&&(EventNum <= '9') )
	{
		ucKey = EventNum;
		EventNum = 0xFE;
	}
	
	Os__light_on();
	
	EventInfo->pt_eve_out->treatment_status = NOK;

	if(EventType!=EVECOM || EventNum!=EVE_POWER_ON)
		UTIL_ClearGlobalData(); 
	settleresult=CFG_AutoSettle();
	if(settleresult==1)
		return ;

	Uart_Printf("DataSave0.RedoTrans.ucRedoFlag=%02x\n",DataSave0.RedoTrans.ucRedoFlag);
	
	switch ( EventType+EventNum)
	{
	case EVECOM + EVE_AWAKE:				
		COMMS_FinComm();
		Os__light_off();
		break;
	case EVECIR + EVE_MAG_CARD: 
		Uart_Printf("Enter EVE_MAG_CARD\n");
		//UTIL_ClearGlobalData(); 
		if(DataSave0.ConstantParamData.ucYTJFlag)
			break;
		ucResult=Mag_Menu(EventInfo->pt_eve_in);  
		if(ucResult!=ERR_NOTPROC)
			EventInfo->pt_eve_out->treatment_status = OK;
		if(DataSave0.ConstantParamData.Pinpadflag)
		{
			//Os__clr_display_pp(255);
			//Os__GB2312_display_pp(0,0,(uchar *)"   ɼͨ");
			//Os__GB2312_display_pp(1,0,(uchar *)"    ӭʹ");
		}
		break;
	case EVECIR + EVE_ICC: 
		//UTIL_ClearGlobalData(); 
		if( EventInfo->pt_eve_in->e_icc.icc_ATR[1] == ICC_SYN )
		{
			ucResult = SLE4442_ATR(strBuf);
			if( ucResult == SUCCESS )
			{
				if(SMC_CheckCard()!=SUCCESS)
				{
					memset(pCust_Card,0,sizeof(pCust_Card));
					for(ucI=0,ucJ=32;ucI<256-32;ucI+=ucJ)
					{
						if( ucResult == SUCCESS )
						{
							ucResult=SLE4442_ReadMainMemory(pCust_Card+ucI,32+ucI,ucJ);
						}
					}
					if( ucResult == SUCCESS )
					{
						if(memcmp(&pCust_Card[217],"SANDPAY",7))
							ucResult = ERR_NOTPROC;
					}
				}
			}
			if( ucResult == SUCCESS )
			{	
				ucResult = SLE4442_Menu(EventInfo->pt_eve_in);
				if( ucResult != SUCCESS)			
				{
					MSG_DisplayErrMsg(ucResult);
				}
				//Os__clr_display(255);
				//Os__GB2312_display(1,0,(uchar *)"γ");
				Os__xdelay(10);
				Os__ICC_remove();
				EventInfo->pt_eve_out->treatment_status = OK;
			}
			else
			{
				Uart_Printf("\nThis card is not treated!!!");
				EventInfo->pt_eve_out->treatment_status = NOK;
			}
		}
		else
		{
			for(i=0;i<DataSave0.ChangeParamData.uiZoneCodeNum;i++)
			{
				XDATA_Read_ZoneCtrls_File(i);
				UTIL_BitFlagAnalyze(DataSave1.ZoneCtrl.aucFunction,8,aucZDFunFlag);
				if(aucZDFunFlag[48])
					break;
			}
			if(i!=DataSave0.ChangeParamData.uiZoneCodeNum)
			{
				if(ucResult==SUCCESS)
				{
					ucResult = PBOC_ISOSelectFilebyFID_Bak(ICC1,(uchar *)"\x3F\x00");
					if(ucResult==SUCCESS)
						ucResult = PBOC_ISOSelectFilebyFID_Bak(ICC1,(uchar *)"\xDF\x01");
				}
				if(ucResult==SUCCESS)
				{
					ucResult = PBOC_ISOReadBinarybySFI_Bak(ICC1,0x15,0x00,0x1e);
					memcpy(NormalTransData.aucBarCode,&Iso7816Out.aucData[10],10);
				}
				if(ucResult==SUCCESS)
				{
					ucResult = Untouch_Zsh_Process();
					if( ucResult != SUCCESS)			
					{
						MSG_DisplayErrMsg(ucResult);
					}
					EventInfo->pt_eve_out->treatment_status = OK;
					break;
				}
			}
			RunData.ucInputmode=1;
			ucResult = Untouch_Menu();
			if( ucResult == ERR_NOTSUCHAPP)
	     	{
	     		EventInfo->pt_eve_out->treatment_status = NOK;
				break;
	     	}
			else
			{
				//Os__clr_display(255);
				//Os__GB2312_display(1,0,(uchar *)"γ");
				Os__xdelay(10);
				Os__ICC_remove();
		     	EventInfo->pt_eve_out->treatment_status = OK;
			}
			if(DataSave0.ConstantParamData.Pinpadflag)
			{
				//Os__clr_display_pp(255);
				//Os__GB2312_display_pp(0,0,(uchar *)"   ɼͨ");
				//Os__GB2312_display_pp(1,0,(uchar *)"    ӭʹ");
			}
		}
		if(DataSave0.ConstantParamData.Pinpadflag)
		{
			//Os__clr_display_pp(255);
			//Os__GB2312_display_pp(0,0,(uchar *)"   ɼͨ");
			//Os__GB2312_display_pp(1,0,(uchar *)"    ӭʹ");
		}
		break;
	case EVECOM + EVE_POWER_ON:			
		Uart_Printf("\nEVECOM + EVE_POWER_ON");
		Os__light_on();
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"   Ϻ.ɼ   ");
		//Os__GB2312_display(1,0,(uchar *)"ϵͳԼ...    ");
		OnEve_power_on();
		if(DataSave0.ConstantParamData.Pinpadflag)
		{
			//Os__clr_display_pp(255);
			//Os__GB2312_display_pp(0,0,(uchar *)"   ɼͨ");
			//Os__GB2312_display_pp(1,0,(uchar *)"    ӭʹ");
		}
		break;
	case EVESEL + KEY_F3: 
		if(DataSave0.ConstantParamData.ucKFCFlag)
		{
			//Os__GB2312_display(0,0,(uchar *)"1.  2.ش");
			//Os__GB2312_display(1,0,(uchar *)"3.ͳ  4.ǩ");   
			//Os__GB2312_display(2,0,(uchar *)"5.6.");	
			//Os__GB2312_display(3,0,(uchar *)"7.  8.ϵͳ  ");   
		}
		else
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"9.");
		}
		EventInfo->pt_eve_out->treatment_status = OK;
	      break;	
	 case EVESEL + KEY_F4:
		//UTIL_ClearGlobalData(); 
		//Os__clr_display(255);     
		if(DataSave0.ConstantParamData.ucKFCFlag)
		{
			//Os__GB2312_display(0,0,(uchar *)"1.  2.ش");
			//Os__GB2312_display(1,0,(uchar *)"3.ͳ  4.ǩ");   
			//Os__GB2312_display(2,0,(uchar *)"5.6.");	
			//Os__GB2312_display(3,0,(uchar *)"7.  8.ϵͳ  ");   
		}
		else
		{
			//Os__GB2312_display(0,0,(uchar *)"1.ǩ  2.");
			//Os__GB2312_display(1,0,(uchar *)"3.ش  4.ͳ");   
			//Os__GB2312_display(2,0,(uchar *)"5.  6.");	
			if(DataSave0.ChangeParamData.aucTermFlag[6])
				//Os__GB2312_display(3,0,(uchar *)"7.ֹ  8.ϵͳ  ");   
			else
				//Os__GB2312_display(3,0,(uchar *)"7.  8.ϵͳ  ");   
		}
		EventInfo->pt_eve_out->treatment_status = OK;
		break;	
	 case EVESEL + 0xFF:
		//UTIL_ClearGlobalData(); 
		//Os__clr_display(255);     
		if(DataSave0.ConstantParamData.ucKFCFlag)
		{
			//Os__GB2312_display(0,0,(uchar *)"1.  2.ش");
			//Os__GB2312_display(1,0,(uchar *)"3.ͳ  4.ǩ");   
			//Os__GB2312_display(2,0,(uchar *)"5.6.");	
			//Os__GB2312_display(3,0,(uchar *)"7.  8.ϵͳ  ");   
		}
		else
		{
			//Os__GB2312_display(0,0,(uchar *)"1.ǩ  2.");
			//Os__GB2312_display(1,0,(uchar *)"3.ش  4.ͳ");   
			//Os__GB2312_display(2,0,(uchar *)"5.  6.");	
			if(DataSave0.ChangeParamData.aucTermFlag[6])
				//Os__GB2312_display(3,0,(uchar *)"7.ֹ  8.ϵͳ  ");   
			else
				//Os__GB2312_display(3,0,(uchar *)"7.  8.ϵͳ  ");   
		}
		if(DataSave0.ConstantParamData.Pinpadflag)
		{
			//Os__clr_display_pp(255);
			//Os__GB2312_display_pp(0,0,(uchar *)"   ɼͨ");
			//Os__GB2312_display_pp(1,0,(uchar *)"    ӭʹ");
		}
		break;	
		//Ӧ水¼
	case EVESEL + 0xFE:
		//UTIL_ClearGlobalData(); 
		OnEve_KeyPress(ucKey);
		if(DataSave0.ConstantParamData.Pinpadflag)
		{
			//Os__clr_display_pp(255);
			//Os__GB2312_display_pp(0,0,(uchar *)"   ɼͨ");
			//Os__GB2312_display_pp(1,0,(uchar *)"    ӭʹ");
		}
		break;
	case EVESEL + EVE_MAN://
		//UTIL_ClearGlobalData(); 
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"ɼ֧");	
		MSG_WaitKey(1);
		ucResult=STL_Settle();
		if(ucResult!=SUCCESS)			
		{
			MSG_DisplayErrMsg(ucResult);
		}
		break;
	case EVESEL + EVE_RESELECT_LOGON://ǩ
		//UTIL_ClearGlobalData(); 
		//Os__clr_display(255);
		NORMALTRANS_ucTransType=TRANS_S_LOGONON;
		//Os__GB2312_display(0,0,(uchar *)"ɼ֧ǩ");
		MSG_WaitKey(1);
		ucResult=LOGON_LogonProcess();
		if(ucResult!=SUCCESS)			
		{
			MSG_DisplayErrMsg(ucResult);
		}
		break;
	case EVESEL + EVE_TREAT_PCSC:
		Os__xlinefeed(5);
		break;
	case EVECIR + 0x25: 
		//UTIL_ClearGlobalData(); 
		RunData.ucInputmode=0;
		ucResult = Untouch_Menu();
        //Os__clr_display_pp(255);
        //Os__GB2312_display_pp(0,0,(uchar *)"   ɼͨ");
        //Os__GB2312_display_pp(1,0,(uchar *)"    ӭʹ");
		if( ucResult == ERR_NOTSUCHAPP)
        {
            EventInfo->pt_eve_out->treatment_status = NOK;
        }
		else
        {
            EventInfo->pt_eve_out->treatment_status = OK;
        }
		break;
	case EVESEL + EVE_CASH_REGISTER:
		//UTIL_ClearGlobalData(); 

		if(DataSave0.ConstantParamData.ucYTJFlag)
		{
			memset(PORTAUCBUF,0,sizeof(PORTAUCBUF));
			memcpy(PORTAUCBUF,EventInfo->pt_eve_in->e_cash_register.ecr_msg,sizeof(PORTAUCBUF));
		     	if(!memcmp(&PORTAUCBUF[2],APPID_MEMBERCARD,8)
		     	||!memcmp(&PORTAUCBUF[2],APPID_SMARTCARD,8)
		     	||!memcmp(&PORTAUCBUF[2],APPID_WKCARD,8))
	     		{
				memcpy(HeadData.aucMuchAppNo,&PORTAUCBUF[2],8);
				memcpy(&ReceiveTransData_Gd,&PORTAUCBUF[18],sizeof(RECEIVETRANS_GD));
				RunData.ucGdflag=1;
				Port_Main_Process();
				EventInfo->pt_eve_out->treatment_status = OK; 	      	    
				EventInfo->pt_eve_in->e_cash_register.ecr_msg[10]=0x05;
			}
			else
			{
				
				PortData.ulRecieveLength = bcd_long(&PORTAUCBUF[1],4);
				PortData.ulHeadLength = sizeof(HEADDATA);
				if(PortData.ulRecieveLength<PortData.ulHeadLength)
				{
					EventInfo->pt_eve_out->treatment_status = NOK; 
					break;
				}
				PortData.ulRecieveLength -= PortData.ulHeadLength;
				memcpy(&HeadData,&PORTAUCBUF[3],PortData.ulHeadLength);
				memcpy(&ReceiveTransData,&PORTAUCBUF[PortData.ulHeadLength+3],PortData.ulRecieveLength);

				Uart_Printf("\nindata");
				for(i=0;i<100;i++)
				{
					if(i%10 == 0)
					Uart_Printf("\n");
					Uart_Printf("%02x ",PORTAUCBUF[i]);
				}
			     	if(!memcmp(&PORTAUCBUF[6],APPID_MEMBERCARD,8)
			     	||!memcmp(&PORTAUCBUF[6],APPID_SMARTCARD,8)
			     	||!memcmp(&PORTAUCBUF[6],APPID_WKCARD,8))
				{
					Port_Main_Process();
					memcpy(EventInfo->pt_eve_in->e_cash_register.ecr_msg,PortData.aucBuf,PortData.ulSendLength);
					EventInfo->pt_eve_out->treatment_status = OK; 	      	    
				}else
				{
					if(ReceiveTransData.aucOperType[0] == 'B')
					{
						if(!memcmp(ReceiveTransData.aucTransType,"82",2))
						{
							ucResult = Port_Query_trans();
							if(ucResult == SUCCESS)
								EventInfo->pt_eve_out->treatment_status = OK;
						}
					}else
					{
						if(ReceiveTransData.aucOperType[0] == 'A')
						{
							if(!(memcmp(ReceiveTransData.aucTransType,"90",2)
							&&memcmp(ReceiveTransData.aucTransType,"91",2)
							&&memcmp(ReceiveTransData.aucTransType,"92",2)))
							{
								memcpy(HeadData.aucCardType,"06",2);
								 Port_Main_Process();
								memcpy(EventInfo->pt_eve_in->e_cash_register.ecr_msg,PortData.aucBuf,PortData.ulSendLength);
							}
						} 
						EventInfo->pt_eve_out->treatment_status = NOK; 
						if(RunData.ucPCCOMMFlag == 0x31)
							EventInfo->pt_eve_out->treatment_status = OK; 
					}
				} 
			}
		}
		if(DataSave0.ConstantParamData.Pinpadflag)
		{
			//Os__clr_display_pp(255);
			//Os__GB2312_display_pp(0,0,(uchar *)"   ɼͨ");
			//Os__GB2312_display_pp(1,0,(uchar *)"    ӭʹ");
		}
		break;
	case 0x02 + 0x27:
		//UTIL_ClearGlobalData(); 
		//TMS_DownloadTms();
		break;
//	case EVESEL + EVE_GETINFO://GETINFO
//		UTIL_ClearGlobalData();
//		UTIL_GetTerminalInfo();
//		TMS_GetParam();		
//		memset(EventInfo->pt_eve_in->e_cash_register.ecr_msg,0x00,
//				sizeof(EventInfo->pt_eve_in->e_cash_register.ecr_msg));
//		memcpy(EventInfo->pt_eve_in->e_cash_register.ecr_msg,
//				(unsigned char *)CURRENT_PRJ,sizeof(CURRENT_PRJ));
//		memcpy(&EventInfo->pt_eve_in->e_cash_register.ecr_msg[20],
//				(unsigned char *)CURRENT_VER,sizeof(CURRENT_VER));
//		if(DataSave0.TransInfoData.TransTotal.uiTotalNb != 0)
//			EventInfo->pt_eve_in->e_cash_register.ecr_msg[50] = 1;
//		else
//			EventInfo->pt_eve_in->e_cash_register.ecr_msg[50] = 0;
//		int_bcd(&EventInfo->pt_eve_in->e_cash_register.ecr_msg[51],2,&uiUpdateLen);		
//		memcpy(&EventInfo->pt_eve_in->e_cash_register.ecr_msg[53],aucUpdateBuff,uiUpdateLen);
//		break;
//	case EVESEL + EVE_DOWNINFO://DOWNINFO
//
//		UTIL_ClearGlobalData();
//		UTIL_GetTerminalInfo();
//		memset(aucUpdateBuff,0x00,sizeof(aucUpdateBuff));
//		memcpy(aucUpdateBuff,&EventInfo->pt_eve_in->e_cash_register.ecr_msg[0],2);
//		uiUpdateLen=(unsigned short)tab_long(&EventInfo->pt_eve_in->e_cash_register.ecr_msg[2],2);
//		memset(aucUpdateBuff,0x00,sizeof(aucUpdateBuff));				
//		memcpy(aucUpdateBuff,&EventInfo->pt_eve_in->e_cash_register.ecr_msg[4],uiUpdateLen);
//		TMSAPP_UpdateTMSData_ADS(TMS_WriteFile);
//		break;
	default:
		break;						
	}
}
#pragma arm section code ,rwdata
#endif

unsigned char MASAPP_Trans2Other(void)
{
	unsigned int uiTimeout,index,ucI,ucJ;
	unsigned char ucResult=SUCCESS,ucResult1;
	unsigned char ucMenuPage;
	unsigned char ucCount,ucChar;
	int  uiSendDataLen;
	unsigned char ucKey,ucStatus,aucOutData[200],ucLen;
	unsigned char i;
	unsigned char aucBuf[200],aucTraceTicket[6],aucCommBuf1[512],aucCommBuf2[512];
	struct seven even;
	union seve_in even_in;
	struct seve_out even_out;
	struct seve_cash_register *pxComEvent;


	even.event_type = EVESEL;
	even.event_nr = EVE_CASH_REGISTER;
	pxComEvent = &(even.pt_eve_in->e_cash_register);
	memset(&HeadData,0,sizeof(HEADDATA));
	memcpy(&HeadData,"102B000062501100100",19);
	memcpy(HeadData.aucOvertime,"120",3);
	memset(&ReceiveTransData,0,sizeof(RECEIVETRANS));
	memcpy(ReceiveTransData.aucOperType,"A0",2);
	memcpy(ReceiveTransData.aucTransType,"30",2);
	memcpy(ReceiveTransData.aucCashierNo,&DataSave0.ConstantParamData.aucTerminalID[2],6);
	memcpy(ReceiveTransData.aucCashier,"555555",6);
	long_asc(ReceiveTransData.aucAmount,12,&NormalTransData.ulAmount);
	long_asc(ReceiveTransData.aucCashierTrace,6,&NormalTransData.ulTraceNumber);
       memset(aucCommBuf2,0,sizeof(aucCommBuf2));
        memcpy(aucCommBuf2,&HeadData,sizeof(HEADDATA));
	index=sizeof(HEADDATA);
        memcpy(&aucCommBuf2[index],&ReceiveTransData,sizeof(RECEIVETRANS));
	uiSendDataLen=sizeof(HEADDATA)+sizeof(RECEIVETRANS);
        rs232_PackData(uiSendDataLen,aucCommBuf2,aucCommBuf1);
	memset(pxComEvent->ecr_msg,0,sizeof(pxComEvent->ecr_msg));
	memcpy(pxComEvent->ecr_msg,aucCommBuf1,sizeof(pxComEvent->ecr_msg));
        for(i=0;i<uiSendDataLen;i++)
        {
            if(i%10 == 0)
                Uart_Printf("\n");
            Uart_Printf("%02x  ",aucCommBuf1[i]);
        }


//	OSAPP_EventRun(&even);

	if(even.pt_eve_out->treatment_status == OK)
	{
		memcpy(&SendTransData,&even.pt_eve_in->e_cash_register.ecr_msg[26],sizeof(SENDTRANS));
		if(!memcmp(SendTransData.aucRespCode,"00",2))
			return SUCCESS;
	}
	return ERR_CREDITFAIL;
}

