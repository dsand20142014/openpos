#include "mainkey8.h"
#include "ui_mainkey8.h"
#include "QTime"
#include "include.h"
#include "global.h"
#include "xdata.h"

MainKey8::MainKey8(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainKey8)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose);


    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    setData();
    ui->pushButton_3->setFocus();


}

void MainKey8::setData()
{
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    ui->lineEdit->setText(date.toString("MM-dd yyyy"));
    ui->lineEdit_3->setText(time.toString("hh:mm"));

}

MainKey8::~MainKey8()
{
    delete ui;
}

void MainKey8::on_pushButton_5_clicked()
{
	Load_Default_Keys();
}

void MainKey8::on_pushButton_9_clicked()
{
    Load_Default_Keys_Check();
}



void MainKey8::on_pushButton_4_clicked()
{
    accept();
}

void MainKey8::on_pushButton_3_clicked()
{
    destroy();
}
}
