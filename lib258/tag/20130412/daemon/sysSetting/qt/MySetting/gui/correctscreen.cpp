#include "correctscreen.h"
#include "ostools.h"
extern void _print(char *fmt,...);

ScreenThread::ScreenThread(QObject *parent )
{
    stopFlag = false;
    //signal(SIGSEGV, sig);
    //signal(SIGINT, sig);
    //signal(SIGTERM, sig);

    ts = ts_open("/dev/input/ts0",0);
    _print("ts===========%d",ts);
    if (!ts) {
        perror("ts_open");
        exit(1);
    }

    if (ts_config(ts)) {
        perror("ts_config");
        exit(1);
    }

    if (open_framebuffer()) {
        close_framebuffer();
        exit(1);
    }

    for (unsigned int i = 0; i < NR_COLORS; i++)
        setcolor (i, palette1 [i]);

}

void ScreenThread::run()
{
    int cal_fd;
    char cal_buffer[256];
//    char *tsdevice = NULL;
//    char *calfile = NULL;
    unsigned int i;

    _print("xres = %d, yres = %d\n", xres, yres);

    get_sample (ts, &cal, 0, 50,        50,        "Top left");

    get_sample (ts, &cal, 1, xres - 50, 50,        "Top right");


    get_sample (ts, &cal, 2, xres - 50, yres - 50, "Bot right");

    get_sample (ts, &cal, 3, 50,        yres - 50, "Bot left");

    get_sample (ts, &cal, 4, xres / 2,  yres / 2,  "Center");



    if(perform_calibration (&cal))
    {
        emit correctEnd(1);

        _print ("Calibration constants: ");
        for(i = 0; i < 7; i++)
            _print("%d ", cal.a [i]);
        _print("\n");

        cal_fd = open ("/etc/pointercal", O_CREAT | O_RDWR);
        if(cal_fd>0) {
            memset(cal_buffer,0,sizeof(cal_buffer));
            sprintf (cal_buffer,"%d %d %d %d %d %d %d",
                     cal.a[1], cal.a[2], cal.a[0],
                     cal.a[4], cal.a[5], cal.a[3], cal.a[6]);
            write(cal_fd, cal_buffer, strlen (cal_buffer) + 1);
            close (cal_fd);
            i = 0;
        }else{
            emit correctEnd(0);
            i = -1;
        }

    } else {
        _print("Calibration failed.\n");
        i = -1;
        emit correctEnd(0);
    }

    close_framebuffer();
    ts_close(ts);

    stopFlag = true;
}

void  ScreenThread::sig(int sig)
{
    close_framebuffer();
    fflush (stderr);
    _print ("signal %d caught\n", sig);
    fflush (stdout);
    exit (1);
}

int ScreenThread::perform_calibration(calibration *cal) {
    int j;
    float n, x, y, x2, y2, xy, z, zx, zy;
    float det, a, b, c, e, f, i;
    float scaling = 65536.0;

// Get sums for matrix
    n = x = y = x2 = y2 = xy = 0;
    for(j=0;j<5;j++) {
        n += 1.0;
        x += (float)cal->x[j];
        y += (float)cal->y[j];
        x2 += (float)(cal->x[j]*cal->x[j]);
        y2 += (float)(cal->y[j]*cal->y[j]);
        xy += (float)(cal->x[j]*cal->y[j]);
    }

// Get determinant of matrix -- check if determinant is too small
    det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
    if(det < 0.1 && det > -0.1) {
        _print("ts_calibrate: determinant is too small -- %f\n",det);
        return 0;
    }

// Get elements of inverse matrix
    a = (x2*y2 - xy*xy)/det;
    b = (xy*y - x*y2)/det;
    c = (x*xy - y*x2)/det;
    e = (n*y2 - y*y)/det;
    f = (x*y - n*xy)/det;
    i = (n*x2 - x*x)/det;

// Get sums for x calibration
    z = zx = zy = 0;
    for(j=0;j<5;j++) {
        z += (float)cal->xfb[j];
        zx += (float)(cal->xfb[j]*cal->x[j]);
        zy += (float)(cal->xfb[j]*cal->y[j]);
    }

// Now multiply out to get the calibration for framebuffer x coord
    cal->a[0] = (int)((a*z + b*zx + c*zy)*(scaling));
    cal->a[1] = (int)((b*z + e*zx + f*zy)*(scaling));
    cal->a[2] = (int)((c*z + f*zx + i*zy)*(scaling));

    _print("%f %f %f\n",(a*z + b*zx + c*zy),(b*z + e*zx + f*zy),(c*z + f*zx + i*zy));

// Get sums for y calibration
    z = zx = zy = 0;
    for(j=0;j<5;j++) {
        z += (float)cal->yfb[j];
        zx += (float)(cal->yfb[j]*cal->x[j]);
        zy += (float)(cal->yfb[j]*cal->y[j]);
    }

// Now multiply out to get the calibration for framebuffer y coord
    cal->a[3] = (int)((a*z + b*zx + c*zy)*(scaling));
    cal->a[4] = (int)((b*z + e*zx + f*zy)*(scaling));
    cal->a[5] = (int)((c*z + f*zx + i*zy)*(scaling));

    _print("%f %f %f\n",(a*z + b*zx + c*zy),(b*z + e*zx + f*zy),(c*z + f*zx + i*zy));

// If we got here, we're OK, so assign scaling to a[6] and return
    cal->a[6] = (int)scaling;
    return 1;

}

