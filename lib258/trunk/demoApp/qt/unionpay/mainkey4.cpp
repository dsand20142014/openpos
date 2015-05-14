#include "mainkey4.h"
#include "ui_mainkey4.h"
#include "pro_preauth.h"
#include "pro_cancelauthfinish.h"
#include "pro_cancelauth.h"
#include "pro_authfinishmsg.h"
#include "pro_authfinish.h"
#include "pro_51_authfinishoff.h"
#include "pro_52_tipadjust.h"
#include "pro_53_offcancel.h"


mainkey4::mainkey4(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey4)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
}

mainkey4::~mainkey4()
{
    delete ui;
}

void mainkey4::on_pushButton_7_clicked()
{
    destroy();
}

void mainkey4::on_pushButton_1_clicked()
{
   // Pro_PreAuth *desk=new Pro_PreAuth();
   // desk->show();
   // desk->showFullScreen();

    Pro_PreAuth desk;
    desk.exec();
    desk.showFullScreen();
}

void mainkey4::on_pushButton_3_clicked()
{
   //Pro_AuthFinish *desk=new Pro_AuthFinish();
  //  desk->show();
   // desk->showFullScreen();

    Pro_AuthFinish desk;
    desk.exec();
    desk.showFullScreen();
	
}


void mainkey4::on_pushButton_4_clicked()
{
   // Pro_AuthFinishMsg *desk=new Pro_AuthFinishMsg();
   // desk->show();
   // desk->showFullScreen();
    Pro_AuthFinishMsg desk;
    desk.exec();
    desk.showFullScreen();

}

void mainkey4::on_pushButton_5_clicked()
{
    //Pro_CancelAuth *desk=new Pro_CancelAuth();
    //desk->show();
    //desk->showFullScreen();
    Pro_CancelAuth desk;
    desk.exec();
    desk.showFullScreen();

}

void mainkey4::on_pushButton_6_clicked()
{
  //  Pro_CancelAuthFinish *desk=new Pro_CancelAuthFinish();
  //  desk->show();
  //  desk->showFullScreen();

    Pro_CancelAuthFinish desk;
    desk.exec();
    desk.showFullScreen();
	
}
