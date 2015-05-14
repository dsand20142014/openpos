#include "keyclearsetting.h"

extern "C"{
#include "encrypt.h"
#include "osdriver.h"
#include "sand_modules.h"

}

//DRV_IN  DrvIn1;
//DRV_OUT  DrvOut1;
//DRIVER Drv1;

KeyClearSetting::KeyClearSetting(QWidget *parent) :
    BaseForm("KEY CLEAR SETTING",parent)
{
    btnAuto =  new Button(" Dynamic Key",9);
    btnAuto->setFixedSize(160,40);
    btnAuto->setObjectName("keyBtn");

    labelA = new QLabel;
    labelA->setPixmap(QPixmap("/daemon/image/icon/keyA.png"));

    QTextEdit *textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(10);
    vbox->addWidget(labelA);
    vbox->addWidget(btnAuto);
    vbox->setContentsMargins(30,30,30,10);
    vbox->addStretch(1);

    textEdit->setLayout(vbox);

    layout->addWidget(textEdit);

    size = 0;

    btnExit->setFocus();

    QSettings set("/daemon/conf.ini",QSettings::IniFormat);
    childGroups =  set.childGroups();

    qDebug()<<"childGroups==="<<childGroups;

    for(int i=0;i<childGroups.size();i++)
    {
        set.beginGroup(childGroups.at(i));
        int tempid = set.value("ID").toInt();
        qDebug()<<"tempid==="<<tempid;
        if(tempid>=10){
            id[size] = tempid;
            qDebug()<<"id[size]   size:"<<id[size]<<size;
            size++;
        }

        set.endGroup();
    }

    qDebug()<<"111size========"<<size;

    connect(btnAuto,SIGNAL(clicked()),this,SLOT(slotAuto()));
    connect(btnOk,SIGNAL(clicked()),this,SLOT(slotAuto()));

}

KeyClearSetting::~KeyClearSetting()
{

}

void KeyClearSetting::slotAuto()
{
    QMessageBox box(QMessageBox::Information,tr("  title "),tr("   ARE YOU SURE TO CLEAR?   "));
    box.setWindowFlags(Qt::CustomizeWindowHint);

    QPushButton *okBtn = new QPushButton(tr("Yes"));
    okBtn->setFixedWidth(80);
    okBtn->setFixedHeight(25);
    box.addButton(okBtn,QMessageBox::YesRole);

    QPushButton *noBtn = new QPushButton(tr("No"));
    noBtn->setFixedWidth(80);
    noBtn->setFixedHeight(25);
    box.addButton(noBtn,QMessageBox::NoRole);

    box.setDefaultButton(noBtn);

    box.exec();
    if (box.clickedButton() == noBtn)
        return;

     KEY_APPID_DATA keyData;
     memset(&keyData,0x00,sizeof(KEY_APPID_DATA));

     qDebug()<<"size========"<<size;

     keyData.appnumber = size;
     for(int i=0;i<size;i++)
         keyData.appid[i] = id[i];

     keyData.delflag = KEY_DEL_PART;

     KEY_KeyCheckApp(&keyData);

     showInforText("CLEAR SUCCESS!",true,15,185);

     QTimer::singleShot(1500,informationBox,SLOT(hide()));

}

