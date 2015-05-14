#include "test.h"
#include "typeinfo"
#include "type_traits"
#include "TypeDef.h"


ICC_ORDER IccOrder;

UCHAR	OnEve_CheckExistFile(PUCHAR pszFileName,USHORT uiFileLen,PUCHAR pLoadAddr);

ICC_ANSWER* pAnswer;

unsigned char SMART_WarmReset(
        unsigned char ucReader,
        unsigned char ucCardStandard,
        unsigned char * pucReset,
        unsigned short * puiLength)
{
    unsigned char ucCardType;
    /* Power On */


    /* Wait for a Card */


    IccOrder.pt_order_in = &OrderType;
    IccOrder.ptout = pucReset;
    IccOrder.order = NEW_WARM_RESET;
    IccOrder.pt_order_in->new_power.card_standards = ucCardStandard;
    IccOrder.pt_order_in->new_power.GR_class_byte_00 = 1;
    IccOrder.pt_order_in->new_power.preferred_protocol = 0xFF;

    /* Send Command */
    pAnswer = Os__ICC_command(ucReader, &IccOrder);
    /* Answer Command */
    if ( pAnswer->drv_status != OK )
    {
        return (SMART_DRIVER_ERROR);
    }
    switch ( pAnswer->card_status )
    {
    case ICC_ASY:
    case ICC_SYN:
        ucCardType = pAnswer->card_status;
        break;
    default:
        return (SMART_CARD_ERROR);
    }
    if ( pAnswer->Len > *puiLength )
    {
        return (SMART_OWERFLOW);
    }
    *puiLength = pAnswer->Len;
    return (ucCardType);
}

