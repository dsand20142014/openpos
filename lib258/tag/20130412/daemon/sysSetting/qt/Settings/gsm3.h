#ifndef GSM3_H
#define GSM3_H

#include <QDialog>
#include <QSettings>
#include "readsetting.h"
#include "gsm4.h"

namespace Ui {
class Gsm3;
}

class Gsm3 : public QDialog
{
    Q_OBJECT
    
public:
    explicit Gsm3(QSettings *setting, QWidget *parent = 0);
    ~Gsm3();

signals:
    void closeParentWidget();

private slots:
    void on_pbNext_clicked();
    void on_pbCancel_clicked();
    void closeWidget();

private:
    Ui::Gsm3 *ui;
    QSettings *settings;

    void displayValue(void);
};

#endif // GSM3_H
