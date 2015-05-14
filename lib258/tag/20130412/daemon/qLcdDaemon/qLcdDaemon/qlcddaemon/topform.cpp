#include "topform.h"
#include <QHBoxLayout>

TopForm::TopForm(QWidget *) :
    QWidget(0, Qt::Tool| Qt::WindowStaysOnTopHint| Qt::FramelessWindowHint)
{   
    setStyleSheet("QWidget{background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgb(175, 175, 255, 255), stop:0.4 rgb(51,91,151, 190), stop:0.5 rgb(51,91,151, 180), stop:1 rgb(0,50,125, 50))} QLabel{color: white; font: bold; background-color: transparent}");
    setFocusPolicy(Qt::NoFocus);
    setFixedHeight(25);

    label_wifi = new QLabel();
    label_connect = new QLabel();
    label_gprs = new QLabel();
    label_gprs->setMaximumWidth(20);
    label_gprs->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

    label_signal = new QLabel();
    label_signal->setMaximumWidth(20);
    label_signal->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

    label_gps = new QLabel();
    label_power = new QLabel();
    label_power->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

    label_sd = new QLabel();    
    label_time = new QLabel();
    label_usb = new QLabel();
    label_nopaper = new QLabel();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(5, 3, 5, 3);
    layout->addWidget(label_signal);
    layout->addWidget(label_gprs);
    //layout->addSpacing(10);
    layout->addWidget(label_wifi);
    layout->addWidget(label_usb);
    layout->addWidget(label_nopaper);
    layout->addWidget(label_gps);
    layout->addWidget(label_sd);
    //layout->addWidget(label_connect);
    layout->addWidget(label_power, 0, Qt::AlignRight);
    layout->addWidget(label_time, 0, Qt::AlignRight);
    setLayout(layout);
}

TopForm::~TopForm()
{
}
