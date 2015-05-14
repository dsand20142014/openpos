/*
	SAND China
--------------------------------------------------------------------------
	FILE  print.c								(Copyright SAND 2001)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2001-08-13		Xiaoxi Jiang
    Last modified :	2001-08-13		Xiaoxi Jiang
    Module :  Shanghai Golden Card Network
    Purpose :
        Source file for print.

    List of routines in file :

    File history :

*/

#include <include.h>
#include <global.h>
#include <xdata.h>
#include <print.h>
#include <osgraph.h>
#include <oslib.h>

unsigned char PRINT_InstallmentStageFormat(unsigned char *pstrInBuf,unsigned char *pstrOutBuf)
{
	unsigned char aucTempBuf[12];
	unsigned char  ucLen,uiI;

	util_Printf("奖励积分:%s\n",pstrInBuf);
	memset(aucTempBuf,0,sizeof(aucTempBuf));
	if (pstrInBuf==NULL)
		return(ERR_END);
	ucLen=strlen((char*)pstrInBuf);
	util_Printf("数据Len=%d\n",ucLen);
	for(uiI=0;uiI<ucLen;uiI++)
	{
		util_Printf("pstrInBuf=[%02x]\n",pstrInBuf[uiI]);
		if (pstrInBuf[uiI] != '0')
		{
			util_Printf("uiI=%d\n",uiI);
			memcpy(pstrOutBuf,&pstrInBuf[uiI],ucLen-uiI);
			util_Printf("奖励R积R分:%s\n",pstrOutBuf);
			break;
		}
	}
	return(SUCCESS);
}

void PRINT_InstallmentInfor(unsigned char ucFont)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned char aucPrintBuf1[PRINT_MAXCHAR+1];

	FONT=ucFont;

	if (G_NORMALTRANS_ucTransType==TRANS_CREDITSALE)
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memcpy( aucPrintBuf, "分期数:",7 );
		util_Printf("\n交易期数:%s",G_NORMALTRANS_aucNumStages);
		memcpy( &aucPrintBuf[7], G_NORMALTRANS_aucNumStages,TRANS_NumStages);
		PRINT_GB2312_xprint(aucPrintBuf,FONT);

		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memcpy( aucPrintBuf, "首付金额:",9 );
		UTIL_Form_Montant(&aucPrintBuf[9],G_NORMALTRANS_ulRepayAmount,2);

		PRINT_GB2312_xprint(aucPrintBuf,FONT);
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memcpy( aucPrintBuf, "交易币种:",9 );
		if (!memcmp(G_NORMALTRANS_aucCBuf,"156",3))
			memcpy(&aucPrintBuf[9],"人民币(RMB)",11);
		else
			memcpy(&aucPrintBuf[9],G_NORMALTRANS_aucCBuf,3);
		PRINT_GB2312_xprint(aucPrintBuf,FONT);
		//一次性支付手续费时打印
		if (G_NORMALTRANS_bInstallments)
		{
			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memcpy( aucPrintBuf, "手续费:",7 );
			UTIL_Form_Montant(&aucPrintBuf[7],G_NORMALTRANS_ulRepayFeeAmt,2);

			PRINT_GB2312_xprint(aucPrintBuf,FONT);

			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memset(aucPrintBuf1,0,sizeof(aucPrintBuf1));
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memcpy( aucPrintBuf, "奖励积分:",9 );
			long_asc(aucPrintBuf1,12,&G_NORMALTRANS_ulStages);
			PRINT_InstallmentStageFormat(aucPrintBuf1,&aucPrintBuf[9]);

			PRINT_GB2312_xprint(aucPrintBuf,FONT);

			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memcpy( aucPrintBuf, "手续费支付方式:",15 );
			memcpy(&aucPrintBuf[15],"一次性支付",10);

			PRINT_GB2312_xprint(aucPrintBuf,FONT);
		}else
		{
			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memset(aucPrintBuf1,0,sizeof(aucPrintBuf1));
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memcpy( aucPrintBuf, "奖励积分:",9 );
			long_asc(aucPrintBuf1,12,&G_NORMALTRANS_ulStages);
			PRINT_InstallmentStageFormat(aucPrintBuf1,&aucPrintBuf[9]);

			PRINT_GB2312_xprint(aucPrintBuf,FONT);

			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memcpy( aucPrintBuf, "手续费支付方式:",15 );
			memcpy(&aucPrintBuf[15],"分期支付",8);
			PRINT_GB2312_xprint(aucPrintBuf,FONT);

			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memcpy( aucPrintBuf, "首期手续费:",11 );
			UTIL_Form_Montant(&aucPrintBuf[11],G_NORMALTRANS_ulFCurrency,2);
			PRINT_GB2312_xprint(aucPrintBuf,FONT);

			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memcpy( aucPrintBuf, "每期手续费:",11 );
			UTIL_Form_Montant(&aucPrintBuf[11],G_NORMALTRANS_ulNCurrency,2);
			PRINT_GB2312_xprint(aucPrintBuf,FONT);
		}
	}
}
/*---------------------------------------------------------
功能说明:
	打印不同格式的两行行汉字，不足两行打印空格;
参数说明:
	ptSrc：		  需要打印的数据
	uiDataLen：	  打印数据的长度
	ucPrinterType:打印机类型:0X01:热敏,0X02:针打,0X03:套打
-----------------------------------------------------------*/
unsigned char PrintGB2312Info(unsigned char *ptSrc,unsigned int uiDataLen,unsigned char ucPrinterType )
{
	unsigned char aucPrintBuf[64+1];
	unsigned int  uiI;
	unsigned int  uiPrintLen,uiLen,uiNum;
	unsigned char ucASCNum;
	unsigned int  uiRowNum,uiRowLen;
	unsigned char ucFont;

	//if(uiPrintLen>40) uiPrintLen =40;
	if(ucPrinterType == 0x03)//套打
	{
		uiRowLen = 30;
		ucFont = 0x01;
	}else
	if(ucPrinterType == 0x01)//热敏
	{
		uiRowLen = 40;
		ucFont = 0x1D;
	}else									//针打
	{
		uiRowLen = 24;
		ucFont = 0X00;
	}

	for(uiPrintLen=0;uiPrintLen<uiDataLen;uiPrintLen++)
	{
		if( ptSrc[uiPrintLen] == 0 )
			break;
	}

	//现在统一规定，打印两行，多余的不打印，不足两行的第二行打印空格,无需打印的两行空格
	if((uiDataLen == 0)
	 ||(  (uiDataLen == 1)&&(*ptSrc >= 0x80)  )
	 ||(uiPrintLen == 0)
	 )
	{
	    if(ucPrinterType == 0x01 ||ucPrinterType == 0x02)//热敏
	        return 0;
	     else
            {
                Os__GB2312_xprint((unsigned char *)" ",ucFont);
                Os__GB2312_xprint((unsigned char *)" ",ucFont);
            }
            return 0;
	}

	uiLen = 0;
	uiRowNum = 0;
	do
	{
		for(uiNum = 0;((uiLen+uiNum) < uiPrintLen)&&(uiNum < uiRowLen);uiNum++);

		if(uiNum >= uiRowLen)
		{
			uiI = 0;
			ucASCNum = 0;
			while(uiI < uiNum)
			{
				if(ptSrc[uiLen + uiI] < 0x80)
				{
					ucASCNum++;
					uiI++;
				}else
				{
					uiI+=2;
				}
			}
			if((ucASCNum%2) != (uiRowLen%2))
			{
				uiNum--;
			}
		}

		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,&ptSrc[uiLen],uiNum);
#if PS300
		PRINT_ext_GB2312_xprint(aucPrintBuf,ucFont);
#endif
#if PS100|PS400
		Os__GB2312_xprint((unsigned char *)aucPrintBuf,ucFont);
#endif
		uiRowNum++;
		uiLen += uiNum;
		//现在统一规定，打印两行，多余的不打印，不足两行的第二行打印空格,无需打印的两行空格
		if(uiRowNum == 0x02)
			break;
	}while(uiLen < uiPrintLen);

	if(uiRowNum == 0x00)
	{
		Os__GB2312_xprint((unsigned char *)" ",ucFont);
		Os__GB2312_xprint((unsigned char *)" ",ucFont);
	}
	if(uiRowNum == 0x01 && (ucPrinterType != 0x01 && ucPrinterType != 0x02))
	{
		Os__GB2312_xprint((unsigned char *)" ",ucFont);
	}
	return uiRowNum;
}

void PRINT_ConstantParam(void)
{
	PRINT_PrintMsg( FONT,MSG_CONFIGBASEDATA);

	PRINT_PrintTwoMSG( FONT,MSG_TERMINALID,DataSave0.ConstantParamData.aucTerminalID,
						PARAM_TERMINALIDLEN,-1,0,0);
	PRINT_PrintTwoMSG( FONT,MSG_MERCHANTID,DataSave0.ConstantParamData.aucMerchantID,
						PARAM_MERCHANTIDLEN,-1,0,0);
	PRINT_PrintTwoMSG( FONT,MSG_MERCHANTNAME,DataSave0.ConstantParamData.aucMerchantName,
						strlen((char *)DataSave0.ConstantParamData.aucMerchantName),-1,0,0);
	PRINT_PrintTwoMSG( FONT,MSG_HOSTTEL1,DataSave0.ConstantParamData.aucHostTelNumber1,
						strlen((char *)DataSave0.ConstantParamData.aucHostTelNumber1),-1,0,0);
    PRINT_xlinefeed(5*LINENUM);
}

void PRINT_Ticket(unsigned char flag,unsigned char ucSaleCopy)
{
	Os__light_off();
	PRINT_Ticket_HeatPrinter(flag,ucSaleCopy);
	Os__light_on();
}

void PRINT_transType_CN(unsigned char Flag,unsigned char ucFONT)
{
	unsigned char ucTransType;

	if ( Flag == 0)
		ucTransType = G_NORMALTRANS_ucTransType;
	else
		ucTransType = DataSave0.Trans_8583Data.ReversalTrans.ucTransType;

	switch(ucTransType)
	{
		case TRANS_PURCHASE:
			if((G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_HB_PUTCARD)
			    ||(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_SAND_PUTCARD))
				PRINT_GB2312_xprint((unsigned char*)"QPBOC消费",ucFONT);
			else if(G_NORMALTRANS_ucECTrans ==TRANS_EC_PURCHASE)
				PRINT_GB2312_xprint((unsigned char*)"EC 消 费", ucFONT);
			else
				PRINT_GB2312_xprint((unsigned char*)"消 费", ucFONT);
	   		break;
		case TRANS_CUPMOBILE:
			PRINT_GB2312_xprint((unsigned char*)"移动支付消费", ucFONT);
			break;
		case TRANS_VOIDCUPMOBILE:
			PRINT_GB2312_xprint((unsigned char*)"移动支付撤销", ucFONT);
			break;
		case TRANS_PREAUTH:
			PRINT_GB2312_xprint((unsigned char*)"预授权", ucFONT);
			break;
		case TRANS_PREAUTHFINISH:
			PRINT_GB2312_xprint((unsigned char*)"预授权完成(联机)", ucFONT);
			break;
		case TRANS_VOIDPURCHASE:
			PRINT_GB2312_xprint((unsigned char*)"消费撤销", ucFONT);
			break;
		case TRANS_VOIDPREAUTH:
			PRINT_GB2312_xprint((unsigned char*)"预授权撤销", ucFONT);
			break;
		case TRANS_VOIDPREAUTHFINISH:
			PRINT_GB2312_xprint((unsigned char*)"预授权完成撤销", ucFONT);
			break;
		case TRANS_REFUND:
			PRINT_GB2312_xprint((unsigned char*)"退 货", ucFONT);
			break;
		case TRANS_TIPADJUST:
		case TRANS_TIPADJUSTOK:
			PRINT_GB2312_xprint((unsigned char*)"离线调整", ucFONT);
			break;
		case TRANS_UNDOOFF:
			PRINT_GB2312_xprint((unsigned char*)"离线撤销", ucFONT);
			break;
		case TRANS_OFFPREAUTHFINISH:
			PRINT_GB2312_xprint((unsigned char*)"离线结算", ucFONT);
			break;
		case TRANS_OFFPURCHASE:
			if((G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_HB_PUTCARD)
			    ||(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_SAND_PUTCARD))
				PRINT_GB2312_xprint((unsigned char*)"QPBOC脱机消费",ucFONT);
			else if(G_NORMALTRANS_ucECTrans ==TRANS_EC_PURCHASE)
				PRINT_GB2312_xprint((unsigned char*)"EC脱机消费",ucFONT);
			else
				PRINT_GB2312_xprint((unsigned char*)"脱机消费", ucFONT);
			break;
		case TRANS_VOIDOFFPURCHASE:
			PRINT_GB2312_xprint((unsigned char*)"脱机消费撤销", ucFONT);
			break;
		case TRANS_PREAUTHSETTLE:
			PRINT_GB2312_xprint((unsigned char*)"预授权完成(离线)", ucFONT);
			break;
		case TRANS_PREAUTHADD:
			PRINT_GB2312_xprint((unsigned char*)"追加预授权", ucFONT);
			break;
		case TRANS_EC_ASSIGNLOAD:
			PRINT_GB2312_xprint((unsigned char*)"指定账户圈存", ucFONT);
			break;
		case TRANS_EC_UNASSIGNLOAD:
			PRINT_GB2312_xprint((unsigned char*)"非指定账户圈存", ucFONT);
			break;
		case TRANS_EC_CASHLOAD:
			PRINT_GB2312_xprint((unsigned char*)"现金充值", ucFONT);
			break;
		case TRANS_MOBILEAUTH:
			PRINT_GB2312_xprint((unsigned char*)"手机无卡预约消费", ucFONT);
			break;
		case TRANS_VOIDMOBILEAUTH:
			PRINT_GB2312_xprint((unsigned char*)"手机预约撤销", ucFONT);
			break;
		case TRANS_MOBILEREFUND:
			PRINT_GB2312_xprint((unsigned char*)"手机预约退货", ucFONT);
			break;
		case TRANS_CREDITSALE:
			PRINT_GB2312_xprint((unsigned char*)"分期付款消费", ucFONT);		 
			break;
		case TRANS_CREDITSVOID:
			PRINT_GB2312_xprint((unsigned char*)"分期付款撤消", ucFONT);
			break;
		case TRANS_CREDITSREFUND:
			PRINT_GB2312_xprint((unsigned char*) "分期付款退货", ucFONT);
			break;
		default:
			PRINT_GB2312_xprint((unsigned char*)"未知交易类型", ucFONT);
			break;
	}
}
void PRINT_Ticket_HeatPrinter_1_CN(unsigned char flag,unsigned char ucCopy)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned char aucTotalAmtBuf[PRINT_MAXCHAR+1];
	unsigned char aucCardNo[21];
	unsigned char ucI;
	unsigned char temp[17];
	unsigned char aucPrintBuf1[PRINT_MAXCHAR+1];
	unsigned char aucPrintBuf2[PRINT_MAXCHAR+1];
	unsigned long ulAmount;
	unsigned int uiPrintLen;
	
