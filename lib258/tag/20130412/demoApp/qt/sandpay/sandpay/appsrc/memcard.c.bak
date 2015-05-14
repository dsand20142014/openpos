#include <include.h>
#include <global.h>
#include <xdata.h>
#include <osicc.h>
#include <smart.h>
#include <newsync.h>
#include "new_drv.h"


ICC_ORDER 	SLE4442_Order;
ICC_ANSWER *SLE4442_Answer;
union ORDER_IN_TYPE SLE4442_Type;
unsigned char SLE4442_Buffer[256];
static unsigned char buff_index;
unsigned char aucKey[4];


unsigned char SLE4442_Purchase(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],aucMac[20];
	unsigned long ulAmount;
	ucResult=SAV_SavedTransIndex(0);
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)":");
		if(NormalTransData.ucNewOrOld==1)
			Os__display(0,9,(uchar *)"N");
		else
			Os__display(0,9,(uchar *)"O");
		if(NormalTransData.ucPtCode==0x02)
			Os__display(0,10,(uchar *)"S");
		else if(NormalTransData.ucPtCode==0x01)
			Os__display(0,10,(uchar *)"W");
		else
			Os__display(0,10,(uchar *)"M");
			
		Os__display(1,0,aucBuf);
		//Os__GB2312_display(2,0,(uchar *)"ѽ:");
		ucResult=UTIL_InputAmount(3, &ulAmount,1, NormalTransData.ulPrevEDBalance);
	}
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,NormalTransData.aucZoneCode,3);
		if(NormalTransData.ucTransType==TRANS_S_PURCHASE)
			aucBuf[3]=0x21;
		NORMALTRANS_ulAmount=ulAmount;
		ucResult=CFG_CheckRate(aucBuf);
		if(ucResult!=SUCCESS)
			return ucResult;
		Uart_Printf("RunData.ucRateType=%02x\n",RunData.ucRateType);
		if(RunData.ucRateType=='1')
		{
			NormalTransData.ulRateAmount=RunData.ulRate/100;
			if(NormalTransData.ulRateAmount<RunData.ulMinRate)
				NormalTransData.ulRateAmount=RunData.ulMinRate;
			if(NormalTransData.ulRateAmount>RunData.ulMaxRate)
				NormalTransData.ulRateAmount=RunData.ulMaxRate;
		}
		else
		if(RunData.ucRateType=='0')
		{
			NormalTransData.ulRateAmount=(ulAmount*RunData.ulRate+5000)/10000;
			if(NormalTransData.ulRateAmount<RunData.ulMinRate)
				NormalTransData.ulRateAmount=RunData.ulMinRate;
			if(NormalTransData.ulRateAmount>RunData.ulMaxRate)
				NormalTransData.ulRateAmount=RunData.ulMaxRate;
		}
		Uart_Printf("RunData.ucRateType1=%02x\n",RunData.ucRateType1);
		Uart_Printf("NormalTransData.ulRateAmount=%d\n",NormalTransData.ulRateAmount);
		if(RunData.ucRateType1=='0')
			NORMALTRANS_ulAmount=ulAmount+NormalTransData.ulRateAmount;	
		else
			NORMALTRANS_ulAmount=ulAmount;
		if(NormalTransData.ulRateAmount!=0)
		{
			ucResult=UTIL_DisPayFeeInfo();
			if(ucResult!=SUCCESS)
				return ucResult;
		}
		if(NORMALTRANS_ulAmount>NormalTransData.ulPrevEDBalance)
			return ERR_LESSBALANCE;
	}
	if(ucResult==SUCCESS)
	{
		ucResult=UTIL_IncreaseTraceNumber(0);
	}
	if(ucResult==SUCCESS)
	{
		ucResult = SLE4442_CardTrans();
	}
	if(ucResult==SUCCESS)
		SAV_PurchaseSave();
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	return ucResult;

}


