#include "wifipswdialog.h"
#include "ui_wifipswdialog.h"
#include "wifi.h"

WifiPswDialog::WifiPswDialog(QString title,QString strength,QWidget *parent) :
    QDialog(parent,Qt::FramelessWindowHint),
    ui(new Ui::WifiPswDialog)
{
    ui->setupUi(this);
    curPsw = "";
    ui->label->setText(title);
    ui->label_3->setText(strength);
    ui->label_6->hide();
    ui->lineEdit->setFocus();
    int screenWidth = qApp->desktop()->screen(0)->width();//240
    int screenHeight = qApp->desktop()->screen(0)->height();//320
    setGeometry((screenWidth-width())/2,(screenHeight-height())/2,width(),height());

    this->setStyleSheet("QPushButton{background-color:white;color:black;} QPushButton:focus{background-color:rgb(171,211,60);}");
}

WifiPswDialog::~WifiPswDialog()
{
    delete ui;
}

void WifiPswDialog::on_pushButton_clicked()//connect
{
    if(ui->lineEdit->text().length()<8){
        ui->label_6->show();
        QTimer::singleShot(2000,ui->label_6,SLOT(hide()));
        ui->lineEdit->setFocus();
        return;
    }

    curPsw = ui->lineEdit->text();
    accept();
}

void WifiPswDialog::on_pushButton_2_clicked()
{
    reject();
}

//void WifiPswDialog::hidePswLine()
//{
//    ui->lineEdit->setEnabled(false);
//}
