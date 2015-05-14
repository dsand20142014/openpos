#include <include.h>
#include <global.h>
#include <xdata.h>


unsigned char Port_Main_Process(RECEIVETRANS *ReceiveData,unsigned char* aucOutData)
{
        unsigned char ucResult = SUCCESS,ucResult1 = SUCCESS;
        unsigned char ucFlag;
        SENDTRANS SendTransData;

        memset(&SendTransData,0,sizeof(SENDTRANS));
        util_Printf("\n*****ReceiveData->aucOperType[0]=%02x********\n",ReceiveData->aucOperType[0]);
        util_Printf("\n*****ReceiveData->aucOperType[1]=%02x********\n",ReceiveData->aucOperType[1]);
        if(!memcmp(ReceiveData->aucOperType,"A0",OPERTYPE_LEN))
        {
            DataSave0.ChangeParamData.ucResult = 0;
            DataSave0.ChangeParamData.ucErrorExtCode = 0;
            XDATA_Write_Vaild_File(DataSaveChange);
            ucResult = Port_Card_trans(&ucFlag,ReceiveData);
        }
        else if(!memcmp(ReceiveData->aucOperType,"A1",OPERTYPE_LEN))
        {
            ucResult = Port_Card_trans_SecondDeal(&ucFlag,ReceiveData);
        }
        else if(!memcmp(ReceiveData->aucOperType,"B0",OPERTYPE_LEN))
        {
            ucResult = Port_Query_trans(&ucFlag,ReceiveData);
        }
        else
        {
            ucResult = ERR_TRANS_UNKNOWOPERTYPE;
            ucFlag = 0x01;
        }

        Port_PackResult(ReceiveData,&SendTransData,ucResult);

        ucResult1 = Rs232yth_PackPortData(&SendTransData,aucOutData,ucFlag);
        if(ucResult1 != SUCCESS)
        {
            //Os__clr_display(255);
            //Os__GB2312_display(0,0,(uchar *)"请检查程序!");
            //Os__GB2312_display(1,0,(uchar *)"ucFlag定义错误");
            MSG_WaitKey(3);
        }
        if(DataSave0.ChangeParamData.ucResult == ERR_HOSTCODE)
        {
            DataSave0.ChangeParamData.ucErrorExtCode = G_RUNDATA_ucErrorExtCode;
            XDATA_Write_Vaild_File(DataSaveChange);
        }
        if((ucResult != SUCCESS)&&(ucResult != ERR_CANCEL)&&(!DataSave0.ChangeParamData.ucResult))
        {
            MSG_DisplayErrMsg(ucResult);
        }
        return(ucResult);
}

