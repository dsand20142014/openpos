#ifndef MODULE_H
#define MODULE_H

#include <QDialog>
#include <QSettings>
#include <QStringList>
#include <QString>
#include "readsetting.h"

namespace Ui {
class Module;
}

class Module : public QDialog
{
    Q_OBJECT
    
public:
    explicit Module(QWidget *parent = 0);
    ~Module();
    
private slots:
    void on_pbEnter_clicked();
    void on_pbCancel_clicked();

private:
    Ui::Module *ui;
    QSettings *settings;

    void displayValue(void);
};

#endif // MODULE_H
