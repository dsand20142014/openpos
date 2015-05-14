#include "wifipswdialog.h"
#include "ui_wifipswdialog.h"
#include "wifi.h"

WifiPswDialog::WifiPswDialog(QString title,int strength,QWidget *parent) :
    QDialog(parent,Qt::FramelessWindowHint),
    ui(new Ui::WifiPswDialog)
{
    ui->setupUi(this);

    curPsw = "";
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

    ui->lineEdit->setFocus();

    int screenWidth = qApp->desktop()->screen(0)->width();//240
    int screenHeight = qApp->desktop()->screen(0)->height();//320


    int width = (210*screenWidth)/240;
    int height = (200*screenHeight)/320;

    this->setGeometry((screenWidth-width)/2,(screenHeight-height)/2,width,height);

    connect(ui->checkBox,SIGNAL(toggled(bool)),SLOT(showPswOrNot(bool)));

}

void WifiPswDialog::showPswOrNot(bool radioFlag)
{
    if(radioFlag)
        ui->lineEdit->setEchoMode(QLineEdit::Normal);
    else
        ui->lineEdit->setEchoMode(QLineEdit::Password);

    ui->lineEdit->setFocus();

}

WifiPswDialog::~WifiPswDialog()
{
    delete ui;
}

void WifiPswDialog::on_pushButton_clicked()//connect
{
    if(ui->lineEdit->text().length()<8){
        ui->label_6->setText(tr("   password length is too short!"));
        QTimer::singleShot(2000,ui->label_6,SLOT(clear()));
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
