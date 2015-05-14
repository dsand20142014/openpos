#include "pro_debit.h"
#include "ui_pro_debit.h"
#include "msginfo.h"
#include <QDebug>
#include <QtCore/qmath.h>
#include <QKeyEvent>
#include <QMessageBox>
#include <QWSServer>
#include "mythread.h"
#include "frame.h"
#include "mainentrythread.h"
#include "include.h"
#include "global.h"
#include "toolslib.h"
//#define QT_KEYPAD_NAVIGATION
//#include "mainwindow.h"

#include "osevent.h"
#include "sand_main.h"

extern MainEntryThread *mainEntryThread;


Pro_Debit::Pro_Debit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Pro_Debit)
{
	ui->setupUi(this);
    pwdStr="";
    pwdCnt=0;
    xingCnt=0;
    pwdNum=0;
    amt=0;
    amtStr="";
    amtCnt=0;
    amtTmp=0.0;
//    ui->line1_amt->setText("0.00");
	setWindowFlags(Qt::FramelessWindowHint);

    sandtimer = new QTimer();
    connect(sandtimer,SIGNAL(timeout()),this,SLOT(slottimer1GetCardInfo()));
    sandtimer->start( GUI_GETCARD_TIMES );
	ProUiFace.uiTimes=60;
	//ui->cb_elecCash->enabled = false;
	ui->checkBox->setEnabled(false);

    ui->line1_amt->installEventFilter(this);
    ui->line2_pwd->installEventFilter(this);
    ui->line1_receiveCursor->hide();
    connect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    reswipeFlag=0;
    g_ProgressBarFlag = 1;
}

void Pro_Debit::sysEventSlot(int sysEvent)
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
        if(reswipeFlag!=1)
        {
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
            qDebug("@@@@@@@@@@@@@@@@@@@@@@@@@@@%s",ProUiFace.UiToPro.CardInfo.aucCardAcc);
            qDebug("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s",((struct seven *)sysEvent)->pt_eve_in->e_mag_card.mag_card);
            ProUiFace.UiToPro.CardInfo.ucInputMode=TRANS_INPUTMODE_SWIPECARD;
            //        }
//            switch(ProUiFace.UiToPro.CardInfo.ucInputMode)
//            {
//            case TRANS_INPUTMODE_SWIPECARD:
                sandtimer->stop();
                qDebug("1. aucCardAcc:%s.",ProUiFace.UiToPro.CardInfo.aucMagInfo);
                ui->line0_card->setText( (char*)ProUiFace.UiToPro.CardInfo.aucMagInfo);
                ui->line1_amt->setFocus();
                ui->line1_amt->setText("0.00");
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
            reswipeFlag=1;
            mainEntryThread->wakeUp();
            qDebug("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW  ");
        }


        break;
    case OSEVENTTYPE_CIR + OSEVENTID_ICC:
//        ((struct seven *)sysEvent)->pt_eve_out->treatment_status = OK;
//        disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
//        qDebug("SIGNAL : OSEVENTTYPE_CIR + OSEVENTID_ICC signal get from APP[%d]", appnum);
//        ucResult=EXTrans_ProHead_Ic(((struct seven *)sysEvent));
//        //ui->cb_elecCash->enabled = true;
//        ui->checkBox->setEnabled(true);
//        ui->line0_card->setText( (char*)"Card Trans IC>>>");
        mainEntryThread->wakeUp();
        break;
    case OSEVENTTYPE_SEL+0xff:
//    case OSEVENTTYPE_SEL + 0xFE:
        mainEntryThread->wakeUp();
        break;
    case OSEVENTTYPE_CIR + OSEVENTID_MFCCRD:
//        ((struct seven *)sysEvent)->pt_eve_out->treatment_status = OK;
//        disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
//        qDebug("SIGNAL : OSEVENTTYPE_CIR + OSEVENTID_MFCCRD signal get from APP[%d]", appnum);
//        //ui->cb_elecCash->enabled = true;
//        ui->checkBox->setEnabled(true);
//        ui->line0_card->setText( (char*)"Card Trans MFC>>>");
        mainEntryThread->wakeUp();
        break;

    default:
        mainEntryThread->wakeUp();
        break;
    }

}

Pro_Debit::~Pro_Debit()
{
    delete ui;
}

void Pro_Debit::slottimer1GetCardInfo()
{
	seven *EventInfo;
	unsigned char str[6];
	unsigned char ucResult=0;

	memset(str,0,sizeof(str));
	str[0]='[';
	int_str(&str[1],2, &ProUiFace.uiTimes);
	str[3]=']';

	ProUiFace.uiTimes--;

	ui->label_times->setText(( char*)&str);
	if(ProUiFace.uiTimes == 0) {
		sandtimer->stop();
		destroy();
	}
	
}

