#include "gpstest.h"
#include "ui_gpstest.h"
#include "gpswin.h"

extern "C"{
#include "gps.h"
#include "sand_debug.h"
}
extern void _print(char *fmt,...);

GPSTest::GPSTest(QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint),
    ui(new Ui::GPSTest)
{
    ui->setupUi(this);

    count=0;
    lastCount = 0;

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("/daemon/image/thirdback.png")));
    setPalette(palette);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->stackedWidget->setCurrentIndex(0);

    exitBtn = new Button(tr("Exit"), 1);
    ui->horizontalLayout->addWidget(exitBtn);
    btnDetail = new Button(tr("Detail"));
    ui->horizontalLayout->addWidget(btnDetail);
    setbtn = new Button(tr("Qgps"));
    ui->horizontalLayout->addWidget(setbtn);

    btnDetail->setMinimumSize(68, 36);
    setbtn->setMinimumSize(68, 36);
    exitBtn->setMinimumSize(68, 36);

    exitBtn->setFocus();

    for(int i=0;i<MAXSATNUM;i++)
    {
        barList[i] = 0;
        nameList[i] = 0;
    }

    connect(&timer,SIGNAL(timeout()),SLOT(refresh_data()));
    connect(&timer2,SIGNAL(timeout()),SLOT(showDetail()));

    connect(setbtn, SIGNAL(clicked()), this, SLOT(slot_setbtn()));
    connect(btnDetail,SIGNAL(clicked()),this,SLOT(showDetail()));
    connect(btnDetail,SIGNAL(clicked()),&timer,SLOT(stop()));
    connect(exitBtn,SIGNAL(clicked()),this,SLOT(slot_detailclose()));

    timer.start(100);
}

void GPSTest::slot_setbtn()
{
    setbtn->setEnabled(false);

    GpsWin *gw = new GpsWin();
    gw->show();

    setbtn->setEnabled(true);
}

void GPSTest::showDetail()
{
    btnDetail->setEnabled(false);

     int result = -1;
     GPGGA gpGGA;
     memset(&gpGGA,0,sizeof(GPGGA));
     ui->stackedWidget->setCurrentIndex(1);

     result = getGPSData(C_GPGGA,&gpGGA);

     if(result==0){
         ui->label_13->setText(QString(gpGGA.Longitude));
         ui->label_14->setText(QString::number(gpGGA.LongitudePositionFieldNumber));
         ui->label_15->setText(QString(gpGGA.Latitude));
         ui->label_16->setText(QString::number(gpGGA.LatitudePositionFieldNumber));
         ui->label_17->setText(QString::number(gpGGA.AntennaAltitudeMeters));
     }
     timer2.start(500);
     btnDetail->setEnabled(true);

}

GPSTest::~GPSTest()
{
    delete ui;
}

void GPSTest::refresh_data()
{
    timer.stop();

    GPGSV gpGSV[6];
    int result = -1;

    //qDebug()<<"~~~~~~~~count "<<count;
    for(int i=0;i<count;i++){
        if(barList[i]!=NULL)
            barList[i]->hide();
        if(nameList[i]!=NULL)
            nameList[i]->hide();
    }

    memset(gpGSV,0,sizeof(GPGSV)*6);
    result = getGPSData(C_GPGSV,gpGSV);

    if(result == 0)
    {
        count = gpGSV[0].NumberOfSatellites;
        int k = 0;
        int h  = 0;

        //qDebug()<<"********count"<<count;
         for(int i=0;i<lastCount;i++){
             delete barList[i];
             delete nameList[i];
             barList[i] = 0;
             nameList[i] = 0;
         }

         lastCount = count;

        for(int i=0;i<count;i++){
            //FIXME:Here may cause a mem leak problem,please not to new widget in the timer. zengshu
            barList[i] = new QProgressBar(ui->scrollAreaWidgetContents);
            barList[i]->setOrientation(Qt::Vertical);
            barList[i]->setValue(0);
            barList[i]->setFormat("%v");
            barList[i]->setRange(0,100);
            barList[i]->setAlignment(Qt::AlignCenter);
            barList[i]->setGeometry(i*20,1,20,186);

            nameList[i] = new QLabel(ui->page);
            nameList[i]->setGeometry(30+i*20,186,20,16);
            nameList[i]->setObjectName("gpssatname");

            barList[i]->show();
//            qDebug()<<"nameList[i]->x()"<<nameList[i]->x();
            if(nameList[i]->x()+nameList[i]->width()>230)
                nameList[i]->hide();
            else
                nameList[i]->show();
        }

        //qDebug()<<"zengshu:"<<__LINE__;
        for(int i=0;i<(gpGSV[i].TotalNumberOfMessages>5 ? 5:gpGSV[i].TotalNumberOfMessages);i++)
        {
            //qDebug()<<"gpGSV[i].TotalNumberOfMessages===="<<gpGSV[i].TotalNumberOfMessages;

            for(int j=0;j<4;j++)
            {
                if(gpGSV[i].SatellitesInView[j].SatelliteNumber == 0)
                {
                    continue;
                }
                else{
                    //qDebug()<<"i== j== "<<i<<j;
                    //qDebug()<<"k===="<<k;
                    if((k>count-1)||(h>count-1)) break;
                    if(k>11||h>11) break;

                    _print("\n~~~~~~~~G%d\n",gpGSV[i].SatellitesInView[j].SatelliteNumber);
                    if(barList[k]!=NULL)
                        barList[k]->setValue(gpGSV[i].SatellitesInView[j].SNR);
                    if(nameList[h]!=NULL)
                        nameList[h]->setText("G"+QString::number(gpGSV[i].SatellitesInView[j].SatelliteNumber));
                    k++;
                    h++;
                }
            }
        }
    }

    //qDebug()<<"99999999";
    timer.start(1000);
}

void GPSTest::slot_detailclose()
{
    if(ui->stackedWidget->currentIndex()==1)
    {
        ui->stackedWidget->setCurrentIndex(0);
        timer2.stop();
        timer.start();
    }
    else
    {
      _print("on gpstest close******\n");
      close();
    }
}

void GPSTest::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Escape:
        slot_detailclose();
        break;
    case Qt::Key_Enter:
        showDetail();
        break;
    }
}
