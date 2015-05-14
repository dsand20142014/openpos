/*
|   Application System
|-----------------------------------------------------------------
|   SAND China						(Copyright SAND 2001)
|------------------------------------------------------------------
|   INTRODUCTION
|   ============
|
|   Project :        EMV for China union Pay
|
|   File name :     pc_download.c
|
|------------------------------------------------------------------
|   Creation date :	2005-09-01		Fangbo
|   Last modified  :	2005-09-23		Fangbo
|------------------------------------------------------------------
*/



#include <include.h>
#include <global.h>
#include <pc_download.h>
#include <xdata.h>

unsigned char ucCommFalg;
//#define PCDOWN_TEST

const TAGDEFINE  TagDefine[]={
				{8 ,   "\xA0\x01"},//
				{15 , "\xA0\x02"},//
				{40 , "\xA0\x03"},//
				{14 , "\xA0\x04"},//
				{14 , "\xA0\x05"},//
				{14 , "\xA0\x06"},//
				{14 , "\xA0\x07"},//
				{4 ,   "\xA0\x08"},//
				{6 ,   "\xA0\x09"},//
				{6 ,   "\xA0\x0A"},//
				{1 ,   "\xA0\x0B"},//
				{3 ,   "\xA0\x0C"},//
				{1 ,   "\xA0\x0D"},//
				{5 ,   "\xA0\x0E"},//
				{1 ,   "\xB0\x01"},//
				{1 ,   "\xB0\x02"},//
				{1 ,   "\xB0\x03"},//
				{2 ,   "\xB0\x04"},//
				{12 , "\xB0\x05"},//
				{1 ,   "\xB0\x06"},//
				{1 ,   "\xB0\x07"},//
				{1 ,   "\xB0\x08"},//
				{1 ,   "\xB0\x09"},//
				{1 ,   "\xB0\x0A"},//
				{1 ,   "\xB0\x0B"},//
				{1 ,   "\xB0\x0C"},//
				{1 ,   "\xB0\x0D"},//
				{1 ,   "\xB0\x0E"},//
				{1 ,   "\xB0\x0F"},//
				{1 ,   "\xB0\x10"},//
				{1 ,   "\xB0\x11"},//
				{1 ,   "\xC0\x01"},//
				{2 ,   "\xC0\x02"},//
				{10 , "\xD0\x01"},//
				{4 ,   "\xD0\x02"},//
				{1 ,   "\xD0\x03"},//
				{1 ,   "\xD0\x04"},//
				{1 ,   "\xE0\xE0"},//
				{32 , 	"\xE0\x00"},//
				{32 , 	"\xE0\x01"},//
				{32 , 	"\xE0\x02"},//
				{32 , 	"\xE0\x03"},//
				{32 , 	"\xE0\x04"},//
				{32 , 	"\xE0\x05"},//
				{32 , 	"\xE0\x06"},//
				{32 , 	"\xE0\x07"},//
				{32 , 	"\xE0\x08"},//
				{32 , 	"\xE0\x09"},//
				{NULL ,NULL},//
                      	};

unsigned char KEY_DownKeyFromPOS(void)
{
#if 1
	unsigned char ucResult=SUCCESS;
	unsigned char ucResultBuf[2],key;

	memset(ucResultBuf , 0, sizeof(ucResultBuf));
	#if 0
	if(!(ucResult = UTIL_Is_Trans_Empty()))
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1, 0, (uchar *)"POS存有交易数据");
		//Os__GB2312_display(2, 0, (uchar *)"请先结算");
		//Os__GB2312_display(2, 0, (uchar *)"或清除交易数据");
		Os__xget_key();
		return ERR_CANCEL;
	}
	#endif 

#if PS100
	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"密钥存储位置");
	//Os__GB2312_display(1,0,(unsigned char *)"0.主机");
	//Os__GB2312_display(2,0,(unsigned char *)"1.PIN");
	memset(aucBuf,0,sizeof(aucBuf));
	aucBuf[0] = DataSave0.ConstantParamData.ucKEYMODE;
	if( UTIL_Input(3,true,1,1,'N',aucBuf,(unsigned char *)"01") != KEY_ENTER )
		return(ERR_CANCEL);
	else
	{
		ucChar = aucBuf[0];
		Os__saved_copy(&ucChar,&DataSave0.ConstantParamData.ucKEYMODE,
			sizeof(unsigned char));
		XDATA_Write_Vaild_File(DataSaveConstant);
	}
	util_Printf("密钥存储位置 =%02x\n",DataSave0.ConstantParamData.ucKEYMODE);
#endif

	 ucResult = PCDown_SelectCOM();
	 if(ucResult!=SUCCESS)
		return ucResult;
	while(1)
	{
		ucResult=SUCCESS;
		ucResult = PCDown_CommInit(0);
		if(ucResult!=SUCCESS)
			return ucResult;
		ucResult = PCDown_RecvFromPC(60);
		if(ucResult!=SUCCESS)
		{
			if(ucCommFalg == TOPC_COM2)
				OSUART_Close2();
			return ucResult;
		}

		if(!ucResult)
			ucResult = PCDown_UnPackProcess( );

		memcpy(ucResultBuf , &ucResult, 1);
		PCDown_ResponeToPC(ucResultBuf,1);

		if(!ucResult)
		{
			key = PCDown_DispSucess();
			if(key != KEY_ENTER)	break;
		}
		else
			break;
	}
	return ucResult;
