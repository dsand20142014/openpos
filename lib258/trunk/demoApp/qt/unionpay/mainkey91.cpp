#include "mainkey91.h"
#include "ui_mainkey91.h"

mainkey91::mainkey91(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey91)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    //	ui->showFullScreen();
}

mainkey91::~mainkey91()
{
    delete ui;
}

void mainkey91::on_pushButton_7_clicked()
{
destroy();
}
