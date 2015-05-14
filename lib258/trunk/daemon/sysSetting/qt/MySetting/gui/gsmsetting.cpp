#include "gsmsetting.h"

extern "C"{
#include "sysparam.h"
#include "gsm.h"
}

GsmSetting::GsmSetting(QWidget *parent) : BaseForm(tr("GSM SETTING"), parent)
{
    btnOk->show();
    btnOk->setText(tr("Save"));
    btnExit->setFocus();

    moveY = 0;
    moveYmax = 0;
    pressflag = false;

    windowHeight = 0;

    lastpointY = 0;

    myscroll = 0;
    widget = 0;
    vlayout = 0 ;

    for(int i=0;i<MAXCOUNT;i++){
        tabOneLabel[i] = 0;
        tabOneEdit[i] = 0;
        tabOneComB[i] = 0;
    }

    getValue();

    connect(btnOk, SIGNAL(clicked()), this, SLOT(on_pushButton_2_clicked()));
}

void GsmSetting::getValue()
{

//add by xiongjian at 20130201
    SYS_SETTING sysSetting;
    int cn = 0, i = 0;

    myscroll = new QScrollArea();
    myscroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(myscroll);
    widget = new QWidget;
    vlayout = new QVBoxLayout;

    //get WIRELESS_MODULE and draw UI
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"WIRELESS_MODULE");
    cn = PARAM_setting_get(&sysSetting);

    tabOneLabel[0] = new QLabel(tr("%1").arg(sysSetting.key));
    vlayout->addWidget(tabOneLabel[0]);

    tabOneComB[0] = new QComboBox;
    tabOneComB[0]->setFocus();
    i = 0;
    while(i<cn)
        tabOneComB[0]->addItem(QString(sysSetting.briefList[i++]));
    if(atoi(sysSetting.value) > 0)
        tabOneComB[0]->setCurrentIndex(atoi(sysSetting.value)-1);

    vlayout->addWidget(tabOneComB[0]);
    tabOneComB[0]->installEventFilter(this);

    //get DEFAULT_APN and draw UI
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"DEFAULT_APN");
    cn = PARAM_setting_get(&sysSetting);

    tabOneLabel[1] = new QLabel(tr("%1").arg(sysSetting.key));
    vlayout->addWidget(tabOneLabel[1]);

    tabOneComB[1] = new QComboBox;
    tabOneComB[1]->setEditable(true);
    i = 0;
    while(i<cn)
        tabOneComB[1]->addItem(QString(sysSetting.briefList[i++]));

    if(!QString(sysSetting.value).isEmpty())
    {
        for(i=0;i<cn;i++)
        {
            if(strcmp(sysSetting.value,sysSetting.briefList[i]) == 0)
                break;
        }
        if(i >= cn)
            tabOneComB[1]->addItem(QString(sysSetting.value));

        tabOneComB[1]->setEditText(QString(sysSetting.value));
    }

    vlayout->addWidget(tabOneComB[1]);
    tabOneComB[1]->installEventFilter(this);


    //get USERNAME and draw UI
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"USERNAME");
    cn = PARAM_setting_get(&sysSetting);

    tabOneLabel[2] = new QLabel(tr("%1").arg(sysSetting.key));

    vlayout->addWidget(tabOneLabel[2]);

    tabOneEdit[0] = new QLineEdit(QString(sysSetting.value));
    tabOneEdit[0]->setMaxLength(25);
   // tabOneEdit[0]->setGeometry(40,110+160,150,30);
    vlayout->addWidget(tabOneEdit[0]);
    tabOneEdit[0]->installEventFilter(this);

    //get PASSWD and draw UI
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"PASSWD");
    cn = PARAM_setting_get(&sysSetting);

    tabOneLabel[3] = new QLabel(tr("%1").arg(sysSetting.key));
    vlayout->addWidget(tabOneLabel[3]);

    tabOneEdit[1] = new QLineEdit(QString(sysSetting.value));
    tabOneEdit[1]->setMaxLength(25);
    tabOneEdit[1]->setEchoMode(QLineEdit::Password);
    vlayout->addWidget(tabOneEdit[1]);
    tabOneEdit[1]->installEventFilter(this);

    //get DEFAULT_BAND and draw UI
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"DEFAULT_BAND");
    cn = PARAM_setting_get(&sysSetting);

    tabOneLabel[4] = new QLabel(tr("%1").arg(sysSetting.key));
    vlayout->addWidget(tabOneLabel[4]);

    tabOneComB[2] = new QComboBox;

    i = 0;
    while(i<cn)
        tabOneComB[2]->addItem(QString(sysSetting.briefList[i++]));
    if(atoi(sysSetting.value) > 0)
        tabOneComB[2]->setCurrentIndex(atoi(sysSetting.value)-1);

    vlayout->addWidget(tabOneComB[2]);
    tabOneComB[2]->installEventFilter(this);

    widget->setLayout(vlayout);
    myscroll->setWidget(widget);

