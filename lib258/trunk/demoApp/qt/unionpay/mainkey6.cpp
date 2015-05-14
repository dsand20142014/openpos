#include "mainkey6.h"
#include "ui_mainkey6.h"
#include "pro_reprint.h"
#include "msginfo.h"

#include "mythread.h"
#include "include.h"
#include "global.h"
#include "toolslib.h"

mainkey6::mainkey6(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey6)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    //ui->showFullScreen();
}

mainkey6::~mainkey6()
{
    delete ui;
}

void mainkey6::on_pushButton_6_clicked()
{
    destroy();
}

void mainkey6::on_pushButton_2_clicked()
{
    //Print Every Trans
    Pro_Reprint desk;
    desk.exec();
//    desk.show();
//    desk.showFullScreen();
}

void mainkey6::on_pushButton_1_clicked()
{
    //Print LastTrans
    ProUiFace.UiToPro.ucTransType = TRANS_PRINT_LASTTRANS;
//    MyThread *a=new MyThread();
//    a->start();

    MsgInfo *desk=new MsgInfo();
    desk->exec();
//    desk->show();
//    desk->showFullScreen();

}
void mainkey6::on_pushButton_3_clicked()
{
    //Print LastSettle 重打印结算单据
    ProUiFace.UiToPro.ucTransType = TRANS_PRINT_LASTSETTLE;
//    MyThread *a=new MyThread();
//    a->start();

    MsgInfo *desk=new MsgInfo();
    desk->exec();
//    desk->show();
//    desk->showFullScreen();

}

void mainkey6::on_pushButton_4_clicked()
{
    //Print Detail 重打印交易明细
    ProUiFace.UiToPro.ucTransType = TRANS_PRINT_TRANSDETAIL;
//    MyThread *a=new MyThread();
//    a->start();

    MsgInfo *desk=new MsgInfo();
    desk->exec();
//    desk->show();
//    desk->showFullScreen();

}

void mainkey6::on_pushButton_5_clicked()
{
    //Print Static 重打印交易汇总
    ProUiFace.UiToPro.ucTransType = TRANS_PRINT_STATIC;
//    MyThread *a=new MyThread();
//    a->start();
    MsgInfo *desk=new MsgInfo();
    desk->exec();
    //    desk->show();
    //    desk->showFullScreen();

}


}
