
#include	<MasApp.h>
#include	<Global.h>
#include 	<Util.h>
#include	<Msg.h>
//#include	<digital.h>

extern const QDATAPROPERTY	QICCardProperty[];
extern const	QREADERPARAM	ConstReaderDefault;

void ProcessMag(BOOL bFallBack,unsigned char* bufMag);
UCHAR SelectAccountType(void);
BOOL	TransCapture(QTRANSRESULT	enTransResult);
void	PrintTransTicket(void);
UCHAR	ReadICTransLog(void);
UCHAR 	MagTrack2[40];
UCHAR	QICC(QICCIN *pQICCIn ,QICCOUT *pQICCOut)
{
	UCHAR	ucResult;
	USHORT	uiI;

	if(QConstParam.bICCDebug)
	{
		Uart_Printf("\n\nSend ICC Data:\n");
		Uart_Printf("%02X %02X %02X %02X ",pQICCIn->ucCla,pQICCIn->ucIns,pQICCIn->ucP1,pQICCIn->ucP2);
		for(uiI=0;uiI<pQICCIn->uiLc;uiI++)
			Uart_Printf("%02X ",*(pQICCIn->aucDataIn+uiI));
		if(pQICCIn->uiLe) Uart_Printf("00");
	}

	ucResult=IccIsoCommand(0,pQICCIn,pQICCOut);
	   
	if(!ucResult)
	{
		if(QConstParam.bICCDebug)
		{
			Uart_Printf("\nRecv ICC Data:\n");
			for(uiI=0;uiI<pQICCOut->uiLenOut;uiI++)
				Uart_Printf("%02X ",pQICCOut->aucDataOut[uiI]);
			Uart_Printf("\nSW: %02X %02X",pQICCOut->ucSWA,pQICCOut->ucSWB);
		}
	}
	else Uart_Printf("\nICC COMMAND return %02X \n",ucResult);
	return ucResult;
	
}

unsigned char MASAPP_Event(void)
{
    	DRV_OUT *pxKey;
	DRV_OUT *pxMifare;

	unsigned char ucCardDetect,ucResult,ucByte,ucKey,aucDateTime[30];
	QTRANSTYPE	enTransType;
	unsigned long	uiAmount,uiOtherAmount;
	unsigned int  uiTimesOut;
	
	OnEve_power_on();

	while(1)
	{
		Os__light_off();
		pxKey = Os__get_key();
//		Uart_Printf("\nPolling 111111");
		pxMifare = Os__MIFARE_Polling( );
//		Uart_Printf("\nPolling 22222");
		Os__clr_display(255);
		Os__GB2312_display(1,2,(uchar *)"QPBOC TEST ");
		OSMMI_DisplayASCII(0x30,5,5,(uchar *)"   Ver 1.2");
	
		while((pxMifare->gen_status != DRV_ENDED)
			&&(pxKey->gen_status != DRV_ENDED));

		Os__light_on();
		memcpy(QReaderParam.aucTermTransQ,ConstReaderDefault.aucTermTransQ,4);

		if(pxKey->gen_status == DRV_ENDED)
		{
			Os__abort_drv(drv_mifare);
			if((pxKey->xxdata[1]>='0')&&(pxKey->xxdata[1]<='9'))
			{
				Os__clr_display(255);
				Os__GB2312_display(2,0,(PUCHAR)"Trans Amount:");
				uiAmount = pxKey->xxdata[1] - '0';
				ucResult=UTIL_InputAmount(3,&uiAmount,0,99999999999);
				if(!ucResult)
				{
#if 1
					ucResult = QVSDC_PreTransProcess(uiAmount);
					if(ucResult) 
					{
						QERROR_DisplayMsg(ucResult);
						continue;
					}
#endif				
					
					Os__clr_display(255);
					Os__GB2312_display(2,0,(PUCHAR)"Put the Card");
					pxMifare = Os__MIFARE_Polling( );
					pxKey = Os__get_key();
					while(pxMifare->gen_status != DRV_ENDED)
					{
						if(pxKey->gen_status == DRV_ENDED)
						{
							if(pxKey->xxdata[1] ==KEY_CLEAR)
							{
								Os__abort_drv(drv_mifare);
								break;
							}
							else 
								pxKey = Os__get_key();
						}
						//Os__xdelay(10);
					}
					Os__abort_drv(drv_mmi);
					
					if(pxMifare->gen_status == DRV_ENDED)
					{	
						if(pxMifare->xxdata[0] ==0x00)
						{
							Os__abort_drv(drv_mmi);
							ucResult = Os__MIFARE_Active();
							if(ucResult == 0x07)
								ucResult = QERROR_COLLISION;
							if(!ucResult)
								ucResult = EMVTransProcess(QGOODS,uiAmount,uiOtherAmount);
	
						}
						else if(pxMifare->xxdata[0] ==0x07)
							ucResult = QERROR_COLLISION;
						 
						if (ucResult ==QERROR_COLLISION)
						{
								Os__clr_display(255);
								Os__GB2312_display(2,0,"Single Card");
								UTIL_GetKey(1);
						}
						
						Os__clr_display(255);
						Os__GB2312_display(2,2,"Remove pls");
						Os__MIFARE_Remove();
					}
						
					
				}
				Os__light_off();
			}
			else 
			{
				switch(pxKey->xxdata[1])
				{
					case KEY_F1:
						TERMSETTING_Func();
						break;
					case KEY_F2:
						TERMSETTING_TransParam();
						break;
					case KEY_F3:
						do
						{
							Os__clr_display(255);
							if(QConstParam.bPBOC20==FALSE)
							{
								Os__GB2312_display(0,0,"1.Batch Up");
								Os__GB2312_display(1,0,"2.Param Update");
								Os__GB2312_display(2,0,"3.Trans TVR");
							}
							else
							{
								Os__GB2312_display(0,0,"1.批上送");
								Os__GB2312_display(1,0,"2.参数更新");
								Os__GB2312_display(2,0,"3.终端验证结果");
								Os__GB2312_display(3,0,"4.交易日志");
							}
							
								
							ucKey =Os__xget_key();
							switch(ucKey)
							{
							case '1':
								//MSG_BatchUp();
								MSG_Reconciliation();
								break;
							case '2':
								MSG_UpdateParam();
								break;
							case '3':
								CheckTransTVR();
								break;
							case '4':
								if(QConstParam.bPBOC20)
									ReadICTransLog();
								break;
							default:
								break;
							}
						}while(ucKey != KEY_CLEAR);
						break;
#if 1
					case KEY_F4:
						do
						{
							Os__clr_display(255);
							Os__GB2312_display(1,0,(PUCHAR)"Turn Off ");
							Os__GB2312_display(2,1,(PUCHAR)"1.OK  2.Cancel");
							ucKey =Os__xget_key();
							switch(ucKey)
							{
							case '1':
								OSMOBILE_PowerOff();
								break;
							default:
								break;
							}
						}while(ucKey != KEY_CLEAR);
						break;
#endif
					default:
						break;
						
				}
			}
		}
		else if(pxMifare->gen_status == DRV_ENDED)
		{
			Os__abort_drv(drv_mmi);

			if(pxMifare->xxdata[0] ==0x00)
			{	
				ucResult = Os__MIFARE_Active();
				if(ucResult ==0x07)
					ucResult = QERROR_COLLISION;
			}
			else if(pxMifare->xxdata[0] ==0x07)
				ucResult = QERROR_COLLISION;
				
			if (ucResult ==QERROR_COLLISION)
			{
				Os__clr_display(255);
				Os__GB2312_display(2,0,"Single Card");
				UTIL_GetKey(1);
			}
			if(ucResult) continue;
			uiAmount =200;
			
			if(QReaderParam.aucTermTransQ[0]&TTQ_QVSDCSUPPORT)
			{
				ucResult = QVSDC_PreTransProcess(uiAmount);
				if(ucResult) 
				{
					QERROR_DisplayMsg(ucResult);
					continue;
				}
			}
			enTransType =QGOODS;
			EMVTransProcess(enTransType,uiAmount,0);

			Os__clr_display(255);
			Os__GB2312_display(2,2,"Remove pls");
			Os__MIFARE_Remove();
		}
	Os__light_off();
	}
}


