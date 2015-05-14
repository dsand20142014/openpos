#include "gps.h"
#include "ui_gps.h"

Gps::Gps(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Gps)
{
    ui->setupUi(this);
    connect(ui->rbYes,SIGNAL(toggled(bool)),this,SLOT(radioButtonDeal(bool)));
    settings = new QSettings("setting.ini",QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
    displayValue();
}

Gps::~Gps()
{
    delete ui;
}

void Gps::radioButtonDeal(bool isChecked)
{
    if(!isChecked)
    {
        ui->rbCm->setEnabled(false);
        ui->rbWm->setEnabled(false);
        ui->pbEnter->setEnabled(false);
    }
    else
    {
        ui->rbCm->setEnabled(true);
        ui->rbWm->setEnabled(true);
        ui->pbEnter->setEnabled(true);
    }
}

void Gps::displayValue(void)
{
    readSetting.clear();
    readSetting.readKeys(settings,"GPS_SETTING");
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "02.ACTIVE")
        {
            if(readSetting.keysValue[i] == "1")
            {
                isActiveFlag = true;
                ui->rbYes->setChecked(true);
                ui->pbEnter->setEnabled(true);
            }
            if(readSetting.keysValue[i] == "0")
            {
                isActiveFlag = false;
                ui->rbNo->setChecked(true);
                ui->pbEnter->setEnabled(false);
            }
        }
        if(readSetting.keys[i] == "01.MODE")
        {
            if(isActiveFlag)
            {
                if(readSetting.keysValue[i] == "1")
                {
                    ui->rbCm->setChecked(true);
                    ui->rbCm->setEnabled(true);
                }
                if(readSetting.keysValue[i] == "2")
                {
                    ui->rbWm->setChecked(true);
                    ui->rbWm->setEnabled(true);
                }
            }
            else
            {
                if(readSetting.keysValue[i] == "1")
                {
                    ui->rbCm->setChecked(true);
                    ui->rbCm->setEnabled(false);
                }
                if(readSetting.keysValue[i] == "2")
                {
                    ui->rbWm->setChecked(true);
                    ui->rbWm->setEnabled(false);
                }
            }
        }
    }
}

void Gps::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
}

void Gps::on_pbEnter_clicked()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "02.ACTIVE")
        {
            if(ui->rbYes->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
            if(ui->rbNo->isChecked())
            {
                readSetting.keysValueBuffer[i] = "0";
            }
        }
        if(readSetting.keys[i] == "01.MODE")
        {
            if(ui->rbCm->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
            if(ui->rbWm->isChecked())
            {
                readSetting.keysValueBuffer[i] = "2";
            }
        }
    }
    readSetting.keysValue = readSetting.keysValueBuffer;
    readSetting.setKeys(settings,"GPS_SETTING");
    readSetting.clear();

    close();
}
