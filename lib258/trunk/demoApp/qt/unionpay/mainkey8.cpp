#include "mainkey8.h"
#include "mainkey81.h"
#include "mainkey82.h"
#include "mainkey931.h"

#include "ui_mainkey8.h"
#include "pro_quarybal.h"

#include "msginfo.h"
#include "mythread.h"
#include "include.h"
#include "global.h"
mainkey8::mainkey8(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey8)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    //ui->showFullScreen();
}

mainkey8::~mainkey8()
{
    delete ui;
}

void mainkey8::on_pushButton_1_pressed()
{
    //    mainkey81 *desk = new mainkey81(this);
    //    desk->show();
    //    desk->showFullScreen();
    mainkey81 desk;
    desk.exec();
    desk.showFullScreen();
}

void mainkey8::on_pushButton_2_pressed()
{
    //    mainkey82 *desk = new mainkey82(this);
    //    desk->show();
    //    desk->showFullScreen();
    mainkey82 desk;
    desk.exec();
    desk.showFullScreen();
}


void mainkey8::on_pushButton_3_clicked()
{
    //    Pro_QuaryBal *desk = new Pro_QuaryBal(this);
    //    desk->show();
    //    desk->showFullScreen();

    Pro_QuaryBal desk;
    desk.exec();
    desk.showFullScreen();
}

void mainkey8::on_pushButton_8_clicked()
{
    destroy();
}

void mainkey8::on_pushButton_4_clicked()
{
    // RollBack Test

    ProUiFace.UiToPro.ucTransType = TRANS_ONLINETEST;
//    MyThread *a=new MyThread();
//    a->start();
    MsgInfo *desk=new MsgInfo();
    desk->exec();
    desk->showFullScreen();
}

void mainkey8::on_pushButton_2_clicked()
{
    // ZHIFU CHEXIAO
}

void mainkey8::on_pushButton_5_clicked()
{
    // Change ComModle
	mainkey931 desk;
	desk.exec();
	desk.showFullScreen();

}

}
