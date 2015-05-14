#include "mainkey51.h"
#include "ui_mainkey51.h"
#include <QtDebug>
#include "include.h"
#include "global.h"
#include "xdata.h"

MainKey51::MainKey51(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainKey51)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    ui->pushButton->setFocus();

    ui->lineEdit_6->setInputMask("000.000.000.000");

    setInitValue();
}

void MainKey51::setInitValue()
{
    XDATA_Read_Vaild_File(DataSaveConstant);

    Uart_Printf("\nucAPN:%s\n",DataSave0.ConstantParamData.ucAPN);
    Uart_Printf("\nTID:%s\n",DataSave0.ConstantParamData.aucTerminalID);
    Uart_Printf("MID:%s\n",DataSave0.ConstantParamData.aucMerchantID);

    ui->lineEdit_14->setText(QString((char *)DataSave0.ConstantParamData.aucMerchantID));//???
    ui->lineEdit_12->setText(QString((char *)DataSave0.ConstantParamData.aucTerminalID));//???

    if(DataSave0.ConstantParamData.ucYTJFlag==0)
        ui->radioButton_14->setChecked(true);
    else
        ui->radioButton_15->setChecked(true);

    qDebug()<<"0000000000ulHostIPAdd%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<&DataSave0.ConstantParamData.ulHostIPAdd;

    if(DataSave0.ConstantParamData.ucCommMode==2)
    {
        ui->radioButton_9->setChecked(true);
        ui->lineEdit_3->setText(QString((char *)DataSave0.ConstantParamData.aucInitTelNumber1));
        ui->lineEdit_4->setText(QString((char *)DataSave0.ConstantParamData.aucInitTelNumber2));
        ui->lineEdit_9->setText(QString((char *)DataSave0.ConstantParamData.aucSwitchBoard));
    }
    else if(DataSave0.ConstantParamData.ucCommMode==5)
    {
        unsigned char aucIPV4Addr[4];
        memcpy(aucIPV4Addr,&DataSave0.ConstantParamData.ulHostIPAdd,4);
        //sprintf((char *)aucDisp,"%3d.%3d.%3d.%3d",aucIPV4Addr[0],aucIPV4Addr[1],aucIPV4Addr[2],aucIPV4Addr[3]);
        ui->radioButton_10->setChecked(true);
        qDebug()<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%aucIPV4Addr%"<<aucIPV4Addr[0]<<aucIPV4Addr[1]<<aucIPV4Addr[2]<<aucIPV4Addr[3];
        qDebug()<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%aucIPV4Addr%"<<aucIPV4Addr[1];
        qDebug()<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%aucIPV4Addr%"<<aucIPV4Addr[2];
        qDebug()<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%aucIPV4Addr%"<<aucIPV4Addr[3];

        QString str;
        str.sprintf("%3d.%3d.%3d.%3d",aucIPV4Addr[0],aucIPV4Addr[1],aucIPV4Addr[2],aucIPV4Addr[3]);
        ui->lineEdit_6->setText(str);
        ui->lineEdit_7->setText(QString::number(DataSave0.ConstantParamData.uiHostPort));
    }
    else if(DataSave0.ConstantParamData.ucCommMode==3)
    {
        ui->radioButton_11->setChecked(true);

        qDebug()<<"IN GPRS*********************";
        unsigned char aucIPV4Addr[4];
        memcpy(aucIPV4Addr,&DataSave0.ConstantParamData.ulHostIPAdd,4);
        qDebug()<<"%%%%%%%%%%aucIPV4Addr%"<<aucIPV4Addr[0]<<aucIPV4Addr[1]<<aucIPV4Addr[2]<<aucIPV4Addr[3];
        qDebug()<<"%%%%%%%%%%%aucIPV4Addr%"<<aucIPV4Addr[1];
        qDebug()<<"%%%%%%%%%%aucIPV4Addr%"<<aucIPV4Addr[2];
        qDebug()<<"%%%%%%%%%aucIPV4Addr%"<<aucIPV4Addr[3];
        QString str;
        str.sprintf("%3d.%3d.%3d.%3d",aucIPV4Addr[0],aucIPV4Addr[1],aucIPV4Addr[2],aucIPV4Addr[3]);
        ui->lineEdit_6->setText(str);

        ui->lineEdit_7->setText(QString::number(DataSave0.ConstantParamData.uiHostPort));

        ui->lineEdit_8->setText(QString((char *)DataSave0.ConstantParamData.ucAPN));
    }
    else if(DataSave0.ConstantParamData.ucCommMode==1)
        ui->radioButton_12->setChecked(true);
    else
        ui->radioButton_13->setChecked(true);

}

MainKey51::~MainKey51()
{
    delete ui;
}

void MainKey51::on_pushButton_clicked()
{
    qDebug()<<"exit on mainkey51*************";
    destroy();
}