void Pro_Debit::destroyWindow(int)
{

//    a->terminate();
    qDebug("5555555555555555555555555555555555555555555");
    memset((uchar*)&ProUiFace.OweDrv,0x00,sizeof(ProUiFace.OweDrv));
    memset((uchar*)&ProUiFace.ProToUi,0x00,sizeof(ProUiFace.ProToUi));
    memset((uchar*)&ProUiFace.UiToPro,0x00,sizeof(ProUiFace.UiToPro));
    destroy();
}
void Pro_Debit::on_confirmButton_clicked()
{
    //显示 UI界面 提示： 刷卡 输入金额 输入密码
    //给全局变量赋值
    //调用线程，处理
    //ui显示线程处理过程
    //
    disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    sandtimer->stop();

    //密码
//    ProUiFace.UiToPro.uiPwLen = strlen(ui->line2_pwd->text().toAscii());
//    memset(ProUiFace.UiToPro.aucPasswd,0,24);
//    memcpy(ProUiFace.UiToPro.aucPasswd,
//           ui->line2_pwd->text().toAscii(),
//           ProUiFace.UiToPro.uiPwLen);
    ProUiFace.UiToPro.uiPwLen = strlen(pwdStr.toAscii());
    memset(ProUiFace.UiToPro.aucPasswd,0,24);
    memcpy(ProUiFace.UiToPro.aucPasswd,
               pwdStr.toAscii(),
               ProUiFace.UiToPro.uiPwLen);
    //memcpy(ProUiFace.UiToPro.aucPasswd,"123456", iLen);
    //金额
//    ProUiFace.UiToPro.ulAmount = ui->line1_amt->text().toInt(0,10);
    //ProUiFace.UiToPro.ulAmount = 1;
    ProUiFace.UiToPro.ulAmount = amtStr.toUInt(0,10);

    qDebug("1.mainkey7...[%d]", ProUiFace.UiToPro.ulAmount);
    qDebug("2.mainkey7...[%d][%s]",ProUiFace.UiToPro.uiPwLen, ProUiFace.UiToPro.aucPasswd);
    //交易类型
    ProUiFace.UiToPro.ucTransType = TRANS_PURCHASE;
    //ProUiFace.UiToPro.CardInfo.ucInputMode= TRANS_INPUTMODE_SWIPECARD;

    if(ui->checkBox->checkState() == true)
        ProUiFace.UiToPro.ucEPTrans = 0x01;

    MsgInfo *desk=new MsgInfo();
    connect(desk,SIGNAL(returnGrandFartherMenu(int)),this,SLOT(destroyWindow(int)));
    desk->show();
    desk->showFullScreen();
    // 启动线程 处理交易
//    MyThread*a=new MyThread();
//    a->start();
}

void Pro_Debit::on_cancelButton_clicked()
{
    disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
       sandtimer->stop();
       memset((uchar*)&ProUiFace.OweDrv,0x00,sizeof(ProUiFace.OweDrv));
       memset((uchar*)&ProUiFace.ProToUi,0x00,sizeof(ProUiFace.ProToUi));
       memset((uchar*)&ProUiFace.UiToPro,0x00,sizeof(ProUiFace.UiToPro));
       destroy();
}

bool Pro_Debit::eventFilter(QObject *target, QEvent *event)
{
     QString str;
    if(target==ui->line1_amt)
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
                ui->line1_amt->setText("0.00");
                break;
            case 1:
                ui->line1_amt->setText("0.0"+amtStr);
                break;
            case 2:
                ui->line1_amt->setText("0."+amtStr);
                break;
            default:
                ui->line1_amt->setText(amtStr.left(amtCnt-2)+"."+amtStr.right(2));
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
    if(target==ui->line2_pwd)
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
                    ui->line2_pwd->setText("");
                    break;
                case 1:
                    ui->line2_pwd->setText("*");
                    break;
                case 2:
                    ui->line2_pwd->setText("**");
                    break;
                case 3:
                    ui->line2_pwd->setText("***");
                    break;
                case 4:
                    ui->line2_pwd->setText("****");
                    break;
                case 5:
                    ui->line2_pwd->setText("*****");
                    break;
                case 6:
                    ui->line2_pwd->setText("******");
                    break;
                case 7:
                    ui->line2_pwd->setText("*******");
                    break;
                case 8:
                    ui->line2_pwd->setText("********");
                    break;
                case 9:
                    ui->line2_pwd->setText("*********");
                    break;
                case 10:
                    ui->line2_pwd->setText("**********");
                    break;
                case 11:
                    ui->line2_pwd->setText("***********");
                    break;
                case 12:
                    ui->line2_pwd->setText("************");
                    break;
                }
                return true;

//            }
        }

    }
    return QDialog::eventFilter(target,event);
}




void Pro_Debit::on_line1_amt_textEdited(const QString &arg1)
{
//    if(!ProUiFace.UiToPro.CardInfo.ucInputMode)
//    {
//        QMessageBox::warning(this,tr("warning"),tr("Please swipe card !"),QMessageBox::Yes);
//    }
}

}
