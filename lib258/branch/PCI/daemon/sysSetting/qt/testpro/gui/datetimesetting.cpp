#include "datetimesetting.h"
#include "ui_datetimesetting.h"
#include "ostime.h"

DateTimeSetting::DateTimeSetting(QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint),
    ui(new Ui::DateTimeSetting)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/thirdback.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    ui->lineEdit->setFocus();
    ui->label_3->hide();

    ui->lineEdit->setInputMask("00/00/00");
    ui->lineEdit_2->setInputMask("00:00");

    getValue();
}



void DateTimeSetting::getValue()
{

     unsigned char ptDate[10];
     memset(ptDate,0x00,sizeof(ptDate));
     Os__read_date_format(ptDate);
     printf("%s\n",ptDate);

     ui->lineEdit->setText(QString((char*)ptDate));    // ptDate = "29/12/11"


     unsigned char ptTime[10];
     memset(ptTime,0x00,sizeof(ptTime));
     Os__read_time_format(ptTime);
     printf("%s\n",ptTime);

     ui->lineEdit_2->setText(QString((char*)ptTime));      // ptDate = "16:59"

}

void DateTimeSetting::setValue()
{
    QStringList dateList = ui->lineEdit->text().split("/");
    if(dateList.count()<3) return;

    QString tep =  dateList[0]+dateList[1]+dateList[2];

    uchar ucResult = Os__write_date((uchar*)tep.toAscii().data());

    qDebug()<<"ucResult======"<<ucResult;
    if(ucResult)
    {
        ui->label_3->setText("FORMAT ERROR");
        ui->label_3->show();

        return;
    }


    QStringList timeList = ui->lineEdit_2->text().split(":");
    if(timeList.count()<2) return;

    QString tep1 =  timeList[0]+timeList[1];


    uchar ucResult1 = Os__write_time((uchar*)tep1.toAscii().data());
    qDebug()<<"ucResult1======"<<ucResult1;

    if(ucResult1)
    {
        ui->label_3->setText("FORMAT ERROR");
        ui->label_3->show();
    }

    else
    {
        ui->label_3->setText("SAVING...");
        ui->label_3->show();

       // sleep(2);
        close();
    }
 }


DateTimeSetting::~DateTimeSetting()
{
     qDebug()<<"ccccccccccc";
      delete ui;
}

void DateTimeSetting::on_pushButton_clicked()
{
    close();
}

void DateTimeSetting::on_pushButton_2_clicked()
{
    qDebug()<<"aaaaaaaaaaa";
    setValue();
    system("sync");
    qDebug()<<"bbbbbbbbbbbbbbb";

}

void DateTimeSetting::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()){
    case Qt::Key_Escape:
    case Qt::Key_Cancel:
        close();
        break;
    }
}