UCHAR	EMVTransProcess(TRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount)
{
    UCHAR	ucResult,aucDateTime[10];
    CRYPTTYPE	CryptType;
    UINT		uiLogSumAmount;
    BOOL	bConfirm,bConnectSuccess,bHostApproval;
    UCHAR	ucAppIndex;
    CANDIDATELIST	CandidateList;
    UCHAR	aucRecvBuff[500],aucBuff[30],ucKey,ucRequestNum,ucLen;
    USHORT  uiRecvLen;
    CONFIG_PARAM	cp;

    Os__clr_display(255);
    if(ConstParam.bPBOC20==FALSE)
        Os__GB2312_display(2,2,"Processing");
    else
        Os__GB2312_display(2,2,"处理中");

//	EMVConfig.ucTransCurrencyExp = 0x02;
    EMVConfig.ucCAPKNums =ucTermCAPKNum;
    EMVConfig.pTermSupportCAPK =TermCAPK;
    EMVConfig.ucTermSupportAppNum = ucTermAIDNum;
    EMVConfig.pTermSupportApp = TermAID;
    EMVConfig.ucIPKRevokeNum =ucIPKRevokeNum;
    EMVConfig.pIPKRevoke =IPKRevoke;
    EMVConfig.ucExceptFileNum = ucExceptFileNum;
    EMVConfig.pExceptPAN = ExceptFile;
    EMVConfig.pfnEMVICC = EMVICC;
    EMVConfig.bPBOC20 =ConstParam.bPBOC20;
    EMVConfig.pfnEMVIDCardVerify =UTIL_IDCardVerify;
    EMVConfig.bIssuerReferal=ConstParam.bIssuerReferal;
    EMVConfig.bECSupport=ConstParam.bECSupport;

    EMVTAG_SetbDEBUGValue (ConstParam.bDEBUG);
    memset(&cp,0x00,sizeof(CONFIG_PARAM));
    cp =OSCFG_ReadParam();
    memset(aucBuff,0x00,sizeof(aucBuff));
    memcpy(aucBuff,cp.aucSerialNo,sizeof(cp.aucSerialNo));
    ucLen =strlen((char*)aucBuff);

    if(ucLen>=MAXIFDLEN)
        memcpy(EMVConfig.aucIFD,&aucBuff[ucLen-MAXIFDLEN],MAXIFDLEN);
    else
        memcpy(EMVConfig.aucIFD,aucBuff,ucLen);
    if(EMVConfig.bIssuerReferal)
    {
        EMVConfig.pfnIssuerReferalProcess =UTIL_IssuerReferalProcess;
    }
    if(ConstParam.bSelectAccount)
    {
        if(SelectAccountType()!=KEY_ENTER)
            return EMVERROR_CANCEL;
        EMVConfig.ucAccountType =ConstParam.ucAccountType;
    }

    EMVDATA_TermInit(&EMVConfig);
    ucResult = APPSELECT_CreateAppCandidate(&CandidateList);



    while(!ucResult && CandidateList.CandidateNums)
    {
        bConfirm =FALSE;
        if(CandidateList.CandidateNums ==1 &&CandidateList.CandidateApp[0].bAppPriorityExist)
        {
            if(CandidateList.CandidateApp[0].ucAppPriorityIndicator &APPNEEDCONFIRM)
                bConfirm = TRUE;
        }
        else if (CandidateList.CandidateNums >1)
            bConfirm = TRUE;

        ucAppIndex = 0;

        if(bConfirm)
        {
            ucResult = CardHolderConfirmApp(&CandidateList,&ucAppIndex);
        }

        if(!ucResult)
        {
            ucResult = APPSELECT_FinalSelect(ucAppIndex,&CandidateList);
            if(ucResult == EMVERROR_RESELECT)
            {
                Os__clr_display(255);
                if(ConstParam.bPBOC20==FALSE)
                    Os__GB2312_display(2,2,(PUCHAR)"Try Again");
                else
                    Os__GB2312_display(2,0,"再来一次");


                Os__xget_key();
                ucResult =EMVERROR_SUCCESS;
                continue;
            }
        }
        if(!ucResult)
        {
            memset(&TransReqInfo,0x00,sizeof(TRANSREQINFO));
            TransReqInfo.enTransType = enTransType;
            Os__read_date(aucDateTime);
            memcpy(TransReqInfo.aucTransDate,"20",2);
            memcpy(&TransReqInfo.aucTransDate[2],&aucDateTime[4],2);
            memcpy(&TransReqInfo.aucTransDate[4],&aucDateTime[2],2);
            memcpy(&TransReqInfo.aucTransDate[6],&aucDateTime[0],2);

            Os__read_time_sec(TransReqInfo.aucTransTime);

            srand(CONV_AscLong(TransReqInfo.aucTransDate,sizeof(TransReqInfo.aucTransDate))
                +CONV_AscLong(TransReqInfo.aucTransTime,sizeof(TransReqInfo.aucTransTime)));
            TransReqInfo.uiRandNum= rand();
            TransReqInfo.enTransType = enTransType;
            TransReqInfo.uiAmount = uiAmount;
#if 1
            if(enTransType == CASHBACK)
            {
                TransReqInfo.uiAmount += uiOtherAmount;
                TransReqInfo.uiOtherAmount = uiOtherAmount;

            }
#else
            TransReqInfo.uiOtherAmount = uiOtherAmount;
#endif
            memcpy(TransReqInfo.aucTerminalID,ConstParam.aucTerminalID,TERMINALIDLEN);
            memcpy(TransReqInfo.aucMerchantID,ConstParam.aucMerchantID,MERCHANTIDLEN);
            memcpy(TransReqInfo.aucMerchantName,ConstParam.aucMerchantName,MERCHANTNAMELEN);
            TransReqInfo.ucEntryMode = 0x02;
            TransReqInfo.bForceOnline = ConstParam.bForceOnline;
            TransReqInfo.uiTraceNumber = ConstParam.uiTraceNumber;
            ConstParam.uiTraceNumber++;
            UTIL_WriteConstParamFile(&ConstParam);
            EMVDATA_TransInit(&TransReqInfo);
        }
        if(!ucResult)
        {
            Os__clr_display(255);
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(2,2,"Processing");
            else
                Os__GB2312_display(2,2,"处理中");

            if(!ucResult) ucResult = INTIAPP_GPO();

            if(ucResult == EMVERROR_RESELECT)
            {
                ucResult = APPSELECT_GetAppCandidate(&CandidateList);
                Os__clr_display(255);
                if(ConstParam.bPBOC20==FALSE)
                    Os__GB2312_display(2,2,(PUCHAR)"Try Again");
                else
                    Os__GB2312_display(2,0,"再来一次");
                Os__xget_key();
                continue;
            }
            else break;
        }
    }
    /*//2cm.091
    if(!CandidateList.CandidateNums)
        return EMVERROR_NOSUPPORTAPP;
    */
    if(!ucResult) ucResult = READAPP_ReadData();
    if(!ucResult)
    {
        uiLogSumAmount = CalcLogTransAmount();
    }
    if(!ucResult) ucResult = DATAAUTH_Process();
    if(!ucResult) ucResult = PROCESSRESTRICT_Process();

    if(!ucResult)
    {
        Os__clr_display(255);

        if(EMVTransInfo.bECTrans)
        {
            memset(aucBuff,0x00,sizeof(aucBuff));
            memcpy(aucBuff,ECTransInfo.aucECICode,ECISSUERCODELEN);
            Os__GB2312_display(0,0,aucBuff);
            memset(aucBuff,0x00,sizeof(aucBuff));
            if(ConstParam.bPBOC20==FALSE)
            {
                strcpy((char*)aucBuff,"Balance:");
                UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff)-3,ECTransInfo.uiBalance,2);
            }
            else
            {
                strcpy((char*)aucBuff,"余额:");
                UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),ECTransInfo.uiBalance,2);
            }

            Os__GB2312_display(1,0,aucBuff);
        }
        memset(aucBuff,0x00,sizeof(aucBuff));

        if(ConstParam.bPBOC20==FALSE)
            strcpy((char*)aucBuff,"Amt:");
        else
            strcpy((char*)aucBuff,"金额:");

        UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),TransReqInfo.uiAmount,2);