unsigned char SLE4442_Preload(void)
{
	unsigned char ucResult=SUCCESS,aucBuf[200],tCardNo[10];
	unsigned long ulMaxAmount;
	unsigned char ucStatus,ucLen,aucOutData[300],ucInput;
	ulMaxAmount=999999999;
	ucResult=SAV_SavedTransIndex(0);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		if(ucResult==SUCCESS)
			COMMS_PreComm();
	}
	if(ucResult==SUCCESS)
	{
		ucResult=Untouch_CkeckReload();
		NormalTransData.ulTraceNumber=DataSave0.ChangeParamData.ulTraceNumber;
	}
	if(ucResult==SUCCESS)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)":");
		Os__display(1,0,aucBuf);
		//Os__GB2312_display(2,0,(uchar *)"ֵ:");
		ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,0, ulMaxAmount);
	}
	if(ucResult==SUCCESS)
	{
		if(NormalTransData.ulPrevEDBalance+NormalTransData.ulAmount>NormalTransData.ulMaxAmount)
			return ERR_OVERAMOUNT;
		if(RunData.aucFunFlag[5])
			ucResult=UTIL_InputEncryptPIN();
	}
	if(ucResult==SUCCESS&&NormalTransData.ulAmount==0)
	{
		ucResult=Trans_PreloadQuery();
		if(ucResult==SUCCESS)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, NormalTransData.ulAmount, 2);
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"ɳ:");
			Os__display(1,0,aucBuf);
			//Os__GB2312_display(2,0,(uchar *)"[1]޸Ľ");
			//Os__GB2312_display(3,0,(uchar *)"[ȷ]");
			ucInput=0;
			while(ucInput!='1'&&ucInput!=KEY_ENTER
				&&ucInput!=KEY_CLEAR&&ucInput!=ERR_APP_TIMEOUT)
			{
				ucInput=MSG_WaitKey(60);
			}
			if(ucInput=='1')
			{
				//Os__GB2312_display(2,0,(uchar *)"ֵ:");
				ucResult=UTIL_InputAmount(3, &NormalTransData.ulAmount,1, NormalTransData.ulAmount);
				if(ucResult==SUCCESS)
				{
					if(NormalTransData.ulPrevEDBalance+NormalTransData.ulAmount>NormalTransData.ulMaxAmount)
						return ERR_OVERAMOUNT;
				}
			}else
			if(ucInput!=KEY_ENTER)
				ucResult=ERR_CANCEL;
		}
	}
	if(ucResult==SUCCESS)
		ucResult=Trans_Preload();
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}
	if(ucResult==SUCCESS)
	{
		ucResult = SLE4442_CardTrans();
	}
	if(ucResult==SUCCESS)
		SAV_LoadSave();
	if(ucResult==SUCCESS)
	{
		PRT_PrintTicket();
	}
	return ucResult;

}
unsigned char SLE4442_Choice(void)
{
	unsigned char ucResult=SUCCESS,flag1=1,flag2=1,flag3=1,flag4=1,flag5=1,flag6=1,flag7=1,flag8=1,flag9=1,flaga=1,flagb=1,flagc=1,flagd=1;
	int i;
	SELMENU ManagementMenu;
	/*if(SleTransData.ucStatus==0x04)
	{
		if(RunData.aucZDFunFlag[3]==1)
		{
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"δ");
			//Os__GB2312_display(1,0,(uchar *)"[ȷ]ۿ");
			if(Os__xget_key()!=KEY_ENTER)
				return ERR_CANCEL;
			return Untouch_Sale();
		}
		else
			return ERR_CARDNOACT;
	}*/
	memset(&ManagementMenu,0,sizeof(SELMENU));
	Uart_Printf("DataSave1.CardTable.ucCountType=%02x\n",DataSave1.CardTable.ucCountType);
	Uart_Printf("RunData.aucZDFunFlag\n");
	for(i=0;i<64;i++)
		Uart_Printf("%02x ",RunData.aucZDFunFlag[i]);
	Uart_Printf("\n");
	Uart_Printf("RunData.aucFunFlag\n");
	for(i=0;i<32;i++)
		Uart_Printf("%02x ",RunData.aucFunFlag[i]);
	Uart_Printf("\n");
	Uart_Printf("RunData.aucWDFunFlag\n");
	for(i=0;i<16;i++)
		Uart_Printf("%02x ",RunData.aucWDFunFlag[i]);
	Uart_Printf("\n");
	if(RunData.aucZDFunFlag[27]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1)
		||DataSave1.CardTable.ucCountType=='3')
		flaga=0;
	if(RunData.aucZDFunFlag[23]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1)
		||DataSave1.CardTable.ucCountType=='3')
		flag2=0;
	Uart_Printf("flag2=%02x\n",flag2);
	if(SleTransData.ucStatus==CARD_RECYCLED)		
	{
		if(flaga==0&&flag2==0)
			return ERR_RECYCLE;
		flag1=0;flag4=0;flag3=0;flag6=0;
		flag9=0;flag5=0;flag7=0;flag8=0;flagc=0;
	}
	else
	{
		if(NormalTransData.ucZeroFlag==1
			&&(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1))
		{
			if(flag2==0&&flaga==0)
			{
				return ERR_MUSTRETURN;
			}
			flag1=0;flag4=0;flag3=0;flag6=0;
			flag9=0;flag5=0;flag7=0;flag8=0;flagc=0;
		}
		else
		{
			if(RunData.aucZDFunFlag[0]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1)
				||DataSave1.CardTable.ucCountType=='3')
				flag1=0;
			if(RunData.aucZDFunFlag[20]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==0)
				||DataSave1.CardTable.ucCountType=='3')
				flag4=0;
			if(RunData.aucZDFunFlag[40]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==0))
				flag3=0;
			if(RunData.aucZDFunFlag[47]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==0)
				||DataSave1.CardTable.ucCountType=='3')
				flag5=0;/**/
			if(RunData.aucZDFunFlag[1]!=1&&RunData.aucZDFunFlag[21]!=1&&RunData.aucZDFunFlag[13]!=1
				||DataSave1.CardTable.ucCountType=='3')
				flag6=0;
			if(RunData.aucZDFunFlag[26]!=1||DataSave1.CardTable.ucCountType=='3')
				flag7=0;
			if(RunData.aucZDFunFlag[34]!=1||DataSave1.CardTable.ucCountType!='3')
				flag8=0;
			if(RunData.aucZDFunFlag[35]!=1||DataSave1.CardTable.ucCountType!='3')
				flag9=0;
			if(RunData.aucZDFunFlag[8]!=1||(DataSave1.CardTable.ucCountType=='1'&&RunData.aucFunFlag[2]==1)
				||DataSave1.CardTable.ucCountType=='3')
				flagc=0;
			if(RunData.aucZDFunFlag[41]!=1)
				flagd=0;
		}
	}
	if(SleTransData.ucStatus==0x04)	
	{
		if(NormalTransData.ucZeroFlag==1&&flag2)
		{
			flag1=0;flag4=0;flag3=0;flag6=0;flaga=0;
			flag9=0;flag5=0;flag7=0;flag8=0;flagc=0;

		}
		else
			return ERR_CARDNOACT;
	}
	UTIL_GetMenu_Value(TRANS_S_PURCHASE,MSG_PURCHASE,Untouch_Purchase,&flag1,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PURCHASE_P,MSG_PURCHASE,Untouch_Purchase_P,&flag8,&ManagementMenu);
	UTIL_GetMenu_Value(MSG_NULL,  MSG_PAYADMIN,Untouch_PayAdmin,&flag6,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_ONLINEPURCHASE,MSG_ONLINEPURCHASE,Untouch_OnlinePurchase,&flag4,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_VOID,MSG_VOID,Untouch_Void,&flag4,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_ONLINEPURCHASE_P,MSG_ONLINEPURCHASE_P,Untouch_OnlinePurchase,&flag9,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_VOID_P,MSG_VOID_P,Untouch_Void,&flag9,&ManagementMenu);
	UTIL_GetMenu_Value(TRANS_S_PRELOAD,  MSG_PRELOAD,	  SLE4442_Preload,&flag2,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_QUERY,  MSG_QUERY,	  Untouch_Query,&flag3,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_MOBILECHARGE,  MSG_MOBILEADMIN,	  Untouch_MobileAdmin,&flag7,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_PREAUTH,  MSG_PREAUTHPROCESS,	  Untouch_PreAuthProcess,&flag5,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_REFUND,  MSG_REFUND,Untouch_Refund,&flaga,&ManagementMenu);	
	UTIL_GetMenu_Value(TRANS_S_PTCPURCHASE,MSG_PTCPURCHASE,Untouch_PTCPurchase,&flagc,&ManagementMenu);
	UTIL_GetMenu_Value(MSG_NULL,  MSG_CHANEPIN,	  Untouch_ChangePin,&flagd,&ManagementMenu);	
	if(flag1==0&&flag2==0&&flag3==0&&flag4==0&&flag5==0&&flag6==0&&flag7==0&&flag8==0&&flag9==0&&flaga==0&&flagc==0&&flagd==0)
		return ERR_NOFUNCTION;
	if(flag1==1&&flag2==0&&flag3==0&&flag4==0&&flag5==0&&flag6==0&&flag7==0&&flag8==0&&flag9==0&&flaga==0&&flagc==0&&flagd==0)
	{
		NormalTransData.ucTransType=TRANS_S_PURCHASE;
		ucResult=Untouch_Purchase();
		return ucResult;
	}
	ucResult = UTIL_DisplayMenu(&ManagementMenu);
	if(Commun_flag==0x31||Commun_flag==0x33||Commun_flag==0x35)
	{
		COMMS_FinComm();
	}	
	return ucResult;
}

unsigned char SLE4442_Menu(union seve_in *EveIn)
{
	unsigned char ucResult=SUCCESS,aucBuf[20];
	unsigned char strBuf[17];
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"Ժ...");
	
	memset(strBuf,0,sizeof(strBuf));
	if(ucResult==SUCCESS)
	{
		ucResult=CFG_CheckFlag();
	}
	
	if(ucResult == SUCCESS )
	{
		SLE4442_ATR(strBuf);
	}

	//ǲǷǷ
	if(ucResult==SUCCESS)
	{
		//ucResult=SMC_CheckCard();
	}

	if(ucResult==SUCCESS)
	{
		ucResult = SLE4442_ReadCardData();
		NormalTransData.ucCardType=CARD_MEM;
	}
	if(ucResult==SUCCESS)
	{
		ucResult=CFG_CheckCardValid();
	}
	if(ucResult==SUCCESS)
	{
		CFG_SetBatchNumber();
	}	
	if(ucResult==SUCCESS)
		ucResult=Untouch_CheckRedo();

	if(ucResult==SUCCESS)
	{
		if(RunData.ucMustReturn)
		{
			if(DataSave1.CardTable.ucCountType!='1'||RunData.aucFunFlag[2]!=1)
				ucResult= ERR_MUSTRETURN;
		}
	}
	if(ucResult==SUCCESS)
	{
		if(DataSave0.ConstantParamData.ucYTJFlag)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			UTIL_Form_Montant(aucBuf, NormalTransData.ulPrevEDBalance, 2);
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)":");
			Os__display(1,0,aucBuf);
			//Os__GB2312_display(3,0,(uchar *)"");
			MSG_WaitKey(10);
			//Os__clr_display(255);
		}else
		{
			if(RunData.ucExpireFlag)
			{
				if(NormalTransData.ulYaAmount!=0)
				{
					if(SleTransData.ucStatus==0x04)		
						ucResult	= ERR_CARDNOACT;
					if(SleTransData.ucStatus==CARD_RECYCLED)		
						ucResult	= ERR_CARDRECYCLE;
					if(ucResult==SUCCESS)
					{
						//Os__clr_display(255);
						//Os__GB2312_display(0,0,(uchar*)"ѹ");
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,(uchar*)":",5);
						UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulPrevEDBalance,2);
						//Os__GB2312_display(1,0,aucBuf);
						memset(aucBuf,0,sizeof(aucBuf));
						memcpy(aucBuf,(uchar*)"Ѻ:",5);
						UTIL_Form_Montant(&aucBuf[5],NormalTransData.ulYaAmount,2);
						//Os__GB2312_display(2,0,aucBuf);
						//Os__GB2312_display(3,0,(uchar*)"[ȷ]һο۳");
						if(Os__xget_key()==KEY_ENTER)
						{
							NormalTransData.ucTransType=TRANS_S_EXPPURCHASE;
							ucResult=Untouch_ExpPurchase();
						}
                        //else
							//Os__clr_display(255);
					}
				}
			}
			else
			ucResult=SLE4442_Choice();
		}
	}
	if(ucResult!=SUCCESS)
	{
		MSG_DisplayErrMsg(ucResult);
	}
		
	return SUCCESS;	
}


