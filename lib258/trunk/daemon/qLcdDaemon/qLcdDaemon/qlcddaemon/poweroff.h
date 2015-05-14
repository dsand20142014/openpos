#ifndef PowerOff_H
#define PowerOff_H

#include <QWidget>
#include <QLabel>
#include <QKeyEvent>
#include "gui/button.h"
#include "wifithread.h"

class PowerOff : public QWidget
{
    Q_OBJECT
    
public:
    explicit PowerOff(QWidget *parent = 0);
    ~PowerOff();
    WifiThread sleepThread;

private:
    Button *btn_poweroff;
    Button *btn_reboot;
    Button *btn_suspend;
    Button *btn_exit;
    QLabel *label;   
//    void keyPressEvent(QKeyEvent *e);	
    void keyReleaseEvent(QKeyEvent *e);	

private slots:
    void slot_poweroff();
    void slot_reboot();
    void slot_suspend();
};

#endif // PowerOff_H
