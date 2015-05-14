#include "mainkey81.h"
#include "ui_mainkey81.h"

mainkey81::mainkey81(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey81)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    //ui->showFullScreen();
}

mainkey81::~mainkey81()
{
    delete ui;
}

void mainkey81::on_pushButton_5_clicked()
{
    destroy();
}
