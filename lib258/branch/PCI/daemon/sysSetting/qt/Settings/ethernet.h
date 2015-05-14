#ifndef ETHERNET_H
#define ETHERNET_H

#include <QDialog>
#include <QSettings>
#include <QStringList>
#include <QString>
#include "readsetting.h"

namespace Ui {
class Ethernet;
}

class Ethernet : public QDialog
{
    Q_OBJECT
    
public:
    explicit Ethernet(QWidget *parent = 0);
    ~Ethernet();
    
private slots:
    void on_pbCancel_clicked();

    void on_pbEnter_clicked();

private:
    Ui::Ethernet *ui;
    QSettings *settings;

    void displayValue(void);
};

#endif // ETHERNET_H
