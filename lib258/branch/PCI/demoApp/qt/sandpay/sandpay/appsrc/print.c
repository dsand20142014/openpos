#include <include.h>
#include <global.h>
#include <xdata.h>



void PRT_ConstantParam_L(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned char aucBuf[PARAM_MERCHANTNAMELEN+1],aucBuf1[PARAM_MERCHANTNAMELEN+1];
	unsigned short uioffset;
	switch(NORMALTRANS_ucTransType)
	{
		case TRANS_S_ONLINEPURCHASE:
		case TRANS_S_ONLINEPURCHASE_P:
		case TRANS_S_ZSHONLINEPURCHASE:
		case TRANS_S_PAYONLINE:
		case TRANS_S_VOIDPAYONLINE:
		case TRANS_S_VOID:
		case TRANS_S_VOID_P:
		case TRANS_S_PREAUTH:
		case TRANS_S_PREAUTHFINISH:
		case TRANS_S_PREAUTHFINISHVOID:
		case TRANS_S_PREAUTHVOID:
		case TRANS_S_MOBILECHARGEONLINE:
					
			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memcpy(aucPrintBuf,"欢迎使用",8);
			memcpy(&aucPrintBuf[8],NormalTransData.aucZoneName1,ZONE_NAME_LEN);
			Os__GB2312_xprint((unsigned char *)aucPrintBuf,FONT);
			break;
		case TRANS_S_PURCHASE:
		case TRANS_S_ZSHPURCHASE:
		case TRANS_S_PAY:
		case TRANS_S_RETURNCARD:
		case TRANS_S_RETURNCARD_P:
		case TRANS_S_PRELOAD:
		case TRANS_S_SALE:
		case TRANS_S_REFUND:
		case TRANS_S_MOBILECHARGE:
		case TRANS_S_EXPPURCHASE:
		case TRANS_S_PTCPURCHASE:
			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memcpy(aucPrintBuf,"欢迎使用",8);
			memcpy(&aucPrintBuf[8],NormalTransData.aucZoneName,ZONE_NAME_LEN);
			Os__GB2312_xprint((unsigned char *)aucPrintBuf,FONT);
			break;
		default:
			if(RunData.ucPtcode==1)
				Os__GB2312_xprint((unsigned char *)"欢迎使用杉德巍康卡",FONT);
			else if(RunData.ucPtcode==2)
				Os__GB2312_xprint((unsigned char *)"欢迎使用斯玛特卡",FONT);
			else if(RunData.ucPtcode==3)
				Os__GB2312_xprint((unsigned char *)"欢迎使用会员卡",FONT);
			else	
				Os__GB2312_xprint((unsigned char *)"欢迎使用杉德支付卡",FONT);
			break;
	}
    memset(aucPrintBuf,0,sizeof(aucPrintBuf));
    memset(aucBuf1,0,sizeof(aucBuf1));
    CFG_GetVersionString(1,aucPrintBuf);
    memcpy(aucBuf1,aucPrintBuf,24);
	Os__xprint((unsigned char *)aucBuf1);
	if(aucPrintBuf[24] != 0)
	    Os__xprint((unsigned char *)&aucPrintBuf[24]); 
	
	Os__xlinefeed(LINENUM*1);	
	/*print Merchant Name*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,DataSave0.ConstantParamData.aucMerchantName,PARAM_MERCHANTNAMELEN);
	uioffset=strlen((char *)aucBuf);
	aucPrintBuf[30] = '\0';

	memcpy(&aucPrintBuf[0],DataSave0.ConstantParamData.aucMerchantName,22);
	Os__GB2312_xprint(aucPrintBuf,FONT); 
	
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30] = '\0';
	memcpy(&aucPrintBuf[0],&DataSave0.ConstantParamData.aucMerchantName[22],18);
	Os__GB2312_xprint(aucPrintBuf,FONT); 

	
	Os__xlinefeed(LINENUM*0.5);	
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30] = '\0';
	if(RunData.ucPtcode==0)
		memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
	else
		bcd_asc(&aucPrintBuf[9],DataSave0.ConstantParamData.aucTid[RunData.ucPtcode-1],8);
	if(NormalTransData.ucPtCode==0x01&&NormalTransData.ucNewOrOld==0&&(NormalTransData.aucUserCardNo[0]>>4)==0x09)
	{
		/*print psam terminal */
		aucPrintBuf[17]='(';
		hex_asc(&aucPrintBuf[18],&NormalTransData.aucPsamTerminal[2],8);
		aucPrintBuf[26]=')';
	}
	Os__xprint(aucPrintBuf);
	Os__xlinefeed(LINENUM*0.5);
	
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30] = '\0';
	if(RunData.ucPtcode==0)
		memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
	else
	{
		bcd_asc(&aucPrintBuf[9],DataSave0.ConstantParamData.aucMid[RunData.ucPtcode-1],16);
		aucPrintBuf[24]=' ';
	}
	Os__xprint(aucPrintBuf);
	Os__xlinefeed(LINENUM*1.5);	
}

void PRT_TransType_L(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1],Date[10];
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30] = '\0';
	memset(Date,0,sizeof(Date));
	if(NormalTransData.ucTransType==TRANS_S_CHANGEEXP)
	{
		bcd_asc(Date,&NormalTransData.aucNewExpiredDate[1],2);
		memset(&Date[2],'/',1);
		bcd_asc(&Date[3],&NormalTransData.aucNewExpiredDate[2],2);
		memset(&Date[5],'/',1);
		bcd_asc(&Date[6],&NormalTransData.aucNewExpiredDate[3],2);
	}
	else
	{
		bcd_asc(Date,&NormalTransData.aucExpiredDate[1],2);
		memset(&Date[2],'/',1);
		bcd_asc(&Date[3],&NormalTransData.aucExpiredDate[2],2);
		memset(&Date[5],'/',1);
		bcd_asc(&Date[6],&NormalTransData.aucExpiredDate[3],2);
	}
	switch(NORMALTRANS_ucTransType)
	{
	case TRANS_S_PURCHASE:
		memcpy(&aucPrintBuf[6],(unsigned char*)"消费",4);
		break;
	case TRANS_S_RETURNCARD:
		memcpy(&aucPrintBuf[6],(unsigned char*)"回收消费",8);
		break;
	case TRANS_S_PURCHASE_P:
		memcpy(&aucPrintBuf[2],(unsigned char*)"汇点卡脱机消费",14);
		break;
	case TRANS_S_RETURNCARD_P:
		memcpy(&aucPrintBuf[2],(unsigned char*)"汇点卡回收消费",14);
		break;
	case TRANS_S_PREAUTH:
		memcpy(&aucPrintBuf[6],(unsigned char*)"预授权",6);
		break;
	case TRANS_S_PREAUTHFINISH:
		memcpy(&aucPrintBuf[6],(unsigned char*)"预授权完成",10);
		break;
	case TRANS_S_PREAUTHVOID:
		memcpy(&aucPrintBuf[6],(unsigned char*)"预授权撤销",10);
		break;
	case TRANS_S_PREAUTHFINISHVOID:
		memcpy(&aucPrintBuf[2],(unsigned char*)"预授权完成撤销",14);
		break;
	case TRANS_S_ONLINEPURCHASE:
		if(NormalTransData.ucReturnflag==true)
			memcpy(&aucPrintBuf[0],(unsigned char*)"联机回收消费",12);
		else
			memcpy(&aucPrintBuf[4],(unsigned char*)"联机消费",8);
		if(NormalTransData.ucHandFlag)
			memcpy(&aucPrintBuf[12],"(手工)",6);
		break;
	case TRANS_S_ONLINEPURCHASE_P:
		memcpy(&aucPrintBuf[6],(unsigned char*)"积点消费",8);
		break;
	case TRANS_S_VOID:
		memcpy(&aucPrintBuf[2],(unsigned char*)"联机撤销",8);
		if(NormalTransData.ucHandFlag)
			memcpy(&aucPrintBuf[10],"(手工)",6);
		break;
	case TRANS_S_VOID_P:
		memcpy(&aucPrintBuf[2],(unsigned char*)"积点消费撤销",12);
		break;
	case TRANS_S_EDLOAD:
    //case TRANS_MEM_EDLOAD:
		memcpy(&aucPrintBuf[6],(unsigned char*)"回收充值",8);
		break;
	case TRANS_S_RESETCARD:
		memcpy(&aucPrintBuf[6],(unsigned char*)"清余额",6);
		break;
	case TRANS_S_PRELOAD: 
		memcpy(&aucPrintBuf[6],(unsigned char*)"预充值",6);
		break;
	case TRANS_S_CHANGECD: 
		memcpy(&aucPrintBuf[6],(unsigned char*)"换卡",4);
		break;	
	case TRANS_S_SETTLE_ONE:
	case TRANS_S_SETTLE_TWO:
		memcpy(&aucPrintBuf[6],(unsigned char*)"结算",4);
		break;
	case TRANS_S_TERMINPRT:
		memcpy(&aucPrintBuf[2],(unsigned char*)"终端统计打印",12);
		break;
	case TRANS_S_CHANGEEXP: 
		memcpy(&aucPrintBuf[6],(unsigned char*)"续期",4);
		break;
	case TRANS_S_PAY:
		memcpy(&aucPrintBuf[6],(unsigned char*)"缴费",4);
		break;
	case TRANS_S_PAYNOBAR:
		memcpy(&aucPrintBuf[6],(unsigned char*)"缴费",4);
		break;
	case TRANS_S_SALE:
		memcpy(&aucPrintBuf[6],(unsigned char*)"卡激活",6);
		break;
	case TRANS_S_REFUND:
		memcpy(&aucPrintBuf[6],(unsigned char*)"退货",4);
		break;
	case TRANS_S_PAYONLINE:
		memcpy(&aucPrintBuf[6],(unsigned char*)"联机缴费",8);
		break;
	case TRANS_S_VOIDPAYONLINE:
		memcpy(&aucPrintBuf[2],(unsigned char*)"联机缴费撤销",12);
		break;
	case TRANS_S_MOBILECHARGE:
		memcpy(&aucPrintBuf[6],(unsigned char*)"脱机扣款",8);
		break;
	case TRANS_S_MOBILECHARGEONLINE:
		memcpy(&aucPrintBuf[6],(unsigned char*)"联机扣款",8);
		break;
	case TRANS_S_LOADONLINE: 
		memcpy(&aucPrintBuf[6],(unsigned char*)"联机充值",8);
		break;
	case TRANS_S_TRANSPURCHASE:
		memcpy(&aucPrintBuf[6],(unsigned char*)"移资扣款",8);
		break;
	case TRANS_S_TRANSFER: 
		memcpy(&aucPrintBuf[6],(unsigned char*)"移资充值",8);
		break;
	case TRANS_S_EXPPURCHASE: 
		memcpy(&aucPrintBuf[6],(unsigned char*)"过期卡消费",10);
		break;
	case TRANS_S_REFAPL:
		memcpy(&aucPrintBuf[6],(unsigned char*)"退货申请",8);
		break;
	case TRANS_S_VOIDREFAPL:
		memcpy(&aucPrintBuf[2],(unsigned char*)"撤销退货申请",12);
		break;
	case TRANS_S_CASHLOAD: 
		memcpy(&aucPrintBuf[6],(unsigned char*)"现金充值",8);
		break;
	case TRANS_S_PTCPURCHASE: 
		memcpy(&aucPrintBuf[2],(unsigned char*)"公交卡充值扣款",14);
		break;
	case TRANS_S_ONLINEREFUND: 
		memcpy(&aucPrintBuf[6],(unsigned char*)"联机退货",8);
		break;
	default:
		break;
	}
	if(NormalTransData.aucFunFlag[3]&&(
		NormalTransData.ucTransType==TRANS_S_PURCHASE
		||NormalTransData.ucTransType==TRANS_S_RETURNCARD
		||NormalTransData.ucTransType==TRANS_S_PURCHASE_P
		||NormalTransData.ucTransType==TRANS_S_RETURNCARD_P
		||NormalTransData.ucTransType==TRANS_S_PRELOAD
		||NormalTransData.ucTransType==TRANS_S_LOADONLINE
		||NormalTransData.ucTransType==TRANS_S_PAY
		||NormalTransData.ucTransType==TRANS_S_PAYNOBAR
		||NormalTransData.ucTransType==TRANS_S_CHANGEEXP
		||NormalTransData.ucTransType==TRANS_S_MOBILECHARGE
		||NormalTransData.ucTransType==TRANS_S_TRANSPURCHASE
		||NormalTransData.ucTransType==TRANS_S_REFUND)
		)
		memcpy(&aucPrintBuf[17],Date,9);
	Os__GB2312_xprint(aucPrintBuf,FONT);
}


void PRT_FeedLine(void)
{
	Os__xlinefeed(LINENUM*2.5);
	switch(NORMALTRANS_ucTransType)
	{
		case TRANS_S_RETURNCARD:
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*1);
			break;
		case TRANS_S_PAY:
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*3);
			break;
		case TRANS_S_VOID:
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*5.5);
			if(NormalTransData.ulRateAmount==0)
				Os__xlinefeed(LINENUM*4);
			break;
		case TRANS_S_ONLINEPURCHASE:
		case TRANS_S_TRANSFER:
		case TRANS_S_PREAUTH:
		case TRANS_S_PREAUTHFINISH:
		case TRANS_S_PREAUTHVOID:
		case TRANS_S_PREAUTHFINISHVOID:
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*2.5);
			if(NormalTransData.ulRateAmount==0)
				Os__xlinefeed(LINENUM*4);
			break;
		case TRANS_S_PRELOAD:
			Os__xlinefeed(LINENUM*3);
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*4);
			Os__xlinefeed(LINENUM*4);
			//Os__xlinefeed(LINENUM*3);
			break;
		case TRANS_S_REFUND:
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*5);
			//Os__xlinefeed(LINENUM*5);
			break;
		case TRANS_S_PURCHASE_P:
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*3);
			Os__xlinefeed(LINENUM*1);
			break;
		case TRANS_S_TRANSPURCHASE:
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*4);
			if(NormalTransData.ulRateAmount==0)
				Os__xlinefeed(LINENUM*4);
			break;
		default:
			Os__xlinefeed(LINENUM*5);
			Os__xlinefeed(LINENUM*4);
			Os__xlinefeed(LINENUM*3);
			if(NormalTransData.ulRateAmount==0)
				Os__xlinefeed(LINENUM*4);
			break;
				
	}
}
void PRT_ChangeParam_L(void)
{
	unsigned char 	aucPrintBuf[PRINT_MAXCHAR+1],uiI;
	unsigned long	ulAmount,ulYajin,ulAmountTotal;
	int i;

	Os__xlinefeed(LINENUM*1);
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30] = '\0';
    bcd_asc(&aucPrintBuf[0],NormalTransData.aucUserCardNo,16);
	if(DataSave0.ChangeParamData.aucTermFlag[8]||NormalTransData.aucFunFlag[20])
		memcpy(&aucPrintBuf[6],"******",6);
	OSPRN_XPrintASCII(0x31,aucPrintBuf);
		
	if(!RUNDATA_ucReprintFlag)
		Os__xlinefeed(LINENUM*1);	
		
	Os__xlinefeed(LINENUM/2);		
	PRT_TransType_L();
	Os__xlinefeed(LINENUM*0.5);
	
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]='\0';
	bcd_asc(&aucPrintBuf[5],DataSave0.ChangeParamData.aucBatchNumber,24);
	Os__xprint((unsigned char*)aucPrintBuf);
	Os__xlinefeed(LINENUM*0.5);
	
	PRT_DateTime();
	Os__xlinefeed(LINENUM*0.5);

	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30] = '\0';
	long_asc(&aucPrintBuf[5],6,&NORMALTRANS_ulTraceNumber);

	if(NormalTransData.ucTransType==TRANS_S_PREAUTH
	||NormalTransData.ucTransType==TRANS_S_PREAUTHVOID
	||NormalTransData.ucTransType==TRANS_S_PREAUTHFINISH
	||NormalTransData.ucTransType==TRANS_S_PREAUTHFINISHVOID)
	{
		memcpy(&aucPrintBuf[14],NormalTransData.aucAuthCode,14);
		//Os__GB2312_xprint(aucPrintBuf,FONT); 
		//Os__xlinefeed(1*LINENUM);
	}
	else	 if(DataSave0.ConstantParamData.ucYTJFlag)
	{
		memcpy(&aucPrintBuf[14],NormalTransData.aucCashTicket,6);
	}
		
	Os__xprint((unsigned char*)aucPrintBuf);
	Os__xlinefeed(LINENUM*1);
	if(NormalTransData.ucTransType==TRANS_S_VOID
		||NormalTransData.ucTransType==TRANS_S_VOID_P
		||NormalTransData.ucTransType==TRANS_S_PREAUTHFINISHVOID
		||NormalTransData.ucTransType==TRANS_S_VOIDPAYONLINE
		||NormalTransData.ucTransType==TRANS_S_VOIDREFAPL
		)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"原流水号:",9);
		memcpy(&aucPrintBuf[9],NormalTransData.aucOldTrace,6);
		Os__GB2312_xprint(aucPrintBuf,FONT);
	}
	
	if(NormalTransData.ucTransType==TRANS_S_RETURNCARD||NormalTransData.ucTransType==TRANS_S_RETURNCARD_P)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"实扣余额:",9);
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulPrevEDBalance,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"实扣押金:",9);
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulAmount-NormalTransData.ulPrevEDBalance,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
	}

	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30]='\0';
	switch(NORMALTRANS_ucTransType)
	{
	case TRANS_S_PURCHASE:
	case TRANS_S_EXPPURCHASE:
	case TRANS_S_PURCHASE_P:
		memcpy(aucPrintBuf,(unsigned char *)"消费金额:",9);
		break;
	case TRANS_S_RETURNCARD:
	case TRANS_S_RETURNCARD_P:
		memcpy(aucPrintBuf,(unsigned char *)"总扣金额:",9);
		break;
	case TRANS_S_PREAUTH:
		memcpy(aucPrintBuf,(unsigned char *)"授权金额:",9);
		break;
	case TRANS_S_PREAUTHFINISH:
		memcpy(aucPrintBuf,(unsigned char *)"消费金额:",9);
		break;
	case TRANS_S_PREAUTHVOID:
		memcpy(aucPrintBuf,(unsigned char *)"撤销金额:",9);
		break;
	case TRANS_S_PREAUTHFINISHVOID:
		memcpy(aucPrintBuf,(unsigned char *)"撤销金额:",9);
		break;
	case TRANS_S_ONLINEPURCHASE:
	case TRANS_S_ONLINEPURCHASE_P:
		memcpy(aucPrintBuf,(unsigned char *)"消费金额:",9);
		break;
	case TRANS_S_VOID:
	case TRANS_S_VOID_P:
		memcpy(aucPrintBuf,(unsigned char *)"撤销金额:",9);
		break;
	case TRANS_S_EDLOAD:
		memcpy(aucPrintBuf,(unsigned char *)"充值金额:",9);
		break;
	case TRANS_S_PRELOAD: 
	case TRANS_S_LOADONLINE: 
		memcpy(aucPrintBuf,(unsigned char *)"充值金额:",9);
		break;
	case TRANS_S_PAY:
	case TRANS_S_PAYNOBAR:
		memcpy(aucPrintBuf,(unsigned char *)"缴费金额:",9);
		break;
	case TRANS_S_PAYONLINE:
		memcpy(aucPrintBuf,(unsigned char *)"缴费金额:",9);
		break;
	case TRANS_S_VOIDPAYONLINE:
		memcpy(aucPrintBuf,(unsigned char *)"撤销金额:",9);
		break;
	case TRANS_S_MOBILECHARGE:
		memcpy(aucPrintBuf,(unsigned char *)"消费金额:",9);
		break;
	case TRANS_S_MOBILECHARGEONLINE:
		memcpy(aucPrintBuf,(unsigned char *)"消费金额:",9);
		break;
	case TRANS_S_REFUND:
		memcpy(aucPrintBuf,(unsigned char *)"退货金额:",9);
		break;
	case TRANS_S_CHANGECD:
		memcpy(aucPrintBuf,(unsigned char *)"转移金额:",9);
		break;
	case TRANS_S_CHANGEEXP:
		if(NormalTransData.ucPackFlag=='1')
			memcpy(aucPrintBuf,(unsigned char *)"扣手续费:",9);
		else
			memcpy(aucPrintBuf,(unsigned char *)"收手续费:",9);
		break;
	case TRANS_S_REFAPL:
		memcpy(aucPrintBuf,(unsigned char *)"申请金额:",9);
		break;
	case TRANS_S_VOIDREFAPL:
		memcpy(aucPrintBuf,(unsigned char *)"撤销金额:",9);
		break;
	case TRANS_S_TRANSPURCHASE:
		memcpy(aucPrintBuf,(unsigned char *)"转出金额:",9);
		break;
	case TRANS_S_TRANSFER:
		memcpy(aucPrintBuf,(unsigned char *)"转入金额:",9);
		break;
	default:
		memcpy(aucPrintBuf,(unsigned char *)"交易金额:",9);
		break;
	}
	if(TRANS_S_REFUND==NormalTransData.ucTransType
		&&!NormalTransData.aucFunFlag[1]&&NormalTransData.ulPrevEDBalance==0)
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulAmount+NormalTransData.ulYaAmount,2);
	else
	{
		if(NormalTransData.ucBonusflag)
			UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulAmount+NormalTransData.ulRateAmount,2);
		else
			UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulAmount-NormalTransData.ulRateAmount,2);
	}
	if(NormalTransData.ucTransType==TRANS_S_VOID
		||NormalTransData.ucTransType==TRANS_S_PREAUTHFINISHVOID
		||NormalTransData.ucTransType==TRANS_S_PREAUTHVOID
		||NormalTransData.ucTransType==TRANS_S_VOIDREFAPL
		)
	{
		for(i=0;i<10;i++)
		{
			if(aucPrintBuf[9+i]!=' ')
			{
				aucPrintBuf[8+i]='-';
				break;
			}
		}
	}
	OSPRN_XPrintGB2312(0x31,FONT,aucPrintBuf);
	//EXTPRINT_ext_xprintGB2312(0x31,FONT,aucPrintBuf);
	if(NormalTransData.ulRateAmount&&NormalTransData.ucTransType!=TRANS_S_CHANGEEXP)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		if(NormalTransData.ucBonusflag)
			memcpy(aucPrintBuf,(unsigned char *)"返利:       ",9);
		else
			memcpy(aucPrintBuf,(unsigned char *)"服务费:     ",9);
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulRateAmount,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		if(NormalTransData.ucBonusflag)
			memcpy(aucPrintBuf,(unsigned char *)"实扣金额:",9);
		else
			memcpy(aucPrintBuf,(unsigned char *)"总扣金额:",9);
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulAmount,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);

	}

	if(NormalTransData.ucTransType==TRANS_S_MOBILECHARGE
	||NormalTransData.ucTransType==TRANS_S_MOBILECHARGEONLINE)
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"手机号:",7);
		memcpy(&aucPrintBuf[7],NormalTransData.aucBarCode,11);
		Os__GB2312_xprint(aucPrintBuf);
	}
	if(NORMALTRANS_ucTransType==TRANS_S_PURCHASE_P)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"还需扣款:",9);
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulNeedCash,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
	}
	
}


