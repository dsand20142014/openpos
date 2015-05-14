#include "mainmenudialog.h"
#include "ui_mainmenudialog.h"

#include "MainKey5.h"
#include "MainKey8.h"
#include "mainkey1.h"
#include "mainkey3.h"
#include "mainkey4.h"
#include "mainkey6.h"
#include "mainkey7.h"
#include <QtDebug>
#include "msginfoui.h"

#include "include.h"
#include "global.h"
#include "xdata.h"

MainMenuDialog::MainMenuDialog(QWidget *parent) :
    QDialog(parent,Qt::CustomizeWindowHint),
    ui(new Ui::MainMenuDialog)
{
    ui->setupUi(this);


    /*加背景图片*/
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    myThread = 0;
    ui->pushButton->setFocus();

}

MainMenuDialog::~MainMenuDialog()
{
    if(myThread!=NULL){

         delete myThread;
         myThread = NULL;
    }

    delete ui;

}

void MainMenuDialog::on_pushButton_clicked()//签到
{
    MainKey1 *desk = new MainKey1(this);
    desk->showFullScreen();

}

void MainMenuDialog::on_pushButton_2_clicked()//结算
{
    MsgInfoUI  *desk = new MsgInfoUI(this);
    desk->showFullScreen();

    if(myThread!=NULL){
         delete myThread;
         myThread = NULL;
    }

    ProUiFace.UiToPro.ucTransType= TRANS_S_BATCHUP;

    myThread = new MyThread();
    myThread->start();
    connect(myThread,SIGNAL(signal_mythread()),desk,SLOT(deleteLater()));

}

void MainMenuDialog::on_pushButton_3_clicked()
{
    MainKey3 *desk = new MainKey3(this);
    desk->showFullScreen();


}

void MainMenuDialog::on_pushButton_4_clicked()
{
    MainKey4 *desk = new MainKey4(this);
    desk->showFullScreen();
}

void MainMenuDialog::on_pushButton_5_clicked()//设置
{
    MainKey5 *desk = new MainKey5(this);
    desk->showFullScreen();
}

void MainMenuDialog::on_pushButton_6_clicked()
{
    MainKey6 *desk = new MainKey6(this);
    desk->showFullScreen();
}

void MainMenuDialog::on_pushButton_7_clicked()
{
    MainKey7 *desk = new MainKey7(this);
    desk->showFullScreen();
}

void MainMenuDialog::on_pushButton_8_clicked()
{
    MainKey8 *desk = new MainKey8(this);
    desk->showFullScreen();
}

void MainMenuDialog::on_pushButton_9_clicked()
{
   emit signal();
   destroy();

}
}
