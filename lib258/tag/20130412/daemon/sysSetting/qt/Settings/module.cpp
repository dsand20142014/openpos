#include "module.h"
#include "ui_module.h"

Module::Module(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Module)
{
    ui->setupUi(this);
    settings = new QSettings("setting.ini",QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
    displayValue();
}

Module::~Module()
{
    delete ui;
}

void Module::displayValue(void)
{
    readSetting.clear();
    readSetting.readKeys(settings,"MODULE_SETTING");
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "01.MACHINE_SERIAL_NO")
        {
            ui->leMsn->setText(readSetting.keysValue[i]);
            ui->leMsn->setEnabled(false);
        }
        if(readSetting.keys[i] == "02.PINPAD_SERIAL_NO")
        {
            ui->lePsn->setText(readSetting.keysValue[i]);
            ui->lePsn->setEnabled(false);
        }
        if(readSetting.keys[i] == "03.APP_DEBUG")
        {
            if(readSetting.keysValue[i] == "0")
            {
                ui->rbAdno->setChecked(true);
            }
            if(readSetting.keysValue[i] == "1")
            {
                ui->rbAdyes->setChecked(true);
            }
        }
        if(readSetting.keys[i] == "04.DAEMON_DEBUG")
        {
            if(readSetting.keysValue[i] == "0")
            {
                ui->rbDdno->setChecked(true);
            }
            if(readSetting.keysValue[i] == "1")
            {
                ui->rbDdyes->setChecked(true);
            }
        }
    }
}

void Module::on_pbEnter_clicked()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "01.MACHINE_SERIAL_NO")
        {
            readSetting.keysValueBuffer[i] = ui->leMsn->text();
        }
        if(readSetting.keys[i] == "02.PINPAD_SERIAL_NO")
        {
            readSetting.keysValueBuffer[i] = ui->lePsn->text();
        }
        if(readSetting.keys[i] == "03.APP_DEBUG")
        {
            if(ui->rbAdno->isChecked())
            {
                readSetting.keysValueBuffer[i] = "0";
            }
            if(ui->rbAdyes->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
        }
        if(readSetting.keys[i] == "04.DAEMON_DEBUG")
        {
            if(ui->rbDdno->isChecked())
            {
                readSetting.keysValueBuffer[i] = "0";
            }
            if(ui->rbDdyes->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
        }
    }
    readSetting.keysValue = readSetting.keysValueBuffer;
    readSetting.setKeys(settings,"MODULE_SETTING");
    readSetting.clear();

    close();
}

void Module::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
}