#else
	unsigned char ucKey,ucResult;
	unsigned char aucBuf[5];
	unsigned short uiKeyIndex;
	unsigned char ucI;

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(uchar*)"请输入密钥索引：");
	//Os__GB2312_display(1,0,(uchar*)"(0-2)");

	while(1)
	{
		memset(aucBuf,0,sizeof(aucBuf));
		aucBuf[0]=DataSave0.ConstantParamData.ucUseKeyIndex+'0';
		if( UTIL_Input(1,true,1,1,'N',aucBuf,(uchar *)"012") != KEY_ENTER )
			return(ERR_CANCEL);
		else
		{
			uiKeyIndex = aucBuf[0]-'0';
			ucI = (unsigned char)uiKeyIndex;
			Os__saved_copy(&ucI,&DataSave0.ConstantParamData.ucUseKeyIndex,
						sizeof(unsigned char));
			XDATA_Write_Vaild_File(DataSaveConstant);
			return(SUCCESS);
		}
	}
#endif

}
unsigned char PCDown_UpdataParam(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char ucResultBuf[2],key;

	memset(ucResultBuf , 0, sizeof(ucResultBuf));
	if(!(ucResult = UTIL_Is_Trans_Empty()))
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1, 0, (uchar *)"POS存有交易数据");
		//Os__GB2312_display(2, 0, (uchar *)"请先结算");
		//Os__GB2312_display(2, 0, (uchar *)"或清除交易数据");
		Os__xget_key();
		return ERR_CANCEL;
	}
	ucResult = PCDown_SelectCOM();

	util_Printf("1COM3=%02x\n",ucResult);
	if(ucResult!=SUCCESS)
		return ucResult;

	util_Printf("2COM3=%02x\n",ucResult);
	while(1)
	{
		ucResult=SUCCESS;
		ucResult = PCDown_CommInit(0);
		if(ucResult != SUCCESS)
			return ucResult;
		ucResult = PCDown_RecvFromPC(60);
		if(ucResult!= SUCCESS)
		{
			if(ucCommFalg == TOPC_COM2)
				OSUART_Close2();
			return ucResult;
		}

		if(!ucResult)
			ucResult = PCDown_UnPackProcess( );

		memcpy(ucResultBuf , &ucResult, 1);
		PCDown_ResponeToPC(ucResultBuf,1);

		if(!ucResult)
		{
			key = PCDown_DispSucess();
			if(key != KEY_ENTER)	break;
		}
		else
			break;
	}
	return ucResult;
}

unsigned char PCDown_UnPackProcess(void)
{
	unsigned char ucResult = SUCCESS;
	unsigned char ucCommandFlag;

	ucCommandFlag = PCDownload_RecvData.ucCommand;

	#ifdef PCDOWN_TEST
	util_Printf("PCDownload_RecvData.ucCommand = %02x \n",ucCommandFlag);
	#endif

	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(uchar *)" 数据处理中...");
	switch(ucCommandFlag)
	{
		case POSINIT:
			ucResult = PCDown_POSInit();
			break;
		case POSCLNTRANS:
			ucResult = PCDown_POSClnTrans();
			break;
		case POSUPDATAPARAM:
			ucResult = PCDown_POSUpdataParam();
			break;
		default:
			ucResult = ERR_COMMANDBYTE;
			break;
	}
	return ucResult;
}

unsigned char PCDown_POSInit(void)
{
	unsigned char ucResult=SUCCESS;

	UTIL_SetDefault_State(false);
	return ucResult;
}

unsigned char PCDown_POSClnTrans(void)
{
	unsigned char ucResult = SUCCESS;

	/*清除数据*/
	Os__saved_set((unsigned char *)&DataSave0.ConstantParamData.ucFunctStep,
		0,sizeof(unsigned char));
	Os__saved_set((unsigned char *)&DataSave0.ConstantParamData.aucSettleFlag,
						0,sizeof(unsigned char));
	Os__saved_set(&DataSave0.ChangeParamData.ucCashierLogonFlag,0,1);

	Os__saved_set((unsigned char *)&DataSave0.ChangeParamData.uiTotalNumber,0,
		sizeof(unsigned int));
	memset(&DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid,0x00,sizeof(unsigned char));
	memset(&DataSave0.Trans_8583Data.ReversalISO8583Data,0x00,sizeof(REVERSALISO8583));
       Os__saved_set((unsigned char*)&DataSave0.ChangeParamData.uiEMVICLogNum,0x00,sizeof(unsigned short));
	Os__saved_set((unsigned char*)&DataSave0.ChangeParamData.ucScriptInformValid,0x00,sizeof(unsigned char));

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
	Os__saved_set((unsigned char *)DataSave0.TransInfoData.auiTransIndex,
				0,sizeof(unsigned short)*TRANS_MAXNB);

	XDATA_Write_Vaild_File(DataSaveConstant);
	XDATA_Write_Vaild_File(DataSaveChange);
	XDATA_Write_Vaild_File(DataSaveCashier);
	XDATA_Write_Vaild_File(DataSaveTransInfo);
	XDATA_Write_Vaild_File(DataSaveTrans8583);

	return ucResult;
}

