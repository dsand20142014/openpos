#include "wifi3.h"
#include "ui_wifi3.h"

#include <QValidator>
#include <QRegExp>

Wifi3::Wifi3(QSettings *setting, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Wifi3)
{
    ui->setupUi(this);
    connect(ui->rbDyes,SIGNAL(toggled(bool)),this,SLOT(radioButtonDeal(bool)));
    settings = setting;
    displayValue();
}

Wifi3::~Wifi3()
{
    delete ui;
}

void Wifi3::radioButtonDeal(bool isChecked)
{
    if(isChecked)
    {
        ui->leIp->setEnabled(false);
        ui->leMask->setEnabled(false);
        ui->leRouter->setEnabled(false);
    }
    else
    {
        ui->leIp->setEnabled(true);
        ui->leMask->setEnabled(true);
        ui->leRouter->setEnabled(true);
    }
}

void Wifi3::displayValue(void)
{
    QString strIP,strMask,strRouter;
    //QRegExp regExp("[0-2][0-5][0-5]");
    QRegExp regExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");

    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "02.DHCP")
        {
            if(readSetting.keysValue[i] == "0")
            {
                dhcpFlag = false;
                ui->rbDno->setChecked(true);
            }
            if(readSetting.keysValue[i] == "1")
            {
                dhcpFlag = true;
                ui->rbDyes->setChecked(true);
            }
        }
        if(!dhcpFlag)
        {
            if(readSetting.keys[i] == "03.MACHINE_IP")
            {
                strIP = readSetting.keysValue[i];
                ui->leIp->setValidator(new QRegExpValidator(regExp,this));
                ui->leIp->setInputMask("000.000.000.000");
                ui->leIp->setText(strIP);
            }
            if(readSetting.keys[i] == "04.MASK")
            {
                strMask = readSetting.keysValue[i];
                ui->leMask->setValidator(new QRegExpValidator(regExp,this));
                ui->leMask->setInputMask("000.000.000.000");
                ui->leMask->setText(strMask);
            }
            if(readSetting.keys[i] == "05.ROUTER")
            {
                strRouter = readSetting.keysValue[i];
                ui->leRouter->setValidator(new QRegExpValidator(regExp,this));
                ui->leRouter->setInputMask("000.000.000.000");
                ui->leRouter->setText(strRouter);
            }
        }
        else
        {
            if(readSetting.keys[i] == "03.MACHINE_IP")
            {
                strIP = readSetting.keysValue[i];
                ui->leIp->setValidator(new QRegExpValidator(regExp,this));
                ui->leIp->setInputMask("000.000.000.000");
                ui->leIp->setText(strIP);
                ui->leIp->setEnabled(false);
            }
            if(readSetting.keys[i] == "04.MASK")
            {
                strMask = readSetting.keysValue[i];
                ui->leMask->setValidator(new QRegExpValidator(regExp,this));
                ui->leMask->setInputMask("000.000.000.000");
                ui->leMask->setText(strMask);
                ui->leMask->setEnabled(false);
            }
            if(readSetting.keys[i] == "05.ROUTER")
            {
                strRouter = readSetting.keysValue[i];
                ui->leRouter->setValidator(new QRegExpValidator(regExp,this));
                ui->leRouter->setInputMask("000.000.000.000");
                ui->leRouter->setText(strRouter);
                ui->leRouter->setText(false);
            }
        }
    }
}

void Wifi3::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
    emit(closeParentWidget());
}

void Wifi3::on_pbEnter_clicked()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "02.DHCP")
        {
            if(ui->rbDno->isChecked())
            {
                readSetting.keysValueBuffer[i] = "0";
            }
            if(ui->rbDyes->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
        }
//        if(!dhcpFlag)
//        {
            if(readSetting.keys[i] == "03.MACHINE_IP")
            {
                readSetting.keysValueBuffer[i] = ui->leIp->text();
            }
            if(readSetting.keys[i] == "04.MASK")
            {
                readSetting.keysValueBuffer[i] = ui->leMask->text();
            }
            if(readSetting.keys[i] == "05.ROUTER")
            {
                readSetting.keysValueBuffer[i] = ui->leRouter->text();
            }
//        }
    }
    readSetting.keysValue = readSetting.keysValueBuffer;
    readSetting.setKeys(settings,"WIFI_SETTING");
    readSetting.clear();

    close();
    emit(closeParentWidget());
}
