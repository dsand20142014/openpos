#include "signintest.h"

#include "button.h"
extern "C"{
#include "printer.h"
}

extern void _print(char *fmt,...);


MyThread::MyThread(int x,int y,int width,int height,QObject *parent )
{
    fbuffer = 0;
    line_addr = 0;

    screenWidth = qApp->desktop()->screen(0)->width();
    fb_fd = 0;
    bytes_per_pixel = 0;
    xres= 0;
    yres = 0;

    runFlag = true;

    label_x = x;
    label_y = y;
    label_w = width;
    label_h = height;

    ts = 0;

    memset(&samp,0,sizeof(ts_sample));

    memset(&fix,0,sizeof(fb_fix_screeninfo));

    memset(line,0x00,sizeof(line));



    memset(&var,0,sizeof(fb_var_screeninfo));


    memset(colormap,0,sizeof(colormap));


    int rec = open_framebuffer();

    for (int i = 0; i < NR_COLORS; i++)
      setcolor (i, palette_sign[i]);

}

void MyThread::setcolor(unsigned colidx, unsigned value)
{
  unsigned res;
  unsigned short red, green, blue;
  struct fb_cmap cmap;

#ifdef DEBUG
  if (colidx > 255) {
    fprintf (stderr, "WARNING: color index = %u, must be <256\n",
       colidx);
    return;
  }
#endif


  switch (0) {
  default:
  case 1:
    res = colidx;
    red = (value >> 8) & 0xff00;
    green = value & 0xff00;
    blue = (value << 8) & 0xff00;
    cmap.start = colidx;
    cmap.len = 1;
    cmap.red = &red;
    cmap.green = &green;
    cmap.blue = &blue;
    cmap.transp = NULL;

          if (ioctl (fb_fd, FBIOPUTCMAP, &cmap) < 0)
                  perror("ioctl FBIOPUTCMAP");
    break;
  case 2:
  case 4:
    red = (value >> 16) & 0xff;
    green = (value >> 8) & 0xff;
    blue = value & 0xff;
    res = ((red >> (8 - var.red.length)) << var.red.offset) |
                      ((green >> (8 - var.green.length)) << var.green.offset) |
                      ((blue >> (8 - var.blue.length)) << var.blue.offset);
  }
        colormap [colidx] = res;
}


int MyThread::open_framebuffer(void)
{
  unsigned y, addr;

  const char * fbdevice = "/dev/fb0";


  fb_fd = open(fbdevice, O_RDWR);
  if (fb_fd == -1) {
    perror("open fbdevice");
    return -1;
  }

  if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix) < 0) {
    perror("ioctl FBIOGET_FSCREENINFO");
    close(fb_fd);
    return -1;
  }

  if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &var) < 0) {
    perror("ioctl FBIOGET_VSCREENINFO");
    close(fb_fd);
    return -1;
  }

  xres = var.xres;
  yres = var.yres;

  fbuffer = (unsigned char*)mmap(NULL, fix.smem_len, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fb_fd, 0);
  if (fbuffer == (unsigned char *)-1) {
    perror("mmap framebuffer");
    close(fb_fd);
    return -1;
  }
//  memset(fbuffer,0xFF,fix.smem_len);

  bytes_per_pixel = (var.bits_per_pixel + 7) / 8;
//  _print("X:%d,Y:%d,bpp:%d\n",xres,yres,bytes_per_pixel);
  line_addr = (unsigned char**)malloc (sizeof (__u32) * var.yres_virtual);
  addr = 0;
  for(y = 0; y < var.yres_virtual; y++, addr += fix.line_length)
    line_addr [y] = fbuffer + addr;

  return 0;
}

void MyThread::__setpixel (union multiptr loc, unsigned xormode, unsigned color)
{
  switch(bytes_per_pixel) {
  case 1:
  default:
    if (xormode)
      *loc.p8 ^= color;
    else
      *loc.p8 = color;
    break;
  case 2:
    if (xormode)
      *loc.p16 ^= color;
    else
      *loc.p16 = color;
    break;
  case 4:
    if (xormode)
      *loc.p32 ^= color;
    else
      *loc.p32 = color;
    break;
  }
}