#define		MAXDISPROWS		8

UCHAR	CardHolderConfirmApp(PQCANDIDATELIST pCandidateList,PUCHAR pucAppIndex)
{
	UCHAR		ucI,ucRow,ucCurPage,ucBeginIndex;
	UCHAR		aucBuff[30],ucKey,ucICTI;
	PQCANDIDATEAPP	pCandidateApp; 
	BOOL		bDispFlag;
	
	ucCurPage =0;
/*
	bDispFlag = TRUE;
	for(ucI=0;ucI<pCandidateList->CandidateNums;ucI++)
	{
		pCandidateApp =&pCandidateList->CandidateApp[ucI];
		if(pCandidateApp->ucADFNameLen*2 > MAXDISPCHAR-2)
		{
			bDispFlag = FALSE;
			break;
		}
	}
	*/
	do
	{
		ucBeginIndex =ucCurPage*MAXDISPROWS;
		Os__clr_display(255);
		for(ucI =0; ucI <MAXDISPROWS; ucI++)
		{
			if(ucI+ucBeginIndex >= pCandidateList->CandidateNums) break;
			pCandidateApp =&pCandidateList->CandidateApp[ucI+ucBeginIndex];
			memset(aucBuff,0x00,sizeof(aucBuff));
			aucBuff[0] = ucI+ucBeginIndex+0x31;
			aucBuff[1] = '.';
			if(pCandidateApp->bICTIExist
			   &&(pCandidateApp->ucICTI ==0x01)
			   &&pCandidateApp->ucAppPreferNameLen)
				memcpy(&aucBuff[2],pCandidateApp->aucAppPreferName,pCandidateApp->ucAppPreferNameLen);
			else if(pCandidateApp->ucAppLabelLen)
				memcpy(&aucBuff[2],pCandidateApp->aucAppLabel,pCandidateApp->ucAppLabelLen);
			else if(pCandidateApp->ucADFNameLen)
				CONV_HexAsc(&aucBuff[2],pCandidateApp->aucADFName,pCandidateApp->ucADFNameLen*2);

//			if(bDispFlag)
//				Os__GB2312_display(ucI,0,aucBuff);
//			else
				OSMMI_DisplayASCII(0x30,ucI,0,aucBuff);
			
		}

		ucKey = Os__xget_key();
		if(ucKey ==KEY_ENTER)
		{
			*pucAppIndex =0;
			return QERROR_SUCCESS;
		}
		else if(ucKey == KEY_CLEAR)
			return QERROR_CANCEL;
		else if((ucKey>'0')&&(ucKey < pCandidateList->CandidateNums +'1'))
		{
			*pucAppIndex =ucKey -0x31;
			return QERROR_SUCCESS;
		}

		if(ucKey == KEY_F3 &&ucCurPage)
			ucCurPage --;
		if(ucKey == KEY_F4 && (ucCurPage+1)*MAXDISPROWS<pCandidateList->CandidateNums)
			ucCurPage++;
			
			
	}while(1);
	
}