#ifdef GUI_PROJECT
	util_Printf("\n-----------------------------------------\n0.PRINT_Ticket_HeatPrinter_1_CN\n1.    China Unipay PBOC2.0");
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,DataSave0.ConstantParamData.aucMerchantID,15 );
	util_Printf("\n2.    MERCH ID:%s",aucPrintBuf);
	util_Printf("\n3.    TERM  ID:%s",DataSave0.ConstantParamData.aucTerminalID);
	util_Printf("\n4.    BatchNumber:%06d",DataSave0.ChangeParamData.ulBatchNumber);
	util_Printf("\n5.    TraceNumber:%06d",G_NORMALTRANS_ulTraceNumber);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	if(G_NORMALTRANS_ucSourceAccLen%2)
		bcd_asc(aucPrintBuf,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
	else
		bcd_asc(aucPrintBuf,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
	util_Printf("\n6.    CARD NUMBER:%s",aucPrintBuf);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(&aucPrintBuf[0],"20",2);
	bcd_asc(&aucPrintBuf[2],G_NORMALTRANS_ucExpiredDate,2);
	aucPrintBuf[4] = '/';
	bcd_asc(&aucPrintBuf[5],&G_NORMALTRANS_ucExpiredDate[1],2);
	util_Printf("\n7.    ExpiredDate:%s",aucPrintBuf);
	UTIL_Form_Montant(&aucPrintBuf[0],G_NORMALTRANS_ulAmount,2);
	util_Printf("\n8.    Amount:%s",aucPrintBuf);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(&aucPrintBuf[0],G_NORMALTRANS_aucAuthCode,TRANS_AUTHCODELEN);
	util_Printf("\n9.    AuthCode:%s",aucPrintBuf);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(&aucPrintBuf[0],G_NORMALTRANS_aucRefNum,TRANS_REFNUMLEN);
	util_Printf("\n10    RefNum:%s\n-----------------------------------------\n",aucPrintBuf);
	return ;
#endif

	if(ucCopy != 2)
	{
		PRINT_GB2312_xprint((unsigned char *)"    中国银联POS签购单",0x1E);
	}

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',48);

	if(ucCopy == 0)
	{
		memcpy(aucPrintBuf,"商户存根",8);
		memcpy(&aucPrintBuf[38],"请妥善保管",10);
	}
	else if (ucCopy ==1)
	{
		memcpy(aucPrintBuf,"持卡人存根",10);
		memcpy(&aucPrintBuf[38],"请妥善保管",10);
	}
	if(ucCopy != 2)
	{
		PRINT_GB2312_xprint(aucPrintBuf,0x01);
	}
	if (ucCopy == 2)
	{
		PRINT_GB2312_xprint((unsigned char *)"    中国银联POS签购单",0x1E);
	}
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户名称:",9);
	uiPrintLen = strlen((char*)DataSave0.ConstantParamData.aucMerchantName);
	if(uiPrintLen>40) uiPrintLen =40;
	memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucMerchantName,uiPrintLen);
	PrintGB2312Info(aucPrintBuf,uiPrintLen+9,0x01);

   memset(aucPrintBuf,0,sizeof(aucPrintBuf));
   memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户编号:",9);
	memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
	memcpy(&aucPrintBuf[29],"操作员:",7);
	memcpy(&aucPrintBuf[36],DataSave0.Cashier_SysCashier_Data.aucCashierNo[DataSave0.ChangeParamData.ucCashierLogonIndex],
				CASH_CASHIERNOLEN);
	PRINT_GB2312_xprint(aucPrintBuf,0X1D);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"终端编号:",9);
	memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
	memcpy(&aucPrintBuf[29],"清算日期:",9);
	memset(temp,0,sizeof(temp));
	memcpy(temp,G_NORMALTRANS_aucSettleDate,2);
	memcpy(&temp[2],"/",1);
	memcpy(&temp[3],&G_NORMALTRANS_aucSettleDate[2],2);
	memcpy(&aucPrintBuf[38],temp,5);
	PRINT_GB2312_xprint(aucPrintBuf,0X1D);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf1,0,sizeof(aucPrintBuf1));
	memset(aucPrintBuf1,0x20,sizeof(aucPrintBuf1)-1);
	/* Issuer ID */
	if(G_NORMALTRANS_ucCardType==TRANS_CARDTYPE_INTERNAL)
	{
		memset(temp,0,sizeof(temp));
		memcpy(temp,G_NORMALTRANS_aucAcquereID,8);
		if (!memcmp(temp,"0100",4))
		{
			memcpy( aucPrintBuf1,"发卡行:" ,7);
			GetINCardName(1,G_NORMALTRANS_aucIssuerID,aucPrintBuf);
			memcpy(&aucPrintBuf1[7],aucPrintBuf,strlen((char *)aucPrintBuf));
			PRINT_GB2312_xprint(aucPrintBuf1,0x1D);

			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memset(aucPrintBuf1,0,sizeof(aucPrintBuf1));
			memset(aucPrintBuf1,0x20,sizeof(aucPrintBuf1)-1);
			/* Acquere ID  */
			memcpy(&aucPrintBuf1[0],"收单行:" ,7);
			
			GetINCardName(2,G_NORMALTRANS_aucAcquereID,aucPrintBuf);			
			memcpy(&aucPrintBuf1[7],aucPrintBuf,strlen((char *)aucPrintBuf));
			PRINT_GB2312_xprint(aucPrintBuf1,0x1D);
		}else
		{
			memcpy( aucPrintBuf1,"发卡行:" ,7);
			GetINCardName(1,G_NORMALTRANS_aucIssuerID,aucPrintBuf);
			memcpy(&aucPrintBuf1[7],aucPrintBuf,strlen((char *)aucPrintBuf));

			/* Acquere ID  */
			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memcpy(&aucPrintBuf1[29],"收单行:" ,7);
			memcpy(temp,G_NORMALTRANS_aucAcquereID,8);
			GetINCardName(2,G_NORMALTRANS_aucAcquereID,aucPrintBuf);
			if(strlen((char *)aucPrintBuf))
				memcpy(&aucPrintBuf1[36],aucPrintBuf,strlen((char *)aucPrintBuf));
			else
				memcpy(&aucPrintBuf1[36],G_NORMALTRANS_aucAcquereID ,ACQUERELEN);
			PRINT_GB2312_xprint(aucPrintBuf1,0x1D);
		}
	}
	else
	{
		/* Acquere ID  */
		memcpy(&aucPrintBuf1[0],"收单行:" ,7);
		memcpy(temp,G_NORMALTRANS_aucAcquereID,8);
		GetINCardName(2,G_NORMALTRANS_aucAcquereID,aucPrintBuf);
		if(strlen((char *)aucPrintBuf))
			memcpy(&aucPrintBuf1[7],aucPrintBuf,strlen((char *)aucPrintBuf));
		else
			memcpy(&aucPrintBuf1[7],G_NORMALTRANS_aucAcquereID ,ACQUERELEN);
		PRINT_GB2312_xprint(aucPrintBuf1,0x1D);
	}
    
    	if ((DataSave0.ConstantParamData.ucORDERFLAG == '1')&&(DataSave0.ConstantParamData.uclujiaomc=='1')&&((G_NORMALTRANS_ucTransType == TRANS_PURCHASE
			||G_NORMALTRANS_ucTransType == TRANS_OFFPURCHASE || G_NORMALTRANS_ucTransType == TRANS_MOBILEAUTH)||(G_NORMALTRANS_ucTransType == TRANS_VOIDPURCHASE
					||G_NORMALTRANS_ucTransType==TRANS_VOIDMOBILEAUTH)))
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"客户名称:",9);
		uiPrintLen = strlen((char*)NormalTransData.aucCustName);
		if(uiPrintLen>20) uiPrintLen =20;
		memcpy(&aucPrintBuf[9],NormalTransData.aucCustName,uiPrintLen);
		PrintGB2312Info(aucPrintBuf,uiPrintLen+9,0x01);
	}

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	if(G_NORMALTRANS_ucTransType==TRANS_EC_UNASSIGNLOAD)
		PRINT_GB2312_xprint((unsigned char*)"转入方卡号:",0x1D);
	else
		PRINT_GB2312_xprint((unsigned char*)"卡    号:",0x1D);
	//PRINT_GB2312_xprint((unsigned char*)"卡    号:",0x1D);
	if(G_NORMALTRANS_ucSourceAccLen%2)
	{
		bcd_asc(aucPrintBuf,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
	}else
	{
		bcd_asc(aucPrintBuf,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
	}
	if(G_NORMALTRANS_ucSourceAccLen%2)
	 {
		aucCardNo[G_NORMALTRANS_ucSourceAccLen] =' ';
	 }
	memcpy(&aucPrintBuf[G_NORMALTRANS_ucSourceAccLen],"     ",3);

	if ((G_NORMALTRANS_ucTransType != TRANS_TIPADJUST) &&
		(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)&&
		(G_NORMALTRANS_ucTransType != TRANS_UNDOOFF)&&
		(G_NORMALTRANS_ucTransType != TRANS_EC_UNASSIGNLOAD)&&
		(G_NORMALTRANS_ucTransType != TRANS_OFFPURCHASE)&&
		(G_NORMALTRANS_ucECTrans !=TRANS_EC_PURCHASE)
		){
		if(DataSave0.ConstantParamData.ucPrintCardNo==0x31)
		{
			if (DataSave0.ConstantParamData.ucPBeforeSix==0x30)
			{
				if (DataSave0.ConstantParamData.ucCardtype == 0x30)
				{
	/*Changed 08-12-10*/
					if (G_NORMALTRANS_euCardSpecies==CARDSPECIES_MAG)
					{
						if (DataSave0.ConstantParamData.ucDisAuth == 0x30)
						{
							if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
							{
								for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
								{
								   aucPrintBuf[ucI]='*';
								}
							}
						}
						else if (DataSave0.ConstantParamData.ucDisAuth == 0x31)
						{
							for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
							   aucPrintBuf[ucI]='*';
							}
						}
					}
	/*=End=*/
				}
				else if (DataSave0.ConstantParamData.ucCardtype == 0x31)
				{
	//Changed 08-12-10
					if (G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV
						|| DataSave0.ConstantParamData.ucForeignCard==0x31)
					{
						if (DataSave0.ConstantParamData.ucDisAuth == 0x30)
						{
							if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
							{
								for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
								{
								   aucPrintBuf[ucI]='*';
								}
							}
						}
						else if (DataSave0.ConstantParamData.ucDisAuth == 0x31)
						{
							for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
							   aucPrintBuf[ucI]='*';
							}
						}
					}
	//End
	/*Changed 08-12-3*/
				}
				else if (DataSave0.ConstantParamData.ucCardtype == 0x32)
				{
					if (DataSave0.ConstantParamData.ucDisAuth == 0x30)
					{
						if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
						{
							for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
							   aucPrintBuf[ucI]='*';
							}
						}
					}
					else if (DataSave0.ConstantParamData.ucDisAuth == 0x31)
					{
						for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
						{
						   aucPrintBuf[ucI]='*';
						}
					}
	//Changed 08-12-03
				}
			}
			else if (DataSave0.ConstantParamData.ucPBeforeSix==0x31)
			{
				if (DataSave0.ConstantParamData.ucCardtype == 0x30)
				{
					if (G_NORMALTRANS_euCardSpecies==CARDSPECIES_MAG)
					{
						if (DataSave0.ConstantParamData.ucDisAuth == 0x30)
						{
							if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
							{
								util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
								for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
								{
									aucPrintBuf[ucI]='*';
								}
							}
						}
						else if (DataSave0.ConstantParamData.ucDisAuth == 0x31)
						{
							util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
							for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
					}
	//Changed 08-12-03
				}
				else if (DataSave0.ConstantParamData.ucCardtype == 0x31)
				{
	//Changed 08-12-10
					if (G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV
						|| DataSave0.ConstantParamData.ucForeignCard==0x31)
					{
						if (DataSave0.ConstantParamData.ucDisAuth == 0x30)
						{
							if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
							{
								util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
								for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
								{
									aucPrintBuf[ucI]='*';
								}
							}
						}
						else if (DataSave0.ConstantParamData.ucDisAuth == 0x31)
						{
							util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
							for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
					}
	//End
				}
				else if (DataSave0.ConstantParamData.ucCardtype == 0x32)
				{
					if  (DataSave0.ConstantParamData.ucDisAuth == 0x31)
					{
						util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
						for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
						{
							aucPrintBuf[ucI]='*';
						}
					}
					else if  (DataSave0.ConstantParamData.ucDisAuth == 0x30)
					{
						if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
						{
							util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
							for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
					}
				}
			}
		}
	}
	memcpy(aucPrintBuf+G_NORMALTRANS_ucSourceAccLen,"  ",2);
	aucPrintBuf[strlen((char*)aucPrintBuf)]=PRINT_GetReadCardMethord();
	PRINT_GB2312_xprint(aucPrintBuf,0x1E);

	if(G_NORMALTRANS_ucTransType==TRANS_EC_UNASSIGNLOAD)
	{
		PRINT_GB2312_xprint((unsigned char*)"转出方卡号:",0x1D);
		memset(aucPrintBuf2,0,sizeof(aucPrintBuf2));
		memcpy(aucPrintBuf2,G_NORMALTRANS_aucUnAssignAccountISO2,G_NORMALTRANS_ucMainAccountLen);
		if(DataSave0.ConstantParamData.ucPrintCardNo==0x31)
			memset(&aucPrintBuf2[6],'*',G_NORMALTRANS_ucMainAccountLen-6-4);
		PRINT_GB2312_xprint(aucPrintBuf2,0x1E);
	}

	//交易类型
	PRINT_transType_CN(0,0x01);

    memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
    memset(aucPrintBuf,0x20,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"有效期:",7);
	if(memcmp(G_NORMALTRANS_ucExpiredDate,"\x00\x00\x00\x00",4))
	{
		memcpy(&aucPrintBuf[7],"20",2);
		bcd_asc(&aucPrintBuf[9],G_NORMALTRANS_ucExpiredDate,2);
		aucPrintBuf[11] = '/';
		bcd_asc(&aucPrintBuf[12],&G_NORMALTRANS_ucExpiredDate[1],2);
	}
	//订单编号
	if (DataSave0.ConstantParamData.ucORDERFLAG == '1')
	{
		if (G_NORMALTRANS_ucTransType == TRANS_PURCHASE
			||G_NORMALTRANS_ucTransType == TRANS_OFFPURCHASE
			|| G_NORMALTRANS_ucTransType == TRANS_MOBILEAUTH)
		{
			memcpy(&aucPrintBuf[29],"订单号:",7);
			memcpy(&aucPrintBuf[36],G_NORMALTRANS_aucOrdersNo,10);
		}
	}
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);

	memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"批次号:",7);
	long_asc(&aucPrintBuf[7],6,&DataSave0.ChangeParamData.ulBatchNumber);
	memcpy(&aucPrintBuf[29],"凭证号:",7);
	long_asc(&aucPrintBuf[36],6,&G_NORMALTRANS_ulTraceNumber);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"日  期:",7);
	UTIL_format_date_bcd_str(&aucPrintBuf[7],G_NORMALTRANS_aucDate);
	memcpy(&aucPrintBuf[29],"时  间:",7);
	UTIL_format_time_bcd_str(&aucPrintBuf[36],G_NORMALTRANS_aucTime);
	aucPrintBuf[44] = 0;
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);

	//AUTH.NO & REF.NO
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"授权号:",7);
	if (G_NORMALTRANS_aucAuthCode[0])
		memcpy(&aucPrintBuf[7],G_NORMALTRANS_aucAuthCode,6);
	memcpy(&aucPrintBuf[29],"参考号:",7);
	if(	(G_NORMALTRANS_ucTransType != TRANS_TIPADJUST)
		&&(G_NORMALTRANS_ucTransType != TRANS_UNDOOFF)
		&&(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)
	  )
	{
		memcpy(&aucPrintBuf[36],G_NORMALTRANS_aucRefNum,TRANS_REFNUMLEN);
	}
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);

	/*print amount*/
	PRINT_xlinefeed(1);
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy( &aucPrintBuf[0], "金额：",6 );
	if(G_NORMALTRANS_ucTransType==TRANS_UNDOOFF
		&&G_NORMALTRANS_ucOldTransType!=TRANS_OFFPREAUTHFINISH
		)
		UTIL_Form_Montant(&aucPrintBuf[11],G_NORMALTRANS_ulFeeAmount,2);
	else
		UTIL_Form_Montant(&aucPrintBuf[11],G_NORMALTRANS_ulAmount,2);

	for(ucI=6;ucI<22;ucI++)
	{
		if(aucPrintBuf[ucI+5] != ' ')
		{
			if(G_NORMALTRANS_ucTransType == TRANS_UNDOOFF
				||G_NORMALTRANS_ucTransType == TRANS_VOIDPURCHASE
				||G_NORMALTRANS_ucTransType == TRANS_VOIDPREAUTH
				||G_NORMALTRANS_ucTransType == TRANS_VOIDCUPMOBILE
				||G_NORMALTRANS_ucTransType == TRANS_VOIDPREAUTHFINISH
				||G_NORMALTRANS_ucTransType == TRANS_VOIDMOBILEAUTH
				||G_NORMALTRANS_ucTransType == TRANS_CREDITSVOID
				)
				memcpy(&aucPrintBuf[ucI],"RMB -",5);
			else
				memcpy(&aucPrintBuf[ucI],"RMB",3);
			break;
		}
	}
	PRINT_GB2312_xprint(aucPrintBuf,0x1E);
	
	
	
	//---------Auth add total amount-------
	if(G_NORMALTRANS_ucTransType == TRANS_PREAUTHADD)
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memset(aucPrintBuf,' ',PRINT_MAXCHAR);
		memset(aucTotalAmtBuf,0,sizeof(aucTotalAmtBuf));
		memset(aucTotalAmtBuf,' ',PRINT_MAXCHAR);

		memcpy( &aucTotalAmtBuf[0], "累计金额：",10 );
		UTIL_Form_Montant(&aucPrintBuf[3],G_NORMALTRANS_ulAuthAddTotalAmount,2);
		for(ucI=0;ucI<11;ucI++)
		{
			if(aucPrintBuf[ucI+3] != ' ')
			{
				memcpy(&aucPrintBuf[ucI],"RMB",3);
				break;
			}
		}
		memcpy(  &aucTotalAmtBuf[10] ,&aucPrintBuf[0],sizeof(aucPrintBuf) );
		PRINT_GB2312_xprint(aucTotalAmtBuf,0x1D);
		PRINT_xlinefeed(1*LINENUM);
	}

	if(G_NORMALTRANS_ucTransType==TRANS_TIPADJUST
		||G_NORMALTRANS_ucTransType==TRANS_TIPADJUSTOK)
	{
	//-------小费-------
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[PRINT_MAXCHAR] = '\0';
		if(G_NORMALTRANS_ulFeeAmount == 0)
		{
			PRINT_xprint( aucPrintBuf );
		}else
		{
			memcpy( &aucPrintBuf[0], "小 费:",11 );
			ulAmount =  G_NORMALTRANS_ulFeeAmount;
			UTIL_Form_Montant(&aucPrintBuf[18],ulAmount,2);

			for(ucI=13;ucI<29;ucI++)
			{
				if(aucPrintBuf[ucI+5] != ' ')
				{
					memcpy(&aucPrintBuf[ucI],"RMB",3);
					break;
				}
			}
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
		}

		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memset(aucPrintBuf,' ',48) ;
		memcpy( &aucPrintBuf[0], "总 计:",12 );
		ulAmount = G_NORMALTRANS_ulAmount + G_NORMALTRANS_ulFeeAmount;
		UTIL_Form_Montant(&aucPrintBuf[18],ulAmount,2);
		for(ucI=13;ucI<29;ucI++)
		{
			if(aucPrintBuf[ucI+5] != ' ')
			{
				memcpy(&aucPrintBuf[ucI],"RMB",3);
				break;
			}
		}
		PRINT_GB2312_xprint(aucPrintBuf,0x1D);
	}
	else
	{
		if(DataSave0.ConstantParamData.ucTerminalType == RESTAURANT)
		{
			PRINT_GB2312_xprint((unsigned char*)"小 费:",0x1D);
			PRINT_GB2312_xprint((unsigned char*)"总 计:",0x1D);
		}
	}
        if(G_NORMALTRANS_ucECTrans ==TRANS_EC_PURCHASE)
        {
            PRINT_xlinefeed(1);
            memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
            memcpy( &aucPrintBuf[0], "电子现金余额:",13 );
            PRINT_GB2312_xprint(aucPrintBuf,0x1E);
            PRINT_GB2312_xprint((unsigned char*)"(OFFLINE BALANCE)",0x01);
            memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
            UTIL_Form_Montant(&aucPrintBuf[11],G_NORMALTRANS_PECTransInfo.uiNBalance,2);
            for(ucI=6;ucI<22;ucI++)
            {
                if(aucPrintBuf[ucI+5] != ' ')
                {
                    memcpy(&aucPrintBuf[ucI],"RMB",3);
                    break;
                }
            }
            PRINT_GB2312_xprint(aucPrintBuf,0x1E);
        }
	/*备注*/
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	PRINT_GB2312_xprint((unsigned char*)"备  注:",0x1D);
	
	PRINT_InstallmentInfor(0x1D);
	
	if ((G_RUNDATA_ucPTicketFlag==1||G_NORMALTRANS_ucPTicketFlag==1)
	    && G_NORMALTRANS_ucTransType==TRANS_OFFPURCHASE)
	    PRINT_GB2312_xprint((unsigned char*)"此卡已开发票",0x1E);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	switch( G_NORMALTRANS_ucTransType )
	{
		case TRANS_MOBILEAUTH:
			strcpy((char*)aucPrintBuf,"手机号码:");
			memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],G_NORMALTRANS_aucMobileNUM,3);
			memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],"*****",5);
			memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],&G_NORMALTRANS_aucMobileNUM[8],3);
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			break;
		case TRANS_UNDOOFF:
		case TRANS_CREDITSVOID:
		case TRANS_VOIDPURCHASE:
		case TRANS_VOIDMOBILEAUTH:
		case TRANS_VOIDPREAUTHFINISH:
			strcpy((char*)aucPrintBuf,"原凭证号: ");
			long_asc(&aucPrintBuf[strlen((char*)aucPrintBuf)],6,&G_NORMALTRANS_ulOldTraceNumber);
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			if (DataSave0.ConstantParamData.ucORDERFLAG == 0x31)
			{
				if (G_NORMALTRANS_ucTransType == TRANS_VOIDPURCHASE
					||G_NORMALTRANS_ucTransType==TRANS_VOIDMOBILEAUTH)
				{
					memset(aucPrintBuf,0,sizeof(aucPrintBuf));
					strcpy((char*)aucPrintBuf,"原订单号: ");
					memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],G_NORMALTRANS_aucOldOrdersNo,TRANS_ORDERSNO);
					PRINT_GB2312_xprint(aucPrintBuf,0x1D);
				}
			}
			if (G_NORMALTRANS_ucTransType==TRANS_VOIDMOBILEAUTH)
			{
				memset(aucPrintBuf,0,sizeof(aucPrintBuf));
				strcpy((char*)aucPrintBuf,"手机号码(MOBILE NO.):");
				memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],G_NORMALTRANS_aucMobileNUM,3);
				memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],"*****",5);
				memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],&G_NORMALTRANS_aucMobileNUM[8],3);
				PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			}
			break;
		case TRANS_REFUND:
			strcpy((char*)aucPrintBuf,"原参考号: ");
			memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],G_NORMALTRANS_aucOldRefNumber,TRANS_REFNUMLEN);
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			break;
		case TRANS_PREAUTHFINISH:
			strcpy((char*)aucPrintBuf,"原授权码: ");
			memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],G_NORMALTRANS_aucOldAuthCode,TRANS_AUTHCODELEN);
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			break;
	}
	if(DataSave0.ChangeParamData.ucSuperLogonFlag==CASH_LOGONFLAG)
	{
		PRINT_GB2312_xprint((unsigned char *)"测 试",0x1D);
	}

	//打印相关银联广告信息
	PRINT_remark();

	if(RE_PRINT_FLAG==(flag&RE_PRINT_FLAG))
		PRINT_GB2312_xprint((unsigned char *)"重打印",0x1D);

	if(G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV&&
	  (TRANS_PURCHASE==G_NORMALTRANS_ucTransType||
	   TRANS_OFFPURCHASE==G_NORMALTRANS_ucTransType||
	   TRANS_PREAUTH==G_NORMALTRANS_ucTransType||
	   TRANS_EC_CASHLOAD ==G_NORMALTRANS_ucTransType||
	   TRANS_EC_UNASSIGNLOAD ==G_NORMALTRANS_ucTransType||
	   TRANS_EC_ASSIGNLOAD ==G_NORMALTRANS_ucTransType
	   )
	)
	{
		memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
		memset(aucPrintBuf,0x20,sizeof(aucPrintBuf)-1);
		strcpy((char*)aucPrintBuf,"TC:");
		hex_asc(aucPrintBuf+strlen((char*)aucPrintBuf),G_NORMALTRANS_AppCrypt,sizeof(G_NORMALTRANS_AppCrypt)*2);
		memcpy(&aucPrintBuf[26],"AID:",4);
		hex_asc(&aucPrintBuf[30],G_NORMALTRANS_aucAID,G_NORMALTRANS_ucAIDLen*2);
		util_Printf("%s\n",aucPrintBuf);
		PRINT_GB2312_xprint(aucPrintBuf,0x01);

		memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
		memset(aucPrintBuf,0x20,sizeof(aucPrintBuf)-1);
		strcpy((char*)aucPrintBuf,"TVR:");
		hex_asc(aucPrintBuf+strlen((char*)aucPrintBuf),G_NORMALTRANS_TVR,sizeof(G_NORMALTRANS_TVR)*2);
		memcpy(&aucPrintBuf[15],"TSI:",4);
		hex_asc(&aucPrintBuf[19],G_NORMALTRANS_TSI,sizeof(G_NORMALTRANS_TSI)*2);
		strcpy((char*)&aucPrintBuf[15+4+2+5],"ATC:");
		hex_asc(&aucPrintBuf[15+4+2+5+4],G_NORMALTRNAS_ATC,sizeof(G_NORMALTRNAS_ATC)*2);
		util_Printf("%s\n",aucPrintBuf);
		PRINT_GB2312_xprint(aucPrintBuf,0x01);

		memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
		memset(aucPrintBuf,0x20,sizeof(aucPrintBuf)-1);
		strcpy((char*)aucPrintBuf,"应用标签:");
		memcpy(aucPrintBuf+strlen((char*)aucPrintBuf),G_NORMALTRANS_aucAppLabel,G_NORMALTRANS_ucAppLabelLen);
		memcpy(&aucPrintBuf[23],"首选名称:",9);
		memcpy(&aucPrintBuf[32],G_NORMALTRANS_aucAppPreferName,G_NORMALTRANS_ucAppPreferNameLen);
		util_Printf("%s\n",aucPrintBuf);
		PRINT_GB2312_xprint(aucPrintBuf,0x01);
	}
	if (ucCopy !=1)
	{
		PRINT_GB2312_xprint((unsigned char*)"持卡人签名:",0x1D);
		PRINT_xlinefeed(1*LINENUM);
	}
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	strcpy((char*)aucPrintBuf,"       本人确认以上交易同意记入本卡帐户");
	PRINT_GB2312_xprint(aucPrintBuf,0x01);
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	PRINT_version();
	PRINT_xlinefeed(LINENUM*3);
}


void PRINT_Ticket_HeatPrinter(unsigned char flag,unsigned char ucSaleCopy)
{

#ifdef GUI_PROJECT		
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=2;
	memcpy(ProUiFace.ProToUi.aucLine1,"交易成功",14);
	memcpy(ProUiFace.ProToUi.aucLine2,"正在打印...",13);
if(!g_ThreadFlag)
 sleep(1);
 else{
    sem_post(&binSem1);
    sem_wait(&binSem2);
  }
#endif

	if(DataSave0.ConstantParamData.ucHeatPrintFormat==1) //格式一
	{
		if (DataSave0.ConstantParamData.ucCNOREN == 0x31)
		{
	            if (!memcmp(G_NORMALTRANS_aucAuthGroup,"CUP",TRANS_GROUPLEN))
	            {
	                PRINT_Ticket_HeatPrinter_1_CN(flag,ucSaleCopy);
	            }
	            else
	            {
	                PRINT_Ticket_HeatPrinter_1(flag,ucSaleCopy);
	            }
		}
		else
		{
			PRINT_Ticket_HeatPrinter_1(flag,ucSaleCopy);
		}
	}
}

void PRINT_Ticket_HeatPrinter_1(unsigned char flag,unsigned char ucCopy)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1],aucPrintBuf2[PRINT_MAXCHAR+1];
	unsigned char aucTotalAmtBuf[PRINT_MAXCHAR+1];
	unsigned char ucI;
	unsigned char temp[17];
	unsigned char aucPrintBuf1[PRINT_MAXCHAR+1];
	unsigned long ulAmount;
	unsigned int uiPrintLen;
