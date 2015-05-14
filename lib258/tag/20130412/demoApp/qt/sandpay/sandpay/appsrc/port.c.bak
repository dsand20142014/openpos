#include <include.h>
#include <xdata.h>
#include <global.h>


unsigned char Port_Main_Process(void)
{
	 unsigned char ucResult = SUCCESS;
	 
	if(RunData.ucGdflag==1)
	{
		memcpy( ReceiveTransData.aucCashierTrace, ReceiveTransData_Gd.aucCashierTrace,6);
		memcpy(ReceiveTransData.aucCashier, ReceiveTransData_Gd.aucCashier,6);
		memcpy( ReceiveTransData.aucOperType, ReceiveTransData_Gd.aucOperType,2);
		memcpy(ReceiveTransData.aucTransType, ReceiveTransData_Gd.aucTransType,2);
		memcpy(HeadData.aucCardType, ReceiveTransData_Gd.aucCardType,2);
		memcpy( ReceiveTransData.aucCashierNo, ReceiveTransData_Gd.aucCashierNo,6);
		memcpy(ReceiveTransData.aucAmount, ReceiveTransData_Gd.aucAmount,12);
	}
	if(!memcmp(HeadData.aucCardType,"24",2)	)
	{
		RunData.ucDxMobile=1;
		memcpy(HeadData.aucCardType,"06",3);
	}
	if(ucResult==SUCCESS)
 	{
		 switch(ReceiveTransData.aucOperType[0])
		 {
		    case 'A':  //
		    	  ucResult = Port_Card_trans();
		    	  break;
		    case 'B':   //ѯ
		    	  ucResult = Port_Query_trans();
		    	  break;
		    default:
		    	  ucResult = ERR_UNKNOWOPERATCODE;
			    	rs232_SendCancelData();
		    	  break;	
		 }
 	}
	 if(ucResult != SUCCESS)
	 	 MSG_DisplayErrMsg(ucResult);
	 return ucResult;
}


