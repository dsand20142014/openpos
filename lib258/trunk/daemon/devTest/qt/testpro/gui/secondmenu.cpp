#include "secondmenu.h"
#include "ui_secondmenu.h"
#include <QPropertyAnimation>

#include "autolcdtest.h"
#include "manuallcdtest.h"
//#include "timetest.h"
#include "printertest.h"
#include "gsmcalltest.h"
#include "gsmdetailtest.h"
#include "com2test.h"
#include "asynccardtest.h"
#include "beeptest.h"

#include "tftest.h"
#include "wifitest.h"
#include "gsmtestdial.h"
#include "card4442test.h"
#include "apntest.h"

#include "button.h"
//#include <QTextStream>
//#include <stdio.h>

extern "C"{
#include "printer.h"
#include "test/test.h"

}

extern void _print(char *fmt,...);

SecondMenu::SecondMenu(QString title,int curnum,QStringList items,QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint),
    ui(new Ui::SecondMenu)
{
    ui->setupUi(this);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("/daemon/image/thirdback.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    setAttribute(Qt::WA_DeleteOnClose);

    screenWidth = qApp->desktop()->screen(0)->width();
    screenHeight = qApp->desktop()->screen(0)->height();

    qDebug()<<"screenWidth== screenh == "<<screenWidth<<screenHeight;
    this->resize(screenWidth,screenHeight);
    ui->title->setText(title);

    Button *exitBtn = new Button(tr("Exit"), 1);
    ui->verticalLayout->addWidget(exitBtn);
    exitBtn->setFocus();

    ui->listWidget->clear();
    ui->listWidget->addItems(items);

    curNum = curnum;

    connect(ui->listWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(listSlot(QModelIndex)));


    connect(exitBtn, SIGNAL(clicked()), this, SLOT(close()));

}

void SecondMenu::listSlot(QModelIndex idx)
{
    switch(curNum)
    {
    case 1:
        slot1(idx);
        break;
    case 2:
        slot2(idx);
        break;
    case 3:
        slot3(idx);
        break;
    case 4:
        slot4(idx);
        break;
    case 5:
        slot5(idx);
        break;
    case 6:
        slot6(idx);
        break;
    }
}

void SecondMenu::slot1(QModelIndex idx)
{

        switch(idx.row())
        {
        case 0:
        {
            AutoLcdTest *al = new AutoLcdTest();
            al->showMaximized();
        }
            break;
        case 1:
        {
            ManualLcdTest *ml = new ManualLcdTest();
            ml->showMaximized();
        }
            break;
        case 2:
        {
            BeepTest *bt = new BeepTest(1);
            bt->showMaximized();
        }
            break;
        case 3:
        {

            BeepTest *bt = new BeepTest(2);
            bt->showMaximized();
        }
            break;
        }


}


void SecondMenu::slot2(QModelIndex idx)
{
    switch(idx.row())
    {
    case 0:
    {
        unsigned char ucResult = 0x00;
        ucResult = Os__checkpaper();
        if(ucResult == 0x00)
        {
            QMessageBox box(QMessageBox::Information,tr("  title "),tr("   CHECK SUCCESS   "));
            box.setWindowFlags(Qt::CustomizeWindowHint);            

            QPushButton *okBtn = new QPushButton(tr("Ok"));
            okBtn->setFixedWidth((80*screenWidth)/240);
            okBtn->setFixedHeight((25*screenHeight)/320);
            box.addButton(okBtn,QMessageBox::YesRole);

            box.exec();
        }
        else
        {
            QMessageBox box(QMessageBox::Information,tr("    title   "),tr("NO PAPER, PLEASE CHECK!"));
            box.setWindowFlags(Qt::CustomizeWindowHint);

            QPushButton *okBtn = new QPushButton(tr("Ok"));
            okBtn->setFixedWidth((80*screenWidth)/240);
            okBtn->setFixedHeight((25*screenHeight)/320);
            box.addButton(okBtn,QMessageBox::YesRole);

            box.exec();

        }

    }
        break;
    case 1:{
        unsigned char ucResult = 0x00;
        ucResult = Os__checkpaper();
        if(ucResult!=0x00)
        {
            QMessageBox box(QMessageBox::Information,tr("    title   "),tr("NO PAPER, PLEASE CHECK!"));
            box.setWindowFlags(Qt::CustomizeWindowHint);

            QPushButton *okBtn = new QPushButton(tr("Ok"));
            okBtn->setFixedWidth((80*screenWidth)/240);
            okBtn->setFixedHeight((25*screenHeight)/320);
            box.addButton(okBtn,QMessageBox::YesRole);

            box.exec();

            return;
        }

        PrinterTest *pt = new PrinterTest(false);
        pt->showMaximized();

       }
        break;
    case 2:{
        _print("feed is click ************\n");
        Os__linefeed(5);
    }
        break;
    case 3:{
        unsigned char ucResult = 0x00;
        ucResult = Os__checkpaper();
        if(ucResult!=0x00)
        {
            QMessageBox box(QMessageBox::Information,tr("    title   "),tr("NO PAPER, PLEASE CHECK!"));
            box.setWindowFlags(Qt::CustomizeWindowHint);

            QPushButton *okBtn = new QPushButton(tr("Ok"));
            okBtn->setFixedWidth((80*screenWidth)/240);
            okBtn->setFixedHeight((25*screenHeight)/320);
            box.addButton(okBtn,QMessageBox::YesRole);

            box.exec();

            return;
        }

        PrinterTest *pt = new PrinterTest(true);
        pt->show();
    }
        break;

    }

}


void SecondMenu::slot3(QModelIndex idx)
{
    switch(idx.row())
    {
        case 0://async card test
        {
       // ////qDebug()<<"BEGAIN ASYNCCARD***********";

            AsyncCardTest *act = new AsyncCardTest(this);
            act->show();
        }
        break;
        case 1:
        {
            Card4442Test *ct = new Card4442Test(this);
            ct->show();
        }
            break;
    }

}

void SecondMenu::slot4(QModelIndex idx)
{
     switch(idx.row())
    {

    case 0:
    {
        GSMCallTest *gc = new GSMCallTest();
        gc->showMaximized();
    }
        break;

    case 1:
    {
        GsmDetailTest *gt = new GsmDetailTest(this);
        gt->show();
    }
        break;
    case 2:
    {
        GsmTestDial *gt = new GsmTestDial(this);
        gt->show();
    }
        break;

//        system("/daemon/dial.dm.4.1.4 dbg=0 &");


//    case 3:
//    {
//        WifiTest *wt = new WifiTest(0);
//        wt->show();
//    }
//        break;

    }

}

void SecondMenu::slot5(QModelIndex idx)
{
    if(idx.row()==0)
    {
       WifiTest *wt = new WifiTest(1);
       wt->show();

    }
    else if(idx.row()==1)
    {
        WifiTest *wt = new WifiTest(0);
        wt->show();
    }
    else if(idx.row() ==2)
    {
        ApnTest *at = new ApnTest();
        at->showMaximized();
    }
}

void SecondMenu::slot6(QModelIndex idx)
{
    //qDebug()<<"slot6***************";
    if(idx.row()==0)
    {
        TfTest *tf = new TfTest(1);
        tf->show();
    }
    else if(idx.row()==1)
    {
        TfTest *tf = new TfTest(2);
        tf->show();
    }
}


SecondMenu::~SecondMenu()
{
    delete ui;
}


void SecondMenu::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case  Qt::Key_Escape:
    case Qt::Key_Backspace:
        close();
        break;
    case Qt::Key_Enter:
         if(curNum==1)
             slot1(ui->listWidget->currentIndex());
         else if(curNum==2)
             slot2(ui->listWidget->currentIndex());
         else if(curNum==3)
             slot3(ui->listWidget->currentIndex());
         else if(curNum==4)
             slot4(ui->listWidget->currentIndex());
         else if(curNum==5)
             slot5(ui->listWidget->currentIndex());
         else if(curNum==6)
             slot6(ui->listWidget->currentIndex());
        break;
    case Qt::Key_1:
            ui->listWidget->setFocus();
            ui->listWidget->setCurrentRow(0);
        break;
    case Qt::Key_2:
            ui->listWidget->setFocus();
            ui->listWidget->setCurrentRow(1);
        break;
    case Qt::Key_3:
        if(ui->listWidget->count()>=3)
        {
            ui->listWidget->setFocus();
            ui->listWidget->setCurrentRow(2);
        }

        break;

    case Qt::Key_4:
        if(ui->listWidget->count()>=4)
        {
            ui->listWidget->setFocus();
            ui->listWidget->setCurrentRow(3);
        }
        break;
    }
}