#if 1

        Os__GB2312_display(2,0,aucBuff);
        if(ConstParam.bPBOC20==FALSE)
            Os__GB2312_display(3,2,(PUCHAR)"Confirm pls");
        else
            Os__GB2312_display(3,3,(PUCHAR)"确认");

        do
        {
            ucKey = Os__xget_key();
            if(ucKey ==KEY_CLEAR)	return EMVERROR_CANCEL;
            if(ucKey ==KEY_ENTER)	break;
        }while(1);
        //if(Os__xget_key()!=KEY_ENTER) return EMVERROR_CANCEL;
        Os__clr_display(255);
        if(ConstParam.bPBOC20==FALSE)
            Os__GB2312_display(2,2,"Processing");
        else
            Os__GB2312_display(2,2,"处理中");

#endif
    }
    if(!ucResult) ucResult = EMVVERIFY_Process();

    if(!ucResult)
    {
        ucResult = TERMRISKMANAGE_Process(uiLogSumAmount);
        if(ConstParam.bShowRandNum)
        {
            memset(aucBuff,0x00,sizeof(aucBuff));
            Os__clr_display(255);
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(1,2,"Random Num:");
            else
                Os__GB2312_display(1,0,"随机数:");

            //CONV_LongAsc(aucBuff,&EMVTransInfo.ucRandNum,2);
            CONV_CharAsc(aucBuff,2,&EMVTransInfo.ucRandNum);
            Os__GB2312_display(2,6,aucBuff);
            UTIL_GetKey(2);

        }
    }
    if(!ucResult)
    {
        CryptType = TERMACTANALYSIS_Process(TRUE);
        ucResult = APPCRYPT_OfflineProcess(CryptType);

        if(ucResult ==EMVERROR_NEEDARQC)
        {
            memset(aucRecvBuff,0x00,sizeof(aucRecvBuff));
            uiRecvLen = sizeof(aucRecvBuff);

            for(ucRequestNum =0;ucRequestNum<3;ucRequestNum++)
            {

                if(!ConstParam.bBatchCapture)
                    ucResult = MSG_FinaTransRequest(aucRecvBuff,&uiRecvLen);
                else
                    ucResult = MSG_AuthTransRequest(aucRecvBuff,&uiRecvLen);

                if(ucResult ==EMVERROR_OPENCOM || ucResult ==EMVERROR_SENDDATA )
                {
                    bConnectSuccess = FALSE;
                    break;
                }
#if 0
                else if( ucResult ==EMVERROR_RECEIVEDATA)
                {
                    bConnectSuccess = FALSE;
                    EMVTransInfo.bReversal = TRUE;
                    break;
                }
#endif
                if(!ucResult)
                {
                    ucResult = APPCRYPT_OnlineRespDataProcess(NULL,&aucRecvBuff[1],uiRecvLen-1);
                    if(!ucResult)
                    {
                        bConnectSuccess = TRUE;
                        break;
                    }
                }


            }
            if(ucRequestNum>=3)
            {
                bConnectSuccess=FALSE;
                EMVTransInfo.bReversal = TRUE;
            }
            if(bConnectSuccess == false && EMVTransInfo.bAdvice)
                ucResult = EMVERROR_CANCEL;
            else
                ucResult = APPCRYPT_OnlineProcess(bConnectSuccess/*,&aucRecvBuff[1],uiRecvLen-1*/);
        }


    }
    CompleteTrans();

    EMVTrans_Infor_Display();

    if(ucResult)
    {
        Os__clr_display(255);

        if(ConstParam.bPBOC20==FALSE)
            Os__GB2312_display(2,0,"TRANS TERMINATED");
        else
            Os__GB2312_display(2,0,"交易终止");

        if(ucResult ==EMVERROR_FALLBACK)
        {
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(3,0,(PUCHAR)"USE MAG CARD");
            else
                Os__GB2312_display(3,0,(PUCHAR)"请用磁条卡");
        }

        UTIL_GetKey(3);
    }
    else
    {
        Os__clr_display(255);
        switch(EMVTransInfo.enTransResult)
        {
        case OFFLINEAPPROVED:
            {
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(2,0,"OFFLINE APPROVED");
            else
                Os__GB2312_display(2,0,"脱机批准");
            }
            break;
        case OFFLINEDECLINED:
            {
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(2,0,"OFFLINE DECLINED");
            else
                Os__GB2312_display(2,0,"脱机拒绝");
            }
            break;

        case ONLINEAPPROVED:
            {
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(2,0,"ONLINE APPROVED");
            else
                Os__GB2312_display(2,0,"联机批准");
            }
            break;
        case ONLINEDECLINED:
            {
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(2,0,"ONLINE DECLINED");
            else
                Os__GB2312_display(2,0,"联机拒绝");
            }
            break;
        case UNABLEONLINE_OFFLINEAPPROVED:
            {
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(2,0,"OFFLINE APPROVED");
            else
                Os__GB2312_display(2,0,"脱机批准");
            }
            break;
        case UNABLEONINE_OFFLINEDECLINED:
            {
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(2,0,"OFFLINE DECLINED");
            else
                Os__GB2312_display(2,0,"脱机拒绝");
            }
            break;
        default:
            {
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(2,2,"UNKNOWN");
            else
                Os__GB2312_display(2,2,"未知");
            }
            break;
        }
        if(EMVTransInfo.bECTrans)
        {
            {
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(0,0,(PUCHAR)"BALANCE:");
            else
                Os__GB2312_display(0,0,(PUCHAR)"余额:");
            }
            memset(aucBuff,0x00,sizeof(aucBuff));
            UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),ECTransInfo.uiNBalance,2);
            Os__GB2312_display(1,0,aucBuff);

        }
        UTIL_GetKey(3);

    }

    return ucResult;
