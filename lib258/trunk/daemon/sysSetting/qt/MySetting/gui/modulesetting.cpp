#include "modulesetting.h"
#include <QSettings>
#include <QDebug>
#include <QRadioButton>


extern void _print(char *fmt,...);

ModuleSetting::ModuleSetting(QWidget *parent) : BaseForm(tr("MODULE SETTING"), parent)
{
    btnScan = new Button(tr("Scan"));
    btnScan->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);//68, 36);
    btnOk->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);
    btnExit->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);
    btnlayout->addWidget(btnScan, Qt::AlignBottom|Qt::AlignCenter);

    tabWidget = new QTabWidget();
    tab = new QWidget();
    tabWidget->addTab(tab, tr("MODULE"));
    tab_2 = new QWidget();
    tabWidget->addTab(tab_2, tr("DAEMON"));
    tab_3 = new QWidget();
    tabWidget->addTab(tab_3, tr("DRIVER"));
    layout->addWidget(tabWidget);
    layout->setMargin(0);

    btnExit->setFocus();


    moveY = 0;
    moveYmax = 0;
    pressflag = false;
    lastpointY = 0;
    windowHeight = 0;

    scroll1 = 0;
    widget1 = 0;
    vlayout = 0;
    scroll2 = 0;
    widget2 = 0;
    gridlayout = 0;
    scroll3 = 0;
    widget3 = 0;
    gridlayout3 = 0;

    for(int i=0;i<MAXCOUNT;i++){
        debugNamelabel[i] = 0;
        box[i] = 0;
        box2[i] = 0;
        tabOneLabel[i] = 0;
        tabOneEdit[i] = 0;
        Namekolabel[i] = 0;

    }

    btnOk->setText(tr("Save"));
    btnOk->show();

    getInitData();
    drawModuleGUI();

    tabWidget->setCurrentIndex(0);
    tabWidget->setFocusPolicy(Qt::NoFocus);

    connect(tabWidget,SIGNAL(currentChanged(int)),this,SLOT(setScanBtn(int)));
    connect(btnScan,SIGNAL(clicked()),this,SLOT(on_scanButton_clicked()));
    connect(btnOk, SIGNAL(clicked()), this, SLOT(on_pushButton_2_clicked()));
    connect(&barTimer,SIGNAL(timeout()),this,SLOT(slot_barcode()));

}

void ModuleSetting::on_scanButton_clicked()
{
    if(tabWidget->currentIndex()==0){
        Os__bar_open();
        barTimer.start(500);
    }
}

void ModuleSetting::setScanBtn(int idx)
{
    if(tabWidget->currentIndex()==0)
        btnScan->show();
    else{
        barTimer.stop();
        btnScan->hide();
        tabOneEdit[0]->setReadOnly(false);
        tabOneEdit[1]->setReadOnly(false);
        Os__bar_close();

    }

}

void ModuleSetting::slot_barcode()
{
    if(tabOneEdit[0]==NULL&&tabOneEdit[1]==NULL) return;

    if(tabOneEdit[0]->hasFocus())
        tabOneEdit[0]->setReadOnly(true);
    else if (tabOneEdit[1]->hasFocus())
        tabOneEdit[1]->setReadOnly(true);

    barTimer.stop();
    unsigned char buf[256];
    int ret=0;

   memset(buf,0,sizeof(buf));
   ret=Os__bar_read(buf,sizeof(buf));
   _print("bar code buf************==%s  ret ==%d\n",buf,ret);
   if(ret)
   {
       if(tabOneEdit[0]->hasFocus()){
           tabOneEdit[0]->setText(QString((char*)buf));
           tabOneEdit[0]->setReadOnly(false);
       }
       else if(tabOneEdit[1]->hasFocus()){
           tabOneEdit[1]->setText(QString((char*)buf));
           tabOneEdit[1]->setReadOnly(false);
       }

   }

  else barTimer.start(500);


}



void ModuleSetting::on_pushButton_2_clicked()//ok
{
    btnOk->setEnabled(false);

#if 1 //xl def for bug not fixed
    updateInitData();
    setInitData();
    system("sync");

    showInforText(tr("  SET SUCCESS!  "));
#endif
    QTimer::singleShot(2000,this,SLOT(close()));
}


ModuleSetting::~ModuleSetting()
{
    Os__bar_close();
}


void ModuleSetting::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()){
    case Qt::Key_Escape:
    case Qt::Key_Cancel:
        close();
        break;
    case Qt::Key_F1:
            tabWidget->setCurrentIndex(0);
            break;
    case Qt::Key_F2:
            tabWidget->setCurrentIndex(1);
            break;
    case Qt::Key_F3:
            tabWidget->setCurrentIndex(2);
            break;
    case Qt::Key_Enter:
        if(btnOk->hasFocus())
            on_pushButton_2_clicked();
        break;
    }
}


