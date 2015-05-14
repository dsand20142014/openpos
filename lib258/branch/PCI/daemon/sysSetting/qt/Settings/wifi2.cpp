#include "wifi2.h"
#include "ui_wifi2.h"

#include <QDebug>

Wifi2::Wifi2(QSettings *setting, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Wifi2)
{
    ui->setupUi(this);
    connect(ui->lePwd,SIGNAL(textEdited(QString)),this,SLOT(pwdDeal(QString)));
    settings = setting;
    displayValue();
}

Wifi2::~Wifi2()
{
    delete ui;
}

void Wifi2::displayValue(void)
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
//        if(readSetting.keys[i] == "02.DHCP")
//        {
//            if(readSetting.keysValue[i] == "0")
//            {
//                ui->rbDno->setChecked(true);
//            }
//            if(readSetting.keysValue[i] == "1")
//            {
//                ui->rbDyes->setChecked(true);
//            }
//        }
        if(readSetting.keys[i] == "06.PASSWD")
        {
            if(readSetting.keysValue[i] == "NULL")
            {
                ui->lePwd->setText("");
            }
            else
            {
                //ui->lePwd->setText(readSetting.keysValue[i]);
                ui->lePwd->setText("");
            }
        }
        if(readSetting.keys[i] == "07.SECURITY")
        {
            if(readSetting.keysValue[i] == "1")
            {
                ui->rbSwep->setChecked(true);
            }
            if(readSetting.keysValue[i] == "2")
            {
                ui->rbSwpa->setChecked(true);
            }
            if(readSetting.keysValue[i] == "3")
            {
                ui->rbSwpa2->setChecked(true);
            }
            if(readSetting.keysValue[i] == "4")
            {
                ui->rbSwpaa->setChecked(true);
            }
        }
//        if(readSetting.keys[i] == "08.OPEN")
//        {
//            if(readSetting.keysValue[i] == "0")
//            {
//                ui->rbOno->setChecked(true);
//            }
//            if(readSetting.keysValue[i] == "1")
//            {
//                ui->rbOyes->setChecked(true);
//            }
//        }
    }
}

void Wifi2::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
    emit(closeParentWidget());
}

void Wifi2::closeWidget()
{
    close();
    emit(closeParentWidget());
}

void Wifi2::pwdDeal(QString str)
{
    QString strXing;
    if(strPwd.length() - str.length() < 0)
    {
        strPwd += str.right(1);
    }
    else
    {
        strPwd = strPwd.remove(strPwd.length()-1,1);
    }
    strXing = "";
    for(int i=0; i<strPwd.length(); i++)
    {
        strXing += "*";
    }
    ui->lePwd->setText(strXing);
    //ui->lePwd->setText(strPwd);
}

void Wifi2::on_pbNext_clicked()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
//        if(readSetting.keys[i] == "02.DHCP")
//        {
//            if(ui->rbDno->isChecked())
//            {
//                readSetting.keysValueBuffer[i] = "0";
//            }
//            if(ui->rbDyes->isChecked())
//            {
//                readSetting.keysValueBuffer[i] = "1";
//            }
//        }
        if(readSetting.keys[i] == "07.SECURITY")
        {
            if(ui->rbSwep->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
            if(ui->rbSwpa->isChecked())
            {
                readSetting.keysValueBuffer[i] = "2";
            }
            if(ui->rbSwpa2->isChecked())
            {
                readSetting.keysValueBuffer[i] = "3";
            }
            if(ui->rbSwpaa->isChecked())
            {
                readSetting.keysValueBuffer[i] = "4";
            }
        }
        if(readSetting.keys[i] == "06.PASSWD")
        {
            //readSetting.keysValueBuffer[i].replace(i,ui->lePwd->text());
            if(ui->lePwd->text().isEmpty())
            {
                readSetting.keysValueBuffer[i] = "NULL";
            }
            else
            {
                readSetting.keysValueBuffer[i] = strPwd;
            }
        }
//        if(readSetting.keys[i] == "08.OPEN")
//        {
//            if(ui->rbOno->isChecked())
//            {
//                readSetting.keysValueBuffer[i] = "0";
//            }
//            if(ui->rbOyes->isChecked())
//            {
//                readSetting.keysValueBuffer[i] = "1";
//            }
//        }
    }
    Wifi3 *wifi3 = new Wifi3(settings,this);
    connect(wifi3,SIGNAL(closeParentWidget()),this,SLOT(closeWidget()));
    //wifi3->show();
    wifi3->showFullScreen();
}
