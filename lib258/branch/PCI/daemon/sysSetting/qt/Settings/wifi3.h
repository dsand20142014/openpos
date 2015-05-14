#ifndef WIFI3_H
#define WIFI3_H

#include <QDialog>
#include <QSettings>
#include <QStringList>
#include <QString>
#include "readsetting.h"

namespace Ui {
class Wifi3;
}

class Wifi3 : public QDialog
{
    Q_OBJECT
    
public:
    explicit Wifi3(QSettings *setting, QWidget *parent = 0);
    ~Wifi3();

signals:
    void closeParentWidget();

private slots:
    void on_pbCancel_clicked();
    void on_pbEnter_clicked();
    void radioButtonDeal(bool isChecked);

private:
    Ui::Wifi3 *ui;
    QSettings *settings;
    bool dhcpFlag;

    void displayValue(void);
};

#endif // WIFI3_H
