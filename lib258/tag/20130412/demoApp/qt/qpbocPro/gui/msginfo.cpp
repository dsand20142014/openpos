#include "msginfo.h"
#include "ui_msginfo.h"
#include <QtDebug>
#include "include.h"
#include "MasApp.h"
#include "Global.h"
MyThread::MyThread(int flag,QObject *parent )
{
    flags = flag;
    begFlag = true;
    ThreadCloseFlag = true;
}

void MyThread::run()
{
    switch(flags)
    {
    case 1:
        MASAPP_Event();
        break;

    }
     begFlag = false;
}

MsgInfo::MsgInfo(int flag,QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint),
    ui(new Ui::MsgInfo)
{
    ui->setupUi(this);
    QPalette palette;
    palette.setBrush(QPalette::Background,QBrush(QColor("gray")));
    setPalette(palette);
    setAutoFillBackground(true);

    setAttribute(Qt::WA_DeleteOnClose);


    thread = new MyThread(flag,this);
    thread->start();

    connect(&timer,SIGNAL(timeout()),this,SLOT(slot_showMsg()));
    timer.start(100);
}


void MsgInfo::slot_showMsg()
{
    unsigned char result=0;
    update();
    timer.stop();

    result=sem_wait(&binSem1);
    if(result==0)
    {
        ui->label->setText(myString[0]);
        ui->label_2->setText(myString[1]);
        ui->label_3->setText(myString[2]);
        ui->label_4->setText(myString[3]);
        ui->label_5->setText(myString[4]);
        ui->label_6->setText(myString[5]);
        ui->label_7->setText(myString[6]);
        ui->label_8->setText(myString[7]);
    }

    sem_post(&binSem2);

    if(thread->begFlag) timer.start(100);


}

MsgInfo::~MsgInfo()
{
    qDebug()<<"IN ~MSGINFO********";
    if(thread)
    {
//        thread->terminate();
//        thread->wait();
        delete thread;
        thread = 0;
    }
    delete ui;
}

//void MsgInfo::on_pushButton_clicked()
//{
//   // qDebug()<<"endFlag======="<<thread->endFlag;

//    ThreadCloseFlag = false;
//     close();

//}

//void MsgInfo::keyPressEvent(QKeyEvent *e)
//{
//    switch(e->key())
//    {
//        case Qt::Key_Escape:
//        // if(thread->endFlag)
//             close();
//        break;
//    }
//}
