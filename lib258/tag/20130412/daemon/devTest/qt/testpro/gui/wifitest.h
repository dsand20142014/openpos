#ifndef WIFITEST_H
#define WIFITEST_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include "MyIpAddrEdit.h"
#include "baseForm.h"



#define DEV_WIFI 1
#define DEV_GSM 0


class WifiTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit WifiTest(bool flag,QWidget *parent = 0);
    ~WifiTest();
    
private slots:

    void check_ip_port();
    void btnOk_clicked();

private:

    bool disabelFlag;
    bool okflag;
    MyIpAddrEdit *mt;
    QTimer timer;

    void getResult();
    int net_interface(struct _if *if_info);

    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_5;
    QLineEdit *lineEdit_2;
    bool wifiFlag;
    bool eventFilter(QObject *, QEvent *);
};

#endif // WIFITEST_H