UCHAR	EMVTransProcess(QTRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount)
{
	UCHAR	ucResult,aucDateTime[10];
	QCRYPTTYPE	CryptType;
	UINT		uiLogSumAmount;
	BOOL	bConfirm,bConnectSuccess,bHostApproval;
	UCHAR	ucAppIndex,ucAuthRespCode;
	QCANDIDATELIST	QCandidateList;
	UCHAR	aucRecvBuff[500],aucBuff[30],ucKey,ucRequestNum,aucTransResult[30],ucCVMResult;
	USHORT  uiRecvLen;
	QTRANSTAGDATA *pRespCode,* pAOSA;

	Os__light_on();
	ucResult = QERROR_SUCCESS;
	QConfig.ucCAPKNums =ucQTermCAPKNum;
	QConfig.pTermSupportCAPK =QTermCAPK;
	QConfig.ucTermSupportAppNum = ucQTermAIDNum;
	QConfig.pTermSupportApp = QTermAID;
	QConfig.ucQIPKRevokeNum =ucQIPKRevokeNum;
	QConfig.pIPKRevoke =QIPKRevoke;
	QConfig.ucQExceptFileNum = ucQExceptFileNum;
	QConfig.pExceptPAN = QExceptFile;
	QConfig.pfnQICC = QICC;
	QConfig.bPBOC20 =QConstParam.bPBOC20;
	QConfig.pfnEMVIDCardVerify =UTIL_IDCardVerify;
	QDATA_TermInit(&QConfig);
	
	Os__clr_display(255);
	Os__GB2312_display(2,0,(PUCHAR)"Processing");
	
	if(!ucResult)
	{
		ucResult = QAPPSELECT_CreateAppCandidate(&QCandidateList);
	}
	
	while(!ucResult && QCandidateList.CandidateNums)
	{
		ucAppIndex =0;
		ucResult = QAPPSELECT_FinalSelect(ucAppIndex,&QCandidateList);
//		Uart_Printf("\n ***QAPPSELECT_FinalSelect ucResult = %02x",ucResult);

		if(ucResult == QERROR_RESELECT)
		{
			ucResult =QERROR_SUCCESS;
			continue;
		}
	
		if(!ucResult)
		{
			memset(&QTransReqInfo,0x00,sizeof(QTRANSREQINFO));
			QTransReqInfo.enTransType = enTransType;
			Os__read_date(aucDateTime);
			memcpy(QTransReqInfo.aucTransDate,"20",2);
			memcpy(&QTransReqInfo.aucTransDate[2],&aucDateTime[4],2);
			memcpy(&QTransReqInfo.aucTransDate[4],&aucDateTime[2],2);
			memcpy(&QTransReqInfo.aucTransDate[6],&aucDateTime[0],2);

			Os__read_time_sec(QTransReqInfo.aucTransTime);
			
			srand(CONV_AscLong(QTransReqInfo.aucTransDate,sizeof(QTransReqInfo.aucTransDate))
				+CONV_AscLong(QTransReqInfo.aucTransTime,sizeof(QTransReqInfo.aucTransTime)));
			QTransReqInfo.uiRandNum= rand();
//			memcpy(&QTransReqInfo.uiRandNum ,"\x73\x8D\x2B\x0A",4);		
			QTransReqInfo.enTransType = enTransType;
			QTransReqInfo.uiAmount = uiAmount;

			memcpy(QTransReqInfo.aucTerminalID,QConstParam.aucTerminalID,QTERMINALIDLEN);
			memcpy(QTransReqInfo.aucMerchantID,QConstParam.aucMerchantID,QMERCHANTIDLEN);
			memcpy(QTransReqInfo.aucMerchantName,QConstParam.aucMerchantName,QMERCHANTNAMELEN);
			QTransReqInfo.ucEntryMode = 0x02;
			QTransReqInfo.bForceOnline = QConstParam.bForceOnline;
			QTransReqInfo.uiTraceNumber = QConstParam.uiTraceNumber;
			QConstParam.uiTraceNumber++;
//			UTIL_WriteConstParamFile(&QConstParam);
			QDATA_TransInit(&QTransReqInfo);
		}
		if(!ucResult)
		{
			
			if(!ucResult) ucResult = QINTIAPP_GPO();

			if(ucResult == QERROR_RESELECT)
			{
				ucResult = QAPPSELECT_GetAppCandidate(&QCandidateList);
				continue;
			}
			else break;
		}
	}
	if(!ucResult)
	{
		Os__clr_display(255);
		strcpy((char*)aucBuff,"Amount:");
		Os__GB2312_display(0,0,aucBuff);
		memset(aucBuff,0x00,sizeof(aucBuff));
		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),uiAmount,2);
		Os__GB2312_display(1,0,aucBuff);
		Os__GB2312_display(3,0,(PUCHAR)"Processing");
		switch(QEMVTransInfo.EntryMode)
		{
		case ENTRYMODE_MSD:
			ucResult = QREADAPP_ReadData();
			if(!ucResult)
				ucResult = MSD_TrackInfo();
			if(!ucResult)
				ucResult = QERROR_NEEDARQC;
			break;
		case ENTRYMODE_QVSDC:
			ucResult = QVSDC_ProcessFlow();
			if(!ucResult||ucResult==QERROR_NEEDARQC)
			{
//				if(uiAmount>=QReaderParam.ulRCVMReqLimit)
				{
					ucCVMResult = QVERIFY_qVSDC(uiAmount);

					if(ucCVMResult)
						ucResult =QERROR_TRANSOVER;
				}
			}
			else if(ucResult==QERROR_TRANSOVER)
				ucResult = QERROR_SUCCESS;
			break;
		case ENTRYMODE_CSVSDC:
			break;
			
		}
		if(!ucResult||ucResult==QERROR_NEEDARQC)
		{
			
			Os__GB2312_display(2,0,(PUCHAR)"Card Read OK");
			Os__GB2312_display(3,0,(PUCHAR)"Processing");
			UTIL_GetKey(1);
		}
		
		
	}
	
		if(ucResult ==QERROR_NEEDARQC)
		{
			memset(aucRecvBuff,0x00,sizeof(aucRecvBuff));
			uiRecvLen = sizeof(aucRecvBuff);
			
//			for(ucRequestNum =0;ucRequestNum<3;ucRequestNum++)
			{
				
				if(!QConstParam.bBatchCapture)
					ucResult = MSG_FinaTransRequest(aucRecvBuff,&uiRecvLen);
				else 
					ucResult = MSG_AuthTransRequest(aucRecvBuff,&uiRecvLen);
				
				if(ucResult ==QERROR_OPENCOM || ucResult ==QERROR_SENDDATA )
				{
					bConnectSuccess = FALSE;
					
				}

				if(!ucResult)
				{
					ucResult = QAPPCRYPT_OnlineRespDataProcess(NULL,&aucRecvBuff[1],uiRecvLen-1);
					if(!ucResult)
					{
						bConnectSuccess = TRUE;
						pRespCode =QTRANSTAG_SearchTag(QALLPHASETAG,(PUCHAR)"\x8A");
						if(pRespCode&&pRespCode->uiLen==2)
						{
							ucAuthRespCode = CONV_AscLong(pRespCode->pTagValue,pRespCode->uiLen);
							if(!ucAuthRespCode)
								QEMVTransInfo.enTransResult =QONLINEAPPROVED;
							else
								QEMVTransInfo.enTransResult =QONLINEDECLINED;
//							break;
						}
					}
					
				}
				else 
				{
					QEMVTransInfo.enTransResult =QONLINEDECLINED;
					QEMVTransInfo.bReversal = TRUE;
					ucResult=QERROR_SUCCESS;
				}
			}
/*
			if(ucRequestNum>=3)
			{
				bConnectSuccess=FALSE;
				QEMVTransInfo.bReversal = TRUE;
			}
*/			
		}
		
		
	
	memset(aucTransResult,0x00,sizeof(aucTransResult));
	if(ucResult)
	{
	//	DIGITAL_Light(LIGHT_TERMINATED);
	//	DIGITAL_Beep(BEEP_TERMINATED);
		if(ucResult==QERROR_CHANGEINTERFACE)
			strcpy((char*)aucTransResult,"CHANGE INTERFACE");
		else
			strcpy((char*)aucTransResult,"TRANS TERMINATED");
		Os__clr_display(255);
		Os__GB2312_display(1,0,aucTransResult);
	}
	else
	{
		switch(QEMVTransInfo.enTransResult)
		{
		case	QONLINEAPPROVED:
		case	QOFFLINEAPPROVED:
//		case	QUNABLEONLINE_OFFLINEAPPROVED:
//			DIGITAL_Light(LIGHT_APPROVED);
//			DIGITAL_Beep(BEEP_APPROVED);
			if(QEMVTransInfo.enTransResult==QONLINEAPPROVED)
				strcpy((char*)aucTransResult,"ONLINE APPROVED");
			else
				strcpy((char*)aucTransResult,"OFFLINE APPROVED");
			break;
		case	QONLINEDECLINED:
		case	QOFFLINEDECLINED:
//			DIGITAL_Light(LIGHT_DECLINED);
//			DIGITAL_Beep(BEEP_DECLINED);
			if(QEMVTransInfo.enTransResult==QONLINEDECLINED)
				strcpy((char*)aucTransResult,"ONLINE DECLINED");
			else
				strcpy((char*)aucTransResult,"OFFLINE DECLINED");
			break;
		}
		Os__clr_display(255);
		Os__GB2312_display(1,0,aucTransResult);
		pAOSA = QTRANSTAG_SearchTag(QALLPHASETAG,(PUCHAR)"\x9F\x5D");
		if(pAOSA)
		{
			memset(aucBuff,0x00,sizeof(aucBuff));
			uiAmount =(UINT)CONV_BcdLong(pAOSA->pTagValue,pAOSA->uiLen*2);
			sprintf(aucBuff,"       %d.%02d",uiAmount/100,uiAmount%100);
			Os__GB2312_display(2,0,(unsigned char*)"OFFLINE AMOUNT:");
			Os__GB2312_display(3,0,(unsigned char*)aucBuff);
		}
		
	}
	UTIL_GetKey(3);
	QTRANSTAG_SetTag(QALLPHASETAG,(PUCHAR)"\xDF\x32",aucTransResult,strlen((char*)aucTransResult));
	CompleteTrans();
	UTIL_WriteConstParamFile(&QConstParam);
	return ucResult;
}

