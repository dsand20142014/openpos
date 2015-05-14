#ifndef TOPFORM_H
#define TOPFORM_H

#include <QWidget>
#include <QLabel>


class TopForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit TopForm(QWidget *parent = 0);
    ~TopForm();

    QLabel *label_wifi;
    QLabel *label_connect;
    QLabel *label_gprs;
    QLabel *label_gps;
    QLabel *label_power;
    QLabel *label_sd;
    QLabel *label_signal;
    QLabel *label_time;
    QLabel *label_usb;
    QLabel *label_nopaper;
};

#endif // TOPFORM_H