void MyThread::pixel (int x, int y, unsigned colidx)
{
  unsigned xormode;
  union multiptr loc;

  int x1,y1;

  x1 = y;
  y1 = screenWidth-x;

  if ((x1 < 0) || ((__u32)x1 >= var.xres_virtual) ||
      (y1 < 0) || ((__u32)y1 >= var.yres_virtual))
    return;

  xormode = colidx & XORMODE;
  colidx &= ~XORMODE;

#ifdef DEBUG
  if (colidx > 255) {
    fprintf (stderr, "WARNING: color value = %u, must be <256\n",
       colidx);
    return;
  }
#endif

  loc.p8 = line_addr [y1] + x1 * bytes_per_pixel;
  __setpixel (loc, xormode, colormap[colidx]);
}

void MyThread::add_point(int x,int y,unsigned colidx)
{
  int i,j,m,n;
  for(i= -1;i<=1;i++)
   for(j= -1;j<=1;j++){
    if(i==0 && j==0)
    {
        pixel(x+i,y+j,colidx);
    }
        else if(i==0 && j==-1)
        {
            pixel(x+i,y+j,colidx);
        }
        else if(i==1 && j==0)
        {
            pixel(x+i,y+j,colidx);
        }
    m = x+i+74;
    n = y+j;

    if(n<48)
      n = 48;
    if(m<0)
      m = 0;

    line[(n-48)*48+m/8] |= (0x1<<(7-m%8));
  }
}


void MyThread::line2 (int x1, int y1, int x2, int y2, unsigned colidx)
{
  static volatile int tmp;
  static volatile int dx;
  static volatile int dy;

  dx = x2 - x1;
  dy = y2 - y1;

  if (abs (dx) < abs (dy)) {
    if (y1 > y2) {
      tmp = x1; x1 = x2; x2 = tmp;
      tmp = y1; y1 = y2; y2 = tmp;
      dx = -dx; dy = -dy;
    }
    x1 <<= 16;
    /* dy is apriori >0 */
    dx = (dx << 16) / dy;
    while (y1 <= y2) {
      add_point(x1 >> 16,y1,colidx);
      //pixel (x1 >> 16,y1,colidx);
      x1 += dx;
      y1++;
    }
  } else {
    if (x1 > x2) {
      tmp = x1; x1 = x2; x2 = tmp;
      tmp = y1; y1 = y2; y2 = tmp;
      dx = -dx; dy = -dy;
    }
    y1 <<= 16;
    dy = dx ? (dy << 16) / dx : 0;
    while (x1 <= x2) {

      add_point(x1,y1 >>16,colidx);
            //pixel (x1,y1 >>16,colidx);
      y1 += dy;
      x1++;
    }
  }


}


void MyThread::run()
{
    int x = 0; int y = 0;
    ts = ts_open("/dev/input/ts0",1);
    if(!ts) return;
    if(ts_config(ts))  return;
    int mode = 1;

    while(runFlag){
        int ret;
        ret = ts_read(ts, &samp, 1);

        if(ret < 0) break;
        if(ret!=1) {
            usleep(20000);
            continue;
        }

        if (samp.pressure > 0) {
            //in sign zone
            if((samp.x>label_y)&&(samp.x<label_y+label_h) && (screenWidth-samp.y>label_x)&&(screenWidth-samp.y<label_w+label_x))
            {
                if(mode == 0x3)
                {
                    line2(x, y, screenWidth-samp.y, samp.x,2);
                }
                else
                {
                    mode |= 0x2;
                }

                x = screenWidth-samp.y;
                y = samp.x;

            }
            else
            {
                mode &= ~0x2;
            }
        }
        else
        {
            mode &= ~0x2;

        }

      }

//    ts_close(ts);
}