unsigned char Port_Card_trans(void)
{
	unsigned char ucResult = SUCCESS,ucpt=0,aucBuf[40],ucStatus,aucOutData[255],ucLen;
	NORMALTRANS tNormal;
	memcpy(NormalTransData.aucCashTicket, ReceiveTransData.aucCashierTrace,6);
	memcpy(RunData.aucCashID, ReceiveTransData.aucCashier,6);
	switch(ReceiveTransData.aucTransType[0])
	{
		case '8':
			if(ucResult==SUCCESS)
				ucResult=CFG_CheckFlag();
			if(ucResult==SUCCESS)
			{
				ucResult=Untouch_WaitAllCard();
			}
			if(ucResult==SUCCESS)
			{
				if(NormalTransData.ucCardType==CARD_MEM)
				{
					if(ucResult == SUCCESS )
						SLE4442_ATR(aucBuf);

					if(ucResult==SUCCESS)
						ucResult = SLE4442_ReadCardData();
					if(ucResult==SUCCESS)
						ucResult=CFG_CheckCardValid();
					if(ucResult==SUCCESS)
						CFG_SetBatchNumber();
					if(ucResult==SUCCESS)
						ucResult=Untouch_CheckRedo();

				}
				if(NormalTransData.ucCardType==CARD_M1)
				{
					if(ucResult==SUCCESS)
						ucResult=Untouch_ReadCardInfo();
					if(ucResult==SUCCESS)
						ucResult=CFG_SetBatchNumber();

					if(ucResult==SUCCESS)
						ucResult=CFG_CheckCardValid();

					if(ucResult==SUCCESS)
						ucResult=Untouch_CheckRedo();

				}
			}		   	
			if(ucResult==SUCCESS)
			{
				NormalTransData.ulAmount=NormalTransData.ulPrevEDBalance;
				memset(aucBuf,0,sizeof(aucBuf));
				bcd_asc(aucBuf,NormalTransData.aucUserCardNo,16);
				aucBuf[16]=0x00;
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)": ");
				Os__display(1,0,aucBuf);
				memset(aucBuf,0,sizeof(aucBuf));
				UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulAmount,2);
				memcpy(aucBuf,":",5);
				aucBuf[16]=0x00;
				//Os__GB2312_display(2,0,aucBuf);
			}
			Port_SendPortResult(ucResult);
			if(ucResult>ERR_CANCEL)
				MSG_DisplayErrMsg(ucResult);
			//Os__clr_display(3);
			//Os__GB2312_display(3,0,(uchar *)"γ");
			Os__xdelay(10);
			Os__ICC_remove();
			while(1)
			{
				ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
				if(ucResult==0x04)
				{
					ucResult= SUCCESS;
					break;
				}
				//Os__clr_display(3);
				//Os__GB2312_display(3,0,"ȡ");
			}
			break;
		case '2':
		case '7':
			if(asc_long(ReceiveTransData.aucAmount,12)==0&&memcmp(ReceiveTransData.aucTransType,"76",2))
				ucResult=ERR_AMOUNTERR;
			if(asc_long(ReceiveTransData.aucAmount,12)>DataSave0.ConstantParamData.ulMaxXiaoFeiAmount)
			   	ucResult=ERR_VOIDDEPOSITAMOUNT;
			if(ucResult==SUCCESS)
				ucResult=CFG_CheckFlag();
			if(!memcmp(ReceiveTransData.aucTransType,"22",2))
				NormalTransData.ucTransType=TRANS_S_PAYNOBAR;
			else if(!memcmp(ReceiveTransData.aucTransType,"28",2))
				NormalTransData.ucTransType=TRANS_S_PURCHASE;
			else if(!memcmp(ReceiveTransData.aucTransType,"72",2))
				NormalTransData.ucTransType=TRANS_S_LOADONLINE;
			else if(!memcmp(ReceiveTransData.aucTransType,"76",2))
				NormalTransData.ucTransType=TRANS_S_PRELOAD;
			if(ucResult==SUCCESS)
				ucResult=SAV_SavedTransIndex(0);
			if(ucResult==ERR_TRANSFILEFULL)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(unsigned char *)"");
				//Os__GB2312_display(1,0,(unsigned char *)"1.");
				//Os__GB2312_display(2,0,(unsigned char *)"[].˳");
				if(MSG_WaitKey(20)=='1')
				{
					 memcpy(&tNormal,&NormalTransData,sizeof(NORMALTRANS));
		   	   	  	 ucResult =  STL_Settle();
					 memcpy(&NormalTransData,&tNormal,sizeof(NORMALTRANS));
					 NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
					if(ucResult==SUCCESS)
						ucResult=SAV_SavedTransIndex(0);
				}

			}
			if(RunData.ucDxMobile)
			{
				if(ucResult==SUCCESS)
					ucResult=Untouch_DX_WaitCard();
			}
			else
			{
				if(ucResult==SUCCESS)
				{
					ucResult=Untouch_WaitAllCard();
				}
				if(ucResult==SUCCESS)
				{
					if(NormalTransData.ucCardType==CARD_MEM)
					{
						if(ucResult == SUCCESS )
							SLE4442_ATR(aucBuf);

						if(ucResult==SUCCESS)
							ucResult = SLE4442_ReadCardData();
						if(ucResult==SUCCESS)
							ucResult=CFG_CheckCardValid();
						if(ucResult==SUCCESS)
							CFG_SetBatchNumber();
						if(ucResult==SUCCESS)
							ucResult=Untouch_CheckRedo();

					}
					if(NormalTransData.ucCardType==CARD_M1)
					{
						if(ucResult==SUCCESS)
							ucResult=Untouch_ReadCardInfo();
						if(ucResult==SUCCESS)
							ucResult=CFG_SetBatchNumber();

						if(ucResult==SUCCESS)
							ucResult=CFG_CheckCardValid();

						if(ucResult==SUCCESS)
							ucResult=Untouch_CheckRedo();

					}
				}
			}
			/*
			if(ucResult==SUCCESS)
			{
				if(memcmp(HeadData.aucCardType,"00",2))
				{
					if((!memcmp(HeadData.aucCardType,"03",2)&&NormalTransData.ucPtCode!=0x02)
					||(!memcmp(HeadData.aucCardType,"06",2)&&NormalTransData.ucPtCode!=0x01)
					||(!memcmp(HeadData.aucCardType,"02",2)&&NormalTransData.ucPtCode!=0x03))
					{
						ucResult=ERR_CARDTYPE;
					}
				}
			}
			*/
			if(NormalTransData.ucTransType==TRANS_S_PAYNOBAR)
			{
				if(ucResult==SUCCESS)
				{
					if(RunData.aucZDFunFlag[1]==0&&RunData.aucZDFunFlag[13]==0
						||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1))
						ucResult=ERR_NOFUNCTION;
				}
				if(ucResult==SUCCESS)
				{
					if(RunData.ucMustReturn)
					{
						if(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1)
							ucResult= ERR_MUSTRETURN;
					}
					if(SleTransData.ucStatus==CARD_RECYCLED)
						ucResult=ERR_CARDRECYCLE;
					if(SleTransData.ucStatus==0x04&&!RunData.ucDxMobile)	
						ucResult=ERR_CARDNOACT;
				}
				if(ucResult==SUCCESS)
				{
					if(RunData.ucExpireFlag)
						ucResult=ERR_EXPIREDATE;
				}
				if(ucResult==SUCCESS)
			   	   	   ucResult = Untouch_PayNobar();   	
			   	else
					Port_SendPortResult(ucResult);
			}
			if(NormalTransData.ucTransType==TRANS_S_PURCHASE)
			{
				if(ucResult==SUCCESS)
		   	   	   ucResult = Untouch_Purchase();   	
		   		else
					Port_SendPortResult(ucResult);
			}
			if(NormalTransData.ucTransType==TRANS_S_LOADONLINE)
			{
				if(ucResult==SUCCESS)
			   	   	   ucResult = Untouch_LoadOnline();   	
			   	else
					Port_SendPortResult(ucResult);
			}
			if(NormalTransData.ucTransType==TRANS_S_PRELOAD)
			{
				if(ucResult==SUCCESS)
				{
					if(RunData.aucZDFunFlag[23]==0)
						ucResult=ERR_NOFUNCTION;
				}
				if(ucResult==SUCCESS)
				{
					if(SleTransData.ucStatus==0x04&&!RunData.ucDxMobile)	
						ucResult=ERR_CARDNOACT;
				}
				if(ucResult==SUCCESS)
			   	   	   ucResult = Untouch_Preload();
			   	else
					Port_SendPortResult(ucResult);
			}
			if(ucResult>ERR_CANCEL)
				MSG_DisplayErrMsg(ucResult);
			//Os__clr_display(255);
			//Os__GB2312_display(1,0,(uchar *)"γ");
			Os__xdelay(10);
			Os__ICC_remove();
			while(1)
			{
				ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
				if(ucResult==0x04)
				{
					ucResult= SUCCESS;
					break;
				}
				//Os__clr_display(1);
				//Os__GB2312_display(1,0,"ȡ");
			}
	   	   	   break;
		case '3':
			if(asc_long(ReceiveTransData.aucAmount,12)==0)
				ucResult=ERR_AMOUNTERR;
			if(asc_long(ReceiveTransData.aucAmount,12)>DataSave0.ConstantParamData.ulMaxXiaoFeiAmount)
			   	ucResult=ERR_VOIDDEPOSITAMOUNT;
			if(ucResult==SUCCESS)
				ucResult=CFG_CheckFlag();
			NormalTransData.ucTransType=TRANS_S_PURCHASE;
			if(ucResult==SUCCESS)
				ucResult=SAV_SavedTransIndex(0);
			if(ucResult==ERR_TRANSFILEFULL)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(unsigned char *)"");
				//Os__GB2312_display(1,0,(unsigned char *)"1.");
				//Os__GB2312_display(2,0,(unsigned char *)"[].˳");
				if(MSG_WaitKey(20)=='1')
				{
					 memcpy(&tNormal,&NormalTransData,sizeof(NORMALTRANS));
		   	   	  	 ucResult =  STL_Settle();
					 memcpy(&NormalTransData,&tNormal,sizeof(NORMALTRANS));
					 NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
					if(ucResult==SUCCESS)
						ucResult=SAV_SavedTransIndex(0);
				}

			}
			if(RunData.ucDxMobile)
			{
				if(ucResult==SUCCESS)
					ucResult=Untouch_DX_WaitCard();
			}
			else
			{
				if(ucResult==SUCCESS)
				{
					if(!memcmp(ReceiveTransData.aucHoldData,"һ",4))
					{
						NormalTransData.ucYhflag=ReceiveTransData.aucHoldData[4];
						memcpy(NormalTransData.aucBarCode,&ReceiveTransData.aucHoldData[5],16);
					}
					if(RunData.ucGdflag==1||!memcmp(ReceiveTransData.aucHoldData,"һ",4))
					{
						//Os__clr_display(255);
						//Os__GB2312_display(1,0,"()ɼ¿");
                    //	if(!memcmp(HeadData.aucCardType,"06",2)	)
						       //Os__GB2312_display(3,0,"F4->֧");
					}
					ucResult=Untouch_WaitAllCard1();
					if(ucResult==KEY_F4)
					{
						RunData.ucDxMobile=1;
						ucResult=Untouch_DX_WaitCard();
					}
				}
				if(ucResult==SUCCESS&&RunData.ucDxMobile!=1)
				{
					if(NormalTransData.ucCardType==CARD_MEM)
					{
						if(ucResult == SUCCESS )
							SLE4442_ATR(aucBuf);

						if(ucResult==SUCCESS)
							ucResult = SLE4442_ReadCardData();
						if(ucResult==SUCCESS)
							ucResult=CFG_CheckCardValid();
						if(ucResult==SUCCESS)
							CFG_SetBatchNumber();
						if(ucResult==SUCCESS)
							ucResult=Untouch_CheckRedo();

					}
					if(NormalTransData.ucCardType==CARD_M1)
					{
						if(ucResult==SUCCESS)
							ucResult=Untouch_ReadCardInfo();
						if(ucResult==SUCCESS)
							ucResult=CFG_SetBatchNumber();

						if(ucResult==SUCCESS)
							ucResult=CFG_CheckCardValid();

						if(ucResult==SUCCESS)
							ucResult=Untouch_CheckRedo();

					}
				}
			}
			/*
			if(ucResult==SUCCESS)
			{
				if(memcmp(HeadData.aucCardType,"00",2))
				{
					if((!memcmp(HeadData.aucCardType,"03",2)&&NormalTransData.ucPtCode!=0x02)
					||(!memcmp(HeadData.aucCardType,"06",2)&&NormalTransData.ucPtCode!=0x01)
					||(!memcmp(HeadData.aucCardType,"02",2)&&NormalTransData.ucPtCode!=0x03))
					{
						ucResult=ERR_CARDTYPE;
					}
				}
			}
			*/
			if(ucResult==SUCCESS)
			{
				if(RunData.aucZDFunFlag[0]==0||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1))
					ucResult=ERR_NOFUNCTION;
			}
			if(ucResult==SUCCESS)
			{
				if(RunData.ucMustReturn||NormalTransData.ucZeroFlag)
				{
					if(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1)
						ucResult= ERR_MUSTRETURN;
				}
				if(SleTransData.ucStatus==CARD_RECYCLED)
					ucResult=ERR_CARDRECYCLE;
				if(SleTransData.ucStatus==0x04&&!RunData.ucDxMobile)	
					ucResult=ERR_CARDNOACT;
			}
			if(ucResult==SUCCESS)
			{
				if(RunData.ucExpireFlag)
					ucResult=ERR_EXPIREDATE;
			}
			if(ucResult==SUCCESS)
		   	   	   ucResult = Untouch_Purchase();   	
		   	else
				Port_SendPortResult(ucResult);
			if(ucResult>ERR_CANCEL)
				MSG_DisplayErrMsg(ucResult);
			//Os__clr_display(255);
			//Os__GB2312_display(1,0,(uchar *)"γ");
			Os__xdelay(10);
			Os__ICC_remove();
			while(1)
			{
				ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
				if(ucResult==0x04)
				{
					ucResult= SUCCESS;
					break;
				}
				//Os__clr_display(1);
				//Os__GB2312_display(1,0,"ȡ");
			}
	   	   	   break;
		case '9':
			switch(ReceiveTransData.aucTransType[1])
			{
				case '2'://
					ucResult =  STL_Settle();
					PackPortResult(ucResult);
					return ucResult;
				case '1'://ǩ
					ucResult =  LOGON_LogonProcess();
					PackPortResult(ucResult);
					return ucResult;
				case '0'://ʼ
					  ucResult =   LOGON_IniProcess();
					PackPortResult(ucResult);
					return ucResult;
				default:
					ucResult = ERR_UNKNOWTRANSTYPE;
					rs232_SendCancelData();
					return ucResult;
			}
			break;
		default:
			ucResult = ERR_UNKNOWTRANSTYPE;
			rs232_SendCancelData();
			MSG_DisplayErrMsg(ucResult);
			return ucResult;
	}
	return ucResult;
}


