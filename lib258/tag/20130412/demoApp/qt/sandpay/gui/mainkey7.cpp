#include "mainkey7.h"
#include "ui_mainkey7.h"
#include "msginfoui.h"
#include <QtDebug>
#include "include.h"
#include "global.h"

MainKey7::MainKey7(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainKey7)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    myThread= 0;
    ui->pushButton->setFocus();

}

MainKey7::~MainKey7()
{
    if(myThread!=NULL){

         delete myThread;
         myThread = NULL;
    }

    delete ui;
}

void MainKey7::on_pushButton_clicked()
{
    MsgInfoUI  *desk = new MsgInfoUI(this);
    desk->showFullScreen();

    if(myThread!=NULL){

         delete myThread;
         myThread = NULL;
    }
    myThread = new MyThread(3);
    myThread->start();



}

void MainKey7::on_pushButton_2_clicked()
{
    MsgInfoUI  *desk = new MsgInfoUI(this);
    desk->showFullScreen();

    if(myThread!=NULL){

         delete myThread;
         myThread = NULL;
    }

    myThread =new MyThread(4);
    myThread->start();


}

void MainKey7::on_pushButton_3_clicked()
{
    MsgInfoUI  *desk = new MsgInfoUI(this);
    desk->showFullScreen();

    Untouch_DX_Process();
}

void MainKey7::on_pushButton_4_clicked()
{
    destroy();
}

}
