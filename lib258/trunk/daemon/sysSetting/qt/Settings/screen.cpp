#include "screen.h"
#include "ui_screen.h"

Screen::Screen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Screen)
{
    ui->setupUi(this);
    settings = new QSettings("setting.ini",QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
    displayValue();
}

Screen::~Screen()
{
    delete ui;
}

void Screen::displayValue(void)
{
    readSetting.clear();
    readSetting.readKeys(settings,"SCREEN_SETTING");
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "02.SCREENSAVER_TIME")
        {
            ui->leSs->setText(readSetting.keysValue[i]);
        }
        if(readSetting.keys[i] == "03.SLEEP_TIME")
        {
            ui->leSt->setText(readSetting.keysValue[i]);
        }
        if(readSetting.keys[i] == "01.BRIGHTNESS")
        {
            if(readSetting.keysValue[i] == "1")
            {
                ui->rbL1->setChecked(true);
            }
            if(readSetting.keysValue[i] == "2")
            {
                ui->rbL2->setChecked(true);
            }
            if(readSetting.keysValue[i] == "3")
            {
                ui->rbL3->setChecked(true);
            }
            if(readSetting.keysValue[i] == "4")
            {
                ui->rbL4->setChecked(true);
            }
            if(readSetting.keysValue[i] == "5")
            {
                ui->rbL5->setChecked(true);
            }
        }
    }
}

void Screen::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
}

void Screen::on_pbEnter_clicked()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "02.SCREENSAVER_TIME")
        {
            readSetting.keysValueBuffer[i] = ui->leSs->text();
        }
        if(readSetting.keys[i] == "03.SLEEP_TIME")
        {
            readSetting.keysValueBuffer[i] = ui->leSt->text();
        }
        if(readSetting.keys[i] == "01.BRIGHTNESS")
        {
            if(ui->rbL1->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
            if(ui->rbL2->isChecked())
            {
                readSetting.keysValueBuffer[i] = "2";
            }
            if(ui->rbL3->isChecked())
            {
                readSetting.keysValueBuffer[i] = "3";
            }
            if(ui->rbL4->isChecked())
            {
                readSetting.keysValueBuffer[i] = "4";
            }
            if(ui->rbL5->isChecked())
            {
                readSetting.keysValueBuffer[i] = "5";
            }
        }
    }
    readSetting.keysValue = readSetting.keysValueBuffer;
    readSetting.setKeys(settings,"SCREEN_SETTING");
    readSetting.clear();

    close();
}