unsigned char Port_Query_trans(void)
{
      unsigned char ucResult = SUCCESS;
	unsigned int uiIndex;
	unsigned char k;
	unsigned long ulAmount;
	unsigned char aucBuf[7];

	 ulAmount = asc_long(ReceiveTransData.aucAmount,12);
	
	 
	 
	for(uiIndex = 0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
	      if( DataSave0.TransInfoData.auiTransIndex[uiIndex] == 0)
		  	break;
	     
	}	
       if(uiIndex == 0)
	    ucResult =  ERR_NOTRANS;	
	 else
	    uiIndex --;
	 
          if(ucResult == SUCCESS)
	  {
		XDATA_Read_SaveTrans_File_2Datasave(uiIndex);
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,&DataSave0.SaveTransData.aucDateTime[1],6);

		Uart_Printf("\nReceiveTransData.aucAmount=%d",ulAmount);
		Uart_Printf("\nDataSave0.SaveTrans.ulAmount=%d",DataSave0.SaveTransData.ulAmount);
		Uart_Printf("\nReceiveTransData.aucCashierTrace\n");
		for(k=0;k<6;k++)
			Uart_Printf("%02x  ",ReceiveTransData.aucCashierTrace[k]);
		Uart_Printf("\nDataSave0.SaveTrans.ucCashTicket\n");
		for(k=0;k<6;k++)
			Uart_Printf("%02x  ",DataSave0.SaveTransData.aucCashTicket[k]);
		Uart_Printf("\nReceiveTransData.aucOldDateTime\n");
		for(k=0;k<8;k++)
			Uart_Printf("%02x  ",ReceiveTransData.aucOldDateTime[k]);
		Uart_Printf("\naucBuf\n");
		for(k=0;k<6;k++)
			Uart_Printf("%02x  ",aucBuf[k]);
		if((!memcmp(ReceiveTransData.aucCashierTrace,DataSave0.SaveTransData.aucCashTicket,6))
		&&(ulAmount == DataSave0.SaveTransData.ulAmount)
		/*&&(!memcmp(&ReceiveTransData.aucOldDateTime[2],aucBuf,6))*/)
		{
			   memset((unsigned char *)&NormalTransData,0,sizeof(NormalTransData));
			   memcpy(&NormalTransData,&DataSave0.SaveTransData,sizeof(NORMALTRANS));
		}else
			  ucResult =  ERR_NOTRANS;	
	    }
	    if(ucResult == SUCCESS)
	         Port_SendPortResult(ucResult);
	   return ucResult;
}





