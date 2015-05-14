
#include	<test/MasApp.h>
#include	<test/Global.h>
#include 	<test/Util.h>
#include	<test/Msg.h>
#include <smart.h>

#include "new_drv.h"
#include "tools.h"



#define HALIREXTPRNADDR				0x70
#define EXTPRINTCMD_FEED			0x17
#define EXTPRINTCMD_REVERSE			0x18
#define EXTPRINTCMD_PRINTGRAPH		0x39
#define EXTPRINTCMD_PRINTGB2312		0x3A
#define EXTPRINTCMD_PRINTASCII		0x3C
#define EXTPRINT_MAX_WAITIME		4000



extern const DATAPROPERTY	ICCardProperty[];
void ProcessMag(BOOL bFallBack,unsigned char* bufMag);
UCHAR SelectAccountType(void);
BOOL	TransCapture(TRANSRESULT	enTransResult);
void	PrintTransTicket(void);
UCHAR	ReadICTransLog(void);
UCHAR	ReadICBalance(void);
UCHAR 	MagTrack2[40];

unsigned char Os__xget_key11()
{
    NEW_DRV_TYPE new_drv;
    unsigned char ucKey;
    unsigned int ret = Os_Wait_Event(KEY_DRV,&new_drv,0);
    if(new_drv.drv_type == KEY_DRV){
        ucKey = new_drv.drv_data.xxdata[1];
        return ucKey;
    }
}

void OSMMI_DisplayASCII11 (unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char * text)
{
    if(ucFont==0x31)
        memcpy(myString[ucRow*2+1],text,strlen(text));
    else
        memcpy(myString[ucRow],text,strlen(text));

}
unsigned char Os__GB2312_display_pp (unsigned char line, unsigned char   Column, unsigned char * text)
{
    memcpy(myString[line*2+1],text,strlen(text));
}

void OSMMI_GB2312Display11(unsigned char ucFont,unsigned char ucRow, unsigned char ucCol, unsigned char *text)
{
    if(ucFont==0x31)
        memcpy(myString[ucRow*2+1],text,strlen(text));
    else
        memcpy(myString[ucRow],text,strlen(text));
}

void Os__GB2312_display11(unsigned char line, unsigned char Column, unsigned char *text)
{
    memcpy(myString[line*2+1],text,strlen((char*)text));
    printf("myString[line*2+1]===%s\n",myString[line*2+1]);
}

 void Os__display11(uchar line,uchar col,uchar *text)
 {
     memcpy(myString[line*2+1],text,strlen(text));
 }

 void Os__display_pp11(uchar line,uchar col,uchar *text)
 {
     memcpy(myString[line*2+1],text,strlen(text));

 }

void Os__clr_display_pp11(uchar line)
{
  memset(myString[line*2+1],0x00,sizeof(myString[line*2+1]));
}

 void Os__clr_display11( unsigned char line )
 {
     if(line==255)
         memset(myString,0x00,sizeof(myString));
     else
         memset(myString[line*2+1],0x00,sizeof(myString[line*2+1]));
 }

int emitSignal()
{
    sem_post(&binSem1);
    return(sem_wait(&binSem2)) ;
}


UCHAR	EMVICC(EMVICCIN *pEMVICCIn ,EMVICCOUT *pEMVICCOut)
{
	unsigned char ucResult;
	USHORT	uiI;

	
	Uart_Printf("\nSend ICC Data:\n");
	Uart_Printf("%02X %02X %02X %02X ",pEMVICCIn->ucCla,pEMVICCIn->ucIns,pEMVICCIn->ucP1,pEMVICCIn->ucP2);
	for(uiI=0;uiI<pEMVICCIn->uiLc;uiI++)
		Uart_Printf("%02X ",*(pEMVICCIn->aucDataIn+uiI));
	if(pEMVICCIn->uiLe) Uart_Printf("00");

	ucResult=IccIsoCommand(0,pEMVICCIn,pEMVICCOut);

	if(!ucResult)
	{
		Uart_Printf("\nRecv ICC Data:\n");
		for(uiI=0;uiI<pEMVICCOut->uiLenOut;uiI++)
			Uart_Printf("%02X ",pEMVICCOut->aucDataOut[uiI]);
		Uart_Printf("\nSW: %02X %02X",pEMVICCOut->ucSWA,pEMVICCOut->ucSWB);
	}

	return ucResult;
	
}

