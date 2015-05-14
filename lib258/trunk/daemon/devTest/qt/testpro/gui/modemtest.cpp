#include "modemtest.h"


ModemTest::ModemTest(QWidget *parent) : BaseForm(tr("MODEM TEST"), parent)
{
    textEdit = new QTextEdit();
    textEdit->setMinimumHeight(120);
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);
    textEdit->setText(tr("NO DEVICE!"));


}

ModemTest::~ModemTest()
{

}


void ModemTest::getResult()
{
}

