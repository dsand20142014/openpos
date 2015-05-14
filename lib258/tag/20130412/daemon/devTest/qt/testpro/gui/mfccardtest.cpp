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

        //qDebug()<<"00000000 runflagss========="<<runflagss;

            unsigned int rec = Os_Wait_Event(KEY_DRV|cardType , &new_drv, 500);

            if(rec == 0x00){

                //qDebug()<<"111111111 runflagss========="<<runflagss;
                if(!runflagss)  return;

                if(new_drv.drv_type == cardType&&runflagss)
                {
                    //qDebug()<<"2222222 runflagss========="<<runflagss;

                    QMutex mutex;
                    QMutexLocker locker(&mutex);

                    //qDebug()<<"333333 runflagss========="<<runflagss;

                    emit signalCard();
                    //qDebug()<<"***********emit card signal*************";


                    //qDebug()<<"444444444 runflagss========="<<runflagss;

                    waitCon.wait(locker.mutex());

                }
                else if(new_drv.drv_type == KEY_DRV &&runflagss)
                {
//                    if(new_drv.drv_data.xxdata[1]==KEY_CLEAR)
//                        emit signalExit();
                }
                else if(new_drv.drv_type == DRV_TIMEOUT&&runflagss)
                {
//                    emit signalFresh();
                }

            }

    }
    //qDebug()<<"END THREAD*******"<<runflagss;
}


void CardThread::wakeUp()
{
    waitCon.wakeAll();
}

void CardThread::changeFlag()
{
    //qDebug()<<"change flag =========="<<runflagss;
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
//    connect(thread,SIGNAL(signalFresh()),textEdit,SLOT(clear()));

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
    ////qDebug()<<"IN MFC GETRESULT****";

    Os__MIFARE_Request(0,0x26);
    ////qDebug()<<"11111 IN MFC GETRESULT****";

    ucResult = OSMIFARE_Anti(1,aucCardId,&ucCardLen,&ucCardClass);
    ////qDebug()<<"222 IN MFC GETRESULT****"<<ucResult;

    if (!ucResult){
        ////qDebug()<<"333 IN MFC GETRESULT****";
        memset(aucCardIdDis,0,sizeof(aucCardIdDis));
        ////qDebug()<<"444 IN MFC GETRESULT****";

        hex_str(aucCardIdDis,aucCardId,ucCardLen*2);
//        qDebug()<<"555 IN MFC GETRESULT****";

        textEdit->setText((char*)aucCardIdDis);
         Os__beep();
//       qDebug()<<"666 IN MFC GETRESULT****";

    }

    QTimer::singleShot(20,this,SLOT(slotRemove()));

}

void MFCCardTest::slotRemove()
{
    OSMIFARE_Remove();
    textEdit->clear();
    thread->wakeUp();
//    qDebug()<<"END IN MFC GETRESULT****";

}

void MFCCardTest::slotExit()
{
    _print("MFCCardTest********slotExit\n");

    thread->changeFlag();

    close();
}

//void MFCCardTest::keyPressEvent(QKeyEvent *e)
//{
//        switch(e->key())
//        {
//            case Qt::Key_Escape:
//                  slotExit();
//            break;
//        }
//}


