#include "pro_cancelauth.h"
#include "ui_pro_cancelauth.h"
#include "msginfo.h"
#include <QDebug>
#include <QKeyEvent>
#include "mythread.h"


extern "C"{
#include "include.h"
#include "global.h"
#include "guitools.h"
#include "iconv.h"
#include "osevent.h"
#include "sand_main.h"
}

extern MainEntryThread *mainEntryThread;

Pro_CancelAuth::Pro_CancelAuth(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Pro_CancelAuth)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    sandtimer = new QTimer();
    connect(sandtimer,SIGNAL(timeout()),this,SLOT(slottimer1GetCardInfo()));
    sandtimer->start( GUI_GETCARD_TIMES );
    ProUiFace.uiTimes=60;
    memset((uchar*)&ProUiFace.ProToUi,0x00,sizeof(ProUiFace.ProToUi));
    pwdStr="";
    pwdCnt=0;
    xingCnt=0;
    pwdNum=0;
    amt=0;
    amtStr="";
    amtCnt=0;
    amtTmp=0.0;
    ui->line1_date->installEventFilter(this);
    ui->line2_auth->installEventFilter(this);
    ui->line3_amt->installEventFilter(this);
    ui->line4_pwd->installEventFilter(this);
    connect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    g_ProgressBarFlag = 1;
}

