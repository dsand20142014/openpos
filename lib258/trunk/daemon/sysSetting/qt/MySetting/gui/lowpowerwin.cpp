#include "lowpowerwin.h"

extern "C"{
#include "wifi.h"
}

extern void _print(char *fmt,...);

LowPowerWin::LowPowerWin(QWidget *parent) :
    BaseForm(tr("WIFI SUSPEND MODULE"),parent)
{
    btnOk->setText(tr("SAVE"));
    btnOk->show();


    radio1 = new QRadioButton(tr("   POWER OFF   "));
    radio2 = new QRadioButton(tr("   SUSPEND     "));


    int value = WIFI_get_suspendModel();
    if(value)
        radio1->setChecked(true);
    else
        radio2->setChecked(true);

    QTextEdit *textEdit = new QTextEdit;
    textEdit->setReadOnly(true);

    QVBoxLayout *vbox = new QVBoxLayout;

    vbox->addWidget(radio1);
    vbox->addSpacerItem(new QSpacerItem(50,30));
    vbox->addWidget(radio2);
    vbox->addStretch(1);
    vbox->setContentsMargins(30,30,30,10);

    textEdit->setLayout(vbox);

    layout->addWidget(textEdit);

    connect(btnOk,SIGNAL(clicked()),this,SLOT(slot_btnOK()));
}

LowPowerWin::~LowPowerWin()
{

}


void LowPowerWin::slot_btnOK()//suspend 0
{
    if(radio1->isChecked())
        WIFI_set_suspendModel(1);
    else if(radio2->isChecked())
        WIFI_set_suspendModel(0);

    showInforText(tr("SET SUCCESS!"));
    QTimer::singleShot(2000,this,SLOT(close()));


}
