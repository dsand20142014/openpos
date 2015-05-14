#include "gsm2.h"
#include "ui_gsm2.h"

#include <QDebug>

Gsm2::Gsm2(QSettings *setting, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Gsm2)
{
    ui->setupUi(this);
    settings = setting;
    displayValue();
}

Gsm2::~Gsm2()
{
    delete ui;
}

void Gsm2::displayValue(void)
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "01.WIRELESS_MODULE")
        {
            if(readSetting.keysValue[i] == "0")
            {
                ui->rbGprs->setChecked(true);
            }
            if(readSetting.keysValue[i] == "1")
            {
                ui->rbWcdma->setChecked(true);
            }
        }
        if(readSetting.keys[i] == "02.MODULE_ALWAYS_ONLINE")
        {
            if(readSetting.keysValue[i] == "0")
            {
                ui->rbNo->setChecked(true);
            }
            if(readSetting.keysValue[i] == "1")
            {
                ui->rbYes->setChecked(true);
            }
        }
    }
}

void Gsm2::on_pbNext_clicked()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "01.WIRELESS_MODULE")
        {
            if(ui->rbGprs->isChecked())
            {
                readSetting.keysValueBuffer[i] = "0";
            }
            if(ui->rbWcdma->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
        }
        if(readSetting.keys[i] == "02.MODULE_ALWAYS_ONLINE")
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
    }
    Gsm3 *gsm3 = new Gsm3(settings,this);
    connect(gsm3,SIGNAL(closeParentWidget()),this,SLOT(closeWidget()));
    //gsm3->show();
    gsm3->showFullScreen();
}

void Gsm2::closeWidget()
{
    close();
    emit(closeParentWidget());
}

void Gsm2::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
    emit(closeParentWidget());
}
