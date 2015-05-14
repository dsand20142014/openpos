#include "ethernettest.h"

extern void _print(char *fmt,...);

EthernetTest::EthernetTest(QWidget *parent) : BaseForm(tr("ETHERNET TEST"), parent)
{
    textEdit = new QTextEdit();
    textEdit->setMinimumHeight(120);
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);

    textEdit->setText("NO DEVICE!");

}

EthernetTest::~EthernetTest()
{

}

