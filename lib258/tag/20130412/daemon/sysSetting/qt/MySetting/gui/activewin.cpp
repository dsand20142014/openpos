#include "activewin.h"
#include "modules.h"

extern "C"{
#include "devSignal.h"
#include "sysparam.h"
}

ActiveWin::ActiveWin(QWidget *parent) :
    BaseForm("MODULE ACTIVE",parent)
{
    signalMapper = 0;
    vlayout = 0;
    myscroll = 0;
    widget = 0;
    count = 0;

    btnOk->show();
    btnOk->setFocus();
    btnOk->setText("Save");

    for(int i=0;i<NUM;i++)
    {
        label[i]=NULL;
        btn[i]=NULL;
        hlayout[i]=NULL;
        clickNum[i]=0;
        oldNum[i] = 0;
    }

    titleList<<"WIFI"<<"GPS";

    getValue();

    connect(btnOk,SIGNAL(clicked()),this,SLOT(btnOkClick()));

}


bool ActiveWin::getDataInfo(char* group , char* key)
{
    SYS_SETTING sysSetting;

    memset(&sysSetting,0,sizeof(SYS_SETTING));

    strcpy(sysSetting.section,group);
    strcpy(sysSetting.key,key);

    int rec = PARAM_setting_get(&sysSetting);
    if(rec<0) return false;

    ////qDebug()<<"~~~~~~~~sysSetting.value==="<<sysSetting.value;
    clickNum[count] = atoi(sysSetting.value);

    count++;
    return true ;
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

    count = 2;
    for(int i=0;i<count;i++)
    {
        label[i] = new QLabel(titleList.at(i));
        label[i]->setFixedSize(80,30);
        btn[i] = new QPushButton();
        btn[i]->setFixedSize(80,30);

        hlayout[i] = new QHBoxLayout;
        hlayout[i]->addWidget(label[i]);
        hlayout[i]->addWidget(btn[i]);

        vlayout->addLayout(hlayout[i]);

        signalMapper->setMapping(btn[i], i);

        connect(btn[i], SIGNAL(clicked()),signalMapper, SLOT (map()));

        if(clickNum[i]==0){
            btn[i]->setText("CLOSE");
            btn[i]->setStyleSheet("QPushButton{background-color:gray;}");
        }
        else{
            clickNum[i]==1;
            btn[i]->setText("OPEN");
            btn[i]->setStyleSheet("QPushButton{background-color:blue;}");
        }

        if(i==0)
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

void ActiveWin::setDataInfo(char* group, char* key,int i)
{
    SYS_SETTING sysSetting;

    memset(&sysSetting,0,sizeof(SYS_SETTING));

    strcpy(sysSetting.section,group);
    strcpy(sysSetting.key,key);
    sprintf(sysSetting.value,"%d",clickNum[i]);

    ////qDebug()<<"@@@@@@@  clickNum[i]"<<clickNum[i];
    PARAM_setting_set(&sysSetting);
}

void ActiveWin::btnOkClick()
{
    informationBox->setText(tr("  SETTING...  "));
    informationBox->raise();
    informationBox->show();
    btnOk->setEnabled(false);

    QTimer::singleShot(20,this,SLOT(showInfo()));

}

void ActiveWin::showInfo()
{
    if(clickNum[0]!=oldNum[0])
         modules_wifi_active(clickNum[0]);
    if(clickNum[1]!=oldNum[1])
        modules_gps_active(clickNum[1]);

    system("sync");
    informationBox->setText(tr("  SET SUCCESS!  "));
    informationBox->raise();
    informationBox->show();
    QTimer::singleShot(2000,this,SLOT(close()));
}

void ActiveWin::setNum(int i)
{
    ////qDebug()<<"i===="<<i;

    if(btn[i]==NULL) return;

    if(clickNum[i]==1){
        btn[i]->setText("CLOSE");
        btn[i]->setStyleSheet("QPushButton{background-color:gray;}");
        clickNum[i] = 0;
    }else{
        btn[i]->setText("OPEN");
        btn[i]->setStyleSheet("QPushButton{background-color:blue;}");
        clickNum[i] = 1;
    }


}
