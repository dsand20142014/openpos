#include "msginfo.h"
#include "ui_msginfo.h"
#include <QtDebug>
#include "test/include.h"
#include "test/MasApp.h"
#include "test/Global.h"

#include "iconv.h"

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

//   / ui->pushButton->setFocus();

    thread = new MyThread(flag,this);
    thread->start();

    connect(&timer,SIGNAL(timeout()),this,SLOT(slot_showMsg()));
    timer.start(100);
}

int MsgInfo::gb2312toutf8(char *sourcebuf, size_t sourcelen, char *destbuf, size_t destlen)
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

void MsgInfo::slot_showMsg()
{
    char uBuf[1024];
    unsigned char result=0;
    update();
    timer.stop();

    result=sem_wait(&binSem1);
    if(result==0)
    {
       int rec = gb2312toutf8(myString[5],strlen(myString[5]),uBuf,sizeof(uBuf));
       printf("~~~~~~~~~~rec====%d   uBuf =%s\n",rec,uBuf);
       if(rec==0)
           ui->label_6->setText(uBuf);
       else
           ui->label_6->setText(tr(myString[5]));

        ui->label->setText(tr(myString[0]));
        ui->label_2->setText(tr(myString[1]));
        ui->label_3->setText(tr(myString[2]));
        ui->label_4->setText(tr(myString[3]));
        ui->label_5->setText(tr(myString[4]));
        ui->label_7->setText(tr(myString[6]));
        ui->label_8->setText(tr(myString[7]));
    }
    sem_post(&binSem2);

    if(thread->begFlag) timer.start(100);


}

MsgInfo::~MsgInfo()
{
    qDebug()<<"IN ~MSGINFO********";
    if(thread)
    {
        delete thread;
        thread = 0;
    }
    delete ui;
}


