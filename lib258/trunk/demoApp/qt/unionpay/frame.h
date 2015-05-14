#ifndef FRAME_H
#define FRAME_H

#include <QMainWindow>
//#include "msgrecivethread.h"
#include <QCloseEvent>
#include <QPaintEvent>
#include <QTimer>
#include "mainentrythread.h"
#include "new_drv.h"
//#include "osevent.h"
#include "cardnumdisplay.h"
#include "mainmenu.h"

//#include "includes.h"


namespace Ui {
class Frame;
}


class Frame : public QWidget
{
    Q_OBJECT

public:
    explicit Frame(int argc, char *argv[],QWidget *parent = 0);
    ~Frame();
    QTimer * lpTimer;
    char aucISO2Data[38];
//    MainEntryThread *mainEntryThread;
private slots:
    void timeupdate();
    void reStartTimer();
    void sysEventSlot(int);
    void wakeUpMainThread();
protected:
//    void closeEvent(QCloseEvent *);
//    void paintEvent(QPaintEvent *event);
//    void showEvent(QShowEvent *event);
//     bool eventFilter(QObject *target, QEvent *event);
private:
    Ui::Frame *ui;
    MainMenu * lpMainMenu;
    CardNumDisplay * lpCardNumDisplay;

    unsigned char mainMenuEntryFlag;




    void MainMenu_Display(void);
    void MainMenu_ReciveMsg(void);
    void CardInfo_Display(void);

//    MsgReciveThread msgReciveThread;
};

#endif // FRAME_H