unsigned char Port_Card_trans(unsigned char *ucFlag,RECEIVETRANS *ReceiveData/*,SENDTRANS *SendTransData*/)
{
	  unsigned char ucResult = ERR_TRANS_UNKNOWTRANSTYPE;
	  util_Printf("ReceiveData->aucTransType[%02x]\n",ReceiveData->aucTransType[0]);
	  util_Printf("ReceiveData->aucTransType[%02x]\n",ReceiveData->aucTransType[1]);
	  if(!memcmp(ReceiveData->aucTransType,"30",TRANSTYPE_LEN))//消费
	  {
	  	  G_NORMALTRANS_ulAmount = asc_long(ReceiveData->aucSettleAmount,SETTLEAMOUNT_LEN);
	  	  memcpy(NormalTransData.aucCashTicket,ReceiveData->aucCashierTrace,CASHIERTRACE_LEN);
	  	  G_NORMALTRANS_ucTransType = TRANS_PURCHASE;
	  	  if(G_NORMALTRANS_ulAmount == 0)
	  	  	  ucResult = 0x31;
	  	  else
	  	  {
			if(!I_Sale)
				ucResult = ERR_TRANSNOTSUP;
			else
			{
				ucResult = Trans_Process(0);
			}
	  	  }
		if(ucResult != SUCCESS)
		{
		   DataSave0.ChangeParamData.ucResult = ucResult;
		   XDATA_Write_Vaild_File(DataSaveChange);
		}
	  	  *ucFlag = 0;
	  }
	  else if(!memcmp(ReceiveData->aucTransType,"32",TRANSTYPE_LEN))//预授权
	  {
	  	  *ucFlag = 0x01;
	  }
	   else if(!memcmp(ReceiveData->aucTransType,"34",TRANSTYPE_LEN))//预授权完成
	  {
	  	  *ucFlag = 0x01;
	  }
	  else if(!memcmp(ReceiveData->aucTransType,"40",TRANSTYPE_LEN))//撤销
	  {
		memcpy(NormalTransData.aucCashTicket,ReceiveData->aucCashierTrace,CASHIERTRACE_LEN);
		memcpy(G_RUNDATA_aucOriginalTraceNum,ReceiveData->aucOldTicketNumber,TRANS_TRACENUMLEN);
		G_NORMALTRANS_ucTransType = TRANS_VOIDPURCHASE;
		if(!I_UnSale)
			ucResult = ERR_TRANSNOTSUP;
		else
		{
			ucResult = Trans_Process(0);
		}
		 if(ucResult != SUCCESS)
		{
		   DataSave0.ChangeParamData.ucResult = ucResult;
		   XDATA_Write_Vaild_File(DataSaveChange);
		}
  	  *ucFlag = 0;
	  }
	   else if(!memcmp(ReceiveData->aucTransType,"42",TRANSTYPE_LEN))//预授权撤销
	  {
	  	  *ucFlag = 0x01;
	  }
	  else if(!memcmp(ReceiveData->aucTransType,"44",TRANSTYPE_LEN))//预授权完成撤销
	  {
	  	  *ucFlag = 0x01;
	  }
	   else if(!memcmp(ReceiveData->aucTransType,"48",TRANSTYPE_LEN))//追加预授权
	  {
	  	   *ucFlag = 0x01;
	  }
	  else if(!memcmp(ReceiveData->aucTransType,"50",TRANSTYPE_LEN))//退货
	  {
	  	  memcpy(NormalTransData.aucCashTicket,ReceiveData->aucCashierTrace,CASHIERTRACE_LEN);
	  	  G_NORMALTRANS_ucTransType = TRANS_REFUND;
	  	  G_NORMALTRANS_ulAmount = asc_long(ReceiveData->aucSettleAmount,SETTLEAMOUNT_LEN);
	  	  memcpy(G_NORMALTRANS_aucOldRefNumber,ReceiveData->aucReserved,TRANS_REFNUMLEN);
	  	  asc_bcd(&G_NORMALTRANS_aucOldDate[2], 2, &ReceiveData->aucReserved[TRANS_REFNUMLEN], 4);
	  	  if(G_NORMALTRANS_ulAmount == 0)
	  	  	  ucResult = 0x31;
	  	  else
	  	  {
	  	  	 if(!I_Refund)
				ucResult = ERR_TRANSNOTSUP;
			else
			{
				ucResult = Trans_Process(0);
			}
	  	  }
	  	   if(ucResult != SUCCESS)
			{
			   DataSave0.ChangeParamData.ucResult = ucResult;
			   XDATA_Write_Vaild_File(DataSaveChange);
			}
	  	  *ucFlag = 0;
	  }
	   else if(!memcmp(ReceiveData->aucTransType,"80",TRANSTYPE_LEN))//余额查询
	  {
	  	   *ucFlag = 0x01;
	  }
	  else if(!memcmp(ReceiveData->aucTransType,"90",TRANSTYPE_LEN))//初始化
	  {
	  	   *ucFlag = 0x01;
	  }
	   else if(!memcmp(ReceiveData->aucTransType,"91",TRANSTYPE_LEN))//签到
	  {
	  	   *ucFlag = 0x01;
	  }
	  else if(!memcmp(ReceiveData->aucTransType,"92",TRANSTYPE_LEN))//结算
	  {
	  	   *ucFlag = 0x01;
	  }
	   else if(!memcmp(ReceiveData->aucTransType,"94",TRANSTYPE_LEN))//签退
	  {
	  	   *ucFlag = 0x01;
	  }
	  else if(!memcmp(ReceiveData->aucTransType,"10",TRANSTYPE_LEN))//直接给收银机返回终端号、商户号、商户名称
	  {
	  	   *ucFlag = 0x01;
		   ucResult = 0x00;
	  }
	   else
	  	    *ucFlag = 0x01;

	  return ucResult;
}

