#ifndef WIFISETWIN_H
#define WIFISETWIN_H

#include <QtGui>
#include "baseForm.h"
#include "button.h"
extern "C"{
#include "wifi.h"
#include "wpa.h"
}


class MyThread : public QThread
{
    Q_OBJECT


private:
    void run();

signals:
    void emitrecB(bool,int);

};


class WifiSetWin : public BaseForm
{
    Q_OBJECT
    
public:
    explicit WifiSetWin(QWidget *parent = 0);
    ~WifiSetWin();
    
private:
    int curMacRow;
     int curRow ;
     QString tempname;
    QListWidget *wifiList;
    WIFISCAN wifiScan[20];

    QWidget *win[20] ;
    QHBoxLayout *lay[20] ;
    QLabel *name[20]  ;
    QLabel *pic[20] ;

    Button *modifyBtn;
    void setWifiInfo(WIFIINFO *wifiinfo);
    void removeWifiInfo(char *addressMAC);
//    void ensureWin();
//    void pswWin();
private slots:
    void connectSLot(QModelIndex);
    void refreshData();
    void modifyWifi();
    void slot_showrecB(bool,int);
    void slot_disableBtn();
    void slot_enableBtn();
};

#endif // WIFISETWIN_H