unsigned char Port_SendPortResult(unsigned char ucResp)
{
	   unsigned char ucResult = SUCCESS;
	   unsigned int uiLen;
	   unsigned char Temp[40],aucBuf[7],aucMid[16];
	   unsigned char Date[4];
	   unsigned long	Balance;
	   unsigned int i;
	  
	   
	   memset(PORTAUCBUF1,0,sizeof(PORTAUCBUF1));
	   memset(PORTAUCBUF2,0,sizeof(PORTAUCBUF2));
	   HeadData.ucMessageGoon = 0;
	   memset((unsigned char *)&SendTransData,0,sizeof(SendTransData));
	   switch(NormalTransData.ucPtCode)
	   {
		case 1:
		   memcpy(HeadData.aucCardType,"06",2);
		   break;
		case 2:
		   memcpy(HeadData.aucCardType,"03",2);
		   break;
		case 3:
		   memcpy(HeadData.aucCardType,"02",2);
		   break;
		default:
		   break;
	   }
	   if(RunData.ucGdflag==1)
	   {
		PortData.ulHeadLength=16;
		memcpy(PORTAUCBUF2,PORTAUCBUF,16);
		memcpy(&PORTAUCBUF2[8],"\x05\x00\x00\x00\x00",5);
	   }
	   else
		   memcpy(PORTAUCBUF2,&HeadData,PortData.ulHeadLength);
	   
	   memcpy(SendTransData.aucOperType,ReceiveTransData.aucOperType,2);
	   SendTransData.aucOperType[1] =0x31;
	   memcpy(SendTransData.aucTransType,ReceiveTransData.aucTransType,2);
	 //  SendTransData.aucOperType[1] += 1;
	  // memcpy(SendTransData.CardType,ReceiveTransData.aucCardType,2);

	 //96822

	  memset(aucBuf,0,sizeof(aucBuf));
          bcd_asc(aucBuf,NormalTransData.aucZoneCode,6);

	   switch(NormalTransData.ucPtCode)
	   {
		case 1:
		   memcpy(SendTransData.CardType,"06",2);
                 memcpy(SendTransData.aucCardType,"810",3);
		   memcpy(&SendTransData.aucCardType[3],aucBuf,3);	 
		   break;
		case 2:
		   memcpy(SendTransData.CardType,"03",2);
                 memcpy(SendTransData.aucCardType,"820",3);
		   memcpy(&SendTransData.aucCardType[3],&aucBuf[3],3);	 
		   break;
		case 3:
		   memcpy(SendTransData.CardType,"02",2);
                 memcpy(SendTransData.aucCardType,"83",2);
		   memcpy(&SendTransData.aucCardType[2],aucBuf,4);	 
		   break;
		default:
		   memcpy(SendTransData.CardType,HeadData.aucCardType,2);
		   break;
	   }
	   hex_asc(SendTransData.aucRespCode,&ucResp,2);
		 memset(Temp,0,sizeof(Temp));		 
		MSG_GetErrMsg(ucResp, Temp);
		 memcpy(SendTransData.aucRespInfo,Temp,40);
	   memcpy(SendTransData.aucCashRegNo,ReceiveTransData.aucCashierNo,6);
	   memcpy(SendTransData.aucCashier,ReceiveTransData.aucCashier,6);
	   long_asc(SendTransData.aucAmount,12,&NormalTransData.ulAmount);	   
	  
	   
	   bcd_asc(aucMid,DataSave0.ConstantParamData.aucMid[NormalTransData.ucPtCode-1],16);
	   memcpy(SendTransData.aucMerchID,aucMid,15);

	   memcpy(SendTransData.aucMerchantName,DataSave0.ConstantParamData.aucMerchantName,40);
	   bcd_asc(SendTransData.aucTermID,DataSave0.ConstantParamData.aucTid[NormalTransData.ucPtCode-1],8);
	   bcd_asc(SendTransData.aucCardId,NormalTransData.aucUserCardNo,16);
	   memset(Date,0,sizeof(Date));
	   bcd_asc(&Date[2],&NormalTransData.aucExpiredDate[1],2);
		 bcd_asc(Date,&NormalTransData.aucExpiredDate[2],2);
		 memcpy(SendTransData.aucValidDate,Date,4);
	   bcd_asc(SendTransData.aucTransDate,&NormalTransData.aucDateTime[0],8);
	   bcd_asc(SendTransData.aucTransTime,&NormalTransData.aucDateTime[4],6);
	   memcpy(SendTransData.aucCashTraceNo,NormalTransData.aucCashTicket,6);
	   long_asc(SendTransData.aucTraceNo,6,&NORMALTRANS_ulTraceNumber);

	   
	   if(NormalTransData.ulPrevEDBalance<NORMALTRANS_ulAmount)
	   	   Balance = 0;
	   else
	       Balance=NormalTransData.ulPrevEDBalance-NORMALTRANS_ulAmount;
	   long_asc(SendTransData.aucHoldData,12,&Balance);
          bcd_asc(&SendTransData.aucHoldData[12],DataSave0.ChangeParamData.aucBatchNumber,24);
     
	   memcpy(&PORTAUCBUF2[PortData.ulHeadLength],&SendTransData,sizeof(SendTransData));
	   PortData.ulSendLength = PortData.ulHeadLength+ sizeof(SendTransData);
	   
	   ucResult = rs232_PackData(PortData.ulSendLength,PORTAUCBUF2,PORTAUCBUF1);
	   uiLen = (unsigned int)PortData.ulSendLength + 5;

          Uart_Printf("\nrs232_SendData");
         for(i=0;i<uiLen;i++)
         {
             if(i%10 == 0)
              Uart_Printf("\n");
		Uart_Printf("%02x  ",PORTAUCBUF1[i]);
         }
    

	   
	   if(ucResult == SUCCESS)
	   {
		  RunData.ucComType=DataSave0.ConstantParamData.ucCom;
	   	  rs232_InitCOM();
		  ucResult = rs232_SendData(PORTAUCBUF1,uiLen);
		   memset(PortData.aucBuf,0,sizeof(PortData.aucBuf));
		   memcpy(PortData.aucBuf,PORTAUCBUF1,PortData.ulSendLength);
	   }
	   
	   return ucResult;
}