unsigned char PCDown_POSUpdataParam(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char ucTagData[TAGMAXLEN];
	unsigned short usTagLen;
	unsigned char ucKey,aucTransTAB[3] ,ucChar;

	PCDown_SaveTagVal();

	//==========================  0xA0 ==========================
	//Terminal ID
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x01" ,ucTagData ,&usTagLen)))
	{
		memcpy(DataSave0.ConstantParamData.aucTerminalID , ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,(uint)usTagLen,(UCHAR *)"ucTagData = ",NULL);
		#endif
	}
	//Merchant ID
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x02" ,ucTagData ,&usTagLen)))
	{
		memcpy(DataSave0.ConstantParamData.aucMerchantID , ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,(uint)usTagLen,(UCHAR *)"aucMerchantID = ",NULL);
		#endif
	}
	//Merchant name
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x03" ,ucTagData ,&usTagLen)))
	{
		memcpy(DataSave0.ConstantParamData.aucMerchantName , ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,(uint)usTagLen,(UCHAR *)"aucMerchantName = ",NULL);
		#endif
	}
	//TEL 1
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x04" ,ucTagData ,&usTagLen)))
	{
		memcpy(DataSave0.ConstantParamData.aucHostTelNumber1 , ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,(uint)usTagLen,(UCHAR *)"aucHostTelNumber1 = ",NULL);
		#endif
	}
	//TEL 2
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x05" ,ucTagData ,&usTagLen)))
	{
		memcpy(DataSave0.ConstantParamData.aucHostTelNumber2 , ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,(uint)usTagLen,(UCHAR *)"aucHostTelNumber2 = ",NULL);
		#endif
	}
	//TEL 3
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x06" ,ucTagData ,&usTagLen)))
	{
		memcpy(DataSave0.ConstantParamData.aucHostTelNumber3 , ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,(uint)usTagLen,(UCHAR *)"aucHostTelNumber3 = ",NULL);
		#endif
	}
	//TEL 4
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x07" ,ucTagData ,&usTagLen)))
	{
		memcpy(DataSave0.ConstantParamData.aucHostTelNumber4 , ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"aucHostTelNumber4 = ",NULL);
		#endif
	}

	//TPDU
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x08" ,ucTagData ,&usTagLen)))
	{
		asc_bcd(DataSave0.ConstantParamData.aucTpdu, 2, ucTagData, 4);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"aucTerminalID = ",NULL);
		#endif
	}
	//Trans number
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x09" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ChangeParamData.ulTraceNumber = asc_long(ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ulTraceNumber = ",NULL);
		#endif
	}
	//Batch number
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x0A" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ChangeParamData.ulBatchNumber = asc_long(ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ulBatchNumber = ",NULL);
		#endif
	}
	//Comm mode
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x0B" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucCommMode = ucTagData[0]-0x30;
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucCommMode = ",NULL);
		#endif
	}
	//Max transcation number
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x0C" ,ucTagData ,&usTagLen)))
	{
		TRANS_MAXNB = (uint)asc_long(ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"TRANS_MAXNB = ",NULL);
		#endif
	}
	//Dial wait time
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x0D" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.uiDialWaitTime = (ushort)tab_long(ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"uiDialWaitTime = ",NULL);
		#endif
	}
	//Comm speed
	if(!(ucResult = PCDown_GetTag((uchar *)"\xA0\x0E" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.uiCommSpeed = (ushort)asc_long(ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"uiCommSpeed = ",NULL);
		#endif
	}

	//==========================  0xB0 ==========================
	//Receive Timeout
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x01" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.uiReceiveTimeout = (ushort)tab_long(ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"uiReceiveTimeout = ",NULL);
		#endif
	}
	//Dial Retry
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x02" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucDialRetry = ucTagData[0]-0x30;
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucDialRetry = ",NULL);
		#endif
	}
	//Xor tip fee
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x03" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucXorTipFee = ucTagData[0]-0x30;
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucXorTipFee = ",NULL);
		#endif
	}
	//Tip fee prepercentum
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x04" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.usTipFeePre = (ushort)asc_long(ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"usTipFeePre = ",NULL);
		#endif
	}
	//Refund max amount
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x05" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ulRefundMaxAmount = asc_long(ucTagData, usTagLen);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ulRefundMaxAmount = ",NULL);
		#endif
	}
	//手输卡号设置
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x06" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucInputCardNo = ucTagData[0]-0x30;
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucInputCardNo = ",NULL);
		#endif
	}
	//支持自动签退
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x07" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucSelfLogoff  = ucTagData[0];
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucSelfLogoff = ",NULL);
		#endif
	}
	//交易重试次数
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x08" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucTransRetry = ucTagData[0]-0x30;
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucTransRetry = ",NULL);
		#endif
	}
	//下次联机上送离线
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x09" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucOFFLineSendType = ucTagData[0];
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucOFFLineSendType = ",NULL);
		#endif
	}
	//打印故障报告单
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x0A" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucPrnErrTicket = ucTagData[0];
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucPrnErrTicket = ",NULL);
		#endif
	}

	//消费撤消刷卡
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x0B" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucUnSaleSwipe = ucTagData[0];
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucUnSaleSwipe = ",NULL);
		#endif
	}
	//预授权完成撤消刷卡
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x0C" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucUnAuthFinSwipe = ucTagData[0];
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucUnAuthFinSwipe = ",NULL);
		#endif
	}
	//撤消是否输密码
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x0D" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucVoidInputPin = ucTagData[0];
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucVoidInputPin = ",NULL);
		#endif
	}
	//预授权完成联机是否输密码
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x0E" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucPREAUTHFINISHInputPin = ucTagData[0];
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucPREAUTHFINISHInputPin = ",NULL);
		#endif
	}
	//缺省交易类型
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x0F" ,ucTagData ,&usTagLen)))
	{
		if(ucTagData[0] == '1')
			DataSave0.ConstantParamData.ucDefaultTransParam=TRANS_PURCHASE;
		else
			DataSave0.ConstantParamData.ucDefaultTransParam=TRANS_PREAUTH;
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucDefaultTransParam = ",NULL);
		#endif
	}
	//终端加密方式参数
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x10" ,ucTagData ,&usTagLen)))
	{
		if(ucTagData[0]=='1')
			DataSave0.ConstantParamData.ENCRYPTTYPEParam=ENCRYPTTYPE_SINGLE;
		else
			DataSave0.ConstantParamData.ENCRYPTTYPEParam=ENCRYPTTYPE_DOUBLE;
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ENCRYPTTYPEParam = ",NULL);
		#endif
	}
	//磁卡授权完成参数
	if(!(ucResult = PCDown_GetTag((uchar *)"\xB0\x11" ,ucTagData ,&usTagLen)))
	{
		memset( aucTransTAB, 0, sizeof(aucTransTAB));
		ucKey=0;
		ucChar =0;
		ucKey = ucTagData[0];

		if(ucKey>='0'&&ucKey<='2')
		{
			DataSave0.ConstantParamData.ucMagPreauthFinish=ucKey;
			memcpy(aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,PARAM_GLOBALTRANSENABLELEN);
			util_Printf("aucTransTAB--1-- = %02x,%02x\n",aucTransTAB[0],aucTransTAB[1]);
			if(ucKey == '0')
			{
				//加联机
				ucChar = aucTransTAB[0];
				ucChar |= 0x10;
				aucTransTAB[0] = ucChar;
				//加离线
				ucChar = aucTransTAB[1];
				ucChar |= 0x20;
				aucTransTAB[1] = ucChar;
				Os__saved_copy( aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
					PARAM_GLOBALTRANSENABLELEN);
			}
			if(ucKey == '1')
			{
				//加联机
				ucChar = aucTransTAB[0];
				ucChar |= 0x10;
				aucTransTAB[0] = ucChar;
				//去离线
				ucChar = aucTransTAB[1];
				ucChar &= 0xDF;
				aucTransTAB[1] = ucChar;
				Os__saved_copy( aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
					PARAM_GLOBALTRANSENABLELEN);
			}
			if(ucKey == '2')
			{
				//去联机
				ucChar = aucTransTAB[0];
				ucChar &= 0xEF;
				aucTransTAB[0] = ucChar;
				//加离线
				ucChar = aucTransTAB[1];
				ucChar |= 0x20;
				aucTransTAB[1] = ucChar;
				Os__saved_copy( aucTransTAB,DataSave0.ConstantParamData.aucGlobalTransEnable,
					PARAM_GLOBALTRANSENABLELEN);
			}
		}
		#ifdef PCDOWN_TEST
		util_Printf("aucTransTAB--2-- = %02x,%02x\n",aucTransTAB[0],aucTransTAB[1]);
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"aucGlobalTransEnable --1--= ",NULL);
		#endif
	}

	//==========================  0xC0 ==========================
	//选择交易状态类型
	if(!(ucResult = PCDown_GetTag((uchar *)"\xC0\x01" ,ucTagData ,&usTagLen)))
	{
		if(ucTagData[0] == '0')
		{
			LOGON_WriteSuperLogonFlag(CASH_LOGONFLAG);/*状态设为测试交易*/
		}else
		{
			LOGON_WriteSuperLogonFlag(0);		/*状态设为正常交易*/
		}
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucSuperLogonFlag = ",NULL);
		#endif
	}
	//状态设置值
	if(!(ucResult = PCDown_GetTag((uchar *)"\xC0\x02" ,ucTagData ,&usTagLen)))
	{
		memcpy(DataSave0.ConstantParamData.aucGlobalTransEnable , ucTagData, PARAM_GLOBALTRANSENABLELEN);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"aucGlobalTransEnable --2--= ",NULL);
		#endif
	}
	//==========================  0xD0 ==========================
	//日期时间:
	if(!(ucResult = PCDown_GetTag((uchar *)"\xD0\x01" ,ucTagData ,&usTagLen)))
	{
		CFG_WriteDateTimetoPOS(ucTagData);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucVoidInputPin = ",NULL);
		#endif
	}
	//分机外线
	if(!(ucResult = PCDown_GetTag((uchar *)"\xD0\x02" ,ucTagData ,&usTagLen)))
	{
		unsigned char aucModemParam[10]={0,0,8,6,115,8,1,3,0,0};
		unsigned char index;
		memcpy(DataSave0.ConstantParamData.aucSwitchBoard  , ucTagData ,usTagLen);
		index = strlen((char *)ucTagData);
		DataSave0.ConstantParamData.aucSwitchBoard[index]=',';
		Os__saved_copy(aucModemParam,(uchar *)&DataSave0.ConstantParamData.ModemParam,10);
		#ifdef PCDOWN_TEST
		util_Printf("index = %02x   ,  ",index);
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"aucSwitchBoard = ",NULL);
		UTIL_TestDispStr(DataSave0.ConstantParamData.aucSwitchBoard,PARAM_SWITCHLEN,(UCHAR *)"DataSave0.ConstantParamData.aucSwitchBoard = ",NULL);
		#endif
	}

	//打印格式
	if(!(ucResult = PCDown_GetTag((uchar *)"\xD0\x03" ,ucTagData ,&usTagLen)))
	{
		UTIL_SavPrinterParamToPOS(ucTagData[0]);
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"Pinter flag = ",NULL);
		#endif
	}
	//密码键盘设置
	if(!(ucResult = PCDown_GetTag((uchar *)"\xD0\x04" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.Pinpadflag = ucTagData[0] - 0x30;
		#ifdef PCDOWN_TEST
		UTIL_TestDispStr(ucTagData,usTagLen,(UCHAR *)"ucVoidInputPin = ",NULL);
		#endif
	}
	//==========================  0xE0 0xE0 密钥==========================
	//
	XDATA_Write_Vaild_File(DataSaveConstant);
	XDATA_Write_Vaild_File(DataSaveChange);

	if(!(ucResult = PCDown_GetTag((uchar *)"\xE0\xE0" ,ucTagData ,&usTagLen)))
	{
		DataSave0.ConstantParamData.ucUseKeyIndex = ucTagData[0]-0x30;
		XDATA_Write_Vaild_File(DataSaveConstant);
		util_Printf("\n--------- E0 E0------get key---------\n");
		PCDown_UpdataMasterKey();
		util_Printf("\n--------- E0 E0------key end---------\n");
	}

	return SUCCESS;
}

void PCDown_UpdataMasterKey(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char ucTagData[TAGMAXLEN];
	unsigned short usTagLen ,usI;
	unsigned char ucTag[3];

	memset(ucTag , 0, sizeof(ucTag));
	memset(ucTagData,0,sizeof(ucTagData));
	ucTag[0] = 0xE0;

	for(usI=0;usI<MASTERKEYMAXNUM;usI++)
	{
		ucTag[1] = usI;
		#ifdef PCDOWN_TEST
		util_Printf("key index ---1--- [%02x][%02x]\n",ucTag[0],ucTag[1]);
		#endif
		if(!(ucResult = PCDown_GetTag(ucTag,ucTagData ,&usTagLen)))
		{
			PCDown_SaveMasterKey( usI , ucTagData , usTagLen);
		}
	}
}

unsigned char  PCDown_SaveMasterKey(unsigned char ucKeyIndex , unsigned char * pucInBuf , unsigned short usInLen)
{
	unsigned char ucResult;
	unsigned char aucAscData[33];
	unsigned char aucAscData1[17];
	unsigned char aucAscData2[17];
	unsigned char aucHexData[9];
	unsigned char aucHexData1[9];
	unsigned char ucArrayIndex;

	memset(aucAscData,0,sizeof(aucAscData));
	memset(aucAscData1,0,sizeof(aucAscData1));
	memset(aucAscData2,0,sizeof(aucAscData2));
	memset(aucHexData,0,sizeof(aucHexData));
	memset(aucHexData1,0,sizeof(aucHexData1));

	ucArrayIndex = KEYARRAY_GOLDENCARDSH;

	memcpy(aucAscData , pucInBuf, usInLen);
	memcpy(aucAscData1 , aucAscData , 16);
	memcpy(aucAscData2 , &aucAscData[16] , 16);

	asc_hex(aucHexData,8,aucAscData1,16);
	asc_hex(aucHexData1,8,aucAscData2,16);

	#ifdef PCDOWN_TEST
	util_Printf("ucKeyIndex = %02x\n",ucKeyIndex);
	UTIL_TestDispStr(aucAscData,usInLen,(UCHAR *)"Master key asc = ",NULL);
	UTIL_TestDispStr(aucHexData,8,(UCHAR *)"Master key hex 1= ",NULL);
	UTIL_TestDispStr(aucHexData1,8,(UCHAR *)"Master key hex 2= ",NULL);
	#endif

	//---------------
	ucResult = PINPAD_47_Encrypt8ByteSingleKey(ucArrayIndex,
					ucKeyIndex,aucHexData,aucHexData);
	if(ucResult )	return(ucResult);
	UTIL_TestDispStr(aucHexData,8,(UCHAR *)"Master key hex 1= ",NULL);
	ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(ucArrayIndex,
					ucKeyIndex,ucKeyIndex,aucHexData);
	if(ucResult )	return(ucResult);
	UTIL_TestDispStr(aucHexData1,8,(UCHAR *)"Master key hex 2= ",NULL);
	//--------------------
	ucResult = PINPAD_47_Encrypt8ByteSingleKey(ucArrayIndex,
					ucKeyIndex+3,aucHexData1,aucHexData1);
	UTIL_TestDispStr(aucHexData1,8,(UCHAR *)"Master key hex 2= ",NULL);
	if(ucResult )	return(ucResult);
	ucResult = PINPAD_42_LoadSingleKeyUseSingleKey(ucArrayIndex,
					ucKeyIndex+3,ucKeyIndex+3,aucHexData1);
	UTIL_TestDispStr(aucHexData1,8,(UCHAR *)"Master key hex 2= ",NULL);
}


unsigned char PCDown_SwitchTag(unsigned char * pucTag , unsigned short usTagLen)
{
	unsigned char ucResult=SUCCESS;
	unsigned char ucTag[2];
	unsigned short usIndex;

	memcpy( ucTag, pucTag, 2);

	usIndex=0;
	while(1)
	{
		if(!memcmp(TagDefine[usIndex].ucVal , ucTag, 2))
		{
			if(usTagLen == TagDefine[usIndex].ucLen)
				return SUCCESS;
			else
				return ERR_TAGLEN;
		}
		else
		if(!memcmp(TagDefine[usIndex].ucVal , NULL, 1))
			return ERR_TAGVAL;
		usIndex++;
	}

	return ucResult;
}
unsigned char PCDown_SaveTagVal(void)
{
	unsigned char ucTag[2];
	unsigned short usTagLen;
	unsigned char ucRevBuf[COMMBUFMAXLEN];
	unsigned short usLen;
	unsigned short ucIndex;

	#ifdef PCDOWN_TEST
	util_Printf("\n=================SAVE TAG  for all!!!===================\n");
	#endif
	memset(TagData ,0,sizeof(RECVTAG)*TAGMAXNUM);
	memset(ucRevBuf ,0,sizeof(ucRevBuf));
	usTagNum = 0;

	memcpy(ucRevBuf , PCDownload_RecvData.ucRecvData, PCDownload_RecvData.ucRecvLen);
	usLen = PCDownload_RecvData.ucRecvLen;
	ucIndex=0;

	while(ucIndex<usLen)
	{
		memcpy(ucTag, &ucRevBuf[ucIndex], 2);
		ucIndex+=2;
		usTagLen = (USHORT)tab_long(&ucRevBuf[ucIndex],1);
		ucIndex+=1;
		if(usTagLen < TAGMAXLEN)
		{
			if(!PCDown_SwitchTag(ucTag , usTagLen))
			{
				memcpy(TagData[usTagNum].ucTag, ucTag, 2);
				TagData[usTagNum].usTagLen = usTagLen;
				memcpy(TagData[usTagNum].ucTagData, &ucRevBuf[ucIndex], usTagLen);
				TagData[usTagNum].ucExistFlag = true;
				#ifdef PCDOWN_TEST
				util_Printf("TagData[%d].usTagLen = %d\n",usTagNum,TagData[usTagNum].usTagLen);
				util_Printf("TagData[%d].ucExistFlag = %02x\n",usTagNum,TagData[usTagNum].ucExistFlag);
				util_Printf("ucTag = [%02x][%02x]\n",ucTag[0],ucTag[1]);
				util_Printf("TagData[%d].ucTag = [%02x][%02x]\n",usTagNum,TagData[usTagNum].ucTag[0],TagData[usTagNum].ucTag[1]);
				UTIL_TestDispStr(TagData[usTagNum].ucTagData,(uint)TagData[usTagNum].usTagLen,(UCHAR *)" ",NULL);
				#endif
				usTagNum++;
			}
		}

		ucIndex += usTagLen;
		#ifdef PCDOWN_TEST
		util_Printf("-------------ucIndex = %d  ;usTagLen------------end----------\n",ucIndex,usTagLen);
		#endif
	}
	#ifdef PCDOWN_TEST
	util_Printf("\n=================  END  ===================\n");
	#endif

	return SUCCESS;

}


unsigned char PCDown_GetTag(unsigned char * pucTag ,unsigned char * pucOutBuf ,unsigned short * pusOutLen)
{
	unsigned char ucResult=SUCCESS;
	unsigned short usIndex;

	usIndex=0;
	memset(pucOutBuf ,0 , TAGMAXLEN );

	while(usIndex<usTagNum)
	{
		if(!memcmp(TagData[usIndex].ucTag, pucTag, 2))
		{
			memcpy( pucOutBuf, TagData[usIndex].ucTagData, TagData[usIndex].usTagLen);
			*pusOutLen = TagData[usIndex].usTagLen;
			break;
		}
		usIndex++;
	}

	if((usIndex >= TAGMAXNUM)|| TagData[usIndex].ucExistFlag != 0xFF )
	{
		util_Printf("-------GET ERR-----\n");
		ucResult = ERR_END;
	}else
	{
		#ifdef PCDOWN_TEST
		util_Printf("Get Tag  : [%02x] [%02x]   ",*(pucTag),*(pucTag+1));
		UTIL_TestDispStr(TagData[usIndex].ucTagData,
						(uint)TagData[usIndex].usTagLen,
						(UCHAR *)" ",NULL);
		#endif
		ucResult = SUCCESS;
	}
	return ucResult;
}

unsigned char PCDown_CommInit(unsigned char ucFlag)
{

	COM_PARAM CommParam =
	{
		1,     /* Stop bit */
		9600,  /* Speed */
		1,	   /* MODEM 0 RS232 1 HDLC 2 */
		8,     /* Data bit */
	   	'N'     /* NO parity */
	};

	if(ucCommFalg == TOPC_COM1)
	{
		if(!OSUART_Init1(&CommParam))
			return SUCCESS;
		else
			return ERR_COMMINIT;
	}
	else
	if(ucCommFalg == TOPC_COM2)
	{
		Os__end_com3();
		if( !OSUART_Init2(0x0303,0x0C00,0x0C))	//COM2 speed:9600
		{
			return SUCCESS;
		}
		else
		{
			return ERR_COMMINIT;
		}
	}
}

unsigned char PCDown_SelectCOM(void)
{
	unsigned char KEY;

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,(unsigned char *)"请选择通讯串口：");
	//Os__GB2312_display(1,0,(unsigned char *)"[1]---串口1 ");
	//Os__GB2312_display(2,0,(unsigned char *)"[2]---串口2");

	while(!(KEY=='1' ||KEY=='2'||KEY==KEY_CLEAR))
		KEY = Os__xget_key();

	ucCommFalg = 0;
	if(KEY == '1')
	{
		ucCommFalg = TOPC_COM1;
	}
	else if(KEY == '2')
	{
		ucCommFalg = TOPC_COM2;
	}
	else if(KEY == KEY_CLEAR)
	{
		return ERR_CANCEL;
	}
	return SUCCESS;
}