UCHAR	CheckTransTVR(void)
{
	UCHAR ucI,aucVer[16],aucBuff[32],aucTempBuff[16];
	UCHAR aucCfgToSign[512],ucIndex,aucCfgHash[20];
	QEMVTRANSINFO QEMVTransInfo;
	
	Os__clr_display(255);
	Os__GB2312_display(0,0,"TVR value:");
	

	memset(aucBuff,0,sizeof(aucBuff));
	QDATA_TransInfo(&QEMVTransInfo);
	for(ucI=0;ucI<5;ucI++)
	{
		sprintf(aucTempBuff,"%02X ",QEMVTransInfo.EMVTVR[ucI]);
		strcat((char*)aucBuff,(char*)aucTempBuff);
	}
	Os__GB2312_display(1,0,aucBuff);
	

	sprintf(aucBuff,"TSI: %02X %02X",QEMVTransInfo.EMVTSI[0],QEMVTransInfo.EMVTSI[1]);
	Os__GB2312_display(2,0,aucBuff);
	


	memset(aucVer,0,16);
	QDATA_KernelVer(aucVer);
	sprintf(aucBuff,"%s",(char*)aucVer);
	Os__GB2312_display(3,0,aucBuff);

	Os__xget_key();

	//Display check sum(4-20 bytes) for Configurable Kernel 
	ucIndex=0;
	memset(aucCfgToSign,0,sizeof(aucCfgToSign));
	
	memcpy(aucCfgToSign+ucIndex,QConfig.aucTermCapab,sizeof(QConfig.aucTermCapab));
	ucIndex+=sizeof(QConfig.aucTermCapab);

	memcpy(aucCfgToSign+ucIndex,QConfig.aucTermAddCapab,sizeof(QConfig.aucTermAddCapab));
	ucIndex+=sizeof(QConfig.aucTermAddCapab);

	memcpy(aucCfgToSign+ucIndex,&(QConfig.ucTermType),sizeof(QConfig.ucTermType));
	ucIndex+=sizeof(QConfig.ucTermType);

	
/*
	memcpy(aucCfgToSign+ucIndex,&(TermInfo.bForceAccept),sizeof(TermInfo.bForceAccept));
	ucIndex+=sizeof(TermInfo.bForceAccept);
*/
	memcpy(aucCfgToSign+ucIndex,&(QConstParam.bForceOnline),sizeof(QConstParam.bForceOnline));
	ucIndex+=sizeof(QConstParam.bForceOnline);

	memcpy(aucCfgToSign+ucIndex,&(QConfig.ucDefaultDOLLen),sizeof(QConfig.ucDefaultDOLLen));
	ucIndex+=sizeof(QConfig.ucDefaultDOLLen);

	memcpy(aucCfgToSign+ucIndex,QConfig.aucDefaultDOL,QConfig.ucDefaultDOLLen);
	ucIndex+=QConfig.ucDefaultDOLLen;

	memcpy(aucCfgToSign+ucIndex,&(QConfig.ucDefaultTDOLLen),sizeof(QConfig.ucDefaultTDOLLen));
	ucIndex+=sizeof(QConfig.ucDefaultTDOLLen);

	memcpy(aucCfgToSign+ucIndex,QConfig.aucDefaultTDOL,QConfig.ucDefaultTDOLLen);
	ucIndex+=QConfig.ucDefaultTDOLLen;

	
	SHA1_Compute(aucCfgToSign,ucIndex,aucCfgHash);
	

	Os__clr_display(255);
	Os__GB2312_display(0,0,"Cfg Check Sum:");
	
	memset(aucBuff,0,sizeof(aucBuff));
	for(ucI=0;ucI<4;ucI++)
	{
		sprintf(aucTempBuff,"%02X ",aucCfgHash[ucI]);
		strcat((char*)aucBuff,(char*)aucTempBuff);
	}
	Os__GB2312_display(1,1,aucBuff);
	Os__xget_key();

}


void	CompleteTrans(void)
{

	UCHAR	ucResult,ucI,aucBuff[40],ucScriptResult;
	ULONG		uiAmount;
	QSCRIPTRESULT	*pScriptResult;
	
	ucResult =QERROR_SUCCESS;

	if(!QConstParam.bBatchCapture)
	{
		if(QEMVTransInfo.bReversal)
			ucResult = MSG_ReversalRequest();
	}
	
	if(!ucResult&&TransCapture(QEMVTransInfo.enTransResult))  
		ucResult =MSG_AddBatchRecord(Msg_EMV_BatchTrans);
	
	if(TransCapture(QEMVTransInfo.enTransResult))
	{
		ucResult = AddTransLog();
		if(QConstParam.bPrintTicket)
			PrintTransTicket();
	}
	if(ucResult)
		QERROR_DisplayMsg(ucResult);
	
//	MSG_TransResult();
	
	
	
}

UCHAR	AddTransLog()
{
	TRANSRECORD	TransRecord;
	UCHAR		ucResult;
	USHORT		uiLen;
	
	memset(&TransRecord, 0x00 ,sizeof(TransRecord));
	TransRecord.ucTransType = QConstParam.ucTransType;

	uiLen = QMAXPANDATALEN;
	QTRANSTAG_GetTagValue(QALLPHASETAG,(PUCHAR)"\x5A",TransRecord.aucPAN,&uiLen);
	TransRecord.ucPANLen = uiLen;
	
	uiLen = sizeof(UCHAR);
	QTRANSTAG_GetTagValue(QALLPHASETAG,(PUCHAR)"\x5F\x34",&TransRecord.ucPANSeq,&uiLen);

	uiLen = QAUTHCODELEN;
	QTRANSTAG_GetTagValue(QALLPHASETAG,(PUCHAR)"\x89",TransRecord.aucAuthCode,&uiLen);

	uiLen = sizeof(UINT);
	QTRANSTAG_GetTagValue(QALLPHASETAG,(PUCHAR)"\x9F\x02",(PUCHAR)&TransRecord.uiAmount,&uiLen);
	TransRecord.uiAmount =QEMVTransInfo.uiTransAmount;
	TransRecord.uiOtherAmount =QEMVTransInfo.uiOtherTransAmount;
	memcpy(TransRecord.aucTransDate,QTransReqInfo.aucTransDate,sizeof(TransRecord.aucTransDate));
	memcpy(TransRecord.aucTransTime,QTransReqInfo.aucTransTime,sizeof(TransRecord.aucTransTime));

	ucResult =FILE_InsertRecordByFileName(TRANSLOG,&TransRecord,sizeof(TRANSRECORD));
	if(ucResult)
	{
		QConstParam.uiTotalAmount +=TransRecord.uiAmount;
		QConstParam.uiTotalNums++;
		ucResult = UTIL_WriteConstParamFile(&QConstParam);
	}
	return ucResult;
	
}