void ScreenThread::get_sample(struct tsdev *ts, calibration *cal, int index, int x, int y, char *name)
{
    static int last_x = -1, last_y;

    if (last_x != -1)
    {
        #define NR_STEPS 10
        int dx = ((x - last_x) << 16) / NR_STEPS;
        int dy = ((y - last_y) << 16) / NR_STEPS;
        int i;
        last_x <<= 16;
        last_y <<= 16;
        for (i = 0; i < NR_STEPS; i++)
        {
            put_cross (last_x >> 16, last_y >> 16, 2 | XORMODE);
            usleep (1000);
            put_cross (last_x >> 16, last_y >> 16, 2 | XORMODE);
            last_x += dx;
            last_y += dy;
        }
    }

    put_cross(x, y, 2 | XORMODE);
    getxy (ts, &cal->x [index], &cal->y [index]);
    put_cross(x, y, 2 | XORMODE);

    last_x = cal->xfb [index] = x;
    last_y = cal->yfb [index] = y;

    _print("%s : X = %4d Y = %4d\n", name, cal->x [index], cal->y [index]);
}




CorrectScreen::CorrectScreen(QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint)
{
    sThread = 0;
    setGeometry(0,0,240,320);
    setAttribute(Qt::WA_DeleteOnClose);

    sThread = new ScreenThread;
    sThread->stopFlag = false;

    labelScreen = new QLabel("PRESS THE CROSS TO ADJUST!",this);
    labelScreen->raise();
    labelScreen->setAlignment(Qt::AlignCenter);
    labelScreen->show();
    labelScreen->setGeometry(0,0,240,320);
    labelScreen->setStyleSheet("QLabel{background-color:black;color:green;}");


    this->setFocus();
    QTimer::singleShot(1000,sThread,SLOT(start()));

    connect(sThread,SIGNAL(correctEnd(int)),this,SLOT(slotEnd(int)));

}


void CorrectScreen::slotEnd(int ok)
{
    //qDebug()<<"~~~~~slotEnd  ok====~~~~~~~~~~";
    if(ok)
        labelScreen->setText("ADJUST SUCCESS!");
    else
        labelScreen->setText("ADJUST FAILED!");

    //qDebug()<<"~~~~~~~~~~~~labelScreen->text()~~~~~~~~~~"<<labelScreen->text();

}


CorrectScreen::~CorrectScreen()
{
//    sThread->terminate();
//    sThread->wait();

}


void CorrectScreen::keyPressEvent(QKeyEvent *e)
{
    //qDebug()<<"stopFlag*******"<<sThread->stopFlag;
    if(!sThread->stopFlag) return;

    if(e->key()==Qt::Key_Escape)
    {
        //qDebug()<<"e->key()==="<<e->key();

        close();
    }

}
