#include "pro_authfinishmsg.h"
#include "ui_pro_authfinishmsg.h"
#include "msginfo.h"

#include "mythread.h"
#include "include.h"
#include "global.h"
#include "toolslib.h"

#include "osevent.h"
#include "sand_main.h"

extern MainEntryThread *mainEntryThread;

Pro_AuthFinishMsg::Pro_AuthFinishMsg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Pro_AuthFinishMsg)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    sandtimer = new QTimer();
    connect(sandtimer,SIGNAL(timeout()),this,SLOT(slottimer1GetCardInfo()));
    sandtimer->start( GUI_GETCARD_TIMES );

    connect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    g_ProgressBarFlag = 1;
}
Pro_AuthFinishMsg::~Pro_AuthFinishMsg()
{
    delete ui;
}
void Pro_AuthFinishMsg::on_cancelButton_clicked()
{
    disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    sandtimer->stop();
    destroy();
}

void Pro_AuthFinishMsg::sysEventSlot(int sysEvent)
{

    //     * 增加原来void manager(struct seven *EventInfo)里面的代码
    //     * 参数也是指针，在使用时需要进行强制类型转换
    qDebug("the slot is runningt");

    int i;
    int appnum = 39;
    //    unsigned char ucKey;
    //    unsigned int ret;
    //    NEW_DRV_TYPE  new_drv;
    unsigned char ucKey;
    unsigned char EventType;
    unsigned char EventNum;

    seven *EventInfo;
    unsigned char str[6];
    unsigned char ucResult=0;

    //    seven *EventInfo;
    ((struct seven *)sysEvent)->pt_eve_out->treatment_status = NOK;

    UTIL_ClearGlobalData();
    UTIL_GetTerminalInfo();

//        qDebug("??????????????????????????????????????%d",(((struct seven *)sysEvent)->event_nr + ((struct seven *)sysEvent)->event_type));
    switch ((((struct seven *)sysEvent)->event_nr + ((struct seven *)sysEvent)->event_type))
    {
    case OSEVENTTYPE_COM + OSEVENTID_POWERON:
        qDebug("SIGNAL : OSEVENTTYPE_COM + OSEVENTID_POWERON signal get from APP[%d]", appnum);
        mainEntryThread->wakeUp();
        break;
    case OSEVENTTYPE_SEL + 39:
        qDebug("SIGNAL : OSEVENTTYPE_SEL + 39 signal get from APP[%d]", appnum);
        mainEntryThread->wakeUp();
        break;
    case OSEVENTTYPE_CIR + OSEVENTID_CUSTOMER:
        qDebug("SIGNAL : OSEVENTTYPE_CIR + OSEVENTID_CUSTOMER signal get from APP[%d]", appnum);
        mainEntryThread->wakeUp();
        break;
    case OSEVENTTYPE_CIR + OSEVENTID_MASTERAPPLI:
        qDebug("SIGNAL : OSEVENTTYPE_CIR + OSEVENTID_MASTERAPPLI signal get from APP[%d]", appnum);
        mainEntryThread->wakeUp();
        break;

    case OSEVENTTYPE_CIR + OSEVENTID_MAGCARD:
        ((struct seven *)sysEvent)->pt_eve_out->treatment_status = OK;
        disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
        mainEntryThread->wakeUp();
        //        if(!mainMenuEntryFlag)
//        {
            qDebug("SIGNAL : OSEVENTTYPE_CIR + OSEVENTID_MAGCARD signal get from APP[%d]", appnum);
            //        if(new_drv.drv_data.gen_status  == DRV_ENDED){
            //ret = OWE_GetTrackInfo((unsigned char *)&new_drv.drv_data);
            OWE_GetIsoTrack(&(((struct seven *)sysEvent)->pt_eve_in->e_mag_card.mag_card[0]));
            if(ProUiFace.UiToPro.CardInfo.uiLenIso2 >= 16)
                ProUiFace.UiToPro.CardInfo.ucOweFlag= 0xAA;
            //获得卡号
            memset(ProUiFace.UiToPro.CardInfo.aucCardAcc,0x00, sizeof(ProUiFace.UiToPro.CardInfo.aucCardAcc));
            //memcpy(ProUiFace.UiToPro.CardInfo.aucCardAcc, (unsigned char *)&new_drv.drv_data, sizeof(NEW_DRV_TYPE));
            memcpy(ProUiFace.UiToPro.CardInfo.aucCardAcc, &(((struct seven *)sysEvent)->pt_eve_in->e_mag_card.mag_card[0]), sizeof(NEW_DRV_TYPE));
            ProUiFace.UiToPro.CardInfo.ucInputMode=TRANS_INPUTMODE_SWIPECARD;
            //        }
////            switch(ProUiFace.UiToPro.CardInfo.ucInputMode)
////            {
//            case TRANS_INPUTMODE_SWIPECARD:
                sandtimer->stop();
                qDebug("1. aucCardAcc:%s.",ProUiFace.UiToPro.CardInfo.aucMagInfo);
                ui->line0_card->setText( (char*)ProUiFace.UiToPro.CardInfo.aucMagInfo);
                ui->line1_date->setFocus();
//                break;
//            case TRANS_INPUTMODE_INSERTCARD:
//                ucResult=EXTrans_ProHead_Ic(EventInfo);
//                //ui->cb_elecCash->enabled = true;
//                ui->checkBox->setEnabled(true);
//                ui->line0_card->setText( (char*)"Card Trans IC>>>");
//                break;
//            case TRANS_INPUTMODE_SAND_PUTCARD:
//                //ui->cb_elecCash->enabled = true;
//                ui->checkBox->setEnabled(true);
//                ui->line0_card->setText( (char*)"Card Trans MFC>>>");
//                break;
//            }
//        }
        break;
    case OSEVENTTYPE_CIR + OSEVENTID_ICC:
        qDebug("SIGNAL : OSEVENTTYPE_CIR + OSEVENTID_ICC signal get from APP[%d]", appnum);
        mainEntryThread->wakeUp();
        break;
    case OSEVENTTYPE_SEL+0xff:
//    case OSEVENTTYPE_SEL + 0xFE:

        //qDebug("SIGNAL : OSEVENTTYPE_SEL+0xff signal get from APP[%d]", appnum);
        //break;
        mainEntryThread->wakeUp();
        break;
    case OSEVENTTYPE_CIR + OSEVENTID_MFCCRD:
        qDebug("SIGNAL : OSEVENTTYPE_CIR + OSEVENTID_MFCCRD signal get from APP[%d]", appnum);
        //        ((struct seven *)sysEvent)->pt_eve_out->treatment_status = OK;
        //        for(i = 0; i < 10; i++)
        //            qDebug("%02x", ((struct seven *)sysEvent)->pt_eve_in->e_icc.icc_ATR[i]);
        mainEntryThread->wakeUp();
        break;

    default:
        mainEntryThread->wakeUp();
        break;
    }

}