unsigned char SLE4442_ATR(unsigned char *pBuf)
{	
	buff_index = 0;
	buff_index += SLE4442_ResetAddress(SLE4442_Buffer+buff_index);	//return 7
	buff_index += SLE4442_NReadBits(SLE4442_Buffer+buff_index, 31);	//return 5
	buff_index += SLE4442_End_ATR(SLE4442_Buffer+buff_index);		//return 1
	buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);	//return 1

	if( buff_index )		//buff_index=14
	{
		SLE4442_Type.new_sync_order.Len = buff_index;
		SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;
		SLE4442_Order.pt_order_in = &SLE4442_Type;
		SLE4442_Order.order = NEW_SYNC_ORDER;
		SLE4442_Order.ptout = SLE4442_Buffer;
		SLE4442_Answer = Os__ICC_command(0,&SLE4442_Order);
		if( SLE4442_Answer->drv_status != OK )
		{
		    return (SMART_CARD_ERROR); 
		}
		if( SLE4442_Answer->card_status != ASY_OK )
		{
			return (SMART_CARD_ERROR);
		}
		memcpy(pBuf,SLE4442_Buffer,SLE4442_Answer->Len);
	}
	return(SUCCESS);
}

unsigned char SLE4442_ResetAddress(unsigned char *code_buff)
{
	code_buff[0] = CLR_CLK_CODE;  	//0x08
	code_buff[1] = CLR_RST_CODE;	//0x02
	code_buff[2] = SET_IO_CODE;		//0x22
	code_buff[3] = SET_RST_CODE;	//0x04
	code_buff[4] = CLK_5US_CODE;	//0x0c
	code_buff[5] = CLR_RST_CODE;	//0x02
	code_buff[6] = IO_TO_LSB_CODE;	//0x2a
	return(7);
}

unsigned char SLE4442_NReadBits(unsigned char *code_buff, unsigned char N_Bits)
{
	code_buff[0] = MACRO_CODE;		//0xff
	code_buff[1] = CLK_5US_CODE;	//0x0c
	code_buff[2] = IO_TO_LSB_CODE;	//0x2a
	code_buff[3] = MACRO_CODE;		//0xff
	code_buff[4] = N_Bits;			//31
	return(5);
}

unsigned char SLE4442_End_ATR(unsigned char *code_buff)
{
	code_buff[0] = CLK_5US_CODE;	//0x0c
	return(1);
}

unsigned char SLE4442_FinishCommand(unsigned char *code_buff)
{
	code_buff[0] = EOC_CODE;	//0x00
   	return (1);
}

unsigned char SLE4442_ReadCardData(void)
{
	unsigned char ucResult,uctreat;
	unsigned char pCust_Card[300],aucBuf[20],aucBuf1[20];
	int i,ucI,ucJ,j;
	/*Read Card*/
	ucResult=SUCCESS;//he:add
	memset(pCust_Card,0,sizeof(pCust_Card));
	for(ucI=0,ucJ=32;ucI<256-32;ucI+=ucJ)
	{
		if( ucResult == SUCCESS )
		{
			ucResult=SLE4442_ReadMainMemory(pCust_Card+ucI,32+ucI,ucJ);
		}
	}
	Uart_Printf("SleTransData:\n");
	for(i=0;i<sizeof(SleTransData);i++)
	{
		Uart_Printf("%02x",pCust_Card[i]);
	}
	Uart_Printf("\n");
	//asc_bcd(pCust_Card,8,(uchar*)"8803100700000011",16);
	if(memcmp(&pCust_Card[217],"SANDPAY",7))
	{
		NormalTransData.ucPtCode=pCust_Card[17];
		NormalTransData.ucNewOrOld=0;
		if(pCust_Card[17]==0x99)
		{
			Uart_Printf("ow\n");
			memcpy(&SleTransData,pCust_Card,sizeof(SleTransData));
			memcpy(NormalTransData.aucVerifyCode,SleTransData.ucAuthCode,4);
			memcpy(NormalTransData.aucUserCardNo,SleTransData.ucSourceAcc,10);
			bcd_asc(NormalTransData.aucLoadTrace,SleTransData.ucDepositTrace,16);
			memcpy(NormalTransData.aucZoneCode,SleTransData.ucIssueCode,3);
			memcpy(NormalTransData.aucExpiredDate,SleTransData.ucExperiDate,4);
			memcpy(&NormalTransData.ucHopassFlag,&SleTransData.ucMainAccFlag,1);
			memcpy(&NormalTransData.ucMemFlag,&SleTransData.ucMemberFlag,1);
			memcpy(NormalTransData.aucSamTace,SleTransData.ucCount,4);
			NormalTransData.ulPrevEDBalance= tab_long(SleTransData.ucAmount,4);
			memcpy(RunData.aucCheckCrc,SleTransData.ucCheckCrc,4);
			if(SleTransData.ucBakup[0]=='1'&&NormalTransData.ulPrevEDBalance==0)
				RunData.ucNewLgFlag=1;
		}
		else if(pCust_Card[17]==0x00)
		{
			Uart_Printf("os\n");
			memcpy(&SleTransDataSmt,pCust_Card,sizeof(SleTransDataSmt));
			memcpy(NormalTransData.aucUserCardNo,SleTransDataSmt.ucSourceAcc,10);
			bcd_asc(&NormalTransData.aucLoadTrace[2],SleTransDataSmt.ucDepositTrace,14);
			memcpy(NormalTransData.aucExpiredDate,SleTransDataSmt.ucExperiDate,4);
			NormalTransData.ulPrevEDBalance= tab_long(SleTransDataSmt.ucAmount,4);
			memcpy(RunData.aucCheckCrc,SleTransDataSmt.ucCheckCrc,4);
			memcpy(NormalTransData.aucZoneCode,&SleTransDataSmt.ucAreaLimitCode[1],3);
			if(SleTransDataSmt.ucStatus==0x01)
				SleTransData.ucStatus=0x01;
			if(SleTransDataSmt.ucStatus==0x02)
				SleTransData.ucStatus=0x03;
			if(SleTransDataSmt.ucStatus==0x03)
				SleTransData.ucStatus=0x02;
		}
	}
	else
	{
			Uart_Printf("n\n");
		NormalTransData.ucNewOrOld=1;
		NormalTransData.ucPtCode=pCust_Card[16];
		memcpy(&SleTransDataSp,pCust_Card,sizeof(SleTransDataSp));
		memcpy(NormalTransData.aucVerifyCode,SleTransDataSp.ucAuthCode,4);
		memcpy(NormalTransData.aucUserCardNo,SleTransDataSp.ucSourceAcc,10);
		bcd_asc(NormalTransData.aucLoadTrace,SleTransDataSp.ucDepositTrace,16);
		memcpy(NormalTransData.aucZoneCode,SleTransDataSp.ucIssueCode,3);
		memcpy(NormalTransData.aucExpiredDate,SleTransDataSp.ucExperiDate,4);
		memcpy(&NormalTransData.ucHopassFlag,&SleTransDataSp.ucMainAccFlag,1);
		memcpy(&NormalTransData.ucMemFlag,&SleTransDataSp.ucMemberFlag,1);
		memcpy(NormalTransData.aucSamTace,SleTransDataSp.ucCount,4);
		NormalTransData.ulPrevEDBalance= tab_long(SleTransDataSp.ucAmount,4);
		memcpy(NormalTransData.aucDiscount,SleTransDataSp.ucDiscount,2);
		memcpy(RunData.aucCheckCrc,SleTransDataSp.ucCheckCrc,4);
		SleTransData.ucStatus=SleTransDataSp.ucStatus;
	}
	if(NormalTransData.ucPtCode==0x99)
		NormalTransData.ucPtCode=0x01;
	if(NormalTransData.ucPtCode==0x00)
		NormalTransData.ucPtCode=0x02;
	if(NormalTransData.ucPtCode==0x66)
		NormalTransData.ucPtCode=0x03;
	if(ucResult==SUCCESS)
	{
		ucResult=SMC_CheckCard();
		if(ucResult!=SUCCESS&&NormalTransData.ucPtCode!=0x03)
			return ucResult;
		else
			ucResult=SUCCESS;
	}

	if(ucResult == SUCCESS)
	{
		uctreat=DataSave0.ConstantParamData.aucTreat[NormalTransData.ucPtCode-1];
		if(uctreat=='0'
		||(NormalTransData.ucNewOrOld&&uctreat=='1')
		||(!NormalTransData.ucNewOrOld&&uctreat=='2'))
		{
			return ERR_CARDSUPPORT;
		}
	}
	if(ucResult == SUCCESS )
	{
		RunData.ucKeyArrey=0;
		if(NormalTransData.ucPtCode==0x03)
		{
			for(i=0;i<15;i++)
			{
				if(!memcmp(DataSave0.ChangeParamData.aucIssueKeyIndex[i],NormalTransData.aucZoneCode,2))
				{
					RunData.ucKeyArrey=i;
					break;
				}
			}
		}
		if(NormalTransData.ucPtCode==0x01)
		{
			bcd_asc(aucBuf1,NormalTransData.aucZoneCode,6);
			for(i=15;i<18;i++)
			{
				bcd_asc(aucBuf,DataSave0.ChangeParamData.aucIssueKeyIndex[i],6);
				if(!memcmp(aucBuf,aucBuf1,3))
				{
					RunData.ucKeyArrey=i;
					break;
				}
			}
		}
		memset(aucKey,0,sizeof(aucKey));
		ucResult = SLE4442_CheckCardValid();
		if(ucResult==SUCCESS)
			Uart_Printf("SLE_Check_Card_Valid112\n");
		else
			Uart_Printf("SLE_Check_Card_Valid555\n");
	}
	if(ucResult==SUCCESS)
		NormalTransData.ulTrueBalance=NormalTransData.ulPrevEDBalance;
	return(ucResult);
}