unsigned char  MASAPP_Event(void)
{
	unsigned char ucKey;
	unsigned char ucResult;

    unsigned char ucICCardType;

	DRV_OUT *pxMag;
	DRV_OUT *pxIcc;
	DRV_OUT *pxKey;

	TRANSTYPE	enTransType;
	unsigned long	uiAmount,uiOtherAmount;
	UCHAR	ucLen1,ucLen2,aucMagBuff[200],aucDateTime[10],aucResetBuf[50],ucResetBufLen,ucAuthRespCode;
	BOOL	bFallBack,bNextPage;
	EMVTRANSTAGDATA *  pRespCode;
	unsigned char ucI,aucBuff[30];
	
	OnEve_power_on();
	bFallBack = FALSE;
    while(ThreadCloseFlag)
    {
        NEW_DRV_TYPE new_drv;

        memset(&new_drv,0,sizeof(NEW_DRV_TYPE));

        Os__clr_display11(255);

        printf("ConstParam.bPBOC20===%d\n",ConstParam.bPBOC20);

        if(ConstParam.bPBOC20==false)
        {
            Os__GB2312_display11(1,0,"EMV Level 2 TEST ");
            Os__GB2312_display11(2,0,"Insert/Swap Card");
        }
        else
        {
            Os__GB2312_display11(1,0,"   PBOC20 测试 ");
            Os__GB2312_display11(2,0,"请插入IC卡/刷卡");
        }


        emitSignal();

        printf("222=%d\n",ConstParam.bAutoSupport);

        unsigned int ret = Os_Wait_Event(ICC_DRV|MAG_DRV|KEY_DRV , &new_drv, 15000);

        int c=0;

        printf("retry %d\n",c++);
        if(new_drv.drv_type == ICC_DRV)
        {
            while(ThreadCloseFlag)
            {
                pxIcc = &new_drv.drv_data;
                Os__abort_drv(drv_mag);
                Os__abort_drv(drv_mmi);
                printf("1111111111\n");

                ucICCardType =pxIcc->xxdata[1];
                ucResetBufLen=sizeof(aucResetBuf);
                printf("warm reset\n");
                if(ucICCardType==0x39)
                {
                    ucResult = SMART_Reset(0,aucResetBuf,(unsigned short*)&ucResetBufLen);
                    printf("~~~~~~~SMART_Reset=%02x\n",ucResult);

                }
                //   ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);
                printf("warm reset finish\n");
                printf("ucICCardType====%02x\n",ucICCardType);
                if(ucICCardType!=ICC_ASY)
                {
                    Os__clr_display11(255);

                    if(ConstParam.bPBOC20==FALSE)
                        Os__GB2312_display11(2,2,"Swap Card");
                    else{
                        Os__GB2312_display11(2,2,"请刷磁条卡");
                    }

                    emitSignal();

                    Os__xdelay(50);

                    bFallBack = TRUE;
                    break;
                }

                switch(ConstParam.ucTransType)
                {
                case '1':
                    enTransType =CASH;
                    break;
                case '2':
                    enTransType =GOODS;
                    break;
                case '3':
                    enTransType =SERVICES;
                    break;
                case '4':
                    enTransType =CASHBACK;
                    break;
                default:
                    enTransType =GOODS;
                    break;

                }

                printf("wait input amount [%d]\n",ConstParam.bAutoSupport);
                if(!ConstParam.bAutoSupport)
                {
                    Os__clr_display11(255);

                    if(ConstParam.bPBOC20==false)
                        Os__GB2312_display11(2,0,(PUCHAR)"Trans Amount:");
                    else
                        Os__GB2312_display11(2,0,(PUCHAR)"交易金额:");


                    emitSignal();

                    uiAmount=0;


                    if(!(ucResult=UTIL_InputAmount(3,&uiAmount,1,99999999)))
                    {
                        printf("enTransType====%d\n",enTransType);
                        if(enTransType ==CASHBACK)
                        {
                            Os__clr_display11(255);

                            if(ConstParam.bPBOC20==FALSE)
                                Os__GB2312_display11(2,0,(PUCHAR)"Cashback Amount:");
                            else
                                Os__GB2312_display11(2,0,(PUCHAR)"返现金:");

                            emitSignal();

                            ucResult =UTIL_InputAmount(3,&uiOtherAmount,1,99999999);
                        }
                        else
                            uiOtherAmount =0;

                        if(!ucResult)
                        {

                            printf("*******************************1-->%02x\n",enTransType);
                            printf("*******************************2-->%d\n",uiAmount);


                            ucResult = EMVTransProcess(enTransType,uiAmount,uiOtherAmount);
                            if(ucResult ==EMVERROR_FALLBACK)
                            {
                                bFallBack = TRUE;

                            }
                            else
                                bFallBack = FALSE;

                        }
                    }

                }
                else
                {
                    printf("1-->%02x\n",ConstParam.ucTransType);
                    printf("2-->%d\n",ConstParam.uiTransAmount);
                    printf("3-->%d\n",ConstParam.uiOtherAmount);

                    ucResult =EMVTransProcess(ConstParam.ucTransType,ConstParam.uiTransAmount,ConstParam.uiTransAmount);
                    if(ucResult ==EMVERROR_FALLBACK)
                    {
                        bFallBack = TRUE;
                    }
                    else
                        bFallBack = FALSE;
                }

                printf("finish input amount\n");


                if(!ConstParam.bAutoSupport)
                {
                    printf("\n@@@@@@ Os__ICC_detect(0)==%d\n",Os__ICC_detect(0));
                    if(Os__ICC_detect(0)==0){//检测是否有卡
                        Os__clr_display11(255);
                        if(ConstParam.bPBOC20==FALSE)
                            Os__GB2312_display11(2,2,"Remove pls");
                        else
                            Os__GB2312_display11(2,2,"请移卡");
                        emitSignal();

                        Os__ICC_remove();
                        UTIL_GetKey(2);

                    }
                    break;
                }
                else
                {

                    if(UTIL_GetKey(ConstParam.uiDelayTime)==KEY_CLEAR) break;


                    UTIL_GetKey(ConstParam.uiDelayTime);
                    Os__clr_display11(255);

                    if(ConstParam.bPBOC20==FALSE)
                        Os__GB2312_display11(2,2,"Wait pls");
                    else
                        Os__GB2312_display11(2,2,"请稍候");

                    emitSignal();


                    UTIL_GetKey(2);
                }



            }

        }
        else if(new_drv.drv_type == MAG_DRV)
        {
            pxMag = &new_drv.drv_data;
            Os__abort_drv(drv_icc);
            Os__abort_drv(drv_mmi);
            printf("1111IN MAG DRV*********\n");

            ucLen1 = pxMag->xxdata[0];
            printf("IN MAG DRV*********\n");
            ucLen2 = pxMag->xxdata[ucLen1+2];

            if(ucLen1&&ucLen1<38&& ucLen2<105)
            {
                memcpy(aucMagBuff,pxMag,ucLen1+ucLen2+7);
                ProcessMag(bFallBack,aucMagBuff);
                if(bFallBack)
                {
                    //					if(!TransReqInfo.uiAmount)
                    {
                        Os__clr_display11(255);
                        if(ConstParam.bPBOC20==FALSE)
                            Os__GB2312_display11(2,0,(PUCHAR)"Trans Amount:");

                        else
                            Os__GB2312_display11(2,0,(PUCHAR)"交易金额:");


                        emitSignal();

                        ucResult=UTIL_InputAmount(3,&uiAmount,1,99999999999);
                    }
                    TransReqInfo.enTransType = enTransType;
                    TransReqInfo.uiAmount = uiAmount;

                    if(enTransType == CASHBACK)
                    {
                        TransReqInfo.uiAmount += uiOtherAmount;
                        TransReqInfo.uiOtherAmount = uiOtherAmount;

                    }

                    Os__read_date(aucDateTime);
                    memcpy(TransReqInfo.aucTransDate,"20",2);
                    memcpy(&TransReqInfo.aucTransDate[2],&aucDateTime[4],2);
                    memcpy(&TransReqInfo.aucTransDate[4],&aucDateTime[2],2);
                    memcpy(&TransReqInfo.aucTransDate[6],&aucDateTime[0],2);

                    Os__read_time_sec(TransReqInfo.aucTransTime);////

                    TransReqInfo.uiTraceNumber = ConstParam.uiTraceNumber;
                    ConstParam.uiTraceNumber++;
                    UTIL_WriteConstParamFile(&ConstParam);

                    ucResult =MSG_ProcessIccFailBack();
                    if(!ucResult)
                    {
                        pRespCode =EMVTRANSTAG_SearchTag(ALLPHASETAG,(PUCHAR)"\x8A");

                        if(pRespCode&&pRespCode->uiLen==2)
                        {
                            ucAuthRespCode = CONV_AscLong(pRespCode->pTagValue,pRespCode->uiLen);
                            Os__clr_display11(255);
                            if(ucAuthRespCode ==0)
                            {
                                ConstParam.uiTotalAmount +=TransReqInfo.uiAmount;
                                ConstParam.uiTotalNums++;
                                ucResult = UTIL_WriteConstParamFile(&ConstParam);

                                if(ConstParam.bPBOC20==FALSE)
                                    Os__GB2312_display11(3,0,(PUCHAR)"ONLINE APPROVED");
                                else
                                    Os__GB2312_display11(3,0,(PUCHAR)"联机批准");

                            }
                            else
                            {
                                if(ConstParam.bPBOC20==FALSE)
                                    Os__GB2312_display11(3,0,(PUCHAR)"ONLINE DECLINED");
                                else
                                    Os__GB2312_display11(3,0,(PUCHAR)"联机拒绝");


                            }
                            emitSignal();

                            UTIL_GetKey(3);
                        }
                    }

                    bFallBack = FALSE;
                }

            }
        }
        else if(new_drv.drv_type == KEY_DRV)
        {

            pxKey = &new_drv.drv_data;
            bNextPage =FALSE;
            Os__abort_drv(drv_mag);
            Os__abort_drv(drv_icc);

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
                    Os__clr_display11(255);
                    if(!bNextPage)
                    {
                        if(ConstParam.bPBOC20==FALSE)
                        {
                            Os__GB2312_display11(0,0,"1.Batch Up");
                            Os__GB2312_display11(1,0,"2.Param Update");
                            Os__GB2312_display11(2,0,"3.Trans TVR");
                        }
                        else
                        {
                            Os__GB2312_display11(0,0,"1.批上送");
                            Os__GB2312_display11(1,0,"2.参数更新");
                            Os__GB2312_display11(2,0,"3.终端验证结果");
                            Os__GB2312_display11(3,0,"4.交易日志");
                        }
                        emitSignal();

                    }
                    else
                    {
                        if(ConstParam.bPBOC20==FALSE)
                        {
                            Os__GB2312_display11(0,0,"1.Batch Up");
                            Os__GB2312_display11(1,0,"2.Param Update");
                            Os__GB2312_display11(2,0,"3.Trans TVR");

                        }
                        else
                        {
                            if(ConstParam.bECSupport)
                                Os__GB2312_display11(0,0,"5.卡片余额");

                            else
                            {

                                Os__GB2312_display11(0,0,"1.批上送");
                                Os__GB2312_display11(1,0,"2.参数更新");
                                Os__GB2312_display11(2,0,"3.终端验证结果");
                                Os__GB2312_display11(3,0,"4.交易日志");

                            }

                        }
                        emitSignal();
                    }

                    //                    Os_Wait_Event(KEY_DRV,&new_drv,5000);
                    //                    if(new_drv.drv_type ==KEY_DRV)
                    //                        ucKey = &new_drv.drv_data.xxdata[1];
                    ucKey =Os__xget_key11();
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
                        if(ConstParam.bPBOC20||ConstParam.bECSupport)
                            ReadICTransLog();//OS_GET_KEY
                        break;
                    case '5':
                        if(ConstParam.bECSupport)
                            ReadICBalance();//OS_GET_KEY
                        break;
                    case KEY_F3:
                        bNextPage =bNextPage?FALSE:TRUE;
                        break;
                    default:
                        break;
                    }
                }while(ucKey != KEY_CLEAR);
                break;
#ifdef _PS300
            case KEY_F4:
                Os__clr_display11(255);


                Os__GB2312_display11(1,0,(PUCHAR)"Turn Off Confirm");
                Os__GB2312_display11(2,1,(PUCHAR)"1.OK  2.Cancel");
                if(UTIL_GetKey(3)=='1')
                    OSMOBILE_PowerOff();

                break;
#endif
            case KEY_F4:
                ucI=0;
                while(ucI<EMVTransInfo.ucScriptResultNum)
                {
                    unsigned char ucJ;
                    Os__clr_display11(255);
                    for(ucJ=0;(ucJ<4)&&(ucI<EMVTransInfo.ucScriptResultNum);ucJ++,ucI++)
                    {
                        memset(aucBuff,0x00,sizeof(aucBuff));
                        CONV_HexAsc(aucBuff,&EMVTransInfo.ScriptResult[ucI],sizeof(SCRIPTRESULT)*2);
                        Os__GB2312_display11(ucJ,0,aucBuff);

                        ////Os__GB2312_display11(ucJ,0,aucBuff);


                    }
                    Os__xget_key11();
                }
                break;
            default:
                break;

            }

        }




    }

}

#define		MAXDISPROWS		8

