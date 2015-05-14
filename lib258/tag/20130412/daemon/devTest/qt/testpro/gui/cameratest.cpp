#include "cameratest.h"
#include "ui_cameratest.h"
#include <QtDebug>
#include "button.h"
extern "C"{
#include "camera.h"
#include "sand_debug.h"
}
extern void _print(char *fmt,...);

CameraTest::CameraTest(QWidget *parent) :
    QWidget(parent,Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint),
    ui(new Ui::CameraTest)
{
    ui->setupUi(this);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("/daemon/image/thirdback.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    setAttribute(Qt::WA_DeleteOnClose);

    flag = false;
    onceFLag = false;
    buf = NULL;
    imgWidth = 320;
    imgHeight = 240;

    bufSize = imgWidth*imgHeight*2;
    buf = (char *)calloc(bufSize,1);

    exitBtn = new Button(tr("Exit"), 1);
    ui->horizontalLayout->addWidget(exitBtn);
    rephotoBtn = new Button(tr("Rephoto"));
    ui->horizontalLayout->addWidget(rephotoBtn);
    exitBtn->setMinimumSize(68, 36);
    rephotoBtn->setMinimumSize(68, 36);

    exitBtn->hide();
    rephotoBtn->hide();

    setFocus();

    ini_data();

    connect(exitBtn,SIGNAL(clicked()),this,SLOT(slot_quit()));
    connect(rephotoBtn,SIGNAL(clicked()),this,SLOT(slot_rephoto()));
}

void CameraTest::ini_data()
{
    int fd = 0;
    int i, imgSize;

    if(buf == NULL)
        return ;
    if((fd = Os__camera_init(320,240)) < 0)
    {

      if((fd = Os__camera_init(320,240)) < 0)
        Os__camera_init(320,240);

    }

}

CameraTest::~CameraTest()
{
    free(buf);
    delete ui;
}

void CameraTest::slot_quit(void)
{
    Os__camera_close();
    flag = false;
    close();
}

void CameraTest::slot_rephoto(void)
{
    int fd;
    if(buf == NULL)
        return ;
    if((fd = Os__camera_init(320,240)) < 0)
    {

      if((fd = Os__camera_init(320,240)) < 0)
        Os__camera_init(320,240);

    }
    flag = false;
    exitBtn->hide();
    rephotoBtn->hide();

}


void CameraTest::keyPressEvent(QKeyEvent *e)
{
    int fd;
    switch(e->key())
    {
        case Qt::Key_Camera:
        {
        if(onceFLag) return;

        onceFLag = true;

        if(flag)
        {
            ini_data();

            exitBtn->hide();
            rephotoBtn->hide();

            flag = false;
            QTimer::singleShot(400,this,SLOT(slot_changeFlagFalse()));

            return ;
        }


        int imgSize = Os__camera_capture(buf,bufSize);
        if(imgSize > 0)
        {
            if(Os__camera_getBMP((unsigned char*)buf,imgSize,"testaaa.bmp",imgWidth,imgHeight) != 0)
            {
                _print("RGB24 to bmp error\n");
                onceFLag = false;
                return ;
            }

            Os__camera_close();

            this->repaint();
            update();

            QLabel *imageLabel = new QLabel(this);
            imageLabel->setScaledContents(true);
            QImage image("./image/testaaa.bmp");

            imageLabel->setGeometry(20,30,200,220);
            image.scaled(imageLabel->width(),imageLabel->height(),Qt::KeepAspectRatio);

            QImage *imgRotate = new QImage;
            QMatrix matrix;
            matrix.rotate(90);
            *imgRotate = image.transformed(matrix);
            imageLabel->setPixmap(QPixmap::fromImage(*imgRotate));
            imageLabel->show();

            exitBtn->show();
            rephotoBtn->show();
            flag = true;

            QTimer::singleShot(400,this,SLOT(slot_changeFlagFalse()));

          }

         }
            break;
        case Qt::Key_Escape:
            Os__camera_close();
            flag = false;
            close();
        break;
    case Qt::Key_Enter:
        if(!rephotoBtn->isHidden())
             slot_rephoto();

        break;
    }
}




void CameraTest::slot_changeFlagFalse()
{
    onceFLag = false;
}
