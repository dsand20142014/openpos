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
    BaseForm("WIFI SETTING",parent)
{
   _print("IN WIFISETWIN***********\n");
    wifiList = new QListWidget;

    QLabel *title = new QLabel(tr(" WLAN 网络"));

    layout->setSpacing(0);
    layout->addWidget(title);
    layout->addWidget(wifiList);

    modifyBtn = new Button("MODIFY");
    btnlayout->addWidget(modifyBtn);
    btnOk->setText("SCAN");
    modifyBtn->setMinimumSize(68, 36);
    btnOk->setMinimumSize(68, 36);
    btnExit->setMinimumSize(68, 36);

    btnOk->show();


    curMacRow = 0;
    curMacRow = 0;


    memset(wifiScan,0,sizeof(wifiScan));

    for(int i=0;i<20;i++)
    {
        win[i] = NULL;
        name[i] = NULL;
        lay[i] = NULL;
        pic[i] = NULL;
    }

    QTimer::singleShot(50,this,SLOT(refreshData()));

    title->setStyleSheet("QLabel{background-color:gray;color:white;}");
    wifiList->setStyleSheet("QListWidget::item{height:45px;} QListWidget::item:selected{background-color:rgb(237,234,247);}");


    connect(btnOk, SIGNAL(clicked()), this, SLOT(refreshData()));
    connect(modifyBtn,SIGNAL(clicked()),this,SLOT(modifyWifi()));
    connect(wifiList,SIGNAL(clicked(QModelIndex)),this,SLOT(connectSLot(QModelIndex)));

}

WifiSetWin::~WifiSetWin()
{

}

void WifiSetWin::slot_showrecB(bool startFlag,int recB)
{
//    int curROW = 0;


    if(startFlag){
        slot_disableBtn();

        disconnect(wifiList,SIGNAL(clicked(QModelIndex)),this,SLOT(connectSLot(QModelIndex)));

        win[curMacRow]->setStyleSheet("QWidget{color:black;font: 12px ;}");

        name[curMacRow]->setText(wifiScan[curMacRow].essid);

        curRow = wifiList->currentRow();
        tempname = wifiScan[curRow].essid;

        win[curRow]->setStyleSheet("QWidget{color:red;}");

        name[curRow]->setText(tempname+"\nconnectting...");


    }
    else{

        char myMAC[59];
        memset(myMAC,0,59);

        int rec_mac = WIFI_get_curMAC(myMAC);

        if(recB>0)
            name[curRow]->setText(tempname+"\nconnected");
        else
            name[curRow]->setText(tempname+"\nconnect failed");


        if(!strcmp(myMAC,wifiScan[curRow].addressMAC)){
            win[curRow]->setStyleSheet("QWidget{color:red;font:bold 12px ;}");
            curMacRow = curRow;
        }

        slot_enableBtn();
        connect(wifiList,SIGNAL(clicked(QModelIndex)),this,SLOT(connectSLot(QModelIndex)));

    }

    _print("~~~~~~~~~~recB = %d curRow==%d curMacRow =%d ~~~~~~~~~~~~~~\n",recB,curRow,curMacRow);


}

void WifiSetWin::modifyWifi()
{
    char addressMAC[50];
    QMessageBox box(QMessageBox::Information,tr(wifiScan[wifiList->currentRow()].essid),tr("MODIFY THIS WLAN?"));
    box.setWindowFlags(Qt::CustomizeWindowHint);

    QPushButton *okBtn = new QPushButton(tr("YES"));
    okBtn->setFixedWidth(80);
    okBtn->setFixedHeight(25);

    QPushButton *noBtn = new QPushButton(tr("NO"));
    noBtn->setFixedWidth(80);
    noBtn->setFixedHeight(25);

    box.addButton(okBtn,QMessageBox::YesRole);
    box.addButton(noBtn,QMessageBox::NoRole);

    box.exec();
    if(box.clickedButton()==okBtn){
        WIFIINFO curmodyWifi;
        QString strength;

        memset(&curmodyWifi,0,sizeof(WIFIINFO));

        switch(wifiScan[wifiList->currentRow()].sigLevNum)
        {
        case 1:
            strength = "week";
            break;
        case 2:
            strength = "general";
            break;
        case 3:
            strength = "strong";
            break;
        case 4:
            strength = "strongest";
            break;
        }


        memcpy(curmodyWifi.addressMAC ,wifiScan[wifiList->currentRow()].addressMAC,50);
        memcpy(curmodyWifi.essid,wifiScan[wifiList->currentRow()].essid,50);

        //检测上一次链接和本次链接是否相同
        memset(addressMAC,0,sizeof(addressMAC));
        if(WIFI_get_curMAC(addressMAC) == 0)
        {
            if(strcmp(wifiScan[wifiList->currentRow()].addressMAC,addressMAC) != 0)
                changeFlag = 0x01;
        }

        if(wifiScan[wifiList->currentRow()].encryption[0]==0x31){
            WifiPswDialog dialog(curmodyWifi.essid,strength,this);
            int ret  = dialog.exec();
            if(ret==QDialog::Accepted){
                curmodyWifi.encryption[0] = 0x31;
                memcpy(curmodyWifi.psk,dialog.curPsw.toAscii().data(),8);
                setWifiInfo(&curmodyWifi);
            }
        }else{
            WifiEnsureDialog ensureDialog(curmodyWifi.essid,strength,this);
            int ret = ensureDialog.exec();
            if(ret == 1){
                 curmodyWifi.encryption[0]= 0x30;
                 setWifiInfo(&curmodyWifi);
            }
            else if(ret ==0){
                removeWifiInfo(wifiScan[wifiList->currentRow()].addressMAC);

            }

        }
    }


}



