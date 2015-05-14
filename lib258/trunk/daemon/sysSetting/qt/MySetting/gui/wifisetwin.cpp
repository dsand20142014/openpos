#include "wifisetwin.h"
#include "wifipswdialog.h"
#include "wifiensuredialog.h"

extern void _print(char *fmt,...);

static unsigned char changeFlag = 0x00;


void MyThread::run()
{
    emit emitrecB(true,0);

    static bool flag = false;
    if(!flag)
        conn_init();


    qDebug()<<"changeFlag==="<<changeFlag;
    int recB = connectB(3,changeFlag);
    changeFlag = 0x00;
    qDebug()<<"22changeFlag==="<<changeFlag;

    emit emitrecB(false,recB);

}

WifiSetWin::WifiSetWin(QWidget *parent) :
    BaseForm(tr("WIFI SETTING"),parent)
{
   _print("IN WIFISETWIN***********\n");
    wifiList = new QListWidget;

    QLabel *title = new QLabel(tr(" WLAN 网络"));

    layout->setSpacing(0);
    layout->addWidget(title);
    layout->addWidget(wifiList);

    modifyBtn = new Button(tr("MODIFY"));
    btnlayout->addWidget(modifyBtn);
    btnOk->setText(tr("SCAN"));

    modifyBtn->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);
    btnOk->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);
    btnExit->setMinimumSize((68*screenWidth)/240,(36*screenHeight)/320);
    btnOk->show();

    curMacRow = 0;
    curRow = 0;
    tempname = "";
    arraycount = 0;


    memset(wifiScan,0,sizeof(wifiScan));

    slot_showInfo();

    title->setObjectName("wlanLabel");
    wifiList->setObjectName("wifiList");

    connect(btnOk, SIGNAL(clicked()), this, SLOT(slot_showInfo()));
    connect(modifyBtn,SIGNAL(clicked()),this,SLOT(modifyWifi()));
    connect(wifiList,SIGNAL(clicked(QModelIndex)),this,SLOT(connectSLot(QModelIndex)));

}

WifiSetWin::~WifiSetWin()
{

}

void WifiSetWin::slot_showrecB(bool startFlag,int recB)
{
    int ret = 0;
    if(startFlag){
        slot_disableBtn();

        curRow = wifiList->currentRow();

        disconnect(wifiList,SIGNAL(clicked(QModelIndex)),this,SLOT(connectSLot(QModelIndex)));

        _print("@@curMacRow==%d curRow = %d\n",curMacRow,curRow);

        if(curMacRow!=curRow){
            if(win[curMacRow]!=NULL && name[curMacRow]!=NULL){
                win[curMacRow]->setStyleSheet("QWidget{color:black;}");
                name[curMacRow]->setText(wifiScan[curMacRow].essid);
            }
        }

        qDebug()<<"tempname"<<tempname;

        tempname = QString(wifiScan[curRow].essid) ;

        if(win[curRow]!=NULL && name[curRow]!=NULL){
            win[curRow]->setStyleSheet("QWidget{color:red;}");
            name[curRow]->setText(tempname+"\nconnecting...");
        }


    }
    else{
        char myMAC[59];
        memset(myMAC,0,59);

        WIFI_get_curMAC(myMAC);

        _print("888curMacRow==%d curRow = %d\n",curMacRow,curRow);

        if(win[curRow]!=NULL && name[curRow]!=NULL){
            if(recB>0)
                name[curRow]->setText(tempname+"\nconnected");
            else
                name[curRow]->setText(tempname+"\ndisconnected");
        }

        if(!strcmp(myMAC,wifiScan[curRow].addressMAC)){
            if(win[curRow]!=NULL)
               win[curRow]->setStyleSheet("QWidget{color:red;}");
            curMacRow = curRow;
        }

        slot_enableBtn();

        _print("~~~~~~~~~~recB = %d curRow==%d curMacRow =%d ~~~~~~~~~~~~~~\n",recB,curRow,curMacRow);
        connect(wifiList,SIGNAL(clicked(QModelIndex)),this,SLOT(connectSLot(QModelIndex)));
        ret = Os__get_device_signal(DEV_GPRS_SIGNALL);
        if(recB>0 && ret>= 2 && ret != 9){
             activeDev(DEV_COMM_GSM);
        }

    }



}

