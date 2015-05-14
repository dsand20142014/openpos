#include "mainkey7.h"
#include "ui_mainkey7.h"
#include "mainkey931.h"
#include "mainkey73.h"
#include "msginfo.h"
#include "mythread.h"

#include "pro_settle.h"

extern "C"{
#include "include.h"
#include "global.h"
}

mainkey7::mainkey7(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey7)
{
	ui->setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
    this->showFullScreen();
    g_ProgressBarFlag = 0;
}

mainkey7::~mainkey7()
{
    delete ui;
}

void mainkey7::on_pushButton_9_clicked()
{
    g_ProgressBarFlag = 0;
    qDebug("000000000000000000000000000000000000000000000000000000");
    destroy();
}

void mainkey7::on_pushButton_1_clicked()
{
    g_ProgressBarFlag = 1;
    ProUiFace.UiToPro.ucTransType = TRANS_LOGONON;
    MsgInfo *desk=new MsgInfo();
    desk->show();
    desk->showFullScreen();

        qDebug("%d####################################################",ProUiFace.UiToPro.ucTransType);
//    MyThread *a=new MyThread();
//    a->start();
}

void mainkey7::on_pushButton_2_clicked()
{

    g_ProgressBarFlag = 1;
    ProUiFace.UiToPro.ucTransType = TRANS_LOGONOFF;
    MsgInfo *desk=new MsgInfo();
    desk->show();
    desk->showFullScreen();
//    MyThread *a=new MyThread();
//    a->start();
}
void mainkey7::on_pushButton_4_clicked()
{
// Show Static Info.


    ProUiFace.UiToPro.ucTransType = TRANS_SHOW_STATIC;
    MsgInfo *desk=new MsgInfo();
    desk->show();
    desk->showFullScreen();
//    MyThread *a=new MyThread();
//    a->start();
}

void mainkey7::on_pushButton_5_clicked()
{
// waixian bohao
//    mainkey931 *desk=new mainkey931();
//    desk->show();
//    desk->showFullScreen();
}

void mainkey7::on_pushButton_clicked()
{
    UITL_Delete_Reversal_Flag();
}

void mainkey7::on_pushButton_6_clicked()
{
// Settle
    Pro_Settle *desk=new Pro_Settle();
    desk->show();
    desk->showFullScreen();
}

void mainkey7::on_pushButton_8_clicked()
{
// Show Version

    ProUiFace.UiToPro.ucTransType = TRANS_SHOW_VERSION;
//    MyThread *a=new MyThread();
//    a->start();
    MsgInfo *desk=new MsgInfo();
    desk->show();
    desk->showFullScreen();
}



void mainkey7::on_pushButton_3_clicked()
{
    // Show TransDetail
    MainKey73 *desk=new MainKey73();
    desk->show();
    desk->showFullScreen();
}


}