#ifdef GUI_PROJECT
	util_Printf("\n-----------------------------------------\n0.PRINT_Ticket_HeatPrinter_1\n1.    China Unipay PBOC2.0");
	util_Printf("\n2.    MERCH ID:%s",DataSave0.ConstantParamData.aucMerchantID);
	util_Printf("\n3.    TERM  ID:%s",DataSave0.ConstantParamData.aucTerminalID);
	util_Printf("\n4.    BatchNumber:%6d",DataSave0.ChangeParamData.ulBatchNumber);
	util_Printf("\n5.    TraceNumber:%6d",G_NORMALTRANS_ulTraceNumber);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	if(G_NORMALTRANS_ucSourceAccLen%2)
		bcd_asc(aucPrintBuf,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
	else
		bcd_asc(aucPrintBuf,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
	util_Printf("\n6.    CARD NUMBER:%s",aucPrintBuf);

	memcpy(&aucPrintBuf[0],"20",2);
	bcd_asc(&aucPrintBuf[2],G_NORMALTRANS_ucExpiredDate,2);
	aucPrintBuf[4] = '/';
	bcd_asc(&aucPrintBuf[5],&G_NORMALTRANS_ucExpiredDate[1],2);
	util_Printf("\n7.    ExpiredDate:%s",aucPrintBuf);
	UTIL_Form_Montant(&aucPrintBuf[0],G_NORMALTRANS_ulAmount,2);
	util_Printf("\n8.    Amount:%s",aucPrintBuf);
	util_Printf("\n9.    AuthCode:%d",G_NORMALTRANS_aucAuthCode);
	util_Printf("\n10    RefNum:%6d\n-----------------------------------------\n",G_NORMALTRANS_aucRefNum);
	
	return ;
#endif

	PRINT_GB2312_xprint((unsigned char *)"    中国银联POS签购单",0x1E);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memset(aucPrintBuf1,' ',sizeof(aucPrintBuf1));
	if(ucCopy == 0)
	{
		memcpy(&aucPrintBuf[0],"商户存根",8);
		memcpy(&aucPrintBuf[32],"请妥善保管",10);
		memcpy(aucPrintBuf1,"MERCHANT COPY",13);
	}
	else if (ucCopy ==1)
	{
		memcpy(&aucPrintBuf[0],"持卡人存根",10);
		memcpy(&aucPrintBuf[32],"请妥善保管",10);
		memcpy(aucPrintBuf1,"CARDHOLDER COPY",15);
	}
	if(ucCopy != 2)
	{
		PRINT_GB2312_xprint(aucPrintBuf,0x01);
		memcpy(&aucPrintBuf1[29],"KEEP IT CAREFULLY",17);
		PRINT_GB2312_xprint(aucPrintBuf1,0x01);
	}
	if (ucCopy == 2)
	{
		PRINT_GB2312_xprint((unsigned char *)"    中国银联POS签购单",0x1E);
	}
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户名称:",9);

	uiPrintLen = strlen((char*)DataSave0.ConstantParamData.aucMerchantName);
	if(uiPrintLen>40) uiPrintLen =40;
	memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucMerchantName,uiPrintLen);
	PrintGB2312Info(aucPrintBuf,uiPrintLen+9,0x01);
	PRINT_GB2312_xprint((unsigned char *)"MERCHANT NAME:",0x01);

	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户编号:",9);
	memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
	memcpy(&aucPrintBuf[29],"操作员:",7);
	memcpy(&aucPrintBuf[36],DataSave0.Cashier_SysCashier_Data.aucCashierNo[DataSave0.ChangeParamData.ucCashierLogonIndex],
				CASH_CASHIERNOLEN);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);
	PRINT_GB2312_xprint((unsigned char *)"MERCHANT NO.:                OPERATOR:",0x01);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"终端编号:",9);
	memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
	memcpy(&aucPrintBuf[29],"清算日期:",9);
	memset(temp,0,sizeof(temp));
	memcpy(temp,G_NORMALTRANS_aucSettleDate,2);
	memcpy(&temp[2],"/",1);
	memcpy(&temp[3],&G_NORMALTRANS_aucSettleDate[2],2);
	memcpy(&aucPrintBuf[38],temp,5);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);
	PRINT_GB2312_xprint((unsigned char *)"TERMINAL NO.:                LIQUIDATION DATE:",0x01);

    /* Issuer ID */
    memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf1,0,sizeof(aucPrintBuf1));
	memset(aucPrintBuf1,0x20,sizeof(aucPrintBuf1)-1);
	if(G_NORMALTRANS_ucCardType==TRANS_CARDTYPE_INTERNAL)
	{
		memset(temp,0,sizeof(temp));
		memcpy(temp,G_NORMALTRANS_aucAcquereID,8);
		if (!memcmp(temp,"0100",4))
		{
			memcpy( aucPrintBuf1,"发卡行(ISS):" ,12);
			GetINCardName(1,G_NORMALTRANS_aucIssuerID,aucPrintBuf);
			memcpy(&aucPrintBuf1[12],aucPrintBuf,strlen((char *)aucPrintBuf));
			PRINT_GB2312_xprint(aucPrintBuf1,0x1D);

			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memset(aucPrintBuf1,0,sizeof(aucPrintBuf1));
			memset(aucPrintBuf1,0x20,sizeof(aucPrintBuf1)-1);
			/* Acquere ID  */
			memcpy(&aucPrintBuf1[0],"收单行(ACQ):" ,12);

			GetINCardName(2,G_NORMALTRANS_aucAcquereID,aucPrintBuf);			
			memcpy(&aucPrintBuf1[12],aucPrintBuf,strlen((char *)aucPrintBuf));
			PRINT_GB2312_xprint(aucPrintBuf1,0x1D);
		}else
		{
			memcpy( aucPrintBuf1,"发卡行:" ,7);
			GetINCardName(1,G_NORMALTRANS_aucIssuerID,aucPrintBuf);
			memcpy(&aucPrintBuf1[7],aucPrintBuf,strlen((char *)aucPrintBuf));

			/* Acquere ID  */
			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memcpy(&aucPrintBuf1[29],"收单行:" ,7);
			memcpy(temp,G_NORMALTRANS_aucAcquereID,8);
			GetINCardName(2,G_NORMALTRANS_aucAcquereID,aucPrintBuf);
			if(strlen((char *)aucPrintBuf))
				memcpy(&aucPrintBuf1[36],aucPrintBuf,strlen((char *)aucPrintBuf));
			else
				memcpy(&aucPrintBuf1[36],G_NORMALTRANS_aucAcquereID ,ACQUERELEN);
			PRINT_GB2312_xprint(aucPrintBuf1,0x1D);
			PRINT_GB2312_xprint((unsigned char *)"ISS:                         ACQ",0x01);
		}
	}else
	{
		/* Acquere ID  */
		memcpy(&aucPrintBuf1[0],"收单行:" ,7);
		memcpy(temp,G_NORMALTRANS_aucAcquereID,8);
		GetINCardName(2,G_NORMALTRANS_aucAcquereID,aucPrintBuf);
		if(strlen((char *)aucPrintBuf))
			memcpy(&aucPrintBuf1[7],aucPrintBuf,strlen((char *)aucPrintBuf));
		else
			memcpy(&aucPrintBuf1[7],G_NORMALTRANS_aucAcquereID ,ACQUERELEN);
		PRINT_GB2312_xprint(aucPrintBuf1,0x1D);
		PRINT_GB2312_xprint((unsigned char *)"ACQ:",0x01);
	}
    if ((DataSave0.ConstantParamData.ucORDERFLAG == '1')&&(DataSave0.ConstantParamData.uclujiaomc=='1')&&((G_NORMALTRANS_ucTransType == TRANS_PURCHASE
			||G_NORMALTRANS_ucTransType == TRANS_OFFPURCHASE || G_NORMALTRANS_ucTransType == TRANS_MOBILEAUTH)||(G_NORMALTRANS_ucTransType == TRANS_VOIDPURCHASE
					||G_NORMALTRANS_ucTransType==TRANS_VOIDMOBILEAUTH)))
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"客户名称:",9);
		uiPrintLen = strlen((char*)NormalTransData.aucCustName);
		if(uiPrintLen>20) uiPrintLen =20;
		memcpy(&aucPrintBuf[9],NormalTransData.aucCustName,uiPrintLen);
		PrintGB2312Info(aucPrintBuf,uiPrintLen+9,0x01);
	}

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	//memset(aucPrintBuf,0x20,sizeof(aucPrintBuf)-1);
	if(G_NORMALTRANS_ucTransType==TRANS_EC_UNASSIGNLOAD)
		PRINT_GB2312_xprint((unsigned char*)"转入方卡号(CARD NUMBER):",0x1D);
	else
		PRINT_GB2312_xprint((unsigned char*)"卡    号(CARD NUMBER):",0x1D);

	if(G_NORMALTRANS_ucSourceAccLen%2)
	{
		bcd_asc(aucPrintBuf,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
	}else
	{
		bcd_asc(aucPrintBuf,G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
	}
	memcpy(&aucPrintBuf[G_NORMALTRANS_ucSourceAccLen],"     ",3);
	if ((G_NORMALTRANS_ucTransType != TRANS_TIPADJUST) &&
		(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)&&
		(G_NORMALTRANS_ucTransType != TRANS_UNDOOFF)&&
		(G_NORMALTRANS_ucTransType != TRANS_EC_UNASSIGNLOAD)&&
		(G_NORMALTRANS_ucTransType != TRANS_OFFPURCHASE)&&
		(G_NORMALTRANS_ucECTrans !=TRANS_EC_PURCHASE)
		)
	{
		if(DataSave0.ConstantParamData.ucPrintCardNo==0x31)
		{
			if (DataSave0.ConstantParamData.ucPBeforeSix==0x30)
			{
				if (DataSave0.ConstantParamData.ucCardtype == 0x30)
				{
	/*Changed 08-12-10*/
					if (G_NORMALTRANS_euCardSpecies==CARDSPECIES_MAG)
					{
						if (DataSave0.ConstantParamData.ucDisAuth == 0x30)
						{
							if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
							{
								for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
								{
								   aucPrintBuf[ucI]='*';
								}
							}
						}
						else if (DataSave0.ConstantParamData.ucDisAuth == 0x31)
						{
							for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
							   aucPrintBuf[ucI]='*';
							}
						}
					}
	/*=End=*/
				}
				else if (DataSave0.ConstantParamData.ucCardtype == 0x31)
				{
	//Changed 08-12-10
					if (G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV
						|| DataSave0.ConstantParamData.ucForeignCard==0x31)
					{
						if (DataSave0.ConstantParamData.ucDisAuth == 0x30)
						{
							if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
							{
								for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
								{
								   aucPrintBuf[ucI]='*';
								}
							}
						}
						else if (DataSave0.ConstantParamData.ucDisAuth == 0x31)
						{
							for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
							   aucPrintBuf[ucI]='*';
							}
						}
					}
	//End
				}
				else if (DataSave0.ConstantParamData.ucCardtype == 0x32)
				{
					if (DataSave0.ConstantParamData.ucDisAuth == 0x30)
					{
						if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
						{
							for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
							   aucPrintBuf[ucI]='*';
							}
						}
					}
					else if (DataSave0.ConstantParamData.ucDisAuth == 0x31)
					{
						for(ucI=6;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
						{
						   aucPrintBuf[ucI]='*';
						}
					}
				}
			}
			else if (DataSave0.ConstantParamData.ucPBeforeSix==0x31)
			{
				if (DataSave0.ConstantParamData.ucCardtype == 0x30)
				{
					if (G_NORMALTRANS_euCardSpecies==CARDSPECIES_MAG)
					{
						if (DataSave0.ConstantParamData.ucDisAuth == 0x30)
						{
							if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
							{
								util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
								for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
								{
									aucPrintBuf[ucI]='*';
								}
							}
						}
						else if (DataSave0.ConstantParamData.ucDisAuth == 0x31)
						{
							util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
							for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
					}
				}
				else if (DataSave0.ConstantParamData.ucCardtype == 0x31)
				{
	//Changed 08-12-10
					if (G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV
						|| DataSave0.ConstantParamData.ucForeignCard==0x31)
					{
						if (DataSave0.ConstantParamData.ucDisAuth == 0x30)
						{
							if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
							{
								util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
								for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
								{
									aucPrintBuf[ucI]='*';
								}
							}
						}
						else if (DataSave0.ConstantParamData.ucDisAuth == 0x31)
						{
							util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
							for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
					}
	//End
				}
				else if (DataSave0.ConstantParamData.ucCardtype == 0x32)
				{
					if  (DataSave0.ConstantParamData.ucDisAuth == 0x31)
					{
						util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
						for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
						{
							aucPrintBuf[ucI]='*';
						}
					}
					else if  (DataSave0.ConstantParamData.ucDisAuth == 0x30)
					{
						if (G_NORMALTRANS_ucTransType != TRANS_PREAUTH)
						{
							util_Printf("Len=%d\n",G_NORMALTRANS_ucSourceAccLen);
							for(ucI=0;ucI<G_NORMALTRANS_ucSourceAccLen-4;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
					}
				}
			}
		}
	}
	memcpy(aucPrintBuf+G_NORMALTRANS_ucSourceAccLen,"  ",2);
	aucPrintBuf[strlen((char*)aucPrintBuf)]=PRINT_GetReadCardMethord();
	PRINT_GB2312_xprint(aucPrintBuf,0x1E);

	if(G_NORMALTRANS_ucTransType==TRANS_EC_UNASSIGNLOAD)
	{
		PRINT_GB2312_xprint((unsigned char*)"转出方卡号(CARD NUMBER.):",0x1D);
		memset(aucPrintBuf2,0,sizeof(aucPrintBuf2));
		memcpy(aucPrintBuf2,G_NORMALTRANS_aucUnAssignAccountISO2,G_NORMALTRANS_ucMainAccountLen);
		if(DataSave0.ConstantParamData.ucPrintCardNo==0x31)
			memset(&aucPrintBuf2[6],'*',G_NORMALTRANS_ucMainAccountLen-6-4);
		PRINT_GB2312_xprint(aucPrintBuf2,0x1E);
	}

	memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
	memset(aucPrintBuf,0x20,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"有效期:",7);
	if(memcmp(G_NORMALTRANS_ucExpiredDate,"\x00\x00\x00\x00",4))
	{
		memcpy(&aucPrintBuf[7],"20",2);
		bcd_asc(&aucPrintBuf[9],G_NORMALTRANS_ucExpiredDate,2);
		aucPrintBuf[11] = '/';
		bcd_asc(&aucPrintBuf[12],&G_NORMALTRANS_ucExpiredDate[1],2);
	}
	//订单编号
	if (DataSave0.ConstantParamData.ucORDERFLAG == '1')
	{
		if (G_NORMALTRANS_ucTransType == TRANS_PURCHASE
			||G_NORMALTRANS_ucTransType == TRANS_OFFPURCHASE
			|| G_NORMALTRANS_ucTransType == TRANS_MOBILEAUTH)
		{
			memcpy(&aucPrintBuf[29],"订单号:",7);
			memcpy(&aucPrintBuf[36],G_NORMALTRANS_aucOrdersNo,10);
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			PRINT_GB2312_xprint((unsigned char *)"EXP DATE:                    ORDERS NO.:",0x01);
		}
		else
		{
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			PRINT_GB2312_xprint((unsigned char *)"EXP DATE:",0x01);
		}
	}
	else
	{
		PRINT_GB2312_xprint(aucPrintBuf,0x1D);
		PRINT_GB2312_xprint((unsigned char *)"EXP DATE:",0x01);
	}
	//交易类型
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memset(aucPrintBuf1,' ',sizeof(aucPrintBuf1));
	PRINT_transType_NEW(aucPrintBuf,aucPrintBuf1);
	memset(&aucPrintBuf[strlen((char *)aucPrintBuf)],' ',PRINT_MAXCHAR+1-strlen((char *)aucPrintBuf));
	
	memcpy(&aucPrintBuf[strlen((char *)aucPrintBuf)+1],aucPrintBuf1,strlen((char *)aucPrintBuf1));
	PRINT_GB2312_xprint(aucPrintBuf,0x01);
	//PRINT_GB2312_xprint(aucPrintBuf1,0x01);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"批次号:",7);
	long_asc(&aucPrintBuf[7],6,&DataSave0.ChangeParamData.ulBatchNumber);
	memcpy(&aucPrintBuf[29],"凭证号:",7);
	long_asc(&aucPrintBuf[36],6,&G_NORMALTRANS_ulTraceNumber);
	PRINT_GB2312_xprint(aucPrintBuf,0X1D);

	PRINT_GB2312_xprint((unsigned char *)"BATCH NO.:                   VOUCHER NO.:",0x01);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"日  期:",7);
	UTIL_format_date_bcd_str(&aucPrintBuf[7],G_NORMALTRANS_aucDate);
	memcpy(&aucPrintBuf[29],"时  间:",7);
	UTIL_format_time_bcd_str(&aucPrintBuf[36],G_NORMALTRANS_aucTime);
	PRINT_GB2312_xprint(aucPrintBuf,0X1d);

	PRINT_GB2312_xprint((unsigned char *)"DATA:                        TIME:",0x01);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"授权号:",7);
	if (G_NORMALTRANS_aucAuthCode[0])
		memcpy(&aucPrintBuf[7],G_NORMALTRANS_aucAuthCode,6);

	memcpy(&aucPrintBuf[29],(unsigned char *)"参考号:",7);
	if(	(G_NORMALTRANS_ucTransType != TRANS_TIPADJUST)
		&&(G_NORMALTRANS_ucTransType != TRANS_UNDOOFF)
		&&(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)
	  )
	{
		memcpy(&aucPrintBuf[36],G_NORMALTRANS_aucRefNum,TRANS_REFNUMLEN);
	}
	PRINT_GB2312_xprint(aucPrintBuf,0X1D);

	PRINT_GB2312_xprint((unsigned char *)"AUTH NO.:                    REF NO.:",0x01);

	/*print amount*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy( &aucPrintBuf[0], "金额：",6 );

	if(G_NORMALTRANS_ucTransType==TRANS_UNDOOFF
		&&G_NORMALTRANS_ucOldTransType!=TRANS_OFFPREAUTHFINISH
		)
		UTIL_Form_Montant(&aucPrintBuf[11],G_NORMALTRANS_ulFeeAmount,2);
	else
		UTIL_Form_Montant(&aucPrintBuf[11],G_NORMALTRANS_ulAmount,2);

	for(ucI=6;ucI<22;ucI++)
	{
		if(aucPrintBuf[ucI+5] != ' ')
		{
			if(G_NORMALTRANS_ucTransType == TRANS_UNDOOFF
				||G_NORMALTRANS_ucTransType == TRANS_VOIDPURCHASE
				||G_NORMALTRANS_ucTransType == TRANS_VOIDPREAUTH
				||G_NORMALTRANS_ucTransType == TRANS_VOIDCUPMOBILE
				||G_NORMALTRANS_ucTransType == TRANS_VOIDPREAUTHFINISH
				||G_NORMALTRANS_ucTransType == TRANS_VOIDMOBILEAUTH
				||G_NORMALTRANS_ucTransType == TRANS_CREDITSVOID
				)
				memcpy(&aucPrintBuf[ucI],"RMB -",5);
			else
				memcpy(&aucPrintBuf[ucI],"RMB",3);
			break;
		}
	}
	PRINT_GB2312_xprint(aucPrintBuf,0x1E);
	PRINT_GB2312_xprint((unsigned char*)"(AMOUNT)",0x01);
		
	//---------Auth add total amount-------
	if(G_NORMALTRANS_ucTransType == TRANS_PREAUTHADD)
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memset(aucPrintBuf,' ',PRINT_MAXCHAR);
		memset(aucTotalAmtBuf,0,sizeof(aucTotalAmtBuf));
		memset(aucTotalAmtBuf,' ',PRINT_MAXCHAR);

		memcpy( &aucTotalAmtBuf[0], "累计金额：",10 );
		util_Printf("G_NORMALTRANS_ulAuthAddTotalAmount = %ld\n",G_NORMALTRANS_ulAuthAddTotalAmount);
		UTIL_Form_Montant(&aucPrintBuf[3],G_NORMALTRANS_ulAuthAddTotalAmount,2);
		for(ucI=0;ucI<11;ucI++)
		{
			if(aucPrintBuf[ucI+3] != ' ')
			{
				memcpy(&aucPrintBuf[ucI],"RMB",3);
				break;
			}
		}
		memcpy(  &aucTotalAmtBuf[10] ,&aucPrintBuf[0],sizeof(aucPrintBuf) );
		PRINT_GB2312_xprint(aucTotalAmtBuf,0x1D);
		PRINT_GB2312_xprint((unsigned char*)"TOTAL AMOUNT:",0x1D);
		PRINT_xlinefeed(1*LINENUM);
	}

	if(G_NORMALTRANS_ucTransType==TRANS_TIPADJUST
		||G_NORMALTRANS_ucTransType==TRANS_TIPADJUSTOK)
	{
		//-------小费-------
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));

		aucPrintBuf[PRINT_MAXCHAR] = '\0';
		if(G_NORMALTRANS_ulFeeAmount == 0)
		{
			PRINT_xprint( aucPrintBuf );
		}else
		{
			memcpy( &aucPrintBuf[0], "小费(TIPS):",11 );
			ulAmount =  G_NORMALTRANS_ulFeeAmount;
			UTIL_Form_Montant(&aucPrintBuf[18],ulAmount,2);

			for(ucI=13;ucI<29;ucI++)
			{
				if(aucPrintBuf[ucI+5] != ' ')
				{
					memcpy(&aucPrintBuf[ucI],"RMB",3);
					break;
				}
			}
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
		}
		//-------TOTAL-------------
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memset(aucPrintBuf,' ',48) ;
		memcpy( &aucPrintBuf[0], "总计(TOTAL):",12 );
		ulAmount = G_NORMALTRANS_ulAmount + G_NORMALTRANS_ulFeeAmount;
		util_Printf("total = %ld\n",ulAmount);
		UTIL_Form_Montant(&aucPrintBuf[18],ulAmount,2);

		for(ucI=13;ucI<29;ucI++)
		{
			if(aucPrintBuf[ucI+5] != ' ')
			{
				memcpy(&aucPrintBuf[ucI],"RMB",3);
				break;
			}
		}
		PRINT_GB2312_xprint(aucPrintBuf,0X1D);
	}
	else
	{
		if(DataSave0.ConstantParamData.ucTerminalType == RESTAURANT)
		{
			PRINT_GB2312_xprint((unsigned char*)"小费(TIPS):",0x1D);
			PRINT_GB2312_xprint((unsigned char*)"总计(TOTAL):",0x1D);
		}
	}
	
	if(G_NORMALTRANS_ucECTrans ==TRANS_EC_PURCHASE)
	{
		PRINT_xlinefeed(1);
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memcpy( &aucPrintBuf[0], "电子现金余额:",13 );
		PRINT_GB2312_xprint(aucPrintBuf,0x1E);
		PRINT_GB2312_xprint((unsigned char*)"(OFFLINE BALANCE)",0x01);
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		UTIL_Form_Montant(&aucPrintBuf[11],G_NORMALTRANS_PECTransInfo.uiNBalance,2);
		for(ucI=6;ucI<22;ucI++)
		{
			if(aucPrintBuf[ucI+5] != ' ')
			{
				memcpy(&aucPrintBuf[ucI],"RMB",3);
				break;
			}
		}
		PRINT_GB2312_xprint(aucPrintBuf,0x1E);
	}
	/*备注*/
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	PRINT_GB2312_xprint((unsigned char*)"备  注(REFERENCE):",0x1D);
	
	PRINT_InstallmentInfor(0x1D);
	
	if ((G_RUNDATA_ucPTicketFlag==1||G_NORMALTRANS_ucPTicketFlag==1)
	    && G_NORMALTRANS_ucTransType==TRANS_OFFPURCHASE)
	{
		PRINT_GB2312_xprint((unsigned char*)"******此卡已开发票******",0x1E);
	}

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	switch( G_NORMALTRANS_ucTransType )
	{
		case TRANS_MOBILEAUTH:
			strcpy((char*)aucPrintBuf,"手机号码(MOBILE NO.):");
			memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],G_NORMALTRANS_aucMobileNUM,3);
			memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],"*****",5);
			memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],&G_NORMALTRANS_aucMobileNUM[8],3);
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			break;
		case TRANS_UNDOOFF:
		case TRANS_CREDITSVOID:
		case TRANS_VOIDPURCHASE:
		case TRANS_VOIDMOBILEAUTH:
		case TRANS_VOIDPREAUTHFINISH:
			strcpy((char*)aucPrintBuf,"原凭证号(VOUCHER. NO.): ");
			long_asc(&aucPrintBuf[strlen((char*)aucPrintBuf)],6,&G_NORMALTRANS_ulOldTraceNumber);
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			if (DataSave0.ConstantParamData.ucORDERFLAG == 0x31)
			{
				if (G_NORMALTRANS_ucTransType == TRANS_VOIDPURCHASE)
				{
					memset(aucPrintBuf,0,sizeof(aucPrintBuf));
					strcpy((char*)aucPrintBuf,"原订单号(ORDER. NO.): ");
					memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],G_NORMALTRANS_aucOldOrdersNo,TRANS_ORDERSNO);
					PRINT_GB2312_xprint(aucPrintBuf,0x1D);
				}
			}
			if (G_NORMALTRANS_ucTransType==TRANS_VOIDMOBILEAUTH)
			{
				memset(aucPrintBuf,0,sizeof(aucPrintBuf));
				strcpy((char*)aucPrintBuf,"手机号码(MOBILE NO.):");
				memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],G_NORMALTRANS_aucMobileNUM,3);
				memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],"*****",5);
				memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],&G_NORMALTRANS_aucMobileNUM[8],3);
				PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			}
			break;
		case TRANS_REFUND:
			strcpy((char*)aucPrintBuf,"原参考号(REFER. NO.):");
			memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],G_NORMALTRANS_aucOldRefNumber,TRANS_REFNUMLEN);
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			break;
		case TRANS_PREAUTHFINISH:
			strcpy((char*)aucPrintBuf,"原授权码(AUTH. NO.):");
			memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],G_NORMALTRANS_aucOldAuthCode,TRANS_AUTHCODELEN);
			PRINT_GB2312_xprint(aucPrintBuf,0x1D);
			break;
	}
	if(DataSave0.ChangeParamData.ucSuperLogonFlag==CASH_LOGONFLAG)
	{
		PRINT_GB2312_xprint((unsigned char *)"测试(TESTING)",0x1D);
	}

	//打印银联广告信息
	PRINT_remark();

	if(RE_PRINT_FLAG==(flag&RE_PRINT_FLAG))
		PRINT_GB2312_xprint((unsigned char *)"重打印(DUPLICATED)",0x1D);

	if(G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV&&
	  (TRANS_PURCHASE==G_NORMALTRANS_ucTransType||
	   TRANS_OFFPURCHASE==G_NORMALTRANS_ucTransType||
	   TRANS_PREAUTH==G_NORMALTRANS_ucTransType||
	   TRANS_EC_CASHLOAD ==G_NORMALTRANS_ucTransType||
	   TRANS_EC_UNASSIGNLOAD ==G_NORMALTRANS_ucTransType||
	   TRANS_EC_ASSIGNLOAD ==G_NORMALTRANS_ucTransType)
	)
	{
		memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
		memset(aucPrintBuf,0x20,sizeof(aucPrintBuf)-1);
		strcpy((char*)aucPrintBuf,"TC:");
		hex_asc(aucPrintBuf+strlen((char*)aucPrintBuf),G_NORMALTRANS_AppCrypt,sizeof(G_NORMALTRANS_AppCrypt)*2);
		memcpy(&aucPrintBuf[26],"AID:",4);
		hex_asc(&aucPrintBuf[30],G_NORMALTRANS_aucAID,G_NORMALTRANS_ucAIDLen*2);
		util_Printf("%s\n",aucPrintBuf);
		PRINT_GB2312_xprint(aucPrintBuf,0x1D);

		memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
		memset(aucPrintBuf,0x20,sizeof(aucPrintBuf)-1);
		strcpy((char*)aucPrintBuf,"TVR:");
		hex_asc(aucPrintBuf+strlen((char*)aucPrintBuf),G_NORMALTRANS_TVR,sizeof(G_NORMALTRANS_TVR)*2);
		memcpy(&aucPrintBuf[15],"TSI:",4);
		hex_asc(&aucPrintBuf[19],G_NORMALTRANS_TSI,sizeof(G_NORMALTRANS_TSI)*2);
		strcpy((char*)&aucPrintBuf[15+4+2+5],"ATC:");
		hex_asc(&aucPrintBuf[15+4+2+5+4],G_NORMALTRNAS_ATC,sizeof(G_NORMALTRNAS_ATC)*2);
		util_Printf("%s\n",aucPrintBuf);
		PRINT_GB2312_xprint(aucPrintBuf,0x1D);

		memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
		memset(aucPrintBuf,0x20,sizeof(aucPrintBuf)-1);
		strcpy((char*)aucPrintBuf,"应用标签:");
		memcpy(aucPrintBuf+strlen((char*)aucPrintBuf),G_NORMALTRANS_aucAppLabel,G_NORMALTRANS_ucAppLabelLen);
		memcpy(&aucPrintBuf[23],"首选名称:",9);
		memcpy(&aucPrintBuf[32],G_NORMALTRANS_aucAppPreferName,G_NORMALTRANS_ucAppPreferNameLen);
		util_Printf("%s\n",aucPrintBuf);
		PRINT_GB2312_xprint(aucPrintBuf,0x1D);
	}

	if (ucCopy !=1)
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"持卡人签名",10);
		memcpy(&aucPrintBuf[11],"(CARDHOLDER SIGNATURE):",24);
		PRINT_GB2312_xprint(aucPrintBuf,0x1D);
		PRINT_xlinefeed(3*LINENUM);
	}

	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	strcpy((char*)aucPrintBuf,"       本人确认以上交易同意记入本卡帐户");
	PRINT_GB2312_xprint(aucPrintBuf,0x01);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	strcpy((char*)aucPrintBuf,"       I ACKNOWLEDGE SATISFACTORY RECEIPT ");
	PRINT_GB2312_xprint(aucPrintBuf,0x01);
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	strcpy((char*)aucPrintBuf,"           OF RELATIVE GOODS/SERVICES");
	PRINT_GB2312_xprint(aucPrintBuf,0x01);
	PRINT_version();
	PRINT_xlinefeed(3*LINENUM);
}