#if 0
    Uart_Printf("\n EMV return %02x",ucResult);
    Uart_Printf("\n TVR: %02X %02X %02X %02X %02X",EMVTransInfo.EMVTVR[0],EMVTransInfo.EMVTVR[1],
                EMVTransInfo.EMVTVR[2],EMVTransInfo.EMVTVR[3],EMVTransInfo.EMVTVR[4]);
    Uart_Printf("\n TSI: %02X %02X ",EMVTransInfo.EMVTSI[0],EMVTransInfo.EMVTSI[1]);

    Uart_Printf("\n CVM: %02X %02X %02X",EMVTransInfo.CVMResult.ucCVMCode,EMVTransInfo.CVMResult.ucConditionCode,EMVTransInfo.CVMResult.ucCVMResult);
#endif
}


UCHAR	FILE_ReadRecordNums(int iHandle,UINT* puiRecordNum)
{
    UINT	uiOutLen;
    UCHAR	ucResult;
    int		iFileResult;

    ucResult=SALEERR_SUCCESS;

    if(iHandle>=0)
    {
        iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);

        if(!iFileResult)
        {
            uiOutLen=sizeof(UINT);
            iFileResult=OSAPP_FileRead(iHandle,(UCHAR*)puiRecordNum,uiOutLen);
            if(iFileResult!=uiOutLen) ucResult=SALEERR_READFILE;

        }
        else ucResult=SALEERR_SEEKFILE;


    }
    else ucResult=SALEERR_OPENFILE;

    return ucResult;
}

