#include "magnetictest.h"
#include <QDebug>
extern "C"{
#include "test/test.h"
#include "sand_debug.h"
#include "fsync_drvs.h"
#include "test/magcom.h"
}

extern void _print(char *fmt,...);

MagneticTest::MagneticTest(QWidget *parent) :
    BaseForm(tr("MAGNETIC CARD TEST"), parent)
{
    //qDebug()<<"*******beginbegin IN MagneticTest() *******";

    pushButton_3 = new Button(tr("Statistic"));
    pushButton_3->hide();

    ucmaglen1 = 0;
    ucmaglen2 = 0;
    ucmaglen3 = 0;
    ucmag2status = 0;
    ucmag1status = 0;
    ucmag3status = 0;

    memset(aucMag1,0,sizeof(aucMag1));
    memset(aucMag2,0,sizeof(aucMag2));
    memset(aucMag3,0,sizeof(aucMag3));

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
    timer2.stop();
    NEW_DRV_TYPE new_drv;
//    _print("111mag swiped \n");


    memset(&new_drv,0,sizeof(NEW_DRV_TYPE));

    unsigned int rec  = Os_Wait_Event(MAG_DRV , &new_drv, 500);
    if(rec== 0){
        if(new_drv.drv_type == MAG_DRV)
       {
            if (&new_drv.drv_data==NULL){
                 timer2.start();
                return ;
            }

            total++;
            tableWidget->setItem(0,0,new QTableWidgetItem(QString::number(total)));
            tableWidget->setItem(1,0,new QTableWidgetItem(QString::number(total)));
            tableWidget->setItem(2,0,new QTableWidgetItem(QString::number(total)));

            unsigned char data1[200];
            unsigned char data2[200];
            unsigned char data3[200];

            uchar len2 = new_drv.drv_data.xxdata[0];
            uchar len3 = new_drv.drv_data.xxdata[len2+2];
            uchar len1 = new_drv.drv_data.xxdata[len2+2+len3+2];

            uchar status2 = new_drv.drv_data.drv_status;
            uchar status3 = new_drv.drv_data.xxdata[len2+1];
            uchar status1 = new_drv.drv_data.xxdata[len2+2+len3+1];

            memset(data1,0,sizeof(data1));
            memset(data2,0,sizeof(data2));
            memset(data3,0,sizeof(data3));

            memcpy(data2,&new_drv.drv_data.xxdata[1],len2);
            memcpy(data3,&new_drv.drv_data.xxdata[len2+1+2],len3);
            memcpy(data1,&new_drv.drv_data.xxdata[len2+1+2+len3+2],len1);

            _print(" pre = %03d %02x %s\n",ucmaglen1,ucmag1status,aucMag1);
            _print("iso1 = %03d %02x %s\n",len1,status1,data1);
            _print(" pre = %03d %02x %s\n",ucmaglen2,ucmag2status,aucMag2);
            _print("iso2 = %03d %02x %s\n",len2,status2,data2);
            _print(" pre = %03d %02x %s\n",ucmaglen3,ucmag3status,aucMag3);
            _print("iso3 = %03d %02x %s\n\n",len3,status3,data3);

            if((len1 == ucmaglen1)&&(status1==ucmag1status)&&(!memcmp(aucMag1,data1,ucmaglen1)))
                tableWidget->setItem(0,1,new QTableWidgetItem(QString::number(++iOkIso1)));
            else if(!iOkIso1)
                tableWidget->setItem(0,1,new QTableWidgetItem("0"));

            if((ucmaglen2==len2)&&(ucmag2status ==status2)&&(!memcmp(aucMag2,data2,ucmaglen2)))
                tableWidget->setItem(1,1,new QTableWidgetItem(QString::number(++iOkIso2)));
            else if(!iOkIso2)
                tableWidget->setItem(1,1,new QTableWidgetItem("0"));

            if((ucmaglen3==len3)&&(ucmag3status ==status3)&&( !memcmp(aucMag3,data3,ucmaglen3)))
                tableWidget->setItem(2,1,new QTableWidgetItem(QString::number(++iOkIso3)));
            else if(!iOkIso3)
                tableWidget->setItem(2,1,new QTableWidgetItem("0"));

            tableWidget->setItem(1,2,new QTableWidgetItem(QString::number(float(iOkIso2*100)/total,'f',1)+"%"));
            tableWidget->setItem(2,2,new QTableWidgetItem(QString::number(float(iOkIso3*100)/total,'f',1)+"%"));
            tableWidget->setItem(0,2,new QTableWidgetItem(QString::number(float(iOkIso1*100)/total,'f',1)+"%"));


            MAGCOM_GetIsoTrack(len1,status1,data1,len2,status2,data2,len3,status3,data3);
        }

    }

    timer2.start();

}

void MagneticTest::getResult()
{
    qDebug()<<"0000~~~~~~~~~~~~in getresult**********";

        unsigned char aucBuf[200];

        char aucDisplay[100];

        textEdit->clear();

        if (&thread->new_drv.drv_data==NULL){
            thread->start();
            return ;
        }


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
   _print("!!on_pushButton_3_clicked\n");


   QMessageBox box(QMessageBox::Information,tr("  title "),tr("   ARE YOU SURE TO STATISTIC?   "));
   box.setWindowFlags(Qt::CustomizeWindowHint);

   QPushButton *okBtn = new QPushButton(tr("Yes"));
   okBtn->setFixedWidth((80*screenWidth)/240);
   okBtn->setFixedHeight((25*screenHeight)/320);
   box.addButton(okBtn,QMessageBox::YesRole);

   QPushButton *noBtn = new QPushButton(tr("No"));
   noBtn->setFixedWidth((80*screenWidth)/240);
   noBtn->setFixedHeight((25*screenHeight)/320);
   box.addButton(noBtn,QMessageBox::NoRole);

   box.setDefaultButton(noBtn);

   box.exec();
   if (box.clickedButton() == noBtn){

       return;
   }

    thread->changeFlag();

    stackedWidget->setCurrentIndex(1);
    pushButton_3->setEnabled(false);

    btnExit->setFocus();

    timer2.start(20);
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