UINT	CalcLogTransAmount(void)
{
	UINT	uiLogAmount,uiRecordNums,uiI;
	UCHAR	ucResult,aucPAN[QMAXPANDATALEN];
	USHORT	uiLen;
	TRANSRECORD	TransRecord;
	uiLogAmount = 0;

	ucResult = FILE_ReadRecordNumByFileName(TRANSLOG,&uiRecordNums);

	if(!ucResult)
	{
		uiLen = QMAXPANDATALEN;
		memset(aucPAN,0x00,sizeof(aucPAN));
		ucResult = QTRANSTAG_GetTagValue(QALLPHASETAG,(PUCHAR)"\x5A",aucPAN,&uiLen);
	}
#if 1
	for(uiI =1; !ucResult && uiI <= uiRecordNums; uiI++)
	{
		ucResult = FILE_ReadRecordByFileName(TRANSLOG,uiI,&TransRecord,sizeof(TRANSRECORD));
		if(!ucResult)
		{
			if(!memcmp(aucPAN,&TransRecord.aucPAN,QMAXPANDATALEN))
			{
				uiLogAmount += TransRecord.uiAmount;
//				uiLogAmount += TransRecord.uiOtherAmount;
			}
		}
	}
#else
	if(QConstParam.bPBOC20)
	{
//		for(uiI =1; !ucResult && uiI <= uiRecordNums; uiI++)
		for(uiI =uiRecordNums; !ucResult && uiI >0; uiI--)
		{
			ucResult = FILE_ReadRecordByFileName(TRANSLOG,uiI,&TransRecord,sizeof(TRANSRECORD));
			if(!ucResult)
			{
				if(!memcmp(aucPAN,&TransRecord.aucPAN,QMAXPANDATALEN))
				{
					uiLogAmount += TransRecord.uiAmount;
					break;
	//				uiLogAmount += TransRecord.uiOtherAmount;
				}
			}
		}
	}
	else
	{
		for(uiI =1; !ucResult && uiI <= uiRecordNums; uiI++)
		{
			ucResult = FILE_ReadRecordByFileName(TRANSLOG,uiI,&TransRecord,sizeof(TRANSRECORD));
			if(!ucResult)
			{
				if(!memcmp(aucPAN,&TransRecord.aucPAN,QMAXPANDATALEN))
				{
					uiLogAmount += TransRecord.uiAmount;
	//				uiLogAmount += TransRecord.uiOtherAmount;
				}
			}
		}
	}
#endif

	return uiLogAmount;
	
}


void ProcessMag(BOOL bFallBack,unsigned char* bufMag)
{
	UCHAR i,len1;
	char buf[32],pbuf[32];
	

	memset(MagTrack2,0x00,sizeof(MagTrack2));
	if(bufMag[2]!=0)//Track 2 data is not correct.
		return;
	len1=bufMag[3];
	for(i=0;i<len1;i++)
	{
		MagTrack2[i]=bufMag[4+i] | 0x30;
	}
	for(i=1;i<24;i++)
	{
		if(MagTrack2[i]=='=') break;
	}
	if(i<23)
	{
		Os__clr_display(255);
		memset(pbuf,0,32);
		memcpy(pbuf,MagTrack2,i);
		OSMMI_DisplayASCII(0x30,2, 0, (PUCHAR)"Card NO:");
		OSMMI_DisplayASCII(0x30,3, 0,  (PUCHAR)pbuf);
		
		memset(pbuf,0,32);
		memcpy(pbuf,(PUCHAR)&MagTrack2[i+1],4);
		
		sprintf(buf,"EXP DATE: %4s",pbuf);
		OSMMI_DisplayASCII(0x30,5,0, (PUCHAR)buf);
		
		memset(pbuf,0,32);
		memcpy(pbuf,&MagTrack2[i+5],3);
		
		sprintf(buf,"Service Code: %3s",pbuf);
		OSMMI_DisplayASCII(0x30,6,0, (PUCHAR)buf);
	}

	Os__xget_key();
	if(pbuf[0]=='2'||pbuf[0]=='6')//2CM.086.00
	{
		if(!bFallBack)
		{
			Os__clr_display(255);
			Os__GB2312_display(2,0,(PUCHAR)"Use Chip Reader");
			Os__xget_key();
		}
	}
	
}

UCHAR SelectAccountType(void)
{
	UCHAR k;
	char y,i,ch;
	char str[5],tempbuf[32];
	memset(str,0,5);
	str[0]=0x2A;//'*'
	k=18;

	Os__clr_display(255);

	OSMMI_DisplayASCII(0x30,0,0,(PUCHAR)"  Select Account");
	
	
	OSMMI_DisplayASCII(0x30,2,0,(PUCHAR)"1. Default");
	OSMMI_DisplayASCII(0x30,3,0,(PUCHAR)"2. Saving");
	OSMMI_DisplayASCII(0x30,4,0,(PUCHAR)"3. Debit");
	OSMMI_DisplayASCII(0x30,5,0,(PUCHAR)"4. Credit");

	switch (QConstParam.ucAccountType){
		case 0x00:
			y=2;
			break;
		case 0x10:
			y=3;
			break;
		case 0x20:
			y=4;
			break;
		case 0x30:
			y=5;
			break;
		default:
			y=6;
			break;
	}
	if(y<6)
		OSMMI_DisplayASCII(0x30,y,k,(PUCHAR)str);

	while (1) {
        ch = Os__xget_key();
        switch (ch){
			case '1':
				QConstParam.ucAccountType=0x00;
				OSMMI_DisplayASCII(0x30,y,k,(PUCHAR)" ");
				OSMMI_DisplayASCII(0x30,2,k,(PUCHAR)str);
				y=2;
				break;
			case '2':
				QConstParam.ucAccountType=0x10;
				OSMMI_DisplayASCII(0x30,y,k,(PUCHAR)" ");
				OSMMI_DisplayASCII(0x30,3,k,(PUCHAR)str);
				y=3;
				break;
			case '3':
				QConstParam.ucAccountType=0x20;
				OSMMI_DisplayASCII(0x30,y,k,(PUCHAR)" ");
				OSMMI_DisplayASCII(0x30,4,k,(PUCHAR)str);
				y=4;
				break;
			case '4':
				QConstParam.ucAccountType=0x30;
				OSMMI_DisplayASCII(0x30,y,k,(PUCHAR)" ");
				OSMMI_DisplayASCII(0x30,5,k,(PUCHAR)str);
				y=5;
				break;
			case KEY_ENTER:
			case KEY_CLEAR:
                break;
			default:
				break;
		}
		if(ch==KEY_ENTER || ch==KEY_CLEAR)
			break;
		else 
			continue;
	}
	return ch;
}

BOOL	TransCapture(QTRANSRESULT	enTransResult)
{
	switch(enTransResult)
	{
	case	QONLINEAPPROVED:
	case	QOFFLINEAPPROVED:
	case	QUNABLEONLINE_OFFLINEAPPROVED:
		return	TRUE;
	case	QOFFLINEDECLINED:
	case	QONLINEDECLINED:
	case	QUNABLEONINE_OFFLINEDECLINED:
		return	FALSE;
		
	}
	
}
#if 1
void	PrintTransTicket(void)
{
	
	UCHAR	aucBuff[200],aucPAN[QMAXPANDATALEN],ucI,ucLen,aucAID[32];
	UCHAR	aucTVR[5],aucTSI[2],ucResult,ucTrack2Len,*pstr;
	USHORT	uiPANLen,uiLen;
	QTRANSTAGDATA *  pAOSA,*pTrack2;
	UINT		uiAmount;
	
	Os__clr_display(255);
	Os__GB2312_display(1,2,(uchar *)"PRINTING");
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),QConstParam.aucMerchantName,QMERCHANTNAMELEN);
	Os__GB2312_xprint(aucBuff,0x00);