UCHAR	CardHolderConfirmApp(PCANDIDATELIST pCandidateList,PUCHAR pucAppIndex)
{
	UCHAR		ucI,ucRow,ucCurPage,ucBeginIndex;
    UCHAR		aucBuff[30],ucKey,ucICTI;
	PCANDIDATEAPP	pCandidateApp; 
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
    unsigned char caNum = pCandidateList->CandidateNums;
    for(ucI=0;ucI<caNum;ucI++)
      {
         pCandidateApp =&pCandidateList->CandidateApp[ucI];

        printf("pCandidateApp->ucAppPreferNameLen = %d\n",pCandidateApp->ucAppPreferNameLen);
        printf("pCandidateApp->ucAppLabelLen = %d\n",pCandidateApp->ucAppLabelLen);
        printf("pCandidateApp->ucADFNameLen = %d\n",pCandidateApp->ucADFNameLen);
    }
    printf("############3\n");
	do
	{
		ucBeginIndex =ucCurPage*MAXDISPROWS;
        Os__clr_display11(255);
		for(ucI =0; ucI <MAXDISPROWS; ucI++)
		{
            printf("CandidateNums = %02x \n",pCandidateList->CandidateNums);
            if(ucI+ucBeginIndex >= caNum) break;

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

            printf("aucBuff = %s\n",aucBuff);


            OSMMI_DisplayASCII11(0x30,ucI,0,aucBuff);

          emitSignal();
			
		}

        ucKey = Os__xget_key11();
        printf("uckey==%02x \n",ucKey);
        if(ucKey ==KEY_ENTER)
        {
            *pucAppIndex =0;
            return EMVERROR_SUCCESS;
        }
        else if(ucKey == KEY_CLEAR){
            printf("key clear*******\n");
            return EMVERROR_CANCEL;
        }
        else if((ucKey>'0')&&(ucKey < caNum +'1'))
        {
            *pucAppIndex =ucKey -0x31;
            return EMVERROR_SUCCESS;
        }

        if(ucKey == KEY_F3 &&ucCurPage)
            ucCurPage --;
        if(ucKey == KEY_F4 && (ucCurPage+1)*MAXDISPROWS<caNum)
            ucCurPage++;

			
	}while(1);
	
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
	
    printf("in emv trans process**********uiAmount =%d,uiOtherAmount=%d\n",uiAmount,uiOtherAmount);
    Os__clr_display11(255);

    if(ConstParam.bPBOC20==false)
        Os__GB2312_display11(2,2,"Processing");

    else
        Os__GB2312_display11(2,2,"处理中");

    emitSignal();

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
    printf("1111111**********\n");

	EMVTAG_SetbDEBUGValue (ConstParam.bDEBUG);
	memset(&cp,0x00,sizeof(CONFIG_PARAM));
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff,cp.aucSerialNo,sizeof(cp.aucSerialNo));
	ucLen =strlen((char*)aucBuff);
     printf("2222222222**********\n");
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
    printf("before EMVDATA_TermInit\n");
	EMVDATA_TermInit(&EMVConfig);
     printf("before APPSELECT_CreateAppCandidate\n");
    ucResult = APPSELECT_CreateAppCandidate(&CandidateList);	
     printf("after APPSELECT_CreateAppCandidate\n");

    printf("555555555***ucResult ==%d*******\n",ucResult);

    unsigned char caNum = CandidateList.CandidateNums;
    PCANDIDATEAPP   pCandidateApp;
    unsigned char ucI;
    for(ucI=0;ucI<caNum;ucI++)
         pCandidateApp =&CandidateList.CandidateApp[ucI];



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
        printf("66666666**********\n");

		if(!ucResult)
		{
			ucResult = APPSELECT_FinalSelect(ucAppIndex,&CandidateList);
			if(ucResult == EMVERROR_RESELECT)
			{
                Os__clr_display11(255);

                if(ConstParam.bPBOC20==FALSE)
                    Os__GB2312_display11(2,2,(PUCHAR)"Try Again");

                else
                    Os__GB2312_display11(2,0,"再来一次");



            emitSignal();
                Os__xget_key11();//?
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
                printf("\n!!!!!!!!!!TransReqInfo.uiAmount==%d\n",TransReqInfo.uiAmount);
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
            Os__clr_display11(255);
            if(ConstParam.bPBOC20==false)
                Os__GB2312_display11(2,2,"Processing");

            else
                Os__GB2312_display11(2,2,"处理中");

            emitSignal();

			if(!ucResult) ucResult = INTIAPP_GPO();

			if(ucResult == EMVERROR_RESELECT)
			{
				ucResult = APPSELECT_GetAppCandidate(&CandidateList);
                Os__clr_display11(255);
                if(ConstParam.bPBOC20==FALSE)
                    Os__GB2312_display11(2,2,(PUCHAR)"Try Again");

                else
                    Os__GB2312_display11(2,0,"再来一次");


                emitSignal();

                Os__xget_key11();
				continue;
			}
			else break;
		}
	}

    printf("7777777**********\n");

	if(!ucResult) ucResult = READAPP_ReadData();
	if(!ucResult)
	{
		uiLogSumAmount = CalcLogTransAmount();
	}
	if(!ucResult) ucResult = DATAAUTH_Process();
	if(!ucResult) ucResult = PROCESSRESTRICT_Process();
    printf("88888888888**********\n");

	if(!ucResult)
	{
        Os__clr_display11(255);
		if(EMVTransInfo.bECTrans)
		{
			memset(aucBuff,0x00,sizeof(aucBuff));
			memcpy(aucBuff,ECTransInfo.aucECICode,ECISSUERCODELEN);


            Os__GB2312_display11(0,0,aucBuff);
            emitSignal();
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


            Os__GB2312_display11(1,0,aucBuff);
            emitSignal();
		}
		memset(aucBuff,0x00,sizeof(aucBuff));
		
		if(ConstParam.bPBOC20==FALSE)
			strcpy((char*)aucBuff,"Amt:");
		else
            strcpy((char*)aucBuff,"金额:");

		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),TransReqInfo.uiAmount,2);
#if 1

        Os__GB2312_display11(2,0,aucBuff);

        if(ConstParam.bPBOC20==false)
            Os__GB2312_display11(3,2,(PUCHAR)"Confirm pls");
        else
            Os__GB2312_display11(3,3,(PUCHAR)"确认");

        emitSignal();

		do
		{
          ucKey = Os__xget_key11();
			if(ucKey ==KEY_CLEAR)	return EMVERROR_CANCEL;
			if(ucKey ==KEY_ENTER)	break;
		}while(1);

         sleep(3);
         Os__clr_display11(255);
        if(ConstParam.bPBOC20==false){
            Os__GB2312_display11(2,2,"Processing");
        }
		else
            Os__GB2312_display11(2,2,"处理中");

         emitSignal();

#endif
	}
      Os__clr_display11(255);
	if(!ucResult) ucResult = EMVVERIFY_Process();
    printf("999999999999********ucResult ==%d **\n",ucResult);

    sleep(3);
	if(!ucResult) 
	{
		ucResult = TERMRISKMANAGE_Process(uiLogSumAmount);
        printf("********TERMRISKMANAGE_Process () ==%d **\n",ucResult);

		if(ConstParam.bShowRandNum)
		{
			memset(aucBuff,0x00,sizeof(aucBuff));
            Os__clr_display11(255);
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display11(1,2,"Random Num:");
            else
                Os__GB2312_display11(1,0,"随机数:");

            emitSignal();
            printf("bbbbbb********ucResult ==%d **\n",ucResult);


			CONV_CharAsc(aucBuff,2,&EMVTransInfo.ucRandNum);

            Os__GB2312_display11(2,6,aucBuff);

            emitSignal();


			UTIL_GetKey(2);
			
		}
	}

     sleep(5);
	if(!ucResult)
	{
        printf("ccc********ucResult ==%d **\n",ucResult);

        CryptType = TERMACTANALYSIS_Process(TRUE);
        printf("aaaaaaaaa********ucResult ==%d **\n",ucResult);
           sleep(5);
		ucResult = APPCRYPT_OfflineProcess(CryptType);

        printf("********APPCRYPT_OfflineProcess () ==%d **\n",ucResult);
    sleep(5);
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
    printf("aaaaaaaaaaaa**********\n");

	EMVTrans_Infor_Display();
	
	if(ucResult)
	{
        Os__clr_display11(255);

        if(ConstParam.bPBOC20==FALSE)
            Os__GB2312_display11(2,0,"TRANS TERMINATED");

        else
            Os__GB2312_display11(2,0,"交易终止");

        emitSignal();

		if(ucResult ==EMVERROR_FALLBACK)
		{
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display11(3,0,(PUCHAR)"USE MAG CARD");

            else
                Os__GB2312_display11(3,0,(PUCHAR)"请用磁条卡");

            emitSignal();

        }

		UTIL_GetKey(3);
	}
	else
	{
        Os__clr_display11(255);
		switch(EMVTransInfo.enTransResult)
		{
		case OFFLINEAPPROVED:
			{
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display11(2,0,"OFFLINE APPROVED");

            else
                Os__GB2312_display11(2,0,"脱机批准");

            emitSignal();

            }
            break;
		case OFFLINEDECLINED:
			{
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display11(2,0,"OFFLINE DECLINED");

            else
                Os__GB2312_display11(2,0,"脱机拒绝");


            emitSignal();

			}
			break;

		case ONLINEAPPROVED:
			{
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display11(2,0,"ONLINE APPROVED");

            else
                Os__GB2312_display11(2,0,"联机批准");


            emitSignal();

			}
			break;
		case ONLINEDECLINED:
			{
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display11(2,0,"ONLINE DECLINED");

            else
                Os__GB2312_display11(2,0,"联机拒绝");

            emitSignal();

			}
			break;	
		case UNABLEONLINE_OFFLINEAPPROVED:
			{
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display11(2,0,"OFFLINE APPROVED");
            else
                Os__GB2312_display11(2,0,"脱机批准");

            emitSignal();

			}
			break;
		case UNABLEONINE_OFFLINEDECLINED:
			{
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display11(2,0,"OFFLINE DECLINED");
            else
                Os__GB2312_display11(2,0,"脱机拒绝");

              emitSignal();
			}
			break;
		default:
			{
            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display11(2,2,"UNKNOWN");
            else
                Os__GB2312_display11(2,2,"未知");

            emitSignal();

         }
			break;
		}


		if(EMVTransInfo.bECTrans)
		{

            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display11(0,0,(PUCHAR)"BALANCE:");

            else
                Os__GB2312_display11(0,0,(PUCHAR)"余额:");


            emitSignal();

			memset(aucBuff,0x00,sizeof(aucBuff));
			UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),ECTransInfo.uiNBalance,2);

            Os__GB2312_display11(1,0,aucBuff);

            emitSignal();

		}
		UTIL_GetKey(3);
		
	}
    printf("bbbbbb**********\n");

	return ucResult;


}

UCHAR	CheckTransTVR(void)
{
	UCHAR ucI,aucVer[16],aucBuff[32],aucTempBuff[16];
	UCHAR aucCfgToSign[512],ucIndex,aucCfgHash[20];
	EMVTRANSINFO EMVTransInfo;
	
    Os__clr_display11(255);
    Os__GB2312_display11(0,0,"TVR value:");

    emitSignal();


	memset(aucBuff,0,sizeof(aucBuff));
	EMVDATA_TransInfo(&EMVTransInfo);
	for(ucI=0;ucI<5;ucI++)
	{
		sprintf(aucTempBuff,"%02X ",EMVTransInfo.EMVTVR[ucI]);
		strcat((char*)aucBuff,(char*)aucTempBuff);
	}
    Os__GB2312_display11(1,0,aucBuff);


	sprintf(aucBuff,"TSI: %02X %02X",EMVTransInfo.EMVTSI[0],EMVTransInfo.EMVTSI[1]);
    Os__GB2312_display11(2,0,aucBuff);



	memset(aucVer,0,16);
	EMVDATA_KernelVer(aucVer);
	sprintf(aucBuff,"%s",(char*)aucVer);

    Os__GB2312_display11(3,0,aucBuff);

    emitSignal();


    Os__xget_key11();

	ucIndex=0;
	memset(aucCfgToSign,0,sizeof(aucCfgToSign));
	
	memcpy(aucCfgToSign+ucIndex,EMVConfig.aucTermCapab,sizeof(EMVConfig.aucTermCapab));
	ucIndex+=sizeof(EMVConfig.aucTermCapab);

	memcpy(aucCfgToSign+ucIndex,EMVConfig.aucTermAddCapab,sizeof(EMVConfig.aucTermAddCapab));
	ucIndex+=sizeof(EMVConfig.aucTermAddCapab);

	memcpy(aucCfgToSign+ucIndex,&(EMVConfig.ucTermType),sizeof(EMVConfig.ucTermType));
	ucIndex+=sizeof(EMVConfig.ucTermType);


	memcpy(aucCfgToSign+ucIndex,&(ConstParam.bForceOnline),sizeof(ConstParam.bForceOnline));
	ucIndex+=sizeof(ConstParam.bForceOnline);

	memcpy(aucCfgToSign+ucIndex,&(EMVConfig.ucDefaultDOLLen),sizeof(EMVConfig.ucDefaultDOLLen));
	ucIndex+=sizeof(EMVConfig.ucDefaultDOLLen);

	memcpy(aucCfgToSign+ucIndex,EMVConfig.aucDefaultDOL,EMVConfig.ucDefaultDOLLen);
	ucIndex+=EMVConfig.ucDefaultDOLLen;

	memcpy(aucCfgToSign+ucIndex,&(EMVConfig.ucDefaultTDOLLen),sizeof(EMVConfig.ucDefaultTDOLLen));
	ucIndex+=sizeof(EMVConfig.ucDefaultTDOLLen);

	memcpy(aucCfgToSign+ucIndex,EMVConfig.aucDefaultTDOL,EMVConfig.ucDefaultTDOLLen);
	ucIndex+=EMVConfig.ucDefaultTDOLLen;

	
	SHA1_Compute(aucCfgToSign,ucIndex,aucCfgHash);
	

    Os__clr_display11(255);
    Os__GB2312_display11(0,0,"Cfg Check Sum:");

	memset(aucBuff,0,sizeof(aucBuff));
	for(ucI=0;ucI<4;ucI++)
	{
		sprintf(aucTempBuff,"%02X ",aucCfgHash[ucI]);
		strcat((char*)aucBuff,(char*)aucTempBuff);
	}
    Os__GB2312_display11(1,1,aucBuff);
    emitSignal();

    Os__xget_key11();

}


