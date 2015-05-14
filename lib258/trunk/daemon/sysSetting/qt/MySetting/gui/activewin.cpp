#include "activewin.h"


extern "C"{
#include "modules.h"
#include "devSignal.h"
#include "sysparam.h"
}

ActiveWin::ActiveWin(QWidget *parent) :
    BaseForm(tr("MODULE ACTIVE"),parent)
{
    signalMapper = 0;
    vlayout = 0;
    myscroll = 0;
    widget = 0;
    count = 0;

    btnOk->show();
    btnOk->setFocus();
    btnOk->setText(tr("Save"));

    for(int i=0;i<NUM;i++)
    {
        label[i]=NULL;
        btn[i]=NULL;
        hlayout[i]=NULL;
        clickNum[i]=0;
        oldNum[i] = 0;
    }

    titleList<<tr("WIFI")<<tr("GPS")<<tr("DEBUG");

    getValue();

    connect(btnOk,SIGNAL(clicked()),this,SLOT(btnOkClick()));

}




void ActiveWin::getValue()
{
    myscroll = new QScrollArea();
    layout->addWidget(myscroll);
    widget = new QWidget;

    vlayout = new QVBoxLayout;
    signalMapper = new QSignalMapper;


    clickNum[0] = Os__get_device_signal(DEV_WIFI);
    oldNum[0] = clickNum[0];

    clickNum[1] =  Os__get_device_signal(DEV_GPS);
    oldNum[1] = clickNum[1];

    clickNum[2] = modules_comdbg_status();

    qDebug()<<"modules_comdbg_status == clickNum[2]"<<modules_comdbg_status()<<clickNum[2];
    count = 3;
    for(int i=0;i<count;i++)
    {
        label[i] = new QLabel(titleList.at(i));
        label[i]->setFixedSize((80*screenWidth)/240,(30*screenHeight)/320);
        btn[i] = new QPushButton();
        btn[i]->setFixedSize((80*screenWidth)/240,(30*screenHeight)/320);

        hlayout[i] = new QHBoxLayout;
        hlayout[i]->addWidget(label[i]);
        hlayout[i]->addWidget(btn[i]);

        vlayout->addLayout(hlayout[i]);

        signalMapper->setMapping(btn[i], i);

        connect(btn[i], SIGNAL(clicked()),signalMapper,SLOT(map()));

        if(clickNum[i]==0){
            btn[i]->setText(tr("CLOSE"));
            btn[i]->setStyleSheet("QPushButton{background-color:gray;}");
        }
        else{
            clickNum[i]==1;
            btn[i]->setText(tr("OPEN"));
            btn[i]->setStyleSheet("QPushButton{background-color:blue;}");
        }


         vlayout->addSpacerItem(new QSpacerItem(10, 12));

    }

    widget->setLayout(vlayout);
    myscroll->setWidget(widget);

    connect(signalMapper, SIGNAL(mapped(int)),this, SLOT(setNum(int)));

    ////qDebug()<<"555sysSetting.value";

}


ActiveWin::~ActiveWin()
{

}



void ActiveWin::btnOkClick()
{
    showInforText(tr("  SETTING...  "));
    btnOk->setEnabled(false);

    QTimer::singleShot(20,this,SLOT(showInfo()));

}

void ActiveWin::showInfo()
{
    if(clickNum[0]!=oldNum[0])
         modules_wifi_active(clickNum[0]);
    if(clickNum[1]!=oldNum[1])
        modules_gps_active(clickNum[1]);
    if(clickNum[2])
        modules_comdbg_open();



    system("sync");
    showInforText(tr(" SET SUCCESS!  "));

    QTimer::singleShot(2000,this,SLOT(close()));
}

void ActiveWin::setNum(int i)
{
   qDebug()<<"i==== clickNum===="<<i<<clickNum[i];

    if(btn[i]==NULL) return;

    if((i==2)&&clickNum[2]) return;

    if(clickNum[i]==1){
        btn[i]->setText(tr("CLOSE"));
        btn[i]->setStyleSheet("QPushButton{background-color:gray;}");
        clickNum[i] = 0;

    }else{
        btn[i]->setText(tr("OPEN"));
        btn[i]->setStyleSheet("QPushButton{background-color:blue;}");
        clickNum[i] = 1;
    }


}
