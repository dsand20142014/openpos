#ifndef BATTERYTEST_H
#define BATTERYTEST_H

//#include <QWidget>
#include <QTimer>
#include <QtGui>
#include "baseForm.h"

class BatteryTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit BatteryTest(QWidget *parent = 0);
    ~BatteryTest();
    
private slots:
    void get_data();

private:
    QLabel *labone;
    QLabel *labtwo;
    QTimer timer;
    //void keyPressEvent(QKeyEvent *);
};

#endif // BATTERYTEST_H