//	Os__GB2312_xprint("\n",0x00);

	strcpy((char*)aucBuff,"Terminal ID:");
	Os__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff,QConstParam.aucTerminalID,QTERMINALIDLEN);
	Os__GB2312_xprint(aucBuff,0x00);

	strcpy((char*)aucBuff,"Merchant ID:");
	Os__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),QConstParam.aucMerchantID,QMERCHANTIDLEN);
	Os__GB2312_xprint(aucBuff,0x00);
//	Os__GB2312_xprint("\n",0x00);

	strcpy((char*)aucBuff,"Card PAN:");
	Os__GB2312_xprint(aucBuff,0x00);
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	uiPANLen = sizeof(aucPAN);
	ucLen =strlen((char*)aucBuff);
	pTrack2 =QTRANSTAG_SearchTag(QALLPHASETAG,(PUCHAR)"\x57");
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	CONV_HexAsc(aucBuff,pTrack2->pTagValue,pTrack2->uiLen*2);
	pstr = (PUCHAR)strchr((char*)aucBuff,'D');
	*pstr =0;
	ucTrack2Len=pstr-aucBuff;
	ucTrack2Len = pTrack2->uiLen*2;
	if(aucBuff[ucTrack2Len -1]=='F')
		ucTrack2Len --;

	Os__GB2312_xprint(aucBuff,0x00);

	memset(aucBuff,0x00,sizeof(aucBuff));
	uiLen = sizeof(aucAID);
	strcpy((char*)aucBuff,"AID:");
	ucI =strlen((char*)aucBuff);
	ucResult = QTRANSTAG_GetTagValue(QALLPHASETAG,(PUCHAR)"\x4F",aucAID,(USHORT*)&uiLen);
	if(ucResult) uiLen=0;
//	memcpy(aucBuff+ucLen,aucAID,ucI);
	CONV_HexAsc(aucBuff+ucI,aucAID,uiLen*2);
	Os__GB2312_xprint(aucBuff,0x00);

	memset(aucBuff,0x00,sizeof(aucBuff));
	Os__GB2312_xprint((PUCHAR)"Trans Date     Time",0x00);
	memcpy(aucBuff,QEMVTransInfo.aucTransDate,QTRANSDATELEN);
	memcpy(aucBuff+strlen((char*)aucBuff),"    ",4);
	memcpy(aucBuff+strlen((char*)aucBuff),QEMVTransInfo.aucTransTime,2);
	memcpy(aucBuff+strlen((char*)aucBuff),":",1);
	memcpy(aucBuff+strlen((char*)aucBuff),&QEMVTransInfo.aucTransTime[2],2);
	memcpy(aucBuff+strlen((char*)aucBuff),":",1);
	memcpy(aucBuff+strlen((char*)aucBuff),&QEMVTransInfo.aucTransTime[4],2);
	Os__GB2312_xprint(aucBuff,0x00);
	if(QEMVTransInfo.EntryMode==ENTRYMODE_MSD)
		Os__GB2312_xprint("MSD",0x00);
	else if(QEMVTransInfo.EntryMode==ENTRYMODE_QVSDC)
		Os__GB2312_xprint("QVSDC",0x00);
	else if(QEMVTransInfo.EntryMode==ENTRYMODE_CSVSDC)
		Os__GB2312_xprint("CONTACTLESS VSDC",0x00);
	
	strcpy((char*)aucBuff,"Amount:");
	Os__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),QEMVTransInfo.uiTransAmount,2);
	Os__GB2312_xprint(aucBuff,0x00);

	if(QEMVTransInfo.enTransType ==QCASHBACK)
	{
		strcpy((char*)aucBuff,"Other Amount:");
		Os__GB2312_xprint(aucBuff,0x00);
		memset(aucBuff,0x00,sizeof(aucBuff));
		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),QEMVTransInfo.uiOtherTransAmount,2);
		Os__GB2312_xprint(aucBuff,0x00);
	}
	pAOSA = QTRANSTAG_SearchTag(QALLPHASETAG,(PUCHAR)"\x9F\x5D");
	if(pAOSA)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiAmount =(UINT)CONV_BcdLong(pAOSA->pTagValue,pAOSA->uiLen*2);
		sprintf(aucBuff,"       %d.%02d",uiAmount/100,uiAmount%100);
		Os__GB2312_xprint((unsigned char*)"OFFLINE AMOUNT:",0x00);
		Os__GB2312_xprint((unsigned char*)aucBuff,0x00);
	}
	
	if(QEMVTransInfo. bCheckSignature)
		Os__GB2312_xprint((uchar*)"Signature: ___________________",0x00);
//	Os__GB2312_xprint("\n\n",0x00);
	Os__GB2312_xprint((uchar*)"    Transaction Approved",0x00);
	Os__xlinefeed(60);
	
}
#else

void	PrintTransTicket(void)
{
	
	UCHAR	aucBuff[200],aucPAN[QMAXPANDATALEN],ucI,ucLen,aucAID[32];
	UCHAR	aucTVR[5],aucTSI[2],ucResult,ucTrack2Len,*pstr;
	USHORT	uiPANLen,uiLen;
	QTRANSTAGDATA *  pAOSA,*pTrack2;
	UINT		uiAmount;
	
	Os__clr_display(255);
	Os__GB2312_display(1,2,(uchar *)"PRINTING");
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),QConstParam.aucMerchantName,QMERCHANTNAMELEN);
	ext__GB2312_xprint(aucBuff,0x00);
//	Os__GB2312_xprint("\n",0x00);

	strcpy((char*)aucBuff,"Terminal ID:");
	ext__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff,QConstParam.aucTerminalID,QTERMINALIDLEN);
	ext__GB2312_xprint(aucBuff,0x00);

	strcpy((char*)aucBuff,"Merchant ID:");
	ext__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),QConstParam.aucMerchantID,QMERCHANTIDLEN);
	ext__GB2312_xprint(aucBuff,0x00);
//	Os__GB2312_xprint("\n",0x00);

	strcpy((char*)aucBuff,"Card PAN:");
	ext__GB2312_xprint(aucBuff,0x00);
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	uiPANLen = sizeof(aucPAN);
	ucLen =strlen((char*)aucBuff);
	pTrack2 =QTRANSTAG_SearchTag(QALLPHASETAG,(PUCHAR)"\x57");
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	CONV_HexAsc(aucBuff,pTrack2->pTagValue,pTrack2->uiLen*2);
	pstr = (PUCHAR)strchr((char*)aucBuff,'D');
	*pstr =0;
	ucTrack2Len=pstr-aucBuff;
	ucTrack2Len = pTrack2->uiLen*2;
	if(aucBuff[ucTrack2Len -1]=='F')
		ucTrack2Len --;

	ext__GB2312_xprint(aucBuff,0x00);

	memset(aucBuff,0x00,sizeof(aucBuff));
	uiLen = sizeof(aucAID);
	strcpy((char*)aucBuff,"AID:");
	ucI =strlen((char*)aucBuff);
	ucResult = QTRANSTAG_GetTagValue(QALLPHASETAG,(PUCHAR)"\x4F",aucAID,(USHORT*)&uiLen);
	if(ucResult) uiLen=0;
