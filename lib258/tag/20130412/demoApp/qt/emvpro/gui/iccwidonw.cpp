#include "iccwidonw.h"
#include "ui_iccwidonw.h"

#include <QtDebug>
#include "EMVError.h"
#include "oslib.h"
#include "test/Global.h"
#include "test/Util.h"

#include "msginfo.h"
ICCWidonw::ICCWidonw(QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint),
    ui(new Ui::ICCWidonw)
{
    ui->setupUi(this);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QColor("gray")));
    setPalette(palette);
    setAutoFillBackground(true);

    ui->lineEdit->setFocus();
    uiAmount = 0;
    uiOtherAmount =0;

      qDebug()<<"IN ICCWINDOW***************";
    connect(ui->lineEdit,SIGNAL(editingFinished()),this,SLOT(enter_slot()));
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(enter_slot()));
}

ICCWidonw::~ICCWidonw()
{
    delete ui;
}

void ICCWidonw::enter_slot()
{
    uiAmount = ui->lineEdit->text().toInt();
    if(enTransType ==CASHBACK)
    {

       if(ConstParam.bPBOC20==FALSE)
            ui->label->setText("Cashback Amount:");

        else
            ui->label->setText("返现金:");


        uiOtherAmount = ui->lineEdit->text().toInt();

    }
    else
        uiOtherAmount =0;

    MsgInfo *msg = new MsgInfo(1,this);
    msg->show();

       // ucResult = EMVTransProcess(enTransType,uiAmount,uiOtherAmount);

//        if(ucResult ==EMVERROR_FALLBACK)
//        {
//            bFallBack = TRUE;
//        }
//        else
//            bFallBack = FALSE;


    if(ConstParam.bPBOC20==FALSE)
        ui->label->setText("Remove pls");
    else
        ui->label->setText(tr("请 移 卡！"));

    Os__ICC_remove();

}

void ICCWidonw::setText(QString text, TRANSTYPE	enTranstype)
{
     qDebug()<<"IN ICCWINDOW11111111111***************";
        show();
        ui->label->setText(text);
        enTransType = enTranstype;
}



UCHAR ICCWidonw::EMVTransProcess(TRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount)
{
#if 0
    UCHAR	ucResult,aucDateTime[10];
    CRYPTTYPE	CryptType;
    UINT		uiLogSumAmount;
    BOOL	bConfirm,bConnectSuccess,bHostApproval;
    UCHAR	ucAppIndex;
    CANDIDATELIST	CandidateList;
    UCHAR	aucRecvBuff[500],aucBuff[30],ucKey,ucRequestNum,ucLen;
    USHORT  uiRecvLen;
    CONFIG_PARAM	cp;


    if(ConstParam.bPBOC20==FALSE)
        ui->label->setText(tr("Processing"));
    else
        ui->label->setText(tr("´¦ÀíÖÐ"));

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
    //////cp =OSCFG_ReadParam();
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

                if(ConstParam.bPBOC20==FALSE)
                   ui->label->setText(tr("Try Again"));
                else
                    ui->label->setText(tr("ÔÙÀ´Ò»´Î"));


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

            if(ConstParam.bPBOC20==FALSE)
                ui->label->setText(tr("Processing"));
            else
                ui->label->setText(tr("´¦ÀíÖÐ"));

            if(!ucResult) ucResult = INTIAPP_GPO();

            if(ucResult == EMVERROR_RESELECT)
            {
                ucResult = APPSELECT_GetAppCandidate(&CandidateList);

                if(ConstParam.bPBOC20==FALSE)
                    ui->label->setText(tr("Try Again"));
                else
                    ui->label->setText(tr("ÔÙÀ´Ò»´Î"));
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


        if(EMVTransInfo.bECTrans)
        {
            memset(aucBuff,0x00,sizeof(aucBuff));
            memcpy(aucBuff,ECTransInfo.aucECICode,ECISSUERCODELEN);
            ui->label_2->setText(tr(aucBuff));
            memset(aucBuff,0x00,sizeof(aucBuff));
            if(ConstParam.bPBOC20==FALSE)
            {
                strcpy((char*)aucBuff,"Balance:");
                UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff)-3,ECTransInfo.uiBalance,2);
            }
            else
            {
                strcpy((char*)aucBuff,"Óà¶î:");
                UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),ECTransInfo.uiBalance,2);
            }

           ui->label->setText(tr(aucBuff));
        }
        memset(aucBuff,0x00,sizeof(aucBuff));

        if(ConstParam.bPBOC20==FALSE)
            strcpy((char*)aucBuff,"Amt:");
        else
            strcpy((char*)aucBuff,"½ð¶î:");

        UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),TransReqInfo.uiAmount,2);
#if 1

        ui->label->setText(tr(aucBuff));
        if(ConstParam.bPBOC20==FALSE)
            ui->label->setText(tr("Confirm pls"));
        else
            ui->label->setText(tr("È·ÈÏ"));

        do
        {
            ucKey = Os__xget_key();
            if(ucKey ==KEY_CLEAR)	return EMVERROR_CANCEL;
            if(ucKey ==KEY_ENTER)	break;
        }while(1);
        //if(Os__xget_key()!=KEY_ENTER) return EMVERROR_CANCEL;

        if(ConstParam.bPBOC20==FALSE)
            ui->label->setText(tr("Processing"));
        else
           ui->label->setText(tr("´¦ÀíÖÐ"));