void	CompleteTrans(void)
{
    printf("IN COMPLETETRANS*****************\n");
	UCHAR	ucResult,ucI,aucBuff[40],ucScriptResult;
	SCRIPTRESULT	*pScriptResult;
	UCHAR ucDispBuff[20];
	
	ucResult =EMVERROR_SUCCESS;
	
	if(!ConstParam.bBatchCapture)
	{

		if(EMVTransInfo.bReversal)
			ucResult = MSG_ReversalRequest();
		
		if(TransCapture(EMVTransInfo.enTransResult))
			ucResult = MSG_TransConfirm();

		if(!TransCapture(EMVTransInfo.enTransResult))
		{
			if(EMVTransInfo.ucScriptResultNum||EMVTransInfo.bAdvice)
				ucResult=MSG_AdviceRequest();
		}
#if 0
		if(!ucResult&&EMVTransInfo.enTransResult ==ONLINEDECLINED)
		{
			if(!EMVTransInfo.bReversal&&EMVTransInfo.ucScriptResultNum)
				ucResult =MSG_AdviceRequest();
		}
		
		if(!ucResult&&!TransCapture(EMVTransInfo.enTransResult)&&EMVTransInfo.bAdvice)
			ucResult =MSG_AdviceRequest();
#endif
	}	
	else
	{
		/*
		if(EMVTransInfo.enTransResult ==ONLINEDECLINED)
		{
			if(!ucResult&&!EMVTransInfo.bReversal&&EMVTransInfo.ucScriptResultNum)
				ucResult =MSG_AddBatchRecord(Msg_EMV_BatchAdvice);
		}
		
		if(!ucResult&& !TransCapture(EMVTransInfo.enTransResult)&&EMVTransInfo.bAdvice)
		ucResult=MSG_AddBatchRecord(Msg_EMV_BatchAdvice);
		
		*/
		if(!ucResult)
		{
			if(!TransCapture(EMVTransInfo.enTransResult))
			{
				if(EMVTransInfo.ucScriptResultNum||EMVTransInfo.bAdvice)
					ucResult=MSG_AddBatchRecord(Msg_EMV_BatchAdvice);
			}
		}
	

		if(!ucResult&&TransCapture(EMVTransInfo.enTransResult))  
			ucResult =MSG_AddBatchRecord(Msg_EMV_BatchTrans);
		
	}

	if(/*!ucResult&&*/TransCapture(EMVTransInfo.enTransResult))
    {    printf("1111111111IN COMPLETETRANS*****************\n");

		ucResult = AddTransLog();
		if(/*!ucResult &&*/ConstParam.bPrintTicket)
			PrintTransTicket();
	}
	
	if(ucResult)
    {
        memset(ucDispBuff,0x00,sizeof(ucDispBuff));
        Os__clr_display11(255);
        Os__GB2312_display11(2,0,ucDispBuff);

        UTIL_GetKey(3);

	}
	
	
}

UCHAR	AddTransLog()
{
	TRANSRECORD	TransRecord;
	UCHAR		ucResult;
	USHORT		uiLen;
	
	memset(&TransRecord, 0x00 ,sizeof(TransRecord));
	TransRecord.ucTransType = ConstParam.ucTransType;

	uiLen = MAXPANDATALEN;
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5A",TransRecord.aucPAN,&uiLen);
	TransRecord.ucPANLen = uiLen;
	
	uiLen = sizeof(UCHAR);
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5F\x34",&TransRecord.ucPANSeq,&uiLen);

	uiLen = AUTHCODELEN;
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x89",TransRecord.aucAuthCode,&uiLen);

	uiLen = sizeof(UINT);
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x02",(PUCHAR)&TransRecord.uiAmount,&uiLen);
	TransRecord.uiAmount =EMVTransInfo.uiTransAmount;
	TransRecord.uiOtherAmount =EMVTransInfo.uiOtherTransAmount;
	memcpy(TransRecord.aucTransDate,TransReqInfo.aucTransDate,sizeof(TransRecord.aucTransDate));
	memcpy(TransRecord.aucTransTime,TransReqInfo.aucTransTime,sizeof(TransRecord.aucTransTime));

	ucResult =FILE_InsertRecordByFileName(TRANSLOG,&TransRecord,sizeof(TRANSRECORD));
	if(ucResult)
	{
		ConstParam.uiTotalAmount +=TransRecord.uiAmount;
		ConstParam.uiTotalNums++;
		ucResult = UTIL_WriteConstParamFile(&ConstParam);
	}
	return ucResult;
	
}