UCHAR	FILE_ReadRecordNumByFileName(UCHAR* pucFileName,UINT *puiRecordNum)
{
    int		iHandle;
    UCHAR	ucResult;

    ucResult=SALEERR_SUCCESS;

    iHandle=OSAPP_OpenFile((char*)pucFileName,O_READ);
    if(iHandle>=0)
    {
        ucResult=FILE_ReadRecordNums(iHandle,puiRecordNum);
        OSAPP_FileClose(iHandle);
    }
    else ucResult=SALEERR_OPENFILE;
    return ucResult;
}


unsigned char OnEve_power_on(void)
{
    unsigned char ucResult;
    USHORT	 uiRecNum,uiI;

    ucResult  = EMVERROR_SUCCESS;

    if( ucResult == EMVERROR_SUCCESS )
        ucResult = OnEve_CheckExistFile((PUCHAR)CONSTPARAMFILE,sizeof(EMVCONSTPARAM),(PUCHAR)&ConstParam);
    if( ucResult == EMVERROR_SUCCESS )
        ucResult = OnEve_CheckExistFile((PUCHAR)EMVCONFIGFILE,sizeof(TERMCONFIG),(PUCHAR)&EMVConfig);


    if(!ucResult)
    {
        if( ucResult == EMVERROR_SUCCESS)
        ucResult= OnEve_CheckExistFile((PUCHAR)CAPKFILE,0,NULL);
        if( ucResult == EMVERROR_SUCCESS)
            ucResult= OnEve_CheckExistFile((PUCHAR)TERMSUPPORTAPPFILE,0,NULL);

        if( ucResult == EMVERROR_SUCCESS)
            ucResult= OnEve_CheckExistFile((PUCHAR)TRANSLOG,0,NULL);
        if( ucResult == EMVERROR_SUCCESS)
            ucResult= OnEve_CheckExistFile((PUCHAR)EXCEPTFILE,0,NULL);
        if( ucResult == EMVERROR_SUCCESS)
            ucResult= OnEve_CheckExistFile((PUCHAR)BATCHRECORD,0,NULL);
        if( ucResult == EMVERROR_SUCCESS)
            ucResult= OnEve_CheckExistFile((PUCHAR)IPKREVOKEFILE,0,NULL);

        ucResult =FILE_ReadRecordNumByFileName(CAPKFILE, &ucTermCAPKNum);
        if(!ucResult)
            for(uiI =0;!ucResult &&uiI< ucTermCAPKNum && uiI < MAXTERMCAPKNUMS;uiI++)
                ucResult = FILE_ReadRecordByFileName(CAPKFILE, uiI+1, &TermCAPK[uiI], sizeof(CAPK));
        if(!ucResult)
        {
            EMVConfig.ucCAPKNums = ucTermCAPKNum;
            EMVConfig.pTermSupportCAPK =TermCAPK;

        }

//		if(!ucResult)
        {
            ucResult =FILE_ReadRecordNumByFileName(TERMSUPPORTAPPFILE, &ucTermAIDNum);
            if(!ucResult)
                for(uiI =0;!ucResult &&uiI< ucTermAIDNum && uiI < MAXTERMAIDNUMS;uiI++)
                    ucResult = FILE_ReadRecordByFileName(TERMSUPPORTAPPFILE, uiI+1, &TermAID[uiI], sizeof(TERMSUPPORTAPP));
            if(!ucResult)
            {
                EMVConfig.ucCAPKNums = ucTermAIDNum;
                EMVConfig.pTermSupportApp=TermAID;

            }
        }

//		if(!ucResult)
        {
            ucResult =FILE_ReadRecordNumByFileName(EXCEPTFILE, &ucExceptFileNum);
            if(!ucResult)
                for(uiI =0;!ucResult &&uiI< ucExceptFileNum && uiI <MAXEXCEPTFILENUMS;uiI++)
                    ucResult = FILE_ReadRecordByFileName(EXCEPTFILE, uiI+1, &ExceptFile[uiI], sizeof(EXCEPTPAN));
            if(!ucResult)
            {
                EMVConfig.ucExceptFileNum= ucExceptFileNum;
                EMVConfig.pExceptPAN=ExceptFile;

            }
        }

//		if(!ucResult)
        {
            ucResult =FILE_ReadRecordNumByFileName(IPKREVOKEFILE, &ucIPKRevokeNum);
            if(!ucResult)
                for(uiI =0;!ucResult &&uiI< ucIPKRevokeNum && uiI <MAXIPKREVOKENUMS ;uiI++)
                    ucResult = FILE_ReadRecordByFileName(IPKREVOKEFILE, uiI+1, &IPKRevoke[uiI], sizeof(IPKREVOKE));
            if(!ucResult)
            {
                EMVConfig.ucIPKRevokeNum= ucIPKRevokeNum;
                EMVConfig.pIPKRevoke=IPKRevoke;

            }
        }


    }

    if(strlen((char*)ConstParam.aucTerminalID)==0||strlen((char*)ConstParam.aucMerchantID)==0)
        TERMSETTING_LoadDefaultSetting();

    EMVConfig.pfnEMVICC = EMVICC;
    EMVConfig.pfnOfflinePIN = UTIL_OfflinePIN;
    EMVConfig.pfnOnlinePIN = UTIL_OnlinePIN;
    EMVConfig.pfnEMVInformMsg =UTIL_EMVInformMsg;

    return ucResult;


}


