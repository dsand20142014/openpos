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

SecondMenu::SecondMenu(QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint),
    ui(new Ui::SecondMenu)
{
    ui->setupUi(this);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("/daemon/image/thirdback.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    setAttribute(Qt::WA_DeleteOnClose);

    Button *pushButton = new Button(tr("Exit"), 1);
    ui->verticalLayout->addWidget(pushButton);
    pushButton->setFocus();

    connect(ui->listWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(slot1(QModelIndex)));
    connect(ui->listWidget_2,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(slot2(QListWidgetItem*)));
    connect(ui->listWidget_3,SIGNAL(clicked(QModelIndex)),this,SLOT(slot3(QModelIndex)));


    connect(ui->listWidget_4,SIGNAL(clicked(QModelIndex)),this,SLOT(slot4(QModelIndex)));
    connect(ui->listWidget_5,SIGNAL(clicked(QModelIndex)),this,SLOT(slot5(QModelIndex)));
    connect(ui->listWidget_6,SIGNAL(clicked(QModelIndex)),this,SLOT(slot6(QModelIndex)));

    connect(pushButton, SIGNAL(clicked()), this, SLOT(close()));

}

void SecondMenu::slot1(QModelIndex idx)
{
    //qDebug()<<"in slot1"<<idx.row();
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


void SecondMenu::slot2(QListWidgetItem* item)
{
    switch(ui->listWidget_2->row(item))
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
            okBtn->setFixedWidth(80);
            okBtn->setFixedHeight(25);
            box.addButton(okBtn,QMessageBox::YesRole);

            box.exec();
        }
        else
        {
            QMessageBox box(QMessageBox::Information,tr("    title   "),tr("NO PAPER, PLEASE CHECK!"));
            box.setWindowFlags(Qt::CustomizeWindowHint);

//            QPainterPath path;
//            QRectF rect = QRectF(0, 0, box.width(), box.height());
//            path.addRoundRect(rect, 20, 20);
//            QPolygon polygon= path.toFillPolygon().toPolygon();
//            QRegion region(polygon);
//            box.setMask(region);

            QPushButton *okBtn = new QPushButton("Ok");
            okBtn->setFixedWidth(80);
            okBtn->setFixedHeight(25);
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

//            QPainterPath path;
//            QRectF rect = QRectF(0, 0, box.width(), box.height());
//            path.addRoundRect(rect, 20, 20);
//            QPolygon polygon= path.toFillPolygon().toPolygon();
//            QRegion region(polygon);
//            box.setMask(region);

            QPushButton *okBtn = new QPushButton("Ok");
            okBtn->setFixedWidth(80);
            okBtn->setFixedHeight(25);
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

            QPushButton *okBtn = new QPushButton("Ok");
            okBtn->setFixedWidth(80);
            okBtn->setFixedHeight(25);
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



void SecondMenu::setCurWidget(QString title,int idx)
{
    ui->title->setText(title);
    ui->stackedWidget->setCurrentIndex(idx);
}


SecondMenu::~SecondMenu()
{
    delete ui;
}


void SecondMenu::keyPressEvent(QKeyEvent *e)
{
    ////qDebug()<<"e->key()*************"<<e->key();
    switch(e->key())
    {
    case  Qt::Key_Escape:
    case Qt::Key_Backspace:
        ////qDebug()<<"close()**************";
        close();
        break;
    case Qt::Key_Enter:
        ////qDebug()<<"Key_Enter  stackedWidget->currentIndex****************"<<ui->stackedWidget->currentIndex();
         if(ui->stackedWidget->currentIndex()==0)
             slot1(ui->listWidget->currentIndex());
         else if(ui->stackedWidget->currentIndex()==1)
             slot2(ui->listWidget_2->currentItem());
         else if(ui->stackedWidget->currentIndex()==2)
             slot3(ui->listWidget_3->currentIndex());
         else if(ui->stackedWidget->currentIndex()==3)
             slot4(ui->listWidget_4->currentIndex());
         else if(ui->stackedWidget->currentIndex()==4)
             slot5(ui->listWidget_5->currentIndex());
         else if(ui->stackedWidget->currentIndex()==5)
             slot6(ui->listWidget_6->currentIndex());
        break;
    case Qt::Key_1:
        ////qDebug()<<"stackedWidget->currentIndex****************"<<ui->stackedWidget->currentIndex();
        if(ui->stackedWidget->currentIndex()==0)
        {
            ui->listWidget->setFocus();
            ui->listWidget->setCurrentRow(0);
        }
        else if(ui->stackedWidget->currentIndex()==1)
        {
            ui->listWidget_2->setFocus();
            ui->listWidget_2->setCurrentRow(0);
        }
        else if(ui->stackedWidget->currentIndex()==2)
        {
            ui->listWidget_3->setFocus();
            ui->listWidget_3->setCurrentRow(0);
        }
        else if(ui->stackedWidget->currentIndex()==3)
        {
            ui->listWidget_4->setFocus();
            ui->listWidget_4->setCurrentRow(0);
        }
        else if(ui->stackedWidget->currentIndex()==4)
        {
            ui->listWidget_5->setFocus();
            ui->listWidget_5->setCurrentRow(0);
        }
        else if(ui->stackedWidget->currentIndex()==5)
        {
            ui->listWidget_6->setFocus();
            ui->listWidget_6->setCurrentRow(0);
        }
        break;
    case Qt::Key_2:
        if(ui->stackedWidget->currentIndex()==0)
        {
            ui->listWidget->setFocus();
            ui->listWidget->setCurrentRow(1);
        }
        else if(ui->stackedWidget->currentIndex()==1)
        {
            ui->listWidget_2->setFocus();
            ui->listWidget_2->setCurrentRow(1);
        }
        else if(ui->stackedWidget->currentIndex()==2)
        {
            ui->listWidget_3->setFocus();
            ui->listWidget_3->setCurrentRow(1);
        }
        else if(ui->stackedWidget->currentIndex()==3)
        {
            ui->listWidget_4->setFocus();
            ui->listWidget_4->setCurrentRow(1);
        }
        else if(ui->stackedWidget->currentIndex()==4)
        {
            ui->listWidget_5->setFocus();
            ui->listWidget_5->setCurrentRow(1);
        }
        else if(ui->stackedWidget->currentIndex()==5)
        {
            ui->listWidget_6->setFocus();
            ui->listWidget_6->setCurrentRow(1);
        }
        break;
    case Qt::Key_3:
        if(ui->stackedWidget->currentIndex()==0)
        {
            ui->listWidget->setFocus();
            ui->listWidget->setCurrentRow(2);
        }
        else if(ui->stackedWidget->currentIndex()==1)
        {
            ui->listWidget_2->setFocus();
            ui->listWidget_2->setCurrentRow(2);
        }
        else if(ui->stackedWidget->currentIndex()==3)
        {
            ui->listWidget_4->setFocus();
            ui->listWidget_4->setCurrentRow(2);
        }
        break;

    case Qt::Key_4:
        if(ui->stackedWidget->currentIndex()==0)
        {
            ui->listWidget->setFocus();
            ui->listWidget->setCurrentRow(3);
        }
        break;
    }
}


