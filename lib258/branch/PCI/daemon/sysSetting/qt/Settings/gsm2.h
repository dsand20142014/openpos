#ifndef GSM2_H
#define GSM2_H

#include <QDialog>
#include <QSettings>
#include "readsetting.h"
#include "gsm3.h"

namespace Ui {
class Gsm2;
}

class Gsm2 : public QDialog
{
    Q_OBJECT
    
public:
    explicit Gsm2(QSettings *setting, QWidget *parent = 0);
    ~Gsm2();

signals:
    void closeParentWidget();
    
private slots:
    void on_pbNext_clicked();
    void on_pbCancel_clicked();
    void closeWidget();

private:
    Ui::Gsm2 *ui;
    QSettings *settings;

    void displayValue(void);
};

#endif // GSM2_H
