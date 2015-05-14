#include "datetimesetting.h"
#include "button.h"

extern "C"{
#include "ostime.h"
#include "ostools.h"
}
extern void _print(char *fmt,...);

DateTimeSetting::DateTimeSetting(QWidget *parent) : BaseForm(tr("DATE/TIME SETTING"))
{
    lineEdit = new QDateEdit();
    lineEdit_2 = new QTimeEdit();
    lineEdit->setAlignment(Qt::AlignCenter);
    lineEdit_2->setAlignment(Qt::AlignCenter);

    lineEdit->setFixedHeight((30*screenHeight)/320);
    lineEdit_2->setFixedHeight((30*screenHeight)/320);

    lineEdit->installEventFilter(this);
    lineEdit_2->installEventFilter(this);

    lineEdit->setCalendarPopup(true);


    lineEdit->setMaximumDate(QDate(2037,12,31));
    lineEdit->setMinimumDate(QDate(1970,1,1));

    lineEdit->setDisplayFormat("yyyy/MM/dd");
    lineEdit_2->setDisplayFormat("hh:mm");


    QTextEdit *textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(10);
    vbox->addWidget(new QLabel(tr("CURRENT DATE(yyyy/mm//dd):")));
    vbox->addWidget(lineEdit);
    vbox->addWidget(new QLabel(tr("CURRENT TIME(hh:mm):")));
    vbox->addWidget(lineEdit_2);

    vbox->setContentsMargins(20,20,20,10);
    vbox->addStretch(1);

    textEdit->setLayout(vbox);

    layout->addWidget(textEdit);


    btnOk->setText(tr("Save"));
    btnOk->show();

    lineEdit->setFocus();

    getValue();

    connect(btnOk, SIGNAL(clicked()), this, SLOT(on_btnSave_clicked()));


}


bool DateTimeSetting::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==lineEdit||obj==lineEdit_2)
    {
        if(event->type()==QEvent::KeyPress)
        {
            QKeyEvent *keyEve = static_cast<QKeyEvent*>(event) ;
            switch(keyEve->key())
            {
            case Qt::Key_Enter:

                if(lineEdit->hasFocus()&&(lineEdit->currentSection()==QDateTimeEdit::DaySection))
                    lineEdit_2->setFocus();
                else if(lineEdit_2->hasFocus()&&(lineEdit_2->currentSection()==QDateTimeEdit::MinuteSection))
                    on_btnSave_clicked();

                return true;

            default:
                return false;
            }

        }
        else
            return false;
    }
    else
        return QObject::eventFilter(obj,event);
}


void DateTimeSetting::getValue()
{
    //get date
     unsigned char ptDate[10];
     memset(ptDate,0x00,sizeof(ptDate));
     Os__read_date_format(ptDate);
     _print("ptDate  %s\n",ptDate);

     QStringList temp_date = QString((char*)ptDate).split("/");
     int dd = temp_date.at(0).toInt();
     int mo = temp_date.at(1).toInt();
     int yy = temp_date.at(2).toInt();

     QString compYear;
     //qDebug()<<"yy"<<yy;
     if(yy>=70) compYear = "19"+QString::number(yy);
     else {
         if(yy<10){
             compYear = "200"+QString::number(yy);
             //qDebug()<<"aaaa  compYear"<<compYear;
         }
         else{
             compYear = "20"+QString::number(yy);
             //qDebug()<<"bbbb  compYear"<<compYear;

         }
     }

     //qDebug()<<"dd  mo  yy  compYear  compYear.toInt()"<<dd<<mo<<yy<<compYear<<compYear.toInt();

     if(QDate::isValid(compYear.toInt(), mo, dd))
         lineEdit->setDate(QDate(compYear.toInt(),mo,dd));    // ptDate = "29/12/11"

    //get time
     unsigned char ptTime[10];
     memset(ptTime,0x00,sizeof(ptTime));
     Os__read_time_format(ptTime);
     _print("ptTime %s\n",ptTime);

     QStringList temp_time = QString((char*)ptTime).split(":");
     int hh = temp_time.at(0).toInt();
     int mm = temp_time.at(1).toInt();

     //qDebug()<<"hh  mm  "<<hh<<mm;

     lineEdit_2->setTime(QTime(hh,mm));      // ptDate = "16:59"

}

void DateTimeSetting::setValue()
{
    //set date
    int yy = lineEdit->date().year();
    int mo = lineEdit->sectionText(QDateTimeEdit::MonthSection).toInt();
    int dd = lineEdit->sectionText(QDateTimeEdit::DaySection).toInt();


    QString date;
    date.sprintf("%02d%02d%02d",dd,mo,yy%100);


    uchar ucResult = Os__write_date((uchar*)date.toAscii().data());

    if(ucResult)
    {
        showInforText(tr("  Set Failed!  "));
        QTimer::singleShot(2000,this,SLOT(close()));

    }


    //set time
    int hh = lineEdit_2->sectionText(QDateTimeEdit::HourSection).toInt();
    int mm = lineEdit_2->sectionText(QDateTimeEdit::MinuteSection).toInt();


    QString time ;
    time.sprintf("%02d%02d",hh,mm);


    uchar ucResult1 = Os__write_time((uchar*)time.toAscii().data());

    if(ucResult1)
    {
        showInforText(tr("  Set Failed!  "));

        QTimer::singleShot(2000,this,SLOT(close()));
    }
    else
    {
        showInforText(tr("  Set SUCCESS!  "));

        QTimer::singleShot(2000,this,SLOT(close()));

    }
 }


DateTimeSetting::~DateTimeSetting()
{
      //delete ui;
}


void DateTimeSetting::on_btnSave_clicked()
{
    btnOk->setEnabled(false);

//    qDebug()<<"IN BTNSAVE CLICKED!~~~~~~~~~~";
    setValue();
    system("sync");

}


