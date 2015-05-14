#include "mainkey9.h"
#include "mainkey91.h"
#include "mainkey92.h"
#include "mainkey93.h"
#include "mainkey94.h"
#include "ui_mainkey9.h"

mainkey9::mainkey9(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey9)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
}

mainkey9::~mainkey9()
{
    delete ui;
}

void mainkey9::on_pushButton_1_pressed()
{
//    mainkey91 *desk = new mainkey91();
//    desk->show();
//    desk->showFullScreen();

    mainkey91 desk;
    desk.exec();
    desk.showFullScreen();
}

void mainkey9::on_pushButton_2_pressed()
{
//    mainkey92 *desk = new mainkey92();
//    desk->show();
//    desk->showFullScreen();

    mainkey92 desk;
    desk.exec();
    desk.showFullScreen();
}

void mainkey9::on_pushButton_3_pressed()
{
//    mainkey93 *desk = new mainkey93();
//    desk->show();
//    desk->showFullScreen();
    mainkey93 desk;
    desk.exec();
    desk.showFullScreen();
}



void mainkey9::on_pushButton_4_clicked()
{
    mainkey94 desk;
    desk.exec();
    desk.showFullScreen();
}
void mainkey9::on_pushButton_Close_clicked()
{
    destroy();
}
