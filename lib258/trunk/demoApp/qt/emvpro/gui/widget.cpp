#include "widget.h"
#include "ui_widget.h"

#include "new_drv.h"
#include "test/smart.h"

#include "modules.h"
#include "oslib.h"
#include "test/Global.h"
#include "test/MasApp.h"

UCHAR 	MagTrack2[40];
extern unsigned char OnEve_power_on(void);

Widget::Widget(QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QColor("gray")));
    setPalette(palette);
    setAutoFillBackground(true);

    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget->setFocus();

    bFallBack = FALSE;
    OnEve_power_on();
    connect(&timer,SIGNAL(timeout()),this,SLOT(getResult()));
    timer.start(300);

}

Widget::~Widget()
{
    delete ui;
}

unsigned char Widget::getResult(void)
{

    timer.stop();
    unsigned int  uiTimeout;
    unsigned char ucMenuPage;
    unsigned char ucCount;
    unsigned char ucKey;
    unsigned char ucResult;

    unsigned char Date[7],Time[5],Date_Time[17],display[17];
    unsigned char aucCardId[16],ucCardClass,ucLen,aucDate[20],ucICCardType;
    printf("~~~~~~~~~~~~~~~IN getResult()**************************\n");

    DRV_OUT *pxMag;
    DRV_OUT *pxIcc;
    DRV_OUT *pxKey;
    DRV_OUT *pxMifare;
    struct seven even;
    union seve_in even_in;
    struct seve_out even_out;

    struct seve_mag_card *pMagEvent;
    struct seve_icc *pIccEvent;

    UCHAR	ucLen1,ucLen2,aucMagBuff[200],aucDateTime[10],aucResetBuf[50],ucResetBufLen,ucAuthRespCode;
    BOOL	bNextPage;
    EMVTRANSTAGDATA *  pRespCode;
    unsigned char ucI,aucBuff[30];

    NEW_DRV_TYPE new_drv;


    if(ConstParam.bPBOC20==FALSE)
    {
        ui->label->setText("EMV Level 2 TEST ");
        ui->label_2->setText("Insert/Swap Card");
    }
    else
    {
        ui->label->setText(tr("   PBOC20 测试 "));
        ui->label_2->setText(tr("请插入IC卡/刷卡"));
    }

    unsigned int ret = Os_Wait_Event(ICC_DRV , &new_drv, 15000);
    if(ret==0){
        Os__light_on();

        if(new_drv.drv_type == ICC_DRV)
        {
            qDebug()<<"22222222";
            pxIcc = &new_drv.drv_data;
            Os__abort_drv(drv_mag);
            Os__abort_drv(drv_mmi);

            ucICCardType =pxIcc->xxdata[1];
            ucResetBufLen=sizeof(aucResetBuf);

            if(ucICCardType==0x39)
                ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);

            qDebug()<<"4444444444ucICCardType=="<<ucICCardType;

            if(ucICCardType!=ICC_ASY)
            {
                ui->label_2->clear();
                if(ConstParam.bPBOC20==FALSE)
                    ui->label->setText("Swap Card");
                else
                    ui->label->setText(tr("请刷磁条卡"));
                Os__xdelay(50);

                bFallBack = TRUE;
                timer.start();
                return 1;
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

            ui->stackedWidget->setCurrentIndex(1);
            ui->lineEdit->setFocus();

            if(ConstParam.bPBOC20==FALSE)
                ui->label_4->setText("Trans Amount:");
            else
                ui->label_4->setText(tr("交易金额:"));

            uiAmount=0;
          //  if(!(ucResult=UTIL_InputAmount(3,&uiAmount,1,99999999999)))
            {
                if(enTransType ==CASHBACK)
                {
                   // Os__clr_display(255);
                    if(ConstParam.bPBOC20==FALSE)
                        Os__GB2312_display(2,0,(PUCHAR)"Cashback Amount:");
                    else
                        Os__GB2312_display(2,0,(PUCHAR)"返现金:");


                    ucResult =UTIL_InputAmount(3,&uiOtherAmount,1,99999999999);
                }
                else
                    uiOtherAmount =0;
                if(!ucResult)
                {
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
    }
    qDebug()<<"end end";


    timer.start();

}

void Widget::keyPressEvent(QKeyEvent *e)
{

}

void Widget::ProcessMag(BOOL bFallBack,unsigned char* bufMag)
{
#if 0
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
        //Os__clr_display(255);
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
            //Os__clr_display(255);

            if(ConstParam.bPBOC20==FALSE)
                Os__GB2312_display(2,0,(PUCHAR)"Use Chip Reader");
            else
                Os__GB2312_display(2,0,(PUCHAR)"请使用芯片卡");

            Os__xget_key();
        }
    }
#endif
}

//#if 0
//{
//if(new_drv.drv_type == MAG_DRV)
//{ qDebug()<<"1111111111";
//#if 0
//    pxMag = new_drv.drv_data;
//    Os__abort_drv(drv_icc);
//    Os__abort_drv(drv_mmi);

//    ucLen1 = pxMag->xxdata[0];
//    ucLen2 = pxMag->xxdata[ucLen1+2];
//    if(ucLen1&&ucLen1<38&& ucLen2<105)
//    {
//        memcpy(aucMagBuff,pxMag,ucLen1+ucLen2+7);
//        ProcessMag(bFallBack,aucMagBuff);
//        if(bFallBack)
//        {
////					if(!TransReqInfo.uiAmount)
//            {
//                //Os__clr_display(255);
//                if(ConstParam.bPBOC20==FALSE)
//                    Os__GB2312_display(2,0,(PUCHAR)"Trans Amount:");
//                else
//                    Os__GB2312_display(2,0,(PUCHAR)"交易金额:");

//                ucResult=UTIL_InputAmount(3,&uiAmount,1,99999999999);
//            }
//            TransReqInfo.enTransType = enTransType;
//            TransReqInfo.uiAmount = uiAmount;

//            if(enTransType == CASHBACK)
//            {
//                TransReqInfo.uiAmount += uiOtherAmount;
//                TransReqInfo.uiOtherAmount = uiOtherAmount;

//            }

//            Os__read_date(aucDateTime);
//            memcpy(TransReqInfo.aucTransDate,"20",2);
//            memcpy(&TransReqInfo.aucTransDate[2],&aucDateTime[4],2);
//            memcpy(&TransReqInfo.aucTransDate[4],&aucDateTime[2],2);
//            memcpy(&TransReqInfo.aucTransDate[6],&aucDateTime[0],2);

//            Os__read_time_sec(TransReqInfo.aucTransTime);

//            TransReqInfo.uiTraceNumber = ConstParam.uiTraceNumber;
//            ConstParam.uiTraceNumber++;
//            UTIL_WriteConstParamFile(&ConstParam);

//            ucResult =MSG_ProcessIccFailBack();
//            if(!ucResult)
//            {
//                pRespCode =EMVTRANSTAG_SearchTag(ALLPHASETAG,(PUCHAR)"\x8A");

//                if(pRespCode&&pRespCode->uiLen==2)
//                {
//                    ucAuthRespCode = CONV_AscLong(pRespCode->pTagValue,pRespCode->uiLen);
//                    //Os__clr_display(255);

//                    if(ucAuthRespCode ==0)
//                    {
//                        ConstParam.uiTotalAmount +=TransReqInfo.uiAmount;
//                        ConstParam.uiTotalNums++;
//                        ucResult = UTIL_WriteConstParamFile(&ConstParam);
//                        if(ConstParam.bPBOC20==FALSE)
//                            Os__GB2312_display(3,0,(PUCHAR)"ONLINE APPROVED");
//                        else
//                            Os__GB2312_display(3,0,(PUCHAR)"联机批准");

//                    }
//                    else
//                    {
//                        if(ConstParam.bPBOC20==FALSE)
//                            Os__GB2312_display(3,0,(PUCHAR)"ONLINE DECLINED");
//                        else
//                            Os__GB2312_display(3,0,(PUCHAR)"联机拒绝");

//                    }
//                    UTIL_GetKey(3);
//                }
//            }

//            bFallBack = FALSE;
//        }

//    }
//#endif
//}
//}
//#endif


//#if 0
//{
//if(new_drv.drv_type == KEY_DRV)
//{ qDebug()<<"keykey";
//#if 0
//    pxKey = new_drv.drv_data;
//    bNextPage =FALSE;
//    Os__abort_drv(drv_mag);
//    Os__abort_drv(drv_icc);

//    switch(pxKey->xxdata[1])
//    {
//    case KEY_F1:
//        //TERMSETTING_Func();
//        break;
//    case KEY_F2:
//       // TERMSETTING_TransParam();
//        break;
//    case KEY_F3:
//        do
//        {
//            //Os__clr_display(255);
//            if(!bNextPage)
//            {
//                if(ConstParam.bPBOC20==FALSE)
//                {
//                    Os__GB2312_display(0,0,"1.Batch Up");
//                    Os__GB2312_display(1,0,"2.Param Update");
//                    Os__GB2312_display(2,0,"3.Trans TVR");
//                }
//                else
//                {
//                    Os__GB2312_display(0,0,"1.批上送");
//                    Os__GB2312_display(1,0,"2.参数更新");
//                    Os__GB2312_display(2,0,"3.终端验证结果");
//                    Os__GB2312_display(3,0,"4.交易日志");

//                }
//            }
//            else
//            {
//                if(ConstParam.bPBOC20==FALSE)
//                {
//                    Os__GB2312_display(0,0,"1.Batch Up");
//                    Os__GB2312_display(1,0,"2.Param Update");
//                    Os__GB2312_display(2,0,"3.Trans TVR");
//                }
//                else
//                {
//                    if(ConstParam.bECSupport)
//                        Os__GB2312_display(0,0,"5.卡片余额");
//                    else
//                    {
//                        Os__GB2312_display(0,0,"1.批上送");
//                        Os__GB2312_display(1,0,"2.参数更新");
//                        Os__GB2312_display(2,0,"3.终端验证结果");
//                        Os__GB2312_display(3,0,"4.交易日志");

//                    }

//                }
//            }

//            ucKey =Os__xget_key();
//            switch(ucKey)
//            {
//            case '1':
//                //MSG_BatchUp();
//                MSG_Reconciliation();
//                break;
//            case '2':
//                MSG_UpdateParam();
//                break;
//            case '3':
//                CheckTransTVR();
//                break;
//            case '4':
//                if(ConstParam.bPBOC20||ConstParam.bECSupport)
//                    ReadICTransLog();
//                break;
//            case '5':
//                if(ConstParam.bECSupport)
//                    ReadICBalance();
//                break;
//            case KEY_F3:
//                bNextPage =bNextPage?FALSE:TRUE;
//                break;
//            default:
//                break;
//            }
//        }while(ucKey != KEY_CLEAR);
//        break;
//#ifdef _PS300
//    case KEY_F4:
//        //Os__clr_display(255);
//        Os__GB2312_display(1,0,(PUCHAR)"Turn Off Confirm");
//        Os__GB2312_display(2,1,(PUCHAR)"1.OK  2.Cancel");
//        if(UTIL_GetKey(3)=='1')
//            OSMOBILE_PowerOff();

//        break;
//#endif
//    case KEY_F4:
//            ucI=0;
//            while(ucI<EMVTransInfo.ucScriptResultNum)
//            {
//                unsigned char ucJ;
//                //Os__clr_display(255);
//                for(ucJ=0;(ucJ<4)&&(ucI<EMVTransInfo.ucScriptResultNum);ucJ++,ucI++)
//                {
//                    memset(aucBuff,0x00,sizeof(aucBuff));
//                    CONV_HexAsc(aucBuff,&EMVTransInfo.ScriptResult[ucI],sizeof(SCRIPTRESULT)*2);
//                    Os__GB2312_display(ucJ,0,aucBuff);


//                }
//                Os__xget_key();
//            }
//            break;
//    default:
//        break;

//    }
//}

//#endif

void Widget::on_pushButton_clicked()
{
    timer.start();
}
