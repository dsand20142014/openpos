#ifndef DATE_H
#define DATE_H

#include <QDialog>
#include <QSettings>
#include <QDate>
#include <QTime>
#include "readsetting.h"


namespace Ui {
class Date;
}

class Date : public QDialog
{
    Q_OBJECT
    
public:
    explicit Date(QWidget *parent = 0);
    ~Date();
    
private slots:
    void on_pbEnter_clicked();
    void on_pbCancel_clicked();

private:
    Ui::Date *ui;
    QDate date;
    QTime time;
    //ReadSetting readSetting;
    QSettings *settings;
    void displayValue(void);
};

#endif // DATE_H
