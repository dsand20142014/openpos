#include "gpswin.h"

extern void _print(char *fmt,...);

GpsWin::GpsWin(QWidget *parent) :
    BaseForm(tr(" GPS DATA "),parent)
{
    textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
//    textEdit->setReadOnly(true);

    layout->addWidget(textEdit);

    connect(&timer,SIGNAL(timeout()),this,SLOT(slot_timer()));
    connect(btnExit,SIGNAL(clicked()),&timer,SLOT(stop()));

    file = new QFile("/dev/gps");
    if(file->open(QFile::ReadOnly)){
        _print("******* timer start \n");
        timer.start(1500);
    }
    else
        textEdit->setText(tr("OPEN FILE FAILED!"));


}

GpsWin::~GpsWin()
{
    file->close();
    timer.stop();
    delete file;
}

void GpsWin::slot_timer()
{
    _print("******* slot_timer start \n");

    timer.stop();

    QTextStream stream(file);
    QString line;
    qDebug()<<"line==="<<line;

    line = stream.read(4096);

    qDebug()<<"111line==="<<line;

    textEdit->append(line);


    //        QTextStream stream(file);
//        QString line = stream.readAll();
//        qDebug()<<"line==="<<line;
//        if(line.isNull())
//            textEdit->append("read failed");
//        else
//             textEdit->append(line);

    timer.start();
}
