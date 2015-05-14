#include "settingdisplay.h"
#include "ui_settingdisplay.h"

SettingDisplay::SettingDisplay(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDisplay)
{
    ui->setupUi(this);
}

SettingDisplay::~SettingDisplay()
{
    delete ui;
}