void WifiSetWin::connectSLot(QModelIndex idx)
{
    WIFIINFO curWifi;
    QString strength;
    char addressMAC[50];

    memset(&curWifi,0,sizeof(WIFIINFO));

    memcpy(curWifi.addressMAC ,wifiScan[idx.row()].addressMAC,50);
    memcpy(curWifi.essid,wifiScan[idx.row()].essid,50);


    switch(wifiScan[idx.row()].sigLevNum)
    {
    case 1:
        strength = "week";
        break;
    case 2:
        strength = "general";
        break;
    case 3:
        strength = "strong";
        break;
    case 4:
        strength = "strongest";
        break;
    }

    int rec = WIFI_check_list(&curWifi);//0 HAS PSW,
    _print("WIFI_check_list == %d\n",rec);


    if(rec==-2){
        showInforText("ERROR MAC,CHECK PLEASE!");
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
             setWifiInfo(&curWifi);
        }
        else if(ret ==0 )
            removeWifiInfo(wifiScan[idx.row()].addressMAC);

    }
    else if(rec ==-1){
        if(wifiScan[idx.row()].encryption[0]==0x31){

            WifiPswDialog dialog(curWifi.essid,strength,this);
            int ret  = dialog.exec();
            if(ret==QDialog::Accepted){
                curWifi.encryption[0] = 0x31;
                _print("dialog.curPsw.toAscii().data()=%s\n",dialog.curPsw.toAscii().data());
                strcpy(curWifi.psk,dialog.curPsw.toAscii().data());

                memset(addressMAC,0,sizeof(addressMAC));
                if(WIFI_get_curMAC(addressMAC) == 0)
                {
                    if(strcmp(curWifi.addressMAC,addressMAC) != 0)
                        changeFlag = 0x01;
                }

                setWifiInfo(&curWifi);
            }
        }else{
            WifiEnsureDialog ensureDialog(curWifi.essid,strength,this);
            int ret = ensureDialog.exec();
            if(ret == 1){
                 curWifi.encryption[0]= 0x30;
                 setWifiInfo(&curWifi);
            }
            else if(ret ==0)
                removeWifiInfo(wifiScan[idx.row()].addressMAC);

        }
    }
}

void WifiSetWin::refreshData()
{
    memset(wifiScan,0,sizeof(wifiScan));

   int rec =  WIFI_scan("wlan0",wifiScan,20);
   if(rec<0) {
      showInforText("NO WIFI ,TRY SCAN PLEASE!");
      QTimer::singleShot(2000,informationBox,SLOT(hide()));

       return;
   }

   _print("WIFI_scan ==%d\n",rec);

   char myMAC[59];
   memset(myMAC,0,59);

   int rec_mac = WIFI_get_curMAC(myMAC);

   int arraycount = rec;

   for(int i=0;i<wifiList->count();i++)
       wifiList->clear();

//   for(int i=0;i<arraycount;i++)
//      qDebug()<<"wifiScan[i].essid:"<<wifiScan[i].essid<<QString(wifiScan[i].sigLev).left(2).toInt();

   for(int i=0;i<arraycount;i++)
       for(int j=0;j<arraycount-i;j++)
           if(QString(wifiScan[j].sigLev).left(2).toInt()<QString(wifiScan[j+1].sigLev).left(2).toInt())
           {
                   WIFISCAN temp = wifiScan[j];
                   wifiScan[j] = wifiScan[j+1];
                   wifiScan[j+1] = temp;
            }



   for(int i=0;i<arraycount;i++){
       wifiList->addItem("");

       win[i] = new QWidget;
       win[i]->setFixedHeight(45);
       lay[i] = new QHBoxLayout;

       name[i] = new QLabel(tr(wifiScan[i].essid));
       pic[i] = new QLabel;
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


       if(!rec_mac)
       {
           _print("\n          myMAC =%s\n   wifiScan[i].addressMAC=%s\n\n",myMAC,wifiScan[i].addressMAC);

           if(!strcmp(myMAC,wifiScan[i].addressMAC)){
               curMacRow = i;
//               QString tempname = name[i]->text();
//               name[i]->setText(tempname+"\n connected");
               win[i]->setStyleSheet("QWidget{color:red;}");
           }
       }

       wifiList->setItemWidget(wifiList->item(i),win[i]);

       }
       wifiList->setCurrentRow(0);
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
//    refreshData();

}

void WifiSetWin::setWifiInfo(WIFIINFO *curWifi)
{
    _print("curWifi->psk=%s\n",curWifi->psk);
    int rec_set =  WIFI_set_info(curWifi);

    if(!rec_set){
//        showInforText("SAVE SUCCESS!");

        system("sync");

        MyThread *mythread = new MyThread;

        connect(mythread,SIGNAL(emitrecB(bool,int)),this,SLOT(slot_showrecB(bool,int)));
        mythread->start();

    }
    else{
        showInforText("SAVE FAILED!");
        QTimer::singleShot(1500,informationBox,SLOT(hide()));
    }

}


void WifiSetWin::removeWifiInfo(char *addressMAC)
{
    int rec_remove =  WIFI_remove_info(addressMAC);
    if(!rec_remove){
        showInforText("REMOVE SUCCESS!");
        QTimer::singleShot(3000,informationBox,SLOT(hide()));

       QListWidgetItem *item =  wifiList->takeItem(wifiList->currentRow());
       delete(item);
    }
    else{
        showInforText("REMOVE FAILED!");
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