unsigned char PCDown_rxcar(unsigned short usOutTime )
{
	if(ucCommFalg == TOPC_COM1)
		return Os__rxcar(usOutTime);
	else
	if(ucCommFalg == TOPC_COM2)
		return Os__rxcar3(usOutTime);
}

unsigned char PCDown_txcar(unsigned char ucSendChar)
{
	if(ucCommFalg == TOPC_COM1)
			Os__txcar(ucSendChar);
	else
	if(ucCommFalg == TOPC_COM2)
			Os__txcar3(ucSendChar);
}

unsigned char PCDown_RecvFromPC( unsigned char WaitTime)
{
	unsigned char ucResult,aucRecvBuf[COMMBUFMAXLEN];
	unsigned char ucLRC,ucCommFlag;
	unsigned short uiI,uiChar;
	unsigned short uiRecvLen;
	unsigned int Timeout;
	unsigned char ucKey;
	DRV_OUT *KEY;


	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(unsigned char *)"等待接收数据...");
	if(ucCommFalg == TOPC_COM1)
		//Os__GB2312_display(3,0,(unsigned char *)"        [串口1 ]");
	if(ucCommFalg == TOPC_COM2)
		//Os__GB2312_display(3,0,(unsigned char *)"        [串口2 ]");

	memset(aucRecvBuf,0,sizeof(aucRecvBuf));
	uiI = 0;
	ucLRC = 0;
	ucCommFlag = false;
	Timeout=WaitTime*100;
	Os__timer_start(&Timeout);
    Os__xdelay(1);

	while(1)
	{
		KEY=Os__get_key();
		ucResult = SUCCESS;
		uiChar = PCDown_rxcar(600);
		if(KEY->gen_status==DRV_ENDED)
		{
			ucKey = 0;
			ucKey = KEY->xxdata[1];
			if(ucKey == KEY_CLEAR) ucResult = ERR_CANCEL;
			else continue;
		}else
		if(Timeout==0) ucResult = ERR_COMMS_RECVTIMEOUT;
		else
		if((!(uiChar/256))&&((uiChar%256)==CHAR_STX))
		{
			aucRecvBuf[0] = uiChar%256;
			for(uiI=0,ucLRC=0;uiI<2;uiI++)
			{
				aucRecvBuf[uiI+1] = PCDown_rxcar(10)%256;
				ucLRC^=aucRecvBuf[uiI+1];
			}
			uiRecvLen = (unsigned short)tab_long(&aucRecvBuf[1],2);

			if(uiRecvLen > COMMBUFMAXLEN)
			{
				Os__abort_drv(drv_mmi);
				Os__timer_stop(&Timeout);
				return ERR_RECVMAXLEN;
			}
			for(uiI=0;uiI<uiRecvLen;uiI++)
			{
				aucRecvBuf[uiI+3] = PCDown_rxcar(10)%256;
				ucLRC^=aucRecvBuf[uiI+3];
			}
			aucRecvBuf[uiI+3] = PCDown_rxcar(10)%256;
			if(aucRecvBuf[uiRecvLen+3] != CHAR_ETX)
			{
				Os__abort_drv(drv_mmi);
				Os__timer_stop(&Timeout);
				return ERR_COMMS_PROTOCOL;
			}
			if(!ucResult)
			{
				aucRecvBuf[uiI+4] = PCDown_rxcar(10)%256;
				if(aucRecvBuf[uiRecvLen+4] != ucLRC)
				{
					Os__abort_drv(drv_mmi);
					Os__timer_stop(&Timeout);
					return ERR_COMMS_LRC;
				}else
				{
					PCDownload_RecvData.ucCommand= aucRecvBuf[3];
					if(PCDownload_RecvData.ucCommand == 0x03)
					{
						PCDownload_RecvData.ucRecvLen = uiRecvLen-1;
						memcpy(PCDownload_RecvData.ucRecvData, &aucRecvBuf[4], uiRecvLen-1);
					}
					Os__abort_drv(drv_mmi);
					Os__timer_stop(&Timeout);
					#ifdef PCDOWN_TEST
					if(ucCommFalg == TOPC_COM1)
					{
						util_Printf("\n===================================");
						util_Printf("Recv data  from PC = ");
						{
							int i;
							for(i=0;i<uiRecvLen;i++)
								util_Printf("%02x ",aucRecvBuf[i]);
							util_Printf("\n===================================\n");
						}
					}
					#endif
					return SUCCESS;
				}
			}
		}
		if(ucResult)
		{
			Os__abort_drv(drv_mmi);
			Os__timer_stop(&Timeout);
			return ucResult;
		}
	}
}

