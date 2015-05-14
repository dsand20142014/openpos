#include "autolcdtest.h"

//#include <QtGui>

AutoLcdTest::AutoLcdTest(QWidget *parent) : BaseForm(NULL, parent)
{
    num = 0;

    palette.setBrush(QPalette::Background, QBrush(QColor("white")));
    setPalette(palette);


    connect(&timer,SIGNAL(timeout()),this,SLOT(refresh()));
    timer.start(600);

}

AutoLcdTest::~AutoLcdTest()
{
}

void AutoLcdTest::refresh( )
{
    timer.stop();

    if(num==0) {
        palette.setBrush(QPalette::Background, QBrush(QColor("red")));
        setPalette(palette);
    }
    else if(num==1) {
        palette.setBrush(QPalette::Background, QBrush(QColor("green")));
        setPalette(palette);
    }
    else if(num==2) {
        palette.setBrush(QPalette::Background, QBrush(QColor("blue")));
        setPalette(palette);
    }
    else if(num==3)
    {
        showInforText(tr("TEST SUCCESS!"),false);


        QTimer::singleShot(1000,this,SLOT(close()));
        return;
    }

    num++;

    timer.start(1000);
}

