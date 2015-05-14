#include "lowpowerwin.h"

extern "C"{
#include "wifi.h"
}

extern void _print(char *fmt,...);

LowPowerWin::LowPowerWin(QWidget *parent) :
    BaseForm("WIFI SUSPEND MODULE",parent)
{
    _print("wifi suspend\n");

    btnOk->setText("SAVE");
    btnOk->show();


    radio1 = new QRadioButton("   POWER OFF   ");
    radio2 = new QRadioButton("   SUSPEND     ");

    int screenWidth = qApp->desktop()->screen(0)->width();//240
    int screenHeight = qApp->desktop()->screen(0)->height();//320

    int value = WIFI_get_suspendModel();
    _print("~~~~~value==%d\n",value);
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
//    groupBox->setFixedSize(screenWidth-20,screenHeight-20-25-80);

    layout->addWidget(textEdit);

    connect(btnOk,SIGNAL(clicked()),this,SLOT(slot_btnOK()));
}

LowPowerWin::~LowPowerWin()
{

}


void LowPowerWin::slot_btnOK()//suspend 0
{
    _print("radio1->isChecked()==%d\n",radio1->isChecked());
    if(radio1->isChecked())
        WIFI_set_suspendModel(1);
    else if(radio2->isChecked())
        WIFI_set_suspendModel(0);

    showInforText("SET SUCCESS!");
    QTimer::singleShot(2000,this,SLOT(close()));



    _print("1111111radio1->isChecked()==%d\n",radio1->isChecked());

}