unsigned char SLE4442_ReadMainMemory(unsigned char *pBuf,unsigned char ucAddr,unsigned char ucNbr)
{
	buff_index = 0;
	if( ucNbr == 0)
		return(ERR_CANCEL);
	if( ucAddr+ucNbr > 256)
		return(ERR_CANCEL);
	
	buff_index += SLE4442_EnterCommandMode(SLE4442_Buffer+buff_index);//return 5
	buff_index += SLE4442_SendCommand(SLE4442_Buffer+buff_index,0x30,ucAddr,0);//rturn 48
	buff_index += SLE4442_LeaveCommandMode(SLE4442_Buffer+buff_index);//return 2
   	buff_index += SLE4442_NReadBits(SLE4442_Buffer+buff_index, ucNbr*8);//return 5
   	buff_index += SLE4442_Break(SLE4442_Buffer+buff_index);
	buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);//return 1
	
	if( buff_index )
	{
		SLE4442_Type.new_sync_order.Len = buff_index;
		SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;
		SLE4442_Order.pt_order_in = &SLE4442_Type;
		SLE4442_Order.order = NEW_SYNC_ORDER;
		SLE4442_Order.ptout = SLE4442_Buffer;
		SLE4442_Answer = Os__ICC_command(0,&SLE4442_Order);
		if( SLE4442_Answer->drv_status != OK )
		{
		    return (ERR_READMEMORY); 
		}
		if( SLE4442_Answer->card_status != ASY_OK )
		{
			return (ERR_READMEMORY); 
		}
		memcpy(pBuf,SLE4442_Buffer,SLE4442_Answer->Len);
	}
	return(SUCCESS);
}


unsigned char SLE4442_EnterCommandMode(unsigned char *code_buff)
{
	code_buff[0] = SET_IO_CODE;
	code_buff[1] = CLR_CLK_CODE;
	code_buff[2] = SET_CLK_CODE;
	code_buff[3] = CLR_IO_CODE;
	code_buff[4] = CLR_CLK_CODE;
	return(5);
}

unsigned char SLE4442_SendCommand(unsigned char *code_buff,unsigned char Command_Type,
							unsigned char Addr,unsigned char Input_Data)
{
	unsigned char Mask = 0x01;
	unsigned char i,j;

	j = 0;

	for(i=0;i<8;i++,Mask=Mask<<1)
	{
		if(Command_Type & Mask)
		{
			code_buff[j++] = SET_IO_CODE;
		}else
		{
			code_buff[j++] = CLR_IO_CODE;
		}
		code_buff[j++] = CLK_5US_CODE;
	}
	
	Mask = 0x01;
	for(i=0;i<8;i++,Mask=Mask<<1)
	{
		if(Addr & Mask)
		{
			code_buff[j++] = SET_IO_CODE;
		}else
		{
			code_buff[j++] = CLR_IO_CODE;
		}
		code_buff[j++] = CLK_5US_CODE;
	}
	Mask = 0x01;
	for(i=0;i<8;i++,Mask=Mask<<1)
	{
		if(Input_Data & Mask)
		{
			code_buff[j++] = SET_IO_CODE;
		}else
		{
			code_buff[j++] = CLR_IO_CODE;
		}
		code_buff[j++] = CLK_5US_CODE;
	}
	code_buff[j++] = CLR_IO_CODE;
	return(j);
}

unsigned char SLE4442_LeaveCommandMode(unsigned char *code_buff)
{
	code_buff[0] = SET_CLK_CODE;
	code_buff[1] = SET_IO_CODE;
	return(2);
}


