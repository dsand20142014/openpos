#include "gsmdetailtest.h"
extern "C"{
#include "test/test.h"

}
GsmDetailTest::GsmDetailTest(QWidget *parent) :
    BaseForm(tr("GSM CSQ TEST"),parent)

{
    btnExit->setFocus();

    myscroll = new QScrollArea();
    layout->addWidget(myscroll);

    bar = new QProgressBar(myscroll);
    bar->setRange(0,100);
    bar->setOrientation(Qt::Vertical);
    bar->setValue(0);
    bar->setFormat("csq:%v");
    bar->setAlignment(Qt::AlignCenter);
    bar->show();
    bar->setGeometry(90,10,40,192);

    connect(&timer,SIGNAL(timeout()),this,SLOT(slot1()));
    timer.start(300);

}

void GsmDetailTest::slot1()
{
    timer.stop();
    int csq = 0;

    csq=OSGSM_signalDetect();

    bar->setValue(csq);

    timer.start(1000);
}



GsmDetailTest::~GsmDetailTest()
{
    OSGSM_hangUp();
    OSGSM_exit();

//    system("/daemon/dial.dm.4.1.4 dbg=0 &");
}


