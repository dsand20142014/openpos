#include "mainkey73.h"
#include "ui_mainkey73.h"
#include "msginfo.h"

#include "mythread.h"
#include "include.h"
#include "global.h"
#include "xdata.h"
#include "guitools.h"
#include "iconv.h"

MainKey73::MainKey73(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainKey73)
{
    ui->setupUi(this);
    sandtimer = new QTimer();
    connect(sandtimer,SIGNAL(timeout()),this,SLOT(slottimer1Done()));
    sandtimer->start( 100 );
    jiaoyiCnt=0;
    ProUiFace.UiToPro.uiIndex = 0;
    if( DataSave0.TransInfoData.auiTransIndex[ProUiFace.UiToPro.uiIndex]){
        ui->MsgRst->setText("");
        ProUiFace.UiToPro.ucTransType = TRANS_SHOW_TRNSDETAIL;
        ui->pushButton_2->setEnabled(false);
        ui->pushButton_3->setEnabled(false);

//        myThreadFlag=1;
//        myThread1=new MyThread();
//        myThread1->start();
         myThread=new MyThread();
         myThread->start();
    }else
        ui->MsgRst->setText("交易为空");
}

MainKey73::~MainKey73()
{
    delete ui;
}

int MainKey73::gb2312toutf8(char *sourcebuf, size_t sourcelen, char *destbuf, size_t destlen)
{
    iconv_t cd;
    if( (cd = iconv_open("utf-8","gb2312")) ==0 )
        return -1;
    memset(destbuf,0,destlen);
    char **source = (char**)&sourcebuf;
    char **dest   = (char**)&destbuf;

    if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))
        return -1;
    iconv_close(cd);
    return 0;

}
void MainKey73::slottimer1Done()
{
    unsigned int uiI=0;

    char uBuf[1024];
    //int iLen;
    unsigned char result;
    //qDebug("0. slottimer1Done:%d.",ProUiFace.ProToUi.uiLines);

    qDebug("The value of ProgressFlag is %d--------------------------------",ProgressFlag);
    if(ProgressFlag==1)
    {
        ProgressFlag=0;
        sandtimer->stop();
        ui->pushButton_2->setEnabled(true);
        ui->pushButton_3->setEnabled(true);
//        switch(myThreadFlag)
//        {
//        case 1:
//            delete myThread1;
//            myThreadFlag=0;
//            break;
//        case 2:
//            delete myThread2;
//            myThreadFlag=0;
//            break;
//        case 3:
//            delete myThread3;
//            myThreadFlag=0;
//            break;
//        }
    }
    else
    {
        sandtimer->stop();
        memset(uBuf,0,sizeof(uBuf));
        memset(uBuf,0,sizeof(uBuf));
        if((ProUiFace.ProToUi.uiLines >= 1)&&(strlen((char*)ProUiFace.ProToUi.aucLine1))){
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine1,strlen((char *)ProUiFace.ProToUi.aucLine1),uBuf,sizeof(uBuf));
            ui->MsgRst->append(tr(uBuf));

            qDebug("1. aucLine1:%s",uBuf);

        }
        memset(uBuf,0,sizeof(uBuf));
        if((ProUiFace.ProToUi.uiLines >= 2)&&(strlen((char*)ProUiFace.ProToUi.aucLine2))){
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine2,strlen((char *)ProUiFace.ProToUi.aucLine2),uBuf,sizeof(uBuf));
            ui->MsgRst->append(tr(uBuf));

            qDebug("2. aucLine2:%s",uBuf);

        }
        memset(uBuf,0,sizeof(uBuf));
        if((ProUiFace.ProToUi.uiLines >= 3)&&(strlen((char*)ProUiFace.ProToUi.aucLine3))){
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine3,strlen((char *)ProUiFace.ProToUi.aucLine3),uBuf,sizeof(uBuf));
            ui->MsgRst->append(tr(uBuf));
            qDebug("3. aucLine3:%s",uBuf);

        }
        memset(uBuf,0,sizeof(uBuf));
        if((ProUiFace.ProToUi.uiLines >= 4)&&(strlen((char*)ProUiFace.ProToUi.aucLine4))){
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine4,strlen((char *)ProUiFace.ProToUi.aucLine4),uBuf,sizeof(uBuf));
            ui->MsgRst->append(tr(uBuf));
            qDebug("4. aucLine4:%s",uBuf);
        }

        memset(uBuf,0,sizeof(uBuf));
        if((ProUiFace.ProToUi.uiLines >= 5)&&(strlen((char*)ProUiFace.ProToUi.aucLine5))){
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine5,strlen((char *)ProUiFace.ProToUi.aucLine5),uBuf,sizeof(uBuf));
            ui->MsgRst->append(tr(uBuf));
            qDebug("5. aucLine4:%s",uBuf);
        }
        memset(uBuf,0,sizeof(uBuf));
        if((ProUiFace.ProToUi.uiLines >= 6)&&(strlen((char*)ProUiFace.ProToUi.aucLine6))){
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine6,strlen((char *)ProUiFace.ProToUi.aucLine6),uBuf,sizeof(uBuf));
            ui->MsgRst->append(tr(uBuf));
            qDebug("6. aucLine4:%s",uBuf);
        }
        memset(uBuf,0,sizeof(uBuf));
        if((ProUiFace.ProToUi.uiLines >= 7)&&(strlen((char*)ProUiFace.ProToUi.aucLine7))){
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine7,strlen((char *)ProUiFace.ProToUi.aucLine7),uBuf,sizeof(uBuf));
            ui->MsgRst->append(tr(uBuf));
            qDebug("7. aucLine4:%s",uBuf);
        }
        memset(uBuf,0,sizeof(uBuf));
        if((ProUiFace.ProToUi.uiLines >= 8)&&(strlen((char*)ProUiFace.ProToUi.aucLine8))){
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine8,strlen((char *)ProUiFace.ProToUi.aucLine8),uBuf,sizeof(uBuf));
            ui->MsgRst->append(tr(uBuf));
            qDebug("8. aucLine4:%s",uBuf);
        }
        ProUiFace.ProToUi.uiLines=0;
        //ui->progressBar->setValue(ProUiFace.uiProgress);
        result=sem_wait(&binSem1);
        if(result==0)
        {
        qDebug("sem1_wait wait success----------------------------------------------------sem_wait");
        }
        else
        {
        qDebug("sem1_wait wait failure------------------------------------------------------sem_wait");
        }
        sem_post(&binSem2);
        sandtimer->start(100);
    }
}
void MainKey73::on_pushButton_clicked()
{
    sandtimer->stop();
    delete myThread;
    destroy();
}

