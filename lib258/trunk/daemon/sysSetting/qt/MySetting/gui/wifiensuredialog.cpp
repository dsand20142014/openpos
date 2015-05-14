#include "wifiensuredialog.h"
#include "ui_wifiensuredialog.h"

WifiEnsureDialog::WifiEnsureDialog(QString title,int strength,QWidget *parent) :
    QDialog(parent,Qt::FramelessWindowHint),
    ui(new Ui::WifiEnsureDialog)
{
    ui->setupUi(this);

    ui->label->setText(title);

    switch(strength)
    {
    case 1:
        ui->label_3->setPixmap(QPixmap("/daemon/image/icon/lockwifi-1.png"));
        break;
    case 2:
        ui->label_3->setPixmap(QPixmap("/daemon/image/icon/lockwifi-2.png"));
        break;
    case 3:
        ui->label_3->setPixmap(QPixmap("/daemon/image/icon/lockwifi-3.png"));
        break;
    case 4:
        ui->label_3->setPixmap(QPixmap("/daemon/image/icon/lockwifi-4.png"));
        break;
    }

//    ui->label_3->setText(strength);

    ui->pushButton_4->setFocus();
    int screenWidth = qApp->desktop()->screen(0)->width();//240
    int screenHeight = qApp->desktop()->screen(0)->height();//320

    int width = (210*screenWidth)/240;
    int height = (130*screenHeight)/320;

    this->setGeometry((screenWidth-width)/2,(screenHeight-height)/2,width,height);

//    this->setStyleSheet("QPushButton{background-color:white;color:black;} QPushButton:focus{background-color:rgb(171,211,60);}");

}

WifiEnsureDialog::~WifiEnsureDialog()
{
    delete ui;
}


void WifiEnsureDialog::on_pushButton_3_clicked()//remove
{
    done(2);
}


void WifiEnsureDialog::on_pushButton_4_clicked()//connect
{
      done(1);
}

void WifiEnsureDialog::on_pushButton_5_clicked()//cancel
{
    done(0);
}
