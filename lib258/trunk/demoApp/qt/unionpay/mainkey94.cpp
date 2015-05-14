#include "mainkey94.h"
#include "ui_mainkey94.h"
#include "msginfo.h"

#include "mythread.h"
#include "include.h"
#include "global.h"
#include "toolslib.h"

mainkey94::mainkey94(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey94)
{
    ui->setupUi(this);
    
    setWindowFlags(Qt::FramelessWindowHint);
        //ui->showFullScreen();
}

mainkey94::~mainkey94()
{
    delete ui;
}

void mainkey94::on_pushButton_3_clicked()
{
    destroy();
}

void mainkey94::on_pushButton_clicked()
{


    //交易类型 AID Param
    ProUiFace.UiToPro.ucTransType = TRANS_AID_PARAM;
    // 启动线程 处理交易
    MsgInfo *desk=new MsgInfo();
    desk->show();
    desk->showFullScreen();
//    MyThread *a=new MyThread();
//    a->start();
}

void mainkey94::on_pushButton_2_clicked()
{


    //交易类型 RID Param
    ProUiFace.UiToPro.ucTransType = TRANS_RID_PARAM;
    // 启动线程 处理交易
//    MyThread *a=new MyThread();
//    a->start();
    MsgInfo *desk=new MsgInfo();
    desk->show();
    desk->showFullScreen();
}

}
