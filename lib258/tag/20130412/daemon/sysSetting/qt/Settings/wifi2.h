#ifndef WIFI2_H
#define WIFI2_H

#include <QDialog>
#include <QSettings>
#include <QStringList>
#include <QString>
#include "readsetting.h"
#include "wifi3.h"

namespace Ui {
class Wifi2;
}

class Wifi2 : public QDialog
{
    Q_OBJECT
    
public:
    explicit Wifi2(QSettings *setting, QWidget *parent = 0);
    ~Wifi2();

signals:
    void closeParentWidget();

private slots:
    void on_pbCancel_clicked();
    void on_pbNext_clicked();
    void pwdDeal(QString str);
    void closeWidget();

private:
    Ui::Wifi2 *ui;
    QSettings *settings;
    QString strPwd;

    void displayValue(void);    
};

#endif // WIFI2_H