SignInTest::SignInTest(QWidget *parent) :
    BaseForm(tr("SIGN IN TEST"),parent)
{
    xm = 10;
    ym = 58;
    printfFalg = false;

    textEdit = new QTextEdit;
    textEdit->setEnabled(false);
    layout->addWidget(textEdit);

//    sw = qApp->desktop()->screen(0)->width();
//    sh = qApp->desktop()->screen(0)->height();


    mythread =  new MyThread(xm+1,ym+1,screenWidth-22,screenHeight-122,this);
    mythread->start();

    btnOk->setText(tr("Print"));
    btnOk->show();
    btnOk->setFocus();
    btnlayout->addWidget(btnOk);

    clearBtn = new Button(tr("Clear"));
    btnlayout->addWidget(clearBtn);


    clearBtn->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);//68, 36);
    btnOk->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);
    btnExit->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);

    connect(btnOk, SIGNAL(clicked()), this, SLOT(pushButton_3_clicked()));
    connect(btnExit, SIGNAL(clicked()), this, SLOT(pushButton_clicked()));
    connect(clearBtn, SIGNAL(clicked()), this, SLOT(clearButton_clicked()));

}




SignInTest::~SignInTest()
{
    _print("in ~SignInTest\n");
      munmap(mythread->fbuffer, mythread->fix.smem_len);
      ::close(mythread->fb_fd);
      free (mythread->line_addr);//
      ts_close(mythread->ts);
}

void SignInTest::pushButton_clicked()
{
    mythread->runFlag = false;
    if( printfFalg){
        showInforText(tr("TEST SUCCESS!"),false);

        QTimer::singleShot(1000,this,SLOT(close()));
    }
    else
        close();
}


void SignInTest::clearButton_clicked()
{
   setbtnEnable(false);

    for(int i=0;i<screenWidth-22;i++)
        memset(mythread->fbuffer+59*2+(i+11)*640,0xFF,(screenHeight-122)*2);

     memset(mythread->line,0x00,sizeof(mythread->line));

     QTimer::singleShot(800,this,SLOT(slotbtnEnable()));

}

void SignInTest::slotbtnEnable()
{
    setbtnEnable(true);
}

void SignInTest::setbtnEnable(bool enable)
{
    _print("enable===%d\n",enable);
    clearBtn->setEnabled(enable);
    btnOk->setEnabled(enable);

}

void SignInTest::pushButton_3_clicked()//PRINT
{
    unsigned char ucResult = 0x00;

    ucResult = Os__checkpaper();
    if(ucResult == 0x00)
    {
        setbtnEnable(false);
        int recprint = Os__graph_xprint(mythread->line,sizeof(mythread->line));
        _print("rec print**********\n",recprint);

        printfFalg = true;
        QTimer::singleShot(800,this,SLOT(clearButton_clicked()));
    }
    else
    {
        QMessageBox box(QMessageBox::Information,tr("    title   "),tr("NO PAPER, PLEASE CHECK!"));
        box.setWindowFlags(Qt::CustomizeWindowHint);

        QPushButton *okBtn = new QPushButton("Ok");
        okBtn->setFixedSize((80*screenWidth)/240,(25*screenHeight)/320);
        box.addButton(okBtn,QMessageBox::YesRole);

       box.exec();

       if(box.clickedButton() == okBtn){
           qDebug()<<"clear***********1111";
           QTimer::singleShot(100,this,SLOT(clearButton_clicked()));
       }

    }


}

void SignInTest::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Escape:
        mythread->runFlag = false;
        if( printfFalg){
            showInforText(tr("TEST SUCCESS!"),false);

            QTimer::singleShot(1000,this,SLOT(close()));
        }
        else
            close();
        break;

    case Qt::Key_Enter:
        pushButton_3_clicked();
        break;

    case Qt::Key_Clear:
    case Qt::Key_Backspace:
        clearButton_clicked();
        break;

    }
}