UCHAR	OnEve_CheckExistFile(PUCHAR pszFileName,USHORT uiFileLen,PUCHAR pLoadAddr)
{
    int iFileSize;
    int iFileHandel;
    int iFileErrNo;


    if((iFileSize  =  OSAPP_FileSize((char*)pszFileName))<0)
    {
        iFileHandel = OSAPP_OpenFile((char*)pszFileName,O_CREATE);

        if(iFileHandel>=0)
        {
            OSAPP_FileClose(iFileHandel);
            memset(pLoadAddr,0x00,uiFileLen);
        }
    }else
    {
        if(iFileSize==uiFileLen)
        {
            iFileHandel = OSAPP_OpenFile((char*)pszFileName,O_READ);
            iFileErrNo = OSAPP_FileSeek(iFileHandel,0,SEEK_SET);
            if(!iFileErrNo&&pLoadAddr)
                iFileErrNo = OSAPP_FileRead(iFileHandel,(unsigned char *)pLoadAddr,iFileSize);

        }else
        if(uiFileLen)
        {
            iFileHandel = OSAPP_OpenFile((char *)pszFileName,O_READ);
            OSAPP_FileTrunc(iFileHandel,0);
            memset(pLoadAddr,0x00,uiFileLen);
        }
        OSAPP_FileClose(iFileHandel);
    }
    //return(OSAPP_FileGetLastError());
    return EMVERROR_SUCCESS;
}

