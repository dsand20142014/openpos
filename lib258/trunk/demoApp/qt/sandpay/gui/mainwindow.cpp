#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "oncardui.h"
#include "mainmenudialog.h"

#include "include.h"
#include "global.h"
#include "xdata.h"
#include "sand_main.h"
#include "mainentrythread.h"

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mainEntryThread = new MainEntryThread(argc,argv,this);
    connect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->start();

    OnEve_power_on();
    UTIL_ClearGlobalData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sysEventSlot(int sysEvent)
{
    int i;
    int appnum = 194;
    uchar ucResult=0;

    qDebug()<<"IN  SYSEVENTSLOT********************";
    ((struct seven *)sysEvent)->pt_eve_out->treatment_status = NOK;
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
        qDebug("SIGNAL : OSEVENTTYPE_CIR + OSEVENTID_MAGCARD signal get from APP[%d]", appnum);
        mainEntryThread->wakeUp();
        break;
    case OSEVENTTYPE_CIR + OSEVENTID_ICC:
        qDebug("SIGNAL : OSEVENTTYPE_CIR + OSEVENTID_ICC signal get from APP[%d]", appnum);
        mainEntryThread->wakeUp();
        break;

    case OSEVENTTYPE_SEL+0xff:
    case OSEVENTTYPE_SEL+0xfe:/* 按键进去的界面  */
    {
        qDebug("SIGNAL : OSEVENTTYPE_SEL+0xff signal get from APP[%d]", appnum);

        MainMenuDialog *mainMenu = new MainMenuDialog(this) ;
        connect(mainMenu,SIGNAL(signal()),this,SLOT(on_pushButton_slots()));
        connect(mainMenu,SIGNAL(destroyed()),this,SLOT(on_pushButton_slots()));
        mainMenu->show();

    }
        break;

    case OSEVENTTYPE_CIR + OSEVENTID_MFCCRD:/* 放卡进去的界面  */
        qDebug("SIGNAL hdl: OSEVENTTYPE_SEL+OSEVENTID_MFCCRD signal get from APP[%d]", appnum);
        RunData.ucInputmode=0;

        qDebug("\n 20120626-EXTrans_CheckSandCard-0000 .... [%02x]\n",ucResult);
       // ucResult = EXTrans_CheckSandCard();

        qDebug("\n 20120626-EXTrans_CheckSandCard-0001 .... [%02x]~~~~~~~~~~~~~~~~~~~~~~~~\n",ucResult);
        if(ucResult == 0x00)
        {
            qDebug()<<"on tunch*********123456********************";

            OnCardUI *cardUI = new OnCardUI(this);
            connect(cardUI,SIGNAL(signal_oncard()),this,SLOT(on_pushButton_slots()));
            connect(cardUI,SIGNAL(destroyed()),this,SLOT(on_pushButton_slots()));
            cardUI->show();

            ((struct seven *)sysEvent)->pt_eve_out->treatment_status = OK;
        }

        else on_pushButton_slots();

        break;
    default:
        mainEntryThread->wakeUp();
        break;
    }

}





void MainWindow::on_pushButton_slots()
{
    qDebug()<<"on mainwidow slots********************";

    mainEntryThread->wakeUp();

}






}