void PRT_PrintTicket_L(void)
{
	unsigned char aucBuf[25];
	OSPRN_XReverseFeed(LINENUM*5);
	
	Os__xlinefeed(0.5*LINENUM);
	PRT_ConstantParam_L();
	if(RUNDATA_ucReprintFlag)
		Os__GB2312_xprint((unsigned char *)"重打交易",FONT);	
	else
		Os__xlinefeed(1*LINENUM);
	
	if(NormalTransData.aucFunFlag[18])
		Os__GB2312_xprint((unsigned char *)"(测试交易)",FONT);	
	else
		Os__xlinefeed(1*LINENUM);
	
	PRT_ChangeParam_L();
	
	PRT_PrintBalance();
	if(NormalTransData.ucReturnflag==true)
		Os__GB2312_xprint((uchar *)"此卡应回收",FONT);
		
	if(NormalTransData.ucTransType==TRANS_S_PURCHASE_P
	||NormalTransData.ucTransType==TRANS_S_RETURNCARD_P)
	{
		Os__GB2312_xprint((unsigned char *)"汇点卡卡号:",FONT); 
		memset(aucBuf,0,sizeof(aucBuf));
		bcd_asc(aucBuf,NormalTransData.aucOldCardNo,16);
		Os__xprint(aucBuf); 
	}
	else
		Os__xlinefeed(1*LINENUM);

		PRT_PrintTAC();
	
	if(NormalTransData.ucTransType==TRANS_S_PAYONLINE||NormalTransData.ucTransType==TRANS_S_PAY)
	{
		Os__xlinefeed(1*LINENUM);
		PrintBar(NormalTransData.ulTraceNumber);
	}
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,DataSave0.ConstantParamData.aucTerminalID,8);
	aucBuf[8]=' ';
	memcpy(&aucBuf[9],DataSave0.ConstantParamData.aucMerchantID,15);
	Os__xprint(aucBuf);
	PRT_FeedLine();
	Os__clr_display(255);
}