#endif
    }
    if(!ucResult) ucResult = EMVVERIFY_Process();

    if(!ucResult)
    {
        ucResult = TERMRISKMANAGE_Process(uiLogSumAmount);
        if(ConstParam.bShowRandNum)
        {
            memset(aucBuff,0x00,sizeof(aucBuff));

            if(ConstParam.bPBOC20==FALSE)
                ui->label_2->setText(tr("Random Num:"));
            else
                ui->label->setText(tr("Ëæ»úÊý:"));

            //CONV_LongAsc(aucBuff,&EMVTransInfo.ucRandNum,2);
            CONV_CharAsc(aucBuff,2,&EMVTransInfo.ucRandNum);
            ui->label->setText(tr(aucBuff));
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


        if(ConstParam.bPBOC20==FALSE)
            ui->label->setText(tr("TRANS TERMINATED"));
        else
            ui->label->setText(tr("½»Ò×ÖÕÖ¹"));

        if(ucResult ==EMVERROR_FALLBACK)
        {
            if(ConstParam.bPBOC20==FALSE)
                ui->label->setText(tr("USE MAG CARD"));
            else
               ui->label->setText(tr("ÇëÓÃ´ÅÌõ¿¨"));
        }

        UTIL_GetKey(3);
    }
    else
    {

        switch(EMVTransInfo.enTransResult)
        {
        case OFFLINEAPPROVED:
            {
            if(ConstParam.bPBOC20==FALSE)
                ui->label->setText(tr("OFFLINE APPROVED"));
            else
               ui->label->setText(tr("ÍÑ»úÅú×¼"));
            }
            break;
        case OFFLINEDECLINED:
            {
            if(ConstParam.bPBOC20==FALSE)
                ui->label->setText(tr("OFFLINE DECLINED"));
            else
                ui->label->setText(tr("ÍÑ»ú¾Ü¾ø"));
            }
            break;

        case ONLINEAPPROVED:
            {
            if(ConstParam.bPBOC20==FALSE)
                ui->label->setText(tr("ONLINE APPROVED"));
            else
                ui->label->setText(tr("Áª»úÅú×¼"));
            }
            break;
        case ONLINEDECLINED:
            {
            if(ConstParam.bPBOC20==FALSE)
                ui->label->setText(tr("ONLINE DECLINED"));
            else
                ui->label->setText(tr("Áª»ú¾Ü¾ø"));
            }
            break;
        case UNABLEONLINE_OFFLINEAPPROVED:
            {
            if(ConstParam.bPBOC20==FALSE)
                ui->label->setText(tr("OFFLINE APPROVED"));
            else
                ui->label->setText(tr("ÍÑ»úÅú×¼"));
            }
            break;
        case UNABLEONINE_OFFLINEDECLINED:
            {
            if(ConstParam.bPBOC20==FALSE)
                ui->label->setText(tr("OFFLINE DECLINED"));
            else
                ui->label->setText(tr("ÍÑ»ú¾Ü¾ø"));
            }
            break;
        default:
            {
            if(ConstParam.bPBOC20==FALSE)
                ui->label->setText(tr("UNKNOWN"));
            else
                ui->label->setText(tr("Î´Öª"));
            }
            break;
        }
        if(EMVTransInfo.bECTrans)
        {
            {
            if(ConstParam.bPBOC20==FALSE)
                ui->label->setText(tr("BALANCE:"));
            else
                ui->label->setText(tr("Óà¶î:"));
            }
            memset(aucBuff,0x00,sizeof(aucBuff));
            UTIL_Form_Montant(aucBuff+strlen((char*)aucBuff),ECTransInfo.uiNBalance,2);
           ui->label->setText(tr(aucBuff));

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

#endif
}



UCHAR ICCWidonw::SelectAccountType(void)
{
    UCHAR k;
    char y,i,ch;
    char str[5],tempbuf[32];
    memset(str,0,5);
    str[0]=0x2A;//'*'
    k=18;

#if 0
    ui->label->setText("Select Account");


    OSMMI_DisplayASCII(0x30,2,0,(PUCHAR)"1. Default");
    OSMMI_DisplayASCII(0x30,3,0,(PUCHAR)"2. Saving");
    OSMMI_DisplayASCII(0x30,4,0,(PUCHAR)"3. Debit");
    OSMMI_DisplayASCII(0x30,5,0,(PUCHAR)"4. Credit");

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
    if(y<6)
        OSMMI_DisplayASCII(0x30,y,k,(PUCHAR)str);

    while (1) {
        ch = Os__xget_key();
        switch (ch){
            case '1':
                ConstParam.ucAccountType=0x00;
                OSMMI_DisplayASCII(0x30,y,k,(PUCHAR)" ");
                OSMMI_DisplayASCII(0x30,2,k,(PUCHAR)str);
                y=2;
                break;
            case '2':
                ConstParam.ucAccountType=0x10;
                OSMMI_DisplayASCII(0x30,y,k,(PUCHAR)" ");
                OSMMI_DisplayASCII(0x30,3,k,(PUCHAR)str);
                y=3;
                break;
            case '3':
                ConstParam.ucAccountType=0x20;
                OSMMI_DisplayASCII(0x30,y,k,(PUCHAR)" ");
                OSMMI_DisplayASCII(0x30,4,k,(PUCHAR)str);
                y=4;
                break;
            case '4':
                ConstParam.ucAccountType=0x30;
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
#endif
    return ch;
}