unsigned char Port_PackResult(RECEIVETRANS *ReceiveData,SENDTRANS *SendTransData,unsigned char ucResult)
{
	unsigned char Temp[RESPINFO_LEN+1];
	unsigned char ucKey = 0;

	if(!memcmp(ReceiveData->aucTransType,"10",TRANSTYPE_LEN))
	{
		memcpy(SendTransData->aucMerchID,DataSave0.ConstantParamData.aucMerchantID,MERCHID_LEN);
		memset(SendTransData->aucMerchantName,' ',MERCHANTNAME_LEN);
		memcpy(SendTransData->aucMerchantName,DataSave0.ConstantParamData.aucMerchantName,MERCHANTNAME_LEN);
		memcpy(SendTransData->aucTermID,DataSave0.ConstantParamData.aucTerminalID,TERMID_LEN);
		hex_asc(SendTransData->aucRespCode,&ucResult,RESPCODE_LEN);
	}
	else
	{
		memcpy(SendTransData->aucOperType,ReceiveData->aucOperType,OPERTYPE_LEN);
		if(SendTransData->aucOperType[1]%2 == 0)
			ucKey = 1;
		if(ucKey == 1)
		  SendTransData->aucOperType[1]++;
		memcpy(SendTransData->aucTransType,ReceiveData->aucTransType,TRANSTYPE_LEN);
		if(ucKey == 1)
		  SendTransData->aucTransType[1]++;
		memcpy(SendTransData->aucCardType,ReceiveData->aucCardType,CARDTYPE_LEN);
		hex_asc(SendTransData->aucRespCode,&ucResult,RESPCODE_LEN);
		if(ucResult==ERR_HOSTCODE)
		{
			if(G_RUNDATA_ucErrorExtCode == 100)
				memcpy(SendTransData->aucRespCode,"A0",2);
			else
	  		short_asc(SendTransData->aucRespCode,RESPCODE_LEN,&G_RUNDATA_ucErrorExtCode);
		}
		memset(Temp,0,sizeof(Temp));
		MSG_GetErrMsg(ucResult, Temp);
		memcpy(SendTransData->aucRespInfo,Temp,RESPINFO_LEN);
		memcpy(SendTransData->aucCashRegNo,ReceiveData->aucCashRegNo,CASHREGNO_LEN);
		memcpy(SendTransData->aucCashier,ReceiveData->aucCashier,CASHIER_LEN);
		long_asc(SendTransData->aucSettleAmount,SETTLEAMOUNT_LEN,&G_NORMALTRANS_ulAmount);
		long_asc(SendTransData->aucBatchNum,BATCHNUM_LEN,&DataSave0.ChangeParamData.ulBatchNumber);
		memcpy(SendTransData->aucMerchID,DataSave0.ConstantParamData.aucMerchantID,MERCHID_LEN);
		memset(SendTransData->aucMerchantName,' ',MERCHANTNAME_LEN);
		memcpy(SendTransData->aucMerchantName,DataSave0.ConstantParamData.aucMerchantName,MERCHANTNAME_LEN);
		memcpy(SendTransData->aucTermID,DataSave0.ConstantParamData.aucTerminalID,TERMID_LEN);

		memset(SendTransData->aucCardId,' ',CARDID_LEN);
		if(G_NORMALTRANS_ucSourceAccLen%2)
		{
			bcd_asc(SendTransData->aucCardId,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
			SendTransData->aucCardId[G_NORMALTRANS_ucSourceAccLen]=0x00;
			//memcpy(&SendTransData->aucCardId[G_NORMALTRANS_ucSourceAccLen],(unsigned char*)" ",1);
		}else
		{
			bcd_asc(SendTransData->aucCardId,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
		}
		//memcpy(SendTransData->aucCardId,G_NORMALTRANS_ucDispSourceAcc,G_NORMALTRANS_ucDispSourceAccLen);

		bcd_asc(SendTransData->aucValidDate,G_NORMALTRANS_ucExpiredDate,VALIDDATE);
		memcpy(SendTransData->aucBankType,"000000",BANKTYPE_LEN);

		 switch(G_NORMALTRANS_ucCardType)
		 {
	 		   case TRANS_CARDTYPE_INTERNAL:
	 	   		   memcpy(&SendTransData->aucBankType[2],G_NORMALTRANS_aucIssuerID,4);
	 	   		   break;
	 		   case TRANS_CARDTYPE_VISA:
	 	   		   memcpy(SendTransData->aucBankType,"01",2);
	 	   		   break;
	 		   case TRANS_CARDTYPE_MASTER:
	 	   		   memcpy(SendTransData->aucBankType,"02",2);
	 	   		   break;
	 		   case TRANS_CARDTYPE_AE:
	 	   		   memcpy(SendTransData->aucBankType,"05",2);
	 	   		   break;
	 		   case TRANS_CARDTYPE_DINNER:
	 	   		   memcpy(SendTransData->aucBankType,"04",2);
	 	   		   break;
	 		   case TRANS_CARDTYPE_JCB:
	 	   		   memcpy(SendTransData->aucBankType,"03",2);
	 	   		   break;
	 		   default:
	 	   		   break;
		 }

		bcd_asc(SendTransData->aucTransDate,G_NORMALTRANS_aucDate,TRANSDATE_LEN);
		bcd_asc(SendTransData->aucTransTime,G_NORMALTRANS_aucTime,TRANSTIME_LEN);
		memcpy(SendTransData->aucAuthCode,G_NORMALTRANS_aucAuthCode,AUTHCODE_LEN);
		memcpy(SendTransData->aucSysRefNo,G_NORMALTRANS_aucRefNum,SYSREFNO_LEN);

		memcpy(SendTransData->aucCashTraceNo,NormalTransData.aucCashTicket,CASHIERTRACE_LEN);
		memset(SendTransData->aucOrgCashTraceNo,' ',OLDTICKETNUMBER_LEN);
		long_asc(SendTransData->aucTraceNo,TRACENO_LEN,&G_NORMALTRANS_ulTraceNumber);
		memset(SendTransData->aucOrgTraceNo,' ',ORGTRACENO_LEN);
		if(G_NORMALTRANS_ucTransType == TRANS_VOIDPURCHASE)
		{
			  long_asc(SendTransData->aucOrgTraceNo,OLDTICKETNUMBER_LEN,&G_NORMALTRANS_ulOldTraceNumber);
		}
		memset(SendTransData->aucReserved,' ',RESERVERD_LEN);
	}
	return SUCCESS;
}

unsigned char Port_Card_trans_SecondDeal(unsigned char *ucFlag,RECEIVETRANS *ReceiveData)
{
	  unsigned char ucResult = ERR_TRANS_UNKNOWTRANSTYPE;

	  if(!memcmp(ReceiveData->aucTransType,"31",TRANSTYPE_LEN))//消费
	  {
	  	  ucResult = SUCCESS;
	  	  if(DataSave0.ChangeParamData.ucResult == 0)
	  	  {
	  	      Port_PrintTicket();
	  	  }else
	  	  {
	  	  	 G_RUNDATA_ucErrorExtCode = DataSave0.ChangeParamData.ucErrorExtCode;
	  	  	  MSG_DisplayErrMsg(DataSave0.ChangeParamData.ucResult);
	  	  }
	  	  *ucFlag = 3;
	  }
	  else if(!memcmp(ReceiveData->aucTransType,"33",TRANSTYPE_LEN))//预授权
	  {
	  	  *ucFlag = 3;
	  }
	   else if(!memcmp(ReceiveData->aucTransType,"35",TRANSTYPE_LEN))//预授权完成
	  {
	  	  *ucFlag = 3;
	  }
	  else if(!memcmp(ReceiveData->aucTransType,"41",TRANSTYPE_LEN))//撤销
	  {
	  	  ucResult = SUCCESS;
	  	  if(DataSave0.ChangeParamData.ucResult == 0)
	  	  {
	  	      Port_PrintTicket();
	  	  }else
	  	  {
	  	  	  G_RUNDATA_ucErrorExtCode = DataSave0.ChangeParamData.ucErrorExtCode;
	  	  	  MSG_DisplayErrMsg(DataSave0.ChangeParamData.ucResult);
	  	  }
	  	  *ucFlag = 3;
	  }
	   else if(!memcmp(ReceiveData->aucTransType,"43",TRANSTYPE_LEN))//预授权撤销
	  {
	  	  *ucFlag = 3;
	  }
	  else if(!memcmp(ReceiveData->aucTransType,"45",TRANSTYPE_LEN))//预授权完成撤销
	  {
	  	  *ucFlag = 3;
	  }
	   else if(!memcmp(ReceiveData->aucTransType,"49",TRANSTYPE_LEN))//追加预授权
	  {
	  	   *ucFlag = 3;
	  }
	  else if(!memcmp(ReceiveData->aucTransType,"51",TRANSTYPE_LEN))//退货
	  {
	  	   ucResult = SUCCESS;
	  	  if(DataSave0.ChangeParamData.ucResult == 0)
	  	  {
	  	      Port_PrintTicket();
	  	  }else
	  	  {
	  	  	  G_RUNDATA_ucErrorExtCode = DataSave0.ChangeParamData.ucErrorExtCode;
	  	  	  MSG_DisplayErrMsg(DataSave0.ChangeParamData.ucResult);
	  	  }
	  	  *ucFlag = 3;
	  }
	   else if(!memcmp(ReceiveData->aucTransType,"81",TRANSTYPE_LEN))//余额查询
	  {
	  	   *ucFlag = 3;
	  }
	  else
	  	   *ucFlag = 3;
	UTIL_IncreaseTraceNumber();
	  return ucResult;
}

unsigned char Port_Query_trans(unsigned char *ucFlag,RECEIVETRANS *ReceiveData)
{
	unsigned char   ucResult = SUCCESS;
	unsigned short  uiIndex;
	unsigned char   aucBuf[CASHIER_LEN+1];
	unsigned long ulOldTraceNumber;
	int i;

	memset(aucBuf,0,sizeof(aucBuf));
	ulOldTraceNumber = asc_long(ReceiveData->aucCashier,CASHIER_LEN);

	util_Printf("查询流水号:");
	for(i=0;i<CASHIER_LEN;i++)
		util_Printf("%02x ",ReceiveData->aucCashier[i]);
	util_Printf("\n流水:%d\n",ulOldTraceNumber);
	*ucFlag = 1;
	for(uiIndex = 0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
		if( DataSave0.TransInfoData.auiTransIndex[uiIndex])
		{
			memset(&NormalTransData,0,xDATA_NormalTransSize);
			GetOldTransData(uiIndex);
			OSMEM_Memcpy((unsigned char *)&(DataSave1.SaveTrans),(unsigned char *)&NormalTransData,sizeof(NORMALTRANS));
			util_Printf("C流水:%d\n",DataSave1.SaveTrans.ulTraceNumber);
			if (ulOldTraceNumber == DataSave1.SaveTrans.ulTraceNumber)
			{
				util_Printf("OK流水:%d\n",DataSave1.SaveTrans.ulTraceNumber);
				return SUCCESS;
			}
		}
	}

	if(uiIndex == TRANS_MAXNB)
	{
	       return  ERR_NOTRANS;
	}
	return(ucResult);
}


unsigned char Port_PrintTicket(void)
{
    unsigned char ucI;
    unsigned char ucSaleCopy;

    Os__light_off();
    GetOldTransData(DataSave0.ChangeParamData.uiLastTransIndex);
    if( G_NORMALTRANS_ucTransType == TRANS_TIPADJUST )
    {
        //Os__clr_display(255);
        //Os__GB2312_display(1,0,(uchar *)"    调整成功");
    }
    else
    {
        if(G_NORMALTRANS_ucInputMode!=TRANS_INPUTMODE_INSERTCARD)
        {
            //Os__clr_display(255);
            //Os__GB2312_display(1,0,(uchar *)"    交易成功");
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
    else if(G_NORMALTRANS_ucInputMode!=TRANS_INPUTMODE_INSERTCARD)
    {
        //Os__GB2312_display(2,0,(uchar *)"    正在打印");
    }
    else if(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_INSERTCARD)
    {
        //Os__clr_display(255);
        //Os__GB2312_display(1,0,(uchar *)"    交易成功");
        //Os__GB2312_display(2,0,(uchar *)"    正在打印");
        //Os__GB2312_display(3,0,(uchar *)"     请拔卡");
    }
    UTIL_TestDispStr(G_NORMALTRANS_ucDispSourceAcc,(uint)G_NORMALTRANS_ucDispSourceAccLen,(uchar *)"---cc113---G_NORMALTRANS_ucDispSourceAcc=",NULL);
    for(ucSaleCopy=0,ucI=0;ucI<PRINTTIMES;ucI++,ucSaleCopy++)
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
        PRINT_Ticket(NORMAL_PRINT_FLAG,ucSaleCopy);
    }
    return SUCCESS;
}