void PRINT_List(unsigned char flag)
{
	unsigned char aucBuf[40];
	unsigned char aucPrint[40];
	unsigned char aucPrintBuf[40];
	unsigned char aucdate[4];
	unsigned char auctime[4];
	unsigned long ultotalamount;
	unsigned int uitotalnb;
	unsigned long ulBatchNum;
	TRANSTOTAL	*pTransTotal;
	TRANSTOTAL	*pForeignTransTotal;
	UCHAR ucKey;
	unsigned int uiPrintLen;

	//Os__clr_display(255);
	//Os__GB2312_display(1, 0, (uchar * )"正在打印请稍等");
	Os__light_off();
	if((flag&PRINT_TOTAL_FLAG)==PRINT_TOTAL_FLAG)
	{
		PRINT_GB2312_xprint((unsigned char*)"  终端统计TERM TOTAL",0x1E);
	}
	else
	if((flag&PRINT_SETTLE_FLAG)==PRINT_SETTLE_FLAG)
	{
		//PRINT_XReverseFeed(LINENUM*1);
		PRINT_GB2312_xprint((unsigned char*)"    POS结算单",0x1E);
	}

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户名称:",9);
	uiPrintLen = strlen((char*)DataSave0.ConstantParamData.aucMerchantName);
	if(uiPrintLen>40) uiPrintLen =40;
	memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucMerchantName,uiPrintLen);

	util_Printf("打印机:%02x\n",DataSave0.ConstantParamData.prnflag);

	if (DataSave0.ConstantParamData.prnflagOld
		||DataSave0.ConstantParamData.prnflagNew
		)
		PrintGB2312Info(aucPrintBuf,uiPrintLen+9,0x03);
	if (DataSave0.ConstantParamData.prnflagZD)
		PrintGB2312Info(aucPrintBuf,uiPrintLen+9,0x02);
	if (DataSave0.ConstantParamData.prnflag)
		PrintGB2312Info(aucPrintBuf,uiPrintLen+9,0x01);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户号:  ",8);
	memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"终端号:",7);
	memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);

	/*print issuer id*/
	memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
	strcpy((char*)aucPrintBuf,"收单行号:");
	memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],DataSave0.ChangeParamData.aucAcquereID,8);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);

	/*print batch number*/
	memset(aucPrintBuf,'0',sizeof(aucPrintBuf));
	ulBatchNum = DataSave0.ChangeParamData.ulBatchNumber;

	long_asc(aucPrintBuf,6,&ulBatchNum);
	PRINT_PrintTwoMSG( FONT,MSG_BATCHNUM,aucPrintBuf,6,-1,0,0);

	if(PRINT_TOTAL_FLAG==(flag&PRINT_TOTAL_FLAG))
	{
		memset(aucBuf,' ',sizeof(aucBuf));
		memset(aucPrint,0,sizeof(aucPrint));
		Os__read_date(aucdate);
		memcpy(&aucBuf[0],&aucdate[4],2);
		memcpy(&aucBuf[2],&aucdate[2],2);
		memcpy(&aucBuf[4],&aucdate[0],2);
		util_Printf("aucBuf(data)=%s",aucBuf);
		aucPrint[0]=0x20;
		asc_bcd(&aucPrint[1],3,aucBuf,6);
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		UTIL_format_date_bcd_str(&aucPrintBuf[0],&aucPrint[0]);

		Os__read_time_sec(auctime);
		util_Printf("auctime(time)=%s\n",auctime);
		memset(aucPrint,0,sizeof(aucPrint));
		asc_bcd(aucPrint,3,auctime,6);
		UTIL_format_time_bcd_str(&aucPrintBuf[15],aucPrint);

		PRINT_GB2312_xprint(aucPrintBuf,FONT);
	}
	else
	{
		memset(aucPrint,0x00,sizeof(aucPrint));
		memcpy( &aucPrint[0],&DataSave0.ChangeParamData.ucSettleData[0],TRANS_DATELEN );
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		aucPrintBuf[25]=0x00;
		memcpy(aucPrintBuf,"日期-时间:",10);
		PRINT_GB2312_xprint(aucPrintBuf,FONT);
		memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
		memset(aucPrintBuf,0x20,sizeof(aucPrintBuf)-1);
		UTIL_format_date_bcd_str(&aucPrintBuf[0],aucPrint);
		aucPrintBuf[26]=0;
		UTIL_format_time_bcd_str(&aucPrintBuf[15],DataSave0.ChangeParamData.ucSettleTime);
		aucPrintBuf[30] = 0;
		PRINT_GB2312_xprint(aucPrintBuf,FONT);
	}

	if(((flag&PRINT_SETTLE_FLAG)==PRINT_SETTLE_FLAG)&&((flag&RE_PRINT_FLAG)==RE_PRINT_FLAG))
	{
		pTransTotal = &DataSave0.TransInfoData.LastTransTotal;
		pForeignTransTotal = &DataSave0.TransInfoData.LastForeignTransTotal;
	}
	else
	{
		pTransTotal = &DataSave0.TransInfoData.TransTotal;
		pForeignTransTotal = &DataSave0.TransInfoData.ForeignTransTotal;
	}

	/****************************************************************/
	if(PRINT_SETTLE_FLAG==(flag&PRINT_SETTLE_FLAG))
	{
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");

		if(PRINT_TOTAL_FLAG!=(flag&PRINT_TOTAL_FLAG))
		{
			if(pTransTotal->ucSettleFlag== SETTLE_OK)
				PRINT_GB2312_xprint((unsigned char *)"内卡对帐平",FONT);
			else
			if(pTransTotal->ucSettleFlag == SETTLE_FAIL)
				PRINT_GB2312_xprint((unsigned char *)"内卡对帐不平",FONT);
			else
			if(pTransTotal->ucSettleFlag == SETTLE_ERROR)
				PRINT_GB2312_xprint((unsigned char *)"内卡对帐错误",FONT);
		}
		PRINT_GB2312_xprint((unsigned char *)"内卡消费",FONT);
		uitotalnb = pTransTotal->uiPurchaseNb;
		ultotalamount = pTransTotal->ulPurchaseAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);
//		//add
//		PRINT_GB2312_xprint((unsigned char *)"内卡预授权",FONT);
//		uitotalnb = pTransTotal->uiAuthNb;
//		ultotalamount = pTransTotal->ulAuthAmount;
//		memset(aucBuf,' ',sizeof(aucBuf));
//		int_asc(aucBuf,3,&uitotalnb);
//		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
//		PRINT_GB2312_xprint(aucBuf,FONT);
//		//end
		PRINT_GB2312_xprint((unsigned char *)"内卡预授权完成(联机)",FONT);
		uitotalnb = pTransTotal->uiAuthFinishNb;
		ultotalamount = pTransTotal->ulAuthFinishAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_GB2312_xprint((unsigned char *)"内卡预授权完成(离线)",FONT);
		uitotalnb = pTransTotal->uiPreauthSettleNb;
		ultotalamount = pTransTotal->ulPreauthSettleAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_GB2312_xprint((unsigned char *)"内卡离线:",FONT);
		uitotalnb = pTransTotal->uiOfflineNb;
		ultotalamount = pTransTotal->ulOfflineAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_GB2312_xprint((unsigned char *)"内卡退货:",FONT);
		uitotalnb = pTransTotal->uiRefundNb;
		ultotalamount = pTransTotal->ulRefundAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

//		PRINT_GB2312_xprint((unsigned char *)"现金圈存:",FONT);
//		uitotalnb = pTransTotal->uiCashLoadNb;
//		ultotalamount = pTransTotal->uiCashLoadAmount;
//		memset(aucBuf,' ',sizeof(aucBuf));
//		int_asc(aucBuf,3,&uitotalnb);
//		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
//		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
		if(PRINT_TOTAL_FLAG!=(flag&PRINT_TOTAL_FLAG))
		{
			if(pForeignTransTotal->ucSettleFlag==SETTLE_OK)
				PRINT_GB2312_xprint((unsigned char *)"外卡对帐平",FONT);
			else
			if(pForeignTransTotal->ucSettleFlag == SETTLE_FAIL)
				PRINT_GB2312_xprint((unsigned char *)"外卡对帐不平",FONT);
			else
			if(pForeignTransTotal->ucSettleFlag == SETTLE_ERROR)
				PRINT_GB2312_xprint((unsigned char *)"外卡对帐错误",FONT);
		}
		PRINT_GB2312_xprint((unsigned char *)"外卡消费",FONT);
		uitotalnb = pForeignTransTotal->uiPurchaseNb;
		ultotalamount = pForeignTransTotal->ulPurchaseAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);
//		//add
//		PRINT_GB2312_xprint((unsigned char *)"外卡预授权",FONT);
//		uitotalnb = pForeignTransTotal->uiAuthNb;
//		ultotalamount = pForeignTransTotal->ulAuthAmount;
//		memset(aucBuf,' ',sizeof(aucBuf));
//		int_asc(aucBuf,3,&uitotalnb);
//		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
//		PRINT_GB2312_xprint(aucBuf,FONT);
//		//end
		PRINT_GB2312_xprint((unsigned char *)"外卡预授权完成(联机)",FONT);
		uitotalnb = pForeignTransTotal->uiAuthFinishNb;
		ultotalamount = pForeignTransTotal->ulAuthFinishAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_GB2312_xprint((unsigned char *)"外卡预授权完成(离线)",FONT);
		uitotalnb = pForeignTransTotal->uiPreauthSettleNb;
		ultotalamount = pForeignTransTotal->ulPreauthSettleAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_GB2312_xprint((unsigned char *)"外卡离线:",FONT);
		uitotalnb = pForeignTransTotal->uiOfflineNb;
		ultotalamount = pForeignTransTotal->ulOfflineAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	}

       //Print卡明细
	PRINT_EveryCardDetail(flag);
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");


	/*如果是打流水明细，就把每条流水打印出来*/

	if(DataSave0.ChangeParamData.ucSuperLogonFlag==CASH_LOGONFLAG)
	{
		PRINT_GB2312_xprint((unsigned char *)"测试/TESTING",FONT);
	}

	if(DataSave0.ConstantParamData.prnflag)
		PRINT_xlinefeed(1*LINENUM);

	if((RE_PRINT_FLAG!=(flag&RE_PRINT_FLAG))
		&&(PRINT_TOTAL_FLAG!=(flag&PRINT_TOTAL_FLAG)))
	{
		util_Printf("\ntesttPRINT_TOTAL_FLAG[%d]",PRINT_TOTAL_FLAG);
		
		if(PRINT_SETTLE_FLAG==(flag&PRINT_SETTLE_FLAG))
		{
			util_Printf("明细3:%02x\n",DataSave0.ConstantParamData.ucList);
			if (DataSave0.ConstantParamData.ucList == 0x31)
			{
				//PRINT_detail();
				if (DataSave0.ConstantParamData.prnflagZD
					|| DataSave0.ConstantParamData.prnflagOld
					|| DataSave0.ConstantParamData.prnflagNew)
				{
					PRINT_detail_ZD();
				}
				else
				{
					PRINT_detail();
				}
			}else if (DataSave0.ConstantParamData.ucList == 0x30)
			{
					util_Printf("\nG_RUNDATA_ucSettleFlag3:%02x\n",G_RUNDATA_ucSettleFlag);
				if(!G_RUNDATA_ucSettleFlag)
				{
				
					//Os__clr_display(255);
					//Os__GB2312_display(1,0,(uchar*)"   是否打明细");
					//Os__GB2312_display(2,0,(uchar*)"  [确认]键打印");
					//Os__GB2312_display(3,0,(UCHAR*)"  [取消]键退出");
					//ucKey=Os__xget_key();						
						ucKey = KEY_CLEAR;
					if(ucKey==KEY_ENTER)
					{
						//Os__clr_display(255);
						//Os__GB2312_display(1,0,(uchar*)"   请稍等....");
						//Os__GB2312_display(2,0,(uchar*)"正在打印交易明细");
						//PRINT_detail();
						if (DataSave0.ConstantParamData.prnflagZD
							|| DataSave0.ConstantParamData.prnflagOld
							|| DataSave0.ConstantParamData.prnflagNew)
						{
							PRINT_detail_ZD();
						}
						else
						{
							PRINT_detail();
						}
					}
				}else
				{
					if(DataSave0.ConstantParamData.prnflagNew
					||DataSave0.ConstantParamData.prnflagOld)
					{
						util_Printf("结算单==================\n");
						#if PS300
						util_Printf("\nTESTING");
						PRINT_xlinefeed(7*LINENUM);
						#endif
					}
				}
			}
		}
	}
	if(DataSave0.ConstantParamData.prnflagOld)
	{
		if(((flag&PRINT_TOTAL_FLAG)==PRINT_TOTAL_FLAG))
			PRINT_xlinefeed(9.5*LINENUM);
		else if(((flag&PRINT_SETTLE_FLAG)==PRINT_SETTLE_FLAG))
			PRINT_xlinefeed(2.5*LINENUM);
	}

	if(DataSave0.ConstantParamData.prnflagZD)
		PRINT_xlinefeed(LINENUM*10);
	
	Os__light_on();
}

unsigned char PRINT_RePrint_List(void)
{
	unsigned char aucBuf[40];
	unsigned char aucPrint[40];
	unsigned char aucPrintBuf[40];
	unsigned char flag;
	unsigned long ultotalamount;
	unsigned int uitotalnb;
	unsigned long ulBatchNum;
	unsigned int uiPrintLen;
	TRANSTOTAL	*pTransTotal;
	TRANSTOTAL	*pForeignTransTotal;


#ifdef GUI_PROJECT		
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=2;
	memcpy(ProUiFace.ProToUi.aucLine1,"交易成功",14);
	memcpy(ProUiFace.ProToUi.aucLine2,"正在打印...",13);
	if(!g_ThreadFlag)
		sleep(1);
	else{
    sem_post(&binSem1);
    sem_wait(&binSem2);
  }
#endif

	if(DataSave0.TransInfoData.ucLastSettleFlag!=true)
		return ERR_NOLASTSETTLE;
	//Os__clr_display(255);
	//Os__GB2312_display(1, 0, (uchar * )"重打上批次结算单");
	//Os__GB2312_display(2, 0, (uchar * )"    请稍等...");
	Os__light_off();
#if PS300
	util_Printf("\nPS300 \n");
	if(DataSave0.ConstantParamData.prnflagOld)
		PRINT_XReverseFeed(LINENUM*6);
#endif

#if PS100
	util_Printf("\nPS100 \n");
	if(DataSave0.ConstantParamData.prnflagOld)
		PRINT_XReverseFeed(LINENUM*7);
#endif

	PRINT_GB2312_xprint((unsigned char*)"    结算单(重打印)",0x1e);

	if(DataSave0.ConstantParamData.prnflag)
   		PRINT_xlinefeed(1*LINENUM);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户名称:",9);
	//PRINT_GB2312_xprint(aucPrintBuf,FONT);
	//memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	//memset(aucPrintBuf,' ',10);
	uiPrintLen = strlen((char*)DataSave0.ConstantParamData.aucMerchantName);
	if(uiPrintLen>40) uiPrintLen =40;
	memcpy(&aucPrintBuf[9],DataSave0.ConstantParamData.aucMerchantName,uiPrintLen);

	if (DataSave0.ConstantParamData.prnflagOld
		||DataSave0.ConstantParamData.prnflagNew
		)
		PrintGB2312Info(aucPrintBuf,uiPrintLen+9,0x03);
	if (DataSave0.ConstantParamData.prnflagZD)
		PrintGB2312Info(aucPrintBuf,uiPrintLen+9,0x02);
	if (DataSave0.ConstantParamData.prnflag)
		PrintGB2312Info(aucPrintBuf,uiPrintLen+9,0x01);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户号:  ",8);
	memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"终端号:",7);
	memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);


	/*print issuer id*/
	memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
	strcpy((char*)aucPrintBuf,"收单行号:");
	memcpy(&aucPrintBuf[strlen((char*)aucPrintBuf)],DataSave0.ChangeParamData.aucAcquereID,11);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);

	/*print batch number*/
	memset(aucPrintBuf,'0',sizeof(aucPrintBuf));
	ulBatchNum=DataSave0.TransInfoData.ulLastBatchNumber;
	long_asc(aucPrintBuf,6,&ulBatchNum);
	PRINT_PrintTwoMSG( FONT,MSG_BATCHNUM,aucPrintBuf,6,-1,0,0);

	memset(aucPrint,0x00,sizeof(aucPrint));
	memcpy( &aucPrint[0],&DataSave0.TransInfoData.ucLasSettleData[0],TRANS_DATELEN );
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	aucPrintBuf[25]=0x00;
	//memcpy(aucPrintBuf,"日期-时间:",10);
	//PRINT_GB2312_xprint(aucPrintBuf,FONT);
	memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
	memset(aucPrintBuf,0x20,sizeof(aucPrintBuf)-1);
	UTIL_format_date_bcd_str(&aucPrintBuf[0],aucPrint);
	aucPrintBuf[26]=0;
	UTIL_format_time_bcd_str(&aucPrintBuf[15],DataSave0.TransInfoData.ucLasSettleTime);
	aucPrintBuf[30] = 0;
	PRINT_GB2312_xprint(aucPrintBuf,FONT);

	flag =0;
	flag|=RE_PRINT_FLAG;
	flag|=PRINT_SETTLE_FLAG;

	pTransTotal = &DataSave0.TransInfoData.LastTransTotal;
	pForeignTransTotal = &DataSave0.TransInfoData.LastForeignTransTotal;
	{
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
		if(PRINT_TOTAL_FLAG!=(flag&PRINT_TOTAL_FLAG))
		{
			if(pTransTotal->ucSettleFlag== SETTLE_OK)
				PRINT_GB2312_xprint((unsigned char *)"内卡对帐平",FONT);
			else
			if(pTransTotal->ucSettleFlag == SETTLE_FAIL)
				PRINT_GB2312_xprint((unsigned char *)"内卡对帐不平",FONT);
			else
			if(pTransTotal->ucSettleFlag == SETTLE_ERROR)
				PRINT_GB2312_xprint((unsigned char *)"内卡对帐错误",FONT);
		}

		PRINT_GB2312_xprint((unsigned char *)"内卡消费",FONT);
		uitotalnb = pTransTotal->uiPurchaseNb;
		ultotalamount = pTransTotal->ulPurchaseAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);
//		//add
//		PRINT_GB2312_xprint((unsigned char *)"内卡预授权",FONT);
//		uitotalnb = pTransTotal->uiAuthNb;
//		ultotalamount = pTransTotal->ulAuthAmount;
//		memset(aucBuf,' ',sizeof(aucBuf));
//		int_asc(aucBuf,3,&uitotalnb);
//		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
//		PRINT_GB2312_xprint(aucBuf,FONT);
//		//end
		PRINT_GB2312_xprint((unsigned char *)"内卡预授权完成(联机)",FONT);
		uitotalnb = pTransTotal->uiAuthFinishNb;
		ultotalamount = pTransTotal->ulAuthFinishAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_GB2312_xprint((unsigned char *)"内卡预授权完成(离线)",FONT);
		uitotalnb = pTransTotal->uiPreauthSettleNb;
		ultotalamount = pTransTotal->ulPreauthSettleAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_GB2312_xprint((unsigned char *)"内卡离线:",FONT);
		uitotalnb = pTransTotal->uiOfflineNb;
		ultotalamount = pTransTotal->ulOfflineAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_GB2312_xprint((unsigned char *)"内卡退货:",FONT);
		uitotalnb = pTransTotal->uiRefundNb;
		ultotalamount = pTransTotal->ulRefundAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
		if(PRINT_TOTAL_FLAG!=(flag&PRINT_TOTAL_FLAG))
		{
			if(pForeignTransTotal->ucSettleFlag==SETTLE_OK)
				PRINT_GB2312_xprint((unsigned char *)"外卡对帐平",FONT);
			else
			if(pForeignTransTotal->ucSettleFlag == SETTLE_FAIL)
				PRINT_GB2312_xprint((unsigned char *)"外卡对帐不平",FONT);
			else
			if(pForeignTransTotal->ucSettleFlag == SETTLE_ERROR)
				PRINT_GB2312_xprint((unsigned char *)"外卡对帐错误",FONT);
		}

		PRINT_GB2312_xprint((unsigned char *)"外卡消费",FONT);
		uitotalnb = pForeignTransTotal->uiPurchaseNb;
		ultotalamount = pForeignTransTotal->ulPurchaseAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);
