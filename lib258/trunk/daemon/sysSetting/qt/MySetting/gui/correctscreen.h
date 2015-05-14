#ifndef CORRECTSCREEN_H
#define CORRECTSCREEN_H

#include <baseForm.h>
#include <QtGui>

#include "test/tslib.h"
#include "test/config.h"
#include "test/fbutils.h"
#include "test/testutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/fb.h>


static int palette1 [] =
{
    0x000000, 0xffe080, 0xffffff, 0xe0c0a0
};
#define NR_COLORS (sizeof (palette1) / sizeof (palette1 [0]))

typedef struct {
    int x[5], xfb[5];
    int y[5], yfb[5];
    int a[7];
} calibration;



class ScreenThread : public QThread
{
     Q_OBJECT

public:
    ScreenThread(QObject *parent = 0);
    bool stopFlag;
    void run();
   struct tsdev *ts;

signals:
  void correctEnd(int );

private:
    void get_sample(struct tsdev *ts, calibration *cal, int index, int x, int y, char *name);
    int perform_calibration(calibration *cal) ;
    void sig(int sig);

    calibration cal;

//    bool errorFlag;

};

class CorrectScreen : public QWidget
{
    Q_OBJECT
    
public:
    explicit CorrectScreen(QWidget *parent = 0);
    ~CorrectScreen();
    
private:
    QLabel* labelScreen;
    ScreenThread *sThread;
    void keyPressEvent(QKeyEvent *);
    QTimer timer;

private slots:
    void slotEnd(int );

};

#endif // CORRECTSCREEN_H