void PRT_PrintTicket(void)
{
	unsigned char aucBuf[40],uctPt,aucName1[50];
	unsigned long ulAmount;
	int i;
unsigned char aucName[]=
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x80,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xF0,0xFF,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xFC,0xFF,0xFF,0xFF,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x80,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,
0x00,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0xFF,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x80,0x00,0x00,0x00,0xF8,0xFF,0xFF,0x03,0xF0,0xFF,0xFF,0x07,0xFE,0xFF,0xFF,0x03,
0x00,0x00,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0xF0,0xFF,0x9F,0x0F,0xFE,0xFF,0xFF,0x07,
0x00,0x00,0x00,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
0xC0,0xDE,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0xF0,0xFF,0x7F,0x1C,0xFC,0xFF,0xFF,0x07,
0x00,0x00,0x00,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0x00,0x00,0xF8,0x39,0x00,0x00,0xFE,0x0F,
0x00,0x00,0x80,0xF9,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,
0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0xF0,0xFF,0xF9,0x33,0xFE,0x7F,0xFE,0x0F,
0x00,0x00,0xF0,0xFC,0xFF,0x1F,0x00,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFE,0x7F,0x00,0x00,0x00,0xF0,0xFF,0xFB,0x37,0xFF,0x7F,0xFE,0x0F,
0x00,0x00,0x78,0xFC,0xFF,0x01,0xF8,0xF3,0xFF,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFE,0x7F,0x00,0x00,0x00,0xF0,0xFF,0xFB,0x3F,0xFF,0x7F,0xFE,0x0F,
0x00,0x00,0x1C,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFE,0x3F,0x00,0x00,0x00,0x00,0xFF,0xFB,0x3F,0xCE,0x7F,0xFE,0x0F,
0x00,0x00,0x06,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFE,0x3F,0x00,0x00,0x00,0x00,0xFF,0xF9,0x1F,0xE0,0x7F,0xFF,0x07,
0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFE,0x3F,0x00,0x00,0x00,0xE0,0xEF,0xF8,0x1F,0xF0,0x38,0xFF,0x07,
0x00,0x80,0xC3,0xFF,0xFF,0x01,0xC0,0xFF,0xFF,0xFF,0x01,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFE,0x3F,0x00,0x00,0xFC,0xFF,0x7B,0xF8,0x1F,0x3E,0x0E,0xFF,0x07,
0x00,0xC0,0xE1,0xFF,0x3F,0x00,0x00,0xFE,0xFF,0xFF,0x01,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFF,0x3F,0x00,0x00,0xF0,0x3F,0x3C,0xF8,0xFF,0x9F,0x07,0xFF,0x03,
0x00,0xE0,0xF1,0xFF,0x0F,0x00,0x00,0xFC,0xFF,0xFF,0x03,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFE,0x3F,0x00,0x00,0x80,0xFF,0x0F,0xE0,0xFF,0xF1,0x81,0xFF,0x03,
0x00,0x60,0xE0,0xFF,0x03,0x00,0x00,0xF8,0xFF,0xFF,0x03,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFF,0x1F,0x00,0x00,0x00,0x02,0x03,0x00,0xFE,0x7F,0x00,0x00,0x00,
0x00,0x60,0xFC,0xFF,0x01,0x00,0x00,0xE0,0xFF,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFF,0x1F,0x00,0x00,0x00,0x00,0x08,0x00,0xC0,0x00,0x00,0x00,0x00,
0x00,0x30,0xF8,0x7F,0x00,0x00,0x00,0xC0,0xFF,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFF,0x1F,0x00,0x00,0x00,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,
0x00,0x18,0xFC,0x3F,0x00,0x00,0x00,0x80,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,
0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,
0x00,0x18,0xFE,0x0F,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,
0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0xD8,0xFF,0xFF,0xE7,0xFF,0xFF,0xFF,0x01,
0x00,0x0C,0xFE,0x0F,0x00,0x00,0x00,0x00,0xFF,0xFF,0x1F,0x00,0x00,0x00,0x00,0x00,
0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0xF8,0x3F,0x20,0xFE,0x0F,0xC0,0xFF,0x01,
0x00,0x88,0xFF,0x07,0x00,0x00,0x00,0x00,0xFE,0xFF,0x1F,0x00,0x00,0x00,0x00,0x00,
0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0xD8,0x3F,0x00,0xFE,0x07,0xC0,0xFF,0x01,
0x00,0xCC,0xFF,0x01,0x10,0x00,0x00,0x00,0xFC,0xFF,0x1F,0x00,0x00,0x00,0x00,0x00,
0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0xFC,0x1F,0x00,0xFE,0x07,0xC0,0xFF,0x01,
0x00,0xC4,0xFF,0x01,0x10,0x00,0x00,0x00,0xFC,0xFF,0x1F,0x00,0x00,0x00,0x00,0x00,
0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,
0x00,0xE6,0xFF,0x00,0x18,0x00,0x00,0x00,0xFC,0xFF,0x3F,0x00,0x00,0x00,0x00,0x00,
0xFC,0xFF,0xDF,0xFF,0xEF,0xFF,0xFF,0x01,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,
0x00,0xF6,0x7F,0x00,0x08,0x00,0x00,0x00,0xF8,0xFF,0x3F,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xC0,0xFF,0x0F,0x00,0x00,0x00,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,
0x00,0xF6,0xFF,0x00,0x0C,0x00,0x00,0x00,0xF8,0xFF,0xBF,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xC0,0xFF,0x1F,0x00,0x00,0x00,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
0x00,0xFF,0x3F,0x00,0x0C,0x00,0x00,0x00,0xF8,0xFF,0xBF,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xC0,0xFF,0x1F,0x00,0x00,0x00,0xFE,0x1F,0x00,0xFF,0x07,0xC0,0xFF,0x00,
0x00,0xFF,0x3F,0x00,0x1E,0x00,0x00,0x00,0xF8,0xFF,0xBF,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xE0,0xFF,0x1F,0x00,0x00,0x00,0xFE,0x0F,0x00,0xFF,0x07,0xE0,0xFF,0x00,
0x00,0xFF,0x1F,0x00,0x1E,0x00,0x00,0x00,0xF8,0xFF,0xBF,0x01,0x00,0x00,0xFC,0x00,
0x00,0x00,0xE0,0xFF,0x3F,0x00,0x00,0x00,0xFE,0x0F,0x00,0xFF,0xFF,0xFF,0xFF,0x00,
0x80,0xFF,0x1F,0x00,0x1E,0x00,0x00,0x00,0xF8,0xFF,0xBF,0x01,0x00,0x80,0x01,0x00,
0x00,0x00,0xF0,0xFF,0x3F,0x00,0x00,0x00,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
0x80,0xFF,0x0F,0x00,0x3A,0x00,0x00,0x00,0xF8,0xFF,0xBF,0x01,0x00,0xF0,0x00,0xC0,
0x1F,0x00,0xF0,0xFF,0x7F,0x00,0x00,0x00,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x00,
0x80,0xFF,0x0F,0x00,0x39,0x00,0x00,0x00,0xFC,0xFF,0xBF,0x01,0x00,0x18,0x00,0xFC,
0xFF,0x00,0xF8,0xFF,0xFF,0x00,0x00,0x00,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x00,
0x80,0xFF,0x0F,0x00,0x3F,0x00,0x00,0x00,0xFC,0xFF,0xBF,0x01,0x00,0x1E,0x00,0xFF,
0x7F,0x00,0xFC,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xC0,0xFF,0x07,0x00,0x3D,0x00,0x00,0x00,0xFE,0xFF,0xBF,0x01,0x80,0x0F,0xE0,0xFF,
0x07,0x00,0xFC,0xFF,0xFF,0x01,0x00,0x00,0x00,0xF8,0x3F,0x00,0xF8,0x7F,0x00,0x00,
0xC0,0xFF,0x07,0x00,0x7D,0x00,0x00,0x00,0xFE,0xFF,0xBF,0x01,0xE0,0x03,0xF8,0xFF,
0x01,0x00,0xFE,0xFF,0xFF,0x01,0x00,0x00,0x00,0xFC,0x3F,0x00,0xF8,0x7F,0x00,0x00,
0xC0,0xFF,0x07,0x00,0xFD,0x00,0x00,0x00,0xFF,0xFF,0x9F,0x01,0xF0,0x1B,0xFE,0x7F,
0x00,0x00,0xFE,0x9F,0xFF,0x03,0x00,0x80,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,
0xC0,0xFF,0x07,0x00,0xFF,0x01,0x00,0x80,0xFF,0xFF,0x9F,0x01,0xF8,0x8F,0xFF,0x1F,
0x00,0x00,0xFF,0x1F,0xFF,0x03,0x00,0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,
0xC0,0xFF,0x07,0x00,0xFF,0x03,0x00,0x80,0xFF,0xFF,0xDF,0x01,0xFC,0xEF,0xFF,0x0F,
0x00,0x80,0xFF,0x0F,0xFF,0x07,0x00,0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,
0xC0,0xFF,0x07,0x00,0xFF,0x07,0x00,0xE0,0xFF,0xFF,0xDF,0x00,0xFF,0xFF,0xFF,0x03,
0x00,0xC0,0xFF,0x07,0xFF,0x0F,0x00,0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,
0xC0,0xFF,0x07,0x00,0xFF,0x0F,0x00,0xF8,0xFF,0xFF,0xCF,0x80,0xFF,0xFF,0xFF,0x01,
0x00,0xE0,0xFF,0x03,0xFE,0x0F,0x00,0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
0xC0,0xFF,0x07,0x00,0xFF,0x7F,0x00,0xFF,0xFF,0xFF,0xCF,0xC0,0xFF,0xFF,0xF7,0x00,
0x00,0xF0,0xFF,0x03,0xFC,0x1F,0x00,0x00,0x00,0xFE,0x3F,0x00,0xFC,0x3F,0x00,0x00,
0xC0,0xFF,0x07,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC7,0xE0,0xFF,0xFF,0x7F,0x00,
0x00,0xF8,0xFF,0x01,0xFC,0x3F,0x00,0x00,0x00,0xFE,0x1F,0x00,0xFC,0x3F,0x00,0x00,
0xC0,0xFF,0x07,0x00,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xC7,0xE0,0xFF,0xFF,0x3D,0x00,
0x00,0xFC,0xFF,0x00,0xF8,0x3F,0x00,0xC0,0x00,0xFE,0x1F,0x00,0xFC,0x3F,0x00,0x00,
0xC0,0xFF,0x07,0x00,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0x63,0xF0,0xFF,0xFF,0x1F,0x00,
0x00,0xFF,0x7F,0x00,0xF0,0x7F,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,
0xC0,0xFF,0x07,0x00,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0x63,0xF8,0xFF,0xFF,0x0F,0x00,
0x80,0xFB,0x3F,0x00,0xE0,0xFF,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,
0xC0,0xFF,0x07,0x00,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0x71,0xFC,0xFF,0xFF,0x07,0x00,
0xE0,0xFB,0x1F,0x00,0xC0,0xFF,0x01,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,
0xC0,0xFF,0x07,0x00,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0x79,0xFE,0xFF,0xBF,0x07,0x00,
0xF0,0xFF,0x07,0x00,0x80,0xFF,0x07,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,
0xC0,0xFF,0x0F,0x00,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0x38,0xFE,0xFF,0x9F,0x03,0x00,
0x3C,0xFF,0x03,0x00,0x00,0xFF,0x0F,0x00,0x80,0xFF,0x0F,0x00,0xFE,0x3F,0x00,0x00,
0xC0,0xFF,0x0F,0x00,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0x1C,0xFF,0xFF,0xCF,0x00,0x00,
0x8F,0xFF,0x00,0x00,0x20,0x7C,0x1F,0x00,0x80,0xFF,0x03,0x00,0xFC,0x3F,0x00,0x00,
0xC0,0xFF,0x0F,0x00,0xF8,0xFF,0xFF,0xFF,0xFF,0x7F,0x9E,0xFF,0xFF,0xE7,0x00,0xC0,
0xE7,0x3F,0x00,0x00,0x00,0xF8,0xFD,0x00,0xC0,0xFF,0x01,0x00,0xFC,0x3F,0x00,0x00,
0x80,0xFF,0x1F,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0x3F,0xCF,0xFF,0xFF,0xF3,0x00,0xE0,
0xFC,0x0F,0x00,0x00,0x00,0xF0,0xE7,0x01,0xF8,0xFF,0x00,0x00,0xF8,0x7F,0x00,0x00,
0x80,0xFF,0x1F,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0x9F,0xC7,0xFF,0xFF,0x79,0x00,0xB8,
0xFF,0x00,0x00,0x00,0x00,0xC0,0x1F,0xFF,0xFF,0x3F,0x00,0x00,0xF0,0xFF,0x00,0x00,
0x80,0xFF,0x1F,0x00,0xE0,0xFF,0xFF,0xFF,0xFF,0xEF,0xF3,0xFF,0xFF,0x3B,0x00,0xFE,
0x1F,0x00,0x00,0x00,0x00,0x00,0xFF,0xF9,0xFF,0x0F,0x00,0x00,0xE0,0xF9,0x03,0x00,
0x00,0xFF,0x3F,0x00,0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xF9,0xFF,0xFF,0x3F,0xC0,0xFF,
0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0xFF,0xFF,0x01,0x00,0x00,0xC0,0xC7,0x0F,0x00,
0x00,0xFF,0x3F,0x00,0x80,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0xFF,0xFF,0x1C,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0x3F,0x00,0x00,0x00,0x00,0x1F,0x3F,0x00,
0x00,0xFF,0x7F,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x7F,0xF8,0xFF,0x7F,0x0E,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0xFF,0x01,
0x00,0xFF,0x7F,0x00,0x00,0xFE,0xFF,0xFF,0xFF,0x1F,0xFC,0xFF,0x3F,0x0F,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0x01,
0x00,0xFE,0xFF,0x00,0x00,0xF8,0xFF,0xFF,0xFF,0x0F,0xFE,0xFF,0x3F,0x07,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xFE,0xFF,0x00,0x00,0xE0,0xFF,0xFF,0xFF,0x03,0xFF,0xFF,0x9F,0x03,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xFE,0xFF,0x01,0x00,0xC0,0xFF,0xFF,0xFF,0x80,0xFF,0xFF,0xCF,0x03,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xFC,0xFF,0x03,0x00,0x00,0xFE,0xFF,0x3F,0xC0,0xFF,0xFF,0xE7,0x01,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xF8,0xFF,0x03,0x00,0x00,0xF0,0xFF,0x0F,0xE0,0xFF,0xFF,0xFF,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xFF,0xF1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xF8,0xFF,0x07,0x00,0x00,0x00,0x7E,0x00,0xF0,0xFF,0xFF,0x7F,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0xFF,0xF1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xF8,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0xFC,0xFF,0xFF,0x7F,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xFF,0xF9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xF0,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0xFE,0xFF,0xFF,0x3D,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x01,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xE0,0xFF,0x3F,0x00,0x00,0x00,0x00,0x80,0xFF,0xFF,0xFF,0x1F,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xE0,0xFF,0x7F,0x00,0x00,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0x1F,0x00,0x00,0x31,
0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xE0,0xFF,0xFF,0x01,0x00,0x00,0x00,0xF0,0xFF,0xFF,0xFF,0x07,0x00,0x00,0xFF,
0x06,0x1A,0x80,0x0C,0x12,0x0A,0xF8,0x11,0x18,0x00,0xE8,0x03,0x80,0xED,0x20,0x00,
0x00,0xC0,0xFF,0xFF,0x07,0x00,0x00,0x00,0xFC,0xFF,0xFF,0xFF,0x07,0x00,0xC0,0xFF,
0x0F,0x1F,0xC0,0x0F,0x1F,0x1F,0xFF,0x3F,0x78,0xF0,0xFF,0x1F,0x80,0xFF,0x3F,0x00,
0x00,0x80,0xFF,0xFF,0x0F,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x03,0x00,0xF0,0xFF,
0x0F,0x1F,0x80,0x07,0xFF,0xBF,0xFF,0x3F,0x78,0xF8,0xFF,0x1F,0xE0,0xFF,0x3F,0x00,
0x00,0x00,0xFF,0xFF,0xFF,0x01,0x00,0xC8,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0xF0,0xC7,
0x0F,0x1F,0x80,0x07,0xFF,0x3F,0xFF,0x1F,0x7C,0xF8,0x1F,0x3F,0xF0,0xF7,0x3F,0x00,
0x00,0x00,0xFF,0xFF,0xFF,0x1F,0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0xF8,0x00,
0x04,0x1F,0x80,0x87,0xFF,0x0F,0x7C,0x0C,0x7C,0xF8,0x07,0x3E,0xF8,0x01,0x3E,0x00,
0x00,0x00,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x00,0x00,0xF8,0x00,
0x00,0x1F,0x80,0x87,0x3F,0x00,0x7C,0x00,0x7C,0xF8,0x01,0x3E,0xFC,0x00,0x3E,0x00,
0x00,0x00,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3F,0x00,0x00,0xF8,0x01,
0x00,0x0F,0xC0,0x87,0x1F,0x00,0x7E,0x00,0x3C,0xF8,0x00,0x7E,0x7C,0x00,0x3E,0x00,
0x00,0x00,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x1F,0x00,0x00,0xF8,0x01,
0x00,0x0F,0xC0,0x83,0x0F,0x00,0x7E,0x00,0x3E,0xFC,0x00,0x3E,0x3C,0x00,0x3E,0x00,
0x00,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x00,0xF8,0x1F,
0x00,0x0F,0xC0,0x83,0x0F,0x00,0x3C,0x00,0x3E,0xFC,0x00,0x3E,0x3E,0x00,0x3E,0x00,
0x00,0x00,0xE0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x00,0x00,0xF8,0xFF,
0x80,0x0F,0xC0,0x83,0x07,0x00,0x3E,0x00,0x3C,0x7C,0x00,0x3E,0x1E,0x00,0x3E,0x00,
0x00,0x00,0x80,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0x00,0x00,0xE0,0xFF,
0x83,0x0F,0xC0,0x83,0x07,0x00,0x3E,0x00,0x3E,0x7C,0x00,0x3E,0x1E,0x00,0x1E,0x00,
0x00,0x00,0x80,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x00,0xC0,0xFF,
0x87,0x07,0xC0,0xC3,0x07,0x00,0x3E,0x00,0x3E,0x7C,0x00,0x3E,0x1E,0x00,0x1F,0x00,
0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xF8,
0x87,0x07,0xE0,0xC3,0x07,0x00,0x3E,0x00,0x1E,0x7C,0x00,0x3F,0x1F,0x00,0x1F,0x00,
0x00,0x00,0x00,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x00,0x00,0x00,0x00,0xC0,
0x87,0x07,0xE0,0xC3,0x07,0x00,0x3F,0x00,0x3E,0x7E,0x00,0x1F,0x1E,0x00,0x1F,0x00,
0x00,0x00,0x00,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3F,0x00,0x00,0x00,0x00,0xC0,
0xC7,0x07,0xE0,0xC1,0x07,0x00,0x1F,0x00,0x1F,0x3E,0x00,0x1F,0x3E,0x00,0x1F,0x00,
0x00,0x00,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0xC0,
0x87,0x07,0xF0,0xC3,0x03,0x00,0x1F,0x00,0x1F,0x3E,0x00,0x1F,0x3E,0x80,0x1F,0x00,
0x00,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0xFF,0xBF,0xFF,0x03,0x00,0x00,0x00,0x0E,0xC0,
0x87,0x07,0xF8,0xC1,0x03,0x00,0x1F,0x00,0x1F,0x3E,0x00,0x1F,0x3E,0xC0,0x0F,0x00,
0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xF3,0xFF,0x01,0x00,0x00,0x00,0x3E,0xF0,
0x83,0x3F,0xFF,0xC1,0x03,0x00,0x1F,0x00,0x1F,0x3E,0x00,0x1F,0xFE,0xFF,0x0F,0x00,
0x00,0x00,0x00,0x00,0xFC,0xFF,0xFF,0x7F,0xFF,0x7F,0x00,0x00,0x00,0x00,0xFE,0xFF,
0x03,0xFF,0xFF,0xE1,0x03,0x00,0x1F,0x00,0x1F,0x3F,0x00,0x1F,0xFC,0xFF,0x0F,0x00,
0x00,0x00,0x00,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0xFE,0xFF,
0x01,0xFF,0xFF,0xE0,0x01,0x00,0x1F,0x00,0x1F,0x3F,0x00,0x1F,0xFC,0xFF,0x0F,0x00,
0x00,0x00,0x00,0x00,0x80,0xFF,0xFF,0xFF,0xFF,0x03,0x00,0x00,0x00,0x00,0xF4,0x5F,
0x00,0xFC,0xFD,0xC0,0x03,0x00,0x0F,0x00,0x1F,0x1F,0x80,0x0F,0xF0,0x9F,0x0F,0x00,
0x00,0x00,0x00,0x00,0x00,0xFE,0xFF,0xFF,0xFF,0x01,0x00,0x00,0x00,0x00,0x10,0x00,
0x00,0x00,0x00,0x00,0x02,0x00,0x01,0x00,0x06,0x1E,0x80,0x0F,0x00,0x80,0x07,0x00,
0x00,0x00,0x00,0x00,0x00,0xF0,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x0F,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x07,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xF0,0x07,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFC,0x03,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x01,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x3F,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x03,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xFC,0x03,0x00,0x00,0xC0,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x18,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,
0x00,0x00,0xFE,0x0F,0x00,0x00,0xE0,0x3F,0x00,0x00,0xE0,0xFF,0x3F,0x00,0x00,0xFE,
0x3B,0x03,0x00,0x00,0xFF,0xFF,0x00,0x00,0x20,0xFE,0x07,0x00,0x00,0x30,0xFC,0x00,
0x00,0x00,0xFF,0x1F,0x00,0x00,0xF8,0x7F,0x00,0x00,0xC0,0xFF,0x3F,0x00,0x00,0xFC,
0xBF,0x03,0x00,0x80,0xFF,0xFF,0x00,0x00,0xE0,0xFF,0x0F,0x00,0x00,0x70,0xFE,0x00,
0x00,0x00,0x3F,0x3F,0x00,0x00,0xF8,0xFF,0x00,0x00,0xE0,0xFF,0x3F,0x00,0x00,0xB8,
0xB1,0x03,0x00,0x80,0x03,0xE0,0x00,0x00,0xE0,0x3F,0x00,0x00,0x00,0x70,0xFE,0x00,
0x00,0x80,0x1F,0x3E,0x00,0x00,0x7C,0xF0,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x98,
0xF7,0x07,0x00,0x80,0xC3,0xF8,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0xFE,0xCF,0x00,
0x00,0x80,0x0F,0x3E,0x00,0x00,0x7E,0xE0,0x01,0x00,0x00,0x0E,0x00,0x00,0x00,0x98,
0xFF,0x0F,0x00,0x00,0xDF,0xFB,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0xFE,0xCF,0x00,
0x00,0x00,0x04,0x3F,0x00,0x00,0x3C,0xC0,0x01,0x00,0x00,0xFE,0x0F,0x00,0x00,0xF8,
0xFF,0x07,0x00,0x80,0xFF,0xFB,0x00,0x00,0xE0,0xFF,0x0F,0x00,0x00,0xFE,0xCF,0x00,
0x00,0x00,0x80,0x3F,0x00,0x00,0x1E,0x00,0x00,0x00,0x00,0xFE,0x07,0x00,0x00,0xF8,
0x61,0x00,0x00,0x80,0xFB,0xEF,0x00,0x00,0xC0,0xFF,0x07,0x00,0x00,0x38,0xCE,0x00,
0x00,0x00,0xE0,0x1F,0x00,0x00,0x1E,0x00,0x00,0x00,0x00,0x0F,0x06,0x00,0x00,0x98,
0x61,0x00,0x00,0x00,0x73,0xEE,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x78,0xCE,0x00,
0x00,0x00,0xE0,0x1F,0x00,0x00,0x1E,0xFE,0x01,0x00,0x00,0x07,0x07,0x00,0x00,0xB8,
0xF3,0x00,0x00,0x80,0x73,0xEE,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0xFC,0xCE,0x00,
0x00,0x00,0xE0,0x3F,0x00,0x00,0x1E,0xFC,0x01,0x00,0x00,0x07,0x06,0x00,0x00,0xF8,
0xFF,0x0F,0x00,0x80,0x73,0xEE,0x00,0x00,0xF8,0xFF,0x3F,0x00,0x00,0xFC,0xCF,0x00,
0x00,0x00,0x00,0x3E,0x00,0x00,0x1E,0xFC,0x01,0x00,0x00,0xFF,0x07,0x00,0x00,0xF8,
0xFF,0x0F,0x00,0x00,0x7B,0xFF,0x00,0x00,0xF8,0xFF,0x3F,0x00,0x00,0xFE,0xCF,0x00,
0x00,0x00,0x00,0x7C,0x00,0x00,0x1E,0xDC,0x01,0x00,0x80,0xFF,0x07,0x00,0x00,0x98,
0xF3,0x00,0x00,0x80,0xDB,0xFF,0x00,0x00,0xF0,0xBB,0x1F,0x00,0x00,0xBE,0xCF,0x00,
0x00,0x00,0x03,0x7C,0x00,0x00,0x1C,0xC0,0x01,0x00,0x00,0xEE,0x07,0x00,0x00,0xD8,
0xF7,0x00,0x00,0x00,0xCF,0xFB,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x72,0xC6,0x00,
0x00,0x80,0x0F,0x7C,0x00,0x00,0x3C,0xE0,0x01,0x00,0x00,0x00,0x07,0x00,0x00,0xFC,
0xF7,0x01,0x00,0x80,0xCF,0xF9,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x72,0xC7,0x06,
0x00,0x80,0x0F,0x3C,0x00,0x00,0xFC,0xF1,0x01,0x00,0xF0,0xFF,0x3F,0x00,0x00,0xFC,
0xBB,0x03,0x00,0x80,0x87,0xE1,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x30,0xE7,0x06,
0x00,0x00,0x7F,0x3E,0x00,0x00,0xF8,0xFF,0x01,0x00,0xF0,0xFF,0x7F,0x00,0x00,0x80,
0x9B,0x0F,0x00,0x80,0x03,0xE0,0x00,0x00,0x00,0x3A,0x00,0x00,0x00,0xB0,0xE3,0x07,
0x00,0x00,0xFE,0x1F,0x00,0x00,0xF0,0xFF,0x00,0x00,0xF0,0xFF,0x7F,0x00,0x00,0x80,
0x1F,0x0F,0x00,0x00,0x03,0xF8,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,0xF8,0xC1,0x07,
0x00,0x00,0xF8,0x0F,0x00,0x00,0xC0,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
0x0F,0x06,0x00,0x00,0x03,0x70,0x00,0x00,0x00,0x1E,0x00,0x00,0x00,0xF0,0x80,0x03,
0x00,0x00,0x90,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

};
    //Os__clr_display(255);
    //Os__GB2312_display(0,0,((uchar *)"正在打印"));
    //if(NormalTransData.ucReturnflag==true)
    //    Os__GB2312_display(2,0,((uchar *)"此卡应回收"));
	if(NormalTransData.ucTransType==TRANS_S_LOADONLINE)
	{
		memset(aucBuf,0x00,sizeof(aucBuf));
		if(NormalTransData.ucCardLoadFlag)
		{
			if(NormalTransData.ucZeroFlag)
			{
				if(NormalTransData.ulTrueBalance+NormalTransData.ulAmount>NormalTransData.ulYaAmount)
					ulAmount=NormalTransData.ulTrueBalance+NormalTransData.ulAmount-NormalTransData.ulYaAmount;
				else
					ulAmount=0;
			}else
				ulAmount=NormalTransData.ulTrueBalance+NormalTransData.ulAmount-NormalTransData.ulYaAmount;
		}
		else
			ulAmount=NormalTransData.ulAfterEDBalance;
		UTIL_Form_Montant(aucBuf,ulAmount,2);
		Os__GB2312_display(1,1,(uchar *)"卡内余额:");
		Os__GB2312_display(2,0,(uchar *)aucBuf);
		Os__light_on_pp();
		Os__clr_display_pp(255);
		Os__GB2312_display_pp(0,0,(unsigned char *)"充值成功");	
		Os__GB2312_display_pp(1,0,(unsigned char *)aucBuf);
	}
	//为了保护打印头，打印时关闭背光
    //Os__light_off();
    //if(DataSave0.ConstantParamData.Pinpadflag)
    //	Os__light_off_pp();
	uctPt=RunData.ucPtcode;
	if(DataSave0.ChangeParamData.aucTermFlag[1]==1)
		RunData.ucPtcode=NormalTransData.ucPtCode;
	else
		RunData.ucPtcode=0;
	if(DataSave0.ConstantParamData.prnflagL)
	{
		PRT_PrintTicket_L();
		RunData.ucPtcode=uctPt;
		return ;
	}
	if(NormalTransData.ucPtCode==1&&bcd_long(NormalTransData.aucZoneCode,6)/1000==23)
	{
        //Os__clr_display_pp(255);
        //Os__GB2312_display_pp(0,0,(uchar *)"交易成功");
		for(i=0;i<110;i++)
		{
			memcpy(aucName1,&aucName[i*32],32);
			Os__graph_xprint (aucName1, 32) ;
		}
	}

    Uart_Printf("PRINTTIMES~~~~~~~~~~~~~~%d  \n",PRINTTIMES);
	for(i=0;i<PRINTTIMES;i++)
	{
		if(PRINTTIMES==2)
		{
			if(DataSave0.ConstantParamData.ucKFCFlag)
			{
				if(NormalTransData.ucPtCode==2)
					Os__GB2312_xprint((unsigned char *)"     斯玛特卡",FONT);
				else if(NormalTransData.ucPtCode==1)
					Os__GB2312_xprint((unsigned char *)"     巍康卡",FONT);
				else
					Os__GB2312_xprint((unsigned char *)"     会员卡",FONT);
				Os__xlinefeed(1*LINENUM);
				if(i==0)
					Os__GB2312_xprint((unsigned char *)"主管联",FONT);
				else
					Os__GB2312_xprint((unsigned char *)"顾客联",FONT);
			}
			else
			{
				if(i==0)
					Os__GB2312_xprint((unsigned char *)"********持卡人联********",FONT);
				else
					Os__GB2312_xprint((unsigned char *)"********商户联********",FONT);
			}
		}
		PRT_ConstantParam();
		//Os__xlinefeed(2*LINENUM);
		if(NormalTransData.aucFunFlag[18])
			Os__GB2312_xprint((unsigned char *)"(测试交易)",FONT);	
		if(RUNDATA_ucReprintFlag==1)
		{
			if(PRINTTIMES==2)
			{
				if(i==0)
					Os__GB2312_xprint((unsigned char *)"*****重打印*****",FONT);
				else
					Os__GB2312_xprint((unsigned char *)"=====重打印=====",FONT);
			}
			else
				Os__GB2312_xprint((unsigned char *)"=====重打印=====",FONT);
		}	
        PRT_TransType();

        PRT_ChangeParam();
        //Os__xlinefeed(1*LINENUM);
		
		if(DataSave0.ConstantParamData.prnflag)
			Os__GB2312_xprint((unsigned char *)"---------------------",FONT); 
		else
			Os__xprint((unsigned char *)"---------------------");
			
		if(NORMALTRANS_ucTransType==TRANS_S_REFAPL||NORMALTRANS_ucTransType==TRANS_S_VOIDREFAPL)
		{
			memset(aucBuf,0,sizeof(aucBuf));
			memcpy(aucBuf,"顾客电话:",9);
			memcpy(&aucBuf[9],NormalTransData.aucBarCode,15);
			Os__GB2312_xprint(aucBuf);
		}
		PRT_PrintBalance();
		if(NormalTransData.ucReturnflag==true)
			Os__GB2312_xprint((uchar *)"此卡应回收",FONT);
		//Os__xlinefeed(1*LINENUM);
			
		if(NormalTransData.ucTransType==TRANS_S_CHANGEEXP)
		{
			PRT_2ExpDateTime();
		}
		PRT_PrintADInfo();
		if(NormalTransData.ucTransType==TRANS_S_PURCHASE_P
		||NormalTransData.ucTransType==TRANS_S_RETURNCARD_P)
		{
			Os__GB2312_xprint((unsigned char *)"汇点卡卡号:",FONT); 
			memset(aucBuf,0,sizeof(aucBuf));
			bcd_asc(aucBuf,NormalTransData.aucOldCardNo,16);
			Os__xprint(aucBuf); 
		}
			PRT_PrintTAC();
		if(NormalTransData.aucFunFlag[3]&&(
		NormalTransData.ucTransType==TRANS_S_PURCHASE
		||NormalTransData.ucTransType==TRANS_S_RETURNCARD
		||NormalTransData.ucTransType==TRANS_S_PURCHASE_P
		||NormalTransData.ucTransType==TRANS_S_RETURNCARD_P
		||NormalTransData.ucTransType==TRANS_S_PRELOAD
		||NormalTransData.ucTransType==TRANS_S_LOADONLINE
		||NormalTransData.ucTransType==TRANS_S_PAY
		||NormalTransData.ucTransType==TRANS_S_PAYNOBAR
		||NormalTransData.ucTransType==TRANS_S_MOBILECHARGE
		||NormalTransData.ucTransType==TRANS_S_PTCPURCHASE
		||NormalTransData.ucTransType==TRANS_S_TRANSPURCHASE
		||NormalTransData.ucTransType==TRANS_S_REFUND
		||((NormalTransData.ucTransType==TRANS_S_ONLINEPURCHASE||NormalTransData.ucTransType==TRANS_S_VOID)&&(memcmp(&NormalTransData.aucExpiredDate[1],"\x00\x00\x00",3))))
		)
		{
			PRT_ExpDateTime();
		}
		memset(aucBuf,0,sizeof(aucBuf));
		memcpy(aucBuf,DataSave0.ConstantParamData.aucTerminalID,8);
		aucBuf[8]=' ';
		memcpy(&aucBuf[9],DataSave0.ConstantParamData.aucMerchantID,15);
		Os__xprint(aucBuf);
		if(NormalTransData.ucTransType==TRANS_S_PAYONLINE||NormalTransData.ucTransType==TRANS_S_PAY)
		{
			Os__xlinefeed(1*LINENUM);
			PrintBar(NormalTransData.ulTraceNumber);
		}
		if(DataSave0.ConstantParamData.prnflag)
			Os__xlinefeed(5*LINENUM);
		else
		{
			Os__xlinefeed(5*LINENUM);
			Os__xlinefeed(5*LINENUM);
			Os__xlinefeed(5*LINENUM);
		}
		if(PRINTTIMES==2)
		{
			if(i==0)
			{
				Os__clr_display(255);
				Os__GB2312_display(1,0,(unsigned char *)"请撕下凭条");
				MSG_WaitKey(1);
				Os__clr_display(255);
				Os__GB2312_display(0,0,((uchar *)"正在打印"));
				Os__light_off();
			}
		}
	}
    //Os__clr_display(255);
    RunData.ucPtcode=uctPt;
    Uart_Printf("******************PRT_PrintTicket************************\n");
} 

