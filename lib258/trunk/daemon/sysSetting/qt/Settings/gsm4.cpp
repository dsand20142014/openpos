#include "gsm4.h"
#include "ui_gsm4.h"

Gsm4::Gsm4(QSettings *setting, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Gsm4)
{
    ui->setupUi(this);
    settings = setting;
    connect(ui->rbCmnet,SIGNAL(toggled(bool)),this,SLOT(radioButton1Deal(bool)));
    connect(ui->rbWxbc,SIGNAL(toggled(bool)),this,SLOT(radioButton2Deal(bool)));
    connect(ui->rbVodafone,SIGNAL(toggled(bool)),this,SLOT(radioButton3Deal(bool)));
    connect(ui->rbMposcu,SIGNAL(toggled(bool)),this,SLOT(radioButton4Deal(bool)));
    connect(ui->rbGzylcmda,SIGNAL(toggled(bool)),this,SLOT(radioButton5Deal(bool)));
    connect(ui->rbGzdxcmda,SIGNAL(toggled(bool)),this,SLOT(radioButton6Deal(bool)));
    connect(ui->rbChinaUnicom,SIGNAL(toggled(bool)),this,SLOT(radioButton7Deal(bool)));

    connect(ui->leOther,SIGNAL(textEdited(QString)),this,SLOT(lineEditDeal(QString)));
    ui->rbHide->hide();
    displayValue();
}

void Gsm4::lineEditDeal(QString str)
{
    if(str == "CMNET")
    {
        ui->rbCmnet->setChecked(true);
    }
    else if(str == "MPOSCUPSH.SH")
    {
        ui->rbMposcu->setChecked(true);
    }
    else if(str == "VODAFONE")
    {
        ui->rbVodafone->setChecked(true);
    }
    else if(str == "CHINA UNICOM")
    {
        ui->rbChinaUnicom->setChecked(true);
    }
    else if(str == "WXBC")
    {
        ui->rbWxbc->setChecked(true);
    }
    else if(str == "GZYLCMDA")
    {
        ui->rbGzylcmda->setChecked(true);
    }
    else if(str == "GZDXCMDA")
    {
        ui->rbGzdxcmda->setChecked(true);
    }
    else
    {
        ui->rbHide->setChecked(true);
    }
}

void Gsm4::radioButton1Deal(bool isChecked)
{
    if(isChecked)
    {
        ui->leOther->setText("CMNET");
    }
}

void Gsm4::radioButton2Deal(bool isChecked)
{
    if(isChecked)
    {
        ui->leOther->setText("MPOSCUPSH.SH");
    }
}

void Gsm4::radioButton3Deal(bool isChecked)
{
    if(isChecked)
    {
        ui->leOther->setText("VODAFONE");
    }
}

void Gsm4::radioButton4Deal(bool isChecked)
{
    if(isChecked)
    {
        ui->leOther->setText("CHINA UNICOM");
    }
}

void Gsm4::radioButton5Deal(bool isChecked)
{
    if(isChecked)
    {
        ui->leOther->setText("WXBC");
    }
}

void Gsm4::radioButton6Deal(bool isChecked)
{
    if(isChecked)
    {
        ui->leOther->setText("GZYLCMDA");
    }
}

void Gsm4::radioButton7Deal(bool isChecked)
{
    if(isChecked)
    {
        ui->leOther->setText("GZDXCMDA");
    }
}

Gsm4::~Gsm4()
{
    delete ui;
}

void Gsm4::displayValue(void)
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "03.DEFAULT_APN")
        {
            if(readSetting.keysValue[i] == "CMNET")
            {
                ui->rbCmnet->setChecked(true);
            }
            if(readSetting.keysValue[i] == "MPOSCUPSH.SH")
            {
                ui->rbMposcu->setChecked(true);
            }
            if(readSetting.keysValue[i] == "VODAFONE")
            {
                ui->rbVodafone->setChecked(true);
            }
            if(readSetting.keysValue[i] == "CHINA UNICOM")
            {
                ui->rbChinaUnicom->setChecked(true);
            }
            if(readSetting.keysValue[i] == "WXBC")
            {
                ui->rbWxbc->setChecked(true);
            }
            if(readSetting.keysValue[i] == "GZYLCMDA")
            {
                ui->rbGzylcmda->setChecked(true);
            }
            if(readSetting.keysValue[i] == "GZDXCMDA")
            {
                ui->rbGzdxcmda->setChecked(true);
            }
            ui->leOther->setText(readSetting.keysValue[i]);
        }
    }
}

void Gsm4::on_pbEnter_clicked()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "03.DEFAULT_APN")
        {
//            if(ui->rbCmnet->isChecked())
//            {
//                readSetting.keysValueBuffer[i] = "CMNET";
//            }
//            if(ui->rbMposcu->isChecked())
//            {
//                readSetting.keysValueBuffer[i] = "MPOSCUPSH.SH";
//            }
//            if(ui->rbVodafone->isChecked())
//            {
//                readSetting.keysValueBuffer[i] = "VODAFONE";
//            }
//            if(ui->rbChinaUnicom->isChecked())
//            {
//                readSetting.keysValueBuffer[i] = "CHINA UNICOM";
//            }
//            if(ui->rbWxbc->isChecked())
//            {
//                readSetting.keysValueBuffer[i] = "WXBC";
//            }
//            if(ui->rbGzylcmda->isChecked())
//            {
//                readSetting.keysValueBuffer[i] = "GZYLCMDA";
//            }
//            if(ui->rbGzdxcmda->isChecked())
//            {
//                readSetting.keysValueBuffer[i] = "GZDXCMDA";
//            }
            readSetting.keysValueBuffer[i] = ui->leOther->text();
        }
    }
    readSetting.keysValue = readSetting.keysValueBuffer;
    readSetting.setKeys(settings,"GSM_SETTING");
    readSetting.clear();

    close();
    emit(closeParentWidget());
}

void Gsm4::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
    emit(closeParentWidget());
}
