#include "gpswin.h"

extern void _print(char *fmt,...);

GpsWin::GpsWin(QWidget *parent) :
    BaseForm(" GPS DATA ",parent)
{
    textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);

    file = new QFile("/dev/gps");
    if(file->open(QFile::ReadOnly)){
        _print("******* timer start \n");
        timer.start(1000);
    }
    else
        textEdit->setText("OPEN FILE FAILED!");

    connect(btnExit,SIGNAL(clicked()),&timer,SLOT(stop()));

    connect(&timer,SIGNAL(timeout()),this,SLOT(slot_timer()));
}

GpsWin::~GpsWin()
{
    file->close();
    delete file;
}

void GpsWin::slot_timer()
{
    QTextStream in(file);
    QString line = in.readAll();
    if(line.isNull())
        textEdit->append("READ FAILED!");
    else
        textEdit->append(line);


}