unsigned char PCDown_ResponeToPC(unsigned char *aucInBuf,unsigned short uiLen)
{

	unsigned char aucSendBuf[512];
	unsigned char ucLRC;
	unsigned short uiI;

	ucLRC=0;
	memset(aucSendBuf,0,sizeof(aucSendBuf));
	aucSendBuf[0] = CHAR_STX;

	short_tab(&aucSendBuf[1],2,&uiLen);
	memcpy(&aucSendBuf[3],aucInBuf,uiLen);
	aucSendBuf[3+uiLen] = CHAR_ETX;

	for(uiI=0,ucLRC=0;uiI<uiLen+2;uiI++)
	{
		ucLRC ^= aucSendBuf[uiI+1];
	}
	aucSendBuf[4+uiLen] = ucLRC;

	for(uiI=0;uiI<uiLen+5;uiI++)
	{
		PCDown_txcar(aucSendBuf[uiI]);
	}
#ifdef PCDOWN_TEST
	if(ucCommFalg == TOPC_COM1)
	{
		util_Printf("\n*********** Respone_ToPC *****************");
		util_Printf("\nSend Data Len %d",uiLen+5);
		util_Printf("\nSend Data:");

		for(uiI=0;uiI<uiLen+5;uiI++)
		{
			if((!uiI%20)) util_Printf("\n");
			util_Printf("%02X ",aucSendBuf[uiI]);
		}
		util_Printf("\n**************** end *********************\n");

	}
#endif
	if(ucCommFalg == TOPC_COM2)
		OSUART_Close2();
	return(SUCCESS);
}

unsigned char  PCDown_DispSucess(void)
{
	unsigned char key;

	//Os__clr_display(255);
	//Os__GB2312_display(3,0,(uchar *)"按[确认]键继续");

	switch(PCDownload_RecvData.ucCommand)
	{
		case POSINIT:
			//Os__GB2312_display(1,0,(uchar *)"初始化成功");
			key = Os__xget_key();
			break;
		case POSCLNTRANS:
			//Os__GB2312_display(1,0,(uchar *)"交易清除成功");
			key = Os__xget_key();
			break;
		case POSUPDATAPARAM:
			//Os__GB2312_display(1,0,(uchar *)"参数更新成功");
			key = Os__xget_key();
			break;
		default:
			break;
	}

	return key;

}