unsigned char Port_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen)
{
	    unsigned char ucResult = SUCCESS;
	   unsigned char ucChar,ucBuf[17],aucComVersion[9],aucBuf[30];
	   unsigned int uiTimesOut,uiI;
	  unsigned char 	aucTPDU[]={ 0x60,0x00,0x03,0x80,0x00,0xAA,0xBB,0x31,0x31};
          unsigned int 	uiLen=0,i;
	   unsigned short 	OutLen=0;
	  memset(aucComVersion,0,sizeof(aucComVersion));	
	  memcpy(aucComVersion,RunData.aucComVersion,5);
	   
	   MSG_DisplayMsg( true,0,0,MSG_SENDING);
	    RunData.ucPCCOMMFlag = 0;
	   memset(RunData.aucPCCOMMErrCode,0,2);
	   memset(RunData.aucPCCOMMErrData,0,40);
	   memset(PORTAUCBUF1,0,sizeof(PORTAUCBUF1));
	   memset(PORTAUCBUF2,0,sizeof(PORTAUCBUF2));
	   HeadData.ucMessageGoon = 0x55;
	   memcpy(HeadData.aucOvertime,"120",3);
	   switch(NormalTransData.ucPtCode)
	   {
		case 1:
		   memcpy(HeadData.aucCardType,"06",2);
		   break;
		case 2:
		   memcpy(HeadData.aucCardType,"03",2);
		   break;
		case 3:
		   memcpy(HeadData.aucCardType,"02",2);
		   break;
		default:
		   break;
	   }
	   memcpy(PORTAUCBUF2,&HeadData,PortData.ulHeadLength);
          uiLen=uiInLen+18;
          int_bcd(&PORTAUCBUF1[0],2,&uiLen);
          memcpy(&PORTAUCBUF1[2],aucTPDU,9);
	   memcpy(&PORTAUCBUF1[11],aucComVersion,9);
		
	   memcpy(&PORTAUCBUF1[20],pucInData,uiInLen);
	    ucResult = COMMSENCRYPT_SetVersion("1020");
	    if( ucResult != SUCCESS )
	    {
	    	Uart_Printf("COMMSENCRYPT_SetVersion ucResult = %02x\n",ucResult);
	    	COMMSENCRYPT_ErrMsg(ucResult);
	    	return ERR_CANCEL;
	    }


	   Uart_Printf("\n********************\n");
	   Uart_Printf("\nuiInLen=%d",uiInLen);
          ucResult=DataEncrypt(PORTAUCBUF1,(uiInLen+20),DataSave0.ConstantParamData.aucTerminalID,
										PORTAUCBUF1,&OutLen);
          Uart_Printf("\nOutLen=%d",OutLen);
	   
	   memcpy(&PORTAUCBUF2[PortData.ulHeadLength],PORTAUCBUF1,OutLen);
	   PortData.ulSendLength = PortData.ulHeadLength+ (unsigned long)OutLen;
	   memset(PORTAUCBUF1,0,sizeof(PORTAUCBUF1));
	   ucResult = rs232_PackData(PortData.ulSendLength,PORTAUCBUF2,PORTAUCBUF1);
	   uiLen = (unsigned int)PortData.ulSendLength + 5;

          for(i=0;i<uiLen;i++)
          {
               if(i%10 ==0)
			Uart_Printf("\n");
		  Uart_Printf("%02x ",PORTAUCBUF1[i]);
          }
           Uart_Printf("\nsendlen=%d",uiLen);
/*		  memset(aucBuf,0,sizeof(aucBuf));
		  int_asc(aucBuf,3,&uiLen);
		  Os__xprint(aucBuf); 
          for(i=0;i<uiLen;i+=10)
          	{
		  memset(aucBuf,0,sizeof(aucBuf));
		  hex_asc(aucBuf,&PORTAUCBUF1[i],20);
		  Os__xprint(aucBuf); 
          	}
*/	   
	   if(ucResult == SUCCESS)
	   {
		  RunData.ucComType=DataSave0.ConstantParamData.ucCom;
	   	  rs232_InitCOM();
	      ucResult = rs232_SendData(PORTAUCBUF1,uiLen);
	    
	   }
	   
	   if(ucResult == SUCCESS)
	   {
	   	  uiTimesOut = 120*100;
	   	  Os__timer_start(&uiTimesOut);
		  MSG_DisplayMsg( true,0,0,MSG_RECEIVING);
		   memset(ucBuf,0x00,sizeof(ucBuf));
		   memcpy(ucBuf,"ʹ    ",14); 
		   //Os__GB2312_display(3,0,(unsigned char*)"(ɼ֧)");
	   	  while(1)
	   	  {
			 uiI = 120-uiTimesOut/100;
	   	        int_asc(ucBuf+9,3,&uiI);
		        //Os__GB2312_display(1,0,ucBuf);
	   	  	 ucResult = rs232_ComRecvByte(&ucChar,100);
		    	 if(!ucResult)
		    	 {
						Uart_Printf("recv 1st success~~\n");
		    			break;
		    	 }
		    	 if(!uiTimesOut)
	   	  	 	  break;
	   	  }
	   	  Os__timer_stop(&uiTimesOut);
	   	  if(!uiTimesOut)
	   	  {
	   	  	  return ERR_COMMS_RECVTIMEOUT;
	   	  }
	   	  if(!ucResult)
	   	  {
	   	  	  while(ucResult == OK)
					{
						if(ucChar == 0x02)
						{
							break;
						}
						else
						{
						      Os__xdelay(50);
						      ucResult = rs232_ComRecvByte(&ucChar,1);
						}
					}
					 
					if(ucResult == SUCCESS)
					{
						memset(PORTAUCBUF1,0,sizeof(PORTAUCBUF1));
						PORTAUCBUF1[0] = ucChar;
						ucResult = rs232_InceptData(&PORTAUCBUF1[1]);
					}
					else if(ucResult == ERR_COMMS_RECVTIMEOUT)
					{
						return ERR_COMMS_PROTOCOL;
					}		
					//ն
				  while(1)
				  {
				   		if(rs232_ComRecvByte(&ucChar,10) != SUCCESS)
				   			break;
				  }
				  if(ucResult == SUCCESS)
				  {
				         if(!memcmp(&PORTAUCBUF1[6],"CANCEL",6))
				  	   {
				  	   	   RunData.ucPCCOMMFlag = 0x31;
						   memcpy(RunData.aucPCCOMMErrCode,&PORTAUCBUF1[26],42);
						   return ERR_CANCEL;
				  	   }
				          PortData.ulRecieveLength = bcd_long(&PORTAUCBUF1[1],4);
					     for(i=0;i<PortData.ulRecieveLength+5;i++)
				          {
				               if(i%10 ==0)
							Uart_Printf("\n");
						  Uart_Printf("%02x  ",PORTAUCBUF1[i]);
				          }
				           Uart_Printf("\nPortData.ulRecieveLength=%d",PortData.ulRecieveLength+5); 
					   memset(PORTAUCBUF2,0,sizeof(PORTAUCBUF2));
					   ucResult = DataDecrypt(&PORTAUCBUF1[3+PortData.ulHeadLength],PORTAUCBUF2,puiOutLen);
					    if(ucResult == SUCCESS)
					    {
				                *puiOutLen -= 8;
				  	          memcpy(pucOutData,&PORTAUCBUF2[21],*puiOutLen);

						    for(i=0;i<*puiOutLen;i++)
					          {
					               if(i%10 ==0)
								Uart_Printf("\n");
							  Uart_Printf("%02x  ",PORTAUCBUF2[21+i]);
					          }
					           Uart_Printf("\n*puiOutLen=%d",*puiOutLen);
					   }
				  }
			  }
	   }
	   
	   return ucResult;  
}