#if 0
unsigned char UTIL_InputAmount(unsigned char ucLine,
                unsigned long *pulAmount,
                unsigned long ulMinAmount, unsigned long ulMaxAmount)
{
    unsigned char	aucDisp[MAXDISPCHAR+1];		/* Buffer for Display */
    unsigned char	aucInput[MAXAMOUNTLEN+1];	/* Buffer for Key Input */
    unsigned char	ucKey;
    unsigned char	ucRedraw;
    unsigned char	ucCount;
    unsigned char	ucI;
    unsigned char	ucJ;
    DRV_OUT *pdKey;
    unsigned int	uiTimeout;
    unsigned long	ulAmount;

    memset(&aucDisp[0],0,sizeof(aucDisp));
    memset(&aucDisp[0],' ',MAXDISPCHAR);
    aucDisp[MAXDISPCHAR-1] = '0';
    aucDisp[MAXDISPCHAR-2] = '0';
    aucDisp[MAXDISPCHAR-3] = '.';
    aucDisp[MAXDISPCHAR-4] = '0';
    memset(&aucInput[0],0,sizeof(aucInput));
    ucRedraw = TRUE;
    uiTimeout = 50*ONESECOND;
    ucCount = 0;

    Os__timer_start(&uiTimeout);

    do
    {
        if( ucRedraw == TRUE)
        {
            Os__clr_display(ucLine);
            Os__display(ucLine,0,&aucDisp[0]);
            ucRedraw = FALSE;
        }

        pdKey = Os__get_key();

        do{
        }while(  (pdKey->gen_status==DRV_RUNNING)
               &&(uiTimeout !=0)
               );

        if (uiTimeout == 0)
        {
            Os__abort_drv(drv_mmi);
            Os__xdelay(10);
            return(EMVERROR_CANCEL);
        }else
        {
            ucKey = pdKey->xxdata[1];
        }
        Os__timer_stop(&uiTimeout);

        switch(ucKey)
        {
        case KEY_CLEAR:
            if( ucCount )
            {
                ucCount = 0;
                memset(&aucDisp[0],0,sizeof(aucDisp));
                memset(&aucDisp[0],' ',MAXDISPCHAR);
                aucDisp[MAXDISPCHAR-1] = '0';
                aucDisp[MAXDISPCHAR-2] = '0';
                aucDisp[MAXDISPCHAR-3] = '.';
                aucDisp[MAXDISPCHAR-4] = '0';
                ucRedraw = TRUE;
            }else
            {
                return(EMVERROR_CANCEL);
            }
            break;
        case KEY_BCKSP:
            if( ucCount )
            {
                ucCount --;
                aucInput[ucCount] = 0;
                ucRedraw = TRUE;
            }
            break;
        case KEY_ENTER:
            ulAmount = asc_long(aucInput,ucCount);
            if( ulAmount < ulMinAmount )
            {
                Os__beep();
            }else
            {
                if( ulAmount > ulMaxAmount )
                {
                    Os__beep();

                }else
                {
                    *pulAmount = ulAmount;
                    return(EMVERROR_SUCCESS);
                }
            }
            break;
        default :
            if(  (ucKey <= '9')
               &&(ucKey >= '0')
              )
            {
                if(ucCount==0&&ucKey=='0')
                    break;
                if( ucCount < MAXAMOUNTLEN)
                {
                    aucInput[ucCount] = ucKey;
                    ucCount ++;
                    ucRedraw = TRUE;
                }
            }
            break;
        }
        /* Copy data from Input buffer to Display buffer */
        memset(&aucDisp[0],0,sizeof(aucDisp));
        memset(&aucDisp[0],' ',MAXDISPCHAR);
        aucDisp[MAXDISPCHAR-1] = '0';
        aucDisp[MAXDISPCHAR-2] = '0';
        aucDisp[MAXDISPCHAR-3] = '.';
        aucDisp[MAXDISPCHAR-4] = '0';
        if( ucCount )
        {
            for(ucI=0,ucJ=0;ucI<ucCount;ucI++,ucJ++)
            {
                if( ucJ == 2)
                {
                    ucJ ++;
                }
                aucDisp[MAXDISPCHAR-ucJ-1] = aucInput[ucCount-ucI-1];
            }
        }
    }while(1);
}

#endif

UCHAR UTIL_WriteConstParamFile(EMVCONSTPARAM * pConstParam)
{
    int	iHandle,iFileLen;
    unsigned char ucResult,iFileResult;

    iHandle = OSAPP_OpenFile((char *)CONSTPARAMFILE,O_WRITE);
    if(iHandle>=0)
    {
        iFileResult=OSAPP_FileSeek(iHandle,0,SEEK_SET);

        if(!iFileResult) iFileLen=OSAPP_FileWrite(iHandle,(PUCHAR)pConstParam,sizeof(EMVCONSTPARAM));
    }
    OSAPP_FileClose(iHandle);
    ucResult = OSAPP_FileGetLastError();
    if(ucResult)
        ucResult =EMVERROR_SAVEFILE;
    return ucResult;
}

