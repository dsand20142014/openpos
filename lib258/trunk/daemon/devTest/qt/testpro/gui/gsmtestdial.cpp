#include "gsmtestdial.h"
extern "C"{
#include "sand_debug.h"
#include "test/test.h"
#include "gsm.h"
}
extern void _print(char *fmt,...);

GsmTestDial::GsmTestDial(QWidget *parent) :
    BaseForm(tr("GSM DIAL TEST"),parent)

{
    lineEdit = new QLineEdit;
    lineEdit->setMaxLength(15);

    textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    layout->addWidget(lineEdit);
    layout->addWidget(textEdit);

    lineEdit->setFocus();

//    system("/etc/ppp/ppp-off > /dev/null 2>1&");
//    system("pkill dial");

    connect(lineEdit,SIGNAL(returnPressed()),this,SLOT(getResult()));

}

void GsmTestDial::showInfo()
{
    if(strstr((char*)buf,"OK"))
    {
        _print("okok!!\n");
        memset(buf,0,sizeof(buf));
//        OSGSM_ATCmd((uchar*)"ATH\r", buf, 3000);
        GSM_AT_cmd("ATH\r",buf,sizeof(buf),1);

    }

    if(strstr((char*)buf,"NO CARRIER"))
        ;


    textEdit->append("");
    textEdit->append(tr("TEST FINISH"));

}


GsmTestDial::~GsmTestDial()
{
    OSGSM_hangUp();
//    OSGSM_exit();
//    system("/daemon/dial.dm.4.1.4 dbg=0 &");
}


void GsmTestDial::getResult()
{
    textEdit->append(tr("CALLING %1").arg(lineEdit->text()));


    //qDebug()<<"textEdit->isHidden()****"<<textEdit->isHidden();

    unsigned char ucResult;
    int count=0,csq=0;
    unsigned char cmd[50],rsp[50];


//    ucResult=OSGSM_init();

//    if(ucResult!=0x00)
//    {
//      textEdit->append(tr("GSM INI FAILED\n"));
//      return;
//    }

    memset(buf,0,sizeof(buf));

    QString str = lineEdit->text();
    str.sprintf("ATD%s;\r",str.toAscii().data());
    //qDebug()<<"str====="<<str;

    GSM_AT_cmd(str.toAscii().data(),buf,sizeof(buf),1);
//    OSGSM_ATCmd((uchar*)str.toAscii().data(), buf, 3000);
    textEdit->append("MODULE RECV: ["+QString((char*)buf)+"]");

    textEdit->append("");

    if(strlen((char*)buf)==0)
        textEdit->append(tr("TEST FAILED!"));

    QTimer::singleShot(5000,this,SLOT(showInfo()));


}



