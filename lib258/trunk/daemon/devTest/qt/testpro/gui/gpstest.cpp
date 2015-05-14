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

    int screenWidth = qApp->desktop()->screen(0)->width();
    int screenHeight = qApp->desktop()->screen(0)->height();
    this->resize(screenWidth ,screenHeight );

    ui->stackedWidget->setCurrentIndex(0);

    exitBtn = new Button(tr("Exit"), 1);
    ui->horizontalLayout->addWidget(exitBtn);
    btnDetail = new Button(tr("Detail"));
    ui->horizontalLayout->addWidget(btnDetail);
    setbtn = new Button(tr("Qgps"));
    ui->horizontalLayout->addWidget(setbtn);

    btnDetail->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);//68, 36);
    setbtn->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);//68, 36);
    exitBtn->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);//68, 36);

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

    timer.stop();
    timer2.stop();

    GpsWin *gw = new GpsWin();
    gw->show();
    connect(gw,SIGNAL(destroyed()),&timer,SLOT(start()));

    setbtn->setEnabled(true);
}

void GPSTest::showDetail()
{
    btnDetail->setEnabled(false);
    setbtn->setEnabled(false);

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
     setbtn->setEnabled(true);

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
        int k = 0;
        int h  = 0;

        count = gpGSV[0].NumberOfSatellites;

        for(int i=0;i<lastCount;i++){
             delete barList[i];
             delete nameList[i];
             barList[i] = 0;
             nameList[i] = 0;
         }

         lastCount = count;
         _print("count =%d\n",count);


        SATELLITE_DATA sateData[count];
        memset(sateData,0,sizeof(SATELLITE_DATA)*count);

        for(int i=0;i<count;i++){

            //FIXME:Here may cause a mem leak problem,please not to new widget in the timer. zengshu
            barList[i] = new QProgressBar(ui->scrollAreaWidgetContents);
            barList[i]->setOrientation(Qt::Vertical);
            barList[i]->setValue(0);
            barList[i]->setFormat("%v");
            barList[i]->setRange(0,50);
            barList[i]->setAlignment(Qt::AlignCenter);
            barList[i]->setGeometry(i*20,1,20,186);

            nameList[i] = new QLabel(ui->page);
            nameList[i]->setGeometry(ui->scrollArea->geometry().x()+i*20,186,20,16);
            nameList[i]->setObjectName("gpssatname");

            barList[i]->show();
            if(nameList[i]->x()+nameList[i]->width()>(qApp->desktop()->screen(0)->width()-10))
                nameList[i]->hide();
            else
                nameList[i]->show();

          }

        for(int i=0;i<(gpGSV[i].TotalNumberOfMessages>5 ? 5:gpGSV[i].TotalNumberOfMessages);i++)
        {
            for(int j=0;j<4;j++)
            {
                if(gpGSV[i].SatellitesInView[j].SatelliteNumber == 0)
                {
                    continue;
                }
                else
                {
                    if((k>count-1)||(h>count-1)) break;
                    if(k>11||h>11) break;

                    _print("aaa snr =%d  name =%d\n",gpGSV[i].SatellitesInView[j].SNR,gpGSV[i].SatellitesInView[j].SatelliteNumber);
                    sateData[k] = gpGSV[i].SatellitesInView[j];
                    _print("bbb snr =%d  name =%d\n",sateData[k].SNR,sateData[k].SatelliteNumber);
                    k++;

                }
            }
        }


        for(int i=0;i<k-1;i++)
            for(int j=0;j<k-1-i;j++){
//                _print("~~~~~~sateData[j].SNR = %d  sateData[j+1].SNR = %d\n",sateData[j].SNR,sateData[j+1].SNR);
                if(sateData[j].SNR<sateData[j+1].SNR)
                {
                    SATELLITE_DATA temp = sateData[j];
                    sateData[j] = sateData[j+1];
                    sateData[j+1] = temp;

                }
            }

        for(int i=0;i<count;i++){
            _print("ccc snr =%d name =%d\n",sateData[i].SNR,sateData[i].SatelliteNumber);

            if(barList[i]!=NULL)
                barList[i]->setValue(sateData[i].SNR);

            if(nameList[i]!=NULL)
                nameList[i]->setText("G"+QString::number(sateData[i].SatelliteNumber));
        }


    }


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