unsigned char PackPortResult(unsigned char ucResp)
{
	  unsigned char ucResult = SUCCESS;
	   unsigned int uiLen;
	   unsigned char Temp[40];
	   unsigned char Date[4];
	   unsigned long	Balance;
	  
	   
	   memset(PORTAUCBUF1,0,sizeof(PORTAUCBUF1));
	   memset(PORTAUCBUF2,0,sizeof(PORTAUCBUF2));
	   HeadData.ucMessageGoon = 0;
	   memset((unsigned char *)&SendTransData,0,sizeof(SendTransData));
	   memcpy(PORTAUCBUF2,&HeadData,PortData.ulHeadLength);
	   
	   memcpy(SendTransData.aucOperType,ReceiveTransData.aucOperType,2);
	   SendTransData.aucOperType[1] =0x31;
	   memcpy(SendTransData.aucTransType,ReceiveTransData.aucTransType,2);
	//   SendTransData.aucTransType[1] += 1;
	//   memcpy(SendTransData.CardType,ReceiveTransData.aucCardType,2);
	   memcpy(SendTransData.CardType,HeadData.aucCardType,2);
	   hex_asc(SendTransData.aucRespCode,&ucResp,2);
		 memset(Temp,0,sizeof(Temp));		 
		 MSG_GetErrMsg(ucResp, Temp);
		 memcpy(SendTransData.aucRespInfo,Temp,40);
	memcpy(&SendTransData.aucRespInfo[20],ReceiveTransData.aucCashierTrace,6);
	memcpy(SendTransData.aucCashTraceNo,ReceiveTransData.aucCashierTrace,6);
    
	   memcpy(&PORTAUCBUF2[PortData.ulHeadLength],&SendTransData,sizeof(SendTransData));
	   PortData.ulSendLength = PortData.ulHeadLength+ sizeof(SendTransData);
	   
	    ucResult = rs232_PackData(PortData.ulSendLength,PORTAUCBUF2,PORTAUCBUF1);
	   PortData.ulSendLength  =PortData.ulSendLength+ 5;
	   memset(PortData.aucBuf,0,sizeof(PortData.aucBuf));
	   memcpy(PortData.aucBuf,PORTAUCBUF1,PortData.ulSendLength);
	  
	   
	   return ucResult;
}


