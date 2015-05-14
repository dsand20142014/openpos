#include "wifi.h"
#include "ui_wifi.h"

Wifi::Wifi(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Wifi)
{
    ui->setupUi(this);
    connect(ui->rbOyes,SIGNAL(toggled(bool)),this,SLOT(radioButtonDeal(bool)));
    settings = new QSettings("setting.ini",QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
    displayValue();
}

Wifi::~Wifi()
{
    delete ui;
}

void Wifi::radioButtonDeal(bool isActived)
{
    if(isActived)
    {
        isActiveFlag = true;
        //ui->leName->setEnabled(true);
        //ui->pbNext->setEnabled(true);
    }
    else
    {
        isActiveFlag = false;
        //ui->leName->setEnabled(false);
        //ui->pbNext->setEnabled(false);
    }
}

//void Wifi::pwdDeal(QString str)
//{
//    QString strXing;
//    strPwd += str.right(1);
//    strXing = "";
//    for(int i=0; i<strPwd.length(); i++)
//    {
//        strXing += "*";
//    }
//    ui->lePwd->setText(strXing);
//}

void Wifi::displayValue(void)
{
    readSetting.clear();
    readSetting.readKeys(settings,"WIFI_SETTING");
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "08.ACTIVE")
        {
            if(readSetting.keysValue[i] == "1")
            {
                isActiveFlag = true;
                ui->rbOyes->setChecked(true);
                //ui->pbNext->setEnabled(true);
                //ui->leName->setEnabled(true);
            }
            if(readSetting.keysValue[i] == "0")
            {
                isActiveFlag = false;
                ui->rbOno->setChecked(true);
                //ui->pbNext->setEnabled(false);
                //ui->leName->setEnabled(false);
            }
        }
//        if(isActiveFlag)
//        {
            if(readSetting.keys[i] == "01.AP_NAME")
            {
                if(readSetting.keysValue[i] == "NULL")
                {
                    ui->leName->setText("");
                    ui->leName->setEnabled(true);
                }
                else
                {
                    ui->leName->setText(readSetting.keysValue[i]);
                    ui->leName->setEnabled(true);
                }
            }
//        }
        /*else
        {
            if(readSetting.keys[i] == "01.AP_NAME")
            {
                if(readSetting.keysValue[i] == "NULL")
                {
                    ui->leName->setText("");
                    ui->leName->setEnabled(false);
                }
                else
                {
                    ui->leName->setText(readSetting.keysValue[i]);
                    ui->leName->setEnabled(false);
                }
            }
        }*/
//        if(readSetting.keys[i] == "06.PASSWD")
//        {
//            if(readSetting.keysValue[i] == "NULL")
//            {
//                ui->lePwd->setText("");
//            }
//            else
//            {
//                //ui->lePwd->setText(readSetting.keysValue[i]);
//                ui->lePwd->setText("");
//            }
//        }
    }
}

void Wifi::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
}

void Wifi::on_pbNext_clicked()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "01.AP_NAME")
        {
            //readSetting.keysValueBuffer.replace(i,ui->leName->text());
            if(ui->leName->text().isEmpty())
            {
                readSetting.keysValueBuffer[i] = "NULL";
            }
            else
            {
                readSetting.keysValueBuffer[i] = ui->leName->text();
            }
        }
        if(readSetting.keys[i] == "08.ACTIVE")
        {
            if(ui->rbOyes->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
            if(ui->rbOno->isChecked())
            {
                readSetting.keysValueBuffer[i] = "0";
            }
        }
//        if(readSetting.keys[i] == "05.PASSWD")
//        {
//            //readSetting.keysValueBuffer[i].replace(i,ui->lePwd->text());
//            if(ui->lePwd->text().isEmpty())
//            {
//                readSetting.keysValueBuffer[i] = "NULL";
//            }
//            else
//            {
//                readSetting.keysValueBuffer[i] = strPwd;
//            }
//        }
    }
    Wifi2 *wifi2 = new Wifi2(settings,this);
    connect(wifi2,SIGNAL(closeParentWidget()),this,SLOT(close()));
    //wifi2->show();
    wifi2->showFullScreen();
}
