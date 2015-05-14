#include "mainkey5.h"
#include "ui_mainkey5.h"
#include "pro_51_authfinishoff.h"
#include "pro_52_tipadjust.h"
#include "pro_53_offcancel.h"


mainkey5::mainkey5(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey5)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    //ui->showFullScreen();
}

mainkey5::~mainkey5()
{
    delete ui;
}

void mainkey5::on_pushButton_4_clicked()
{
    destroy();
}

void mainkey5::on_pushButton_1_clicked()
{
//[Foreign] lixian Settle
    Pro_51_AuthFinishOff desk;
    desk.exec();
    desk.showFullScreen();
}

void mainkey5::on_pushButton_2_clicked()
{
    //[Foreign] lixian adjust tip
    Pro_52_TipAdjust desk;
    desk.exec();
    desk.showFullScreen();

}

void mainkey5::on_pushButton_3_clicked()
{
    //[Foreign] lixian Cancel
    Pro_53_OffCancel desk;
    desk.exec();
    desk.showFullScreen();

}
