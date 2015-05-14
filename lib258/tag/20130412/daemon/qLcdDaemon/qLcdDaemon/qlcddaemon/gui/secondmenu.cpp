#include "secondmenu.h"
extern "C"{
#include "sand_key.h"
}
#include "../smt/downloadMode.h"
#include "../smt/appManagement.h"

SecondMenu::SecondMenu( const QString & text, QWidget *parent) :
    BaseForm(text, parent)
{
    listWidget = new QListWidget();
    layout->addWidget(listWidget);
    layout->setMargin(0);


    connect(listWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(slot1(QModelIndex)));
    //connect(btnOk, SIGNAL(clicked()), this, SLOT(pushButton_clicked()));

    setMenu();
}

void SecondMenu::setMenu()
{
    QListWidgetItem *item1 = new QListWidgetItem(tr("Application Download"));
    QListWidgetItem *item2 = new QListWidgetItem(tr("Application Management"));
    //QListWidgetItem *item3 = new QListWidgetItem(tr("Download Mode"));
    listWidget->addItem(item1);
    listWidget->addItem(item2);
    //listWidget->addItem(item3);
}

void SecondMenu::slot1(QModelIndex idx)
{
    switch(idx.row())
    {
    case 0:
    {
        DownloadMode *dl = new DownloadMode();
        dl->showMaximized();
    }
        break;

    case 1:
    {        
        AppManagement *bt = new AppManagement();
        bt->showMaximized();
    }
        break;

    case 2:
    {
        DownloadMode *dm = new DownloadMode();
        dm->showMaximized();
    }
        break;

    }

}



SecondMenu::~SecondMenu()
{

}

void SecondMenu::pushButton_clicked()
{

}

void SecondMenu::keyPressEvent(QKeyEvent *e)
{    
    Os__set_key_ensure(200);
    //qDebug("AAAAAAAAAAAAAAA after setkey");

    switch(e->key())
    {
    case  Qt::Key_Escape:
    case Qt::Key_Backspace:
        close();
        break;

    case Qt::Key_Enter:        
        slot1(listWidget->currentIndex());
        break;

    case Qt::Key_1:
        listWidget->setFocus();
        listWidget->setCurrentRow(0);
        break;

    case Qt::Key_2:
        listWidget->setFocus();
        listWidget->setCurrentRow(1);
        break;

    case Qt::Key_3:
        listWidget->setFocus();
        listWidget->setCurrentRow(2);
        break;

    }
}
