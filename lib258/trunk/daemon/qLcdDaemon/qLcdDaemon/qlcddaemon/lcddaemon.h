#ifndef LCDDAEMON_H
#define LCDDAEMON_H

#include <QMainWindow>
#include <QDebug>
#include <QLabel>
#include "lcddaemonmsgqueue.h"
#include <QToolButton>
#include <QGridLayout>
#include <QThread>
#include <QKeyEvent>
#include "gui/parallaxhome.h"
#include <QMessageBox>
#include "topform.h"
#include "syswindow.h"
#include "socketserver.h"
#include "loadgif.h"
#include "poweroff.h"
#include "sanddisplay.h"

extern "C"{
//#include "applist.h"
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

private:
    bool isStart;
	bool TIMER_STOP;
    QVBoxLayout *layout;
    QMovie *loading;
    QLabel *giflabel;
    void stopGif();
    void startGif();
    void setWidgetText(char* text, int widget_type, int widget_info);
    /*void absSetLabelText(QLabel* label,int col,char* text);
    void clearLabelText(int line);
    void qt_display_image(int x,int y,char *fname);*/
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    QWidget* item[MAX_LINE];
    QLabel* sandlabel[MAX_LINE];
    SysWindow *start;
    /*SandButton* sandbutton[MAX_LINE];
    SandButton *sandicon[MAX_LINE+1];
    SandButton *sandicon_page2[MAX_LINE+1];
    SandButton *sandicon_page3[MAX_LINE+1];
    SandButton *sandicon_page4[MAX_LINE+1];
    SandButton *sandicon_page5[MAX_LINE+1];*/
    QStringList testNames;
    QStringList testIcons;
    ParallaxHome *slideScreen;
    bool paral_show;

    QTimer *isStart_timer;
    int g_app_cnt;
    int g_app_curret;
    QLabel printer_state_box;
    QMessageBox battery_state_box;
    TopForm *top_form;

    PowerOff *powerForm;
    QThread *qsocketserverthread;
    LoadGif *myloadgifWin;
    QLabel *message;
    QStringList *sysArgv;

    SandDisplay *sandDisp;
};

#endif // LCDDAEMON_H
