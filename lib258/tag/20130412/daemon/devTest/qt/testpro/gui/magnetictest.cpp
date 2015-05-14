#include "magnetictest.h"
#include <QDebug>
extern "C"{
#include "test/test.h"
#include "sand_debug.h"
}

extern void _print(char *fmt,...);

MagneticTest::MagneticTest(QWidget *parent) :
    BaseForm(tr("MAGNETIC CARD TEST"), parent)
{
    //qDebug()<<"*******beginbegin IN MagneticTest() *******";

    pushButton_3 = new Button(tr("Statistic"));
    pushButton_3->hide();


    //qDebug()<<"*******111 beginbegin IN MagneticTest() *******";

    textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    textEdit->setTextColor(QColor(0,0,255));


    tableWidget = new QTableWidget(3,3);
    tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem(tr("TOTAL")));
    tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem(tr("OK")));
    tableWidget->setHorizontalHeaderItem(2,new QTableWidgetItem(tr("RATE")));

    tableWidget->setVerticalHeaderItem(0,new QTableWidgetItem(tr("ISO1")));
    tableWidget->setVerticalHeaderItem(1,new QTableWidgetItem(tr("ISO2")));
    tableWidget->setVerticalHeaderItem(2,new QTableWidgetItem(tr("ISO3")));
    //qDebug()<<"*******222 beginbegin IN MagneticTest() *******";

    QHeaderView *ss = new QHeaderView(Qt::Horizontal);
    ss->setDefaultSectionSize(58);
    tableWidget->setHorizontalHeader(ss);

    QHeaderView *hv1 = new QHeaderView(Qt::Vertical);
    hv1->setDefaultSectionSize(55);
    tableWidget->setVerticalHeader(hv1);


    //qDebug()<<"*******333 beginbegin IN MagneticTest() *******";

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(textEdit);
    stackedWidget->addWidget(tableWidget);

    stackedWidget->setCurrentIndex(0);

    //qDebug()<<"*******444 beginbegin IN MagneticTest() *******";

    layout->addWidget(stackedWidget);

    thread = new CardThread(MAG_DRV,this);
    connect(thread,SIGNAL(signalCard()),this,SLOT(getResult()));
    connect(thread,SIGNAL(signalExit()),this,SLOT(slotExit()));
    thread->start();


    //qDebug()<<"*******555 beginbegin IN MagneticTest() *******";

    tableWidget->setFocusPolicy(Qt::NoFocus);

    btnExit->setFocus();

    total = 0;
    iOkIso1 = 0;
    iOkIso2 = 0;
    iOkIso3 = 0;
    //qDebug()<<"*******666 beginbegin IN MagneticTest() *******";

    connect(&timer2,SIGNAL(timeout()),this,SLOT(getStatics()));

    connect(btnExit,SIGNAL(clicked()),this,SLOT(slotExit()));

    connect(pushButton_3, SIGNAL(clicked()), this, SLOT(on_pushButton_3_clicked()));


}

MagneticTest::~MagneticTest()
{
    //qDebug()<<"******* IN ~MagneticTest *******"<<thread->isFinished();
    thread->quit();
    thread->wait();
    thread->deleteLater();

    //qDebug()<<"*******endend IN ~MagneticTest *******"<<thread->isFinished();

}


void MagneticTest::getStatics()
{
    NEW_DRV_TYPE new_drv;
    Os_Wait_Event(KEY_DRV|MAG_DRV , &new_drv, 1000);

    if(new_drv.drv_type == KEY_DRV)
        if(new_drv.drv_data.xxdata[1]==KEY_CLEAR)
        {
            btnOk->setEnabled(true);

            return ;
        }

    if(new_drv.drv_type == MAG_DRV)
    {
        _print("mag swiped \n");

        if (&new_drv.drv_data==NULL)
        {
            btnOk->setEnabled(true);

            return ;
        }

        total++;
        tableWidget->setItem(0,0,new QTableWidgetItem(QString::number(total)));
        tableWidget->setItem(1,0,new QTableWidgetItem(QString::number(total)));
        tableWidget->setItem(2,0,new QTableWidgetItem(QString::number(total)));
        //qDebug()<<"total==="<<total;


        if((ucmaglen2==new_drv.drv_data.xxdata[0])&&(ucmag2status == new_drv.drv_data.drv_status)&&( !memcmp(aucMag2,&new_drv.drv_data.xxdata[1],ucmaglen2)))
            tableWidget->setItem(1,1,new QTableWidgetItem(QString::number(++iOkIso2)));
        if((ucmaglen3==new_drv.drv_data.xxdata[ucmaglen2+2])&&(ucmag3status ==new_drv.drv_data.xxdata[ucmaglen2+1])&&( !memcmp(aucMag3,&new_drv.drv_data.xxdata[ucmaglen2+1+2],ucmaglen3)))
            tableWidget->setItem(2,1,new QTableWidgetItem(QString::number(++iOkIso3)));
        if((ucmaglen1 = new_drv.drv_data.xxdata[ucmaglen2+2+ucmaglen3+2])&&(ucmag1status == new_drv.drv_data.xxdata[ucmaglen2+2+ucmaglen3+1])&&( !memcmp(aucMag1,&new_drv.drv_data.xxdata[ucmaglen2+1+2+ucmaglen3+2],ucmaglen1)))
            tableWidget->setItem(0,1,new QTableWidgetItem(QString::number(++iOkIso1)));

        //qDebug()<<"iOkIso2==="<<iOkIso2;
        //qDebug()<<"iOkIso3==="<<iOkIso3;
        //qDebug()<<"iOkIso2==="<<iOkIso1;

        ucmaglen2 = new_drv.drv_data.xxdata[0];
        ucmaglen3 = new_drv.drv_data.xxdata[ucmaglen2+2];
        ucmaglen1 = new_drv.drv_data.xxdata[ucmaglen2+2+ucmaglen3+2];

        ucmag2status = new_drv.drv_data.drv_status;
        ucmag3status = new_drv.drv_data.xxdata[ucmaglen2+1];
        ucmag1status = new_drv.drv_data.xxdata[ucmaglen2+2+ucmaglen3+1];

        memset(aucMag1,0,sizeof(aucMag1));
        memset(aucMag2,0,sizeof(aucMag2));
        memset(aucMag3,0,sizeof(aucMag3));

        memcpy(aucMag2,&new_drv.drv_data.xxdata[1],ucmaglen2);
        memcpy(aucMag3,&new_drv.drv_data.xxdata[ucmaglen2+1+2],ucmaglen3);
        memcpy(aucMag1,&new_drv.drv_data.xxdata[ucmaglen2+1+2+ucmaglen3+2],ucmaglen1);


        tableWidget->setItem(1,2,new QTableWidgetItem(QString::number(float(iOkIso2*100)/total,'f',1)+"%"));
        tableWidget->setItem(2,2,new QTableWidgetItem(QString::number(float(iOkIso3*100)/total,'f',1)+"%"));
        tableWidget->setItem(0,2,new QTableWidgetItem(QString::number(float(iOkIso1*100)/total,'f',1)+"%"));

    }
    btnOk->setEnabled(true);

}

