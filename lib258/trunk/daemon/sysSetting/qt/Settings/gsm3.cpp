#include "gsm3.h"
#include "ui_gsm3.h"
#include <QDebug>

Gsm3::Gsm3(QSettings *setting, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Gsm3)
{
    ui->setupUi(this);
    settings = setting;
    displayValue();
}

Gsm3::~Gsm3()
{
    delete ui;
}

void Gsm3::displayValue(void)
{
     //readSetting.readKeys(settings,"GSM_SETTING");
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "06.DEFAULT_BAND")
        {
            if(readSetting.keysValue[i] == "1")
            {
                ui->rb850->setChecked(true);
            }
            if(readSetting.keysValue[i] == "2")
            {
                ui->rb900->setChecked(true);
            }
            if(readSetting.keysValue[i] == "3")
            {
                ui->rb1800->setChecked(true);
            }
            if(readSetting.keysValue[i] == "4")
            {
                ui->rb1900->setChecked(true);
            }
            if(readSetting.keysValue[i] == "5")
            {
                ui->rb8519->setChecked(true);
            }
            if(readSetting.keysValue[i] == "6")
            {
                ui->rb918->setChecked(true);
            }
            if(readSetting.keysValue[i] == "7")
            {
                ui->rb919->setChecked(true);
            }
        }
    }
}

void Gsm3::on_pbNext_clicked()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "06.DEFAULT_BAND")
        {
            if(ui->rb850->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
            if(ui->rb900->isChecked())
            {
                readSetting.keysValueBuffer[i] = "2";
            }
            if(ui->rb1800->isChecked())
            {
                readSetting.keysValueBuffer[i] = "3";
            }
            if(ui->rb1900->isChecked())
            {
                readSetting.keysValueBuffer[i] = "4";
            }
            if(ui->rb8519->isChecked())
            {
                readSetting.keysValueBuffer[i] = "5";
            }
            if(ui->rb918->isChecked())
            {
                readSetting.keysValueBuffer[i] = "6";
            }
            if(ui->rb919->isChecked())
            {
                readSetting.keysValueBuffer[i] = "7";
            }
        }
    }
    Gsm4 *gsm4 = new Gsm4(settings,this);
    connect(gsm4,SIGNAL(closeParentWidget()),this,SLOT(closeWidget()));
    //gsm4->show();
    gsm4->showFullScreen();
}

void Gsm3::closeWidget()
{
    close();
    emit(closeParentWidget());
}

void Gsm3::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
    emit(closeParentWidget());
}