UINT	CalcLogTransAmount(void)
{
	UINT	uiLogAmount,uiRecordNums,uiI;
	UCHAR	ucResult,aucPAN[MAXPANDATALEN];
	USHORT	uiLen;
	TRANSRECORD	TransRecord;
	uiLogAmount = 0;

	ucResult = FILE_ReadRecordNumByFileName(TRANSLOG,&uiRecordNums);

	if(!ucResult)
	{
		uiLen = MAXPANDATALEN;
		memset(aucPAN,0x00,sizeof(aucPAN));
		ucResult = EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5A",aucPAN,&uiLen);
	}
#if 1
	for(uiI =1; !ucResult && uiI <= uiRecordNums; uiI++)
	{
		ucResult = FILE_ReadRecordByFileName(TRANSLOG,uiI,&TransRecord,sizeof(TRANSRECORD));
		if(!ucResult)
		{
			if(!memcmp(aucPAN,&TransRecord.aucPAN,MAXPANDATALEN))
			{
				uiLogAmount += TransRecord.uiAmount;
			}
		}
	}
#else
	if(ConstParam.bPBOC20)
	{
//		for(uiI =1; !ucResult && uiI <= uiRecordNums; uiI++)
		for(uiI =uiRecordNums; !ucResult && uiI >0; uiI--)
		{
			ucResult = FILE_ReadRecordByFileName(TRANSLOG,uiI,&TransRecord,sizeof(TRANSRECORD));
			if(!ucResult)
			{
				if(!memcmp(aucPAN,&TransRecord.aucPAN,MAXPANDATALEN))
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
				if(!memcmp(aucPAN,&TransRecord.aucPAN,MAXPANDATALEN))
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
        Os__clr_display11(255);
		memset(pbuf,0,32);
		memcpy(pbuf,MagTrack2,i);
        OSMMI_DisplayASCII11(0x30,2, 0, (PUCHAR)"Card NO:");
        OSMMI_DisplayASCII11(0x30,3, 0,  (PUCHAR)pbuf);


		memset(pbuf,0,32);
		memcpy(pbuf,(PUCHAR)&MagTrack2[i+1],4);

		sprintf(buf,"EXP DATE: %4s",pbuf);
        OSMMI_DisplayASCII11(0x30,5,0, (PUCHAR)buf);

		
		memset(pbuf,0,32);
		memcpy(pbuf,&MagTrack2[i+5],3);
		
		sprintf(buf,"Service Code: %3s",pbuf);
        OSMMI_DisplayASCII11(0x30,6,0, (PUCHAR)buf);

        emitSignal();

	}

    Os__xget_key11();
	if(pbuf[0]=='2'||pbuf[0]=='6')//2CM.086.00
	{
		if(!bFallBack)
		{
            Os__clr_display11(255);
			
			if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display11(2,0,(PUCHAR)"Use Chip Reader");

			else
                Os__GB2312_display11(2,0,(PUCHAR)"请使用芯片卡");

            emitSignal();



            Os__xget_key11();
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

    Os__clr_display11(255);

    OSMMI_DisplayASCII11(0x30,0,0,(PUCHAR)"  Select Account");
    OSMMI_DisplayASCII11(0x30,2,0,(PUCHAR)"1. Default");
    OSMMI_DisplayASCII11(0x30,3,0,(PUCHAR)"2. Saving");
    OSMMI_DisplayASCII11(0x30,4,0,(PUCHAR)"3. Debit");
    OSMMI_DisplayASCII11(0x30,5,0,(PUCHAR)"4. Credit");

	switch (ConstParam.ucAccountType){
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
    if(y<6){
        OSMMI_DisplayASCII11(0x30,y,k,(PUCHAR)str);
        emitSignal();

    }
    DRV_OUT *pxKey;
	while (1) {
       ch = Os__xget_key11();
            switch (ch){
			case '1':
				ConstParam.ucAccountType=0x00;
                OSMMI_DisplayASCII11(0x30,y,k,(PUCHAR)" ");
                OSMMI_DisplayASCII11(0x30,2,k,(PUCHAR)str);

				y=2;
				break;
			case '2':
				ConstParam.ucAccountType=0x10;
                OSMMI_DisplayASCII11(0x30,y,k,(PUCHAR)" ");
                OSMMI_DisplayASCII11(0x30,3,k,(PUCHAR)str);

				y=3;
				break;
			case '3':
				ConstParam.ucAccountType=0x20;
                OSMMI_DisplayASCII11(0x30,y,k,(PUCHAR)" ");
                OSMMI_DisplayASCII11(0x30,4,k,(PUCHAR)str);

				y=4;
				break;
			case '4':
				ConstParam.ucAccountType=0x30;
                OSMMI_DisplayASCII11(0x30,y,k,(PUCHAR)" ");
                OSMMI_DisplayASCII11(0x30,5,k,(PUCHAR)str);
				y=5;
				break;
			case KEY_ENTER:
			case KEY_CLEAR:
                break;
			default:
				break;
		}
            emitSignal();

        if(ch==KEY_ENTER || ch==KEY_CLEAR)
			break;
		else 
			continue;

	}
    return ch;
}

BOOL	TransCapture(TRANSRESULT	enTransResult)
{
	switch(enTransResult)
	{
	case	ONLINEAPPROVED:
	case	OFFLINEAPPROVED:
	case	UNABLEONLINE_OFFLINEAPPROVED:
		return	TRUE;
	case	OFFLINEDECLINED:
	case	ONLINEDECLINED:
	case	UNABLEONINE_OFFLINEDECLINED:
		return	FALSE;
		
	}
	
}
#if	1
void	PrintTransTicket(void)
{
    printf("IN PrintTransTicket*****************\n");

	UCHAR	aucBuff[50],aucPAN[MAXPANDATALEN],ucI,ucLen,aucAID[32];
	UCHAR	aucTVR[5],aucTSI[2],ucResult;
	USHORT	uiPANLen,uiLen;

    Os__clr_display11(255);
	if(ConstParam.bPBOC20==FALSE)
        Os__GB2312_display11(2,0,(PUCHAR)"    PRINTING");
	else
        Os__GB2312_display11(2,0,(PUCHAR)"    打印中");

    emitSignal();


	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),ConstParam.aucMerchantName,MERCHANTNAMELEN);
	Os__GB2312_xprint(aucBuff,0x00);

	strcpy((char*)aucBuff,"Terminal ID:");
	Os__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff,ConstParam.aucTerminalID,TERMINALIDLEN);
	Os__GB2312_xprint(aucBuff,0x00);

	strcpy((char*)aucBuff,"Merchant ID:");
	Os__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),ConstParam.aucMerchantID,MERCHANTIDLEN);
	Os__GB2312_xprint(aucBuff,0x00);

	strcpy((char*)aucBuff,"Card PAN:");
	Os__GB2312_xprint(aucBuff,0x00);
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	uiPANLen = sizeof(aucPAN);
	ucLen =strlen((char*)aucBuff);
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5A",aucPAN,&uiPANLen);
	CONV_HexAsc(aucBuff+ucLen,aucPAN,uiPANLen*2);
	for(ucI=0;ucI<uiPANLen*2;ucI++)
	{
		if(aucBuff[ucLen+ucI]=='F')
		{
			aucBuff[ucLen+ucI] =0x00;
			break;
		}
	}
	Os__GB2312_xprint(aucBuff,0x00);

	memset(aucBuff,0x00,sizeof(aucBuff));
	uiLen = sizeof(aucAID);
	strcpy((char*)aucBuff,"AID:");
	ucI =strlen((char*)aucBuff);
	ucResult = EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x4F",aucAID,(USHORT*)&uiLen);
	if(ucResult) uiLen=0;
	CONV_HexAsc(aucBuff+ucI,aucAID,uiLen*2);
	Os__GB2312_xprint(aucBuff,0x00);

	memset(aucBuff,0x00,sizeof(aucBuff));
	Os__GB2312_xprint((PUCHAR)"Trans Date     Time",0x00);
	memcpy(aucBuff,EMVTransInfo.aucTransDate,TRANSDATELEN);
	memcpy(aucBuff+strlen((char*)aucBuff),"    ",4);
	memcpy(aucBuff+strlen((char*)aucBuff),EMVTransInfo.aucTransTime,2);
	memcpy(aucBuff+strlen((char*)aucBuff),":",1);
	memcpy(aucBuff+strlen((char*)aucBuff),&EMVTransInfo.aucTransTime[2],2);
	memcpy(aucBuff+strlen((char*)aucBuff),":",1);
	memcpy(aucBuff+strlen((char*)aucBuff),&EMVTransInfo.aucTransTime[4],2);
	Os__GB2312_xprint(aucBuff,0x00);
	if(EMVTransInfo.bECTrans)
		Os__GB2312_xprint("EC TRANS",0x00);
	
	strcpy((char*)aucBuff,"Amount:");
	Os__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),EMVTransInfo.uiTransAmount,2);
	Os__GB2312_xprint(aucBuff,0x00);

	if(EMVTransInfo.bECTrans)
	{
		strcpy((char*)aucBuff,"BALANCE:");
		Os__GB2312_xprint(aucBuff,0x00);
		memset(aucBuff,0x00,sizeof(aucBuff));
		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),ECTransInfo.uiNBalance,2);
		Os__GB2312_xprint(aucBuff,0x00);

		memset(aucBuff,0x00,sizeof(aucBuff));
		memcpy(aucBuff,ECTransInfo.aucECICode,ECISSUERCODELEN);
		Os__GB2312_xprint(aucBuff,0x00);
	}

	
	if(EMVTransInfo. bCheckSignature)
		Os__GB2312_xprint((uchar*)"Signature: ___________________",0x00);
	Os__GB2312_xprint((uchar*)"    Transaction Approved",0x00);
	
	memcpy(aucTVR,EMVTransInfo.EMVTVR,sizeof(EMVTransInfo.EMVTVR));
	sprintf(aucBuff,"TVR:%02X %02X %02X %02X %02X ",aucTVR[0],aucTVR[1],aucTVR[2],aucTVR[3],aucTVR[4]);
	Os__GB2312_xprint(aucBuff,0x00);

	memcpy(aucTSI,EMVTransInfo.EMVTSI,sizeof(EMVTransInfo.EMVTSI));
	sprintf(aucBuff,"TSI:%02X %02X",aucTSI[0],aucTSI[1]);
	Os__GB2312_xprint(aucBuff,0x00);
	
    //Os__xlinefeed(10);
	
}

#endif

#ifdef _PS100EXPRINT

unsigned char Os__GB2312_exprint(unsigned char *aucInBuf,unsigned char ucFont)
{
	
	unsigned char aucSendBuf[512],ucResult,aucPrintBuf[512];
	unsigned char ucLRC;
	unsigned short uiI,uiLen;
	

	memset(aucPrintBuf,0x00,sizeof(aucPrintBuf));
	uiLen = strlen((char*)aucInBuf);
	aucPrintBuf[0]= 0x11;
	memcpy(&aucPrintBuf[1],aucInBuf,uiLen);
	uiLen ++;
	
	ucLRC=0;
	memset(aucSendBuf,0,sizeof(aucSendBuf));
	aucSendBuf[0] = CHAR_STX;
	short_bcd(&aucSendBuf[1],2,&uiLen);
	memcpy(&aucSendBuf[3],aucPrintBuf,uiLen);
	aucSendBuf[3+uiLen] = CHAR_ETX;
	for(uiI=0,ucLRC=0;uiI<uiLen+2;uiI++)
	{
		ucLRC ^= aucSendBuf[uiI+1];
	}
	aucSendBuf[4+uiLen] = ucLRC;	
	if(ConstParam.ucCommID==1 )
	{
		for(uiI=0;uiI<uiLen+5;uiI++)
			Os__txcar(aucSendBuf[uiI]);
	}
	else
	{
		for(uiI=0;uiI<uiLen+5;uiI++)
			Os__txcar3(aucSendBuf[uiI]);
	}
	
	Os__xdelay(10);
	return(EMVERROR_SUCCESS);	
}
#endif

#if 0
void	PrintTransTicket(void)
{
	
	UCHAR	aucBuff[50],aucPAN[MAXPANDATALEN],ucI,ucLen,aucAID[32];
	UCHAR	aucTVR[5],aucTSI[2],ucResult;
	USHORT	uiPANLen,uiLen;

	COM_PARAM CommParam =
	{
		1,     /* Stop bit */
		9600,  /* Speed */
		1,	   /* MODEM 0 RS232 1 HDLC 2 */
		8,     /* Data bit */
	   'N'     /* NO parity */
	};
    Os__clr_display11(255);
	
	if(ConstParam.bPBOC20==FALSE)
        Os__GB2312_display11(2,0,(PUCHAR)"    PRINTING");
	else
        Os__GB2312_display11(2,0,(PUCHAR)"    打印中");
	
	if(ConstParam.ucCommID==1  )
		ucResult = Os__init_com(&CommParam);
	else
//		ucResult = Os__init_com3(0x0303, 0x1400, 0x14);
		ucResult = Os__init_com3(0x0303, 0x0c00, 0x0c);
	if(ucResult) ucResult =EMVERROR_OPENCOM;
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),ConstParam.aucMerchantName,MERCHANTNAMELEN);
	Os__GB2312_exprint(aucBuff,0x00);
//	Os__GB2312_xprint("\n",0x00);

	strcpy((char*)aucBuff,"Terminal ID:");
	Os__GB2312_exprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff,ConstParam.aucTerminalID,TERMINALIDLEN);
	Os__GB2312_exprint(aucBuff,0x00);

	strcpy((char*)aucBuff,"Merchant ID:");
	Os__GB2312_exprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),ConstParam.aucMerchantID,MERCHANTIDLEN);
	Os__GB2312_exprint(aucBuff,0x00);
