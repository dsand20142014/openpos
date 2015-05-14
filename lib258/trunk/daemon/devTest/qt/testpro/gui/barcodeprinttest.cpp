#include "barcodeprinttest.h"
extern "C"{
#include "Os__barscanner_9600.h"
#include "osdriver.h"
#include "bar.h"
}

BarcodePrintTest::BarcodePrintTest(QWidget *parent) : BaseForm (tr("BARCODE PRINTET"), parent)
{
    label = new QLabel(tr("PLEASE SCAN>>     "), this);
    textEdit = new QTextEdit();
    textEdit->setReadOnly(true);

    layout->addWidget(label);
    layout->addWidget(textEdit);


    btnlayout->addWidget(btnOk);
    btnOk->show();
    btnOk->setFocus();

    Os__bar_open();

    connect(btnOk,SIGNAL(clicked()),textEdit,SLOT(clear()));
    connect(btnOk,SIGNAL(clicked()),&timer,SLOT(start()));

    connect(this, SIGNAL(destroyed()), &timer, SLOT(stop()));
    connect(&timer,SIGNAL(timeout()),SLOT(getResult()));
    timer.start(1000);

}

BarcodePrintTest::~BarcodePrintTest()
{
    timer.stop();
    Os__bar_close();
}

void BarcodePrintTest::getResult()
{
    btnOk->setEnabled(false);

    timer.stop();
    textEdit->clear();
    int ret=0;
    unsigned char buf[200];

    memset(buf,0,sizeof(buf));
    ret=Os__bar_read(buf,sizeof(buf));
    if(ret) 
        textEdit->setText((char *)buf);
    else
       timer.start();
    btnOk->setEnabled(true);

}