unsigned char SLE4442_CheckCardValid(void)
{
	unsigned char aucHexBuf1[9],aucHexBuf2[9],ucResult,temp[20],money[10];
	unsigned char aucBuf[60],ucI,i;
	unsigned char uiI,ucKeyArray,ucCrcKeyIndex,ucKeyAIndex,ucPKeyIndex;
	if(NormalTransData.ucNewOrOld==0)
	{
		switch(NormalTransData.ucPtCode)
		{
			case 0x01:ucKeyArray=KEYARRAY_PURSECARD2;
				ucCrcKeyIndex=KEYINDEX_WKCARD1_CRCKEY1;
				ucKeyAIndex=KEYINDEX_WKCARD1_MESSAGEKEYA;
				ucPKeyIndex=KEYINDEX_WKCARD1_PURCHASEKEY;
				break;
			case 0x02:ucKeyArray=KEYARRAY_PURSECARD3;
				ucKeyAIndex=KEYINDEX_PURSECARD3_CARDCHECK;
				ucPKeyIndex=KEYINDEX_PURSECARD3_AMOUNTCHECK;
				break;
			default:break;
		}
	}
	else
	{
		switch(NormalTransData.ucPtCode)
		{
			case 0x01:
				if(RunData.ucKeyArrey)
					ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;
				else
					ucKeyArray=KEYARRAY_PURSECARD1;
				break;
			case 0x02:ucKeyArray=KEYARRAY_PURSECARD4;break;
			case 0x03:ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;break;
			default:break;
		}
		
		ucCrcKeyIndex=KEYINDEX_PURSECARD1_CRCKEY;
		ucKeyAIndex=KEYINDEX_PURSECARD1_MESSAGEKEYA;
		ucPKeyIndex=KEYINDEX_PURSECARD1_PURCHASEKEY;
	}
	Uart_Printf("uckeyarray=%02x,uccrckeyindex=%02x\n",ucKeyArray,ucCrcKeyIndex);
	memset(aucBuf,0,sizeof(aucBuf));
	ucResult=SUCCESS;
	bcd_asc(aucBuf , NormalTransData.aucUserCardNo , 16);
	
	Uart_Printf("SLE_Check_Card_Valid!\n");	
	Uart_Printf("G_NORMALTRANS_aucSourceAcc=%s\n",aucBuf);
	memcpy(aucHexBuf1,NormalTransData.aucUserCardNo,8);
	memcpy(aucHexBuf2,NormalTransData.aucUserCardNo,8);
/*****************ܼ֤*****************/	
	PINPAD_47_Crypt8ByteSingleKey(ucKeyArray,ucCrcKeyIndex,
								  aucHexBuf1,aucHexBuf1);
	PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray,ucCrcKeyIndex+1,
								  aucHexBuf1,aucHexBuf1);
	PINPAD_47_Crypt8ByteSingleKey(ucKeyArray,ucCrcKeyIndex,
								  aucHexBuf1,aucHexBuf1);
	gz_xor(aucHexBuf2,aucHexBuf1,8);
/*****************************************/	
	if(NormalTransData.ucPtCode!=0x02||NormalTransData.ucNewOrOld!=0x00)
	{
		if(memcmp(NormalTransData.aucVerifyCode,aucHexBuf1,4))
			return ERR_CARDCRC;
	}
	
	long_tab(aucHexBuf1,4,&NormalTransData.ulPrevEDBalance);
	memcpy(&aucHexBuf1[4],&NormalTransData.aucUserCardNo[4],4);
	memcpy(aucHexBuf2,aucHexBuf1,8);
/*Uart_Printf("predes amount:\n");
	for(ucI=0;ucI<8;ucI++)
	{
		Uart_Printf("%02x",aucHexBuf1[ucI]);
	}
	Uart_Printf("\n");
*/
	/*	ܼ㿨Уλ*/	
	/*DES*/
	PINPAD_47_Crypt8ByteSingleKey(ucKeyArray,ucPKeyIndex,
								  aucHexBuf1,aucHexBuf1);
	/*DES*/
	PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray,ucPKeyIndex+1,
								  aucHexBuf1,aucHexBuf1);
	PINPAD_47_Crypt8ByteSingleKey(ucKeyArray,ucPKeyIndex,
								  aucHexBuf1,aucHexBuf1);
	/**/
	gz_xor(aucHexBuf2,aucHexBuf1,8);

#if 1
	if(memcmp(aucHexBuf1,RunData.aucCheckCrc,4))
	{
              Uart_Printf("Amount CRC IS not OK\n");
		return(ERR_CARDAMOUNTCRC);/*Уλ*/
	}
#endif
//Uart_Printf("Amount CRC IS OK\n");
	memcpy(aucHexBuf1,NormalTransData.aucUserCardNo,8);
	memcpy(aucHexBuf2,NormalTransData.aucUserCardNo,8);
	
/*Uart_Printf("G_MEMDATA_ucSourceAcc:\n");
	for(ucI=0;ucI<8;ucI++)
	{
		Uart_Printf("%02x",aucHexBuf1[ucI]);
	}
	Uart_Printf("\n");*/

	/*ܼ㿨֤*/	
	PINPAD_47_Crypt8ByteSingleKey(ucKeyArray,ucKeyAIndex,
								  aucHexBuf1,aucHexBuf1);
	PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray,ucKeyAIndex+1,
								  aucHexBuf1,aucHexBuf1);
	PINPAD_47_Crypt8ByteSingleKey(ucKeyArray,ucKeyAIndex,
								  aucHexBuf1,aucHexBuf1);
	gz_xor(aucHexBuf2,aucHexBuf1,8);
	memcpy(aucKey,aucHexBuf1,3);
	
	Uart_Printf("SLE4442_Check_Secret11\n");
	ucResult=SLE4442_CheckSecret(aucKey);
	
	return(ucResult);
}



unsigned char SLE4442_CheckSecret(unsigned char *pSecret_Code)
{
	unsigned char strBuf[100];

	if( SLE4442_ReadSecurityMemory(strBuf) != OK)
	{
		return(ERR_READMEMORY);
	}
	switch(strBuf[0])
	{
		case 0x07:
			if( SLE4442_UpdateSecurityMemory(0x06,0) != OK)
			{
				return(ERR_WRITEMEMORY);
			}
			break;
		case 0x06:
			if( SLE4442_UpdateSecurityMemory(0x04,0) != OK)
			{
				return(ERR_WRITEMEMORY);
			}
			break;
		case 0x04:
			if( SLE4442_UpdateSecurityMemory(0x00,0) != OK)
			{
				return(ERR_WRITEMEMORY);
			}
			break;
		default:
			return(ERR_CARDLOCK);
	}
	if( SLE4442_CompareVerificationData(*(pSecret_Code),0x01) != OK)
	{
		return(ERR_CHECKSECRET);
	}
	if( SLE4442_CompareVerificationData(*(pSecret_Code+1),0x02) != OK)
	{
		return(ERR_CHECKSECRET);
	}
	if( SLE4442_CompareVerificationData(*(pSecret_Code+2),0x03) != OK)
	{
		return(ERR_CHECKSECRET);
	}
	if( SLE4442_UpdateSecurityMemory(0xFF,0) != OK)
	{
		return(ERR_WRITEMEMORY);
	}
	if( SLE4442_ReadSecurityMemory(strBuf) != OK)
	{
		return(ERR_READMEMORY);
	}
	if( strBuf[0] != 0x07)
	{
		return(ERR_CHECKSECRET);
	}
	return(OK);
}

unsigned char SLE4442_ReadSecurityMemory(unsigned char *pBuf)
{
	
	buff_index = 0;
	buff_index += SLE4442_EnterCommandMode(SLE4442_Buffer+buff_index);
	buff_index += SLE4442_SendCommand(SLE4442_Buffer+buff_index,0x31,0,0);
	buff_index += SLE4442_LeaveCommandMode(SLE4442_Buffer+buff_index);
	buff_index += SLE4442_NReadBits(SLE4442_Buffer+buff_index, 32);
	buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);

	if( buff_index )
	{
		SLE4442_Type.new_sync_order.Len = buff_index;
		SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;
		SLE4442_Order.pt_order_in = &SLE4442_Type;
		SLE4442_Order.order = NEW_SYNC_ORDER;
		SLE4442_Order.ptout = SLE4442_Buffer;
		SLE4442_Answer = Os__ICC_command(0,&SLE4442_Order);
		if( SLE4442_Answer->drv_status != OK )
		{
			return (SMART_CARD_ERROR);
		}
		if( SLE4442_Answer->card_status != ASY_OK )
		{
			return (SMART_CARD_ERROR);
		}
		memcpy(pBuf,SLE4442_Buffer,SLE4442_Answer->Len);
	}
	return(OK);
}