//add end

}



void GsmSetting::setValue()
{
//add by xiongjian at 20130201

    SYS_SETTING sysSetting;
    char username[50], password[50], apn[50];
    int bandIndex = 1;
    
    
    //check APN value
    if(tabOneComB[1]->currentText().isEmpty())
    {
        showInforText(tr("APN SET ERROR, PLEASE CHECK!"));

        QTimer::singleShot(2000,informationBox,SLOT(hide()));
        btnOk->setEnabled(true);
        return;
    }

    showInforText(tr("SAVING...,WAIT PLEASE!"));

    //set WIRELESS_MODULE
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"WIRELESS_MODULE");
    sprintf(sysSetting.value,"%d",tabOneComB[0]->currentIndex()+1);
    PARAM_setting_set(&sysSetting);

    //set DEFAULT_APN
    memset(apn,0,sizeof(apn));
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"DEFAULT_APN");
    sprintf(sysSetting.value,"%s",tabOneComB[1]->currentText().toLocal8Bit().data());
    PARAM_setting_set(&sysSetting);
    strncpy(apn,sysSetting.value,sizeof(apn)-1);

    //set USERNAME
    memset(username,0,sizeof(username));
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"USERNAME");
    sprintf(sysSetting.value,"%s",tabOneEdit[0]->text().toLocal8Bit().data());
    PARAM_setting_set(&sysSetting);
    strncpy(username,sysSetting.value,sizeof(username)-1);
    

    //set PASSWD
    memset(password,0,sizeof(password));
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"PASSWD");
    sprintf(sysSetting.value,"%s",tabOneEdit[1]->text().toLocal8Bit().data());
    PARAM_setting_set(&sysSetting);
	strncpy(password,sysSetting.value,sizeof(password)-1);

    //set DEFAULT_BAND
    bandIndex = tabOneComB[2]->currentIndex() + 1;

    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"DEFAULT_BAND");
    sprintf(sysSetting.value,"%d",bandIndex);
    PARAM_setting_set(&sysSetting);

    //set param
    GSM_config_pap_chap(username,password);
    GSM_set_APN((unsigned char*)apn);

	GSM__setParam_gprs(bandIndex);



    showInforText(tr("    SET SUCCESS!    "));
    QTimer::singleShot(1000,this,SLOT(close()));

//add end

}


GsmSetting::~GsmSetting()
{
}


void GsmSetting::on_pushButton_2_clicked()
{
    btnOk->setEnabled(false);

    QTimer::singleShot(300,this,SLOT(setValue()));

}


bool GsmSetting::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==tabOneComB[0]||obj==tabOneComB[1]||obj==tabOneComB[2]||obj==tabOneEdit[0]||obj==tabOneEdit[1])
    {
        if(event->type()==QEvent::KeyPress)
        {
            QKeyEvent *keyEve = static_cast<QKeyEvent*>(event) ;
            switch(keyEve->key())
            {
            case Qt::Key_Enter:
                if(tabOneComB[0]->hasFocus())
                    tabOneComB[1]->setFocus();
                else if(tabOneComB[1]->hasFocus())
                    tabOneEdit[0]->setFocus();
                else if(tabOneEdit[0]->hasFocus())
                    tabOneEdit[1]->setFocus();
                else if(tabOneEdit[1]->hasFocus())
                    tabOneComB[2]->setFocus();
                else if(tabOneComB[2]->hasFocus())
                    on_pushButton_2_clicked();
                return true;

            case Qt::Key_Escape:
                close();
                break;

            default:
                return false;
            }

        }
        else
            return false;
    }
    else
        return QObject::eventFilter(obj,event);
}


void GsmSetting::mousePressEvent( QMouseEvent * event )
{
    pressflag = true;
    lastpointY = event->pos().y();
}

void GsmSetting::mouseMoveEvent(QMouseEvent * event)
{
    windowHeight = this->rect().height();

    if(pressflag == true)
    {
        QPoint nowpoint = event->pos();
        int y = nowpoint.y()-lastpointY;

        moveY+=y;
        moveYmax = 480 - windowHeight;


        if(moveYmax<= 0|| abs(moveY)>moveYmax|| moveY>0)
        {
            moveY-=y;
            y = 0 ;
        }
        widget->scroll(0,y);

        lastpointY = nowpoint.y();
    }
}

void GsmSetting::mouseReleaseEvent(QMouseEvent * event)
{
    pressflag = false;
}
