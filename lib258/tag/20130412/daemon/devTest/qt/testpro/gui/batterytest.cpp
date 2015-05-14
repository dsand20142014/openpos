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

    layout->addSpacerItem(new QSpacerItem(10,15));

    layout->addWidget(label_2);
    layout->addWidget(labone);

    layout->addSpacerItem(new QSpacerItem(10,15));

    layout->addWidget(label_3);
    layout->addWidget(labtwo);

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