//	Os__GB2312_xprint("\n",0x00);

	strcpy((char*)aucBuff,"Card PAN:");
	Os__GB2312_exprint(aucBuff,0x00);
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	uiPANLen = sizeof(aucPAN);
	ucLen =strlen((char*)aucBuff);
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5A",aucPAN,&uiPANLen);
	CONV_HexAsc(aucBuff+ucLen,aucPAN,uiPANLen*2);
	for(ucI=0;ucI<uiPANLen*2;ucI++)
	{
		if(aucBuff[ucLen+ucI]=='F')
		{
			aucBuff[ucLen+ucI] =0x00;
			break;
		}
	}
	Os__GB2312_exprint(aucBuff,0x00);

	memset(aucBuff,0x00,sizeof(aucBuff));
	uiLen = sizeof(aucAID);
	strcpy((char*)aucBuff,"AID:");
	ucI =strlen((char*)aucBuff);
	ucResult = EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x4F",aucAID,(USHORT*)&uiLen);
	if(ucResult) uiLen=0;
//	memcpy(aucBuff+ucLen,aucAID,ucI);
	CONV_HexAsc(aucBuff+ucI,aucAID,uiLen*2);
	Os__GB2312_exprint(aucBuff,0x00);

	memset(aucBuff,0x00,sizeof(aucBuff));
	Os__GB2312_exprint((PUCHAR)"Trans Date     Time",0x00);
	memcpy(aucBuff,EMVTransInfo.aucTransDate,TRANSDATELEN);
	memcpy(aucBuff+strlen((char*)aucBuff),"    ",4);
	memcpy(aucBuff+strlen((char*)aucBuff),EMVTransInfo.aucTransTime,2);
	memcpy(aucBuff+strlen((char*)aucBuff),":",1);
	memcpy(aucBuff+strlen((char*)aucBuff),&EMVTransInfo.aucTransTime[2],2);
	memcpy(aucBuff+strlen((char*)aucBuff),":",1);
	memcpy(aucBuff+strlen((char*)aucBuff),&EMVTransInfo.aucTransTime[4],2);
	Os__GB2312_exprint(aucBuff,0x00);

	strcpy((char*)aucBuff,"Amount:");
	Os__GB2312_exprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),EMVTransInfo.uiTransAmount,2);
	Os__GB2312_exprint(aucBuff,0x00);

	if(EMVTransInfo.enTransType ==CASHBACK)
	{
		strcpy((char*)aucBuff,"Other Amount:");
		Os__GB2312_exprint(aucBuff,0x00);
		memset(aucBuff,0x00,sizeof(aucBuff));
		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),EMVTransInfo.uiOtherTransAmount,2);
		Os__GB2312_exprint(aucBuff,0x00);
	}

	
	if(EMVTransInfo. bCheckSignature)
		Os__GB2312_exprint((uchar*)"Signature: ___________________",0x00);
//	Os__GB2312_xprint("\n\n",0x00);
	Os__GB2312_exprint((uchar*)"    Transaction Approved",0x00);
	
	memcpy(aucTVR,EMVTransInfo.EMVTVR,sizeof(EMVTransInfo.EMVTVR));
	sprintf(aucBuff,"TVR:%02X %02X %02X %02X %02X ",aucTVR[0],aucTVR[1],aucTVR[2],aucTVR[3],aucTVR[4]);
	Os__GB2312_exprint(aucBuff,0x00);

	memcpy(aucTSI,EMVTransInfo.EMVTSI,sizeof(EMVTransInfo.EMVTSI));
	sprintf(aucBuff,"TSI:%02X %02X",aucTSI[0],aucTSI[1]);
	Os__GB2312_exprint(aucBuff,0x00);
	for(ucI=0;ucI<10;ucI++)
		Os__GB2312_exprint((UCHAR*)"  ",0x00);
	
	if(ConstParam.ucCommID==1)
		OSUART_Close1();
	else
		OSUART_Close2();	
}	

#endif



//#ifdef _PS300
#if 0



unsigned char ext__CheckBase(void)
{
	if( Os__CheckBase() )
	{
		Os__light_on();
        Os__clr_display11(255);
        Os__GB2312_display11(0,0,(uchar *)"请放回座机！");
	}else
	{
		return EMVERROR_SUCCESS;
	}
	
	while(1)
	{
		if( !Os__CheckBase() )
		{
            //Os__light_off();
			break;
		}
	}
    Os__clr_display11(255);
    Os__GB2312_display11(0,0,(unsigned char *)"正在打印...");
	return EMVERROR_SUCCESS;
}


unsigned char EXTPRINT_ext_xprintGB2312(unsigned char ucASCFont,unsigned char ucGBFont,unsigned char *pucPtr)
{
unsigned char aucSendBuf[96],aucRevBuf[16];
unsigned char ucResult,ucI;
unsigned int uiSendLen,uiRevLen;

	ucI=0;
	uiRevLen=sizeof(aucRevBuf);
	memset( aucSendBuf,0,sizeof(aucSendBuf) );
	memset( aucRevBuf,0,sizeof(aucRevBuf) );
    aucSendBuf[ucI++]=EXTPRINTCMD_PRINTGB2312;
    aucSendBuf[ucI++]=ucASCFont;
    aucSendBuf[ucI++]=ucGBFont;
    uiSendLen=strlen((char *)pucPtr);
    memcpy( &aucSendBuf[ucI],pucPtr,uiSendLen );
    uiSendLen+=3;
	ucResult=OSIR_SendReceive( HALIREXTPRNADDR,EXTPRINT_MAX_WAITIME,
			aucSendBuf,uiSendLen,aucRevBuf,&uiRevLen );
	if( ucResult==EMVERROR_SUCCESS )
		ucResult=aucRevBuf[0];		
	return ucResult;
}

unsigned char ext__GB2312_xprint(unsigned char *pucPtr, unsigned char ucGBFont)
{
 if(ext__CheckBase()==EMVERROR_SUCCESS)

	return(EXTPRINT_ext_xprintGB2312(0x31,ucGBFont,pucPtr));
	return EMVERROR_SUCCESS;
}


unsigned char EXTPRINT_XFeed(unsigned char ucLineNB)
{
unsigned char aucSendBuf[96],aucRevBuf[16];
unsigned char ucResult,ucI;
unsigned int uiSendLen,uiRevLen;

	ucI=0;
	uiRevLen=sizeof(aucRevBuf);
	memset( aucSendBuf,0,sizeof(aucSendBuf) );
	memset( aucRevBuf,0,sizeof(aucRevBuf) );
    aucSendBuf[ucI++]=EXTPRINTCMD_FEED;
    aucSendBuf[ucI++]=0x30+ucLineNB;
    uiSendLen=2;
	ucResult=OSIR_SendReceive( HALIREXTPRNADDR,EXTPRINT_MAX_WAITIME,
			aucSendBuf,uiSendLen,aucRevBuf,&uiRevLen );
	if( ucResult==EMVERROR_SUCCESS )
		ucResult=aucRevBuf[0];		
	return ucResult;
}

unsigned char ext__xlinefeed(unsigned char ucLineNB)
{
   if(ext__CheckBase()==EMVERROR_SUCCESS)
    return(EXTPRINT_XFeed(ucLineNB));
    return EMVERROR_SUCCESS;
}


void	PrintTransTicket(void)
{
	
	UCHAR	aucBuff[50],aucPAN[MAXPANDATALEN],ucI,ucLen,aucAID[32];
	UCHAR	aucTVR[5],aucTSI[2],ucResult;
	USHORT	uiPANLen,uiLen;
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),ConstParam.aucMerchantName,MERCHANTNAMELEN);
	ext__GB2312_xprint(aucBuff,0x00);
//	Os__GB2312_xprint("\n",0x00);

	strcpy((char*)aucBuff,"Terminal ID:");
	ext__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff,ConstParam.aucTerminalID,TERMINALIDLEN);
	ext__GB2312_xprint(aucBuff,0x00);

	strcpy((char*)aucBuff,"Merchant ID:");
	ext__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	memcpy(aucBuff+strlen((char*)aucBuff),ConstParam.aucMerchantID,MERCHANTIDLEN);
	ext__GB2312_xprint(aucBuff,0x00);
//	Os__GB2312_xprint("\n",0x00);

	strcpy((char*)aucBuff,"Card PAN:");
	ext__GB2312_xprint(aucBuff,0x00);
	
	memset(aucBuff,0x00,sizeof(aucBuff));
	uiPANLen = sizeof(aucPAN);
	ucLen =strlen((char*)aucBuff);
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5A",aucPAN,&uiPANLen);
	CONV_HexAsc(aucBuff+ucLen,aucPAN,uiPANLen*2);
	for(ucI=0;ucI<uiPANLen*2;ucI++)
	{
		if(aucBuff[ucLen+ucI]=='F')
		{
			aucBuff[ucLen+ucI] =0x00;
			break;
		}
	}
	ext__GB2312_xprint(aucBuff,0x00);

	memset(aucBuff,0x00,sizeof(aucBuff));
	uiLen = sizeof(aucAID);
	strcpy((char*)aucBuff,"AID:");
	
	ext__GB2312_xprint(aucBuff,0x00);//08.09.10 pan 
	memset(aucBuff,0x00,sizeof(aucBuff)); //pan
	   
	// ucI =strlen((char*)aucBuff);pan
	ucResult = EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x4F",aucAID,(USHORT*)&uiLen);
	if(ucResult) uiLen=0;
//	memcpy(aucBuff+ucLen,aucAID,ucI);
    //CONV_HexAsc(aucBuff+ucI,aucAID,uiLen*2);//pan
	CONV_HexAsc(aucBuff,aucAID,uiLen*2);//pan 
	ext__GB2312_xprint(aucBuff,0x00);

	memset(aucBuff,0x00,sizeof(aucBuff));
	ext__GB2312_xprint((PUCHAR)"Trans Date     Time",0x00);
	memcpy(aucBuff,EMVTransInfo.aucTransDate,TRANSDATELEN);
	memcpy(aucBuff+strlen((char*)aucBuff),"    ",4);
	memcpy(aucBuff+strlen((char*)aucBuff),EMVTransInfo.aucTransTime,2);
	memcpy(aucBuff+strlen((char*)aucBuff),":",1);
	memcpy(aucBuff+strlen((char*)aucBuff),&EMVTransInfo.aucTransTime[2],2);
	memcpy(aucBuff+strlen((char*)aucBuff),":",1);
	memcpy(aucBuff+strlen((char*)aucBuff),&EMVTransInfo.aucTransTime[4],2);
	ext__GB2312_xprint(aucBuff,0x00);

	strcpy((char*)aucBuff,"Amount:");
	ext__GB2312_xprint(aucBuff,0x00);
	memset(aucBuff,0x00,sizeof(aucBuff));
	UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),EMVTransInfo.uiTransAmount,2);
	ext__GB2312_xprint(aucBuff,0x00);

	if(EMVTransInfo.enTransType ==CASHBACK)
	{
		strcpy((char*)aucBuff,"Other Amount:");
		ext__GB2312_xprint(aucBuff,0x00);
		memset(aucBuff,0x00,sizeof(aucBuff));
		UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),EMVTransInfo.uiOtherTransAmount,2);
		ext__GB2312_xprint(aucBuff,0x00);
	}

	
	if(EMVTransInfo. bCheckSignature)
		ext__GB2312_xprint((uchar*)"Signature: __________________",0x00);
