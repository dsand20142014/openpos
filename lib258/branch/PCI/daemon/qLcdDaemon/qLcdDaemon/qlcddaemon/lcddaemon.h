#ifndef LCDDAEMON_H
#define LCDDAEMON_H

#include <QMainWindow>
#include <QDebug>
#include <QLabel>
//#include <QMessageBox>
//#include <QToolButton>
//#include <QGridLayout>
//#include <QThread>
#include <QKeyEvent>
#include "gui/parallaxhome.h"
#include "lcddaemonmsgqueue.h"
#include "topform.h"
#include "syswindow.h"
#include "socketserver.h"
#include "loadgif.h"
#include "poweroff.h"
#include "sanddisplay.h"
#include "pin.h"
#include "common.h"

extern "C"{
#include "updateService.h"
}

class LcdDaemon : public QWidget//QMainWindow
{
    Q_OBJECT

public:
    explicit LcdDaemon(QWidget *parent = 0, QStringList *argv = NULL);
    ~LcdDaemon();


public slots:
    void lcdMsgProc(QVariant msgVar);

private slots:
    void timeupdate();
    void restartSleepTimer(bool start=true);
    void start_win();
    void gifloadopt(struct gif_loading_struct *gif_content);
    void slot_get_pin(QString pin);
    void slot_pin_timeout();
    void slot_syswin_exit();
    void slot_setDisabled(bool);

private:
    bool isStart;
    bool keyEnable;
    bool STOP_SLEEP;
    QVBoxLayout *layout;
    QMovie *loading;
    QLabel *giflabel;
    void stopGif();
    void startGif();
    void setupWindow();
    void showDisp();
    void setWidgetText(char* text, int widget_type, int widget_info);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void showPin(QString text, int timeout);
    void sendmsg2linuxpos(struct lcd_message *msg);
    QWidget* item[MAX_LINE];
    QLabel* sandlabel[MAX_LINE];
    SysWindow *start;

    QStringList testNames;
    QStringList testIcons;
    ParallaxHome *slideScreen;
    bool paral_show;

    QTimer *isStart_timer;
    QTimer timerPin;
    int g_app_cnt;
    int g_app_curret;
    QLabel printer_state_box;
    messageBox battery_state_box;
    TopForm *top_form;

    PowerOff *powerForm;
    QThread *qsocketserverthread;
    LoadGif *myloadgifWin;
    QLabel *message;
    QStringList *sysArgv;
    PinForm *pinForm;
    SandDisplay *sandDisp;

signals:
    void display(int c);
};

#endif // LCDDAEMON_H
