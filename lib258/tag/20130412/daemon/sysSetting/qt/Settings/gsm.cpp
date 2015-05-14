#include "gsm.h"
#include "ui_gsm.h"

#include <QDebug>



Gsm::Gsm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Gsm)
{
    ui->setupUi(this);
    connect(ui->lePwd,SIGNAL(textEdited(QString)),this,SLOT(pwdDeal(QString)));
    settings = new QSettings("setting.ini",QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
    displayValue();
}

Gsm::~Gsm()
{
    delete ui;
}

void Gsm::pwdDeal(QString str)
{
    QString strXing;
    if(strPwd.length() - str.length() < 0)
    {
        strPwd += str.right(1);
    }
    else
    {
        strPwd = strPwd.remove(strPwd.length()-1,1);
    }
    strXing = "";
    for(int i=0; i<strPwd.length(); i++)
    {
        strXing += "*";
    }
    ui->lePwd->setText(strXing);
    //ui->lePwd->setText(strPwd);
}

void Gsm::displayValue(void)
{
    QString strXing,strValue;
    readSetting.clear();
    readSetting.readKeys(settings,"GSM_SETTING");
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "04.USERNAME")
        {
            if(readSetting.keysValue[i] == "NULL")
            {
                ui->leName->setText("");
            }
            else
            {
                ui->leName->setText(readSetting.keysValue[i]);
            }
        }
        if(readSetting.keys[i] == "05.PASSWD")
        {
            if(readSetting.keysValue[i] == "NULL")
            {
                ui->lePwd->setText("");
            }
            else
            {
                ui->lePwd->setText("");
            }
        }
    }
}


void Gsm::on_pbNext_clicked()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "04.USERNAME")
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
        if(readSetting.keys[i] == "05.PASSWD")
        {
            //readSetting.keysValueBuffer[i].replace(i,ui->lePwd->text());
            if(ui->lePwd->text().isEmpty())
            {
                readSetting.keysValueBuffer[i] = "NULL";
            }
            else
            {
                readSetting.keysValueBuffer[i] = strPwd;
            }
        }
    }
    Gsm2 *gsm2 = new Gsm2(settings,this);
    connect(gsm2,SIGNAL(closeParentWidget()),this,SLOT(close()));
    //gsm2->show();
    gsm2->showFullScreen();
}

void Gsm::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
}