//	Os__GB2312_xprint("\n\n",0x00);
	ext__GB2312_xprint((uchar*)"  Transaction Approved",0x00);//pan 
	
	memcpy(aucTVR,EMVTransInfo.EMVTVR,sizeof(EMVTransInfo.EMVTVR));
	sprintf(aucBuff,"TVR:%02X %02X %02X %02X %02X ",aucTVR[0],aucTVR[1],aucTVR[2],aucTVR[3],aucTVR[4]);
	ext__GB2312_xprint(aucBuff,0x00);

	memcpy(aucTSI,EMVTransInfo.EMVTSI,sizeof(EMVTransInfo.EMVTSI));
	sprintf(aucBuff,"TSI:%02X %02X",aucTSI[0],aucTSI[1]);
	ext__GB2312_xprint(aucBuff,0x00);
	
	ext__xlinefeed(10);
	
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
	unsigned char ucTransType,aucATC[4],ucTagLenNum;
	CANDIDATELIST	CandidateList;
    NEW_DRV_TYPE new_drv;
	UINT	uiAmount;
	while(1)
	{

        Os__clr_display11(255);
        Os__GB2312_display11(2,0,"请插卡");

        emitSignal();

       unsigned int ret = Os_Wait_Event(KEY_DRV|ICC_DRV,&new_drv,15000);

       if(ret == 0){
        if(new_drv.drv_type == KEY_DRV)
		{
            pxKey = &new_drv.drv_data;
			Os__abort_drv(drv_icc);
			ucKey = pxKey->xxdata[1];
			if(ucKey==KEY_CLEAR) return;
				
					
		}
        else if(new_drv.drv_type == ICC_DRV)
		{
            pxIcc = &new_drv.drv_data;
			Os__abort_drv(drv_mmi);
			ucICCardType =pxIcc->xxdata[1];
			ucResetBufLen=sizeof(aucResetBuf);
			if(ucICCardType==0x39)
				ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);
		
			if(ucICCardType!=ICC_ASY)
				continue;

            Os__clr_display11(255);
            Os__GB2312_display11(2,0,"处理中");

            emitSignal();

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
			
			EMVDATA_TermInit(&EMVConfig);
			ucResult = APPSELECT_CreateAppCandidate(&CandidateList);
			
			bConfirm =FALSE;
			if(CandidateList.CandidateNums ==1 &&CandidateList.CandidateApp[0].bAppPriorityExist)
			{
				if(CandidateList.CandidateApp[0].ucAppPriorityIndicator &APPNEEDCONFIRM)
					bConfirm = TRUE;
			}
			else if (CandidateList.CandidateNums >1)
				bConfirm = TRUE;
				
			while(!ucResult && CandidateList.CandidateNums)
			{
				ucAppIndex = 0;
				
				if(bConfirm)
				{
					ucResult = CardHolderConfirmApp(&CandidateList,&ucAppIndex);
				}
				
				if(!ucResult)
				{
					ucResult = APPSELECT_FinalSelect(ucAppIndex,&CandidateList);
					if(EMVICC_CommandResult() == CARDSWDEF_INVALIDFILE)
					{
						ucResult =EMVERROR_SUCCESS;
						break;
					}
					if(ucResult == EMVERROR_RESELECT)
					{
						ucResult =EMVERROR_SUCCESS;
						continue;
					}
					else break;
				}
			}
			
			if(ucResult)	
			{
				EMVERROR_DisplayMsg(ucResult);
				break;
			}
			else
			{
				EMVTAG_INIT(( DATAPROPERTY*)&ICCardProperty,FALSE);
				if(!ucResult)
					ucResult = EMVTAG_Unpack(EMVIccOut.aucDataOut,EMVIccOut.uiLenOut);
				if(!ucResult)
					ucResult = EMVTRANSTAG_CopyICDataInfo(&ICDataInfo,ADFPHASETAG);
			}
			if(ucResult) break;
			uiLen =sizeof(aucEntryBuf);
			ucResult=EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x4D",aucEntryBuf,&uiLen);
			if(ucResult||uiLen!=2)
			{
                Os__clr_display11(255);
                Os__GB2312_display11(2,0,"无交易记录");

                emitSignal();

                Os__xget_key11();
				break;
			}
			ucRecordSFI=aucEntryBuf[0];
			ucRecordNums=aucEntryBuf[1];
			
			ucResult=EMVICC_GetData((PUCHAR)"\x9F\x4F",2);
			if(ucResult||EMVICC_CommandResult() !=CARDSWDEF_SUCCESS) 
				break;
			
			ucDOLLen =EMVIccOut.aucDataOut[2];
			memset(aucDOL,0x00,sizeof(aucDOL));
			memcpy(aucDOL,&EMVIccOut.aucDataOut[3],ucDOLLen);
			ucRecordNum=0;
			
			ucResult=EMVERROR_SUCCESS;
			for(ucI=1;!ucResult&&ucI<=ucRecordNums;ucI++)
			{
				ucResult=EMVICC_ReadRecordbySFI(ucRecordSFI,ucI);
				if(!ucResult&&EMVICC_CommandResult() == CARDSWDEF_SUCCESS)
				{
					
					ucRecordNum++;
					pDOL=aucDOL;
					pDataOut=(unsigned char *)&EMVIccOut.aucDataOut;
					memset(aucTransDate,0x00,sizeof(aucTransDate));
					memset(aucTransTime,0x00,sizeof(aucTransTime));
					memset(aucAuthAmount,0x00,sizeof(aucAuthAmount));
					memset(aucOtherAmount,0x00,sizeof(aucOtherAmount));
					memset(aucCountryCode,0x00,sizeof(aucCountryCode));
					memset(aucTransCurcyCode,0x00,sizeof(aucTransCurcyCode));
					memset(aucMerchantName,0x00,sizeof(aucMerchantName));
					memset(aucATC,0x00,sizeof(aucATC));
					ucTransType=CASH;
					
					while(pDataOut<EMVIccOut.aucDataOut+EMVIccOut.uiLenOut)
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
					
                    Os__clr_display11(255);
#if 0
					if(ConstParam.bPBOC20==FALSE)
					{
						memset(aucBuff,0x00,sizeof(aucBuff));
						sprintf(aucBuff,"REC %d / %d ",ucI,ucRecordNums);
                        OSMMI_DisplayASCII11(0x30,0,0,aucBuff);

						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"TransDate:");
						bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransDate,6);
                        OSMMI_DisplayASCII11(0x30,1,0,aucBuff);

						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"TransTime:");
						bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransTime,6);
                        OSMMI_DisplayASCII11(0x30,2,0,aucBuff);

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
                        OSMMI_DisplayASCII11(0x30,3,0,aucBuff);

						memset(aucBuff,0x00,sizeof(aucBuff));
					
						strcpy((char *)aucBuff,(char *)"Other Amt:");
                        //OSMMI_DisplayASCII11(0x30,4,0,aucBuff);
						
						//memset(aucBuff,0x00,sizeof(aucBuff));
						//strcpy((char *)aucBuff,(char *)"---:");
						//memset(aucAmtBuf,0x00,sizeof(aucAmtBuf));
						
						bcd_asc(aucAmtBuf,aucOtherAmount,12);
						for(ucJ=0;ucJ<12;ucJ++)
							if(aucAmtBuf[ucJ]!='0') break;
						if(ucJ>=10)
						{
						  if(ucJ==12)
						  {
						  	 memcpy(&aucBuff[10],"0.00",4);
							
						  }
						else
						{
							memcpy(&aucBuff[10],"0.",2);
							memcpy(&aucBuff[12],&aucAmtBuf[ucJ],12-ucJ);
							}
							
						
						}else
						{
							memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[ucJ],12-ucJ-2);
							strcat((char*)aucBuff,".");
							memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[10],2);
						}		
                        OSMMI_DisplayASCII11(0x30,5,0,aucBuff);

						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"Country:");
						bcd_asc(aucBuff+strlen((char*)aucBuff),aucCountryCode,4);
                        OSMMI_DisplayASCII11(0x30,6,0,aucBuff);
						
						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"Currency:");
						bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransCurcyCode,4);
                        //Os__display11(4,0,aucBuff);
                        OSMMI_DisplayASCII11(0x30,7,0,aucBuff);

											
                        Os__xget_key11();
                        Os__clr_display11(255);
						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"Name:");
						memcpy(aucBuff+strlen((char*)aucBuff),aucMerchantName,16);
                        OSMMI_DisplayASCII11(0x30,0,0,aucBuff);
						memset(aucBuff,0x00,sizeof(aucBuff));
						
						memcpy(aucBuff,&aucMerchantName[16],sizeof(aucMerchantName)-16);	
                        OSMMI_DisplayASCII11(0x30,1,0,aucBuff);

						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"TransType:");
						hex_asc(aucBuff+strlen((char*)aucBuff),&ucTransType,2);
                        OSMMI_DisplayASCII11(0x30,2,0,aucBuff);
						
                        Os__xget_key11();
						
                        Os__clr_display11(255);
						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"ATC:");
						hex_asc(aucBuff+strlen((char*)aucBuff),aucATC,4);
                        OSMMI_DisplayASCII11(0x30,0,0,aucBuff);
						if(ucI<ucRecordNums)
                            OSMMI_DisplayASCII11(0x30,6,0,(unsigned char*)"Any Key To Countinue");
						else
                            OSMMI_DisplayASCII11(0x30,6,0,(unsigned char*)"Record End");
                        Os__xget_key11();

					}
					else
	#else
					{
						memset(aucBuff,0x00,sizeof(aucBuff));
						sprintf(aucBuff,"��¼ %d / %d ",ucI,ucRecordNums);
                        Os__GB2312_display11(0,0,aucBuff);

						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"����:");
						bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransDate,6);
                        Os__GB2312_display11(1,0,aucBuff);

						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"ʱ��:");
						bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransTime,6);
                        Os__GB2312_display11(2,0,aucBuff);

                        emitSignal();

                        Os__xget_key11();
                        Os__clr_display11(255);
						
						memset(aucBuff,0x00,sizeof(aucBuff));
						memset(aucAmtBuf,0x00,sizeof(aucAmtBuf));
						strcpy((char *)aucBuff,(char *)"����:");
                        Os__GB2312_display11(0,0,aucBuff);

						memset(aucBuff,0x00,sizeof(aucBuff));

						bcd_asc(aucAmtBuf,aucAuthAmount,12);
						for(ucJ=0;ucJ<12;ucJ++)
							if(aucAmtBuf[ucJ]!='0') break;
						if(ucJ>=10)
						{
							memcpy(&aucBuff[0],"0.",2);
							memcpy(&aucBuff[2],&aucAmtBuf[ucJ],12-ucJ);
						}else
						{
							memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[ucJ],12-ucJ-2);
							strcat((char*)aucBuff,".");
							memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[10],2);
						}		
                        Os__GB2312_display11(1,0,aucBuff);


						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char *)aucBuff,(char *)"��������:");
                        Os__GB2312_display11(2,0,aucBuff);

						memset(aucBuff,0x00,sizeof(aucBuff));

						bcd_asc(aucAmtBuf,aucOtherAmount,12);
						for(ucJ=0;ucJ<12;ucJ++)
							if(aucAmtBuf[ucJ]!='0') break;
						if(ucJ>=10)
						{
						  if(ucJ==12)
						  {
						  	 memcpy(&aucBuff[0],"0.00",4);
							
						  }
						else
						{
							memcpy(&aucBuff[0],"0.",2);
							memcpy(&aucBuff[2],&aucAmtBuf[ucJ],12-ucJ);
						}
							
						
						}
						else
						{
							memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[ucJ],12-ucJ-2);
							strcat((char*)aucBuff,".");
							memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[10],2);
						}		
                        Os__GB2312_display11(3,0,aucBuff);

                        emitSignal();


                        Os__xget_key11();
                        Os__clr_display11(255);

						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"���Ҵ���:");
						bcd_asc(aucBuff+strlen((char*)aucBuff),aucCountryCode,4);
                        Os__GB2312_display11(0,0,aucBuff);

						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"���Ҵ���:");
						bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransCurcyCode,4);
                        Os__GB2312_display11(1,0,aucBuff);


						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"��������:");
						hex_asc(aucBuff+strlen((char*)aucBuff),&ucTransType,2);
                        Os__GB2312_display11(2,0,aucBuff);

                        emitSignal();


                        Os__xget_key11();
                        Os__clr_display11(255);

						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"����:");
                        Os__GB2312_display11(0,0,aucBuff);

                        ////Os__GB2312_display11(0,0,aucBuff);
						memset(aucBuff,0x00,sizeof(aucBuff));

						memcpy(aucBuff,aucMerchantName,16);

                        Os__GB2312_display11(1,0,aucBuff);

						memset(aucBuff,0x00,sizeof(aucBuff));
						
						memcpy(aucBuff,&aucMerchantName[16],sizeof(aucMerchantName)-16);	
                        Os__GB2312_display11(2,0,aucBuff);

                        emitSignal();


                        Os__xget_key11();
						
                        Os__clr_display11(255);
						memset(aucBuff,0x00,sizeof(aucBuff));
						strcpy((char*)aucBuff,"ATC:");
						hex_asc(aucBuff+strlen((char*)aucBuff),aucATC,4);
                        Os__GB2312_display11(0,0,aucBuff);

                        emitSignal();

						if(ucI<ucRecordNums)
                            Os__GB2312_display11(3,0,(unsigned char*)"任意键继续");

						else
                            Os__GB2312_display11(3,0,(unsigned char*)"记录结束");

                        emitSignal();


                        Os__xget_key11();

					}