unsigned char SLE4442_UpdateSecurityMemory(unsigned char Input_Data,unsigned char ucAddr)
{

	buff_index = 0;
	buff_index += SLE4442_EnterCommandMode(SLE4442_Buffer+buff_index);
	buff_index += SLE4442_SendCommand(SLE4442_Buffer+buff_index,0x39,ucAddr,Input_Data);
	buff_index += SLE4442_LeaveCommandMode(SLE4442_Buffer+buff_index);
	buff_index += SLE4442_EraseAndWrite(SLE4442_Buffer+buff_index);
	buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);

	if( buff_index )
	{
		SLE4442_Type.new_sync_order.Len = buff_index;
		SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;
		SLE4442_Order.pt_order_in = &SLE4442_Type;
		SLE4442_Order.order = NEW_SYNC_ORDER;
		SLE4442_Order.ptout = SLE4442_Buffer;
		SLE4442_Answer = Os__ICC_command(0,&SLE4442_Order);
		if( SLE4442_Answer->drv_status != OK )
		{
			return (ERR_MODIFYSECRET);
		}
		if( SLE4442_Answer->card_status != ASY_OK )
		{
			return (ERR_MODIFYSECRET);
		}

	}
	return(OK);
}


unsigned char SLE4442_CompareVerificationData(unsigned char Input_Data,unsigned char ucAddr)
{

	buff_index = 0;
	buff_index += SLE4442_EnterCommandMode(SLE4442_Buffer+buff_index);
	buff_index += SLE4442_SendCommand(SLE4442_Buffer+buff_index,0x33,ucAddr,Input_Data);
	buff_index += SLE4442_LeaveCommandMode(SLE4442_Buffer+buff_index);
	buff_index += SLE4442_CompareProcess(SLE4442_Buffer+buff_index);
	buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);

	if( buff_index )
	{
		SLE4442_Type.new_sync_order.Len = buff_index;
		SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;
		SLE4442_Order.pt_order_in = &SLE4442_Type;
		SLE4442_Order.order = NEW_SYNC_ORDER;
		SLE4442_Order.ptout = SLE4442_Buffer;
		SLE4442_Answer = Os__ICC_command(0,&SLE4442_Order);
		if( SLE4442_Answer->drv_status != OK )
		{
			return (SMART_CARD_ERROR);
		}
		if( SLE4442_Answer->card_status != ASY_OK )
		{
			return (SMART_CARD_ERROR);
		}

	}
	return(OK);
}


char SLE4442_GenCardVerify(char* sCardNo)
{
   int i,j,res,ret;
   int evensum,oddsum,total;
   
   j=1;
   total=0;
   for (i=strlen(sCardNo);i>=1;i--)
   {
    if ((j%2)==1) {
     evensum=0;
     ret=(sCardNo[i-1]-0x30)*2;
     if (ret>=10)
      oddsum=(ret/10) + (ret%10);
     else
      oddsum=ret;
    }
    else {
     evensum=sCardNo[i-1]-0x30;
     oddsum=0;
    }
    total+=evensum+oddsum;
    j++;
   }
   res=(10-(total % 10)) %10;
   return res+0x30;
}

