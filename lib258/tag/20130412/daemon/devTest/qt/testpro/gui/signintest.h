#ifndef SIGNINTEST_H
#define SIGNINTEST_H

#include <QtGui>
#include "baseForm.h"
#include <QThread>


extern "C"{
#include "sand_debug.h"
#include "test/tslib.h"
#include "test/tslib-private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <linux/vt.h>
#include <linux/kd.h>
#include <linux/fb.h>
}


union multiptr {
  unsigned char *p8;
  unsigned short *p16;
  unsigned long *p32;
};


#define XORMODE	0x80000000
static int palette_sign[] = {
    0x000000, 0xffe080, 0xffffff, 0xe0c0a0
};
#define NR_COLORS (sizeof (palette_sign) / sizeof (palette_sign [0]))


class MyThread : public QThread
{
     Q_OBJECT

public:
    MyThread(int , int,int ,int ,QObject *parent = 0);
    struct tsdev *ts;
    struct ts_sample samp;

    int label_x;
    int label_y;
    int label_w;
    int label_h;
    bool runFlag;

    int fb_fd;
    unsigned char line[320*48];
    unsigned char *fbuffer;
    struct fb_fix_screeninfo fix;
    unsigned char **line_addr;

private:
    void run();
    void line2 (int x1, int y1, int x2, int y2, unsigned colidx);
    void add_point(int x,int y,unsigned colidx);
    void pixel (int x, int y, unsigned colidx);
    void __setpixel (union multiptr loc, unsigned xormode, unsigned color);
    void setcolor(unsigned colidx, unsigned value);

    int open_framebuffer(void);

     struct fb_var_screeninfo var;


     int bytes_per_pixel;
     unsigned colormap [256];
     int xres, yres;

};

class SignInTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit SignInTest(QWidget *parent = 0);
    ~SignInTest();
    
private slots:
    void pushButton_clicked();
    void pushButton_3_clicked();
	void clearButton_clicked();
    void setbtnEnable(bool);
    void slotbtnEnable();


private:
    int xm,ym;
    Button *clearBtn;
    MyThread *mythread;
    bool printfFalg;
    bool enterFlag;

protected:
    void keyPressEvent(QKeyEvent *);

};


#endif // SIGNINTEST_H