void PRT_ConstantParam(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned char aucBuf[PARAM_MERCHANTNAMELEN+1];
	unsigned short uioffset;
	unsigned char aucBuf1[25];
	
	switch(NORMALTRANS_ucTransType)
	{
		case TRANS_S_ONLINEPURCHASE:
		case TRANS_S_ONLINEPURCHASE_P:
		case TRANS_S_PAYONLINE:
		case TRANS_S_VOIDPAYONLINE:
		case TRANS_S_VOID:
		case TRANS_S_VOID_P:
		case TRANS_S_PREAUTH:
		case TRANS_S_PREAUTHFINISH:
		case TRANS_S_PREAUTHFINISHVOID:
		case TRANS_S_PREAUTHVOID:
		case TRANS_S_MOBILECHARGEONLINE:
		case TRANS_S_SALEONLINE:
		case TRANS_S_BATCHLOAD:
		case TRANS_S_ONLINEREFUND:
					
			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memcpy(aucPrintBuf,"欢迎使用",8);
			memcpy(&aucPrintBuf[8],NormalTransData.aucZoneName1,ZONE_NAME_LEN);
			Os__GB2312_xprint((unsigned char *)aucPrintBuf,FONT);
			break;
		case TRANS_S_PURCHASE:
		case TRANS_S_PAY:
		case TRANS_S_PAYNOBAR:
		case TRANS_S_RETURNCARD:
		case TRANS_S_RETURNCARD_P:
		case TRANS_S_PRELOAD:
		case TRANS_S_SALE:
		case TRANS_S_REFUND:
		case TRANS_S_MOBILECHARGE:
		case TRANS_S_CASHLOAD:
			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memcpy(aucPrintBuf,"欢迎使用",8);
			memcpy(&aucPrintBuf[8],NormalTransData.aucZoneName,ZONE_NAME_LEN);
			Os__GB2312_xprint((unsigned char *)aucPrintBuf,FONT);
			break;
		case TRANS_S_ZSHPURCHASE:
		case TRANS_S_ZSHONLINEPURCHASE:
		case TRANS_S_JSZSHONLINEPURCHASE:
			Os__GB2312_xprint((unsigned char *)"中石化加油卡充值打印单",FONT);
			break;
		default:
			if(RunData.ucPtcode==1)
				Os__GB2312_xprint((unsigned char *)"欢迎使用杉德巍康卡",FONT);
			else if(RunData.ucPtcode==2)
				Os__GB2312_xprint((unsigned char *)"欢迎使用斯玛特卡",FONT);
			else if(RunData.ucPtcode==3)
				Os__GB2312_xprint((unsigned char *)"欢迎使用会员卡",FONT);
			else	
				Os__GB2312_xprint((unsigned char *)"欢迎使用杉德支付卡",FONT);
			break;
	}

         memset(aucPrintBuf,0,sizeof(aucPrintBuf));
       memset(aucBuf1,0,sizeof(aucBuf1));
       CFG_GetVersionString(1,aucPrintBuf);
	memcpy(aucBuf1,aucPrintBuf,24);
	Os__xprint((unsigned char *)aucBuf1);
	if(aucPrintBuf[24] != 0)
	    Os__xprint((unsigned char *)&aucPrintBuf[24]); 
		
	//Os__xlinefeed(1*LINENUM);
	
	/*print Merchant Name*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memset(aucBuf,0,sizeof(aucBuf));
	memcpy(aucBuf,DataSave0.ConstantParamData.aucMerchantName,PARAM_MERCHANTNAMELEN);
	uioffset=strlen((char *)aucBuf);
	aucPrintBuf[30] = '\0';
	memcpy(aucPrintBuf,(unsigned char *)"商户名:",7);
	memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMerchantName,16);
	Os__GB2312_xprint(aucPrintBuf,FONT); 
	
	if(uioffset>16)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30] = '\0';
		memcpy(&aucPrintBuf[0],&DataSave0.ConstantParamData.aucMerchantName[16],24);
	
		Os__GB2312_xprint(aucPrintBuf,FONT); 
	}
	/*print terminal id*/
	if(NormalTransData.ucTransType==TRANS_S_INIONLINE)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30] = '\0';
		memcpy(aucPrintBuf,(unsigned char *)"终端号:",7);
		memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucTerminalID,8);
		Os__GB2312_xprint(aucPrintBuf,FONT); 
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30] = '\0';
		memcpy(aucPrintBuf,(unsigned char *)"商户号:",7);
		memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMerchantID,15);
		Os__GB2312_xprint(aucPrintBuf,FONT); 

		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30] = '\0';
		memcpy(aucPrintBuf,(unsigned char *)"巍康终端号:",11);
		bcd_asc(&aucPrintBuf[11],DataSave0.ConstantParamData.aucTid[0],8);
		Os__GB2312_xprint(aucPrintBuf,FONT); 
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30] = '\0';
		memcpy(aucPrintBuf,(unsigned char *)"商户号:",7);
		bcd_asc(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMid[0],16);
		aucPrintBuf[22]=' ';
		Os__GB2312_xprint(aucPrintBuf,FONT); 
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30] = '\0';
		memcpy(aucPrintBuf,(unsigned char *)"斯玛特终端号:",13);
		bcd_asc(&aucPrintBuf[13],DataSave0.ConstantParamData.aucTid[1],8);
		Os__GB2312_xprint(aucPrintBuf,FONT); 
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30] = '\0';
		memcpy(aucPrintBuf,(unsigned char *)"商户号:",7);
		bcd_asc(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMid[1],16);
		aucPrintBuf[22]=' ';
		Os__GB2312_xprint(aucPrintBuf,FONT); 
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30] = '\0';
		memcpy(aucPrintBuf,(unsigned char *)"会员卡终端号:",13);
		bcd_asc(&aucPrintBuf[13],DataSave0.ConstantParamData.aucTid[2],8);
		Os__GB2312_xprint(aucPrintBuf,FONT); 
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30] = '\0';
		memcpy(aucPrintBuf,(unsigned char *)"商户号:",7);
		bcd_asc(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMid[2],16);
		aucPrintBuf[22]=' ';
		Os__GB2312_xprint(aucPrintBuf,FONT); 
	}
	else
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30] = '\0';
		memcpy(aucPrintBuf,(unsigned char *)"终端号:",7);
		if(RunData.ucPtcode==0)
			memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucTerminalID,8);
		else
			bcd_asc(&aucPrintBuf[7],DataSave0.ConstantParamData.aucTid[RunData.ucPtcode-1],8);

		Os__GB2312_xprint(aucPrintBuf,FONT); 

		if(NormalTransData.ucPtCode==0x01&&NormalTransData.ucNewOrOld==0&&(NormalTransData.aucUserCardNo[0]>>4)==0x09)
		{
			/*print psam terminal */
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30] = '\0';
			memcpy(aucPrintBuf,(unsigned char *)"PSAM:",5);
			hex_asc(&aucPrintBuf[5],&NormalTransData.aucPsamTerminal[2],8);
			Os__GB2312_xprint(aucPrintBuf,FONT); 
		}
			
		/*print Merchant id*/
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30] = '\0';
		memcpy(aucPrintBuf,(unsigned char *)"商户号:",7);
		if(RunData.ucPtcode==0)
			memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMerchantID,15);
		else
		{
			bcd_asc(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMid[RunData.ucPtcode-1],16);
			aucPrintBuf[22]=' ';
		}
		Os__GB2312_xprint(aucPrintBuf,FONT); 
	}
	if(NormalTransData.ucTransType==TRANS_S_ZSHPURCHASE
		||NormalTransData.ucTransType==TRANS_S_JSZSHONLINEPURCHASE
		||NormalTransData.ucTransType==TRANS_S_ZSHONLINEPURCHASE)
	{
		PRT_DateTime();
	
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"系统流水号:",11); 
		long_asc(&aucPrintBuf[11],6,&NormalTransData.ulTicketNumber);
		Os__GB2312_xprint(aucPrintBuf,FONT);
		Os__GB2312_xprint((unsigned char *)"中石化加油卡号:",FONT);	
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		bcd_asc(&aucBuf[0],NormalTransData.aucBarCode,20);
		memcpy(aucPrintBuf,&aucBuf[1],19); 
		Os__GB2312_xprint(aucPrintBuf,FONT);

		Os__GB2312_xprint((unsigned char *)"扣款卡号            金额",FONT);
		if(NormalTransData.ucPtCode==1)
		{
			if(memcmp(NormalTransData.aucUserCardNo,"\x96\x82",2))
				Os__GB2312_xprint((unsigned char *)"巍康卡",FONT);
			else
				Os__GB2312_xprint((unsigned char *)"大众卡",FONT);
		}
		if(NormalTransData.ucPtCode==2)
			Os__GB2312_xprint((unsigned char *)"斯玛特卡",FONT);
		if(NormalTransData.ucPtCode==3)
			Os__GB2312_xprint((unsigned char *)"会员卡",FONT);
		RunData.ucBcksp=1;
	}
}


void PRT_TransType(void)
{
	switch(NORMALTRANS_ucTransType)
	{
	case TRANS_S_PURCHASE:
		Os__GB2312_xprint((unsigned char*)"消费",FONT);
		break;
	case TRANS_S_RETURNCARD:
		Os__GB2312_xprint((unsigned char*)"回收消费",FONT);
		break;
	case TRANS_S_PURCHASE_P:
		Os__GB2312_xprint((unsigned char*)"汇点卡脱机消费",FONT);
		break;
	case TRANS_S_RETURNCARD_P:
		Os__GB2312_xprint((unsigned char*)"汇点卡回收消费",FONT);
		break;
	case TRANS_S_PREAUTH:
		Os__GB2312_xprint((unsigned char*)"预授权",FONT);
		break;
	case TRANS_S_PREAUTHFINISH:
		Os__GB2312_xprint((unsigned char*)"预授权完成",FONT);
		break;
	case TRANS_S_PREAUTHVOID:
		Os__GB2312_xprint((unsigned char*)"预授权撤销",FONT);
		break;
	case TRANS_S_PREAUTHFINISHVOID:
		Os__GB2312_xprint((unsigned char*)"预授权完成撤销",FONT);
		break;
	case TRANS_S_ONLINEPURCHASE:
		if(NormalTransData.ucHandFlag)
		{
			if(NormalTransData.ucReturnflag==true)
				Os__GB2312_xprint((unsigned char*)"联机回收消费(手工)",FONT);
			else
				Os__GB2312_xprint((unsigned char*)"联机消费(手工)",FONT);
			
		}else
		{
			if(NormalTransData.ucReturnflag==true)
				Os__GB2312_xprint((unsigned char*)"联机回收消费",FONT);
			else
				Os__GB2312_xprint((unsigned char*)"联机消费",FONT);
		}
		break;
	case TRANS_S_ONLINEPURCHASE_P:
		Os__GB2312_xprint((unsigned char*)"积点消费",FONT);
		break;
	case TRANS_S_VOID:
		if(NormalTransData.ucHandFlag)
			Os__GB2312_xprint((unsigned char*)"联机撤销(手工)",FONT);
		else
			Os__GB2312_xprint((unsigned char*)"联机撤销",FONT);
		break;
	case TRANS_S_VOID_P:
		Os__GB2312_xprint((unsigned char*)"积点消费撤销",FONT);
		break;
	case TRANS_S_EDLOAD:
//	case TRANS_MEM_EDLOAD:
		Os__GB2312_xprint((unsigned char*)"回收充值",FONT);
		break;
	case TRANS_S_RESETCARD:
		Os__GB2312_xprint((unsigned char*)"清余额",FONT);
		break;
	case TRANS_S_PRELOAD: 
		Os__GB2312_xprint((unsigned char*)"预充值",FONT);
		break;
	case TRANS_S_CHANGECD: 
		Os__GB2312_xprint((unsigned char*)"换卡",FONT);
		break;	
	case TRANS_S_SETTLE_ONE:
	case TRANS_S_SETTLE_TWO:
		Os__GB2312_xprint((unsigned char*)"结算",FONT);
		break;
	case TRANS_S_TERMINPRT:
		Os__GB2312_xprint((unsigned char*)"终端统计打印",FONT);
		break;
	case TRANS_S_CHANGEEXP: 
		Os__GB2312_xprint((unsigned char*)"续期",FONT);
		break;
	case TRANS_S_PAY:
		Os__GB2312_xprint((unsigned char*)"缴费",FONT);
		break;
	case TRANS_S_PAYNOBAR:
		Os__GB2312_xprint((unsigned char*)"缴费",FONT);
		break;
	case TRANS_S_SALE:
		Os__GB2312_xprint((unsigned char*)"卡激活",FONT);
		break;
	case TRANS_S_REFUND:
		Os__GB2312_xprint((unsigned char*)"退货",FONT);
		break;
	case TRANS_S_PAYONLINE:
		Os__GB2312_xprint((unsigned char*)"联机缴费",FONT);
		break;
	case TRANS_S_VOIDPAYONLINE:
		Os__GB2312_xprint((unsigned char*)"联机缴费撤销",FONT);
		break;
	case TRANS_S_MOBILECHARGE:
		Os__GB2312_xprint((unsigned char*)"脱机扣款",FONT);
		break;
	case TRANS_S_MOBILECHARGEONLINE:
		Os__GB2312_xprint((unsigned char*)"联机扣款",FONT);
		break;
	case TRANS_S_LOADONLINE: 
		Os__GB2312_xprint((unsigned char*)"联机充值",FONT);
		break;
	case TRANS_S_TRANSPURCHASE:
		Os__GB2312_xprint((unsigned char*)"移资扣款",FONT);
		break;
	case TRANS_S_TRANSFER: 
		Os__GB2312_xprint((unsigned char*)"移资充值",FONT);
		break;
	case TRANS_S_EXPPURCHASE: 
		Os__GB2312_xprint((unsigned char*)"过期卡消费",FONT);
		break;
	case TRANS_S_REFAPL:
		Os__GB2312_xprint((unsigned char*)"退货申请",FONT);
		break;
	case TRANS_S_VOIDREFAPL:
		Os__GB2312_xprint((unsigned char*)"撤销退货申请",FONT);
		break;
	case TRANS_S_CASHLOAD: 
		Os__GB2312_xprint((unsigned char*)"现金充值",FONT);
		break;
	case TRANS_S_ONLINEREFUND:
		Os__GB2312_xprint((unsigned char*)"联机退货",FONT);
		break;
	case TRANS_S_BATCHLOAD: 
		Os__GB2312_xprint((unsigned char*)"批量充值",FONT);
		break;
	case TRANS_S_SALEONLINE:
		Os__GB2312_xprint((unsigned char*)"批量激活",FONT);
		break;
	case TRANS_S_PTCPURCHASE:
		Os__GB2312_xprint((unsigned char*)"公交卡充值扣款",FONT);
		break;
	default:
		break;
	}
}



