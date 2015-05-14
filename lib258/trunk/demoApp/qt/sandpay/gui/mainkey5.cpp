#include "mainkey5.h"
#include "ui_mainkey5.h"
#include "mainkey51.h"
#include <QtDebug>
#include <QMessageBox>
#include "informationtips.h"

#include "msginfoui.h"
#include "mythread.h"

#include "include.h"
#include "global.h"
#include "xdata.h"

MainKey5::MainKey5(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainKey5)
{
    ui->setupUi(this);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    myThread = 0;
    ui->pushButton->setFocus();
}

MainKey5::~MainKey5()
{
    if(myThread!=NULL){

         delete myThread;
         myThread = NULL;
    }

    delete ui;
}

void MainKey5::on_pushButton_clicked() //参数设置
{
    qDebug()<<"chan shu se zhi*****************";
    MainKey51  *desk = new MainKey51(this);
    desk->showFullScreen();
}

void MainKey5::on_pushButton_2_clicked()//在线初始化
{
    MsgInfoUI *desk = new MsgInfoUI(this);
    desk->showFullScreen();


    if(myThread!=NULL){

         delete myThread;
         myThread = NULL;
    }

    ProUiFace.UiToPro.ucTransType= TRANS_S_INIONLINE;

    myThread = new MyThread();
    myThread->start();

    connect(myThread,SIGNAL(signal_mythread()),desk,SLOT(deleteLater()));

}


void MainKey5::on_pushButton_3_clicked()
{
    MsgInfoUI *desk = new MsgInfoUI(this);
    desk->showFullScreen();
    desk->update();

    if(myThread!=NULL){

         delete myThread;
         myThread = NULL;
    }

    myThread = new MyThread(2);
    myThread->start();

    desk->update();
    connect(myThread,SIGNAL(signal_mythread()),desk,SLOT(deleteLater()));
}

void MainKey5::on_pushButton_4_clicked()
{
    QMessageBox msgbox ;
    msgbox.setWindowFlags(Qt::CustomizeWindowHint);
    msgbox.setText("        "+QString(tr("清结算标志?"))+"        ");
    msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgbox.setDefaultButton(QMessageBox::Yes);
    int ret =  msgbox.exec();
    if(ret == QMessageBox::Yes)
    {
        Os__saved_set((unsigned char *)&DataSave0.ChangeParamData.ucSettleFlag,
                                        FLAG_SETTLE_UNDO,sizeof(unsigned char));
        XDATA_Write_Vaild_File(DataSaveChange);

        InformationTips *info = new InformationTips(this);
        info->setGeometry(10,100,220,100);
        info->setText_UI(QString(tr("清结算标志成功！")));
        info->setButton(false,2000);
        info->show();
    }

  //  CFG_ClearSettleProcess();
}

void MainKey5::on_pushButton_5_clicked()
{
    QMessageBox msgbox ;
    msgbox.setWindowFlags(Qt::CustomizeWindowHint);
    msgbox.setText("        "+QString(tr("清冲正标志?"))+"        ");
    msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgbox.setDefaultButton(QMessageBox::Yes);
    int ret =  msgbox.exec();
    if(ret == QMessageBox::Yes)
    {
        Os__saved_set((unsigned char *)(&DataSave0.ReversalISO8583Data.ucValid),0, sizeof(unsigned char));
        XDATA_Write_Vaild_File(DataSaveReversalISO8583);

        InformationTips *info = new InformationTips(this);
        info->setGeometry(10,100,220,100);
        info->setText_UI(QString(tr("清冲正标志成功！")));
        info->setButton(false,2000);
        info->show();
    }
  //  CFG_ClearReversalProcess();
}

void MainKey5::on_pushButton_6_clicked()
{
    qDebug()<<"00000000adadad******************"<<endl;

    QMessageBox msgbox ;
    msgbox.setWindowFlags(Qt::CustomizeWindowHint);
    msgbox.setText("        "+QString(tr("允许下传输密钥?"))+"        ");
    msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgbox.setDefaultButton(QMessageBox::Yes);
    int ret =  msgbox.exec();
    if(ret == QMessageBox::Yes)
    {
        qDebug()<<"adadad******************"<<endl;
        Os__saved_set((unsigned char *)(&DataSave0.ConstantParamData.ucTranskeyflag),0, sizeof(unsigned char));
        XDATA_Write_Vaild_File(DataSaveConstant);
        qDebug()<<"11111111111adadad******************"<<endl;

        InformationTips *info = new InformationTips(this);
        info->setGeometry(10,100,220,100);
        info->setText_UI(QString(tr("允许下传输密钥成功！")));
        info->setButton(false,2000);
        info->show();
        qDebug()<<"2222adadad******************"<<endl;

    }

}
void MainKey5::on_pushButton_7_clicked()
{
    qDebug()<<"close%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
    destroy();
}

}