void MainKey73::on_pushButton_2_clicked()
{
    // qianyibi jiaoyi
    sandtimer->start(100);
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(false);
    ProUiFace.UiToPro.uiIndex=jiaoyiCnt;
    if(ProUiFace.UiToPro.uiIndex>0){
        ProUiFace.UiToPro.uiIndex--;
        jiaoyiCnt=ProUiFace.UiToPro.uiIndex;
        if( DataSave0.TransInfoData.auiTransIndex[ProUiFace.UiToPro.uiIndex]){
            ui->MsgRst->setText("");
            ProUiFace.UiToPro.ucTransType = TRANS_SHOW_TRNSDETAIL;
            //myThreadFlag=2;
            //MyThread *myThread2=new MyThread();
            //myThread2->start();
            if(myThread->isFinished())
            {
                myThread->start();
            }
            else
            {
                myThread->wait();
                myThread->start();
            }
        }
    }
    else
    {
        if(myThread->isFinished())
        {
            myThread->start();
        }
        else
        {
            myThread->wait();
            myThread->start();
        }
    }
}

void MainKey73::on_pushButton_3_clicked()
{
    // houyibi jiaoyi
    sandtimer->start(100);
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(false);
    ProUiFace.UiToPro.uiIndex=jiaoyiCnt;
    if(ProUiFace.UiToPro.uiIndex < 300){
        ProUiFace.UiToPro.uiIndex++;
        jiaoyiCnt=ProUiFace.UiToPro.uiIndex;
        if( DataSave0.TransInfoData.auiTransIndex[ProUiFace.UiToPro.uiIndex])
        {
            ui->MsgRst->setText("");
            ProUiFace.UiToPro.ucTransType = TRANS_SHOW_TRNSDETAIL;
//            myThreadFlag=3;
//            MyThread *myThread3=new MyThread();
//            myThread3->start();
            if(myThread->isFinished())
            {
                myThread->start();
            }
            else
            {
                myThread->wait();
                myThread->start();
            }
        }else
            jiaoyiCnt--;

    }
}

}