unsigned char SLE4442_CardTrans(void)
{
	unsigned char ucResult=SUCCESS,ucI,ucOffset,ucLen,ucKeyArray,ucPKeyIndex;
	unsigned char aucHexBuf1[9],aucHexBuf2[9],aucBuf[25];
	unsigned long ulAmount,ulPreBalance,ulCount;
	unsigned char aucOutData[10];
	int i,iWt;
	if(NormalTransData.ucTransType==TRANS_S_PRELOAD
		||NormalTransData.ucTransType==TRANS_S_REFUND
		||NormalTransData.ucTransType==TRANS_S_CHANGECD
		||NormalTransData.ucTransType==TRANS_S_LOADONLINE)
	{
		ucResult=CFG_CalcPreLoadTAC();	
		if(ucResult!=SUCCESS)
			return ucResult;
	}
	else
	{
		ucResult=CFG_CalcPurhaseTAC();	
		if(ucResult!=SUCCESS)
			return ucResult;
	}
	ulPreBalance=NormalTransData.ulTrueBalance;
	memset(aucBuf,0,sizeof(aucBuf));
	memset(aucHexBuf1,0,sizeof(aucHexBuf1));
	memset(aucHexBuf2,0,sizeof(aucHexBuf2));
	
	if(NormalTransData.ucTransType==TRANS_S_PRELOAD
		||NormalTransData.ucTransType==TRANS_S_REFUND
		||NormalTransData.ucTransType==TRANS_S_CHANGECD
		||NormalTransData.ucTransType==TRANS_S_LOADONLINE)
		ulAmount=ulPreBalance+NORMALTRANS_ulAmount;
	else
	{
		if(ulPreBalance>NORMALTRANS_ulAmount)
			ulAmount=ulPreBalance-NORMALTRANS_ulAmount;
		else
			ulAmount=0;
	}
	
	long_tab(aucHexBuf1,4,&ulAmount);
	memcpy(&aucHexBuf1[4],&NormalTransData.aucUserCardNo[4],4);
	memcpy(aucHexBuf2,aucHexBuf1,8);
	memcpy(aucBuf,aucHexBuf1,8);
	if(NormalTransData.ucNewOrOld==0)
	{
		switch(NormalTransData.ucPtCode)
		{
			case 0x01:ucKeyArray=KEYARRAY_PURSECARD2;
				ucPKeyIndex=KEYINDEX_WKCARD1_PURCHASEKEY;
				break;
			case 0x02:ucKeyArray=KEYARRAY_PURSECARD3;
				ucPKeyIndex=KEYINDEX_PURSECARD3_AMOUNTCHECK;
				break;
			default:break;
		}
	}
	else
	{
		switch(NormalTransData.ucPtCode)
		{
			case 0x01:
				if(RunData.ucKeyArrey)
					ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;
				else
					ucKeyArray=KEYARRAY_PURSECARD1;
				break;
			case 0x02:ucKeyArray=KEYARRAY_PURSECARD4;break;
			case 0x03:ucKeyArray=KEYARRAY_PURSECARD5+RunData.ucKeyArrey;break;
			default:break;
		}
		
		ucPKeyIndex=KEYINDEX_PURSECARD1_PURCHASEKEY;
	}
	
	/*DES*/
	PINPAD_47_Crypt8ByteSingleKey(ucKeyArray,ucPKeyIndex,
								  aucHexBuf1,aucHexBuf1);
	/*DES*/
	PINPAD_47_Encrypt8ByteSingleKey(ucKeyArray,ucPKeyIndex+1,
								  aucHexBuf1,aucHexBuf1);
	PINPAD_47_Crypt8ByteSingleKey(ucKeyArray,ucPKeyIndex,
								  aucHexBuf1,aucHexBuf1);
	/**/
	gz_xor(aucHexBuf2,aucHexBuf1,8);


	if(NormalTransData.ucTransType==TRANS_S_PRELOAD
		||NormalTransData.ucTransType==TRANS_S_REFUND
		||NormalTransData.ucTransType==TRANS_S_CASHLOAD
		||NormalTransData.ucTransType==TRANS_S_CHANGECD
		||NormalTransData.ucTransType==TRANS_S_LOADONLINE)
	{
		Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),
						0,sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveReversalISO8583);
	}
	
	memcpy(&aucBuf[4],aucHexBuf1,4);	
	if(NormalTransData.ucPtCode==0x02&&NormalTransData.ucNewOrOld==0x00)
		ucOffset=64;
	else
		ucOffset=89;
	memcpy(&DataSave0.RedoTrans.Tran,&NormalTransData,sizeof(NORMALTRANS));
	DataSave0.RedoTrans.ucRedoFlag=1;
	XDATA_Write_Vaild_File(DataSaveRedoTrans);
	for(iWt=0;iWt<3;iWt++)
	{
		for(ucI=0;ucI<4;ucI++)
		{
			if(ucResult == SUCCESS)
			{
				ucResult=SLE4442_UpdateMainMemory(aucBuf[ucI],ucOffset+ucI);
			}
		}
		
		if(ucResult!=SUCCESS)
		{
			return ucResult;
		}
		
		memset(aucOutData,0,sizeof(aucOutData));
		ucResult= SLE4442_ReadMainMemory(aucOutData,ucOffset,4);
		
		if(!memcmp(aucOutData,aucBuf,4))
			break;
	}
	if(iWt==3)
		return ERR_WRITEMEMORY;
	
	for(iWt=0;iWt<3;iWt++)
	{
		for(ucI=0;ucI<4;ucI++)
		{
			if(ucResult == SUCCESS)
				ucResult= SLE4442_UpdateMainMemory(aucBuf[ucI+4],ucOffset+4+ucI);
		}
		if(ucResult!=SUCCESS)
		{
			return ucResult;
		}
		memset(aucOutData,0,sizeof(aucOutData));
		ucResult=SLE4442_ReadMainMemory(aucOutData,ucOffset+4,4);
		
		if(!memcmp(aucOutData,&aucBuf[4],4))
			break;
	}
	if(iWt==3)
		return ERR_WRITEMEMORY;

	if(NormalTransData.ucReturnflag)
	{
		if(NormalTransData.ucPtCode==0x02&&NormalTransData.ucNewOrOld==0x00)
		{
			ucOffset=63;
			aucBuf[0]=0x02;
		}
		else
		{
			ucOffset=78;
			aucBuf[0]=CARD_RECYCLED;
		}
		ucResult=SLE4442_UpdateMainMemory(aucBuf[0],ucOffset);
		if(ucResult!=SUCCESS)
		{
			return ucResult;
		}
		ucResult=SLE4442_ReadMainMemory(aucOutData,ucOffset,1);
		if(aucOutData[0]!=aucBuf[0])
			return ERR_WRITEMEMORY;
	}
	if(NormalTransData.ucTransType==TRANS_S_REFUND
		||NormalTransData.ucTransType==TRANS_S_CHANGECD)
	{
		if(NormalTransData.ucPtCode==0x02&&NormalTransData.ucNewOrOld==0x00)
		{
			ucOffset=63;
			aucBuf[0]=1;
		}
		else
		{
			ucOffset=78;
			aucBuf[0]=1;
		}
		ucResult=SLE4442_UpdateMainMemory(aucBuf[0],ucOffset);
		if(ucResult!=SUCCESS)
		{
			return ucResult;
		}
		ucResult=SLE4442_ReadMainMemory(aucOutData,ucOffset,1);
		if(aucOutData[0]!=aucBuf[0])
			return ERR_WRITEMEMORY;
	}
	if((NormalTransData.ucTransType==TRANS_S_LOADONLINE
		||NormalTransData.ucTransType==TRANS_S_PRELOAD)
		&&SleTransData.ucStatus==CARD_RECYCLED)
	{
		if(NormalTransData.ucPtCode==0x02&&NormalTransData.ucNewOrOld==0x00)
		{
			ucOffset=63;
			if(RunData.aucFunFlag[9]==1)
				aucBuf[0]=1;
			else
				aucBuf[0]=4;
		}
		else
		{
			ucOffset=78;
			if(RunData.aucFunFlag[9]==1)
				aucBuf[0]=1;
			else
				aucBuf[0]=4;
		}
		ucResult=SLE4442_UpdateMainMemory(aucBuf[0],ucOffset);
		if(ucResult!=SUCCESS)
		{
			return ucResult;
		}
		ucResult=SLE4442_ReadMainMemory(aucOutData,ucOffset,1);
		if(aucOutData[0]!=aucBuf[0])
			return ERR_WRITEMEMORY;
	}
	if(NormalTransData.ucTransType==TRANS_S_PRELOAD
		||NormalTransData.ucTransType==TRANS_S_REFUND
		||NormalTransData.ucTransType==TRANS_S_CHANGECD
		||NormalTransData.ucTransType==TRANS_S_LOADONLINE)
	{

		if(NormalTransData.ucPtCode==0x02&&NormalTransData.ucNewOrOld==0x00)
			ucOffset=59;
		else
			ucOffset=72;
		memcpy(aucBuf,NormalTransData.aucExpiredDate,4);
		for(ucI=0;ucI<4;ucI++)
		{
			ucResult=SLE4442_UpdateMainMemory(aucBuf[ucI],ucOffset+ucI);
			if(ucResult!=SUCCESS)
			{
#ifdef TEST
			Uart_Printf("Card Trans err3=%02x\n",ucResult);
#endif
				return ucResult;
			}
		}
		memset(aucOutData,0,sizeof(aucOutData));
		ucResult=SLE4442_ReadMainMemory(aucOutData,ucOffset,4);
		if(memcmp(aucOutData,&aucBuf[0],4))
			return ERR_WRITEMEMORY;

		asc_bcd(aucBuf,8,NormalTransData.aucLoadTrace,16);
		if(NormalTransData.ucPtCode==0x02&&NormalTransData.ucNewOrOld==0x00)
		{
			ucOffset=40;
			ucLen=7;
			for(ucI=0;ucI<ucLen;ucI++)
			{
				ucResult=SLE4442_UpdateMainMemory(aucBuf[ucI+1],ucOffset+ucI);
				if(ucResult!=SUCCESS)
				{
					return ucResult;
				}
			}
			memset(aucOutData,0,sizeof(aucOutData));
			ucResult=SLE4442_ReadMainMemory(aucOutData,ucOffset,ucLen);
			if(memcmp(aucOutData,&aucBuf[1],ucLen))
				return ERR_WRITEMEMORY;
		}
		else
		{
			ucOffset=56;
			ucLen=8;
			for(ucI=0;ucI<ucLen;ucI++)
			{
				ucResult=SLE4442_UpdateMainMemory(aucBuf[ucI],ucOffset+ucI);
				if(ucResult!=SUCCESS)
				{
					return ucResult;
				}
			}
			memset(aucOutData,0,sizeof(aucOutData));
			ucResult=SLE4442_ReadMainMemory(aucOutData,ucOffset,ucLen);
			if(memcmp(aucOutData,&aucBuf[0],ucLen))
				return ERR_WRITEMEMORY;
		}
	}	

	if(NormalTransData.ucPtCode!=0x02||NormalTransData.ucNewOrOld!=0x00)
	{
		ulCount=tab_long(NormalTransData.aucSamTace,4);
		ulCount++;
		long_tab(aucBuf,4,&ulCount);
		for(ucI=0;ucI<4;ucI++)
		{
			ucResult=SLE4442_UpdateMainMemory(aucBuf[ucI],79+ucI);
			if(ucResult!=SUCCESS)
			{
				return ucResult;
			}
		}
	}
	if(NormalTransData.ucNewOrOld==1)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,&NormalTransData.aucDateTime[1],6);
		aucBuf[6]=NormalTransData.ucTransType;
		long_tab(&aucBuf[7],4,&NormalTransData.ulAmount);
		long_tab(&aucBuf[11],4,&NormalTransData.ulTrueBalance);
		asc_bcd(&aucBuf[15],4,DataSave0.ConstantParamData.aucTerminalID,8);
		long_bcd(&aucBuf[19],3,&NormalTransData.ulTraceNumber);
		if(NormalTransData.ucTransType==TRANS_S_PRELOAD
			||NormalTransData.ucTransType==TRANS_S_REFUND
			||NormalTransData.ucTransType==TRANS_S_CHANGECD
			||NormalTransData.ucTransType==TRANS_S_LOADONLINE)
		{
			for(ucI=0;ucI<22;ucI++)
			{
				SLE4442_UpdateMainMemory(aucBuf[ucI],134+ucI);
			}
		}
		else
		{
			for(ucI=0;ucI<22;ucI++)
			{
				SLE4442_UpdateMainMemory(aucBuf[ucI],112+ucI);
			}
		}
	}
	DataSave0.RedoTrans.ucRedoFlag=2;
	XDATA_Write_Vaild_File(DataSaveRedoTrans);
	return SUCCESS;
}

