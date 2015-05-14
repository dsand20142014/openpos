#ifndef WIFI_H
#define WIFI_H

#include <QDialog>
#include <QSettings>
#include <QStringList>
#include <QString>
#include "readsetting.h"
#include "wifi2.h"

namespace Ui {
class Wifi;
}

class Wifi : public QDialog
{
    Q_OBJECT
    
public:
    explicit Wifi(QWidget *parent = 0);
    ~Wifi();
    
private slots:
    void on_pbCancel_clicked();
    void on_pbNext_clicked();
//    void pwdDeal(QString str);
    void radioButtonDeal(bool isActived);

private:
    Ui::Wifi *ui;
    QSettings *settings;
//    QString strPwd;
    bool isActiveFlag;

    void displayValue(void);
};

#endif // WIFI_H