void PRT_ChangeParam(void)
{
	unsigned char 	aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned long	ulAmount;
	unsigned char Date[8];
	int i;

	if(NORMALTRANS_ucTransType!=TRANS_S_REFAPL&&NORMALTRANS_ucTransType!=TRANS_S_VOIDREFAPL)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		if(NORMALTRANS_ucTransType==TRANS_S_CHANGECD)
			memcpy(aucPrintBuf,(unsigned char *)"新卡:",5);
		else
			memcpy(aucPrintBuf,(unsigned char *)"卡号:",5);
		bcd_asc(&aucPrintBuf[5],NormalTransData.aucUserCardNo,16);
		if(DataSave0.ChangeParamData.aucTermFlag[8]||NormalTransData.aucFunFlag[20])
			memcpy(&aucPrintBuf[5+6],"******",6);
		aucPrintBuf[24]=0x00;
		Os__GB2312_xprint(aucPrintBuf,FONT); 
	}
	else
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,(uchar *)"操作员:",7);
		memcpy(&aucPrintBuf[7],NormalTransData.aucCashierNo,6);
		Os__GB2312_xprint(aucPrintBuf,FONT); 
	}
	if(NORMALTRANS_ucTransType==TRANS_S_CHANGECD)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"旧卡:",5);
		bcd_asc(&aucPrintBuf[5],NormalTransData.aucOldCardNo,16);
		aucPrintBuf[24]=0x00;
		Os__GB2312_xprint(aucPrintBuf,FONT); 
	}
	
	if(NORMALTRANS_ucTransType==TRANS_S_CHANGEEXP
		||NORMALTRANS_ucTransType==TRANS_S_LOADONLINE
		||NORMALTRANS_ucTransType==TRANS_S_SALE
		||NORMALTRANS_ucTransType==TRANS_S_SALEONLINE)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,(uchar *)"操作员:",7);
		memcpy(&aucPrintBuf[7],NormalTransData.aucCashierNo,6);
		Os__GB2312_xprint(aucPrintBuf,FONT); 
	}



	PRT_DateTime();
	//Os__xlinefeed(0.5*LINENUM);
	
	Os__GB2312_xprint((unsigned char *)"批次号:",FONT);
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30]='\0';
	bcd_asc(&aucPrintBuf[0],DataSave0.ChangeParamData.aucBatchNumber,24);
	Os__xprint((unsigned char*)aucPrintBuf);
		
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30]='\0';
	memcpy(&aucPrintBuf[0],(unsigned char *)"流水号:",7);
	long_asc(&aucPrintBuf[7],6,&NORMALTRANS_ulTraceNumber);
	Os__GB2312_xprint(aucPrintBuf,FONT); 
	//Os__xlinefeed(1*LINENUM);

	if(NormalTransData.ucTransType==TRANS_S_PREAUTH
	||NormalTransData.ucTransType==TRANS_S_PREAUTHVOID
	||NormalTransData.ucTransType==TRANS_S_PREAUTHFINISH
	||NormalTransData.ucTransType==TRANS_S_PREAUTHFINISHVOID)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(&aucPrintBuf[0],(unsigned char *)"授权码:",7);
		memcpy(&aucPrintBuf[7],NormalTransData.aucAuthCode,14);
		Os__GB2312_xprint(aucPrintBuf,FONT); 
		//Os__xlinefeed(1*LINENUM);
	}
	if(NormalTransData.ucTransType==TRANS_S_VOID
		||NormalTransData.ucTransType==TRANS_S_VOID_P
		||NormalTransData.ucTransType==TRANS_S_PREAUTHFINISHVOID
		||NormalTransData.ucTransType==TRANS_S_VOIDPAYONLINE
		||NormalTransData.ucTransType==TRANS_S_VOIDREFAPL
		)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"原流水号:",9);
		memcpy(&aucPrintBuf[9],NormalTransData.aucOldTrace,6);
		Os__GB2312_xprint(aucPrintBuf,FONT);
	}
	if(DataSave0.ConstantParamData.ucYTJFlag)
	{
		if(NormalTransData.ucYhflag)
		{
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			if(NormalTransData.ucYhflag=='1')
				memcpy(&aucPrintBuf[0],(unsigned char *)"合同号:",7);
			else if(NormalTransData.ucYhflag=='2')
				memcpy(&aucPrintBuf[0],(unsigned char *)"订单号:",7);
			else if(NormalTransData.ucYhflag=='3')
				memcpy(&aucPrintBuf[0],(unsigned char *)"手机号:",7);
			else if(NormalTransData.ucYhflag=='4')
				memcpy(&aucPrintBuf[0],(unsigned char *)"帐单号:",7);
			memcpy(&aucPrintBuf[7],NormalTransData.aucBarCode,16);
			Os__GB2312_xprint(aucPrintBuf,FONT); 
		}
		else
		{
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			memcpy(&aucPrintBuf[0],(unsigned char *)"收银流水号:",11);
			memcpy(&aucPrintBuf[11],NormalTransData.aucCashTicket,6);
			Os__GB2312_xprint(aucPrintBuf,FONT); 
		}
	}

	if(NormalTransData.ucTransType==TRANS_S_RETURNCARD||NormalTransData.ucTransType==TRANS_S_RETURNCARD_P)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"实扣余额:",9);
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulPrevEDBalance,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"实扣押金:",9);
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulAmount-NormalTransData.ulPrevEDBalance,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
	}

	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30]='\0';
	switch(NORMALTRANS_ucTransType)
	{
	case TRANS_S_PURCHASE:
	case TRANS_S_EXPPURCHASE:
	case TRANS_S_PURCHASE_P:
		memcpy(aucPrintBuf,(unsigned char *)"消费金额:",9);
		break;
	case TRANS_S_RETURNCARD:
	case TRANS_S_RETURNCARD_P:
		memcpy(aucPrintBuf,(unsigned char *)"总扣金额:",9);
		break;
	case TRANS_S_PREAUTH:
		memcpy(aucPrintBuf,(unsigned char *)"授权金额:",9);
		break;
	case TRANS_S_PREAUTHFINISH:
		memcpy(aucPrintBuf,(unsigned char *)"消费金额:",9);
		break;
	case TRANS_S_PREAUTHVOID:
		memcpy(aucPrintBuf,(unsigned char *)"撤销金额:",9);
		break;
	case TRANS_S_PREAUTHFINISHVOID:
		memcpy(aucPrintBuf,(unsigned char *)"撤销金额:",9);
		break;
	case TRANS_S_ONLINEPURCHASE:
	case TRANS_S_ONLINEPURCHASE_P:
		memcpy(aucPrintBuf,(unsigned char *)"消费金额:",9);
		break;
	case TRANS_S_VOID:
	case TRANS_S_VOID_P:
		memcpy(aucPrintBuf,(unsigned char *)"撤销金额:",9);
		break;
	case TRANS_S_EDLOAD:
		memcpy(aucPrintBuf,(unsigned char *)"充值金额:",9);
		break;
	case TRANS_S_PRELOAD: 
	case TRANS_S_LOADONLINE: 
	case TRANS_S_CASHLOAD:
		memcpy(aucPrintBuf,(unsigned char *)"充值金额:",9);
		break;
	case TRANS_S_PAY:
	case TRANS_S_PAYNOBAR:
		memcpy(aucPrintBuf,(unsigned char *)"缴费金额:",9);
		break;
	case TRANS_S_PAYONLINE:
		memcpy(aucPrintBuf,(unsigned char *)"缴费金额:",9);
		break;
	case TRANS_S_VOIDPAYONLINE:
		memcpy(aucPrintBuf,(unsigned char *)"撤销金额:",9);
		break;
	case TRANS_S_MOBILECHARGE:
		memcpy(aucPrintBuf,(unsigned char *)"消费金额:",9);
		break;
	case TRANS_S_MOBILECHARGEONLINE:
		memcpy(aucPrintBuf,(unsigned char *)"消费金额:",9);
		break;
	case TRANS_S_REFUND:
	case TRANS_S_ONLINEREFUND:
		memcpy(aucPrintBuf,(unsigned char *)"退货金额:",9);
		break;
	case TRANS_S_CHANGECD:
		memcpy(aucPrintBuf,(unsigned char *)"转移金额:",9);
		break;
	case TRANS_S_CHANGEEXP:
		if(NormalTransData.ucPackFlag=='1')
			memcpy(aucPrintBuf,(unsigned char *)"扣手续费:",9);
		else
			memcpy(aucPrintBuf,(unsigned char *)"收手续费:",9);
		break;
	case TRANS_S_REFAPL:
		memcpy(aucPrintBuf,(unsigned char *)"申请金额:",9);
		break;
	case TRANS_S_VOIDREFAPL:
		memcpy(aucPrintBuf,(unsigned char *)"撤销金额:",9);
		break;
	case TRANS_S_TRANSPURCHASE:
		memcpy(aucPrintBuf,(unsigned char *)"转出金额:",9);
		break;
	case TRANS_S_TRANSFER:
		memcpy(aucPrintBuf,(unsigned char *)"转入金额:",9);
		break;
	default:
		memcpy(aucPrintBuf,(unsigned char *)"交易金额:",9);
		break;
	}
	if(TRANS_S_REFUND==NormalTransData.ucTransType
		&&!NormalTransData.aucFunFlag[1]&&NormalTransData.ulPrevEDBalance==0)
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulAmount+NormalTransData.ulYaAmount,2);
	else
	{
		if(NormalTransData.ucBonusflag)
			UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulAmount+NormalTransData.ulRateAmount,2);
		else
			UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulAmount-NormalTransData.ulRateAmount,2);
	}
	if(NormalTransData.ucTransType==TRANS_S_VOID
		||NormalTransData.ucTransType==TRANS_S_PREAUTHFINISHVOID
		||NormalTransData.ucTransType==TRANS_S_PREAUTHVOID
		||NormalTransData.ucTransType==TRANS_S_VOIDREFAPL
		||NormalTransData.ucTransType==TRANS_S_VOID_P
		)
	{
		for(i=0;i<10;i++)
		{
			if(aucPrintBuf[9+i]!=' ')
			{
				aucPrintBuf[8+i]='-';
				break;
			}
		}
	}
//Uart_Printf("\n .1. 2012-03-30 PRT_ChangeParam.............");
	Os__GB2312_xprint(aucPrintBuf,FONT);
	if(NormalTransData.ulRateAmount&&NormalTransData.ucTransType!=TRANS_S_CHANGEEXP)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		if(NormalTransData.ucBonusflag)
			memcpy(aucPrintBuf,(unsigned char *)"返利:       ",9);
		else
			memcpy(aucPrintBuf,(unsigned char *)"服务费:     ",9);
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulRateAmount,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		if(NormalTransData.ucBonusflag)
			memcpy(aucPrintBuf,(unsigned char *)"实扣金额:",9);
		else
			memcpy(aucPrintBuf,(unsigned char *)"总扣金额:",9);
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulAmount,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);

	}
//Uart_Printf("\n .2. 2012-03-30 PRT_ChangeParam.............");	
	if(NormalTransData.ucTransType==TRANS_S_RETURNCARD||NormalTransData.ucTransType==TRANS_S_PURCHASE)
	{
		if(memcmp(NormalTransData.aucDiscount,"\x00\x00",2))
		{
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			memcpy(aucPrintBuf,(unsigned char *)"入账金额:",9);
			ulAmount=NormalTransData.ulAmount-NormalTransData.ulAmount*bcd_long(NormalTransData.aucDiscount,4)/1000;
			UTIL_Form_Montant(&aucPrintBuf[9],ulAmount,2);
			Os__GB2312_xprint(aucPrintBuf,FONT);
		}
	}
//Uart_Printf("\n .3. 2012-03-30 PRT_ChangeParam.............");
	if(NormalTransData.ucTransType==TRANS_S_MOBILECHARGE
	||NormalTransData.ucTransType==TRANS_S_MOBILECHARGEONLINE)
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"手机号:",7);
		memcpy(&aucPrintBuf[7],NormalTransData.aucBarCode,11);
		Os__GB2312_xprint(aucPrintBuf);
	}
	if(NORMALTRANS_ucTransType==TRANS_S_PURCHASE_P
		||NORMALTRANS_ucTransType==TRANS_S_RETURNCARD_P)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"还需扣款:",9);
		UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulNeedCash,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
	}
//Uart_Printf("\n .4. 2012-03-30 PRT_ChangeParam.............");

}