unsigned char SLE4442_EraseAndWrite(unsigned char *code_buff)
{
	code_buff[0] = MACRO_CODE;
	code_buff[1] = CLK_10US_CODE;
	code_buff[2] = MACRO_CODE;
	code_buff[3] = 254;
	return(4);
}


unsigned char SLE4442_CompareProcess(unsigned char *code_buff)
{
	code_buff[0] = SET_CLK_CODE;
	code_buff[1] = SET_IO_CODE;
	code_buff[2] = CLR_CLK_CODE;
	code_buff[3] = CLR_IO_CODE;
	code_buff[4] = MACRO_CODE;
	code_buff[5] = CLK_10US_CODE;
	code_buff[6] = MACRO_CODE;
	code_buff[7] = 3;
	return(8);
}

void gz_xor(uchar *a, uchar *b, char lg)
{
	while (lg--)
		*(b++) ^= *(a++);
}

unsigned char SLE4442_Break(unsigned char *code_buff)
{
	code_buff[0] = CLR_CLK_CODE;
	code_buff[1] = SET_RST_CODE;
	code_buff[2] = CLR_RST_CODE;
	return(3);
}	

unsigned char SLE4442_UpdateMainMemory(unsigned char Input_Data,unsigned char ucAddr)
{

	buff_index = 0;
	buff_index += SLE4442_EnterCommandMode(SLE4442_Buffer+buff_index);//return 5
	buff_index += SLE4442_SendCommand(SLE4442_Buffer+buff_index,0x38,ucAddr,Input_Data);//return 49
	buff_index += SLE4442_LeaveCommandMode(SLE4442_Buffer+buff_index);//return 2
	buff_index += SLE4442_EraseAndWrite(SLE4442_Buffer+buff_index);//return 4
	buff_index += SLE4442_FinishCommand(SLE4442_Buffer+buff_index);//return 1

	if( buff_index )
	{
		SLE4442_Type.new_sync_order.Len = buff_index;
		SLE4442_Type.new_sync_order.ptin = SLE4442_Buffer;
		SLE4442_Order.pt_order_in = &SLE4442_Type;
		SLE4442_Order.order = NEW_SYNC_ORDER;
		SLE4442_Order.ptout = SLE4442_Buffer;
		SLE4442_Answer = Os__ICC_command(0,&SLE4442_Order);

		if( SLE4442_Answer->drv_status != OK )
		{
			return (ERR_WRITEMEMORY);
		}
		if( SLE4442_Answer->card_status != ASY_OK )
		{
			return (ERR_WRITEMEMORY);
		}
	}
	return(OK);
}
/*
unsigned char SLE4442_WaitInsert(void)
{
	unsigned char 	ucResult,aucBuf[17];
	unsigned int	uiTimesOut;
	unsigned char 	length1;
	DRV_OUT  		*ptKeyboard;
	DRV_OUT 		*pxIcc;
	union 	seve_in eve_in;
	
	
	ucResult=SUCCESS;
	while(1)
	{
		ptKeyboard=Os__get_key();
		pxIcc = Os__ICC_insert();
		
		uiTimesOut = 60*100;
//		Os__timer_start(&uiTimesOut);
		while((ptKeyboard->gen_status != DRV_ENDED)
				&&(pxIcc->gen_status != DRV_ENDED));
//		Os__timer_stop(&uiTimesOut);
		
		if(!uiTimesOut)
		{
			Os__abort_drv(drv_mmi);
			uiTimesOut = 10;
			Os__timer_start(&uiTimesOut);
			while(uiTimesOut);
			Os__timer_stop(&uiTimesOut);
			return ERR_CANCEL;
		}else if(ptKeyboard->gen_status == DRV_ENDED)
		{
			unsigned char ch;
			
			ch = ptKeyboard->xxdata[1];
			if(ch==KEY_CLEAR)
			{
				Os__abort_drv(drv_mmi);
				return ERR_CANCEL;
			}
			if(ch==KEY_ENTER)
			{
				Os__abort_drv(drv_mmi);
				return KEY_ENTER;
			}
		}else if(pxIcc->gen_status == DRV_ENDED)
		{
			Os__abort_drv(drv_mmi);
			length1 = pxIcc->xxdata[0];
			memcpy(eve_in.e_icc.icc_ATR,pxIcc,length1+4);
//			memcpy(NormalTransData.aucSerial,eve_in.e_icc.icc_ATR+11,8);

			if(ucResult!=SUCCESS)
			{
				//Os__abort_drv(drv_mmi);
				return ucResult;		
				
			}
			break;		
		}
	}
	return SUCCESS;
}
*/


unsigned int SLE4442_WaitInsert(void)
{
	unsigned int ret;            
	unsigned int uiI=0;
	unsigned char ucKey;
	unsigned char aucReq[8],aucCardId[16];
	unsigned char ucLen,ucCardClass;
	unsigned char aucRdInfo[64];
	unsigned char aucWrtInfo[64];
	unsigned char aucData[16];
	NEW_DRV_TYPE  new_drv;                                                    
	unsigned char icc_ATR[300];
	unsigned char  length1,ucResult;

	while(1)
	{
		Uart_Printf("\n1. Os_Wait_Event begin...");
		ret = Os_Wait_Event(KEY_DRV  | ICC_DRV , &new_drv, 1000*60);
		//Uart_Printf("\n2. Os_Wait_Event:%02x",ret);
		//Uart_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
		if(ret == 0)                                                              
		{                                                                         
			if(new_drv.drv_type == KEY_DRV)                                     
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED){
					ucKey=new_drv.drv_data.xxdata[1];
					Uart_Printf("\n ucKey:%02x.", ucKey);
					if(ucKey==KEY_CLEAR)
					{
						return ERR_CANCEL;
					}
					if(ucKey==KEY_ENTER)
					{
						return KEY_ENTER;
					}
				}
			}                                                                   
			else if(new_drv.drv_type == ICC_DRV)                                
			{                                                                   
				if( new_drv.drv_data.gen_status == DRV_ENDED )
				{
					length1 = new_drv.drv_data.xxdata[0];
                    memcpy(icc_ATR,(uchar*)&new_drv.drv_data,length1+4);
					/*if(icc_ATR[4]==ICC_ASY)
					{
						RunData.ucInputmode=1;
						NormalTransData.ucCardType=CARD_M1;
					}*/
					break;
				}
			}   
			Uart_Printf("\n OWE_NewDrvtts End.....\n");
		}                                                                         
		else                                                                      
		{                                                                         
			Untouch_OWE_RstErrInfo(ret);                                               
		}                                                                         
	}
	Uart_Printf("\n OWE_NewDrvtts rest:%02x.\n",ret);
	return 0;
}	