void ModuleSetting::getInitData(void)
{
    SYS_SETTING sysSetting;

    //clear param
    machineSerialNum.clear();
    pinpad_serialNum.clear();
    memset(daemonInitList,0,sizeof(PROCESS_INIT_INFO)*DAEMON_MAX_COUNT);
    memset(driverInitList,0,sizeof(PROCESS_INIT_INFO)*DRIVER_MAX_COUNT);
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    daemonCount = 0;
    driverCount = 0;

    //get machineSerialNum and pinpad_serialNum
    strcpy(sysSetting.section,"MODULE_SETTING");
    strcpy(sysSetting.key,"MACHINE_SERIAL_NO");
    if(PARAM_setting_get(&sysSetting) >= 0)
        machineSerialNum = QString(sysSetting.value);

    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"MODULE_SETTING");
    strcpy(sysSetting.key,"PINPAD_SERIAL_NO");
    if(PARAM_setting_get(&sysSetting) >= 0)
        pinpad_serialNum = QString(sysSetting.value);

    //get daemon init information
    daemonCount = PARAM_shell_get(daemonInitList,DAEMON_MAX_COUNT,DAEMON_SHELL);

    //get drivers init information
    driverCount = PARAM_shell_get(driverInitList,DRIVER_MAX_COUNT,DRIVERS_SHELL);
}
//add by xiongjian at 20130131
void ModuleSetting::setInitData(void)
{
    SYS_SETTING sysSetting;

    memset(&sysSetting,0,sizeof(SYS_SETTING));

    //set machineSerialNum and pinpad_serialNum
    strcpy(sysSetting.section,"MODULE_SETTING");
    strcpy(sysSetting.key,"MACHINE_SERIAL_NO");
    strncpy(sysSetting.value,machineSerialNum.toLocal8Bit().data(),sizeof(sysSetting.value));
    PARAM_setting_set(&sysSetting);

    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"MODULE_SETTING");
    strcpy(sysSetting.key,"PINPAD_SERIAL_NO");
    strncpy(sysSetting.value,pinpad_serialNum.toLocal8Bit().data(),sizeof(sysSetting.value));
     PARAM_setting_set(&sysSetting);

//    set daemon init information
    PARAM_shell_set(daemonInitList,daemonCount,DAEMON_SHELL);

//    set drivers init information
   PARAM_shell_set(driverInitList,driverCount,DRIVERS_SHELL);
}

void ModuleSetting::updateInitData(void)
{
    int i;
    //update machineSerialNum and pinpad_serialNum
    machineSerialNum = tabOneEdit[0]->text();
    pinpad_serialNum = tabOneEdit[1]->text();

    //update daemon init information
    for(i=0;i<daemonCount;i++)
    {

        if(box[i]->isChecked())
            daemonInitList[i].insmod = 1;
        else
            daemonInitList[i].insmod = 0;
        //qDebug()<<"set daemonInitList[i].insmod"<<daemonInitList[i].insmod;


        if(box2[i]->isChecked())
            daemonInitList[i].dbg = 1;
        else
            daemonInitList[i].dbg = 0;
        //qDebug()<<"set daemonInitList[i].dbg"<<daemonInitList[i].dbg;
    }
    //update drivers init information
    for(i=0;i<driverCount;i++)
    {
        if(boxko[i]->isChecked())
            driverInitList[i].insmod = 1;
        else
            driverInitList[i].insmod = 0;

        if(boxko2[i]->isChecked())
            driverInitList[i].dbg = 1;
        else
            driverInitList[i].dbg = 0;
    }
}

