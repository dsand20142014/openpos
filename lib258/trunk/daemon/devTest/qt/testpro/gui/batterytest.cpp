#include "batterytest.h"
extern "C"{
#include "devSignal.h"

}


BatteryTest::BatteryTest(QWidget *parent) :
    BaseForm(tr("BATTERY TEST"), parent)
{
    QLabel *label_2 = new QLabel(tr("    BUTTON BATTERY:    "));
    QLabel *label_3 = new QLabel(tr("    BATTERY:    "));

    labone = new QLabel;
    labtwo = new QLabel;
    labone->setObjectName("labelShowKey");
    labtwo->setObjectName("labelShowKey");

    labone->setAlignment(Qt::AlignCenter);
    labtwo->setAlignment(Qt::AlignCenter);


    QTextEdit *textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(18);
    vbox->addWidget(label_2);
    vbox->addWidget(labone);
    vbox->addWidget(label_3);
    vbox->addWidget(labtwo);
    vbox->setContentsMargins(30,10,30,10);

    vbox->addStretch(1);

    textEdit->setLayout(vbox);

    layout->addWidget(textEdit);

    connect(&timer,SIGNAL(timeout()),this,SLOT(get_data()));
    timer.start(500);
}

void BatteryTest::get_data()
{
    double id1 = devSignal_battery_voltage(2);//纽扣电池
    double id2 = devSignal_battery_voltage(1);//鲤电池


    QString t1,t2;
    t1.sprintf("%.2lf",id1);
    t2.sprintf("%.2lf",id2);

    labone->setText(t1+"V");
    labtwo->setText(t2+"V");
}


BatteryTest::~BatteryTest()
{

}