void WifiSetWin::modifyWifi()
{
    if(!wifiList->count()) return;

    char addressMAC[50];
    QMessageBox box(QMessageBox::Information,tr(wifiScan[wifiList->currentRow()].essid),tr("MODIFY THIS WLAN?"));
    box.setWindowFlags(Qt::CustomizeWindowHint);

    QPushButton *okBtn = new QPushButton(tr("YES"));
    okBtn->setFixedWidth((80*screenWidth)/240);
    okBtn->setFixedHeight((25*screenHeight)/320);

    QPushButton *noBtn = new QPushButton(tr("NO"));
    noBtn->setFixedWidth((80*screenWidth)/240);
    noBtn->setFixedHeight((25*screenHeight)/320);

    box.addButton(okBtn,QMessageBox::YesRole);
    box.addButton(noBtn,QMessageBox::NoRole);

    box.exec();
    if(box.clickedButton()==okBtn){
        WIFIINFO curmodyWifi;
        int strength;

        int idxrow = wifiList->currentRow();


        memset(&curmodyWifi,0,sizeof(WIFIINFO));

        switch(wifiScan[idxrow].sigLevNum)
        {
        case 1:
            strength = 1;//QObject::tr("week");
            break;
        case 2:
            strength = 2;//QObject::tr("general");
            break;
        case 3:
            strength = 3;//QObject::tr("strong");
            break;
        case 4:
            strength = 4;//QObject::tr("strongest");
            break;
        }


        memcpy(curmodyWifi.addressMAC ,wifiScan[idxrow].addressMAC,50);
        memcpy(curmodyWifi.essid,wifiScan[idxrow].essid,50);

        //检测上一次链接和本次链接是否相同
        memset(addressMAC,0,sizeof(addressMAC));
        if(WIFI_get_curMAC(addressMAC) == 0)
        {
            if(strcmp(wifiScan[idxrow].addressMAC,addressMAC) != 0)
                changeFlag = 0x01;
        }

        if(wifiScan[idxrow].encryption[0]==0x31){
            WifiPswDialog dialog(curmodyWifi.essid,strength,this);
            int ret  = dialog.exec();
            if(ret==QDialog::Accepted){
                curmodyWifi.encryption[0] = 0x31;
                memcpy(curmodyWifi.psk,dialog.curPsw.toAscii().data(),dialog.curPsw.length());

//                strcpy(curmodyWifi.psk,dialog.curPsw.toAscii().data());
                setWifiInfo(curmodyWifi);
            }
        }else{
            WifiEnsureDialog ensureDialog(curmodyWifi.essid,strength,this);
            int ret = ensureDialog.exec();
            if(ret == 1){
                 curmodyWifi.encryption[0]= 0x30;
                 setWifiInfo(curmodyWifi);
            }
            else if(ret ==2){
                removeWifiInfo(wifiScan[idxrow].addressMAC);

            }

        }
    }


}



void WifiSetWin::connectSLot(QModelIndex idx)
{
    if(!wifiList->count()) return;
    WIFIINFO curWifi;
    int strength;
    char addressMAC[50];

    int idxrow = idx.row();


    memset(&curWifi,0,sizeof(WIFIINFO));

    memcpy(curWifi.addressMAC ,wifiScan[idxrow].addressMAC,sizeof(curWifi.addressMAC));
    memcpy(curWifi.essid,wifiScan[idxrow].essid,sizeof(curWifi.essid));


    curRow = idxrow;

    switch(wifiScan[idxrow].sigLevNum)
    {
    case 1:
        strength = 1;//QObject::tr("week");
        break;
    case 2:
        strength = 2;//QObject::tr("general");
        break;
    case 3:
        strength = 3;//QObject::tr("strong");
        break;
    case 4:
        strength = 4;//QObject::tr("strongest");
        break;
    }

    int rec = WIFI_check_list(&curWifi);//0 HAS PSW,
    _print("WIFI_check_list == %d\n",rec);


    if(rec==-2){
        showInforText(tr("ERROR MAC,CHECK PLEASE!"));
        QTimer::singleShot(2000,informationBox,SLOT(hide()));
        return;
    }
    else if(rec==0){
        WifiEnsureDialog ensureDialog(curWifi.essid,strength,this);
        int ret = ensureDialog.exec();
        if(ret == 1)
        {
            memset(addressMAC,0,sizeof(addressMAC));
            if(WIFI_get_curMAC(addressMAC) == 0)
            {
                if(strcmp(curWifi.addressMAC,addressMAC) != 0)
                    changeFlag = 0x01;
            }
             setWifiInfo(curWifi);
        }
        else if(ret ==2)
            removeWifiInfo(wifiScan[idxrow].addressMAC);

    }
    else if(rec ==-1){
        if(wifiScan[idxrow].encryption[0]==0x31){

            WifiPswDialog dialog(curWifi.essid,strength,this);
            int ret  = dialog.exec();
            if(ret==QDialog::Accepted){
                curWifi.encryption[0] = 0x31;

                _print("dialog.curPsw.toAscii().data()=%s\n",dialog.curPsw.toAscii().data());

                _print("dialog.curPsw.length()==%d\n",dialog.curPsw.length());
                memcpy(curWifi.psk,dialog.curPsw.toAscii().data(),dialog.curPsw.length());


                memset(addressMAC,0,sizeof(addressMAC));
                if(WIFI_get_curMAC(addressMAC) == 0)
                {
                    if(strcmp(curWifi.addressMAC,addressMAC) != 0)
                        changeFlag = 0x01;
                }
                _print("~~~curWifi.psk===%s\n",curWifi.psk);

                setWifiInfo(curWifi);
            }
        }else{
            WifiEnsureDialog ensureDialog(curWifi.essid,strength,this);
            int ret = ensureDialog.exec();
            if(ret == 1){
                 curWifi.encryption[0]= 0x30;
                 setWifiInfo(curWifi);
            }
            else if(ret ==2)
                removeWifiInfo(wifiScan[idxrow].addressMAC);

        }
    }
}