UCHAR	MSG_ProcessIccFailBack(void)
{

    UCHAR ComPackSend[1024],ComPackRecv[300],ucResult;

    USHORT ComPackSendLen,ComPackRecvLen,VarReadLen,uiTagBuffLen,uiLen,uiBuffsize;
    UCHAR	aucTotalAmount[6],aucOtherAmount[6],aucBatchNumber[6],ucEntryMode,aucTracerNumber[6];
    UCHAR	aucTrack2Data[40],ucTrack2Len;
    UINT	uiTotalAmount;
    int		iHandle;
    long	FleSize;

    ucEntryMode = 0x92;

    ComPackRecvLen = sizeof(ComPackRecv);

    uiBuffsize = sizeof(ComPackSend);
    memset(ComPackSend,0,1024);
    ComPackSendLen=2;
    if(!ConstParam.bBatchCapture)
        ComPackSend[ComPackSendLen] = Msg_EMV_FinaRQ;
    else
        ComPackSend[ComPackSendLen] = Msg_EMV_AuthRQ;
    ComPackSendLen ++;

    EMVTAG_INIT((DATAPROPERTY *)TermDataProperty,TRUE);
    EMVTAG_SetTagValue((PUCHAR)"\x9F\x1C",ConstParam.aucTerminalID,TERMINALIDLEN);
    EMVTAG_SetTagValue((PUCHAR)"\x9F\x16",ConstParam.aucMerchantID,MERCHANTIDLEN);
    if(((EMVConfig.ucTermType&0xF0)==0x10)||((EMVConfig.ucTermType&0xF0)==0x20))
        EMVTAG_SetTagValue((PUCHAR)"\x9F\x35",&EMVConfig.ucTermType,1);
    EMVTAG_SetTagValue((PUCHAR)"\x9C",(PUCHAR)&TransReqInfo.enTransType,1);
    EMVTAG_SetTagValue((PUCHAR)"\x5F\x2A",EMVConfig.aucTransCurrencyCode,TRANSCURRENCYCODELEN);
    ucTrack2Len =strlen((char*)MagTrack2);
    CONV_AscBcd(aucTrack2Data,ucTrack2Len/2,MagTrack2,ucTrack2Len);
    EMVTAG_SetTagValue((PUCHAR)"\x57",aucTrack2Data,ucTrack2Len/2);	 //Track 2 Equivalent Data

    CONV_LongBcd(aucTotalAmount,sizeof(aucTotalAmount),&TransReqInfo.uiAmount);
    EMVTAG_SetTagValue((PUCHAR)"\x9F\x02",aucTotalAmount,sizeof(aucTotalAmount));
    if(TransReqInfo.enTransType ==CASHBACK)
    {
        CONV_LongBcd(aucOtherAmount,sizeof(aucOtherAmount),&TransReqInfo.uiOtherAmount);
        EMVTAG_SetTagValue((PUCHAR)"\x9F\x03",aucOtherAmount,sizeof(aucOtherAmount));
    }
    EMVTAG_SetTagValue((PUCHAR)"\x9F\x39",&ucEntryMode,1);
    CONV_LongBcd(aucBatchNumber,sizeof(aucBatchNumber),&ConstParam.uiBatchNumber);
    EMVTAG_SetTagValue((PUCHAR)"\xDF\x01",aucBatchNumber,sizeof(aucBatchNumber));

    CONV_LongHex(aucTracerNumber,sizeof(UINT),&TransReqInfo.uiTraceNumber);							//Trans sequence Counter
    EMVTAG_SetTagValue((PUCHAR)"\x9F\x41",aucTracerNumber,sizeof(UINT));


    VarReadLen =uiBuffsize -ComPackSendLen;
    ucResult = EMVTAG_Pack(&ComPackSend[ComPackSendLen],&VarReadLen);
    if(!ucResult)
    {
        ComPackSendLen += VarReadLen;
//		if(!ConstParam.bBatchCapture)
//		{
            ucResult = MSG_OpenCom();
            if(!ucResult)
                ucResult = MSG_ComSendData(&ComPackSend[2],ComPackSendLen-2);
            if(!ucResult)
            {
                ComPackRecvLen =sizeof(ComPackRecv);
                ucResult =MSG_ComRecvData(ComPackRecv,&ComPackRecvLen);
            }
            /*
            if(!ucResult)
                if(ComPackRecv[0] !=Msg_EMV_Reconci)
                    ucResult = EMVERROR_RECEIVEDATA;
            */
            if(!ucResult)
                ucResult = APPCRYPT_OnlineRespDataProcess(NULL,&ComPackRecv[1],ComPackRecvLen-1);
            MSG_ComClose();
//		}
#if 0
        else
        {
            CONV_LongHex(ComPackSend,sizeof(USHORT),&ComPackSendLen);

            iHandle = OSAPP_OpenFile(BATCHRECORD,O_CREATE|O_WRITE);
            if(iHandle>=0)
            {


                FleSize =OSAPP_FileSize(BATCHRECORD);
                ucResult =OSAPP_FileSeek(iHandle,FleSize,SEEK_SET);

                if(!ucResult)
                    if(ComPackSendLen != OSAPP_FileWrite(iHandle,ComPackSend,ComPackSendLen))
                        ucResult = EMVERROR_SAVEFILE;
                OSAPP_FileClose(iHandle);
            }
        }
#endif
    }

    return ucResult;

}

