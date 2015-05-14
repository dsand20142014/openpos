#include "secondmenu.h"
#include "autolcdtest.h"
#include "manuallcdtest.h"
#include "printertest.h"
#include "gsmcalltest.h"
#include "gsmbandstest.h"
#include "com2test.h"
#include "asynccardtest.h"
#include "beeptest.h"
#include "usbtest.h"
#include "tftest.h"
#include "card4442test.h"
#include "button.h"
#include "wifitest.h"
#include "test/test.h"
#include "printer.h"


SecondMenu::SecondMenu(QStringList curitems,QWidget *parent) :
    BaseForm(" ",parent)
{
    items = curitems;
    curPage = 0;

    listWidget = new QListWidget;
    qDebug()<<"items"<<items;
    listWidget->addItems(items);
    listWidget->setMinimumHeight(200);

    layout->addWidget(listWidget);
    layout->setContentsMargins(0, 0, 0, 10);

    btnExit->setFocus();

    connect(listWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(slot1(QModelIndex)));

}

void SecondMenu::slot1(QModelIndex idx)
{
    int row = idx.row();
    switch(curPage){
    case 0:{
        switch(row){
        case 0:{
            AutoLcdTest *al = new AutoLcdTest();
            al->showMaximized();
        }
            break;
        case 1:{
            ManualLcdTest *ml = new ManualLcdTest();
            ml->showMaximized();
        }
            break;
        case 2:{
            BeepTest *btt = new BeepTest(1);
            btt->showMaximized();
        }
            break;
        case 3:{
            BeepTest *bt = new BeepTest(2);
            bt->showMaximized();
        }
            break;
        }
    }
        break;
    case 1:{
        switch(row){
        case 0:
        {
            unsigned char ucResult = 0x00;
            ucResult = Os__checkpaper();
            if(ucResult == 0x00)
            {
                QMessageBox box(QMessageBox::Information,"  title ","   CHECK SUCCESS   ");
                box.setWindowFlags(Qt::CustomizeWindowHint);

                QPushButton *okBtn = new QPushButton("Ok");
                okBtn->setFixedWidth(80);
                okBtn->setFixedHeight(25);
                box.addButton(okBtn,QMessageBox::YesRole);

                box.exec();
            }
            else
            {
                QMessageBox box(QMessageBox::Information,"    title   ","NO PAPER, PLEASE CHECK!");
                box.setWindowFlags(Qt::CustomizeWindowHint);

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
                QMessageBox box(QMessageBox::Information,"    title   ","NO PAPER, PLEASE CHECK!");
                box.setWindowFlags(Qt::CustomizeWindowHint);

                QPushButton *okBtn = new QPushButton("Ok");
                okBtn->setFixedWidth(80);
                okBtn->setFixedHeight(25);
                box.addButton(okBtn,QMessageBox::YesRole);

                box.exec();

                return;
            }

            PrinterTest *pt = new PrinterTest();
            pt->showMaximized();
        }
            break;
        case 2:
            Os__linefeed(5);
            break;

        }

    }
        break;
    case 2:{
        switch(row){
        case 0://async card test
        {
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
        break;
    case 3:{
        switch(row)
        {
        case 0:
        {
            GSMCallTest *gc = new GSMCallTest();
            gc->showMaximized();
        }
            break;
        case 1:
        {
            GSMBandsTest *gb = new GSMBandsTest(this);
            gb->show();
        }
            break;

        case 2:
        {
            WifiTest *wt = new WifiTest(0);
            wt->show();
        }
            break;

        }
    }
        break;
    case 4:{
        if(row==0)
        {
            UsbTest *ut = new UsbTest();
            ut->showMaximized();
        }
        else if(row==1)
        {
            TfTest *tf = new TfTest();
            tf->showMaximized();
        }

    }
        break;
    }

}


void SecondMenu::setCurWidget(QString text,int page)
{
    title->setText(text);
    curPage = page;
}


SecondMenu::~SecondMenu()
{
}

void SecondMenu::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Enter:
        slot1(listWidget->currentIndex());
        break;
    case Qt::Key_1:
        listWidget->setCurrentRow(0);
        break;
    case Qt::Key_2:
        listWidget->setCurrentRow(1);

        break;
    case Qt::Key_3:
        if(listWidget->count()>2)
            listWidget->setCurrentRow(2);

        break;

    case Qt::Key_4:
        if(listWidget->count()>3)
          listWidget->setCurrentRow(3);

        break;
    }
}