void WifiSetWin::slot_showInfo()
{
    showInforText(tr("SCANING...,WIAT PLEASE!"));
    slot_disableBtn();

    QTimer::singleShot(300,this,SLOT(refreshData()));

}

void WifiSetWin::refreshData()
{

    win.clear();
    name.clear();
    pic.clear();

    memset(wifiScan,0,sizeof(wifiScan));

    int rec =  WIFI_scan("wlan0",wifiScan,20);
    if(rec<0){
        showInforText(tr("NO WIFI ,TRY SCAN PLEASE!"));
        QTimer::singleShot(2000,informationBox,SLOT(hide()));
        QTimer::singleShot(1000,this,SLOT(slot_enableBtn()));

       return;
   }

   _print("***************@@@@@@@@@WIFI_scan ==%d\n",rec);

   char myMAC[59];
   memset(myMAC,0,59);

   int rec_mac = WIFI_get_curMAC(myMAC);

   arraycount = rec;

   for(int i=0;i<wifiList->count();i++)
       wifiList->clear();

   for(int i=0;i<arraycount;i++)
       _print("   essid=%s [%d]\n",wifiScan[i].essid,QString(wifiScan[i].sigLev).left(2).toInt());

   for(int i=0;i<arraycount;i++)
       for(int j=0;j<arraycount-i;j++)
           if(QString(wifiScan[j].sigLev).left(2).toInt()<QString(wifiScan[j+1].sigLev).left(2).toInt())
           {
                   WIFISCAN temp = wifiScan[j];
                   wifiScan[j] = wifiScan[j+1];
                   wifiScan[j+1] = temp;
            }

   for(int i=0;i<arraycount;i++)
       _print("!!!essid=%s [%d]\n",wifiScan[i].essid,QString(wifiScan[i].sigLev).left(2).toInt());

   for(int i=0;i<arraycount;i++){
       wifiList->addItem("");

       win.append(new QWidget(this));
       win[i]->setFixedHeight((45*screenHeight)/320);
       lay[i] = new QHBoxLayout;

       _print("wifiScan[i].essid==%s\n",wifiScan[i].essid);

       name.append(new QLabel(tr(wifiScan[i].essid),this));
       pic.append(new QLabel(this));

       name[i]->setFixedWidth(160);
       pic[i]->setFixedWidth(25);

       if(wifiScan[i].encryption[0]==0x31){
           switch(wifiScan[i].sigLevNum)
           {
           case 1:
               pic[i]->setPixmap(QPixmap("/daemon/image/icon/lockwifi-1.png"));
               break;
           case 2:
               pic[i]->setPixmap(QPixmap("/daemon/image/icon/lockwifi-2.png"));
               break;
           case 3:
               pic[i]->setPixmap(QPixmap("/daemon/image/icon/lockwifi-3.png"));
               break;
           case 4:
               pic[i]->setPixmap(QPixmap("/daemon/image/icon/lockwifi-4.png"));
               break;
           }

       }
       else{
           switch(wifiScan[i].sigLevNum)
           {
           case 1:
               pic[i]->setPixmap(QPixmap("/daemon/image/icon/wifi-1.png"));
               break;
           case 2:
               pic[i]->setPixmap(QPixmap("/daemon/image/icon/wifi-2.png"));
               break;
           case 3:
               pic[i]->setPixmap(QPixmap("/daemon/image/icon/wifi-3.png"));
               break;
           case 4:
               pic[i]->setPixmap(QPixmap("/daemon/image/icon/wifi-4.png"));
               break;
           }

       }

       lay[i]->addWidget(name[i]);
       lay[i]->addSpacerItem(new QSpacerItem(80, 10));
       lay[i]->addWidget(pic[i]);

       win[i]->setLayout(lay[i]);

       _print("rec_mac==%d\n",rec_mac);

       if(!rec_mac)
       {
           if(!strcmp(myMAC,wifiScan[i].addressMAC)){
               curMacRow = i;

               win[i]->setStyleSheet("QWidget{color:red;}");

               QString curMacName = wifiScan[i].essid;

               int deviceRec =  Os__get_device_signal(DEV_WIFI);

               _print("deviceRec==%d\n",deviceRec);

               if(deviceRec>1)
                   name[i]->setText(curMacName+"\nconnected");
               else
                   name[i]->setText(curMacName+"\ndisconnected");

           }
       }

       wifiList->setItemWidget(wifiList->item(i),win[i]);

       }

       wifiList->setCurrentRow(0);
       showInforText(tr("SCANING FINISHED!"));
       QTimer::singleShot(1500,informationBox,SLOT(hide()));

       QTimer::singleShot(1000,this,SLOT(slot_enableBtn()));

}



