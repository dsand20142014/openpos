#include "frame.h"
#include "ui_frame.h"
#include <QPixmap>
//#include <QPainter>
//#include <QMessageBox>
#include <QDebug>
#include <QDialog>
#include "osevent.h"
//#include "toolslib.h"

#include "include.h"
#include "global.h"
#include "osevent.h"
#include "sand_main.h"

MainEntryThread *mainEntryThread;


Frame::Frame(int argc, char *argv[], QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Frame)
{    qDebug("entry frame constructor");
    ui->setupUi(this);
    mainMenuEntryFlag=0;
    qDebug("setup frame uiover");
    lpTimer = new QTimer(this);
    connect(lpTimer, SIGNAL(timeout()), this, SLOT(timeupdate()));
    //    lpTimer->start(GUI_GETCARD_TIMES);
    qDebug("setup time voer");

    mainEntryThread = new MainEntryThread(argc,argv,this);
    connect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->start();
    sleep(5);
    qDebug("setup mainentry voer");

    QPixmap image,imageTmp;
    image.load("/opt/image/unionpay_logo.png");
    imageTmp=image.scaled(150,130,Qt::KeepAspectRatio);
    ui->label_logo->setPixmap(imageTmp);

    installEventFilter(this);
    //    msgReciveThread.start();
    qDebug("frame constructorfinished");
}

//bool Frame::eventFilter(QObject *target, QEvent *event)
//{
//    if(target==this)
//    {
//        if(event->type()==QEvent::KeyPress)
//        {
//            QKeyEvent*keyEvent=static_cast<QKeyEvent*>(event);
//            switch(keyEvent->key())
//            {

//            }
//        }
//    }
//}


