#include "asynccardtest.h"
#include <QtDebug>
#include "test/smart.h"

extern "C"{
#include "oslib.h"
#include "sand_debug.h"
#include "libicc.h"
#include "sand_icc.h"
#include "test/test.h"

}

extern ISO7816OUT iso7816out;
extern void _print(char *fmt,...);

AsyncCardTest::AsyncCardTest(QWidget *parent) :
    BaseForm(tr("ASYNCCARD TEST"), parent)
{
    textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);
    textEdit->setTextColor(QColor(0,0,255));

    btnExit->setFocus();

    threadss = new CardThread(ICC_DRV,this);
    threadss->start();

    connect(btnExit,SIGNAL(clicked()),this,SLOT(slotExit()));

    connect(threadss,SIGNAL(signalCard()),this,SLOT(getResult()));
    connect(threadss,SIGNAL(signalExit()),this,SLOT(slotExit()));
}





AsyncCardTest::~AsyncCardTest()
{
    //qDebug()<<"********** IN ~AsyncCardTest()*********";
    threadss->quit();
    threadss->wait();
    threadss->deleteLater();

}

void AsyncCardTest::getResult()
{
    //qDebug()<<"00000000000";

    textEdit->clear();

    unsigned int uilen,uidatalen;
    unsigned char aucBuf[300];

    int i;
    unsigned char ucResult=0x00;


    if(threadss->new_drv.drv_data.xxdata[1]==0x37)
    {
        memset(aucBuf,0,sizeof(aucBuf));
        sprintf((char *)aucBuf,"%02X",threadss->new_drv.drv_data.xxdata[1]);

        textEdit->setText(tr("ERR: RESET RESULT    %1").arg(QString((char*)aucBuf)));
        textEdit->append("");
        textEdit->append(tr("PLEASE REMOVE CARD"));

        QTimer::singleShot(50,this,SLOT(slotRemove()));

        return;
    }
    else if(threadss->new_drv.drv_data.xxdata[1]==0x71||threadss->new_drv.drv_data.xxdata[1]==0x21)
    {
        textEdit->setText(tr("ERROR CARD"));
        textEdit->append("");
        textEdit->append(tr("PLEASE REMOVE CARD"));

        QTimer::singleShot(50,this,SLOT(slotRemove()));


        return;
    }
    else if(threadss->new_drv.drv_data.xxdata[1]==0xff)
    {
        textEdit->setText(tr("NOT ICC CARD"));
        textEdit->append("");
        textEdit->append(tr("PLEASE REMOVE CARD"));
        QTimer::singleShot(50,this,SLOT(slotRemove()));

        return;
    }


    //qDebug()<<"1111111111";

    memset(aucBuf,0x00,sizeof(aucBuf));

    uilen = threadss->new_drv.drv_data.xxdata[2];

    //qDebug()<<"222222222 uilen**********"<<uilen;

    uidatalen = 0;
    for(i=0;i<uilen;i++)
    {
        sprintf((char *)&aucBuf[uidatalen],"%02X",threadss->new_drv.drv_data.xxdata[i+3]);
        uidatalen += 2;
    }
    _print("333333333aucBuf===%s\n",aucBuf);

    textEdit->setText(tr("ANSWER TO RESET:"));
    textEdit->append(QString((char*)aucBuf));



    ucResult = PBOC_selectfile(1, 0, (unsigned char *)"1PAY.SYS.DDF01",14);

    if (ucResult!=0){

        memset(aucBuf,0,sizeof(aucBuf));
        sprintf((char *)aucBuf,"%02X",threadss->new_drv.drv_data.xxdata[1]);

        textEdit->append("");

        textEdit->append(tr("ERROR:SELECT FILE %1").arg(QString((char*)aucBuf)));
        textEdit->append(tr("REMOVE CARD"));

        QTimer::singleShot(50,this,SLOT(slotRemove()));
        return;
    }

    uilen = iso7816out.uiLen;

    memset(aucBuf,0,sizeof(aucBuf));

    uidatalen=0;

    for(i=0;i<iso7816out.uiLen;i++)
    {
        sprintf((char *)&aucBuf[uidatalen],"%02X",iso7816out.aucData[i]);
        uidatalen += 2;
    }

    textEdit->append("");

    textEdit->append(tr("SELECT 1PAY.SYS.DDF01"));
    textEdit->append(QString((char*)aucBuf));

    memset(aucBuf,0,sizeof(aucBuf));


    textEdit->append("");
    textEdit->append(tr("REMOVE CARD"));


    QTimer::singleShot(50,this,SLOT(slotRemove()));

}

void AsyncCardTest::slotRemove()
{
    Os__ICC_remove();
   //while(1){
   //    if(Os__ICC_detect(0))
   //    {
   //        Os__ICC_off();
   //        break;
   //    }
   //    usleep(200000);
   //}

    textEdit->clear();
    threadss->wakeUp();

}


void AsyncCardTest::slotExit()
{
    _print("IN SLOTEXIT*********\n");
    threadss->changeFlag();
    close();
}


//void AsyncCardTest::keyPressEvent(QKeyEvent *e)
//{
//    switch(e->key())
//    {
//    case Qt::Key_Escape:
//        slotExit();
//        break;
//    }
//}


