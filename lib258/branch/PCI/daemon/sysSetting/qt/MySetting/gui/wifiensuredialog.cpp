#include "wifiensuredialog.h"
#include "ui_wifiensuredialog.h"

WifiEnsureDialog::WifiEnsureDialog(QString title,QString strength,QWidget *parent) :
    QDialog(parent,Qt::FramelessWindowHint),
    ui(new Ui::WifiEnsureDialog)
{
    ui->setupUi(this);
    ui->label->setText(title);
    ui->label_3->setText(strength);

    ui->pushButton->setFocus();
    int screenWidth = qApp->desktop()->screen(0)->width();//240
    int screenHeight = qApp->desktop()->screen(0)->height();//320
    setGeometry((screenWidth-width())/2,(screenHeight-height())/2,width(),height());

    this->setStyleSheet("QPushButton{background-color:white;color:black;} QPushButton:focus{background-color:rgb(171,211,60);}");

}

WifiEnsureDialog::~WifiEnsureDialog()
{
    delete ui;
}

void WifiEnsureDialog::on_pushButton_clicked()//connect
{
    done(1);
}

void WifiEnsureDialog::on_pushButton_3_clicked()//remove
{
    done(0);
}

void WifiEnsureDialog::on_pushButton_2_clicked()//cancel
{
    done(2);
}
