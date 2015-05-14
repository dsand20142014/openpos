#include "msginfo.h"
#include "ui_msginfo.h"

extern "C"{
#include "include.h"
#include "global.h"
#include "guitools.h"
#include "iconv.h"
}
MsgInfo::MsgInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MsgInfo)
{
        qDebug("%d$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",ProUiFace.UiToPro.ucTransType);
    ui->setupUi(this);
    timeOutCnt=0;
    setWindowFlags(Qt::FramelessWindowHint);
    //progressBar = new QProgressBar;
    //mainLayout = new QGridLayout(this);
    //mainLayout->addWidget(progressBar, 1, 1, 1, 1);
    //progressBar->setRange(0,100);
    ui->MsgRst->setReadOnly(true);
    ui->pushButton->setEnabled(false);
    sandtimer = new QTimer();
    sandtimer2 = new QTimer();
    connect(sandtimer,SIGNAL(timeout()),this,SLOT(slottimer1Done()));
    connect(sandtimer2,SIGNAL(timeout()),this,SLOT(slottimer2Done()));
    sandtimer->start( 100 );  //??????
    // sandtimer->singleShot(300,this,SLOT(slottimer1Done()));
    //if(G_NORMALTRANS_ucTransType==TRANS_AID_PARAM||
    //        G_NORMALTRANS_ucTransType==TRANS_RID_PARAM||
    //        G_NORMALTRANS_ucTransType==TRANS_PRINT_BYTRACE||
    //        G_NORMALTRANS_ucTransType==TRANS_PRINT_LASTTRANS||
    //        G_NORMALTRANS_ucTransType==TRANS_PRINT_LASTSETTLE||
    //        G_NORMALTRANS_ucTransType==TRANS_PRINT_TRANSDETAIL||
    //        G_NORMALTRANS_ucTransType==TRANS_PRINT_STATIC||
    //        G_NORMALTRANS_ucTransType==TRANS_SHOW_STATIC||
    //        G_NORMALTRANS_ucTransType==TRANS_SHOW_VERSION||
    //        G_NORMALTRANS_ucTransType==TRANS_ONLINETEST||
    //        G_NORMALTRANS_ucTransType==TRANS_SHOW_TRNSDETAIL||
    //        G_NORMALTRANS_ucTransType==TRANS_PRINT_TTS
    //        )  
    if(!g_ProgressBarFlag)
    {    	
        //G_NORMALTRANS_ucTransType=0x00;
        ui->progressBar->hide();
    }
    else
    {
        ui->progressBar->show();
    }
    qDebug("%d%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",ProUiFace.UiToPro.ucTransType);
    myThread=new MyThread();
    myThread->start();
}

MsgInfo::~MsgInfo()
{
    delete ui;
}

void MsgInfo::on_pushButton_clicked()
{
    int returnFlag;

    sandtimer->stop();
    sandtimer2->stop();
    if(grandFartherFlag == 1)
    {
        returnFlag=1;
    }
    emit(returnGrandFartherMenu(returnFlag));
    ProUiFace.uiProgress=0;
    g_ProgressBarFlag = 0;
    delete myThread;
    destroy();
}

int MsgInfo::gb2312toutf8(char *sourcebuf, size_t sourcelen, char *destbuf, size_t destlen)
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

void MsgInfo::slottimer2Done()
{
    int returnFlag;
    if(grandFartherFlag == 1)
    {
        returnFlag=1;
    }
    timeOutCnt++;
    if(timeOutCnt==3)
    {
        sandtimer2->stop();
        emit(returnGrandFartherMenu(returnFlag));
        ProUiFace.uiProgress=0;
        g_ProgressBarFlag = 0;
        delete myThread;
        destroy();
    }
}

void MsgInfo::slottimer1Done()
{
	unsigned int uiI=0;
    int result;
	char uBuf[1024];
    //int iLen;
	//qDebug("0. slottimer1Done:%d.",ProUiFace.ProToUi.uiLines);
    static int i=0;
    i++;
    if(i==3000)i=0;
    if(ProgressFlag==1)
    {
        ProgressFlag=0;
        sandtimer->stop();
        sandtimer2->start(1000);
        ui->pushButton->setEnabled(true);
    }
    else
    {
        qDebug("\n%d",i);

        sandtimer->stop();

        memset(uBuf,0,sizeof(uBuf));
        memset(uBuf,0,sizeof(uBuf));
        if((ProUiFace.ProToUi.uiLines >= 1)&&(strlen((char*)ProUiFace.ProToUi.aucLine1))){
            gb2312toutf8((char *)ProUiFace.ProToUi.aucLine1,strlen((char *)ProUiFace.ProToUi.aucLine1),uBuf,sizeof(uBuf));
            ui->MsgRst->append(tr(uBuf));

        qDebug("1.MsgInfo aucLine1:%s",uBuf);

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
        if(ProUiFace.UiToPro.ucTransType == TRANS_SHOW_STATIC){
            //ui->progressBar->enabled(false);
        }else{
         ui->progressBar->setValue(ProUiFace.uiProgress);
        }
        qDebug("sem1_wait wait ----------------------------------------------------------------sem_wait");
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
}
