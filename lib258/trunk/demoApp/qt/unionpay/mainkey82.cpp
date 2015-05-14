#include "mainkey82.h"
#include "ui_mainkey82.h"

mainkey82::mainkey82(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey82)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
        //ui->showFullScreen();
}

mainkey82::~mainkey82()
{
    delete ui;
}

void mainkey82::on_pushButton_4_clicked()
{
    destroy();
}