void MainKey51::on_pushButton_2_clicked()
{

    unsigned char aucIp[6];

    Uart_Printf("jianggs - test data.\n");

    memcpy(DataSave0.ConstantParamData.aucTerminalID,"00000001",8);
    memcpy(DataSave0.ConstantParamData.aucMerchantID, "888002199990001",15);


    aucIp[0] =172;//116;
    aucIp[1] =16;//236;
    aucIp[2] =10;//224;
    aucIp[3] =235;//54;
    memcpy((unsigned char *)&DataSave0.ConstantParamData.ulHostIPAdd,aucIp, 4);//主机IP
    DataSave0.ConstantParamData.uiHostPort = 10001;
   // memcpy(DataSave0.ConstantParamData.ucAPN, "CMNET" ,5 );

    DataSave0.ConstantParamData.ucCommunication	=0x32;
    DataSave0.ConstantParamData.ucCommMode =5;

    Os__saved_set(&DataSave0.ConstantParamData.prnflag,1,1);
    Os__saved_set(&DataSave0.ConstantParamData.prnflagS,0,1);
    Os__saved_set(&DataSave0.ConstantParamData.prnflagL,0,1);
    Os__saved_set(&DataSave0.ConstantParamData.linenum,0x16,1);
    Os__saved_set(&DataSave0.ConstantParamData.font,0x1E,1);
    Os__saved_set(&DataSave0.ConstantParamData.printtimes,2,1);
    Os__saved_set(&Printer_flag,1,1);


    XDATA_Write_Vaild_File(DataSaveConstant);

   destroy();


#if 0


    qDebug()<<"enter on mainkey51**************88";
    unsigned char ucResult=0;
    unsigned char  aucIp[6];
    unsigned char  aucIpgps[5];

    memcpy(DataSave0.ConstantParamData.aucTerminalID,ui->lineEdit_12->text().toAscii().data(),8 );//��//�ն˺�
    memcpy(DataSave0.ConstantParamData.aucMerchantID, ui->lineEdit_14->text().toAscii().data(),15);//�̻���

    if(ui->radioButton_14->isChecked())
        DataSave0.ConstantParamData.ucYTJFlag = 0;
    else if(ui->radioButton_15->isChecked())
        DataSave0.ConstantParamData.ucYTJFlag = 1;

    Os__saved_set(&DataSave0.ConstantParamData.prnflag,1,1);
    Os__saved_set(&DataSave0.ConstantParamData.prnflagS,0,1);
    Os__saved_set(&DataSave0.ConstantParamData.prnflagL,0,1);
    Os__saved_set(&DataSave0.ConstantParamData.linenum,0x16,1);
    Os__saved_set(&DataSave0.ConstantParamData.font,0x1E,1);
    Os__saved_set(&DataSave0.ConstantParamData.printtimes,2,1);
    Os__saved_set(&Printer_flag,1,1);

    if(ui->radioButton_9->isChecked())
    {
        Commun_flag=0x31;
        DataSave0.ConstantParamData.ucCommMode =2;
        memcpy((char*)DataSave0.ConstantParamData.aucInitTelNumber1, ui->lineEdit_3->text().toAscii().data(),ui->lineEdit_3->text().length());//�̻���
        memcpy((char*)DataSave0.ConstantParamData.aucInitTelNumber2, ui->lineEdit_4->text().toAscii().data(),ui->lineEdit_4->text().length());//�̻���
        memcpy((char*)DataSave0.ConstantParamData.aucSwitchBoard, ui->lineEdit_9->text().toAscii().data(),20);//�̻���

    }
    else if(ui->radioButton_10->isChecked())
    {
        Commun_flag=0x32;
        DataSave0.ConstantParamData.ucCommMode =5;

        QStringList strList = ui->lineEdit_6->text().split(".");
        qDebug()<<"STRLIST================="<<strList;
        if(strList.count())
        {
            aucIp[0] = strList.at(0).toInt();//172
            aucIp[1] = strList.at(1).toInt();//16
            aucIp[2] = strList.at(2).toInt();//10
            aucIp[3] = strList.at(3).toInt();//235
            qDebug()<<"auc1  auc 2 "<<aucIp[0]<<aucIp[1]<<aucIp[2]<<aucIp[3];
        }

        memcpy((unsigned char *)&DataSave0.ConstantParamData.ulHostIPAdd,aucIp, 4);

        qDebug()<<"10 rad ulHostIPAdd================="<<(char *)DataSave0.ConstantParamData.ulHostIPAdd;

        DataSave0.ConstantParamData.uiHostPort = ui->lineEdit_7->text().toInt();
    }
    else if(ui->radioButton_11->isChecked())
    {
        Commun_flag=0x33;
        DataSave0.ConstantParamData.ucCommMode =3;
        qDebug()<<"IN radioButton_11 CHECKED******************* ";

        QStringList strList = ui->lineEdit_6->text().split(".");
        qDebug()<<"STRLIST================="<<strList;
        if(strList.count())
        {
            aucIpgps[0] = strList.at(0).toInt();//116;
            aucIpgps[1] = strList.at(1).toInt();//236
            aucIpgps[2] = strList.at(2).toInt();//224
            aucIpgps[3] = strList.at(3).toInt();//54
        }

        memcpy((unsigned char *)&DataSave0.ConstantParamData.ulHostIPAdd,aucIpgps, 4);

        qDebug()<<"ulHostIPAdd****************aucIpgps** "<<&DataSave0.ConstantParamData.ulHostIPAdd<<aucIpgps;

        DataSave0.ConstantParamData.uiHostPort =  ui->lineEdit_7->text().toInt();

        qDebug()<<"uiHostPort========"<< DataSave0.ConstantParamData.uiHostPort;

        //memcpy(DataSave0.ConstantParamData.ucAPN, ui->lineEdit_8->text().toAscii().data(),  20 );

        memcpy(DataSave0.ConstantParamData.ucAPN, "CMNET" ,5 );

    }
    else if(ui->radioButton_12->isChecked())
    {
        Commun_flag=0x34;
        DataSave0.ConstantParamData.ucCommMode =1;
    }
    else if(ui->radioButton_13->isChecked())
    {
        Commun_flag=0x35;
        DataSave0.ConstantParamData.ucCommMode =4;
    }

    qDebug()<<"ulHostIPAdd================="<<(char *)DataSave0.ConstantParamData.ulHostIPAdd;

    unsigned char ret = XDATA_Write_Vaild_File(DataSaveConstant);
    Uart_Printf(" *******************XDATA_Write_Vaild_File(DataSaveConstant)====%d\n\n",ret );



    destroy();
#endif


     }
}