void PRT_DateTime(void)
{
	unsigned char Date[8],Time[8],display[20];
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	
	
	/*print data time*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memset(Date,0,sizeof(Date));
	bcd_asc(Date,&NormalTransData.aucDateTime[1],2);
	memset(&Date[2],'/',1);
	bcd_asc(&Date[3],&NormalTransData.aucDateTime[2],2);
	memset(&Date[5],'/',1);
	bcd_asc(&Date[6],&NormalTransData.aucDateTime[3],2);
	memset(Time,0,sizeof(Time));
	bcd_asc(Time,&NormalTransData.aucDateTime[4],2);
	memset(&Time[2],':',1);
	bcd_asc(&Time[3],&NormalTransData.aucDateTime[5],2);
	memset(&Time[5],':',1);
	bcd_asc(&Time[6],&NormalTransData.aucDateTime[6],2);
	memset(display,' ',sizeof(display));
	memcpy(display,Date,8);
	memcpy(&display[11],Time,8);
	memset(&display[20],0,1);
	if(DataSave0.ConstantParamData.prnflagL)
	{
		memcpy(&aucPrintBuf[6],display,20);
		Os__xprint((unsigned char*)aucPrintBuf);	
	}
	else
	{
		if(DataSave0.ConstantParamData.prnflag)
			Os__GB2312_xprint((unsigned char*)display,FONT);
		else
			Os__xprint((unsigned char*)display);	
	}
}

void PRT_PrintBalance(void)
{
	unsigned char 	aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned long	ulAmount;
	int i;

	//Uart_Printf("\n .1. 2012-03-31 PRT_PrintBalance.............");

	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30]='\0';
	if(NORMALTRANS_ucTransType==TRANS_S_ONLINEPURCHASE&&NormalTransData.aucFunFlag[11]
		||NORMALTRANS_ucTransType==TRANS_S_VOID&&NormalTransData.aucFunFlag[11]
	||NORMALTRANS_ucTransType==TRANS_S_PAYONLINE||NORMALTRANS_ucTransType==TRANS_S_MOBILECHARGEONLINE
	||NORMALTRANS_ucTransType==TRANS_S_PREAUTH||NORMALTRANS_ucTransType==TRANS_S_PREAUTHFINISH
	||NORMALTRANS_ucTransType==TRANS_S_PREAUTHVOID||NORMALTRANS_ucTransType==TRANS_S_PREAUTHFINISHVOID
	||NORMALTRANS_ucTransType==TRANS_S_VOIDPAYONLINE||NORMALTRANS_ucTransType==TRANS_S_ONLINEPURCHASE_P
	||NORMALTRANS_ucTransType==TRANS_S_VOID_P||NORMALTRANS_ucTransType==TRANS_S_REFAPL
	||NORMALTRANS_ucTransType==TRANS_S_VOIDREFAPL
	)
		return;
	else
	{
		if(NORMALTRANS_ucTransType==TRANS_S_LOADONLINE&&!NormalTransData.ucCardLoadFlag)
			memcpy(aucPrintBuf,(unsigned char *)"账户余额:",9);
		else
			memcpy(aucPrintBuf,(unsigned char *)"卡内余额:",9);

	}
	//Uart_Printf("\n .2. 2012-03-31 PRT_PrintBalance.............");
	
	switch(NORMALTRANS_ucTransType)
	{
	case TRANS_S_PURCHASE:
	case TRANS_S_PTCPURCHASE:
	case TRANS_S_PURCHASE_P:
	case TRANS_S_PAY:
	case TRANS_S_PAYNOBAR:
	case TRANS_S_RESETCARD:
	case TRANS_S_MOBILECHARGE:
		if(NORMALTRANS_ulPrevEDBalance>NormalTransData.ulAmount)
			ulAmount=NORMALTRANS_ulPrevEDBalance-NormalTransData.ulAmount;
		else
			ulAmount=0;
		break;
	case TRANS_S_CHANGEEXP:
		if(NormalTransData.ucPackFlag=='1')
			ulAmount=NORMALTRANS_ulPrevEDBalance-NormalTransData.ulAmount;
		else
			ulAmount=NORMALTRANS_ulPrevEDBalance;
		break;
	case TRANS_S_EDLOAD:
	//case TRANMEM_EDLOAD:
		ulAmount=NormalTransData.ulAmount;
		break;
	case TRANS_S_PRELOAD:
	case TRANS_S_REFUND:
	case TRANS_S_CHANGECD:
		if(NormalTransData.ucZeroFlag)
		{
			if(NormalTransData.ulTrueBalance+NormalTransData.ulAmount>NormalTransData.ulYaAmount)
				ulAmount=NormalTransData.ulTrueBalance+NormalTransData.ulAmount-NormalTransData.ulYaAmount;
			else
				ulAmount=0;
		}
		else
			ulAmount=NORMALTRANS_ulPrevEDBalance+NormalTransData.ulAmount;
		break;
	case TRANS_S_LOADONLINE:
		if(NormalTransData.ucCardLoadFlag)
		{
			if(NormalTransData.ucZeroFlag)
			{
				if(NormalTransData.ulTrueBalance+NormalTransData.ulAmount>NormalTransData.ulYaAmount)
					ulAmount=NormalTransData.ulTrueBalance+NormalTransData.ulAmount-NormalTransData.ulYaAmount;
				else
					ulAmount=0;
			}else
				ulAmount=NormalTransData.ulTrueBalance+NormalTransData.ulAmount-NormalTransData.ulYaAmount;
		}
		else
			ulAmount=NormalTransData.ulAfterEDBalance;
		break;
	case TRANS_S_CASHLOAD:
		memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,(unsigned char*)"充值前余额:",11);
		ulAmount = NormalTransData.ulPrevEDBalance;
		UTIL_Form_Montant(&aucPrintBuf[11],ulAmount,2);	
		Os__GB2312_xprint(aucPrintBuf,FONT);
		Os__xlinefeed(LINENUM);
		memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,(unsigned char*)"本次充值额:",11);
		UTIL_Form_Montant(&aucPrintBuf[11],NormalTransData.ulAmount,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
		Os__xlinefeed(LINENUM);
		memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,(unsigned char*)"充值后余额:",11);
		UTIL_Form_Montant(&aucPrintBuf[11],NormalTransData.ulAfterEDBalance,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
		Os__xlinefeed(LINENUM);		
		break;
	case TRANS_S_ONLINEPURCHASE:
	case TRANS_S_ONLINEPURCHASE_P:
	case TRANS_S_PAYONLINE:
	case TRANS_S_MOBILECHARGEONLINE:
	case TRANS_S_VOID:
	case TRANS_S_VOID_P:
	case TRANS_S_PREAUTH:
	case TRANS_S_PREAUTHFINISH:
	case TRANS_S_PREAUTHVOID:
	case TRANS_S_PREAUTHFINISHVOID:
	case TRANS_S_ONLINEREFUND:
		if(NormalTransData.ucReturnflag==true)
			ulAmount=0;
		else
			ulAmount=NormalTransData.ulAfterEDBalance;
		break;
	case TRANS_S_RETURNCARD:
	case TRANS_S_RETURNCARD_P:
		ulAmount=0;
		break;
	default:
		return;
	}
	//Uart_Printf("\n .3. 2012-03-31 PRT_PrintBalance.............");

	UTIL_Form_Montant(&aucPrintBuf[9],ulAmount,2);
	if(NormalTransData.ucBalanceFlag==1)
	{
		for(i=0;i<10;i++)
		{
			if(aucPrintBuf[9+i]!=' ')
			{
				aucPrintBuf[8+i]='-';
				break;
			}
		}
	}
	if(NormalTransData.ucTransType!=TRANS_S_CASHLOAD)
		Os__GB2312_xprint(aucPrintBuf,FONT);
	//Uart_Printf("\n .4. 2012-03-31 PRT_PrintBalance.............");
	
}

void PRT_PrintADInfo(void)
{
	unsigned char 	aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned short 	uioffset;
		
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]=0x00;
	memcpy(aucPrintBuf,DataSave0.ChangeParamData.aucPrintInfo,24);
	uioffset=strlen((char *)aucPrintBuf);
	if(uioffset!=0)
	{
		Os__GB2312_xprint(aucPrintBuf,FONT); 
		if(uioffset>24)
		{
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[PRINT_MAXCHAR]=0x00;
			memcpy(aucPrintBuf,&DataSave0.ChangeParamData.aucPrintInfo[24],16);
			Os__GB2312_xprint(aucPrintBuf,FONT); 
		}
	}
}



void PRT_PrintInitData(void)
{
	unsigned char 	aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned char   aucBuf[PARAM_TELNUMBERLEN+1];
		
	PRT_ConstantParam();
	
	if(Commun_flag==0x31)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memset(aucBuf,0,sizeof(aucBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"主拨1:",6);
		memcpy(aucBuf,DataSave0.ConstantParamData.aucHostTelNumber1,PARAM_TELNUMBERLEN);
		memcpy(&aucPrintBuf[6],aucBuf,strlen((char *)aucBuf));
		Os__GB2312_xprint(aucPrintBuf,FONT);
		
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memset(aucBuf,0,sizeof(aucBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"主拨2:",6);
		memcpy(aucBuf,DataSave0.ConstantParamData.aucHostTelNumber1,PARAM_TELNUMBERLEN);
		memcpy(&aucPrintBuf[6],aucBuf,strlen((char *)aucBuf));
		Os__GB2312_xprint(aucPrintBuf,FONT);		
	}else
	{
		
	}
	if(DataSave0.ConstantParamData.prnflag)
		Os__GB2312_xprint((unsigned char *)"---------------------",FONT); 
	else
		Os__xprint((unsigned char *)"---------------------");
	
	if(DataSave0.ConstantParamData.prnflag)
		Os__xlinefeed(5*LINENUM);
	else
	{
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
	}
}

void PRT_Terminal(void)
{
	unsigned short uiIndex,uiPt;
	unsigned char  aucPrintBuf[50],ucFirstFlag,ucFeedFlag;

//	Os__clr_display(255);
//	Os__GB2312_display(0,0,(uchar *)"正在打印");
	
//	if(DataSave0.TransInfoData.TransTotal.uiTotalNb==0)
//	{
//#ifdef GUI_PROJECT
//        memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
//        ProUiFace.ProToUi.uiLines=1;
//        memcpy(ProUiFace.ProToUi.aucLine1,"空交易",12);
//        sleep(1);
//#else
//		Os__clr_display(255);
//		Os__GB2312_display(0,0,(uchar *)"空交易");
//		UTIL_GetKey(3);
//#endif
//		return ;
//	}
	
    //???????????????
//	Os__light_off();
//	if(DataSave0.ConstantParamData.Pinpadflag)
//		Os__light_off_pp();
	ucFeedFlag=0;
    Uart_Printf("*********************uiPt====%d===============\n",uiPt);
	for(uiPt=1;uiPt<=3;uiPt++)
	{
		ucFirstFlag=0;
        for(uiIndex = 0;uiIndex<TRANS_MAXNB;uiIndex++)
		{
            if(DataSave0.TransInfoData.auiTransIndex[uiIndex])
			{
				memset(&NormalTransData,0,sizeof(NORMALTRANS));
				XDATA_Read_SaveTrans_File(uiIndex);
                if(NormalTransData.ucPtCode==uiPt)
				{
                    Uart_Printf("ucFirstFlag~~~~~~~~~~~~~~~~%d\n",ucFirstFlag);
					if(ucFirstFlag==0)
					{
						if(ucFeedFlag==1)
							Os__xlinefeed(2*LINENUM);
						RunData.ucPtcode=uiPt;
						PRT_ConstantParam();
						Os__GB2312_xprint((uchar *)"终端明细打印",FONT);
						
						memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
						aucPrintBuf[30]='\0';
						Os__GB2312_xprint((uchar *)"批次号:",FONT);
						bcd_asc(&aucPrintBuf[0],DataSave0.ChangeParamData.aucBatchNumber,24);
						Os__GB2312_xprint(aucPrintBuf,FONT); 
						PRT_LocalDateTime();
						Os__xlinefeed(0.5*LINENUM);
			
						Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);
						ucFirstFlag=1;
						ucFeedFlag=1;
					}
					PRT_TerminalList();
					Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);
					PRT_PrintTAC();
					Os__xlinefeed(0.5*LINENUM);
				}
			}
		}
	}
	if(DataSave0.ConstantParamData.prnflag)
		Os__xlinefeed(5*LINENUM);
	else
	{
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
	}
    
}
void PRT_RepDateTime(void)
{
	unsigned char aucDate[7],aucTime[7],DateReverse[7];
	unsigned char Date[8],Time[8],display[20];
	
	memset(aucDate,0,sizeof(aucDate));
	memset(aucTime,0,sizeof(aucTime));
	memset(DateReverse,0,sizeof(DateReverse));
	
  memset(Date,0,sizeof(Date));
	bcd_asc(Date,&DataSave0.ChangeParamData.aucSettleDateTime[1],2);
	memset(&Date[2],'/',1);
	bcd_asc(&Date[3],&DataSave0.ChangeParamData.aucSettleDateTime[2],2);
	memset(&Date[5],'/',1);
	bcd_asc(&Date[6],&DataSave0.ChangeParamData.aucSettleDateTime[3],2);
	
	memset(Time,0,sizeof(Time));
	bcd_asc(Time,&DataSave0.ChangeParamData.aucSettleDateTime[4],2);
	memset(&Time[2],':',1);
	bcd_asc(&Time[3],&DataSave0.ChangeParamData.aucSettleDateTime[5],2);
	memset(&Time[5],':',1);
	bcd_asc(&Time[6],&DataSave0.ChangeParamData.aucSettleDateTime[6],2);
	memset(display,' ',sizeof(display));
	memcpy(display,Date,8);
	memcpy(&display[11],Time,8);
	memset(&display[20],0,1);
	
	if(DataSave0.ConstantParamData.prnflag)
		Os__GB2312_xprint((unsigned char*)display,FONT);
	else
		Os__xprint((unsigned char*)display);	
}

void PRT_LocalDateTime(void)
{
	unsigned char aucDate[7],aucTime[7],DateReverse[7];
	unsigned char Date[8],Time[8],display[20];
	
	memset(aucDate,0,sizeof(aucDate));
	memset(aucTime,0,sizeof(aucTime));
	memset(DateReverse,0,sizeof(DateReverse));
	
	Os__read_time_sec(aucTime);
    
    Os__read_date(aucDate);
    
    RunData.aucSettleDateTime[0]=0x20;
    asc_bcd(&RunData.aucSettleDateTime[1],1,&aucDate[4],2);
		asc_bcd(&RunData.aucSettleDateTime[2],1,&aucDate[2],2);
		asc_bcd(&RunData.aucSettleDateTime[3],1,&aucDate[0],2);
		asc_bcd(&RunData.aucSettleDateTime[4],3,aucTime,6);	
    
    memcpy(&DateReverse[0],&aucDate[4],2);
    memcpy(&DateReverse[2],&aucDate[2],2);
    memcpy(&DateReverse[4],&aucDate[0],2);
    
    memset(Date,0,sizeof(Date));
	memcpy(Date,&DateReverse[0],2);
	memset(&Date[2],'/',1);
	memcpy(&Date[3],&DateReverse[2],2);
	memset(&Date[5],'/',1);
	memcpy(&Date[6],&DateReverse[4],2);
	
	memset(Time,0,sizeof(Time));
	memcpy(Time,&aucTime[0],2);
	memset(&Time[2],':',1);
	memcpy(&Time[3],&aucTime[2],2);
	memset(&Time[5],':',1);
	memcpy(&Time[6],&aucTime[4],2);
	memset(display,' ',sizeof(display));
	memcpy(display,Date,8);
	memcpy(&display[11],Time,8);
	memset(&display[20],0,1);
	
	if(DataSave0.ConstantParamData.prnflag)
		Os__GB2312_xprint((unsigned char*)display,FONT);
	else
		Os__xprint((unsigned char*)display);	
}

void PRT_TerminalList(void)
{
	unsigned char 	aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned long	ulAmount;
	unsigned char Date[8];
	
	PRT_TransType();
	if(NormalTransData.ucKYFlag)
		Os__GB2312_xprint((unsigned char *)"可疑交易",FONT); 
	PRT_DateTime();
	
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30]='\0';
	memcpy(aucPrintBuf,(unsigned char *)"卡号:",5);
	bcd_asc(&aucPrintBuf[5],NormalTransData.aucUserCardNo,16);
	aucPrintBuf[24]=0x00;
	Os__GB2312_xprint(aucPrintBuf,FONT); 
//	if(NormalTransData.ucCardType==CARD_CPU)
	//	Os__xprint((unsigned char *)"CPU CARD"); 
	//else
	//	Os__xprint((unsigned char *)"MEMORY CARD");
	
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(&aucPrintBuf[0],(unsigned char *)"流水号:",7);
	long_asc(&aucPrintBuf[7],6,&NORMALTRANS_ulTraceNumber);
	Os__GB2312_xprint(aucPrintBuf,FONT); 
	
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30]='\0';
	memcpy(aucPrintBuf,(unsigned char *)"交易金额:",9);
	UTIL_Form_Montant(&aucPrintBuf[9],NormalTransData.ulAmount,2);
	Os__GB2312_xprint(aucPrintBuf,FONT);
	
	PRT_PrintBalance();

	if(NORMALTRANS_ucTransType==TRANS_S_CHANGEEXP)
	{
	       memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[PRINT_MAXCHAR]='\0';
		bcd_asc(Date,&NormalTransData.aucNewExpiredDate[1],2);
		memset(&Date[2],'/',1);
		bcd_asc(&Date[3],&NormalTransData.aucNewExpiredDate[2],2);
		memset(&Date[5],'/',1);
		bcd_asc(&Date[6],&NormalTransData.aucNewExpiredDate[3],2);

		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"卡新有效期:",11);
		memcpy(&aucPrintBuf[11],Date,8);
		Os__GB2312_xprint(aucPrintBuf,FONT);
	}
}

void PRT_PrintTAC(void)
{
	unsigned char 	aucBuf[150],aucTemp[50];
	unsigned char 	aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned short 	uioffset=0,uiLen=0;
	unsigned long 	ulAmount;
	unsigned short  i,j,uiI;
	if(NormalTransData.ucTransType==TRANS_S_PURCHASE
	||NormalTransData.ucTransType==TRANS_S_PURCHASE_P
	||NormalTransData.ucTransType==TRANS_S_RETURNCARD_P
	||NormalTransData.ucTransType==TRANS_S_PRELOAD
	||(NormalTransData.ucTransType==TRANS_S_LOADONLINE&&NormalTransData.ucCardLoadFlag)
	||NormalTransData.ucTransType==TRANS_S_RETURNCARD
	||NormalTransData.ucTransType==TRANS_S_PAY
	||NormalTransData.ucTransType==TRANS_S_PAYNOBAR
	||NormalTransData.ucTransType==TRANS_S_MOBILECHARGE
	||NormalTransData.ucTransType==TRANS_S_REFUND
	||NormalTransData.ucTransType==TRANS_S_TRANSPURCHASE
	||NormalTransData.ucTransType==TRANS_S_CHANGECD
	||NormalTransData.ucTransType==TRANS_S_EXPPURCHASE
	||NormalTransData.ucTransType==TRANS_S_CASHLOAD
	||NormalTransData.ucTransType==TRANS_S_PTCPURCHASE
	||(NormalTransData.ucTransType==TRANS_S_CHANGEEXP&&NormalTransData.ucPackFlag=='1')
	)
	{
		memset(aucBuf,' ',sizeof(aucBuf));
		if(DataSave0.ConstantParamData.prnflagL)
			uioffset=2;
		else
			uioffset=0;
		if(DataSave0.ChangeParamData.aucTermFlag[8])
			memcpy(&aucBuf[uioffset],(uchar *)"S",1);
		else
			memcpy(&aucBuf[uioffset],(uchar *)"M",1);
		uioffset += 1;
		hex_asc(&aucBuf[uioffset],NormalTransData.aucTraceTac,8);
		uioffset += 8;
		memset(aucTemp,0,sizeof(aucTemp));
		long_tab(aucTemp,4,&NormalTransData.ulTrueBalance);	
		hex_asc(&aucBuf[uioffset],aucTemp,8);
		uioffset += 8;
		memcpy(&aucBuf[uioffset],(uchar *)" ",1);
		uioffset += 1;
		hex_asc(&aucBuf[uioffset],NormalTransData.aucSamTace,4);	
		uioffset += 4;
		aucBuf[uioffset]=0x00;
		
		if(DataSave0.ConstantParamData.prnflagL)
			uioffset=2;
		else
			uioffset=0;
		Os__xprint((unsigned char *)aucBuf);
		memset(aucBuf,' ',sizeof(aucBuf));
		hex_asc(&aucBuf[uioffset],&NormalTransData.aucSamTace[2],4);	
		uioffset += 4;
		memcpy(&aucBuf[uioffset],(uchar *)" ",1);
		uioffset += 1;
		if(NormalTransData.ucCardType==CARD_M1)
		{
			hex_asc(&aucBuf[uioffset],&NormalTransData.aucIssueDate[2],4);
			uioffset += 4;
			hex_asc(&aucBuf[uioffset],NormalTransData.aucCardSerial,4);
			uioffset += 4;
			memcpy(&aucBuf[uioffset],(uchar *)" ",1);
			uioffset += 1;
			hex_asc(&aucBuf[uioffset],&NormalTransData.aucCardSerial[2],4);
			uioffset += 4;
			hex_asc(&aucBuf[uioffset],NormalTransData.aucVerifyCode,2);
			uioffset += 2;
			memcpy(&aucBuf[uioffset],(uchar *)"02",2);
			uioffset += 2;
		}
		else if(NormalTransData.ucCardType==CARD_MEM)
		{
			hex_asc(&aucBuf[uioffset],NormalTransData.aucUserCardNo,8);
			uioffset += 8;
			memcpy(&aucBuf[uioffset],(uchar *)" ",1);
			uioffset += 1;
			hex_asc(&aucBuf[uioffset],&NormalTransData.aucUserCardNo[4],8);
			uioffset += 8;
			
		}
		else if(NormalTransData.ucCardType==CARD_CPU)
		{
			hex_asc(&aucBuf[uioffset],NormalTransData.aucCardSerial,8);
			uioffset += 8;
			memcpy(&aucBuf[uioffset],(uchar *)" ",1);
			uioffset += 1;
			hex_asc(&aucBuf[uioffset],&NormalTransData.aucUserCardNo[4],8);
			uioffset += 8;
			
		}
		aucBuf[uioffset]=0x00;
		Os__xprint((unsigned char *)aucBuf);
		if(DataSave0.ConstantParamData.prnflagL)
			uioffset=2;
		else
			uioffset=0;
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[uioffset]=NormalTransData.ucNewOrOld+1+'0';uioffset += 1;
		aucBuf[uioffset]=NormalTransData.ucCardType;uioffset += 1;
		if(NormalTransData.ucCardType==CARD_CPU)
		{
			if(NormalTransData.ucTransType==TRANS_S_PRELOAD)
			{
				aucBuf[uioffset]='0';uioffset += 1;
				aucBuf[uioffset]='2';uioffset += 1;
			}
			else
			{
				aucBuf[uioffset]='0';uioffset += 1;
				aucBuf[uioffset]='6';uioffset += 1;
			}
		}
		else
		{
			if(NormalTransData.ucTransType==TRANS_S_PRELOAD)
			{
				aucBuf[uioffset]='6';uioffset += 1;
				aucBuf[uioffset]='2';uioffset += 1;
			}
			else
			{
				aucBuf[uioffset]='2';uioffset += 1;
				aucBuf[uioffset]='1';uioffset += 1;
			}
		}
		aucBuf[uioffset]='0';uioffset += 1;
		aucBuf[uioffset]='0';uioffset += 1;
		if(DataSave0.ChangeParamData.aucTermFlag[8]||NormalTransData.aucFunFlag[20])
		{
			uioffset+=1;
			des(NormalTransData.aucUserCardNo,aucTemp,"87697375");
			hex_asc(&aucBuf[uioffset],aucTemp,16);
		}
		Os__xprint((unsigned char *)aucBuf);
	}
	else if(DataSave0.ChangeParamData.aucTermFlag[8]||NormalTransData.aucFunFlag[20])
	{
		memset(aucBuf,' ',sizeof(aucBuf));
		des(NormalTransData.aucUserCardNo,aucTemp,"87697375");
		hex_asc(aucBuf,aucTemp,16);
		Os__xprint((unsigned char *)aucBuf);
	}
}


void PRT_PrintSettle(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	int i,iprinttimes;
    //Os__clr_display(255);
    //Os__GB2312_display(0,0,((uchar *)"正在打印"));
	//为了保护打印头，打印时关闭背光
    //Os__light_off();
    //if(DataSave0.ConstantParamData.Pinpadflag)
    //	Os__light_off_pp();
		
	if(DataSave0.ConstantParamData.prnflagL)
		OSPRN_XReverseFeed(LINENUM*7);
	if(DataSave0.ChangeParamData.aucTermFlag[1]==1
        &&DataSave0.TransInfoData.TransTotal.uiZshPurchaseNb+DataSave0.TransInfoData.TransTotal.uiZshOnlinePurchaseNb==0)
	{
		iprinttimes=3;
		RunData.ucPtcode=1;
	}
	else
	{
		iprinttimes=1;
		RunData.ucPtcode=0;
	}
	for(i=0;i<iprinttimes;i++)
	{		
		if(iprinttimes==3)
		{
			if(DataSave0.ConstantParamData.aucTreat[i]=='0')
			{
				RunData.ucPtcode++;
				continue;
			}
		}
		if(DataSave0.ConstantParamData.prnflagL)
			PRT_ConstantParam_L();
		else
			PRT_ConstantParam();
		
		if(RUNDATA_ucReprintFlag==1)
			Os__GB2312_xprint((unsigned char *)"重打结算单",FONT);

		if(DataSave0.ConstantParamData.prnflagL)
			PRT_TransType_L();
		else
			PRT_TransType();
			
		Os__GB2312_xprint((unsigned char *)"批次号:",FONT);
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		if(RUNDATA_ucReprintFlag==1)
			bcd_asc(&aucPrintBuf[0],DataSave0.TransInfoData.TransTotal.aucBatchNumber,24);
		else
			bcd_asc(&aucPrintBuf[0],DataSave0.ChangeParamData.aucBatchNumber,24);
		Os__GB2312_xprint(aucPrintBuf,FONT); 
		
		Os__xlinefeed(LINENUM*0.5);
		
		if(RUNDATA_ucReprintFlag==1)
			PRT_RepDateTime();
		else
			PRT_LocalDateTime();

		Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);
		if(DataSave0.TransInfoData.TransTotal.uiZshPurchaseNb+DataSave0.TransInfoData.TransTotal.uiZshOnlinePurchaseNb+DataSave0.TransInfoData.TransTotal.uiJsZshOnlinePurchaseNb==0)
			PRT_Total(RunData.ucPtcode);
		else
			PRT_ZSH_Total();
		Os__xlinefeed(LINENUM*0.5);
		//PRT_TraceFirstEnd();
		Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);
		if((!RUNDATA_ucReprintFlag)&&(NORMALTRANS_ucTransType!=TRANS_S_TERMINPRT))
		{
			if(iprinttimes==3)
			{
				if(RUNDATA_ucSettleStatus[i]==SETTLE_UNBALANCE)
					Os__GB2312_xprint((unsigned char *)"结算不平",FONT);
				else
					Os__GB2312_xprint((unsigned char *)"结算成功",FONT);
			}
			if(iprinttimes==1)
			{
				if(RUNDATA_ucSettleStatus[0]==SETTLE_UNBALANCE
					||RUNDATA_ucSettleStatus[1]==SETTLE_UNBALANCE
					||RUNDATA_ucSettleStatus[2]==SETTLE_UNBALANCE)
					Os__GB2312_xprint((unsigned char *)"结算不平",FONT);
				else
					Os__GB2312_xprint((unsigned char *)"结算成功",FONT);
			}
		}

		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,DataSave0.ConstantParamData.aucTerminalID,8);
		aucPrintBuf[8]=' ';
		memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucMerchantID,15);
		Os__xprint(aucPrintBuf);

		if(DataSave0.ConstantParamData.prnflag)
			Os__xlinefeed(5*LINENUM);
		else if(DataSave0.ConstantParamData.prnflagL)
		{
			Os__xlinefeed(5*LINENUM);
			Os__xlinefeed(5*LINENUM);
			Os__xlinefeed(5*LINENUM);
			Os__xlinefeed(5*LINENUM);
		}
		else
		{
			Os__xlinefeed(5*LINENUM);
			Os__xlinefeed(5*LINENUM);
			Os__xlinefeed(5*LINENUM);
		}
		RunData.ucPtcode++;
	}
}

void PRT_IssueTotal(uchar ucPtcode,uchar ucTranstype)
{
	int i=0,j=0;
	unsigned char ucResult=SUCCESS,aucBuf[PRINT_MAXCHAR+1],aucIssName[16],ucIndex[200];
	unsigned long ulReturnNb=0,ulReturnAmount=0,ulYJAmount=0;
	memset(ucIndex,0,sizeof(ucIndex));
	if(DataSave0.ChangeParamData.aucTermFlag[0]==1&&ucTranstype==TRANS_S_PURCHASE)
	{
		memset(aucIssName,0,sizeof(aucIssName));
		while(1)
		{
			memset(&IssueInfo,0,sizeof(ISSUETRANSINFO));
			if(RUNDATA_ucReprintFlag ==1)
				ucResult=XDATA_Read_LastIssueInfo_File(j);
			else
				ucResult=XDATA_Read_IssueInfo_File(j);
			if(ucResult==SUCCESS)
			{
				if(IssueInfo.ucTranstype==0)
					break;
				Uart_Printf("SAV_IssueInfo ucTranstype=%02x,ucPtcode=%02x\n",ucTranstype,ucPtcode);
				Uart_Printf("SAV_IssueInfo ucTranstype-1=%02x,ucPtcode-1=%02x\n",IssueInfo.ucTranstype,IssueInfo.ucPtcode);
				if(IssueInfo.ucTranstype==TRANS_S_RETURNCARD
					&&(IssueInfo.ucPtcode==ucPtcode||ucPtcode==0))
				{
					ulReturnNb=IssueInfo.uiTransNb;
					ulReturnAmount=IssueInfo.ulTransAmount-IssueInfo.ulYJAmount;
					memcpy(aucIssName,IssueInfo.aucIssueName,16);
					while(1)
					{
						memset(&IssueInfo,0,sizeof(ISSUETRANSINFO));
						if(RUNDATA_ucReprintFlag ==1)
							ucResult=XDATA_Read_LastIssueInfo_File(i);
						else
							ucResult=XDATA_Read_IssueInfo_File(i);
                        if(ucResult==SUCCESS)
                        {
                            if(IssueInfo.ucTranstype==0)
                                break;
                            Uart_Printf("SAV_IssueInfo ucTranstype=%02x,ucPtcode=%02x\n",ucTranstype,ucPtcode);
                            Uart_Printf("SAV_IssueInfo ucTranstype0=%02x,ucPtcode0=%02x\n",IssueInfo.ucTranstype,IssueInfo.ucPtcode);
                            if(IssueInfo.ucTranstype==TRANS_S_PURCHASE
                                    &&(!memcmp(IssueInfo.aucIssueName,aucIssName,16))
                                    &&(IssueInfo.ucPtcode==ucPtcode||ucPtcode==0))
                            {
                                ucIndex[i]=1;
                                ulReturnNb+=IssueInfo.uiTransNb;
                                ulReturnAmount+=IssueInfo.ulTransAmount;
                                break;
                            }

						}
						i++;
					}
					memset(aucBuf,' ',sizeof(aucBuf));
					memcpy(&aucBuf[2],aucIssName,16);
					Os__GB2312_xprint(aucBuf,FONT);
					Os__GB2312_xprint((unsigned char *)"  总笔数     总金额",FONT);
					memset(aucBuf,' ',sizeof(aucBuf));
					aucBuf[PRINT_MAXCHAR]='\0';
					int_asc(&aucBuf[2],3,(uint *)&ulReturnNb);
					UTIL_Form_Montant(&aucBuf[8],ulReturnAmount,2);
					Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
				}
			}
			j++;
		}

	}
	i=0;
	while(1)
	{
		if(ucIndex[i])
		{
			i++;continue;
		}
		memset(&IssueInfo,0,sizeof(ISSUETRANSINFO));
		if(RUNDATA_ucReprintFlag ==1)
			ucResult=XDATA_Read_LastIssueInfo_File(i);
		else
			ucResult=XDATA_Read_IssueInfo_File(i);
		Uart_Printf("SAV_IssueInfo i=%d,ucResult=%02x\n",i,ucResult);
		if(ucResult==SUCCESS)
		{
			Uart_Printf("SAV_IssueInfo ucTranstype=%02x,ucPtcode=%02x\n",ucTranstype,ucPtcode);
			Uart_Printf("SAV_IssueInfo ucTranstype1=%02x,ucPtcode1=%02x\n",IssueInfo.ucTranstype,IssueInfo.ucPtcode);
			if(IssueInfo.ucTranstype==0)
				break;
			if(IssueInfo.ucTranstype==ucTranstype
				&&(IssueInfo.ucPtcode==ucPtcode||ucPtcode==0))
			{
				memset(aucBuf,' ',sizeof(aucBuf));
				memcpy(&aucBuf[2],IssueInfo.aucIssueName,16);
				Os__GB2312_xprint(aucBuf,FONT);
				if(DataSave0.ChangeParamData.aucTermFlag[0]==1&&ucTranstype==TRANS_S_RETURNCARD)
					Os__GB2312_xprint((unsigned char *)"  总笔数     总押金额",FONT);
				else
					Os__GB2312_xprint((unsigned char *)"  总笔数     总金额",FONT);
				memset(aucBuf,' ',sizeof(aucBuf));
				aucBuf[PRINT_MAXCHAR]='\0';
				int_asc(&aucBuf[2],3,&IssueInfo.uiTransNb);
				if(DataSave0.ChangeParamData.aucTermFlag[0]==1&&ucTranstype==TRANS_S_RETURNCARD)
					UTIL_Form_Montant(&aucBuf[8],IssueInfo.ulYJAmount,2);
				else
					UTIL_Form_Montant(&aucBuf[8],IssueInfo.ulTransAmount,2);
				Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
				
			}
		}
		else
			break;
		i++;
	}
}

void PRT_Total_bak(void)
{
	unsigned char aucBuf[PRINT_MAXCHAR+1];
	unsigned long ultotalamount;
	unsigned int uitotalnb;	
	
	
	if(RUNDATA_ucReprintFlag ==1)
	{
		uitotalnb=DataSave0.TransInfoData.LastTransTotal.uiPurchaseNb;
		ultotalamount = DataSave0.TransInfoData.LastTransTotal.ulPurchaseAmount;
	}else
	{
		uitotalnb=DataSave0.TransInfoData.TransTotal.uiPurchaseNb;
		ultotalamount = DataSave0.TransInfoData.TransTotal.ulPurchaseAmount;
	}	
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"消费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(0,TRANS_S_PURCHASE);
	}
	
	if(RUNDATA_ucReprintFlag ==1)
	{
		uitotalnb=DataSave0.TransInfoData.LastTransTotal.uiReturnNb;
		ultotalamount = DataSave0.TransInfoData.LastTransTotal.ulReturnAmount;
	}else
	{
		uitotalnb=DataSave0.TransInfoData.TransTotal.uiReturnNb;
		ultotalamount = DataSave0.TransInfoData.TransTotal.ulReturnAmount;
	}	
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"回收消费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(0,TRANS_S_RETURNCARD);
	}
	
	if(RUNDATA_ucReprintFlag ==1)
	{
		uitotalnb=DataSave0.TransInfoData.LastTransTotal.uiOnlinePurchaseNb;
		ultotalamount = DataSave0.TransInfoData.LastTransTotal.ulOnlinePurchaseAmount;
	}else
	{
		uitotalnb=DataSave0.TransInfoData.TransTotal.uiOnlinePurchaseNb;
		ultotalamount = DataSave0.TransInfoData.TransTotal.ulOnlinePurchaseAmount;
	}	
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"联机消费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(0,TRANS_S_ONLINEPURCHASE);
	}
	
	if(RUNDATA_ucReprintFlag ==1)
	{
		uitotalnb=DataSave0.TransInfoData.LastTransTotal.uiLoadNb;  
		ultotalamount = DataSave0.TransInfoData.LastTransTotal.ulLoadAmount;
	}else
	{
		uitotalnb=DataSave0.TransInfoData.TransTotal.uiLoadNb;
		ultotalamount = DataSave0.TransInfoData.TransTotal.ulLoadAmount;
	}	
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"充值",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(0,TRANS_S_PRELOAD);
	}
	if(RUNDATA_ucReprintFlag ==1)
	{
		uitotalnb=DataSave0.TransInfoData.LastTransTotal.uiPayNb;  
		ultotalamount = DataSave0.TransInfoData.LastTransTotal.ulPayAmount;
	}else
	{
		uitotalnb=DataSave0.TransInfoData.TransTotal.uiPayNb;
		ultotalamount = DataSave0.TransInfoData.TransTotal.ulPayAmount;
	}	
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"缴费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(0,TRANS_S_PAY);
	}
	if(RUNDATA_ucReprintFlag ==1)
	{
		uitotalnb=DataSave0.TransInfoData.LastTransTotal.uiOnlinePayNb;  
		ultotalamount = DataSave0.TransInfoData.LastTransTotal.ulOnlinePayAmount;
	}else
	{
		uitotalnb=DataSave0.TransInfoData.TransTotal.uiOnlinePayNb;
		ultotalamount = DataSave0.TransInfoData.TransTotal.ulOnlinePayAmount;
	}	
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"联机缴费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(0,TRANS_S_PAYONLINE);
	}
	if(RUNDATA_ucReprintFlag ==1)
	{
		uitotalnb=DataSave0.TransInfoData.LastTransTotal.uiMobileNb;  
		ultotalamount = DataSave0.TransInfoData.LastTransTotal.ulMobileAmount;
	}else
	{
		uitotalnb=DataSave0.TransInfoData.TransTotal.uiMobileNb;
		ultotalamount = DataSave0.TransInfoData.TransTotal.ulMobileAmount;
	}	
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"脱机扣款",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(0,TRANS_S_MOBILECHARGE);
	}
	if(RUNDATA_ucReprintFlag ==1)
	{
		uitotalnb=DataSave0.TransInfoData.LastTransTotal.uiOnlineMobileNb;  
		ultotalamount = DataSave0.TransInfoData.LastTransTotal.ulOnlineMobileAmount;
	}else
	{
		uitotalnb=DataSave0.TransInfoData.TransTotal.uiOnlineMobileNb;
		ultotalamount = DataSave0.TransInfoData.TransTotal.ulOnlineMobileAmount;
	}	
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"联机扣款",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(0,TRANS_S_MOBILECHARGEONLINE);
	}
	if(RUNDATA_ucReprintFlag ==1)
	{
		uitotalnb=DataSave0.TransInfoData.LastTransTotal.uiSaleNb;
		ultotalamount = DataSave0.TransInfoData.LastTransTotal.ulSaleAmount;
	}else
	{
		uitotalnb=DataSave0.TransInfoData.TransTotal.uiSaleNb;
		ultotalamount = DataSave0.TransInfoData.TransTotal.ulSaleAmount;
	}	
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"售卡",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(0,TRANS_S_SALE);
	}
}


void PRT_Total(uchar ucPtcode)
{
	unsigned char aucBuf[PRINT_MAXCHAR+1];
	unsigned long ultotalamount;
	unsigned int uitotalnb;	
	TRANSTOTAL	*pTransTotal;	
	
	if(RUNDATA_ucReprintFlag ==1)
	{
		if(ucPtcode==0)
			pTransTotal=&DataSave0.TransInfoData.LastTransTotal;
		else
			pTransTotal=&DataSave0.TransInfoData.LastPtInfoData[ucPtcode-1];
	}
	else
	{
		if(ucPtcode==0)
			pTransTotal=&DataSave0.TransInfoData.TransTotal;
		else
			pTransTotal=&DataSave0.TransInfoData.PtInfoData[ucPtcode-1];
	}

	if(DataSave0.ChangeParamData.aucTermFlag[0]==1)
	{
		uitotalnb=pTransTotal->uiPurchaseNb;
		ultotalamount = pTransTotal->ulPurchaseAmount;
		Os__GB2312_xprint((unsigned char *)"消费笔数总计    金额总计",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[12],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
	}
	
	if(DataSave0.ChangeParamData.aucTermFlag[0]==1) 
	{
		uitotalnb=pTransTotal->uiPurchaseNb+pTransTotal->uiReturnNb;
		ultotalamount = pTransTotal->ulPurchaseAmount+pTransTotal->ulReturnAmount-pTransTotal->ulYJAmount;
	}
	else
	{
		uitotalnb=pTransTotal->uiPurchaseNb;
		ultotalamount = pTransTotal->ulPurchaseAmount;
	}
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"脱机消费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_PURCHASE);
	}
	
	if(DataSave0.ChangeParamData.aucTermFlag[0]==1) 
	{
		uitotalnb=pTransTotal->uiReturnNb;
		ultotalamount = pTransTotal->ulYJAmount;
	}
	else
	{
		uitotalnb=pTransTotal->uiReturnNb;
		ultotalamount = pTransTotal->ulReturnAmount;
	}
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"回收消费",FONT);
		if(DataSave0.ChangeParamData.aucTermFlag[0]==1) 
			Os__GB2312_xprint((unsigned char *)"总笔数     总押金额",FONT);
		else
			Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_RETURNCARD);
	}
	
	uitotalnb=pTransTotal->uiOnlinePurchaseNb;
	ultotalamount = pTransTotal->ulOnlinePurchaseAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"联机消费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_ONLINEPURCHASE);
	}
	
	uitotalnb=pTransTotal->uiLoadNb;
	ultotalamount = pTransTotal->ulLoadAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"充值",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_PRELOAD);
	}
	uitotalnb=pTransTotal->uiPayNb;
	ultotalamount = pTransTotal->ulPayAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"缴费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_PAY);
	}
	uitotalnb=pTransTotal->uiOnlinePayNb;
	ultotalamount = pTransTotal->ulOnlinePayAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"联机缴费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_PAYONLINE);
	}
	uitotalnb=pTransTotal->uiMobileNb;
	ultotalamount = pTransTotal->ulMobileAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"脱机扣款",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_MOBILECHARGE);
	}
	uitotalnb=pTransTotal->uiOnlineMobileNb;
	ultotalamount = pTransTotal->ulOnlineMobileAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"联机扣款",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_MOBILECHARGEONLINE);
	}
	uitotalnb=pTransTotal->uiSaleNb;
	ultotalamount = pTransTotal->ulSaleAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"卡激活",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_SALE);
	}
	uitotalnb=pTransTotal->uiSaleOnlineNb;
	ultotalamount = pTransTotal->ulSaleOnlineAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"批量激活",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_SALEONLINE);
	}
	uitotalnb=pTransTotal->uiOnlinePurchasepNb;
	ultotalamount = pTransTotal->ulOnlinePurchasepAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"积点消费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_ONLINEPURCHASE_P);
	}
	
	uitotalnb=pTransTotal->uiPurchasepNb;
	ultotalamount = pTransTotal->ulPurchasepAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"汇点卡脱机消费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_PURCHASE_P);
	}
	uitotalnb=pTransTotal->uiReturnpNb;
	ultotalamount = pTransTotal->ulReturnpAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"汇点卡回收消费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_RETURNCARD_P);
	}
	uitotalnb=pTransTotal->uiPreAuthNb;
	ultotalamount = pTransTotal->ulPreAuthAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"预授权",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_PREAUTH);
	}
	uitotalnb=pTransTotal->uiPreAuthVoidNb;
	ultotalamount = pTransTotal->ulPreAuthVoidAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"预授权撤销",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_PREAUTHVOID);
	}
	
	uitotalnb=pTransTotal->uiPreAuthFinishNb;
	ultotalamount = pTransTotal->ulPreAuthFinishAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"预授权完成",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_PREAUTHFINISH);
	}
	uitotalnb=pTransTotal->uiLoadOnlineNb;
	ultotalamount = pTransTotal->ulLoadOnlineAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"联机充值",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_LOADONLINE);
	}
	uitotalnb=pTransTotal->uiRefundNb;
	ultotalamount = pTransTotal->ulRefundAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"退货",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_REFUND);
	}
	uitotalnb=pTransTotal->uiChangeCardNb;
	ultotalamount = pTransTotal->ulChangeCardAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"换卡",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_CHANGECD);
	}
	uitotalnb=pTransTotal->uiChangeExpNb;
	ultotalamount = pTransTotal->ulChangeExpAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"续期",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_CHANGEEXP);
	}
	uitotalnb=pTransTotal->uiTransferPurchaseNb;
	ultotalamount = pTransTotal->ulTransferPurchaseAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"移资扣款",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_TRANSPURCHASE);
	}
	uitotalnb=pTransTotal->uiTransferNb;
	ultotalamount = pTransTotal->ulTransferAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"移资充值",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_TRANSFER);
	}
	uitotalnb=pTransTotal->uiExpPurchaseNb;
	ultotalamount = pTransTotal->ulExpPurchaseAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"过期卡消费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_TRANSFER);
	}
	uitotalnb=pTransTotal->uiRefAplNb;
	ultotalamount = pTransTotal->ulRefAplAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"退货申请",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
	}
	uitotalnb=pTransTotal->uiPayNobarNb;
	ultotalamount = pTransTotal->ulPayNobarAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"缴费",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_PAYNOBAR);
	}
	uitotalnb=pTransTotal->uiCashLoadNb;
	ultotalamount = pTransTotal->ulCashLoadAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"现金充值",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_CASHLOAD);
	}
	uitotalnb=pTransTotal->uiBatchLoadNb;
	ultotalamount = pTransTotal->ulBatchLoadAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"批量充值",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_CASHLOAD);
	}
	uitotalnb=pTransTotal->uiOnlineRefundNb;
	ultotalamount = pTransTotal->ulOnlineRefundAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"联机退货",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_CASHLOAD);
	}
	uitotalnb=pTransTotal->uiPTCPurchaseNb;
	ultotalamount = pTransTotal->ulPTCPurchaseAmount;
	if(uitotalnb)
	{
		Os__GB2312_xprint((unsigned char *)"公交卡充值扣款",FONT);
		Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);
		memset(aucBuf,' ',sizeof(aucBuf));
		aucBuf[PRINT_MAXCHAR]='\0';
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[6],ultotalamount,2);
		Os__GB2312_xprint((unsigned char *)aucBuf,FONT);
		PRT_IssueTotal(ucPtcode,TRANS_S_PTCPURCHASE);
	}
}


unsigned char PRT_Reprint(void)
{
	unsigned short 	uiIndex;
	unsigned char  	aucBuf[10];
	unsigned long	ulTraceNum;
    unsigned char   ucResult;
	unsigned short i;
	
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar *)"重打上笔交易Y/N");
#if 0
	if(DataSave0.TransInfoData.TransTotal.uiTotalNb==0)
	{
		Os__clr_display(255);
		Os__GB2312_display(0,0,(uchar *)"空交易");
		UTIL_GetKey(3);
		return SUCCESS;
	}
#endif
	
	for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
		if(DataSave0.TransInfoData.auiTransIndex[uiIndex]==0)
		{
			if(uiIndex==0)
			{
				return MSG_NOTRANS;
			}else
			{
				uiIndex--;		
				break;
			}
		}
	}

    i = TRANS_MAXNB-1;
	if(DataSave0.TransInfoData.auiTransIndex[i ] !=0)
		uiIndex = TRANS_MAXNB-1 ;

	if(uiIndex==TRANS_MAXNB)
		return MSG_NOTRANS;

	RUNDATA_ucReprintFlag=1;

	while(1)
	{
		memset(&NormalTransData,0,sizeof(NORMALTRANS));
        ucResult = XDATA_Read_SaveTrans_File(uiIndex);
        Uart_Printf("PRT_Reprint==ucResult=====%02x==========\n",ucResult);
        if(NormalTransData.ucKYFlag==1||NormalTransData.ucTransType==TRANS_S_BARCODE)
		{
			if(uiIndex==0)return ERR_NOTRANS;
			uiIndex--;
		}
		else
			break;
	}

	PRT_PrintTicket();	
	
	return SUCCESS;
}

unsigned char PRT_ReprintbyTrace(void)
{
     unsigned short 	uiIndex;
	unsigned char  	aucBuf[10];
	unsigned long	ulTraceNum;
	
      if(DataSave0.TransInfoData.TransTotal.uiTotalNb==0)
	{
//		Os__clr_display(255);
//		Os__GB2312_display(0,0,(uchar *)"空交易");
//		UTIL_GetKey(3);
		return SUCCESS;
	}

//	      Os__clr_display(255);
//		Os__GB2312_display(0,0,(uchar *)"请输入流水号");
    //	memset(aucBuf,0,sizeof(aucBuf));
//		if(UTIL_Input(1,true,6,6,'N',aucBuf,0)!=KEY_ENTER)
//			return ERR_CANCEL;
//		else
//		{
            //ulTraceNum=asc_long(aucBuf,6);
           ulTraceNum = ProUiFace.UiToPro.ulTraceNum;
    //	}
		
		for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
		{
			 memset(&NormalTransData,0,sizeof(NORMALTRANS));
		   XDATA_Read_SaveTrans_File(uiIndex);
			if((ulTraceNum==NormalTransData.ulTraceNumber)
				&&(DataSave0.TransInfoData.auiTransIndex[uiIndex]!=0)
				&&(NormalTransData.ucTransType!=TRANS_S_BARCODE))
				break;
		}

		if(uiIndex==TRANS_MAXNB)
		return ERR_NOTRANS;
		if(NormalTransData.ucKYFlag==1)
			return ERR_NOTRANS;
		if(NormalTransData.ucTransType==TRANS_S_BARCODE)
			return ERR_NOTRANS;
		
	RUNDATA_ucReprintFlag=1;
	

	PRT_PrintTicket();	
	
	return SUCCESS;
}


unsigned char PRT_ReprintSettle(void)
{
	RUNDATA_ucReprintFlag=1;
	if(memcmp(DataSave0.TransInfoData.TransTotal.aucBatchNumber,
				"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",12)==0)
	{
//		Os__clr_display(255);
//		Os__GB2312_display(0,0,(uchar *)"无结算单!");
//		UTIL_GetKey(3);
		return SUCCESS;
	}
	PRT_PrintSettle();
	return SUCCESS;
}

unsigned char PRT_Detail(void)
{
	PRT_Terminal();	
	return SUCCESS;
}

unsigned char PRT_TerminalTt(void)
{
       
	NORMALTRANS_ucTransType=TRANS_S_TERMINPRT;
//	if(memcmp(DataSave0.ChangeParamData.aucBatchNumber,
//				"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",12)==0)
//	{
//        //Os__clr_display(255);
//        //Os__GB2312_display(0,0,(uchar *)"空交易!");
//        //UTIL_GetKey(3);
//		return SUCCESS;
//	}

	PRT_PrintSettle();
	return SUCCESS;
}


void PRT_ExpDateTime(void)
{
	unsigned char Date[8],Time[8],display[30];
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	
	
	/*print data time*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]='\0';
	memset(Date,0,sizeof(Date));
	bcd_asc(Date,&NormalTransData.aucExpiredDate[1],2);
	memset(&Date[2],'/',1);
	bcd_asc(&Date[3],&NormalTransData.aucExpiredDate[2],2);
	memset(&Date[5],'/',1);
	bcd_asc(&Date[6],&NormalTransData.aucExpiredDate[3],2);

	memset(display,' ',sizeof(display));
	memcpy(display,"卡有效期:",9);
	memcpy(&display[9],Date,8);

	if(DataSave0.ConstantParamData.prnflagL)
	{
		memcpy(&aucPrintBuf[6],display,20);
		Os__GB2312_xprint((unsigned char*)aucPrintBuf,FONT);	
	}
	else
	{
		Os__GB2312_xprint((unsigned char*)display,FONT);
	}
	
}


void PRT_2ExpDateTime(void)
{
	unsigned char Date[8],Time[8],display[30];
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	
	
	/*print data time*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]='\0';
	memset(Date,0,sizeof(Date));
	bcd_asc(Date,&NormalTransData.aucExpiredDate[1],2);
	memset(&Date[2],'/',1);
	bcd_asc(&Date[3],&NormalTransData.aucExpiredDate[2],2);
	memset(&Date[5],'/',1);
	bcd_asc(&Date[6],&NormalTransData.aucExpiredDate[3],2);

	memset(display,' ',sizeof(display));
	memcpy(display,"原有效期:",9);
	memcpy(&display[9],Date,8);

	if(DataSave0.ConstantParamData.prnflagL)
	{
		memcpy(&aucPrintBuf[6],display,20);
		Os__GB2312_xprint((unsigned char*)aucPrintBuf,FONT);	
	}
	else
	{
		Os__GB2312_xprint((unsigned char*)display,FONT);
	}
	/*print data time*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]='\0';
	memset(Date,0,sizeof(Date));
	bcd_asc(Date,&NormalTransData.aucNewExpiredDate[1],2);
	memset(&Date[2],'/',1);
	bcd_asc(&Date[3],&NormalTransData.aucNewExpiredDate[2],2);
	memset(&Date[5],'/',1);
	bcd_asc(&Date[6],&NormalTransData.aucNewExpiredDate[3],2);

	memset(display,' ',sizeof(display));
	memcpy(display,"现有效期:",9);
	memcpy(&display[9],Date,8);

	if(DataSave0.ConstantParamData.prnflagL)
	{
		memcpy(&aucPrintBuf[6],display,20);
		Os__GB2312_xprint((unsigned char*)aucPrintBuf,FONT);	
	}
	else
	{
		Os__GB2312_xprint((unsigned char*)display,FONT);
	}
	
}

unsigned char PrintBar(ulong ulPayTrace)
{
	int i,j=0,finish=0,never=0,part=0;
	unsigned char aucPrintBuf[40],aucEnrst[20],ucResult=SUCCESS;
	unsigned long ulEncrypt=0;
	ulEncrypt+=ulPayTrace;
	Os__clr_display(255);
	Os__GB2312_display(0,0,(uchar *)"正在打印");

	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30]='\0';
	memcpy(&aucPrintBuf[0],(unsigned char *)"缴费帐单支付统计",16);
	Os__GB2312_xprint(aucPrintBuf,FONT); 

	for(i=0;i<TRANS_MAXNB;i++)
	{
		if(DataSave0.TransInfoData.auiTransIndex[i]!=0x01)
			continue;
		ucResult=XDATA_Read_SaveTrans_File_2Datasave(i);
			if(ucResult!=SUCCESS)
				return ucResult;
		if(DataSave0.SaveTransData.ucTransType==TRANS_S_BARCODE)
		{
			if(DataSave0.SaveTransData.ulTraceNumber==ulPayTrace)
			{

				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				memcpy(aucPrintBuf,DataSave0.SaveTransData.aucBarName,14);
				UTIL_Form_Montant(&aucPrintBuf[strlen((char*)aucPrintBuf)],DataSave0.SaveTransData.ulAmount,2);
				Os__GB2312_xprint(aucPrintBuf,FONT);

				memset(aucPrintBuf,0,sizeof(aucPrintBuf));
				bcd_asc(aucPrintBuf,DataSave0.SaveTransData.aucBarCode,24);
				Os__GB2312_xprint(aucPrintBuf,FONT);
				if(bcd_long(&DataSave0.SaveTransData.ucBarLenth,2)>24)
				{
					memset(aucPrintBuf,0,sizeof(aucPrintBuf));
					bcd_asc(aucPrintBuf,&DataSave0.SaveTransData.aucBarCode[12],bcd_long(&DataSave0.SaveTransData.ucBarLenth,2)-24);
					Os__GB2312_xprint(aucPrintBuf,FONT);
				}
				ulEncrypt+=DataSave0.SaveTransData.ulAmount;
				Os__xlinefeed(0.5*LINENUM);
				
			}
		}
	
	}
	Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	long_tab(aucPrintBuf,4,&ulEncrypt);
	asc_bcd(&aucPrintBuf[4],4,DataSave0.ConstantParamData.aucTerminalID,8);
	des(aucPrintBuf,aucPrintBuf,DataSave0.ConstantParamData.MacMasterKey[0]);
	desm1(aucPrintBuf,aucPrintBuf,DataSave0.ConstantParamData.MacMasterKey[1]);
	des(aucPrintBuf,aucPrintBuf,DataSave0.ConstantParamData.MacMasterKey[0]);
	memset(aucEnrst,0,sizeof(aucEnrst));
	hex_asc(aucEnrst,aucPrintBuf,16);
	Os__xprint(aucEnrst);
	if(DataSave0.ConstantParamData.prnflag)
		Os__xlinefeed(5*LINENUM);
	else
	{
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
	}
	return SUCCESS;
}


void PRT_Sale(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1],ucResult=SUCCESS;
	Os__clr_display(255);
	Os__GB2312_display(0,0,((uchar *)"正在打印"));
	Os__light_off();
	if(DataSave0.ConstantParamData.Pinpadflag)
		Os__light_off_pp();
	
	Os__GB2312_xprint((unsigned char *)"售卡",FONT);
	Os__GB2312_xprint((unsigned char *)"总笔数     总金额",FONT);

	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]='\0';
	int_asc(aucPrintBuf,3,&RunData.uiSaleCardNb);
	UTIL_Form_Montant(&aucPrintBuf[6],RunData.ulSaleCardAmount,2);
	Os__GB2312_xprint((unsigned char *)aucPrintBuf,FONT);
	if(RunData.aucFunFlag[15])
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"入账金额:",9);
		UTIL_Form_Montant(&aucPrintBuf[9],RunData.ulSaleCountAmount,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
	}
	Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);
			
	if(DataSave0.ConstantParamData.prnflag)
		Os__xlinefeed(5*LINENUM);
	else
	{
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
	}
	Os__clr_display(255);
} 


void PRT_SaleOnline(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1],ucResult=SUCCESS;
	Os__clr_display(255);
	Os__GB2312_display(0,0,((uchar *)"正在打印"));
	Os__light_off();
	if(DataSave0.ConstantParamData.Pinpadflag)
		Os__light_off_pp();
	
	/*memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]='\0';
	memcpy(aucPrintBuf,(unsigned char *)"总批数:",7);
	UTIL_Form_Montant(&aucPrintBuf[7],RunData.uiSaleCardNb,0);
	Os__GB2312_xprint((unsigned char *)aucPrintBuf,FONT);*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]='\0';
	memcpy(aucPrintBuf,(unsigned char *)"总卡数:",7);
	UTIL_Form_Montant(&aucPrintBuf[7],RunData.uiSaleCardDetail,0);
	Os__GB2312_xprint((unsigned char *)aucPrintBuf,FONT);
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]='\0';
	memcpy(aucPrintBuf,(unsigned char *)"总金额:",7);
	UTIL_Form_Montant(&aucPrintBuf[7],RunData.ulSaleCardAmount,2);
	Os__GB2312_xprint((unsigned char *)aucPrintBuf,FONT);
	if(RunData.aucFunFlag[15])
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"总入账:",7);
		UTIL_Form_Montant(&aucPrintBuf[7],RunData.ulSaleCountAmount,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);
	}
	Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);
	if(DataSave0.ConstantParamData.prnflag)
		Os__xlinefeed(5*LINENUM);
	else
	{
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
	}
	Os__clr_display(255);
} 


void PRT_BatchLoad(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1],ucResult=SUCCESS;
	Os__clr_display(255);
	Os__GB2312_display(0,0,((uchar *)"正在打印"));
	Os__light_off();
	if(DataSave0.ConstantParamData.Pinpadflag)
		Os__light_off_pp();
	
	/*memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]='\0';
	memcpy(aucPrintBuf,(unsigned char *)"总批数:",7);
	UTIL_Form_Montant(&aucPrintBuf[7],RunData.uiSaleCardNb,0);
	Os__GB2312_xprint((unsigned char *)aucPrintBuf,FONT);*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]='\0';
	memcpy(aucPrintBuf,(unsigned char *)"总卡数:",7);
	UTIL_Form_Montant(&aucPrintBuf[7],RunData.uiSaleCardDetail,0);
	Os__GB2312_xprint((unsigned char *)aucPrintBuf,FONT);
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[PRINT_MAXCHAR]='\0';
	memcpy(aucPrintBuf,(unsigned char *)"总金额:",7);
	UTIL_Form_Montant(&aucPrintBuf[7],RunData.ulSaleCardAmount,2);
	Os__GB2312_xprint((unsigned char *)aucPrintBuf,FONT);
	Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);
	if(DataSave0.ConstantParamData.prnflag)
		Os__xlinefeed(5*LINENUM);
	else
	{
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
	}
	Os__clr_display(255);
} 
unsigned char	PRT_ZSH_PrintTicket(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1],uiI,aucBuf[30];
	
	if(!RunData.ucBcksp)
	{
		if(DataSave0.ConstantParamData.printtimes==2)
			Os__GB2312_xprint((unsigned char *)"客户联",FONT);
		PRT_ConstantParam();
	}
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	UTIL_Form_Montant(&aucPrintBuf[13],NormalTransData.ulAmount,2);
	bcd_asc(&aucPrintBuf[0],NormalTransData.aucUserCardNo,16);
	aucPrintBuf[24]=0x00;
	Os__GB2312_xprint(aucPrintBuf,FONT); 
	return SUCCESS;
}
void PRT_ZSH_Total(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned long ultotalamount,ultotalrate;
	unsigned int uitotalnb;

	uitotalnb=DataSave0.TransInfoData.TransTotal.uiZshPurchaseNb+DataSave0.TransInfoData.TransTotal.uiZshOnlinePurchaseNb+DataSave0.TransInfoData.TransTotal.uiJsZshOnlinePurchaseNb;
	ultotalamount = DataSave0.TransInfoData.TransTotal.ulZshPurchaseAmount+DataSave0.TransInfoData.TransTotal.ulZshOnlinePurchaseAmount+DataSave0.TransInfoData.TransTotal.ulJsZshOnlinePurchaseAmount;
	ultotalrate=DataSave0.TransInfoData.TransTotal.ulZshPurchaseRate+DataSave0.TransInfoData.TransTotal.ulZshOnlinePurchaseRate+DataSave0.TransInfoData.TransTotal.ulJsZshOnlinePurchaseRate;

	if(uitotalnb!=0)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"总计扣款:",9);
		UTIL_Form_Montant(&aucPrintBuf[9],ultotalamount,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);

		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[30]='\0';
		memcpy(aucPrintBuf,(unsigned char *)"手 续 费:",9);
		UTIL_Form_Montant(&aucPrintBuf[9],ultotalrate,2);
		Os__GB2312_xprint(aucPrintBuf,FONT);

		if(ultotalamount>=ultotalrate)
		{
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			aucPrintBuf[30]='\0';
			memcpy(aucPrintBuf,(unsigned char *)"应充金额:",9);
			ultotalamount=ultotalamount-ultotalrate;
			UTIL_Form_Montant(&aucPrintBuf[9],ultotalamount,2);
			Os__GB2312_xprint(aucPrintBuf,FONT);
		}

		Os__GB2312_xprint((unsigned char *)"-----------------------",FONT);
	
		
	}
}
void PRT_ZSH_SettleResult(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned long ultotalamount,ultotalrate;
	unsigned int uitotalnb;
	Os__GB2312_xprint((unsigned char *)"批次号:",FONT);
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[30]='\0';
	bcd_asc(&aucPrintBuf[0],DataSave0.ChangeParamData.aucBatchNumber,24);
	Os__xprint((unsigned char*)aucPrintBuf);
	if(RUNDATA_ucSettleStatus[0]==SETTLE_UNBALANCE
		||RUNDATA_ucSettleStatus[1]==SETTLE_UNBALANCE
		||RUNDATA_ucSettleStatus[2]==SETTLE_UNBALANCE)
		Os__GB2312_xprint((unsigned char *)"结算不平",FONT);
	else
		Os__GB2312_xprint((unsigned char *)"结算成功",FONT);
	if(DataSave0.ConstantParamData.prnflag)
		Os__xlinefeed(5*LINENUM);
	else if(DataSave0.ConstantParamData.prnflagL)
	{
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
	}
	else
	{
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
		Os__xlinefeed(5*LINENUM);
	}
}
unsigned char	PRT_ZSH_PrintTicket2(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1],uiI,aucBuf[30];
	
	PRT_ConstantParam();
	for(uiI=0;uiI<DataSave0.TransInfoData.TransTotal.uiTotalNb;uiI++)
	{
		if(DataSave0.TransInfoData.auiTransIndex[uiI])
		{
			memset(&NormalTransData,0,sizeof(NORMALTRANS));
			XDATA_Read_SaveTrans_File(uiI);
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			UTIL_Form_Montant(&aucPrintBuf[13],NormalTransData.ulAmount,2);
			bcd_asc(&aucPrintBuf[0],NormalTransData.aucUserCardNo,16);
			aucPrintBuf[24]=0x00;
			Os__GB2312_xprint(aucPrintBuf,FONT); 
		}
	}
	PRT_ZSH_Total();
	return SUCCESS;
}

