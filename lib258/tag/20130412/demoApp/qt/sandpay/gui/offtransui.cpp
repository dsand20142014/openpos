#include "offtransui.h"
#include "ui_offtransui.h"
#include "msginfoui.h"
#include <QtDebug>

#include "mythread.h"
#include "include.h"
#include "global.h"
#include "xdata.h"
OffTransUI::OffTransUI(QWidget *parent) :
    QDialog(parent,Qt::CustomizeWindowHint),
    ui(new Ui::OffTransUI)
{
    ui->setupUi(this);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    desk = 0;
    qDebug()<<"IN offtransUI*********************************";

    account = 0;
    myThread = 0;

    getCardBalance();

    ui->lineEdit->setFocus();

    ui->lineEdit->installEventFilter(this);
}


void OffTransUI::getCardBalance()
{
    uchar aucPrintBuf[31];
    memset(aucPrintBuf,0,sizeof(aucPrintBuf));
    aucPrintBuf[30] = '\0';
    UTIL_Form_Montant(aucPrintBuf, NormalTransData.ulPrevEDBalance, 2);

    ui->label_4->setText(QString((char*)aucPrintBuf));

}

OffTransUI::~OffTransUI()
{

    if(myThread!=NULL){

         delete myThread;
         myThread = NULL;
    }
    delete ui;
}


bool OffTransUI::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->lineEdit)
    {
        if(event->type()==QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            switch(keyEvent->key())
            {
                case Qt::Key_0:
                    setkey(0);
                return true;
                case Qt::Key_1:
                     setkey(1);
                return true;
                case Qt::Key_2:
                     setkey(2);
                return true;
                case Qt::Key_3:
                     setkey(3);
                return true;
                case Qt::Key_4:
                     setkey(4);
                return true;
                case Qt::Key_5:
                     setkey(5);
                return true;
                case Qt::Key_6:
                     setkey(6);
                return true;
                case Qt::Key_7:
                     setkey(7);
                return true;
                case Qt::Key_8:
                     setkey(8);
                return true;
                case Qt::Key_9:          
                     setkey(9);
                return true;

            }

        }


    }

     return QObject::eventFilter(target, event);

}


void OffTransUI::setkey(double key)
{
    qDebug()<<"1111 key*********************************"<<key;

    QString text = ui->lineEdit->text();

    if(text.toDouble() > DataSave0.ConstantParamData.ulMaxXiaoFeiAmount)
        return;

    if (text == "0.00")
    {
        text.replace(3, 1, QString::number(key));
    }
    else if (text.startsWith("0.0"))
    {
        QString cent = QString(text.at(3));
        text.replace(2, 1, cent);
        text.remove(3, 1);
        text.append(QString::number(key));
    }
    else if (text.startsWith("0."))
    {
        QString jiao = QString(text.at(2));
        text.replace(0, 1, jiao);
        QString cent = QString(text.at(3));
        text.replace(2, 1, cent);
        text.remove(3, 1);
        text.append(QString::number(key));
    }
    else
    {
        QStringList list = text.split(".");
        if (!list.isEmpty())
        {
            QString string = QString(list.at(0)) + QString(list.at(1));
            string.append(QString::number(key));
            text = string.insert(string.count() - 2, ".");
        }
    }

    ui->lineEdit->setText(text);

    qDebug()<<"ui->lineEdit->text()======"<<ui->lineEdit->text();
}


void OffTransUI::on_pushButton_clicked()//exit
{
    destroy();
}

void OffTransUI::on_pushButton_2_clicked()//enter
{
    account = ui->lineEdit->text().toDouble()*100;

    desk = new MsgInfoUI(this);
    desk->showFullScreen();

    if(myThread!=NULL){

         delete myThread;
         myThread = NULL;
    }

    ProUiFace.UiToPro.ucTransType= TRANS_S_PURCHASE;

    ProUiFace.UiToPro.ulAmount=account;
    ProUiFace.UiToPro.ulPrevEDBalance =  NormalTransData.ulPrevEDBalance;

    myThread =new MyThread();
    connect(myThread,SIGNAL(signal_mythread()),this,SLOT(destroy_ui()));
    myThread->start();

}


void OffTransUI::destroy_ui()
{
    qDebug()<<"IN OffRANSui******************";
    delete desk;
    this->destroy();
    this->parent()->deleteLater();


}

}