void ModuleSetting::drawModuleGUI(void)
{
    int i;
    QString strTmp;
    //draw machineSerialNum and pinpad_serialNum
    scroll1 = new QScrollArea();
    QVBoxLayout *slayout1 = new QVBoxLayout(tab);
    slayout1->addWidget(scroll1);
    slayout1->setMargin(0);
    widget1 = new QWidget;
    vlayout = new QVBoxLayout;
//    vlayout->setContentsMargins(0,5,0,10);

    tabOneLabel[0] = new QLabel(tr("MACHINE_SERIAL_NO"));
    vlayout->addWidget(tabOneLabel[0]);

    tabOneEdit[0] = new QLineEdit(machineSerialNum);
    tabOneEdit[0]->setFixedWidth(screenWidth-40);
    tabOneEdit[0]->setMaxLength(25);
    vlayout->addWidget(tabOneEdit[0]);


    tabOneLabel[1] = new QLabel(tr("PINPAD_SERIAL_NO"));
    vlayout->addWidget(tabOneLabel[1]);

    tabOneEdit[1] = new QLineEdit(pinpad_serialNum);
    tabOneEdit[0]->setFixedWidth(screenWidth-40);

    tabOneEdit[1]->setMaxLength(25);
    vlayout->addWidget(tabOneEdit[1]);

    connect(tabOneEdit[0],SIGNAL(returnPressed()),tabOneEdit[1],SLOT(setFocus()));
    connect(tabOneEdit[1],SIGNAL(returnPressed()),btnOk,SLOT(setFocus()));


    widget1->setLayout(vlayout);
    scroll1->setWidget(widget1);

    //draw daemon init information
    scroll2 = new QScrollArea(tab_2);

    QVBoxLayout *ylayout = new QVBoxLayout(tab_2);
    ylayout->addWidget(scroll2);
    ylayout->setMargin(0);

    widget2 = new QWidget;
    gridlayout = new QGridLayout;

    gridlayout->addWidget(new QLabel(tr("ACTIVE")),0,0);
    gridlayout->addWidget(new QLabel(tr("DEBUG")),0,1);
    gridlayout->addWidget(new QLabel(tr("DAEMON  VERSION")),0,2);

    for(i = 0; i<daemonCount;i++)
     {
        //draw debug checkbox
        strTmp = QString(daemonInitList[i].name) + " " + QString(daemonInitList[i].version);
        debugNamelabel[i]= new QLabel(strTmp);
        box[i] = new QCheckBox;

        //qDebug()<<"daemonInitList[i].insmod"<<daemonInitList[i].insmod;
        if(daemonInitList[i].insmod == 1)
            box[i]->setChecked(true);
        else
            box[i]->setChecked(false);


        box[i]->setEnabled(false);

        box2[i] = new QCheckBox;
        //qDebug()<<"daemonInitList[i].dbg"<<daemonInitList[i].dbg;

        if(daemonInitList[i].dbg == 1)
            box2[i]->setChecked(true);
        else
            box2[i]->setChecked(false);

        gridlayout->addWidget(box[i],i+1,0);
        gridlayout->addWidget(box2[i],i+1,1);
        gridlayout->addWidget(debugNamelabel[i],i+1,2);


     }
    widget2->setLayout(gridlayout);
    scroll2->setWidget(widget2);

    //draw drivers init information
    scroll3 = new QScrollArea();
    scroll3->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *slayout2 = new QVBoxLayout(tab_3);
    slayout2->addWidget(scroll3);
    slayout2->setMargin(0);
    widget3 = new QWidget;
    gridlayout3 = new QGridLayout;

    gridlayout3->addWidget(new QLabel(tr("ACTIVE")),0,0);
    gridlayout3->addWidget(new QLabel(tr("DEBUG")),0,1);
    gridlayout3->addWidget(new QLabel(tr("NAME  VERSION")),0,2);

    for(i = 0; i<driverCount;i++)
    {
       strTmp = QString(driverInitList[i].name) + " " + QString(driverInitList[i].version);
       Namekolabel[i]= new QLabel(strTmp);

       boxko[i] = new QCheckBox;
       boxko2[i] = new QCheckBox;

       if(driverInitList[i].insmod == 0)//active check box
           boxko[i]->setChecked(false);
       else
           boxko[i]->setChecked(true);

       if(driverInitList[i].dbg == 0)//debug check box
           boxko2[i]->setChecked(false);
       else
           boxko2[i]->setChecked(true);

        gridlayout3->addWidget(boxko[i],i+1,0);
        gridlayout3->addWidget(boxko2[i],i+1,1);
        gridlayout3->addWidget(Namekolabel[i],i+1,2);
    }

   widget3->setLayout(gridlayout3);
   scroll3->setWidget(widget3);
}
//add end


void ModuleSetting::mousePressEvent( QMouseEvent * event )
{
    if(tabWidget->currentIndex()!=2) return;
    pressflag = true;
    lastpointY = event->pos().y();
}

void ModuleSetting::mouseMoveEvent(QMouseEvent * event)
{
    if(tabWidget->currentIndex()!=2) return;

    windowHeight = this->rect().height();


    if(pressflag == true)
    {
        QPoint nowpoint = event->pos();
        int y = nowpoint.y()-lastpointY;

        moveY+=y;

        moveYmax = 680 - windowHeight;//360


        _print("moveY==%d  y =%d\n",moveY,y);

        if(moveYmax<= 0|| abs(moveY)>moveYmax|| moveY>0)
        {
            moveY-=y;
            y = 0 ;
        }

        widget3->scroll(0,y);

        lastpointY = nowpoint.y();
    }
}

void ModuleSetting::mouseReleaseEvent(QMouseEvent * event)
{
    if(tabWidget->currentIndex()!=2) return;

    pressflag = false;
}
