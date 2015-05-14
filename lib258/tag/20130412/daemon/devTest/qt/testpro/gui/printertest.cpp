#include "printertest.h"

#include <QDebug>
extern "C"{
#include "tools.h"
#include "printer.h"
#include "fsync_drvs.h"
#include "sand_debug.h"
#include "ostools.h"
#include "osdrv.h"
#include "test/test.h"
}

PrinterTest::PrinterTest(bool flag,QWidget *parent) :
    BaseForm(tr(" "), parent)
{
    if(hotflags)
        title->setText("PRINTER HOT TEST");
    else
        title->setText("PRINTER TEST");

    label = new QLabel(this);
    label->setFixedWidth(130);

    btnOne = new Button("1.SINGLE TEST",9,this);
    btnMore = new Button("2.LOOP TEST",9,this);

    layout->addSpacerItem(new QSpacerItem(10, 28+2));

    layout->addWidget(btnOne);

    layout->addSpacerItem(new QSpacerItem(10, 28+2));

    layout->addWidget(btnMore);

    layout->addWidget(label);

    btnOne->setFocus();

    hotflags = flag;
    moreFlag = false;


    connect(btnOne, SIGNAL(clicked()), this, SLOT(getResult()));
    connect(btnOne, SIGNAL(clicked()), this, SLOT(slotFlag()));

    connect(btnMore, SIGNAL(clicked()), this, SLOT(on_pushButton_3_clicked()));

    connect(btnExit, SIGNAL(clicked()), this, SLOT(slotExit()));

    connect(&timer,SIGNAL(timeout()),this,SLOT(getResult()));
}

PrinterTest::~PrinterTest()
{

}

void PrinterTest::slotExit()
{
    timer.stop();
    Os__clear_printBuf();
    close();
}

void PrinterTest::slotFlag()
{
    moreFlag = false;
}

void PrinterTest::getResult()
{
    btnMore->setEnabled(false);
    btnOne->setEnabled(false);

    if(!moreFlag){
//        btnOne->setEnabled(false);
        disconnect(btnMore, SIGNAL(clicked()), this, SLOT(on_pushButton_3_clicked()));
    }
    else
    {
//        btnMore->setEnabled(false);
//        btnOne->setEnabled(false);

        label->setText(tr(" PRINTTING.....  "));
        disconnect(btnOne, SIGNAL(clicked()), this, SLOT(getResult()));
        disconnect(btnOne, SIGNAL(clicked()), this, SLOT(slotFlag()));
    }


    if(timer.isActive())
        timer.stop();

    if(hotflags)
    {
        int ret = -1;

        ret = Os__sign_xprint((uchar*)"/daemon/image/test.bmp");

        if(ret<0) {
              label->setText(tr("PRINT ERROR"));
              if(!moreFlag)
                  QTimer::singleShot(1500,this,SLOT(slot_connect()));

            return;
         }
        if(moreFlag)
             timer.start(500);

    }
    else{
        unsigned char ucPrint[41];
        //unsigned char ucResult;
        int ret;
        unsigned char prn_fonts[] = {0x00,0x01,0x1c,0x1d,0x1e};


        ret = Os__sign_xprint((unsigned char *)"/daemon/image/printtest.bmp");

        if (ret < 0)
        {
            label->setText(tr("PRINT ERROR"));
            if(!moreFlag)
                QTimer::singleShot(2000,this,SLOT(slot_connect()));

            return;
        }

        memset(ucPrint,0,sizeof(ucPrint));

        ucPrint[2]=' ';
        memcpy(&ucPrint[3],"1234567890abcdefghijklmnopqrstuvwxyz\n",38);

        for (int i=0; i<5; i++) {
            hex_asc(ucPrint, &prn_fonts[i], 2);

            if(Os__GB2312_xprint(ucPrint,prn_fonts[i]))
            {
                label->setText(tr("TOO HOT,STOP PRINT"));
                return;
            }

        }

#ifdef QC430_v01
        Os__xlinefeed (5);
        Os__papercut();
#endif
        if(moreFlag)
            timer.start(800);

    }

    if(!moreFlag)
        QTimer::singleShot(1500,this,SLOT(slot_connect()));


}

void PrinterTest::slot_connect()
{
    connect(btnMore, SIGNAL(clicked()), this, SLOT(on_pushButton_3_clicked()));
    btnOne->setEnabled(true);
    btnMore->setEnabled(true);

}

void PrinterTest::on_pushButton_3_clicked()
{
    moreFlag = true;

    label->clear();

    if(timer.isActive())
        return;
    if(hotflags)
        timer.start(800);
    else
        timer.start(800);

}


void PrinterTest::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_1:
        if(!moreFlag)
           getResult();
        break;
    case Qt::Key_2:
        on_pushButton_3_clicked();
        break;
    case Qt::Key_Escape:
        slotExit();
        break;

    }
}

