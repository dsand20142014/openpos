#include "mfccardtest.h"
#include <QtDebug>
#include "QMutexLocker"

extern "C"{
#include "oscfg_param.h"
#include "osmifare.h"
#include "toolslib.h"
#include "oslib.h"
#include "ostools.h"
#include "test/test.h"
}

/*
#define DRV_TIMEOUT     (1<<0) 1
#define KEY_DRV         (1<<1) 2
#define MAG_DRV         (1<<2) 4
#define MFR_DRV         (1<<3) 8
#define ICC_DRV         (1<<4) 16
*/

extern void _print(char *fmt,...);

CardThread::CardThread(int cardtype,QObject *parent )
    :QThread(parent)
{
    //qDebug()<<"begin CardThread =========";

    runflagss = true;
    count = 0;
    cardType = cardtype;
    memset(&new_drv,0,sizeof(NEW_DRV_TYPE));
}

void CardThread::run()
{
    _print("thread start************\n");

    while(runflagss){

            unsigned int rec = Os_Wait_Event(KEY_DRV|cardType , &new_drv, 500);

            if(rec == 0){

                if(!runflagss)  return;

                if(new_drv.drv_type == cardType&&runflagss)
                {
                    QMutex mutex;
                    QMutexLocker locker(&mutex);

                    emit signalCard();

                    waitCon.wait(locker.mutex());

                }
                else if(new_drv.drv_type == KEY_DRV &&runflagss)
                {

                }
                else if(new_drv.drv_type == DRV_TIMEOUT&&runflagss)
                {
                }

            }

    }
}


void CardThread::wakeUp()
{
    waitCon.wakeAll();
}

void CardThread::changeFlag()
{
    runflagss = false;
    wakeUp();
}


MFCCardTest::MFCCardTest(QWidget *parent) :
    BaseForm(tr("MFC CARD TEST"), parent)
{
    textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);
    textEdit->setTextColor(QColor(0,0,255));

    btnExit->setFocus();
    cc =0;

    thread = new CardThread(MFR_DRV,this);
    thread->start();

    connect(thread,SIGNAL(signalCard()),this,SLOT(getResult()));
    connect(thread,SIGNAL(signalExit()),this,SLOT(slotExit()));

    connect(btnExit,SIGNAL(clicked()),this,SLOT(slotExit()));

}



MFCCardTest::~MFCCardTest()
{
    thread->quit();
    thread->wait();
    thread->deleteLater();
}

void MFCCardTest::getResult()
{
    textEdit->clear();

    unsigned char ucResult;
    unsigned char ucCardLen;
    unsigned char ucCardClass;
    unsigned char aucCardId[64];
    unsigned char aucCardIdDis[128];

    Os__MIFARE_Request(0,0x26);

    ucResult = OSMIFARE_Anti(1,aucCardId,&ucCardLen,&ucCardClass);

    if (!ucResult){
        memset(aucCardIdDis,0,sizeof(aucCardIdDis));

        hex_str(aucCardIdDis,aucCardId,ucCardLen*2);

        textEdit->setText((char*)aucCardIdDis);
         Os__beep();

    }

    QTimer::singleShot(20,this,SLOT(slotRemove()));

}

void MFCCardTest::slotRemove()
{
    OSMIFARE_Remove();
    textEdit->clear();
    thread->wakeUp();

}

void MFCCardTest::slotExit()
{
    thread->changeFlag();

    close();
}