void Frame::sysEventSlot(int sysEvent)
{

    //     * 增加原来void manager(struct seven *EventInfo)里面的代码
    //     * 参数也是指针，在使用时需要进行强制类型转换
    qDebug("the slot is runningt");

    int i;
    int appnum = 104;
    //    unsigned char ucKey;
    //    unsigned int ret;
    //    NEW_DRV_TYPE  new_drv;
    unsigned char ucKey;
    unsigned char EventType;
    unsigned char EventNum;
    //    seven *EventInfo;
    ((struct seven *)sysEvent)->pt_eve_out->treatment_status = NOK;

    UTIL_ClearGlobalData();
    UTIL_GetTerminalInfo();

    qDebug("??????????????????????????????????????%d",(((struct seven *)sysEvent)->event_nr + ((struct seven *)sysEvent)->event_type));
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
            CardInfo_Display();
//        }
        break;
    case OSEVENTTYPE_CIR + OSEVENTID_ICC:
        qDebug("SIGNAL : OSEVENTTYPE_CIR + OSEVENTID_ICC signal get from APP[%d]", appnum);
        mainEntryThread->wakeUp();
        break;
    case OSEVENTTYPE_SEL+0xff:
        qDebug("SIGNAL : OSEVENTTYPE_CIR + 0xff signal get from APP[%d]", appnum);
//    case OSEVENTTYPE_SEL + 0xFE:
//        if(!mainMenuEntryFlag)
//        {
            qDebug("key is press");
    //        mainEntryThread->wakeUp();
            mainMenuEntryFlag=1;
            EventType = (unsigned char )((struct seven *)sysEvent)->event_type;
            EventNum  = (unsigned char )((struct seven *)sysEvent)->event_nr;
            if( (EventNum >= '1')&&(EventNum <= '9'))
            {
                ucKey = EventNum;
                EventNum = 0xFE;
            }
            printf("\n ucKey:%02x.", ucKey);
            //if(ucKey == KEY_CANCEL)
            //        lpTimer->stop();
//            mainEntryThread->wakeUp();
            MainMenu_Display();
    //        mainEntryThread->wakeUp();
//        }
        break;
        //qDebug("SIGNAL : OSEVENTTYPE_SEL+0xff signal get from APP[%d]", appnum);
        //break;
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


void Frame::CardInfo_Display(void)
{
    unsigned char i;

    printf("\n 2.1 CardInfo_Display .......");
    disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    lpCardNumDisplay = new CardNumDisplay((QDialog*)this);
    lpCardNumDisplay->grandFartherFlag = 1;
    memset(lpCardNumDisplay->aucISO2DataBack,0, sizeof(lpCardNumDisplay->aucISO2DataBack));
    memcpy(lpCardNumDisplay->aucISO2DataBack, (char*)ProUiFace.UiToPro.CardInfo.aucMagInfo, 37);

//    connect(lpCardNumDisplay,SIGNAL(cardNumDisplayCancel()),this,SLOT(reStartTimer()));
    connect(lpCardNumDisplay,SIGNAL(cardNumDisplayCancel()),this,SLOT(wakeUpMainThread()));

    lpCardNumDisplay->showFullScreen();
    lpCardNumDisplay->exec();
    printf("\n 2.2 CardInfo_Display .......\n");

    //    lpCardNumDisplay->exec();
    //    lpCardNumDisplay->showFullScreen();
}

void Frame::MainMenu_Display(void)
{
    lpMainMenu = new MainMenu(this);
//    connect(lpMainMenu,SIGNAL(mainMenuCancel()),this,SLOT(reStartTimer()));
    disconnect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    connect(lpMainMenu,SIGNAL(returnFromMainMenu()),this,SLOT(wakeUpMainThread()));
    //    lpMainMenu->exec();
    lpMainMenu->setWindowFlags(Qt::Window);
    lpMainMenu->showFullScreen();
    lpMainMenu->exec();
}

void Frame::timeupdate()
{
    //    MainMenu_ReciveMsg();

}

/*
void Frame::closeEvent(QCloseEvent *event)
{
    msgReciveThread.stop();
    msgReciveThread.wait();
    event->accept();
}
*/
/*
void Frame::showEvent(QShowEvent *event)
{
    qDebug("The window is plainted !");
    lpTimer->start(1000);
    event->accept();
}
*/
/*
void Frame::paintEvent(QPaintEvent *event)
{
    lpTimer->start(1000);
    event->accept();
}
*/

void Frame::reStartTimer()
{
    //    lpTimer->start(1000);
}
void Frame::wakeUpMainThread()
{
    qDebug("@#$##########connect_begin##############################################");
    mainMenuEntryFlag=0;
    connect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->wakeUp();
    qDebug("@#$##########connect_end##############################################");
}

Frame::~Frame()
{
    delete ui;
}
/*
void Frame::MainMenu_ReciveMsg(void)
{
    unsigned int ret;
    NEW_DRV_TYPE  new_drv;
    unsigned char ucKey;
    seven *EventInfo;

    printf("\n 1 Fram Os_Wait_Event..........");

    ret = Os_Wait_Event(MAG_DRV | KEY_DRV | ICC_DRV , &new_drv, 10000);
    //    qDebug("===ret:%02x",ret);
    if(ret == 0)
    {
        if(new_drv.drv_type == KEY_DRV)
        {
            //获得按键事件;
            if(new_drv.drv_data.gen_status== DRV_ENDED)
            {
                ucKey=new_drv.drv_data.xxdata[1];
                printf("\n ucKey:%02x.", ucKey);
                //if(ucKey == KEY_CANCEL)
                lpTimer->stop();
                MainMenu_Display();
            }
        }else if(new_drv.drv_type == MAG_DRV)
        {
            //获得磁条卡事件;
            lpTimer->stop();
            printf("\n 2 MainMenu_ReciveMsg MAG_DRV....");

            if(new_drv.drv_data.gen_status  == DRV_ENDED){
                ret = OWE_GetTrackInfo((unsigned char *)&new_drv.drv_data);
                //获得卡号
                memset(ProUiFace.UiToPro.CardInfo.aucCardAcc,0x00, sizeof(ProUiFace.UiToPro.CardInfo.aucCardAcc));
                memcpy(ProUiFace.UiToPro.CardInfo.aucCardAcc, (unsigned char *)&new_drv.drv_data, sizeof(NEW_DRV_TYPE));

                ProUiFace.UiToPro.CardInfo.ucInputMode=TRANS_INPUTMODE_SWIPECARD;
            }
            CardInfo_Display();
        } else if(new_drv.drv_type == MFR_DRV)
        {
            //              非接卡轮询结果处理;
        } else if(new_drv.drv_type == ICC_DRV)
        {
            //              IC卡插卡结果处理;
            printf("\n 2 MainMenu_ReciveMsg ICC_DRV....");
            ProUiFace.UiToPro.CardInfo.ucInputMode=TRANS_INPUTMODE_INSERTCARD;
            EXTrans_ProHead_Ic(EventInfo);

            CardInfo_Display();
        } else if(new_drv.drv_type == DRV_TIMEOUT)
        {
            //              超时处理;
        }
        if(new_drv.drv_type != DRV_TIMEOUT)
            lpTimer->stop();

    }
    else
    {
        OWE_RstErrInfo(ret);
    }
}

*/

}