//		//add
//		PRINT_GB2312_xprint((unsigned char *)"外卡预授权",FONT);
//		uitotalnb = pForeignTransTotal->uiAuthNb;
//		ultotalamount = pForeignTransTotal->ulAuthAmount;
//		memset(aucBuf,' ',sizeof(aucBuf));
//		int_asc(aucBuf,3,&uitotalnb);
//		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
//		PRINT_GB2312_xprint(aucBuf,FONT);
//		//end
		PRINT_GB2312_xprint((unsigned char *)"外卡预授权完成(联机)",FONT);
		uitotalnb = pForeignTransTotal->uiAuthFinishNb;
		ultotalamount = pForeignTransTotal->ulAuthFinishAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_GB2312_xprint((unsigned char *)"外卡预授权完成(离线)",FONT);
		uitotalnb = pForeignTransTotal->uiPreauthSettleNb;
		ultotalamount = pForeignTransTotal->ulPreauthSettleAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_GB2312_xprint((unsigned char *)"外卡离线:",FONT);
		uitotalnb = pForeignTransTotal->uiOfflineNb;
		ultotalamount = pForeignTransTotal->ulOfflineAmount;
		memset(aucBuf,' ',sizeof(aucBuf));
		int_asc(aucBuf,3,&uitotalnb);
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
		PRINT_GB2312_xprint(aucBuf,FONT);

//		PRINT_GB2312_xprint((unsigned char *)"外卡退货:",FONT);
//		uitotalnb = pForeignTransTotal->uiRefundNb;
//		ultotalamount = pForeignTransTotal->ulRefundAmount;
//		memset(aucBuf,' ',sizeof(aucBuf));
//		int_asc(aucBuf,3,&uitotalnb);
//		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
//		PRINT_GB2312_xprint(aucBuf,FONT);

		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	}

	if(DataSave0.ChangeParamData.ucSuperLogonFlag==CASH_LOGONFLAG)
	{
		PRINT_GB2312_xprint((unsigned char *)"测试/TESTING",FONT);
	}

	PRINT_EveryCardDetail(RE_PRINT_FLAG);
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	util_Printf("KKK=%02x\n",DataSave0.ConstantParamData.prnflag);
	util_Printf("ZD=%02x\n",DataSave0.ConstantParamData.prnflagZD);
	if(DataSave0.ConstantParamData.prnflag)
		{
			PRINT_xlinefeed(1*LINENUM);
		}
	else
		{
#if PS300
			PRINT_xlinefeed(0.5*LINENUM);
#endif
		}

	if(DataSave0.ConstantParamData.prnflagNew
		||DataSave0.ConstantParamData.prnflagOld)
	{
		#if PS300
		PRINT_xlinefeed(11*LINENUM);
		#else
		util_Printf("PS1000000\n");
		PRINT_xlinefeed(10*LINENUM);
		#endif
	}

	if(DataSave0.ConstantParamData.prnflag)
		{
			PRINT_xlinefeed(1*LINENUM);
		}
	else
		{
#if PS300
		util_Printf("1\n");
			PRINT_xlinefeed(0.5*LINENUM);
#endif
		}
	if(DataSave0.ConstantParamData.prnflag)
	{
		util_Printf("2\n");
		PRINT_xlinefeed(2*LINENUM);
	}
	if(DataSave0.ConstantParamData.prnflagZD)
	{
		util_Printf("3\n");
		PRINT_xlinefeed(10*LINENUM);
	}
	Os__light_on();
}

unsigned char PRINT_PrintTwoMSG( unsigned char ucFontType,
					unsigned short iIndex1,
					unsigned char *pucBuf1,
					unsigned short uiBufLen1,
//					unsigned short iIndex2,
					short iIndex2,
					unsigned char *pucBuf2,
					unsigned short uiBufLen2)
{
	unsigned char ucI;
	unsigned char ucResult;
	unsigned char aucBuf[50];
	unsigned char ucType1;
	unsigned char ucType2;
	unsigned short uiOffset;
	unsigned short uiPrintOffset;
	unsigned short uiLen;

	memset(aucBuf,0,sizeof(aucBuf));
	if( ucFontType )
	{
		aucBuf[0] = ucFontType;
		uiOffset = 1;
		uiPrintOffset = 1;
	}else
	{
		uiOffset = 0;
		uiPrintOffset = 0;
	}

	ucType1 = MSG_TYPEEN;
	ucType2 = MSG_TYPEEN;

	if(  (iIndex1 >0 )
	   &&(iIndex1 < MSG_MAXNB)
	  )
	{
//		ucType1 = DataSave0.MSGData.aucMSGType[iIndex1];
		ucType1 = MSGTABPrn[iIndex1].aucMSGType;
		MSG_GetMsg((unsigned short)iIndex1,&aucBuf[uiOffset],sizeof(aucBuf)-uiOffset);
		uiOffset = strlen((char *)aucBuf);
	}
	if( pucBuf1 )
	{
		uiLen = min(sizeof(aucBuf)-uiOffset,uiBufLen1);
		memcpy(&aucBuf[uiOffset],pucBuf1,uiLen);
		uiOffset += uiLen;
	}
	if(  (iIndex2 >=0 )
	   &&(iIndex2 < MSG_MAXNB)
	  )
	{
		ucType2= MSGTABPrn[iIndex2].aucMSGType;
		MSG_GetMsg((unsigned short)iIndex2,&aucBuf[uiOffset],sizeof(aucBuf)-uiOffset);
		uiOffset = strlen((char *)aucBuf);
	}
	if( pucBuf2 )
	{
		uiLen = min(sizeof(aucBuf)-uiOffset,uiBufLen2);
		memcpy(&aucBuf[uiOffset],pucBuf2,uiLen);
		uiOffset += uiLen;
	}

    for(ucI=0;ucI<PRINT_MAXRETRYTIME;ucI++)
    {
		if(  (ucType1 == MSG_TYPEZHCN)
		   ||(ucType2 == MSG_TYPEZHCN)
		  )
		{
	    	ucResult = PRINT_GB2312_xprint(&aucBuf[uiPrintOffset],ucFontType);
		}else
		{
	    	ucResult = PRINT_xprint(aucBuf);
		}
	    if( ucResult == OK)
	    	return(SUCCESS);
	}
}

unsigned char PRINT_PrintMsg( unsigned char ucFontType,unsigned short uiIndex)
{
	unsigned char ucI;
	unsigned char ucType;
	unsigned char ucResult;
	unsigned char aucBuf[PRINT_MAXCHAR+2];
	unsigned char ucOffset;

	memset(aucBuf,0,sizeof(aucBuf));
	if( uiIndex >= MSG_MAXNB)
	{
		return(ERR_MSG_INVALIDINDEX);
	}
//    ucType = DataSave0.MSGData.aucMSGType[uiIndex];
	ucType = MSGTABPrn[uiIndex].aucMSGType;
    ucOffset = 0;
    switch( ucType )
    {
    case MSG_TYPEZHCN:
    	break;
    default:
    	if( ucFontType )
    	{
    		aucBuf[ucOffset] = ucFontType;
    		ucOffset ++;
    	}
    	break;
    }
	MSG_GetMsg(uiIndex,&aucBuf[ucOffset],sizeof(aucBuf)-ucOffset);

    for(ucI=0;ucI<PRINT_MAXRETRYTIME;ucI++)
    {
	    switch( ucType )
	    {
	    case MSG_TYPEZHCN:
	    	ucResult = PRINT_GB2312_xprint(aucBuf,ucFontType);
	    	break;
	    default:
	    	ucResult = PRINT_xprint(aucBuf);
	    	break;
	    }
	    if( ucResult == OK)
	    	return(SUCCESS);
	}
	return(ERR_PRINT);
}

void PRINT_Print_Local_Date_Time(void)
{
	unsigned char aucBuf[40];
	unsigned char aucPrint[40];
	unsigned char aucPrintBuf[40];
	unsigned char aucdate[4];
	unsigned char auctime[4];

	Os__read_date(aucdate);
	memcpy(&aucBuf[0],&aucdate[4],2);
	memcpy(&aucBuf[2],&aucdate[2],2);
	memcpy(&aucBuf[4],&aucdate[0],2);
	asc_bcd(&aucPrint[1],3,aucBuf,6);
	aucPrint[0] = (aucPrint[1] > 0x90) ?  0x19 :  0x20 ;
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	UTIL_format_date_bcd_str(aucPrintBuf,aucPrint);
	Os__read_time(auctime);
	asc_bcd(aucPrint,2,auctime,4);
    	UTIL_format_time_bcd_str(&aucPrintBuf[11],aucPrint);
	PRINT_GB2312_xprint((unsigned char *)"日期　　　　时间",0x00);
	PRINT_xprint(aucPrintBuf);
}

unsigned char PRINT_Detail_Menu(void)
{
	unsigned char ucResult;

	ucResult = UTIL_Is_Trans_Empty();

	if(ucResult != SUCCESS)
	{
		MSG_DisplayMsgKey( true,1,2,MSG_NOTRANS,3);
		return SUCCESS;
	}
	

#ifdef GUI_PROJECT		
	memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
	ProUiFace.ProToUi.uiLines=2;
	memcpy(ProUiFace.ProToUi.aucLine1,"交易成功",14);
	memcpy(ProUiFace.ProToUi.aucLine2,"正在打印...",13);
	if(!g_ThreadFlag)
		sleep(1);
	else{
    sem_post(&binSem1);
    sem_wait(&binSem2);
  }
#endif
	
	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(uchar *)"正在打印交易明细");
	if (DataSave0.ConstantParamData.prnflagZD
		|| DataSave0.ConstantParamData.prnflagOld
		|| DataSave0.ConstantParamData.prnflagNew)
	{
		PRINT_detail_ZD();
	}
	else
	{
		PRINT_detail();
	}
	PRINT_xlinefeed(1*LINENUM);
	return SUCCESS ;
}