void MagneticTest::getResult()
{
    //qDebug()<<"0000~~~~~~~~~~~~in getresult**********";


        unsigned char aucBuf[200];

        char aucDisplay[100];

        textEdit->clear();

        if (&thread->new_drv.drv_data==NULL){
            thread->start();
            return ;
        }

        ucmaglen1 = 0;
        ucmaglen2 = 0;
        ucmaglen3 = 0;

        ucmaglen2 = thread->new_drv.drv_data.xxdata[0];
        ucmaglen3 = thread->new_drv.drv_data.xxdata[ucmaglen2+2];
        ucmaglen1 = thread->new_drv.drv_data.xxdata[ucmaglen2+2+ucmaglen3+2];

        ucmag2status = thread->new_drv.drv_data.drv_status;
        ucmag3status = thread->new_drv.drv_data.xxdata[ucmaglen2+1];
        ucmag1status = thread->new_drv.drv_data.xxdata[ucmaglen2+2+ucmaglen3+1];


        memset(aucMag1,0,sizeof(aucMag1));
        memset(aucMag2,0,sizeof(aucMag2));
        memset(aucMag3,0,sizeof(aucMag3));

        memcpy(aucMag2,&thread->new_drv.drv_data.xxdata[1],ucmaglen2);
        memcpy(aucMag3,&thread->new_drv.drv_data.xxdata[ucmaglen2+1+2],ucmaglen3);
        memcpy(aucMag1,&thread->new_drv.drv_data.xxdata[ucmaglen2+1+2+ucmaglen3+2],ucmaglen1);

        memset(aucDisplay,0,sizeof(aucDisplay));
        memcpy(aucDisplay,"TRACK 2 ",8);
        sprintf(&aucDisplay[8],"%02x  ",ucmag2status);

        sprintf(&aucDisplay[11],"LEN %d",ucmaglen2);

        textEdit->append(QString(aucDisplay));

        if(ucmag2status==0x00){
            memset(aucBuf,0,sizeof(aucBuf));

            memcpy(aucBuf,aucMag2,sizeof(aucMag2));

            textEdit->append(QString((char*)aucBuf));

        }
        textEdit->append("");


        memset(aucDisplay,0,sizeof(aucDisplay));
        memcpy(aucDisplay,"TRACK 3 ",8);

        sprintf(&aucDisplay[8],"%02x  ",ucmag3status);

        sprintf(&aucDisplay[11],"LEN %d",ucmaglen3);

        textEdit->append(QString(aucDisplay));

        if(ucmag3status==0x00){
            memset(aucBuf,0,sizeof(aucBuf));

            memcpy(aucBuf,aucMag3,sizeof(aucMag3));

            textEdit->append(QString((char*)aucBuf));

        }


        textEdit->append("");

        memset(aucDisplay,0,sizeof(aucDisplay));
        memcpy(aucDisplay,"TRACK 1 ",8);
        sprintf(&aucDisplay[8],"%02x  ",ucmag1status);

        sprintf(&aucDisplay[11],"LEN %d",ucmaglen1);


        textEdit->append(QString(aucDisplay));

        if(ucmag1status==0x00){
            memset(aucBuf,0,sizeof(aucBuf));

            memcpy(aucBuf,aucMag1,sizeof(aucMag1));

            textEdit->append(QString((char*)aucBuf));
        }



    btnlayout->addWidget(pushButton_3);

    pushButton_3->show();

    thread->wakeUp();

}


void MagneticTest::slotFresh()
{
    //qDebug()<<"slot fresh************";
    textEdit->clear();

}

void MagneticTest::slotExit()
{
    thread->changeFlag();
    timer2.stop();
//    //qDebug()<<"*********thread->runflag"<<thread->runflag;
    close();
}


void MagneticTest::on_pushButton_3_clicked()
{
    //qDebug()<<"!!on_pushButton_3_clicked";
    btnOk->setEnabled(false);

    thread->changeFlag();

    stackedWidget->setCurrentIndex(1);
    btnExit->setFocus();

    timer2.start(100);
}

void MagneticTest::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Enter:
        if(!pushButton_3->isHidden())
            on_pushButton_3_clicked();
        break;
    case Qt::Key_Escape:
            slotExit();
        break;
    }
}

