#include "mainkey93.h"
#include "mainkey931.h"
#include "ui_mainkey93.h"

#include "include.h"

mainkey93::mainkey93(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey93)
{
    ui->setupUi(this);
    
    setWindowFlags(Qt::FramelessWindowHint);
        //ui->showFullScreen();
}

mainkey93::~mainkey93()
{
    delete ui;
}
void mainkey93::on_pushButton_9_clicked()
{
    destroy();
}

void mainkey93::on_pushButton_2_clicked()
{
    EX_KEY_InputKey_Double(0);
}


void mainkey93::on_pushButton_clicked()
{
    mainkey931 desk;
    desk.exec();
    desk.showFullScreen();
}

}
