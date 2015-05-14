#include "ethernet.h"
#include "ui_ethernet.h"

Ethernet::Ethernet(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Ethernet)
{
    ui->setupUi(this);
    settings = new QSettings("setting.ini",QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
    displayValue();
}

Ethernet::~Ethernet()
{
    delete ui;
}

void Ethernet::displayValue(void)
{
    readSetting.clear();
    readSetting.readKeys(settings,"ETHERNET_SETTING");
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "02.DHCP")
        {
            if(readSetting.keysValue[i] == "1")
            {
                ui->rbYes->setChecked(true);
            }
            if(readSetting.keysValue[i] == "0")
            {
                //ui->rbYes->setChecked(false);
                ui->rbNo->setChecked(true);
            }
        }
        if(readSetting.keys[i] == "01.MAC")
        {
            ui->leMac->setText(readSetting.keysValue[i]);
            ui->leMac->setInputMask("HH:HH:HH:HH:HH:HH");
        }
    }
}

void Ethernet::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
}

void Ethernet::on_pbEnter_clicked()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "02.DHCP")
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
        if(readSetting.keys[i] == "01.MAC")
        {
            readSetting.keysValueBuffer[i] = ui->leMac->text();
        }
    }
    readSetting.keysValue = readSetting.keysValueBuffer;
    readSetting.setKeys(settings,"ETHERNET_SETTING");
    readSetting.clear();

    close();
}
