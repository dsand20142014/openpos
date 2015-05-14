#include "card4442test.h"
#include <QtDebug>

extern "C"
{
#include "fsync_drvs.h"
#include "oslib.h"
#include "sand_icc.h"
#include "test/test.h"
}

extern void _print(char *fmt,...);

Card4442Test::Card4442Test(QWidget *parent) :
    BaseForm(tr("CARD4442 TEST"), parent)
{
    textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setTextColor(QColor(0,0,255));

    layout->addWidget(textEdit);

    btnExit->setFocus();

    thread  = new CardThread(ICC_DRV,this);
    thread->start();

    connect(thread,SIGNAL(signalCard()),this,SLOT(getResult()));
    connect(thread,SIGNAL(signalExit()),this,SLOT(slotExit()));

    connect(btnExit,SIGNAL(clicked()),this,SLOT(slotExit()));

}

Card4442Test::~Card4442Test()
{
    //qDebug()<<"********** IN ~Card4442Test()*********";
    thread->quit();
    thread->wait();
    thread->deleteLater();
}


void Card4442Test::getResult()
{
    //qDebug()<<"111111111111";

    unsigned char ucResult;
    unsigned char aucBuf[300],buf[400];

    unsigned int uidatalen;
    unsigned char i,j;

    ucResult = 0;


    memset(aucBuf,0,sizeof(aucBuf));
    memset(buf,0,sizeof(buf));

    textEdit->clear();

    if(thread->new_drv.drv_data.xxdata[1]==0x35)
    {
        memset(aucBuf,0,sizeof(aucBuf));
        sprintf((char *)aucBuf,"%02X",thread->new_drv.drv_data.xxdata[1]);


        textEdit->setText("ERR: RESET RESULT    "+QString((char *)aucBuf));
        textEdit->append("");
        textEdit->append(tr("REMOVE CARD"));

        QTimer::singleShot(50,this,SLOT(slotRemove()));
        return;
    }
    else if(thread->new_drv.drv_data.xxdata[1]==0x71||thread->new_drv.drv_data.xxdata[1]==0x21)
    {
        textEdit->setText(QString((char*)"ERROR CARD"));
        textEdit->append("");
        textEdit->append(QString((char*)"PLEASE REMOVE CARD"));

        QTimer::singleShot(50,this,SLOT(slotRemove()));

        return;
    }
    else if(thread->new_drv.drv_data.xxdata[1]==0xff)
    {
        textEdit->setText(QString((char*)"NOT ICC CARD"));
        textEdit->append("");
        textEdit->append(QString((char*)"PLEASE REMOVE CARD"));
        QTimer::singleShot(50,this,SLOT(slotRemove()));

        return;
    }

    //qDebug()<<"22222222";

    ucResult = SLE4442_ATR(buf);
    if(ucResult!=OK)
    {
        textEdit->clear();
        QTimer::singleShot(50,this,SLOT(slotRemove()));
        return;
    }

    memset(buf, 0, sizeof(buf));
    int read_error = 0;
    //qDebug()<<"333333333333";

    for (i=0; i<8 && !read_error; i++)
    {
        if (SLE4442_ReadMainMemory(buf+32*i,i*32,32)){

            textEdit->setText(tr("INVALID CARD!!!"));
            textEdit->append(tr("REMOVE CARD"));

            QTimer::singleShot(50,this,SLOT(slotRemove()));
            read_error = 1;
            return;

        }
    }

    if (read_error){
        textEdit->clear();
        QTimer::singleShot(50,this,SLOT(slotRemove()));
        return;
    }
    //qDebug()<<"4444444";


    for(int i =0;i<5;i++)
    {
        if(buf[i] != 0xff)
            break;

        if(i==4 )
        {
            textEdit->setText(QString((char*)"CARD ERROR"));
            QTimer::singleShot(50,this,SLOT(slotRemove()));
            return;
        }

    }

    //qDebug()<<"5555555555";

    memset(aucBuf,0,sizeof(aucBuf));


    uidatalen = 0;
    for(i=0;i<32;i++)
    {
        sprintf((char *)&aucBuf[uidatalen],"%02X",buf[i]);
        uidatalen += 2;
    }

    textEdit->append(tr("FIRST 32 BYTES DATA"));
    textEdit->append(QString((char *)aucBuf));

    textEdit->append("");

    textEdit->append(tr("REMOVE CARD"));

    //qDebug()<<"66666666666";

    QTimer::singleShot(50,this,SLOT(slotRemove()));
}

void Card4442Test::slotRemove()
{
//    Os__ICC_remove();
    while(1){
        if(Os__ICC_detect(0))
        {
            Os__ICC_off();
            break;
        }
          usleep(200000);
    }

    textEdit->clear();
    thread->wakeUp();

}



void Card4442Test::slotExit()
{
    _print("in card 4442 slot exit*********\n");
    thread->changeFlag();
    close();
}

//void Card4442Test::keyPressEvent(QKeyEvent *e)
//{
//    switch(e->key())
//    {
//    case Qt::Key_Escape:
//        slotExit();
//        break;
//    }
//}