Pro_CancelAuth::~Pro_CancelAuth()
{
    delete ui;
}
void Pro_CancelAuth::destroyWindow(int)
{
//    a->terminate();
    memset((uchar*)&ProUiFace.OweDrv,0x00,sizeof(ProUiFace.OweDrv));
    memset((uchar*)&ProUiFace.ProToUi,0x00,sizeof(ProUiFace.ProToUi));
    memset((uchar*)&ProUiFace.UiToPro,0x00,sizeof(ProUiFace.UiToPro));
    destroy();
}
void Pro_CancelAuth::slottimer1GetCardInfo()
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
void Pro_CancelAuth::on_cancelButton_clicked()
{
    disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    sandtimer->stop();
      memset((uchar*)&ProUiFace.OweDrv,0x00,sizeof(ProUiFace.OweDrv));
      memset((uchar*)&ProUiFace.ProToUi,0x00,sizeof(ProUiFace.ProToUi));
      memset((uchar*)&ProUiFace.UiToPro,0x00,sizeof(ProUiFace.UiToPro));
    destroy();
}
bool Pro_CancelAuth::eventFilter(QObject *target, QEvent *event)
{
     QString str;
     if(target==ui->line1_date)
     {
         if(event->type()==QEvent::KeyPress)
         {
 //            amtCnt=strlen(amtStr.toAscii());

             QKeyEvent*keyEvent=static_cast<QKeyEvent*>(event);
             switch(keyEvent->key())
             {
             case Qt::Key_Enter:
                 focusNextChild();
                 break;
             }
         }
     }
     if(target==ui->line2_auth)
     {
         if(event->type()==QEvent::KeyPress)
         {
 //            amtCnt=strlen(amtStr.toAscii());

             QKeyEvent*keyEvent=static_cast<QKeyEvent*>(event);
             switch(keyEvent->key())
             {
             case Qt::Key_Enter:
                 focusNextChild();
                 ui->line3_amt->setText("0.00");
                 break;
             }
         }
     }
    if(target==ui->line3_amt)
    {
        if(event->type()==QEvent::KeyPress)
        {
//            amtCnt=strlen(amtStr.toAscii());

            QKeyEvent*keyEvent=static_cast<QKeyEvent*>(event);
            switch(keyEvent->key())
            {
            case Qt::Key_Enter:
                focusNextChild();
                break;
            case Qt::Key_Backspace:
                qDebug("the key_back is cheak");
                amtStr=amtStr.left(amtCnt-1);
                break;
            case Qt::Key_0:
                qDebug("the key_0 is cheak");

                if(amtCnt<10)
                {
                    amtStr=amtStr+"0";
                }
                break;
            case Qt::Key_1:

                qDebug("the key_1 is cheak");

                if(amtCnt<10)
                {
                    amtStr=amtStr+"1";
                }
                break;
            case Qt::Key_2:
                qDebug("the key_2 is cheak");
                if(amtCnt<10)
                {
                    amtStr=amtStr+"2";
                }
                break;
            case Qt::Key_3:
                qDebug("the key_3 is cheak");
                if(amtCnt<10)
                {
                    amtStr=amtStr+"3";
                }
                break;
            case Qt::Key_4:
                qDebug("the key_4 is cheak");
                if(amtCnt<10)
                {
                    amtStr=amtStr+"4";
                }
                break;
            case Qt::Key_5:
                if(amtCnt<10)
                {
                    amtStr=amtStr+"5";
                }
                break;
            case Qt::Key_6:
                if(amtCnt<10)
                {
                    amtStr=amtStr+"6";
                }
                break;
            case Qt::Key_7:
                if(amtCnt<10)
                {
                    amtStr=amtStr+"7";
                }
                break;
            case Qt::Key_8:
                if(amtCnt<10)
                {
                    amtStr=amtStr+"8";
                }
                break;
            case Qt::Key_9:
                if(amtCnt<10)
                {
                    amtStr=amtStr+"9";
                }
                break;
            }
            qDebug()<<amtStr;
            amtCnt=amtStr.length();
            switch(amtCnt)
            {
            case 0:
                ui->line3_amt->setText("0.00");
                break;
            case 1:
                ui->line3_amt->setText("0.0"+amtStr);
                break;
            case 2:
                ui->line3_amt->setText("0."+amtStr);
                break;
            default:
                ui->line3_amt->setText(amtStr.left(amtCnt-2)+"."+amtStr.right(2));
                break;
            }

            qDebug()<<amtStr;

            return true;
        }
//        if(event->type()==QEvent::FocusIn)
//        {
////            if(!ProUiFace.UiToPro.CardInfo.ucInputMode)
////            {
////                int r=QMessageBox::warning(this,tr("warning"),tr("Please swipe card !"),QMessageBox::Yes);
////                if(r==QMessageBox::Yes)
////                {
////                    ui->line1_receiveCursor->setFocus();
//////                    QWSServer::setCursorVisible(FALSE);
//////                    ui->line1_amt->setCursor(Qt::BlankCursor);
////                }
////            }
//        }
    }
    if(target==ui->line4_pwd)
    {
        if(event->type()==QEvent::KeyPress)
        {
//            if(pwdCnt>=0 && pwdCnt<=5)
//            {
                QKeyEvent*keyEvent=static_cast<QKeyEvent*>(event);

                switch(keyEvent->key())
                {
                    case Qt::Key_Enter:
                        on_confirmButton_clicked();
                        break;
                    case Qt::Key_Backspace:
                        qDebug("the key_back is cheak");
                        pwdStr=pwdStr.left(pwdStr.length()-1);
//                        qDebug()<<pwdStr;
//                        pwdCnt--;
                        break;
                    case Qt::Key_0:
                        qDebug("the key_0 is cheak");
                        if(pwdCnt<12)
                        {
                            pwdStr+="0";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_1:
                        qDebug("the key_1 is cheak");
                        if(pwdCnt<12)
                        {
                            pwdStr+="1";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_2:
                        if(pwdCnt<12)
                        {
                            pwdStr+="2";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_3:
                        if(pwdCnt<12)
                        {
                            pwdStr+="3";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_4:
                        if(pwdCnt<12)
                        {
                            pwdStr+="4";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_5:
                        if(pwdCnt<12)
                        {
                            pwdStr+="5";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_6:
                        if(pwdCnt<12)
                        {
                            pwdStr+="6";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_7:
                        if(pwdCnt<12)
                        {
                            pwdStr+="7";
                        }
                        pwdCnt++;
                        break;
                    case Qt::Key_8:
                        if(pwdCnt<12)
                        {
                            pwdStr+="8";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_9:
                        if(pwdCnt<12)
                        {
                            pwdStr+="9";
                        }
    //                    pwdCnt++;
                        break;
                }
                pwdCnt=strlen(pwdStr.toAscii());
                switch(pwdCnt)
                {
                case 0:
                    ui->line4_pwd->setText("");
                    break;
                case 1:
                    ui->line4_pwd->setText("*");
                    break;
                case 2:
                    ui->line4_pwd->setText("**");
                    break;
                case 3:
                    ui->line4_pwd->setText("***");
                    break;
                case 4:
                    ui->line4_pwd->setText("****");
                    break;
                case 5:
                    ui->line4_pwd->setText("*****");
                    break;
                case 6:
                    ui->line4_pwd->setText("******");
                    break;
                case 7:
                    ui->line4_pwd->setText("*******");
                    break;
                case 8:
                    ui->line4_pwd->setText("********");
                    break;
                case 9:
                    ui->line4_pwd->setText("*********");
                    break;
                case 10:
                    ui->line4_pwd->setText("**********");
                    break;
                case 11:
                    ui->line4_pwd->setText("***********");
                    break;
                case 12:
                    ui->line4_pwd->setText("************");
                    break;
                }
                return true;
    //            if(keyEvent->key()==Qt::Key_Backspace)
    //            {
    //                qDebug("the filter is cheak");
    //                pwdStr=pwdStr.left(pwdStr.length()-1);
    //                pwdCnt--;
    //                return true;
    //            }
    //            if(event->type()==Qt::Key_0||event->type()==Qt::Key_1
    //                    ||event->type()==Qt::Key_2||event->type()==Qt::Key_3
    //                    ||event->type()==Qt::Key_4||event->type()==Qt::Key_5
    //                    ||event->type()==Qt::Key_6||event->type()==Qt::Key_7
    //                    ||event->type()==Qt::Key_8||event->type()==Qt::Key_9)
    //            {

    //            }
//            }
        }

    }
    return QDialog::eventFilter(target,event);
}


void Pro_CancelAuth::sysEventSlot(int sysEvent)
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


void Pro_CancelAuth::on_confirmButton_clicked()
{
    int uiLen=0;

    disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    sandtimer->stop();

    ProUiFace.UiToPro.ucTransType = TRANS_VOIDPREAUTH;
    ProUiFace.UiToPro.CardInfo.ucInputMode = TRANS_INPUTMODE_SWIPECARD;

//date
    uiLen = strlen(ui->line1_date->text().toAscii());
    if(uiLen){
        memset( ProUiFace.UiToPro.aucTransDate,0,sizeof(ProUiFace.UiToPro.aucTransDate));
        memcpy(ProUiFace.UiToPro.aucTransDate, ui->line1_date->text().toAscii(), uiLen);
    }
//authNum
    uiLen = strlen(ui->line2_auth->text().toAscii());
    if(uiLen){
        memset( ProUiFace.UiToPro.aucAuthNo,0,sizeof(ProUiFace.UiToPro.aucAuthNo));
        memcpy(ProUiFace.UiToPro.aucAuthNo, ui->line2_auth->text().toAscii(), uiLen);
    }
// passwd
    uiLen = strlen(pwdStr.toAscii());
    if(uiLen){
        memset( ProUiFace.UiToPro.aucPasswd,0,sizeof(ProUiFace.UiToPro.aucPasswd));
        memcpy(ProUiFace.UiToPro.aucPasswd, pwdStr.toAscii(), uiLen);
    }
//amount
    ProUiFace.UiToPro.ulAmount = amt;

//    MsgInfo *desk=new MsgInfo();

//    desk->show();
//    desk->showFullScreen();

//    MyThread *a=new MyThread();
//    a->start();

    MsgInfo *desk=new MsgInfo();
    connect(desk,SIGNAL(returnGrandFartherMenu(int)),this,SLOT(destroyWindow(int)));
    desk->exec();
}

}
