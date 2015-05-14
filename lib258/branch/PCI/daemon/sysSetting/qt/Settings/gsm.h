#ifndef GSM_H
#define GSM_H

#include <QDialog>
#include <QSettings>
#include <QStringList>
#include <QString>
#include "readsetting.h"
#include "gsm2.h"

namespace Ui {
class Gsm;
}

class Gsm : public QDialog
{
    Q_OBJECT
    
public:
    explicit Gsm(QWidget *parent = 0);
    ~Gsm();
    
private slots:
    void on_pbNext_clicked();
    void on_pbCancel_clicked();
    void pwdDeal(QString str);

private:
    Ui::Gsm *ui;
    QSettings *settings;
    QString strPwd;
    //ReadSetting readSetting;

    void displayValue(void);
};

#endif // GSM_H