void PRINT_detail_void_ZD(void)
{
	unsigned int uiIndex;
	unsigned char aucPrintBuf[30],aucTmp[15];
	unsigned int ucI;

	XDATA_Read_Vaild_File(DataSaveTransInfo);
	for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
		if(uiIndex>=(DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb+
			    DataSave0.TransInfoData.TransTotal.uiTotalNb))
					break;

		if( DataSave0.TransInfoData.auiTransIndex[uiIndex])
		{
			GetOldTransData(uiIndex);
			OSMEM_Memcpy((unsigned char *)&(DataSave1.SaveTrans),
							(unsigned char *)&NormalTransData,
							sizeof(NORMALTRANS));
			if((DataSave0.TransInfoData.auiTransIndex[uiIndex]&TRANS_NIIVOID)!=TRANS_NIIVOID)
			{
				if( (G_NORMALTRANS_ucTransType == TRANS_PURCHASE)
		   		 	  &&(G_NORMALTRANS_ucTIPAdjustFlag)
		   		   	  )
					{}else	continue;
			}

			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			if((DataSave0.TransInfoData.auiTransIndex[uiIndex]&TRANS_NIINORMAL)==TRANS_NIINORMAL)
			{
				if(G_NORMALTRANS_ucTransType == TRANS_PURCHASE
				||G_NORMALTRANS_ucTransType == TRANS_OFFPURCHASE
				||G_NORMALTRANS_ucTransType == TRANS_PREAUTHFINISH
				||G_NORMALTRANS_ucTransType == TRANS_CUPMOBILE
				||G_NORMALTRANS_ucTransType == TRANS_EC_CASHLOAD
				||G_NORMALTRANS_ucTransType == TRANS_MOBILEAUTH
				)
					aucPrintBuf[0] = '*';
				else
					continue;
			}
			else
			if( (G_NORMALTRANS_ucTransType == TRANS_PURCHASE)
   		 	  &&(G_NORMALTRANS_ucTIPAdjustFlag)
   		   	  )
			{
				aucPrintBuf[0] = '*';
			}else
			if((G_NORMALTRANS_ucTransType ==TRANS_VOIDPURCHASE)
			||(G_NORMALTRANS_ucTransType ==TRANS_VOIDPREAUTHFINISH)
			||(G_NORMALTRANS_ucTransType ==TRANS_VOIDOFFPURCHASE)
			||(G_NORMALTRANS_ucTransType ==TRANS_VOIDCUPMOBILE)
			||(G_NORMALTRANS_ucTransType ==TRANS_VOIDMOBILEAUTH)
			)
				aucPrintBuf[0] = 'V';
			else
				continue;

			switch(G_NORMALTRANS_ucOldTransType)
		       {
				case TRANS_PURCHASE:
				case TRANS_CUPMOBILE:
				case TRANS_OFFPURCHASE:
					aucPrintBuf[2] = 'S';
					break;
				case TRANS_PREAUTHFINISH:
					aucPrintBuf[2] = 'P';
					break;
				case TRANS_PREAUTHSETTLE:
					aucPrintBuf[2] = 'C';
					break;
				case TRANS_REFUND:
					aucPrintBuf[2] = 'R';
					break;
				case TRANS_TIPADJUST:
				case TRANS_OFFPREAUTHFINISH:
				case TRANS_TIPADJUSTOK:
		  			aucPrintBuf[2] = 'L';
					break;
				case TRANS_EC_CASHLOAD:
					aucPrintBuf[2] ='Q';
					break;
				case TRANS_MOBILEAUTH:
					aucPrintBuf[2]='M';
					break;
		    		default:
		    			break;
		       }
		    if(G_NORMALTRANS_ucSourceAccLen%2)
		    {
		    	bcd_asc(&aucPrintBuf[4],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
		    }else
		    {
		    	bcd_asc(&aucPrintBuf[4],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
		    }
			aucPrintBuf[G_NORMALTRANS_ucSourceAccLen+4] = 0;
			if (DataSave0.ConstantParamData.ucPrintCardNo==0x31)
				{
					if  ((G_NORMALTRANS_ucTransType != TRANS_PREAUTH)&&
						(G_NORMALTRANS_ucTransType != TRANS_TIPADJUST) &&
						(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)&&
						(G_NORMALTRANS_ucTransType != TRANS_UNDOOFF)&&
                                   	(G_NORMALTRANS_ucTransType != TRANS_EC_UNASSIGNLOAD)&&
                                    	(G_NORMALTRANS_ucECTrans !=TRANS_EC_PURCHASE)
						)
					{
						if (DataSave0.ConstantParamData.ucPBeforeSix==0x30)
						{
							for(ucI=10;ucI<G_NORMALTRANS_ucSourceAccLen;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
						if (DataSave0.ConstantParamData.ucPBeforeSix==0x31)
						{
							for(ucI=4;ucI<G_NORMALTRANS_ucSourceAccLen;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
					}
				}
			PRINT_GB2312_xprint(aucPrintBuf,0X00);

			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memset(aucTmp,' ',sizeof(aucTmp));
			long_asc(aucPrintBuf,6,&G_NORMALTRANS_ulTraceNumber);
			UTIL_Form_Montant(aucTmp,G_NORMALTRANS_ulAmount+G_NORMALTRANS_ulFeeAmount,2);

			if(DataSave0.ConstantParamData.prnflagZD)
			{
				memcpy(&aucPrintBuf[6],aucTmp,12);
				PRINT_GB2312_xprint(aucPrintBuf,0X00);
				if (G_NORMALTRANS_aucAuthCode[0])
				{
					memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
					memcpy(&aucPrintBuf[17],G_NORMALTRANS_aucAuthCode,6);
					aucPrintBuf[26] = 0x00;
					PRINT_GB2312_xprint(aucPrintBuf,0X00);
				}
			}else
			{
				memcpy(&aucPrintBuf[6],aucTmp,12);
				memset(&aucPrintBuf[17] ,' ',8 );
				memcpy(&aucPrintBuf[19],G_NORMALTRANS_aucAuthCode,6);
				aucPrintBuf[26] = 0x00;
				PRINT_GB2312_xprint(aucPrintBuf,0X00);
			}
		}
	}
}


void PRINT_detail_ZD(void)
{
	unsigned int uiIndex;
	unsigned char aucPrintBuf[50],aucTmp[15],aucTimeBuf[20];
	unsigned int ucI;
	unsigned long ulBatchNum;

	PRINT_GB2312_xprint((unsigned char *)"交易明细/TXN LIST",0x1E);
	PRINT_xprint((unsigned char *)" ");

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"批次号:",7);
	ulBatchNum = DataSave0.ChangeParamData.ulBatchNumber;
	long_asc(&aucPrintBuf[7],6,&ulBatchNum);
	PRINT_GB2312_xprint(aucPrintBuf,0x00);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(&aucPrintBuf[0],"日 期:",6);

	UTIL_READ_DateTimeAndFormat(aucTimeBuf);
	memcpy(&aucPrintBuf[6],"20",2);
	memcpy(&aucPrintBuf[8],aucTimeBuf,2);//YY
	memcpy(&aucPrintBuf[10],"/",1);
	memcpy(&aucPrintBuf[11],&aucTimeBuf[2],2);//MM
	memcpy(&aucPrintBuf[13],"/",1);
	memcpy(&aucPrintBuf[14],&aucTimeBuf[4],2);//DD
	memcpy(&aucPrintBuf[16]," ",1);
	memcpy(&aucPrintBuf[17],&aucTimeBuf[6],2);//hh
	memcpy(&aucPrintBuf[19],":",1);
	memcpy(&aucPrintBuf[20],&aucTimeBuf[8],2);
	PRINT_GB2312_xprint(aucPrintBuf,0x00);

	PRINT_GB2312_xprint((unsigned char *)"类型          卡号 ",0X00);
	PRINT_GB2312_xprint((unsigned char *)"凭证号   金额   授权码",0X00);

	XDATA_Read_Vaild_File(DataSaveTransInfo);
	for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
		if(uiIndex>=(DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb+
			    DataSave0.TransInfoData.TransTotal.uiTotalNb))
					break;

		if( DataSave0.TransInfoData.auiTransIndex[uiIndex])
		{
			if((DataSave0.TransInfoData.auiTransIndex[uiIndex]&TRANS_NIIVOID)==TRANS_NIIVOID)
				continue;
			GetOldTransData(uiIndex);
			OSMEM_Memcpy((unsigned char *)&(DataSave1.SaveTrans),
							(unsigned char *)&NormalTransData,
							sizeof(NORMALTRANS));

		    if( (G_NORMALTRANS_ucTransType != TRANS_TIPADJUST)
		      &&(G_NORMALTRANS_ucTransType != TRANS_TIPADJUSTOK)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_PURCHASE)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_REFUND)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_PREAUTHFINISH)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_PREAUTHSETTLE)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_OFFPURCHASE)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_MOBILEAUTH)
   		   	  )
   		   	  	continue;
   		   	if( (G_NORMALTRANS_ucTransType == TRANS_PURCHASE)
   		   	  &&(G_NORMALTRANS_ucTIPAdjustFlag)
   		   	  )
   		   	  	continue;
   		   	if( (G_NORMALTRANS_ucTransType == TRANS_OFFPREAUTHFINISH)
   		   	  &&(G_NORMALTRANS_ucTIPAdjustFlag == 1)
   		   	  )
   		   	  	continue;
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			switch(G_NORMALTRANS_ucTransType)
		       {
				case TRANS_PURCHASE:
				case TRANS_CUPMOBILE: //add 09-1-16
				case TRANS_OFFPURCHASE:
			  		aucPrintBuf[1] = 'S';
				   	break;
				case TRANS_MOBILEAUTH:
					aucPrintBuf[1] = 'M';
					break;
		    		case TRANS_PREAUTHFINISH:
					aucPrintBuf[1] = 'P';
					break;
				case TRANS_PREAUTHSETTLE:
			  		aucPrintBuf[1] = 'C';
			    		break;
		    		case TRANS_REFUND:
		  			aucPrintBuf[1] = 'R';
		    			break;
				case TRANS_TIPADJUST:
				case TRANS_OFFPREAUTHFINISH:
				case TRANS_TIPADJUSTOK:
		  			aucPrintBuf[1] = 'L';
					break;
				case TRANS_EC_CASHLOAD:
						aucPrintBuf[1] ='Q';
						break;
		    		default:
		    			break;
		       }

		    if(G_NORMALTRANS_ucSourceAccLen%2)
		    {
		    	bcd_asc(&aucPrintBuf[4],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
		    }else
		    {
		    	bcd_asc(&aucPrintBuf[4],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
		    }
			aucPrintBuf[G_NORMALTRANS_ucSourceAccLen+4] = 0x20;

			if (DataSave0.ConstantParamData.ucPrintCardNo==0x31)
				{
					if  ((G_NORMALTRANS_ucTransType != TRANS_PREAUTH)&&
						(G_NORMALTRANS_ucTransType != TRANS_TIPADJUST) &&
						(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)&&
						(G_NORMALTRANS_ucTransType != TRANS_UNDOOFF)&&
                                   	(G_NORMALTRANS_ucTransType != TRANS_EC_UNASSIGNLOAD)&&
                                    	(G_NORMALTRANS_ucECTrans !=TRANS_EC_PURCHASE)
						)
					{
						if (DataSave0.ConstantParamData.ucPBeforeSix==0x30)
						{
							for(ucI=10;ucI<G_NORMALTRANS_ucSourceAccLen;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
						if (DataSave0.ConstantParamData.ucPBeforeSix==0x31)
						{
							for(ucI=4;ucI<G_NORMALTRANS_ucSourceAccLen;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
					}
				}
			PRINT_GB2312_xprint(aucPrintBuf,0X00);
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memset(aucTmp,' ',sizeof(aucTmp));
			long_asc(aucPrintBuf,6,&G_NORMALTRANS_ulTraceNumber);
			UTIL_Form_Montant(aucTmp,G_NORMALTRANS_ulAmount+G_NORMALTRANS_ulFeeAmount,2);

			if(DataSave0.ConstantParamData.prnflagZD)
			{
				memcpy(&aucPrintBuf[6],aucTmp,12);
				PRINT_GB2312_xprint(aucPrintBuf,0X00);
				if (G_NORMALTRANS_aucAuthCode[0])
				{
					memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
					memcpy(&aucPrintBuf[17],G_NORMALTRANS_aucAuthCode,6);
					aucPrintBuf[26] = 0x00;
					PRINT_GB2312_xprint(aucPrintBuf,0X00);
				}
			}else
			{
				memcpy(&aucPrintBuf[6],aucTmp,12);
				memset(&aucPrintBuf[17] ,' ',8 );
				memcpy(&aucPrintBuf[19],G_NORMALTRANS_aucAuthCode,6);
				aucPrintBuf[26] = 0x00;
				PRINT_GB2312_xprint(aucPrintBuf,0X00);
			}
		}
	}
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	PRINT_detail_void_ZD();
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");

	if(!DataSave0.ConstantParamData.prnflagZD)
		PRINT_xlinefeed(3*LINENUM);
	else
		PRINT_xlinefeed(10*LINENUM);
}


void PRINT_detail(void)
{
	unsigned int uiIndex;
	unsigned char aucPrintBuf[50],aucTmp[15],aucTimeBuf[20];
	unsigned int ucI;
	unsigned long ulBatchNum;

	PRINT_GB2312_xprint((unsigned char *)"    交易明细/TXN LIST",0x1E);
	PRINT_xprint((unsigned char *)" ");

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"批次号:",7);
	ulBatchNum = DataSave0.ChangeParamData.ulBatchNumber;
	long_asc(&aucPrintBuf[7],6,&ulBatchNum);
	memcpy(&aucPrintBuf[23],"日期:",5);

	UTIL_READ_DateTimeAndFormat(aucTimeBuf);
	memcpy(&aucPrintBuf[22],"20",2);
	memcpy(&aucPrintBuf[24],aucTimeBuf,2);//YY
	memcpy(&aucPrintBuf[26],"/",1);
	memcpy(&aucPrintBuf[27],&aucTimeBuf[2],2);//MM
	memcpy(&aucPrintBuf[29],"/",1);
	memcpy(&aucPrintBuf[30],&aucTimeBuf[4],2);//DD
	memcpy(&aucPrintBuf[32]," ",1);
	memcpy(&aucPrintBuf[33],&aucTimeBuf[6],2);//hh
	memcpy(&aucPrintBuf[35],":",1);
	memcpy(&aucPrintBuf[36],&aucTimeBuf[8],2);
	PRINT_GB2312_xprint(aucPrintBuf,0x1D);

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	PRINT_GB2312_xprint((unsigned char *)"类型     卡    号      凭证号 / 金额   授权码",0X1D);

	XDATA_Read_Vaild_File(DataSaveTransInfo);
	for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
		if(uiIndex>=(DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb+
			    DataSave0.TransInfoData.TransTotal.uiTotalNb))
					break;

		if( DataSave0.TransInfoData.auiTransIndex[uiIndex])
		{
			if((DataSave0.TransInfoData.auiTransIndex[uiIndex]&TRANS_NIIVOID)==TRANS_NIIVOID)
				continue;
			GetOldTransData(uiIndex);
			OSMEM_Memcpy((unsigned char *)&(DataSave1.SaveTrans),
							(unsigned char *)&NormalTransData,
							sizeof(NORMALTRANS));
		    if( (G_NORMALTRANS_ucTransType != TRANS_TIPADJUST)
		      &&(G_NORMALTRANS_ucTransType != TRANS_TIPADJUSTOK)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_PURCHASE)
			  &&(G_NORMALTRANS_ucTransType != TRANS_CREDITSALE)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_REFUND)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_PREAUTHFINISH)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_PREAUTHSETTLE)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_OFFPURCHASE)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_EC_CASHLOAD)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_MOBILEAUTH)
   		   	  )
   		   	  	continue;
   		   	if( (G_NORMALTRANS_ucTransType == TRANS_PURCHASE)
   		   	  &&(G_NORMALTRANS_ucTIPAdjustFlag)
   		   	  )
   		   	  	continue;
   		   	if( (G_NORMALTRANS_ucTransType == TRANS_OFFPREAUTHFINISH)
   		   	  &&(G_NORMALTRANS_ucTIPAdjustFlag == 1)
   		   	  )
   		   	  	continue;
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			switch(G_NORMALTRANS_ucTransType)
		       {
				case TRANS_CREDITSALE:
				case TRANS_PURCHASE:
				case TRANS_CUPMOBILE: //add 09-1-16
				case TRANS_OFFPURCHASE:
			  		aucPrintBuf[1] = 'S';
				   	break;
		    		case TRANS_PREAUTHFINISH:
					aucPrintBuf[1] = 'P';
					break;
				case TRANS_PREAUTHSETTLE:
			  		aucPrintBuf[1] = 'C';
			    		break;
		    		case TRANS_REFUND:
		  			aucPrintBuf[1] = 'R';
		    			break;
				case TRANS_TIPADJUST:
				case TRANS_OFFPREAUTHFINISH:
				case TRANS_TIPADJUSTOK:
		  			aucPrintBuf[1] = 'L';
					break;
				case TRANS_EC_CASHLOAD:
					aucPrintBuf[1] = 'Q';
					break;
				case TRANS_MOBILEAUTH:
					aucPrintBuf[1]='M';
					break;
		    		default:
		    			break;
		       }

		    if(G_NORMALTRANS_ucSourceAccLen%2)
		    {
		    	bcd_asc(&aucPrintBuf[4],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
		    }else
		    {
		    	bcd_asc(&aucPrintBuf[4],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
		    }
		    aucPrintBuf[G_NORMALTRANS_ucSourceAccLen+4] =0x20;

			if (DataSave0.ConstantParamData.ucPrintCardNo==0x31)
				{
					if  ((G_NORMALTRANS_ucTransType != TRANS_PREAUTH)&&
						(G_NORMALTRANS_ucTransType != TRANS_TIPADJUST) &&
						(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)&&
						(G_NORMALTRANS_ucTransType != TRANS_UNDOOFF)&&
                    	(G_NORMALTRANS_ucTransType != TRANS_EC_UNASSIGNLOAD)&&
                    	(G_NORMALTRANS_ucECTrans !=TRANS_EC_PURCHASE)
						)
					{
						if (DataSave0.ConstantParamData.ucPBeforeSix==0x30)
						{
							for(ucI=10;ucI<G_NORMALTRANS_ucSourceAccLen;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
						if (DataSave0.ConstantParamData.ucPBeforeSix==0x31)
						{
							for(ucI=4;ucI<G_NORMALTRANS_ucSourceAccLen;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
					}
				}
			long_asc(&aucPrintBuf[24],6,&G_NORMALTRANS_ulTraceNumber);
			PRINT_GB2312_xprint(aucPrintBuf,0X00);

			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memset(aucTmp,' ',sizeof(aucTmp));
			memcpy(&aucTmp[0],"RMB",3);
			UTIL_Form_Montant(&aucTmp[4],G_NORMALTRANS_ulAmount+G_NORMALTRANS_ulFeeAmount,2);

			if(DataSave0.ConstantParamData.prnflagZD)
			{
				memcpy(&aucPrintBuf[6],aucTmp,12);
				PRINT_GB2312_xprint(aucPrintBuf,0X00);
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				memcpy(&aucPrintBuf[17],G_NORMALTRANS_aucAuthCode,6);
				aucPrintBuf[26] = 0x00;
				PRINT_GB2312_xprint(aucPrintBuf,0X00);
			}
			else
			{
				memcpy(&aucPrintBuf[0],aucTmp,15);
				memset(&aucPrintBuf[15] ,' ',8);
				memcpy(&aucPrintBuf[24],G_NORMALTRANS_aucAuthCode,6);
				aucPrintBuf[30] = 0x00;
				PRINT_GB2312_xprint(aucPrintBuf,0X00);
			}
		}
	}
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	PRINT_detail_void();
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");

	if(!DataSave0.ConstantParamData.prnflagZD)
		PRINT_xlinefeed(3*LINENUM);
	else
		PRINT_xlinefeed(10*LINENUM);
}

void PRINT_detail_void(void)
{
	unsigned int uiIndex;
	unsigned char aucPrintBuf[50],aucTmp[15];
	unsigned int ucI;

	XDATA_Read_Vaild_File(DataSaveTransInfo);
	for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
		if(uiIndex>=(DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb+
			    DataSave0.TransInfoData.TransTotal.uiTotalNb))
					break;

		if( DataSave0.TransInfoData.auiTransIndex[uiIndex])
		{
			GetOldTransData(uiIndex);
			OSMEM_Memcpy((unsigned char *)&(DataSave1.SaveTrans),
							(unsigned char *)&NormalTransData,
							sizeof(NORMALTRANS));
			if((DataSave0.TransInfoData.auiTransIndex[uiIndex]&TRANS_NIIVOID)!=TRANS_NIIVOID)
			{
				if( (G_NORMALTRANS_ucTransType == TRANS_PURCHASE)
		   		 	  &&(G_NORMALTRANS_ucTIPAdjustFlag)
		   		   	  )
					{
					}else	continue;
			}

			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			if((DataSave0.TransInfoData.auiTransIndex[uiIndex]&TRANS_NIINORMAL)==TRANS_NIINORMAL)
			{
				if(G_NORMALTRANS_ucTransType == TRANS_PURCHASE
				||G_NORMALTRANS_ucTransType ==TRANS_CREDITSALE
				||G_NORMALTRANS_ucTransType == TRANS_OFFPURCHASE
				||G_NORMALTRANS_ucTransType == TRANS_PREAUTHFINISH
				||G_NORMALTRANS_ucTransType == TRANS_CUPMOBILE
				||G_NORMALTRANS_ucTransType == TRANS_MOBILEAUTH
				)
					aucPrintBuf[0] = '*';
				else
					continue;
			}
			else
			if( (G_NORMALTRANS_ucTransType == TRANS_PURCHASE)
   		 	  &&(G_NORMALTRANS_ucTIPAdjustFlag)
   		   	  )
			{
				aucPrintBuf[0] = '*';
			}else
			if((G_NORMALTRANS_ucTransType ==TRANS_VOIDPURCHASE)
			||(G_NORMALTRANS_ucTransType ==TRANS_VOIDPREAUTHFINISH)
			||(G_NORMALTRANS_ucTransType ==TRANS_VOIDOFFPURCHASE)
			||(G_NORMALTRANS_ucTransType ==TRANS_VOIDCUPMOBILE)
			||(G_NORMALTRANS_ucTransType ==TRANS_CREDITSVOID)
			||(G_NORMALTRANS_ucTransType ==TRANS_VOIDMOBILEAUTH)
			)
				aucPrintBuf[0] = 'V';
			else
				continue;

			switch(G_NORMALTRANS_ucOldTransType)
		       {
				case TRANS_CREDITSALE:
				case TRANS_PURCHASE:
				case TRANS_CUPMOBILE:
				case TRANS_OFFPURCHASE:
					aucPrintBuf[2] = 'S';
					break;
				case TRANS_PREAUTHFINISH:
					aucPrintBuf[2] = 'P';
					break;
				case TRANS_PREAUTHSETTLE:
					aucPrintBuf[2] = 'C';
					break;
				case TRANS_REFUND:
					aucPrintBuf[2] = 'R';
					break;
				case TRANS_TIPADJUST:
				case TRANS_OFFPREAUTHFINISH:
				case TRANS_TIPADJUSTOK:
		  			aucPrintBuf[2] = 'L';
					break;
				case TRANS_MOBILEAUTH:
					aucPrintBuf[2]='M';
					break;
		    		default:
		    			break;
		       }
		    if(G_NORMALTRANS_ucSourceAccLen%2)
		    {
		    	bcd_asc(&aucPrintBuf[4],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
		    }else
		    {
		    	bcd_asc(&aucPrintBuf[4],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
		    }
		    aucPrintBuf[G_NORMALTRANS_ucSourceAccLen+4] = 0x20;
			if (DataSave0.ConstantParamData.ucPrintCardNo==0x31)
				{
					if  ((G_NORMALTRANS_ucTransType != TRANS_PREAUTH)&&
						(G_NORMALTRANS_ucTransType != TRANS_TIPADJUST) &&
						(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)&&
						(G_NORMALTRANS_ucTransType != TRANS_UNDOOFF)&&
	                 	(G_NORMALTRANS_ucTransType != TRANS_EC_UNASSIGNLOAD)&&
	                 	(G_NORMALTRANS_ucECTrans !=TRANS_EC_PURCHASE)
						)
					{
						if (DataSave0.ConstantParamData.ucPBeforeSix==0x30)
						{
							for(ucI=10;ucI<G_NORMALTRANS_ucSourceAccLen;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
						if (DataSave0.ConstantParamData.ucPBeforeSix==0x31)
						{
							for(ucI=4;ucI<G_NORMALTRANS_ucSourceAccLen;ucI++)
							{
								aucPrintBuf[ucI]='*';
							}
						}
					}
				}
			long_asc(&aucPrintBuf[24],6,&G_NORMALTRANS_ulTraceNumber);
			PRINT_GB2312_xprint(aucPrintBuf,0X00);

			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memset(aucTmp,' ',sizeof(aucTmp));
			//changed
			memcpy(&aucTmp[0],"RMB",3);
			UTIL_Form_Montant(&aucTmp[4],G_NORMALTRANS_ulAmount+G_NORMALTRANS_ulFeeAmount,2);
			//end

			if(DataSave0.ConstantParamData.prnflagZD)
			{
				memcpy(&aucPrintBuf[0],aucTmp,12);
				PRINT_GB2312_xprint(aucPrintBuf,0X00);
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				memcpy(&aucPrintBuf[17],G_NORMALTRANS_aucAuthCode,6);
				aucPrintBuf[26] = 0x00;
				PRINT_GB2312_xprint(aucPrintBuf,0X00);
			}
			else
			{
				memcpy(&aucPrintBuf[0],aucTmp,15);
				memset(&aucPrintBuf[15] ,' ',8);
				memcpy(&aucPrintBuf[24],G_NORMALTRANS_aucAuthCode,6);
				aucPrintBuf[30] = 0x00;
				PRINT_GB2312_xprint(aucPrintBuf,0X00);
			}
		}
	}
}

void PRINT_transType(unsigned char Flag)
{
	unsigned char ucTransType;

	if ( Flag == 0)
		ucTransType = G_NORMALTRANS_ucTransType;
	else
		ucTransType = DataSave0.Trans_8583Data.ReversalTrans.ucTransType;

	switch(ucTransType)
	{
		case TRANS_PURCHASE:
			if(G_NORMALTRANS_ucECTrans ==TRANS_EC_PURCHASE)
			{
				if((G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_HB_PUTCARD)||(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_SAND_PUTCARD))
					PRINT_GB2312_xprint((unsigned char*)"QPBOC消费(SALE)", 0x1E);
				else
					PRINT_GB2312_xprint((unsigned char*)"EC消费(SALE)", 0x1E);
			}
			else
				PRINT_GB2312_xprint((unsigned char*)"消费(SALE)", 0x1E);
	   	break;
		case TRANS_CUPMOBILE:
			PRINT_GB2312_xprint((unsigned char*)"移动支付消费", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(CUPMOBILE SALE)", 0x1E);
			break;
		case TRANS_VOIDCUPMOBILE:
			PRINT_GB2312_xprint((unsigned char*)"移动支付撤销", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(VOID CUPMOBILE)", 0x1E);
			break;
		case TRANS_PREAUTH:
			PRINT_GB2312_xprint((unsigned char*)"预授权(AUTH)", 0x1E);
			break;
		case TRANS_PREAUTHFINISH:
			PRINT_GB2312_xprint((unsigned char*)"预授权完成(联机)", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(AUTH COMPLETE)", 0x1E);
			break;
		case TRANS_VOIDPURCHASE:
			PRINT_GB2312_xprint((unsigned char*)"消费撤销(VOID)", 0x1E);
			break;
		case TRANS_VOIDPREAUTH:
			PRINT_GB2312_xprint((unsigned char*)"预授权撤销(CANCEL)", 0x1E);
			break;
		case TRANS_VOIDPREAUTHFINISH:
			PRINT_GB2312_xprint((unsigned char*)"预授权完成撤销", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(COMPLETE VOID)", 0x1E);
			break;
		case TRANS_REFUND:
			PRINT_GB2312_xprint((unsigned char*)"退货(REFUND)", 0x1E);
			break;
		case TRANS_TIPADJUST:
		case TRANS_TIPADJUSTOK:
			PRINT_GB2312_xprint((unsigned char*)"离线调整(ADJUST)", 0x1E);
			break;
		case TRANS_UNDOOFF:
			PRINT_GB2312_xprint((unsigned char*)"离线撤销(UNDOOFF)", 0x1E);
			break;
		case TRANS_OFFPREAUTHFINISH:
			PRINT_GB2312_xprint((unsigned char*)"离线结算(OFFLINE)", 0x1E);
			break;
		case TRANS_OFFPURCHASE:
			if(G_NORMALTRANS_ucECTrans ==TRANS_EC_PURCHASE)
			{
				if((G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_HB_PUTCARD)||(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_SAND_PUTCARD))
					PRINT_GB2312_xprint((unsigned char*)"QPBOC脱机消费(SALE)", 0x1E);
				else
					PRINT_GB2312_xprint((unsigned char*)"EC脱机消费(OFFSALE)", 0x1E);
			}
			else
				PRINT_GB2312_xprint((unsigned char*)"脱机消费(OFFSALE)", 0x1E);
			break;
		case TRANS_VOIDOFFPURCHASE:
			PRINT_GB2312_xprint((unsigned char*)"脱机消费撤销(VOID)", 0x1E);
			break;
		case TRANS_PREAUTHSETTLE:
			PRINT_GB2312_xprint((unsigned char*)"预授权完成(离线)", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(AUTH SETTLEMENT)", 0x1E);
			break;
		case TRANS_PREAUTHADD:
			PRINT_GB2312_xprint((unsigned char*)"追加预授权", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(ADD_AUTH)", 0x1E);
			break;
		case TRANS_EC_ASSIGNLOAD:
			PRINT_GB2312_xprint((unsigned char*)"指定账户圈存", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(ASSIGN_LOAD)", 0x1E);
			break;
		case TRANS_EC_UNASSIGNLOAD:
			PRINT_GB2312_xprint((unsigned char*)"非指定账户圈存", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(UNASSIGN_LOAD)", 0x1E);
			break;
		case TRANS_EC_CASHLOAD:
			PRINT_GB2312_xprint((unsigned char*)"现金充值", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(CASH_LOAD)", 0x1E);
			break;
		case TRANS_MOBILEAUTH:
			PRINT_GB2312_xprint((unsigned char*)"手机无卡预约消费", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(MOBILE SALE)", 0x1E);
			break;
		case TRANS_VOIDMOBILEAUTH:
			PRINT_GB2312_xprint((unsigned char*)"手机预约撤销", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(VOID MOBILE)", 0x1E);
			break;
		case TRANS_MOBILEREFUND:
			PRINT_GB2312_xprint((unsigned char*)"手机预约退货", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"(MOBILE REFUND)", 0x1E);
			break;
		case TRANS_CREDITSALE:
			PRINT_GB2312_xprint((unsigned char*)"分期付款消费", 0x1E);		 
			PRINT_GB2312_xprint((unsigned char*)"(INSTALLMENT)", 0x1E);
			break;
		case TRANS_CREDITSVOID:
			PRINT_GB2312_xprint((unsigned char*)"分期付款撤消", 0x1E);
			PRINT_GB2312_xprint((unsigned char*) "VOID",0x1E);
			break;
		case TRANS_CREDITSREFUND:
			PRINT_GB2312_xprint((unsigned char*) "分期付款退货", 0x1E);
			PRINT_GB2312_xprint((unsigned char*)"REFUND",0x1E);
			break;
		default:
			PRINT_GB2312_xprint((unsigned char*)"未知交易类型", 0x1E);
			break;
	}
	return ;
}
void PRINT_transType_NEW(unsigned char *paucOutData,unsigned char *paucOutDataEN)
{
	unsigned char ucTransType,ucFont;

	ucTransType = G_NORMALTRANS_ucTransType;
	ucFont =0x1D;
	switch(ucTransType)
	{
		case TRANS_PURCHASE:
			if((G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_HB_PUTCARD)
			    ||(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_SAND_PUTCARD))
				strcpy((char *)paucOutData,"QPBOC消费");
			else if(G_NORMALTRANS_ucECTrans ==TRANS_EC_PURCHASE)
				strcpy((char *)paucOutData, "EC消费");
			else
				strcpy((char *)paucOutData, "消费");
			strcpy((char *)paucOutDataEN, "SALE");
	   	break;
		case TRANS_PREAUTH:
			strcpy((char *)paucOutData, "预授权");
			strcpy((char *)paucOutDataEN, "AUTH");
			break;
		case TRANS_PREAUTHFINISH:
			strcpy((char *)paucOutData, "预授权完成(联机)");
			strcpy((char *)paucOutDataEN, "AUTH COMPLETE");
			break;
		case TRANS_VOIDPURCHASE:
			strcpy((char *)paucOutData, "消费撤销");
			strcpy((char *)paucOutDataEN, "VOID");
			break;
		case TRANS_VOIDPREAUTH:
			strcpy((char *)paucOutData, "预授权撤销");
			strcpy((char *)paucOutDataEN, "CANCEL");
			break;
		case TRANS_VOIDPREAUTHFINISH:
			strcpy((char *)paucOutData, "预授权完成撤销");
			strcpy((char *)paucOutDataEN, "COMPLETE VOID");
			break;
		case TRANS_REFUND:
			strcpy((char *)paucOutData, "退货");
			strcpy((char *)paucOutDataEN, "REFUND");
			break;
		case TRANS_TIPADJUST:
		case TRANS_TIPADJUSTOK:
			strcpy((char *)paucOutData, "离线调整");
			strcpy((char *)paucOutDataEN, "ADJUST");
			break;
		case TRANS_UNDOOFF:
			strcpy((char *)paucOutData, "离线撤销");
			strcpy((char *)paucOutDataEN, "UNDOOFF");
			break;
		case TRANS_OFFPREAUTHFINISH:
			strcpy((char *)paucOutData, "离线结算");
			strcpy((char *)paucOutDataEN, "OFFLINE");
			break;
		case TRANS_OFFPURCHASE:
			if((G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_HB_PUTCARD)||(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_SAND_PUTCARD))
				strcpy((char *)paucOutData, "QPBOC脱机消费");
			else if(G_NORMALTRANS_ucECTrans ==TRANS_EC_PURCHASE)
				strcpy((char *)paucOutData, "EC脱机消费");
			else
				strcpy((char *)paucOutData, "脱机消费");
			strcpy((char *)paucOutDataEN, "OFFSALE");
			break;
		case TRANS_VOIDOFFPURCHASE:
			strcpy((char *)paucOutData, "脱机消费撤销");
			strcpy((char *)paucOutDataEN, "VOID");
			break;
		case TRANS_PREAUTHSETTLE:
			strcpy((char *)paucOutData, "预授权完成(离线)");
			strcpy((char *)paucOutDataEN, "AUTH SETTLEMENT");
			break;
		case TRANS_PREAUTHADD:
			strcpy((char *)paucOutData, "追加预授权");
			strcpy((char *)paucOutDataEN, "ADD AUTH");
			break;
		case TRANS_EC_ASSIGNLOAD:
			strcpy((char *)paucOutData, "指定账户圈存");
			strcpy((char *)paucOutDataEN, "ASSIGN LOAD");
			break;
		case TRANS_EC_UNASSIGNLOAD:
			strcpy((char *)paucOutData, "非指定账户圈存");
			strcpy((char *)paucOutDataEN, "UNASSIGN LOAD");
			break;
		case TRANS_EC_CASHLOAD:
			strcpy((char *)paucOutData, "现金充值");
			strcpy((char *)paucOutDataEN, "CASH LOAD");
			break;
		case TRANS_MOBILEAUTH:
			strcpy((char *)paucOutData, "手机无卡预约消费");
			strcpy((char *)paucOutDataEN, "CUPMOBILE SALE");
			break;
		case TRANS_VOIDMOBILEAUTH:
			strcpy((char *)paucOutData, "手机预约撤销");
			strcpy((char *)paucOutDataEN, "VOID  CUPMOBILE");
			break;
		case TRANS_MOBILEREFUND:
			strcpy((char *)paucOutData, "手机预约退货");
			strcpy((char *)paucOutDataEN, "CUPMOBILE REFUND");
			break;
		default:
			strcpy((char *)paucOutData, "未知交易类型");
			strcpy((char *)paucOutDataEN, "UNKNOW TYPE");
			break;
	}
	return ;
}


void PRINT_TransType_L(unsigned char Flag)
{
	unsigned char ucTransType;
	unsigned char ucPrintBuf[PRINT_MAXCHAR+1];
	unsigned char ucPrintBuf_en[PRINT_MAXCHAR+1];
	unsigned char ucFirstLen = 2;

	memset( ucPrintBuf, 0, sizeof(ucPrintBuf));
	memset( ucPrintBuf, ' ' , PRINT_MAXCHAR);
	memset( ucPrintBuf_en, 0, sizeof(ucPrintBuf_en));
	memset( ucPrintBuf_en, ' ' , PRINT_MAXCHAR);

	if ( Flag == 0)
		ucTransType = G_NORMALTRANS_ucTransType;
	else
		ucTransType = DataSave0.Trans_8583Data.ReversalTrans.ucTransType;
	switch(ucTransType)
	{
		case TRANS_PURCHASE:
			if((G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_HB_PUTCARD)
	    ||(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_SAND_PUTCARD))
				strcpy((char *)&ucPrintBuf[ucFirstLen],"QPBOC消费(SALE)");
			else if(G_NORMALTRANS_ucECTrans ==TRANS_EC_PURCHASE)
				strcpy((char *)&ucPrintBuf[ucFirstLen], "EC消费(SALE)");
			else
				strcpy((char *)&ucPrintBuf[ucFirstLen], "  消费(SALE)");
			//strcpy((char *)paucOutDataEN, "SALE");
			//strcpy((char*)&ucPrintBuf[ucFirstLen], "  消费(SALE)");
	   		break;
		case TRANS_CUPMOBILE:
			strcpy((char*)&ucPrintBuf[ucFirstLen], "  移动支付消费");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(MOBILE SALE)");
	   		break;
		case TRANS_VOIDCUPMOBILE:
			strcpy((char*)&ucPrintBuf[ucFirstLen], "  移动支付撤销");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(VOID MOBILE)");
	   		break;
		case TRANS_PREAUTH:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"      预授权");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"     (AUTH)");
			break;
		case TRANS_PREAUTHFINISH:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"  预授权完成");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(AUTH COMPLETE)");
			break;
		case TRANS_VOIDPURCHASE:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"    消费撤销");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"     (VOID)");
			break;
		case TRANS_VOIDPREAUTH:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"  预授权撤销");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(CANCEL)");
			break;
		case TRANS_VOIDPREAUTHFINISH:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"预授权完成撤销");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(COMPLETE VOID)");
			break;
		case TRANS_REFUND:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"  退货");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(REFUND)");
			break;
		case TRANS_TIPADJUST:
		case TRANS_TIPADJUSTOK:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"  离线调整");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(ADJUST)");
			break;
		case TRANS_OFFPREAUTHFINISH:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"  离线结算");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(OFFLINE)");
			break;
		case TRANS_UNDOOFF:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"  离线撤销");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(UNDOOFF)");
			break;
		case TRANS_OFFPURCHASE:
			if((G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_HB_PUTCARD)||(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_SAND_PUTCARD))
				strcpy((char *)&ucPrintBuf[ucFirstLen], "QPBOC脱机消费");
			else if(G_NORMALTRANS_ucECTrans ==TRANS_EC_PURCHASE)
				strcpy((char *)&ucPrintBuf[ucFirstLen], "EC脱机消费");
			else
				strcpy((char *)&ucPrintBuf[ucFirstLen], "脱机消费");
			strcpy((char *)&ucPrintBuf_en[ucFirstLen], "(OFFSALE)");
//			strcpy((char*)&ucPrintBuf[ucFirstLen],"脱机消费");
//			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(OFFSALE)");
			break;
		case TRANS_VOIDOFFPURCHASE:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"脱机消费撤销");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(OFFVOID)");
			break;
		case TRANS_PREAUTHSETTLE:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"预授权完成(离线)");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(AUTH SETTLEMENT)");
			break;
		case TRANS_PREAUTHADD:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"  追加预授权");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(ADD AUTH)");
			break;
		case TRANS_DEPOSIT:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"  圈存");
			strcpy((char*)&ucPrintBuf_en[ucFirstLen],"(DEPOSIT)");
			break;
		case TRANS_EC_ASSIGNLOAD:
			strcpy((char *)&ucPrintBuf[ucFirstLen], "指定账户圈存");
			strcpy((char *)&ucPrintBuf_en[ucFirstLen], "ASSIGN LOAD");
			break;
		case TRANS_EC_UNASSIGNLOAD:
			strcpy((char *)&ucPrintBuf[ucFirstLen], "非指定账户圈存");
			strcpy((char *)&ucPrintBuf_en[ucFirstLen], "UNASSIGN LOAD");
			break;
		case TRANS_EC_CASHLOAD:
			strcpy((char *)&ucPrintBuf[ucFirstLen], "现金充值");
			strcpy((char *)&ucPrintBuf_en[ucFirstLen], "CASH LOAD");
			break;
		default:
			strcpy((char*)&ucPrintBuf[ucFirstLen],"未知交易类型");
			break;
	}

	PRINT_GB2312_xprint(ucPrintBuf, FONT);
	PRINT_xprint(ucPrintBuf_en);

	PRINT_XReverseFeed(LINENUM*2);
	memset( ucPrintBuf, 0, sizeof(ucPrintBuf));
	memset( ucPrintBuf, ' ' , PRINT_MAXCHAR);
	if(memcmp(G_NORMALTRANS_ucExpiredDate,"\x00\x00\x00\x00",4))
	{
		ucFirstLen=ucFirstLen+3+10+8;
		memcpy(&ucPrintBuf[ucFirstLen],"20",2);
		ucFirstLen+=2;
		bcd_asc(&ucPrintBuf[ucFirstLen],G_NORMALTRANS_ucExpiredDate,2);
		ucFirstLen+=2;
		ucPrintBuf[ucFirstLen] = '/';
		ucFirstLen+=1;
		bcd_asc(&ucPrintBuf[ucFirstLen],&G_NORMALTRANS_ucExpiredDate[1],2);
	}
	PRINT_xprint(ucPrintBuf);
}


void PRINT_Revesal_Ticket(unsigned char flag)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1],aucTempBuf[10],aucTimeDate[10];
	unsigned char ucI;

	if(DataSave0.ConstantParamData.prnflag) FONT = 0x00;
//	PRINT_PrintMsg( FONT,MSG_SPDB);
	PRINT_GB2312_xprint((uchar *)"      故障报告单",0x1e);
	PRINT_xlinefeed(0.5*LINENUM);
	PRINT_GB2312_xprint((uchar *)"    未成功上送冲正交易",FONT);
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	if(DataSave0.ConstantParamData.prnflagOld||DataSave0.ConstantParamData.prnflagNew)
		PRINT_xlinefeed(2*LINENUM);
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"终端号:",7);
	memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);
