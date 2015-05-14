#ifndef GSM4_H
#define GSM4_H

#include <QDialog>
#include <QSettings>

#include "readsetting.h"

namespace Ui {
class Gsm4;
}

class Gsm4 : public QDialog
{
    Q_OBJECT
    
public:
    explicit Gsm4(QSettings *setting, QWidget *parent = 0);
    ~Gsm4();

signals:
    void closeParentWidget();

private slots:
    void on_pbEnter_clicked();
    void on_pbCancel_clicked();
    void radioButton1Deal(bool);
    void radioButton2Deal(bool);
    void radioButton3Deal(bool);
    void radioButton4Deal(bool);
    void radioButton5Deal(bool);
    void radioButton6Deal(bool);
    void radioButton7Deal(bool);
    void lineEditDeal(QString str);

private:
    Ui::Gsm4 *ui;
    QSettings *settings;

    void displayValue(void);
};

#endif // GSM4_H