//	memcpy(aucBuff+ucLen,aucAID,ucI);
	CONV_HexAsc(aucBuff+ucI,aucAID,uiLen*2);
	ext__GB2312_xprint(aucBuff,0x00);

	memset(aucBuff,0x00,sizeof(aucBuff));
	ext__GB2312_xprint((PUCHAR)"Trans Date     Time",0x00);
	memcpy(aucBuff,QEMVTransInfo.aucTransDate,QTRANSDATELEN);
	memcpy(aucBuff+strlen((char*)aucBuff),"    ",4);
	memcpy(aucBuff+strlen((char*)aucBuff),QEMVTransInfo.aucTransTime,2);
	memcpy(aucBuff+strlen((char*)aucBuff),":",1);
	memcpy(aucBuff+strlen((char*)aucBuff),&QEMVTransInfo.aucTransTime[2],2);
	memcpy(aucBuff+strlen((char*)aucBuff),":",1);
	memcpy(aucBuff+strlen((char*)aucBuff),&QEMVTransInfo.aucTransTime[4],2);
	ext__GB2312_xprint(aucBuff,0x00);
	if(QEMVTransInfo.EntryMode==ENTRYMODE_MSD)
		ext__GB2312_xprint("MSD",0x00);
	else if(QEMVTransInfo.EntryMode==ENTRYMODE_QVSDC)
		ext__GB2312_xprint("QVSDC",0x00);
	else if(QEMVTransInfo.EntryMode==ENTRYMODE_CSVSDC)
		ext__GB2312_xprint("CONTACTLESS VSDC",0x00);
	
	strcpy((char*)aucBuff,"Amount:");
	ext__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),QEMVTransInfo.uiTransAmount,2);
	ext__GB2312_xprint(aucBuff,0x00);

	if(QEMVTransInfo.enTransType ==QCASHBACK)
	{
		strcpy((char*)aucBuff,"Other Amount:");
		ext__GB2312_xprint(aucBuff,0x00);
		memset(aucBuff,0x00,sizeof(aucBuff));
		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),QEMVTransInfo.uiOtherTransAmount,2);
		ext__GB2312_xprint(aucBuff,0x00);
	}
	pAOSA = QTRANSTAG_SearchTag(QALLPHASETAG,(PUCHAR)"\x9F\x5D");
	if(pAOSA)
	{
		memset(aucBuff,0x00,sizeof(aucBuff));
		uiAmount =(UINT)CONV_BcdLong(pAOSA->pTagValue,pAOSA->uiLen*2);
		sprintf(aucBuff,"       %d.%02d",uiAmount/100,uiAmount%100);
		ext__GB2312_xprint((unsigned char*)"OFFLINE AMOUNT:",0x00);
		ext__GB2312_xprint((unsigned char*)aucBuff,0x00);
	}
	
	if(QEMVTransInfo. bCheckSignature)
		ext__GB2312_xprint((uchar*)"Signature: ___________________",0x00);
//	Os__GB2312_xprint("\n\n",0x00);
	ext__GB2312_xprint((uchar*)"    Transaction Approved",0x00);
	ext__xlinefeed(30);
	
}
#endif