/*----------------------------------------------------------------------------------*/
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户号:",7);
	memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);
/*----------------------------------------------------------------------------------*/
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户名:",7);
	memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMerchantName,20);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);
  	if(strlen((char *)DataSave0.ConstantParamData.aucMerchantName) >20)
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(&aucPrintBuf,&DataSave0.ConstantParamData.aucMerchantName[20],20);
		PRINT_GB2312_xprint(aucPrintBuf,FONT);
	}
/*----------------------------------------------------------------------------------*/
	/*print trans number*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"批次号:",7);
    	long_asc(&aucPrintBuf[7],6,&DataSave0.ChangeParamData.ulBatchNumber);
    	aucPrintBuf[24] = 0;
	PRINT_GB2312_xprint(aucPrintBuf,FONT);

	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"凭证号:",7);
    	long_asc(&aucPrintBuf[7],6,&DataSave0.Trans_8583Data.ReversalTrans.ulTraceNumber);
    	aucPrintBuf[24] = 0;
	PRINT_GB2312_xprint(aucPrintBuf,FONT);

/*----------------------------------------------------------------------------------*/
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");

	if (DataSave0.Trans_8583Data.ReversalTrans.ucSourceAccLen > 0)
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"卡号:",5);
		if( DataSave0.Trans_8583Data.ReversalTrans.ucSourceAccLen%2 )
		{
			bcd_asc(&aucPrintBuf[5],DataSave0.Trans_8583Data.ReversalTrans.aucSourceAcc,
					DataSave0.Trans_8583Data.ReversalTrans.ucSourceAccLen+1);
	    	aucPrintBuf[5+DataSave0.Trans_8583Data.ReversalTrans.ucSourceAccLen] = 0;
	    }
	    else
	    {
	    	bcd_asc(&aucPrintBuf[5],DataSave0.Trans_8583Data.ReversalTrans.aucSourceAcc,
					DataSave0.Trans_8583Data.ReversalTrans.ucSourceAccLen);
	    	aucPrintBuf[5+DataSave0.Trans_8583Data.ReversalTrans.ucSourceAccLen] = 0;
	    }
		PRINT_GB2312_xprint(aucPrintBuf,FONT);
	}
/*----------------------------------------------------------------------------------*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));

    	PRINT_GB2312_xprint((unsigned char *)"日期             时间",FONT);
	Os__read_date(aucTimeDate);
	asc_bcd(&aucTempBuf[1],1,&aucTimeDate[4],2);
	asc_bcd(&aucTempBuf[2],1,&aucTimeDate[2],2);
	asc_bcd(&aucTempBuf[3],1,&aucTimeDate[0],2);
	aucTempBuf[0]= 0x20;
    	UTIL_format_date_bcd_str(aucPrintBuf,aucTempBuf);

    	Os__read_time_sec(aucTimeDate);
	asc_bcd(aucTempBuf,3,aucTimeDate,6);
	aucTempBuf[3]= 0;
    	UTIL_format_time_bcd_str(&aucPrintBuf[16],aucTempBuf);
    	aucPrintBuf[24] = 0;
	PRINT_GB2312_xprint(aucPrintBuf,FONT);

/*----------------------------------------------------------------------------------*/
    /*print trans type*/
	PRINT_transType(1);
/*----------------------------------------------------------------------------------*/
	/*print amount*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	UTIL_Form_Montant(&aucPrintBuf[3],DataSave0.Trans_8583Data.ReversalTrans.ulAmount,2);
	for(ucI=0;ucI<11;ucI++)
	{
		if(aucPrintBuf[ucI+3] != ' ')
		{
			memcpy(&aucPrintBuf[ucI],"RMB",3);
			break;
		}
	}
	PRINT_PrintTwoMSG( FONT,MSG_AMOUNT,aucPrintBuf,15,-1,0,0);
	if(G_NORMALTRANS_ucTransType==TRANS_TIPADJUST
		||G_NORMALTRANS_ucTransType==TRANS_TIPADJUSTOK)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		UTIL_Form_Montant(&aucPrintBuf[3],G_NORMALTRANS_ulFeeAmount,2);
		for(ucI=0;ucI<11;ucI++)
		{
			if(aucPrintBuf[ucI+3] != ' ')
			{
				memcpy(&aucPrintBuf[ucI],"RMB",3);
				break;
			}
		}
		PRINT_PrintTwoMSG( 0x00,MSG_FEEAMOUNT,aucPrintBuf,15,-1,0,0);
		PRINT_GB2312_xprint((unsigned char*)"TIPS:",FONT);
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	/*----------------------------------------------------------------------------------*/
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		UTIL_Form_Montant(&aucPrintBuf[3],G_NORMALTRANS_ulAmount+G_NORMALTRANS_ulFeeAmount,2);
		for(ucI=0;ucI<11;ucI++)
		{
			if(aucPrintBuf[ucI+3] != ' ')
			{
				memcpy(&aucPrintBuf[ucI],"RMB",3);
				break;
			}
		}
		PRINT_PrintTwoMSG( FONT,MSG_TOTALAMOUNT,aucPrintBuf,15,-1,0,0);
		PRINT_GB2312_xprint((unsigned char*)"TOTAL:",FONT);
		PRINT_xlinefeed(1*LINENUM);
	}
	else
	{
		PRINT_GB2312_xprint((unsigned char*)"小费(TIPS):",FONT);
		PRINT_GB2312_xprint((unsigned char*)"总计(TOTAL):",FONT);

	}
/*----------------------------------------------------------------------------------*/
	/*print cardholder sign*/
	if(DataSave0.ChangeParamData.ucSuperLogonFlag==CASH_LOGONFLAG)
	{
		PRINT_GB2312_xprint((unsigned char *)"测试交易",FONT);
	}
	PRINT_GB2312_xprint((unsigned char *)"冲正不成功 请人工处理",FONT);
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	if(DataSave0.ConstantParamData.prnflagOld||DataSave0.ConstantParamData.prnflagNew)
		PRINT_xlinefeed(5*LINENUM);
	else
		PRINT_xlinefeed(1*LINENUM);
	if(DataSave0.ConstantParamData.prnflagZD)
	{
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
		PRINT_GB2312_xprint((unsigned char *)"本单据只做为故障处理凭据",FONT);
		PRINT_GB2312_xprint((unsigned char *)"****如做它用后果自负******",FONT);
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");

	}else
	{
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
		PRINT_GB2312_xprint((unsigned char *)"***本单据只做为故障处理凭据***",FONT);
		PRINT_GB2312_xprint((unsigned char *)"******如做它用后果自负******",FONT);
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	}
	if(DataSave0.ConstantParamData.prnflagOld||DataSave0.ConstantParamData.prnflagNew)
		PRINT_xlinefeed(11*LINENUM);
	else
	if(DataSave0.ConstantParamData.prnflagZD)
		PRINT_xlinefeed(2*LINENUM);
	else
		PRINT_xlinefeed(3*LINENUM);


}

void PRINT_OffTrans_Ticket(void)
{
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned char ucI;

	PRINT_GB2312_xprint((uchar *)"      故障报告单",0x1e);
	PRINT_xlinefeed(0.5*LINENUM);
	PRINT_GB2312_xprint((uchar *)"    未成功上送离线交易",FONT);
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");

	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"终端号:",7);
	memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucTerminalID,PARAM_TERMINALIDLEN);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);
/*----------------------------------------------------------------------------------*/
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户号:",7);
	memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMerchantID,PARAM_MERCHANTIDLEN);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);
/*----------------------------------------------------------------------------------*/
	memset(aucPrintBuf,0,sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"商户名:",7);
	memcpy(&aucPrintBuf[7],DataSave0.ConstantParamData.aucMerchantName,20);
	PRINT_GB2312_xprint(aucPrintBuf,FONT);
  	if(strlen((char *)DataSave0.ConstantParamData.aucMerchantName) >20)
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(&aucPrintBuf,&DataSave0.ConstantParamData.aucMerchantName[20],20);
		PRINT_GB2312_xprint(aucPrintBuf,FONT);
	}
/*----------------------------------------------------------------------------------*/
	/*print trans number*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"批次号:",7);
    	long_asc(&aucPrintBuf[7],6,&DataSave0.ChangeParamData.ulBatchNumber);
    	aucPrintBuf[24] = 0;
	PRINT_GB2312_xprint(aucPrintBuf,FONT);

	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	memcpy(aucPrintBuf,"凭证号:",7);
    	long_asc(&aucPrintBuf[7],6,&G_NORMALTRANS_ulTraceNumber);
    	aucPrintBuf[24] = 0;
	PRINT_GB2312_xprint(aucPrintBuf,FONT);

/*----------------------------------------------------------------------------------*/
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");

	if (G_NORMALTRANS_ucSourceAccLen > 0)
	{
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"卡号:",5);
		if( G_NORMALTRANS_ucSourceAccLen%2 )
		{
			bcd_asc(&aucPrintBuf[5],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
	    	aucPrintBuf[5+G_NORMALTRANS_ucSourceAccLen] = 0;
	    }
	    else
	    {
	    	bcd_asc(&aucPrintBuf[5],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
	    	aucPrintBuf[5+G_NORMALTRANS_ucSourceAccLen] = 0;
	    }
		PRINT_GB2312_xprint(aucPrintBuf,FONT);
	}
/*----------------------------------------------------------------------------------*/
    /*print trans type*/
	PRINT_transType(0);
/*----------------------------------------------------------------------------------*/
	/*print amount*/
	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	UTIL_Form_Montant(&aucPrintBuf[3],G_NORMALTRANS_ulAmount,2);
	for(ucI=0;ucI<11;ucI++)
	{
		if(aucPrintBuf[ucI+3] != ' ')
		{
			memcpy(&aucPrintBuf[ucI],"RMB",3);
			break;
		}
	}
	PRINT_PrintTwoMSG( FONT,MSG_AMOUNT,aucPrintBuf,15,-1,0,0);
	if(G_NORMALTRANS_ucTransType==TRANS_TIPADJUST
		||G_NORMALTRANS_ucTransType==TRANS_TIPADJUSTOK)
	{
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		UTIL_Form_Montant(&aucPrintBuf[3],G_NORMALTRANS_ulFeeAmount,2);
		for(ucI=0;ucI<11;ucI++)
		{
			if(aucPrintBuf[ucI+3] != ' ')
			{
				memcpy(&aucPrintBuf[ucI],"RMB",3);
				break;
			}
		}
		PRINT_PrintTwoMSG( 0x00,MSG_FEEAMOUNT,aucPrintBuf,15,-1,0,0);
		PRINT_GB2312_xprint((unsigned char*)"TIPS:",FONT);
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	/*----------------------------------------------------------------------------------*/
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		UTIL_Form_Montant(&aucPrintBuf[3],G_NORMALTRANS_ulAmount+G_NORMALTRANS_ulFeeAmount,2);
		for(ucI=0;ucI<11;ucI++)
		{
			if(aucPrintBuf[ucI+3] != ' ')
			{
				memcpy(&aucPrintBuf[ucI],"RMB",3);
				break;
			}
		}
		PRINT_PrintTwoMSG( FONT,MSG_TOTALAMOUNT,aucPrintBuf,15,-1,0,0);
		PRINT_GB2312_xprint((unsigned char*)"TOTAL:",FONT);
		PRINT_xlinefeed(1*LINENUM);
	}
	else
	{
		PRINT_GB2312_xprint((unsigned char*)"小费(TIPS):",FONT);
		PRINT_GB2312_xprint((unsigned char*)"总计(TOTAL):",FONT);

	}
/*----------------------------------------------------------------------------------*/
	PRINT_GB2312_xprint((unsigned char *)"交易不成功 请人工处理",FONT);
	PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	if(DataSave0.ConstantParamData.prnflagOld||DataSave0.ConstantParamData.prnflagNew)
		PRINT_xlinefeed(9*LINENUM);
	else
		PRINT_xlinefeed(1*LINENUM);
	if(DataSave0.ConstantParamData.prnflagZD)
	{
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
		PRINT_GB2312_xprint((unsigned char *)"本单据只做为故障处理凭据",FONT);
		PRINT_GB2312_xprint((unsigned char *)"****如做它用后果自负******",FONT);
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");

	}else
	{
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
		PRINT_GB2312_xprint((unsigned char *)"***本单据只做为故障处理凭据***",FONT);
		PRINT_GB2312_xprint((unsigned char *)"******如做它用后果自负******",FONT);
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	}
	if(DataSave0.ConstantParamData.prnflagOld||DataSave0.ConstantParamData.prnflagNew)
		PRINT_xlinefeed(11*LINENUM);
	else
	if(DataSave0.ConstantParamData.prnflagZD)
		PRINT_xlinefeed(2*LINENUM);
	else
		PRINT_xlinefeed(3*LINENUM);

}

