#include "date.h"
#include "ui_date.h"

#include <QDebug>

Date::Date(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Date)
{
    ui->setupUi(this);
    settings = new QSettings("setting.ini",QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
    displayValue();
}

Date::~Date()
{
    delete ui;
}

void Date::displayValue(void)
{
    readSetting.clear();
    readSetting.readKeys(settings,"DATE_TIME_SETTING");
    int year,month,day,hour,minute,second;
    QString strDate,strTime;
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "01.YYMMDD")
        {
            strDate = readSetting.keysValue[i];
            year = strDate.left(2).insert(0,"20").toInt();
            month = strDate.left(4).right(2).toInt();
            day = strDate.right(2).toInt();
            date.setDate(year,month,day);
            //ui->dateTimeEdit->setDate(date);
            ui->dateEdit->setDate(date);
            //ui->dateTimeEdit->setDateTime();
        }
        if(readSetting.keys[i] == "02.MMDD")
        {
            strTime = readSetting.keysValue[i];
            hour = strTime.left(2).toInt();
            minute = strTime.right(2).toInt();
            second = 0;
            time.setHMS(hour,minute,second);
            //ui->dateTimeEdit->setTime(time);
            ui->timeEdit->setTime(time);
        }
    }
}

void Date::on_pbEnter_clicked()
{
    QDate dateTmp;
    QTime timeTmp;
    QString strYear,strMonth,strDay,strHour,strMinute;
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "01.YYMMDD")
        {
            dateTmp = ui->dateEdit->date();
            //dateTmp = ui->dateTimeEdit->date();
            strYear = strYear.number(dateTmp.year());
            strYear = strYear.right(2);
            if(strYear.length() == 1)
            {
                strYear = "0" + strYear;
            }
            strMonth = strMonth.number(dateTmp.month());
            if(strMonth.length() == 1)
            {
                strMonth = "0" + strMonth;
            }
            strDay = strDay.number(dateTmp.day());
            if(strDay.length() == 1)
            {
                strDay = "0" + strDay;
            }
            readSetting.keysValueBuffer[i] = strYear+strMonth+strDay;
        }
        if(readSetting.keys[i] == "02.MMDD")
        {
            timeTmp = ui->timeEdit->time();
            //timeTmp = ui->dateTimeEdit->time();
            strHour = strHour.number(timeTmp.hour());
            if(strHour.length() == 1)
            {
                strHour = "0" + strHour;
            }
            strMinute = strMinute.number(timeTmp.minute());
            if(strMinute.length() == 1)
            {
                strMinute = "0" + strMinute;
            }
            readSetting.keysValueBuffer[i] = strHour+strMinute;
        }
    }
    readSetting.keysValue = readSetting.keysValueBuffer;
    readSetting.setKeys(settings,"DATE_TIME_SETTING");
    readSetting.clear();

    close();
}

void Date::on_pbCancel_clicked()
{
    readSetting.clear();
    close();
}
