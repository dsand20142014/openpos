#ifndef GPS_H
#define GPS_H

#include <QDialog>
#include <QSettings>
#include <QStringList>
#include <QString>
#include "readsetting.h"

namespace Ui {
class Gps;
}

class Gps : public QDialog
{
    Q_OBJECT
    
public:
    explicit Gps(QWidget *parent = 0);
    ~Gps();
    
private slots:
    void on_pbCancel_clicked();
    void on_pbEnter_clicked();
    void radioButtonDeal(bool isChecked);

private:
    Ui::Gps *ui;
    QSettings *settings;
    bool isActiveFlag;

    void displayValue(void);
};

#endif // GPS_H