#endif
				}
			}
			if(ucRecordNum=0)
			{
                Os__clr_display11(255);
                Os__GB2312_display11(2,0,(unsigned char *)"无交易流水");
                emitSignal();

                Os__xget_key11();
			}
            Os__clr_display11(255);
            Os__GB2312_display11(0,0,(unsigned char*)"请拔出卡");
            emitSignal();

			Os__ICC_remove();
			return;
		}
		
       }
	}

	
	return;			
}
UCHAR	OnlineRespDataProcess(void)
{
	UCHAR	ucResult;
#if 0	
	ucResult =EMVERROR_SUCCESS;
	if(!ucResult)
	{
		EMVTRANSTAG_SetTag(ALLPHASETAG,"\x8A",aucAuthRespCode,uiAuthRespCodeLen);
		EMVTRANSTAG_SetTag(ALLPHASETAG,"\x89",aucAuthCode,uiAuthCodeLen);
		if(uiIssureAuthDatLen)
			EMVTRANSTAG_SetTag(ALLPHASETAG,"\x91",aucIssureAuthData,uiIssureAuthDatLen);
	}
	for(ucI=0;!ucResult&&ucI<ucScriptNums;ucI++)
	{
		ucResult =APPCRIPT_AppendScript(aucScriptData,uiScriptLen);
	}
#endif
	return ucResult;
	
}



UCHAR	ReadICBalance(void)
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
	unsigned char ucTransType,aucATC[4],ucTagLenNum;
	CANDIDATELIST	CandidateList;
	UINT	uiAmount;
    NEW_DRV_TYPE new_drv;
	
	while(1)
	{
      Os__clr_display11(255);

        Os__clr_display11(255);
        Os__GB2312_display11(2,0,"请插卡");

       Os_Wait_Event(KEY_DRV|ICC_DRV,&new_drv,0);

       if(new_drv.drv_type==KEY_DRV)
		{
            pxKey = &new_drv.drv_data;
			Os__abort_drv(drv_icc);
			ucKey = pxKey->xxdata[1];
			if(ucKey==KEY_CLEAR) return EMVERROR_CANCEL;
								
		}
        else if(new_drv.drv_type== ICC_DRV)
		{
            pxIcc = &new_drv.drv_data;
			Os__abort_drv(drv_mmi);
			ucICCardType =pxIcc->xxdata[1];
			ucResetBufLen=sizeof(aucResetBuf);
			if(ucICCardType==0x39)
				ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);
		
			if(ucICCardType!=ICC_ASY)
				continue;

            Os__clr_display11(255);
            Os__GB2312_display11(2,0,"处理中");

            emitSignal();


			EMVConfig.ucCAPKNums =ucTermCAPKNum;
			EMVConfig.pTermSupportCAPK =TermCAPK;
			EMVConfig.ucTermSupportAppNum = ucTermAIDNum;
			EMVConfig.pTermSupportApp = TermAID;
			EMVConfig.ucIPKRevokeNum =ucIPKRevokeNum;
			EMVConfig.pIPKRevoke =IPKRevoke;
			EMVConfig.ucExceptFileNum = ucExceptFileNum;
			EMVConfig.pExceptPAN = ExceptFile;
			EMVConfig.pfnEMVICC = EMVICC;
			
			EMVDATA_TermInit(&EMVConfig);
			ucResult = APPSELECT_CreateAppCandidate(&CandidateList);
			
			bConfirm =FALSE;
			if(CandidateList.CandidateNums ==1 &&CandidateList.CandidateApp[0].bAppPriorityExist)
			{
				if(CandidateList.CandidateApp[0].ucAppPriorityIndicator &APPNEEDCONFIRM)
					bConfirm = TRUE;
			}
			else if (CandidateList.CandidateNums >1)
				bConfirm = TRUE;
				
			while(!ucResult && CandidateList.CandidateNums)
			{
				ucAppIndex = 0;
				
				if(bConfirm)
				{
					ucResult = CardHolderConfirmApp(&CandidateList,&ucAppIndex);
				}
				
				if(!ucResult)
				{
					ucResult = APPSELECT_FinalSelect(ucAppIndex,&CandidateList);
					if(EMVICC_CommandResult() == CARDSWDEF_INVALIDFILE)
					{
						ucResult =EMVERROR_SUCCESS;
						break;
					}
					if(ucResult == EMVERROR_RESELECT)
					{
						ucResult =EMVERROR_SUCCESS;
						continue;
					}
					else break;
				}
			}
			
			if(ucResult)	
			{
				EMVERROR_DisplayMsg(ucResult);
				break;
			}
			else
			{
				EMVTAG_INIT(( DATAPROPERTY*)&ICCardProperty,FALSE);
				if(!ucResult)
					ucResult = EMVTAG_Unpack(EMVIccOut.aucDataOut,EMVIccOut.uiLenOut);
				if(!ucResult)
					ucResult = EMVTRANSTAG_CopyICDataInfo(&ICDataInfo,ADFPHASETAG);
			}
			if(ucResult) break;

			ucResult =EMVICC_GetData((PUCHAR)"\x9F\x79",2);
			if(ucResult)
				break;
			ucResult = EMVICC_CommandResult();
			if(ucResult)
				break;
			uiAmount =CONV_BcdLong(EMVIccOut.aucDataOut,EMVIccOut.uiLenOut*2);
            Os__clr_display11(255);
            Os__GB2312_display11(0,0,"脱机金额:");


			sprintf(aucBuff,"          %d.%02d",uiAmount/100,uiAmount%100);
            Os__GB2312_display11(1,0,aucBuff);

            emitSignal();

			Os__ICC_remove();
			break;
		}
		
	}

	if(ucResult)
	{
        Os__clr_display11(255);
        Os__GB2312_display11(2,0,(PUCHAR)"卡未被接收");
        emitSignal();


		UTIL_GetKey(3);

	}
	return;			
}