UCHAR	ReadICTransLog(void)
{
	DRV_OUT *pxIcc;
	DRV_OUT *pxKey;
	UCHAR	aucResetBuf[50],ucAppIndex,ucResult,aucEntryBuf[10],ucKey;
	USHORT	uiLen;
	BOOL	bConfirm;
	unsigned char ucDOLLen,aucDOL[100],*pDOL,*pDataOut,ucTagLen,aucAmtBuf[30];
	UCHAR	ucRecordSFI,ucRecordNums,ucI,aucBuff[30],ucICCardType,ucResetBufLen;
	unsigned char ucRecordNum,ucJ;
	unsigned char aucTransDate[3],aucTransTime[3],aucAuthAmount[6],aucOtherAmount[6],
				  aucCountryCode[2],aucTransCurcyCode[2],aucMerchantName[20];
	unsigned char ucTransType,aucATC[4];
	QCANDIDATELIST	QCandidateList;
	
	while(1)
	{
		pxIcc = Os__ICC_insert();
		pxKey = Os__get_key();

		Os__clr_display(255);
		Os__GB2312_display(2,2,"Insert Card");
	
		while((pxIcc->gen_status != DRV_ENDED)
		      &&(pxKey->gen_status != DRV_ENDED));
		
		if(pxKey->gen_status==DRV_ENDED)
		{
			Os__abort_drv(drv_icc);
			ucKey = pxKey->xxdata[1];
			if(ucKey==KEY_CLEAR) return;
				
					
		}
		else if(pxIcc->gen_status== DRV_ENDED)
		{
		
			Os__abort_drv(drv_mmi);
			ucICCardType =pxIcc->xxdata[1];
			ucResetBufLen=sizeof(aucResetBuf);
			if(ucICCardType==0x39)
				ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);
		
			if(ucICCardType!=ICC_ASY)
				continue;

			Os__clr_display(255);
			Os__GB2312_display(2,2,"Processing");
		//	QConfig.ucTransCurrencyExp = 0x02;
			QConfig.ucCAPKNums =ucQTermCAPKNum;
			QConfig.pTermSupportCAPK =QTermCAPK;
			QConfig.ucTermSupportAppNum = ucQTermAIDNum;
			QConfig.pTermSupportApp = QTermAID;
			QConfig.ucQIPKRevokeNum =ucQIPKRevokeNum;
			QConfig.pIPKRevoke =QIPKRevoke;
			QConfig.ucQExceptFileNum = ucQExceptFileNum;
			QConfig.pExceptPAN = QExceptFile;
			QConfig.pfnQICC = QICC;
			
			QDATA_TermInit(&QConfig);
			ucResult = QAPPSELECT_CreateAppCandidate(&QCandidateList);
			
			bConfirm =FALSE;
			if(QCandidateList.CandidateNums ==1 &&QCandidateList.CandidateApp[0].bAppPriorityExist)
			{
				if(QCandidateList.CandidateApp[0].ucAppPriorityIndicator &QAPPNEEDCONFIRM)
					bConfirm = TRUE;
			}
			else if (QCandidateList.CandidateNums >1)
				bConfirm = TRUE;
				
			while(!ucResult && QCandidateList.CandidateNums)
			{
				ucAppIndex = 0;
				
				if(bConfirm)
				{
					ucResult = CardHolderConfirmApp(&QCandidateList,&ucAppIndex);
				}
				
				if(!ucResult)
				{
					ucResult = QAPPSELECT_FinalSelect(ucAppIndex,&QCandidateList);
					if(QICC_CommandResult() == QCARDSWDEF_INVALIDFILE)
					{
						ucResult =QERROR_SUCCESS;
						break;
					}
					if(ucResult == QERROR_RESELECT)
					{
						ucResult =QERROR_SUCCESS;
						continue;
					}
					else break;
				}
			}
			
			if(ucResult)	
			{
				QERROR_DisplayMsg(ucResult);
				break;
			}
			else
			{
				QTAG_INIT(( QDATAPROPERTY*)&QICCardProperty,FALSE);
				if(!ucResult)
					ucResult = QTAG_Unpack(QIccOut.aucDataOut,QIccOut.uiLenOut);
				if(!ucResult)
					ucResult = QTRANSTAG_CopyQICDataInfo(&QICDataInfo,QADFPHASETAG);
			}
			if(ucResult) break;
			uiLen =sizeof(aucEntryBuf);
			ucResult=QTRANSTAG_GetTagValue(QALLPHASETAG,(PUCHAR)"\x9F\x4D",aucEntryBuf,&uiLen);
			if(ucResult||uiLen!=2)
			{
				Os__clr_display(255);
				Os__GB2312_display(2,2,"No Log Entry");
				Os__xget_key();
				break;
			}
			ucRecordSFI=aucEntryBuf[0];
			ucRecordNums=aucEntryBuf[1];
			
			ucResult=QICC_GetData((PUCHAR)"\x9F\x4F",2);
			if(ucResult||QICC_CommandResult() !=QCARDSWDEF_SUCCESS) 
				break;
			
			ucDOLLen =QIccOut.aucDataOut[2];
			memset(aucDOL,0x00,sizeof(aucDOL));
			memcpy(aucDOL,&QIccOut.aucDataOut[3],ucDOLLen);
			ucRecordNum=0;
			
			ucResult=QERROR_SUCCESS;
			for(ucI=1;!ucResult&&ucI<=ucRecordNums;ucI++)
			{
				ucResult=QICC_ReadRecordbySFI(ucRecordSFI,ucI);
				if(!ucResult&&QICC_CommandResult() == QCARDSWDEF_SUCCESS)
				{
					
					ucRecordNum++;
					pDOL=aucDOL;
					pDataOut=(unsigned char *)&QIccOut.aucDataOut;
					memset(aucTransDate,0x00,sizeof(aucTransDate));
					memset(aucTransTime,0x00,sizeof(aucTransTime));
					memset(aucAuthAmount,0x00,sizeof(aucAuthAmount));
					memset(aucOtherAmount,0x00,sizeof(aucOtherAmount));
					memset(aucCountryCode,0x00,sizeof(aucCountryCode));
					memset(aucTransCurcyCode,0x00,sizeof(aucTransCurcyCode));
					memset(aucMerchantName,0x00,sizeof(aucMerchantName));
					memset(aucATC,0x00,sizeof(aucATC));
					ucTransType=QCASH;
					
					while(pDataOut<QIccOut.aucDataOut+QIccOut.uiLenOut)
					{
						ucTagLen =TagByteNums(*pDOL);
						
						if(!memcmp(pDOL,"\x9A",ucTagLen)) 
							memcpy(aucTransDate,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x21",ucTagLen))
							memcpy(aucTransTime,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x02",ucTagLen))
							memcpy(aucAuthAmount,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x03",ucTagLen))
							memcpy(aucOtherAmount,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x1A",ucTagLen))
							memcpy(aucCountryCode,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x5F\x2A",ucTagLen))
							memcpy(aucTransCurcyCode,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x4E",ucTagLen))
							memcpy(aucMerchantName,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9C",ucTagLen))
							memcpy(&ucTransType,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x36",ucTagLen))
							memcpy(aucATC,pDataOut,*(pDOL+ucTagLen));
							
						pDataOut+=*(pDOL+ucTagLen);
						pDOL+=ucTagLen+1;
						
					}
					
					Os__clr_display(255);
					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"TransDate:");
					bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransDate,6);
					OSMMI_DisplayASCII(0x30,0,0,aucBuff);
					
					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"TransTime:");
					bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransTime,6);
					OSMMI_DisplayASCII(0x30,1,0,aucBuff);
					
					memset(aucBuff,0x00,sizeof(aucBuff));
					memset(aucAmtBuf,0x00,sizeof(aucAmtBuf));
					strcpy((char *)aucBuff,(char *)"Amt:");
					bcd_asc(aucAmtBuf,aucAuthAmount,12);
					for(ucJ=0;ucJ<12;ucJ++)
						if(aucAmtBuf[ucJ]!='0') break;
					if(ucJ>=10)
					{
						memcpy(&aucBuff[4],"0.",2);
						memcpy(&aucBuff[6],&aucAmtBuf[ucJ],12-ucJ);
					}else
					{
						memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[ucJ],12-ucJ-2);
						strcat((char*)aucBuff,".");
						memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[10],2);
					}		
					OSMMI_DisplayASCII(0x30,2,0,aucBuff);
					
					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"Country:");
					bcd_asc(aucBuff+strlen((char*)aucBuff),aucCountryCode,4);
					OSMMI_DisplayASCII(0x30,3,0,aucBuff);
					
					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"Currency:");
					bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransCurcyCode,4);
					//Os__display(4,0,aucBuff);
					OSMMI_DisplayASCII(0x30,4,0,aucBuff);
					
					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"Name:");
					memcpy(aucBuff+strlen((char*)aucBuff),aucMerchantName,16);
					OSMMI_DisplayASCII(0x30,5,0,aucBuff);
					memset(aucBuff,0x00,sizeof(aucBuff));
					memcpy(aucBuff,&aucMerchantName[16],sizeof(aucMerchantName)-16);	
					OSMMI_DisplayASCII(0x30,6,0,aucBuff);
					
					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"TransType:");
					hex_asc(aucBuff+strlen((char*)aucBuff),&ucTransType,2);
					OSMMI_DisplayASCII(0x30,7,0,aucBuff);
					Os__xget_key();
					
					Os__clr_display(255);
					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"ATC:");
					hex_asc(aucBuff+strlen((char*)aucBuff),aucATC,4);
					OSMMI_DisplayASCII(0x30,0,0,aucBuff);
					OSMMI_DisplayASCII(0x30,7,0,(unsigned char*)"Any Key To Countinue");
					Os__xget_key();    
				}
			}
			if(ucRecordNum=0)
			{
				Os__clr_display(255);
				Os__GB2312_display(2,0,(unsigned char *)"无交易流水");
				Os__xget_key();
			}
		}
	}

	Os__clr_display(255);
	Os__GB2312_display(0,0,(unsigned char*)"请拔出卡");
	Os__ICC_remove();
	return;			
}

UCHAR	OnlineRespDataProcess(void)
{
	UCHAR	ucResult;
#if 0	
	ucResult =QERROR_SUCCESS;
	if(!ucResult)
	{
		QTRANSTAG_SetTag(QALLPHASETAG,"\x8A",aucAuthRespCode,uiAuthRespCodeLen);
		QTRANSTAG_SetTag(QALLPHASETAG,"\x89",aucAuthCode,uiAuthCodeLen);
		if(uiIssureAuthDatLen)
			QTRANSTAG_SetTag(QALLPHASETAG,"\x91",aucIssureAuthData,uiIssureAuthDatLen);
	}
	for(ucI=0;!ucResult&&ucI<ucScriptNums;ucI++)
	{
		ucResult =QAPPCRIPT_AppendScript(aucScriptData,uiScriptLen);
	}
#endif
	return ucResult;
	
}
