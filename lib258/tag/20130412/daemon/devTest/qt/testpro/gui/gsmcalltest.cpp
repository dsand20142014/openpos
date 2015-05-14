#include "gsmcalltest.h"
#include <QtDebug>

extern "C"{
#include "test/test.h"
}

GSMCallTest::GSMCallTest(QWidget *parent) :
    BaseForm(tr("GSM INCOMING CALL"), parent)
{
    count = 0;

    textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);

    textEdit->append(tr("CALL TEST BEGIN"));

    QTimer::singleShot(300,this,SLOT(getResult()));
    connect(&timer,SIGNAL(timeout()),this,SLOT(slot_timer()));
}


GSMCallTest::~GSMCallTest()
{
    OSGSM_hangUp();
    OSGSM_exit();
   // system("/daemon/dial.dm.4.1.4 dbg=0 &");

}


void GSMCallTest::getResult()
{
    unsigned char ucResult;
    int count=0;
    unsigned char buf[30],cmd[50],rsp[50];


//    system("/etc/ppp/ppp-off > /dev/null 2>1&");
//    system("pkill dial");
//    sleep(2);

    ucResult=OSGSM_init();

    if(ucResult!=0x00)
    {
        textEdit->append(tr("GSM INI FAILED"));
        return;
    }

    memset(buf,0,sizeof(buf));
    OSGSM_ATCmd((uchar*)"AT", buf, 3000);
    textEdit->append(QString((char*)buf));

    timer.start(100);

}


void GSMCallTest::slot_timer()
{
    timer.stop();
    unsigned char result=0x30;
    QString c;
    char text[30];

    textEdit->append(tr("WAITING"));

    result = OSGSM_answerCalling();

//    textEdit->append(text);

    if(result==0x00){
        QString temp;
        temp.sprintf("OK COUNTING: %d",++count);
        textEdit->append(temp);
    }

    timer.start(2000);
}