void Pro_AuthFinishMsg::slottimer1GetCardInfo()
{
    //时钟显示
    unsigned char str[6];
    memset(str,0,sizeof(str));
    str[0]='[';
    int_str(&str[1],2, &ProUiFace.uiTimes);
    str[3]=']';


    ProUiFace.uiTimes--;

    ui->label_times->setText(( char*)&str);
    if(ProUiFace.uiTimes == 0)
    {
        sandtimer->stop();
        destroy();
    }
}
void Pro_AuthFinishMsg::on_confirmButton_clicked()
{
    int uiLen=0;



    disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    sandtimer->stop();

    //密码
    uiLen = strlen(ui->line4_pwd->text().toAscii());
    if((uiLen>0)&&(uiLen<13)){
        ProUiFace.UiToPro.uiPwLen= uiLen;
        memset(ProUiFace.UiToPro.aucPasswd,0,24);
        memcpy(ProUiFace.UiToPro.aucPasswd, ui->line4_pwd->text().toAscii(), uiLen);
    }
    //授权金额
    ProUiFace.UiToPro.ulAmount = ui->line3_amt->text().toInt(0,10);

    //授权日期
    uiLen = strlen(ui->line1_date->text().toAscii());
    if((uiLen>0)&&(uiLen<5)){
        memset(ProUiFace.UiToPro.aucTransDate,0,4);
        memcpy(ProUiFace.UiToPro.aucTransDate, ui->line1_date->text().toAscii(), uiLen);
    }
    //授权码
    uiLen = strlen(ui->line2_authnum->text().toAscii());
    if((uiLen>0)&&(uiLen<7)){
        memset(ProUiFace.UiToPro.aucAuthNo,0,6);
        memcpy(ProUiFace.UiToPro.aucAuthNo, ui->line2_authnum->text().toAscii(), uiLen);
    }


    qDebug("1.mainkey7...[%d]", ProUiFace.UiToPro.ulAmount);
    qDebug("2.mainkey7...[%d][%s]",ProUiFace.UiToPro.uiPwLen, ProUiFace.UiToPro.aucPasswd);
    //交易类型
    ProUiFace.UiToPro.ucTransType = TRANS_PREAUTHSETTLE;
    ProUiFace.UiToPro.CardInfo.ucInputMode= TRANS_INPUTMODE_SWIPECARD;

    MsgInfo *desk=new MsgInfo();
    desk->show();
    desk->showFullScreen();
    // 启动线程 处理交易
//    MyThread *a=new MyThread();
//    a->start();
}


}