void PRINT_SettleTotal(unsigned char flag)
{
	TRANSTOTAL	*pTransTotal;
	TRANSTOTAL	*pForeignTransTotal;
	unsigned int  uitotalnb;
	unsigned long ultotalamount;
	unsigned char aucBuf[25];

	XDATA_Read_Vaild_File(DataSaveTransInfo);
	if(((flag&PRINT_SETTLE_FLAG)==PRINT_SETTLE_FLAG)&&((flag&RE_PRINT_FLAG)==RE_PRINT_FLAG))
	{
		pTransTotal = &DataSave0.TransInfoData.LastTransTotal;
		pForeignTransTotal = &DataSave0.TransInfoData.LastForeignTransTotal;
	}
	else
	{
		pTransTotal = &DataSave0.TransInfoData.TransTotal;
		pForeignTransTotal = &DataSave0.TransInfoData.ForeignTransTotal;
	}
	if(flag==PRINT_TOTAL_FLAG)
		PRINT_GB2312_xprint((unsigned char *)"----终端统计打印----",0X1E);

	PRINT_GB2312_xprint((unsigned char *)"总笔数          总金额",FONT);
	PRINT_GB2312_xprint((unsigned char *)"贷记总计(内卡)",FONT);
	uitotalnb = pTransTotal->uiCreditNb;
	ultotalamount = pTransTotal->ulCreditAmount;
	memset(aucBuf,' ',sizeof(aucBuf));
	int_asc(aucBuf,3,&uitotalnb);
	UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
	PRINT_GB2312_xprint(aucBuf,FONT);

	PRINT_GB2312_xprint((unsigned char *)"借记总计(内卡)",FONT);
	uitotalnb = pTransTotal->uiDebitNb;
	ultotalamount = pTransTotal->ulDebitAmount;
	memset(aucBuf,' ',sizeof(aucBuf));
	int_asc(aucBuf,3,&uitotalnb);
	UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
	PRINT_GB2312_xprint(aucBuf,FONT);


	PRINT_GB2312_xprint((unsigned char *)"借记- 贷记(内卡)",FONT);
	if(pTransTotal->uiDebitNb >= pTransTotal->uiCreditNb)
		uitotalnb = pTransTotal->uiDebitNb - pTransTotal->uiCreditNb;
	else
		uitotalnb = pTransTotal->uiCreditNb - pTransTotal->uiDebitNb ;

	if(pTransTotal->ulDebitAmount >= pTransTotal->ulCreditAmount)
		ultotalamount = pTransTotal->ulDebitAmount - pTransTotal->ulCreditAmount;
	else
		ultotalamount = pTransTotal->ulCreditAmount - pTransTotal->ulDebitAmount;

	memset(aucBuf,' ',sizeof(aucBuf));
	if(pTransTotal->uiDebitNb >= pTransTotal->uiCreditNb)
		int_asc(aucBuf,3,&uitotalnb);
	else
	{
		aucBuf[0]='-';
		int_asc(&aucBuf[1],3,&uitotalnb);
	}
	if(pTransTotal->ulDebitAmount >= pTransTotal->ulCreditAmount)
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
	else
	{
		aucBuf[9]='-';
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
	}
	PRINT_GB2312_xprint(aucBuf,FONT);

	PRINT_GB2312_xprint((unsigned char *)"贷记总计(外卡)",FONT);
	uitotalnb = pForeignTransTotal->uiCreditNb;
	ultotalamount = pForeignTransTotal->ulCreditAmount;
	memset(aucBuf,' ',sizeof(aucBuf));
	int_asc(aucBuf,3,&uitotalnb);
	UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
	PRINT_GB2312_xprint(aucBuf,FONT);

	PRINT_GB2312_xprint((unsigned char *)"借记总计(外卡)",FONT);
	uitotalnb = pForeignTransTotal->uiDebitNb;
	ultotalamount = pForeignTransTotal->ulDebitAmount;
	memset(aucBuf,' ',sizeof(aucBuf));
	int_asc(aucBuf,3,&uitotalnb);
	UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
	PRINT_GB2312_xprint(aucBuf,FONT);

	PRINT_GB2312_xprint((unsigned char *)"借记- 贷记(外卡)",FONT);
	if(pForeignTransTotal->uiDebitNb >= pForeignTransTotal->uiCreditNb)
		uitotalnb = pForeignTransTotal->uiDebitNb - pForeignTransTotal->uiCreditNb;
	else
		uitotalnb = pForeignTransTotal->uiCreditNb - pForeignTransTotal->uiDebitNb ;

	if(pForeignTransTotal->ulDebitAmount >= pForeignTransTotal->ulCreditAmount)
		ultotalamount = pForeignTransTotal->ulDebitAmount - pForeignTransTotal->ulCreditAmount;
	else
		ultotalamount = pForeignTransTotal->ulCreditAmount - pForeignTransTotal->ulDebitAmount;
	memset(aucBuf,' ',sizeof(aucBuf));
	if(pForeignTransTotal->uiDebitNb >= pForeignTransTotal->uiCreditNb)
		int_asc(aucBuf,3,&uitotalnb);
	else
	{
		aucBuf[0]='-';
		int_asc(&aucBuf[1],3,&uitotalnb);
	}
	if(pForeignTransTotal->ulDebitAmount >= pForeignTransTotal->ulCreditAmount)
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
	else
	{
		aucBuf[9]='-';
		UTIL_Form_Montant(&aucBuf[10],ultotalamount,2);
	}
	PRINT_GB2312_xprint(aucBuf,FONT);

	PRINT_xlinefeed(LINENUM);

}
void PRINT_OfflineDetial(void)
{
   	unsigned int uiIndex;
	unsigned char ucI;
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned char PrintFlag;

	if(DataSave0.ConstantParamData.prnflag) FONT = 0x00;
	PrintFlag =1;

	XDATA_Read_Vaild_File(DataSaveTransInfo);
	for(uiIndex=0;uiIndex<TRANS_MAXNB;uiIndex++)
	{
		if(uiIndex>=(DataSave0.TransInfoData.ForeignTransTotal.uiTotalNb+
			   		 DataSave0.TransInfoData.TransTotal.uiTotalNb))
			break;
		if( DataSave0.TransInfoData.auiTransIndex[uiIndex])
		{
			if((DataSave0.TransInfoData.auiTransIndex[uiIndex]&TRANS_NIIVOID)==TRANS_NIIVOID)
				continue;
			GetOldTransData(uiIndex);
			OSMEM_Memcpy((unsigned char *)&(DataSave1.SaveTrans),
							(unsigned char *)&NormalTransData,
							sizeof(NORMALTRANS));

		       if( (G_NORMALTRANS_ucTransType != TRANS_TIPADJUST)
	    	         &&(G_NORMALTRANS_ucTransType != TRANS_TIPADJUSTOK)
   		   	  &&(G_NORMALTRANS_ucTransType != TRANS_OFFPREAUTHFINISH)
   		         )
   		    		continue;
		       if(G_NORMALTRANS_ucNotSendOkFlag == false)
		    		continue;
			if(PrintFlag)
			{
				PRINT_GB2312_xprint((unsigned char *)"未上送成功脱机明细",FONT);
				PRINT_xlinefeed(1*LINENUM);
				PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
				PrintFlag = 0;
			}
	//		memcpy(aucPrintBuf,"批次号:",7);
	//		long_asc(&aucPrintBuf[7],6,&DataSave0.ChangeParamData.ulBatchNumber);
	//		aucPrintBuf[13] = ' ';
	//		memcpy(&aucPrintBuf[14],"凭证号:",7);
	//		long_asc(&aucPrintBuf[21],6,&G_NORMALTRANS_ulTraceNumber);
	//		aucPrintBuf[27] = 0;
	//		PRINT_GB2312_xprint(aucPrintBuf,FONT);

			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memcpy(aucPrintBuf,"批次号         凭证号",24);
			aucPrintBuf[24] = 0;
			PRINT_GB2312_xprint(aucPrintBuf,FONT);
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		       long_asc(aucPrintBuf,6,&DataSave0.ChangeParamData.ulBatchNumber);
		       long_asc(&aucPrintBuf[15],6,&G_NORMALTRANS_ulTraceNumber);
		       aucPrintBuf[24] = 0;
			PRINT_xprint(aucPrintBuf);

			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memcpy(aucPrintBuf,"卡号:",5);
			if(G_NORMALTRANS_ucSourceAccLen%2)
			{
				bcd_asc(&aucPrintBuf[5],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen+1);
			}else
			{
				bcd_asc(&aucPrintBuf[5],G_NORMALTRANS_aucSourceAcc,G_NORMALTRANS_ucSourceAccLen);
			}
			aucPrintBuf[5+G_NORMALTRANS_ucSourceAccLen] = 0;
			PRINT_GB2312_xprint(aucPrintBuf,FONT);

			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			memcpy(aucPrintBuf,"日期-时间:",10);
			UTIL_format_date_bcd_str(&aucPrintBuf[10],G_NORMALTRANS_aucDate);
			UTIL_format_time_bcd_str(&aucPrintBuf[22],G_NORMALTRANS_aucTime);
			aucPrintBuf[30] = 0;
			PRINT_GB2312_xprint(aucPrintBuf,FONT);

	              switch(G_NORMALTRANS_ucTransType)
	              {
	        		case TRANS_TIPADJUST:
	        		case TRANS_TIPADJUSTOK:
	        			PRINT_PrintMsg( FONT,MSG_TIPADJUST);
	        			break;
	        		case TRANS_OFFPREAUTHFINISH:
	        			PRINT_PrintMsg( FONT,MSG_OFFPREAUTHFINISH);
	        			break;
	            		default:
	            			break;
	              }
	           	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	        	UTIL_Form_Montant(&aucPrintBuf[3],G_NORMALTRANS_ulAmount,2);
	        	for(ucI=0;ucI<10;ucI++)
	        	{
	        		if(aucPrintBuf[ucI+3] != ' ')
	        		{
	        			memcpy(&aucPrintBuf[ucI],"RMB",3);
	        			break;
	        		}
	        	}
	        	PRINT_PrintTwoMSG( 0x1E,MSG_AMOUNT,aucPrintBuf,14,-1,0,0);
	        /*----------------------------------------------------------------------------------*/
	        	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	        	UTIL_Form_Montant(&aucPrintBuf[3],G_NORMALTRANS_ulFeeAmount,2);
	        	for(ucI=0;ucI<10;ucI++)
	        	{
	        		if(aucPrintBuf[ucI+3] != ' ')
	        		{
	        			memcpy(&aucPrintBuf[ucI],"RMB",3);
	        			break;
	        		}
	        	}
	        	PRINT_PrintTwoMSG( 0x00,MSG_FEEAMOUNT,aucPrintBuf,14,-1,0,0);
	        /*----------------------------------------------------------------------------------*/
	        	memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
	        	UTIL_Form_Montant(&aucPrintBuf[3],G_NORMALTRANS_ulAmount+G_NORMALTRANS_ulFeeAmount,2);
	        	for(ucI=0;ucI<10;ucI++)
	        	{
	        		if(aucPrintBuf[ucI+3] != ' ')
	        		{
	        			memcpy(&aucPrintBuf[ucI],"RMB",3);
	        			break;
	        		}
	        	}
	        	PRINT_PrintTwoMSG( 0x00,MSG_TOTALAMOUNT,aucPrintBuf,14,-1,0,0);
	       		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
		        /*----------------------------------------------------------------------------------*/

		}
    }
	#ifdef PS300
   	PRINT_xlinefeed(5*LINENUM);
	#endif
}


unsigned char PRINT_GetReadCardMethord(void)
{
	unsigned char ucReadCardMethord;
	util_Printf("卡类型:%02x\n",G_NORMALTRANS_euCardSpecies);
	if(G_NORMALTRANS_ucFallBackFlag)
			ucReadCardMethord='F';
	else
	if(G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV)
	{
		if(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_MANUALINPUT)
			ucReadCardMethord='M';
		else
		if(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_SWIPECARD)
			ucReadCardMethord='S';
		else
		if(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_INSERTCARD)
			ucReadCardMethord='C';
		else
			ucReadCardMethord='I';
	}
	else
	if(G_NORMALTRANS_euCardSpecies==CARDSPECIES_MAG)
	{
		if(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_MANUALINPUT)
			ucReadCardMethord='M';
		else
		if(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_SWIPECARD)
			ucReadCardMethord='S';
		else
		if(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_INSERTCARD)
			ucReadCardMethord='C';
		else
			ucReadCardMethord='M';
	}
	else if (G_NORMALTRANS_euCardSpecies==CARDSPECIES_UNTOUCH)
	{
		if(G_NORMALTRANS_ucInputMode == TRANS_INPUTMODE_PUTCARD)
			ucReadCardMethord='P';
	}
	else ucReadCardMethord='U';

	return ucReadCardMethord;

}

void PRINT_version_Heat(void)
{
    unsigned char aucPrintBuf1[PRINT_MAXCHAR+1];
	unsigned char len,index;

	index=0;
	memset(aucPrintBuf1,' ',sizeof(aucPrintBuf1));
	aucPrintBuf1[PRINT_MAXCHAR] = '\0';
	len = (uchar )strlen((char *)DataSave0.ConstantParamData.APP_Project);
	memcpy(aucPrintBuf1, DataSave0.ConstantParamData.APP_Project, len);

	index+=len+2;
	len = (uchar )strlen((char *)DataSave0.ConstantParamData.SOFTWARE_Version);
	memcpy(&aucPrintBuf1[index],DataSave0.ConstantParamData.SOFTWARE_Version,len);
	index+=len+2;
	if (ReaderSupport)
		memcpy(&aucPrintBuf1[index],"PBOC",4);
	else
		memcpy(&aucPrintBuf1[index],"EMV",3);
	index+=4;
	memcpy(&aucPrintBuf1[index+1], __DATE__,sizeof(__DATE__) );
	PRINT_GB2312_xprint(aucPrintBuf1,0x01);

}


void PRINT_version(void)
{
	if(DataSave0.ConstantParamData.prnflag)
	{
		PRINT_version_Heat();
	}
}

void PRINT_remark(void)
{
	unsigned char aucPrintBuf[64],uiPrintLen;

	if (G_NORMALTRANS_ucCardRemarkFlag)
	{
		uiPrintLen = strlen((char*)G_NORMALTRANS_aucCardRemark);
		util_Printf("广告长度:%d\n",uiPrintLen);
		if (uiPrintLen>60)	uiPrintLen=60;

		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memset(aucPrintBuf, ' ',sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,G_NORMALTRANS_aucCardRemark,20);
		memcpy(&aucPrintBuf[20],&G_NORMALTRANS_aucCardRemark[20],20);		
		
		util_Printf("广告:%02x,%02x\n",aucPrintBuf[0],aucPrintBuf[20]);
		
		if (aucPrintBuf[0] != 0x20 || aucPrintBuf[20] != 0x20)
			PrintGB2312Info(aucPrintBuf,strlen((char*)aucPrintBuf),0x05);
			
		if (uiPrintLen>41)
		{
			memset(aucPrintBuf,0,sizeof(aucPrintBuf));
			memset(aucPrintBuf, ' ',sizeof(aucPrintBuf));
			memcpy(aucPrintBuf,&G_NORMALTRANS_aucCardRemark[40],20);
			util_Printf("广告3:%02x\n",aucPrintBuf[0]);
			if (aucPrintBuf[0] != 0x20)
				PrintGB2312Info(aucPrintBuf,strlen((char*)aucPrintBuf),0x05);
		}
	}
}

unsigned char PRINT_GB2312_xprint(unsigned char * pucBuffer, unsigned char ucFont )
{
	unsigned char ucResult = SUCCESS;

#if PS400|PS100
	ucResult = Os__GB2312_xprint(pucBuffer,ucFont);
#endif

#if PS300
	ucResult = PRINT_ext_GB2312_xprint(pucBuffer,ucFont);
#endif
	return ucResult;
}

unsigned char PRINT_xprint(unsigned char * pucBuffer)
{
	unsigned char ucResult = SUCCESS;

#if PS400|PS100
	ucResult = Os__xprint(pucBuffer);
#endif

#if PS300
	ucResult = PRINT_ext_xprint(pucBuffer);
#endif
	return ucResult;
}

unsigned char PRINT_xlinefeed(unsigned char ucLineNum)
{
	unsigned char ucResult = SUCCESS;

#if PS400|PS100
	ucResult = Os__xlinefeed(ucLineNum);
#endif

#if PS300
	ucResult =PRINT_ext_xlinefeed(ucLineNum);
#endif

	return ucResult;
}

unsigned char PRINT_XReverseFeed(unsigned char ucLineNum )
{
	unsigned char ucResult = SUCCESS;

#if PS400|PS100
	ucResult =OSPRN_XReverseFeed(ucLineNum);
#else
#if PS300
	PRINT_ext_xlinereverse(ucLineNum);
#endif
#endif
	return ucResult;
}

void PRINT_EveryCardDetail(unsigned char printFlag)
{
	unsigned char aucPrintBuf[30],aucTmpBuf[15];
	unsigned long ulTotalAmount;
	unsigned int  uiTotalNb;
	unsigned long ulTotalFee;
	TRANSDETAIL		*Pt;
	unsigned int  transTypeIndex;
	UINT_C51	uiTempRate;
	ULONG_C51	ulTotalTrueAmount;

	if (DataSave0.ConstantParamData.prnflagZD)
	{
		PRINT_GB2312_xprint((unsigned char *)"*****DETAIL REPORT********",FONT);
		PRINT_GB2312_xprint((unsigned char *)"笔数       金额     小费",FONT);
	}
	else
	{
		PRINT_GB2312_xprint((unsigned char *)"********DETAIL REPORT********",FONT);
		PRINT_GB2312_xprint((unsigned char *)"笔数    金额   小费",FONT);
	}

	util_Printf("PRINT_EveryCardDetail()----1-------printFlag=%02x\n",printFlag);
	if(printFlag == RE_PRINT_FLAG)
	{
		Pt = DataSave0.TransInfoData.LastTransDetail;
	}
	else
	{
		Pt = DataSave0.TransInfoData.TransDetail;
	}
	for(transTypeIndex = 0;transTypeIndex<6;transTypeIndex++)
	{
		switch(transTypeIndex)
		{
			case 0:
				PRINT_GB2312_xprint((unsigned char *)"RMB",FONT);
				break;
			case 1:
				PRINT_GB2312_xprint((unsigned char *)"VISA",FONT);
				break;
			case 2:
				PRINT_GB2312_xprint((unsigned char *)"MASTER",FONT);
				break;
			case 3:
				PRINT_GB2312_xprint((unsigned char *)"JCB",FONT);
				break;
			case 4:
				PRINT_GB2312_xprint((unsigned char *)"DINNER",FONT);
				break;
			case 5:
				PRINT_GB2312_xprint((unsigned char *)"AMEX",FONT);
				break;
			default:
				PRINT_GB2312_xprint((unsigned char *)"foreign card",FONT);
				break;
		}
		memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
		uiTotalNb = Pt[transTypeIndex].uiTotalNb ;
		ulTotalAmount = Pt[transTypeIndex].ulAmount;
		ulTotalFee	=	Pt[transTypeIndex].ulTipAmount;
		int_asc(aucPrintBuf,3,&uiTotalNb);
		UTIL_Form_Montant_Total(aucTmpBuf,ulTotalAmount,2);
		memcpy(&aucPrintBuf[3],aucTmpBuf,12);//10
		UTIL_Form_Montant(aucTmpBuf,ulTotalFee,2);
		memcpy(&aucPrintBuf[15],&aucTmpBuf[2],13);//13,10
		aucPrintBuf[28]=0;//24
		if(DataSave0.ConstantParamData.prnflag)
			PRINT_GB2312_xprint(aucPrintBuf,FONT);
		else
			PRINT_xprint(aucPrintBuf);

	}
	if(DataSave0.ConstantParamData.ucPrint==0x31)
	{
		if (DataSave0.ConstantParamData.prnflagZD)
		{
			PRINT_GB2312_xprint((unsigned char *)"******TRUE AMOUNT*********",FONT);
			PRINT_GB2312_xprint((unsigned char *)"笔数   扣率         净金额",FONT);
		}
		else
		{
			PRINT_GB2312_xprint((unsigned char *)"*********TRUE AMOUNT*********",FONT);
			PRINT_GB2312_xprint((unsigned char *)"笔数  扣率   净金额",FONT);
		}
		ulTotalTrueAmount = 0;
		for(transTypeIndex = 0;transTypeIndex<6;transTypeIndex++)
		{
			switch(transTypeIndex)
			{
				case 0:
					uiTempRate=DataSave0.ConstantParamData.ulRMBRate;
					PRINT_GB2312_xprint((unsigned char *)"RMB",FONT);
					break;
				case 1:
					uiTempRate = DataSave0.ConstantParamData.ulVISARate;
					PRINT_GB2312_xprint((unsigned char *)"VISA",FONT);
					break;
				case 2:
					uiTempRate = DataSave0.ConstantParamData.ulMASTERRate;
					PRINT_GB2312_xprint((unsigned char *)"MASTER",FONT);
					break;
				case 3:
					uiTempRate = DataSave0.ConstantParamData.ulJCBRate;
					PRINT_GB2312_xprint((unsigned char *)"JCB",FONT);
					break;
				case 4:
					uiTempRate = DataSave0.ConstantParamData.ulDINERRate;
					PRINT_GB2312_xprint((unsigned char *)"DINNER",FONT);
					break;
				case 5:
					uiTempRate = DataSave0.ConstantParamData.ulAMEXRate;
					PRINT_GB2312_xprint((unsigned char *)"AMEX",FONT);
					break;
				default:
					PRINT_GB2312_xprint((unsigned char *)"foreign card",FONT);
					break;
			}
			ulTotalTrueAmount = Pt[transTypeIndex].ulTrueAmount;
			uiTotalNb = Pt[transTypeIndex].uiTotalNb ;
			memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
			int_asc(aucPrintBuf,3,&uiTotalNb);
			sprintf((char *)&aucPrintBuf[7],"%u.%02u%%",uiTempRate/100,uiTempRate%100);//8
			memset(aucTmpBuf,'0',sizeof(aucTmpBuf));
			memset(&aucPrintBuf[12],' ',1);
			UTIL_Form_Montant_Total(aucTmpBuf,ulTotalTrueAmount,2);
			memcpy(&aucPrintBuf[12],&aucTmpBuf[0],13);//13,10
			aucPrintBuf[28]=0;
			if(DataSave0.ConstantParamData.prnflag)
				PRINT_GB2312_xprint(aucPrintBuf,FONT);
			else
				PRINT_xprint(aucPrintBuf);
		}
	}
	else
	{
		if(DataSave0.ConstantParamData.prnflagOld
			||DataSave0.ConstantParamData.prnflagNew)
			PRINT_xlinefeed(9*LINENUM);
	}

	return ;

}

#if PS300
unsigned char Print_CheckBase(void)
{
	unsigned char flag ;

	flag = 0;
	do{
		if( Os__CheckBase()==SUCCESS )
		{
			//Os__clr_display(255);
			//Os__GB2312_display(1,0,(uchar *)"    正在打印");
			//Os__GB2312_display(2,0,(uchar *)"    请稍等...");
			return SUCCESS;
		}
		else
		{
			if(!flag)
			{
				flag=1;
				//Os__clr_display(255);
				//Os__GB2312_display(1,0,(uchar *)"请放回座机！");
			}
		}
	}while(1);

	return ERR_END;
}


void PRINT_ext_xlinereverse(uchar nbline)
{
/*	if( DataSave0.ConstantParamData.ucLReFing == 0x01 )
		return;
	DataSave0.ConstantParamData.ucLReFing = Os__CheckBase();
	CFG_WriteConstantParamDataChecksum(true);*/
	if( Print_CheckBase()==SUCCESS )
	{
		ext__xlineReverse(nbline);
	}
}

unsigned char PRINT_ext_GB2312_xprint(unsigned char *pucPtr, unsigned char ucGBFont)
{
	unsigned char ucResult;
/*	if( DataSave0.ConstantParamData.ucLReFing == 0x01 )
		return;
	DataSave0.ConstantParamData.ucLReFing = Os__CheckBase();
	CFG_WriteConstantParamDataChecksum(true);*/
	if( Print_CheckBase()==SUCCESS )
	{
		ucResult = ext__GB2312_xprint(pucPtr,ucGBFont);
		return ucResult;
	}
	return SUCCESS;
}

unsigned char PRINT_ext_xlinefeed(uchar nbline)
{
	if( Print_CheckBase()==SUCCESS )
	{
		ext__xlinefeed(nbline);
	}
}

unsigned char PRINT_ext_xprint(uchar *buffer)
{

	unsigned char ucResult;
/*	if( DataSave0.ConstantParamData.ucLReFing == 0x01 )
		return;
	DataSave0.ConstantParamData.ucLReFing = Os__CheckBase();
	CFG_WriteConstantParamDataChecksum(true);*/
	if( Print_CheckBase()==SUCCESS )
	{
		ucResult = ext__xprint(buffer);
		return ucResult;
	}
	return SUCCESS;
}

#endif

unsigned char PRINT_DFDatalist(void)
{
    unsigned char ucResult = SUCCESS;
    unsigned int  uiIndex;
	unsigned char aucPrintBuf[PRINT_MAXCHAR+1];
	unsigned char ucI,uiPrintLen;

#if 1
	if(ucResult == SUCCESS)
	{
		PRINT_GB2312_xprint((unsigned char*)"     POS 备份数据单",0x1E);
		PRINT_xlinefeed(1*LINENUM);
		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");

		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"终端号:",7);
		memcpy(&aucPrintBuf[7],G_RUNDATA_aucTerminalID,PARAM_TERMINALIDLEN);
		PRINT_GB2312_xprint(aucPrintBuf,FONT);

		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"商户号:  ",8);
		memcpy(&aucPrintBuf[7],G_RUNDATA_aucMerchantID,PARAM_MERCHANTIDLEN);
		PRINT_GB2312_xprint(aucPrintBuf,FONT);

		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		memcpy(aucPrintBuf,"商户名:",7);
		PRINT_GB2312_xprint(aucPrintBuf,FONT);
		memset(aucPrintBuf,0,sizeof(aucPrintBuf));
		uiPrintLen = strlen((char*)DataSave0.ConstantParamData.aucMerchantName);
		if(uiPrintLen>PARAM_MERCHANTNAMELEN)
			uiPrintLen = PARAM_MERCHANTNAMELEN;
		memcpy(aucPrintBuf,DataSave0.ConstantParamData.aucMerchantName,uiPrintLen);
		PrintGB2312Info(aucPrintBuf,uiPrintLen,0x01);

		PRINT_GB2312_xprint((unsigned char *)"运单号            金额 ",FONT);

		PRINT_xprint((unsigned char *)"----------------------------------------------------------------");
	}
#endif
	if(ucResult == SUCCESS)
	{
		for(uiIndex = 0;uiIndex<DataSave0.ChangeParamData.uiSendTotalNb;uiIndex++)
		{
			util_Printf("uiIndex = %d\n",uiIndex);
			XDATA_Read_CollectTrans_File(uiIndex);
			if(DataSave0.ConstantParamData.ucCollectFlag =='1')
			{
				memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
				memcpy(&aucPrintBuf[0],DataSave0.SaveTrans1.aucOrdersNo,10);
			}
			if((DataSave0.SaveTrans1.ucTransType== TRANS_VOIDPURCHASE)
				||(DataSave0.SaveTrans1.ucTransType==TRANS_VOIDPREAUTHFINISH)
				||(DataSave0.SaveTrans1.ucTransType== TRANS_VOIDPREAUTH)
			  )	//撤销
			{
				UTIL_Form_Montant(&aucPrintBuf[13],DataSave0.SaveTrans1.ulAmount,2);
				for(ucI = 13;ucI < PRINT_MAXCHAR;ucI++)
				{
					if(aucPrintBuf[ucI] != ' ')
					{
						aucPrintBuf[ucI-1] = '-';
						break;
					}
				}
				PRINT_GB2312_xprint(aucPrintBuf,FONT);
			}else
			{
					memset(aucPrintBuf,' ',sizeof(aucPrintBuf));
					memcpy(&aucPrintBuf[0],DataSave0.SaveTrans1.aucOrdersNo,10);
					UTIL_Form_Montant(&aucPrintBuf[13],DataSave0.SaveTrans1.ulAmount,2);
					PRINT_GB2312_xprint(aucPrintBuf,FONT);
			}
	    }
	    PRINT_xprint((unsigned char *)"*****************************************************************");
		PRINT_xlinefeed(LINENUM*3);
	}
	return SUCCESS;
}



void PRINT_tts(void)
{
	PRINT_xprint((unsigned char *)"-------Begin---------------");
	PRINT_xprint((unsigned char *)"MyThread Print test.");
	PRINT_xprint((unsigned char *)"上海山的测试");
	PRINT_xprint((unsigned char *)"ABCDEGAAFSDAF");
	PRINT_xprint((unsigned char *)"12345464548789797");
	PRINT_xprint((unsigned char *)"!@#$%%^^&*()_+");
	PRINT_xprint((unsigned char *)"-------====---------------");
	
	PRINT_xlinefeed(LINENUM*3);
	
	PRINT_xprint((unsigned char *)"-------End---------------");
	
	PRINT_xlinefeed(LINENUM*3);
}