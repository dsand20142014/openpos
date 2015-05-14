#include "pro_cancel.h"
#include "ui_pro_cancel.h"
#include "msginfo.h"
#include "mythread.h"
#include <QKeyEvent>

extern "C"{
#include "include.h"
#include "global.h"
#include "guitools.h"
#include "iconv.h"
#include "osevent.h"
#include "sand_main.h"
}

extern MainEntryThread *mainEntryThread;

Pro_Cancel::Pro_Cancel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Pro_Cancel)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    ucTransOkFlag=0;
    g_ThreadFlag = 0;
    pwdStr="";
    pwdCnt=0;
    xingCnt=0;
    pwdNum=0;
    sandtimer = new QTimer();
    connect(sandtimer,SIGNAL(timeout()),this,SLOT(slottimer1GetCardInfo()));
    sandtimer->start( GUI_GETCARD_TIMES );
    ProUiFace.uiTimes=60;
    memset((uchar*)&ProUiFace.ProToUi,0x00,sizeof(ProUiFace.ProToUi));
    ui->line1_oldTrace->installEventFilter(this);
    ui->line2_pwd->installEventFilter(this);
    connect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    g_ProgressBarFlag = 1;
}

void Pro_Cancel::sysEventSlot(int sysEvent)
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
                ui->line1_oldTrace->setFocus();
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


Pro_Cancel::~Pro_Cancel()
{
    delete ui;
}
void Pro_Cancel::destroyWindow(int)
{
//    a->terminate();
    memset((uchar*)&ProUiFace.OweDrv,0x00,sizeof(ProUiFace.OweDrv));
    memset((uchar*)&ProUiFace.ProToUi,0x00,sizeof(ProUiFace.ProToUi));
    memset((uchar*)&ProUiFace.UiToPro,0x00,sizeof(ProUiFace.UiToPro));
    destroy();
}
bool Pro_Cancel::eventFilter(QObject *target, QEvent *event)
{
     QString str;
     if(target==ui->line1_oldTrace)
     {
         if(event->type()==QEvent::KeyPress)
         {
 //            if(pwdCnt>=0 && pwdCnt<=5)
 //            {
                 QKeyEvent*keyEvent=static_cast<QKeyEvent*>(event);

                 switch(keyEvent->key())
                 {
                     case Qt::Key_Enter:
                         focusNextChild();
                         break;
                 }
         }
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


void Pro_Cancel::slottimer1GetCardInfo()
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

int Pro_Cancel::gb2312toutf8(char *sourcebuf, size_t sourcelen, char *destbuf, size_t destlen)
{
    iconv_t cd;
    if( (cd = iconv_open("utf-8","gb2312")) ==0 )
        return -1;
    memset(destbuf,0,destlen);
    char **source = (char**)&sourcebuf;
    char **dest   = (char**)&destbuf;

    if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))
        return -1;
    iconv_close(cd);
    return 0;
}
void Pro_Cancel::on_confirmButton_clicked()
{
    unsigned int uiTrace,uiLen;
    unsigned char ucResult=0;
    char uBuf[256];

    disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    sandtimer->stop();


    if(ucTransOkFlag == 3){
    qDebug("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq  %d",ucTransOkFlag);
        ProUiFace.UiToPro.ucTransType = TRANS_VOIDPURCHASE;
        ProUiFace.UiToPro.CardInfo.ucInputMode = TRANS_INPUTMODE_SWIPECARD;

        uiLen = strlen(ui->line1_oldTrace->text().toAscii());
        if(uiLen){
            memset( ProUiFace.UiToPro.aucOldTrace,0,sizeof(ProUiFace.UiToPro.aucOldTrace));
            memcpy(ProUiFace.UiToPro.aucOldTrace, ui->line1_oldTrace->text().toAscii(), uiLen);
        }
        uiLen = strlen(pwdStr.toAscii());
        if(uiLen){
            memset( ProUiFace.UiToPro.aucPasswd,0,sizeof(ProUiFace.UiToPro.aucPasswd));
            memcpy(ProUiFace.UiToPro.aucPasswd, pwdStr.toAscii(), uiLen);
        }

        MsgInfo *desk=new MsgInfo();
        connect(desk,SIGNAL(returnGrandFartherMenu(int)),this,SLOT(destroyWindow(int)));
        desk->show();
        desk->showFullScreen();

//        MyThread *a=new MyThread();
//        a->start();
    }else
    {
        uiTrace= ui->line1_oldTrace->text().toInt(0,10);
        int_asc(G_RUNDATA_aucOriginalTraceNum,6,&uiTrace);
        ucResult =  SAV_SavedTransIndex(1);

qDebug(" 2012 07 15 SAV_SavedTransIndex :%02x",ucResult);

        if(ucResult == SUCCESS)
        {
            memset(uBuf,0,sizeof(uBuf));
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine1,strlen((char *)ProUiFace.ProToUi.aucLine1),uBuf,sizeof(uBuf));
            ui->transRst->append(tr(uBuf));
            memset(uBuf,0,sizeof(uBuf));
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine2,strlen((char *)ProUiFace.ProToUi.aucLine2),uBuf,sizeof(uBuf));
            ui->transRst->append(tr(uBuf));
            memset(uBuf,0,sizeof(uBuf));
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine3,strlen((char *)ProUiFace.ProToUi.aucLine3),uBuf,sizeof(uBuf));
            ui->transRst->append(tr(uBuf));
            memset(uBuf,0,sizeof(uBuf));
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine4,strlen((char *)ProUiFace.ProToUi.aucLine4),uBuf,sizeof(uBuf));
            ui->transRst->append(tr(uBuf));
            memset(uBuf,0,sizeof(uBuf));
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine5,strlen((char *)ProUiFace.ProToUi.aucLine5),uBuf,sizeof(uBuf));
            ui->transRst->append(tr(uBuf));
            memset(uBuf,0,sizeof(uBuf));
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine6,strlen((char *)ProUiFace.ProToUi.aucLine6),uBuf,sizeof(uBuf));
            ui->transRst->append(tr(uBuf));
            ucTransOkFlag = 3;
            qDebug("ppppppppppppppppppppppppppppppppppppppp %d",ucTransOkFlag);
        }else
            if(ucResult != SUCCESS){
                ui->transRst->append(tr("ERROR:原交易不存在!"));
            }
    }
}

void Pro_Cancel::on_cancelButton_clicked()
{
    disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    sandtimer->stop();
        memset((uchar*)&ProUiFace.OweDrv,0x00,sizeof(ProUiFace.OweDrv));
        memset((uchar*)&ProUiFace.ProToUi,0x00,sizeof(ProUiFace.ProToUi));
        memset((uchar*)&ProUiFace.UiToPro,0x00,sizeof(ProUiFace.UiToPro));
    destroy();
}

}