void WifiSetWin::slot_disableBtn()
{
    btnExit->setEnabled(false);
    btnOk->setEnabled(false);
    modifyBtn->setEnabled(false);

}

void WifiSetWin::slot_enableBtn()
{
    btnExit->setEnabled(true);
    btnOk->setEnabled(true);
    modifyBtn->setEnabled(true);
}

void WifiSetWin::setWifiInfo(WIFIINFO curWifi)
{
    _print("curWifi->psk=%s\n",curWifi.psk);
    int rec_set =  WIFI_set_info(&curWifi);

    if(!rec_set){

        system("sync");

        MyThread *mythread = new MyThread;

        connect(mythread,SIGNAL(emitrecB(bool,int)),this,SLOT(slot_showrecB(bool,int)));
        mythread->start();

    }
    else{
        showInforText(tr("SAVE FAILED!"));
        QTimer::singleShot(1500,informationBox,SLOT(hide()));
    }

}


void WifiSetWin::removeWifiInfo(char *addressMAC)
{
    if(!wifiList->count()) return;

    int rec_remove =  WIFI_remove_info(addressMAC);
    if(!rec_remove){
        showInforText(tr("REMOVE SUCCESS!"));
        QTimer::singleShot(3000,informationBox,SLOT(hide()));

        int curWrow = wifiList->currentRow();//3

        delete wifiList->takeItem(curWrow);


        win.removeAt(curWrow);
        name.removeAt(curWrow);
        pic.removeAt(curWrow);
//        lay.removeAt(curWrow);

        if(curMacRow>curWrow)
            curMacRow--;

        for(int i=curWrow;i<arraycount-1;i++){
            wifiScan[i] = wifiScan[i+1];

        }

    }
    else{
        showInforText(tr("REMOVE FAILED!"));
        QTimer::singleShot(3000,informationBox,SLOT(hide()));
    }

}


//void WifiSetWin::ensureWin(char *essid,QString &strength,int row)
//{
//    WIFIINFO curWifi;
//    memset(&curWifi,0,sizeof(WIFIINFO));

//    memcpy(curWifi.addressMAC ,wifiScan[row].addressMAC,50);
//    memcpy(curWifi.essid,wifiScan[row].essid,50);

//    WifiEnsureDialog ensureDialog(essid,strength,this);
//    int ret = ensureDialog.exec();
//    if(ret == 1)//connect
//    {
//        memset(addressMAC,0,sizeof(addressMAC));
//        if(WIFI_get_curMAC(addressMAC) == 0)
//        {
//            if(strcmp(curWifi.addressMAC,addressMAC) != 0)
//                changeFlag = 0x01;
//        }
//         setWifiInfo(&curWifi);
//    }
//    else if(ret ==0 )//remove
//        removeWifiInfo(wifiScan[idx.row()].addressMAC);
//}
