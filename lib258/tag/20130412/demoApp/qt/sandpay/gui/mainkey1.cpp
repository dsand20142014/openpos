#include "mainkey1.h"
#include "ui_mainkey1.h"
#include "msginfoui.h"

#include "checkpswdialog.h"
#include "changemanagerpsw.h"
#include <QMessageBox>
#include "informationtips.h"
#include <QtDebug>
#include "include.h"
#include "global.h"
#include "xdata.h"


MainKey1::MainKey1(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainKey1)
{
    ui->setupUi(this);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);


    ui->pushButton->setFocus();//设置焦点
    myThread = 0;

}

MainKey1::~MainKey1()
{
    if(myThread!=NULL){

         delete myThread;//在析构时要手动删掉创建在堆上的指针
         myThread = NULL;
    }
    delete ui;

}

void MainKey1::on_pushButton_5_clicked()
{

    destroy();

}



void MainKey1::on_pushButton_clicked()
{
    MsgInfoUI  *desk = new MsgInfoUI(this);
    desk->showFullScreen();
    desk->update();

    if(myThread!=NULL){
         delete myThread;
         myThread = NULL;
    }

    ProUiFace.UiToPro.ucTransType= TRANS_S_LOGONON;

    myThread = new MyThread();
    myThread->start();

    desk->update();
    connect(myThread,SIGNAL(signal_mythread()),desk,SLOT(deleteLater()));//当签到完成时自动关掉desk界面。



}

void MainKey1::on_pushButton_2_clicked()
{

    LOGON_LogoffProcess();

    if(strlen((char*)ProUiFace.ProToUi.aucLine1)){
        InformationTips *info = new InformationTips(this);
        info->setGeometry(10,120,220,100);
        info->setText_UI(QString(tr((char*)ProUiFace.ProToUi.aucLine1)));
        info->setButton(true);
        info->exec();

    }

}

void MainKey1::on_pushButton_3_clicked()
{
    CheckPswDialog checkangePsw ;
    int rec = checkangePsw.exec();
    if(rec == QDialog::Accepted)
    {
        ChangeManagerPsw *changePsw = new ChangeManagerPsw(this);
        changePsw->show();
    }

}

void MainKey1::on_pushButton_4_clicked()//这个要插卡
{
//    MsgInfoUI  *desk = new MsgInfoUI(this);
//    desk->showFullScreen();

//    if(myThread!=NULL){
//         myThread->quit();
//         delete myThread;
//         myThread = NULL;
//    }
//    myThread=new MyThread(this,1);
//    myThread->start();



}


}
