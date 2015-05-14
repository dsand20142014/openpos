#include "mainkey931.h"
#include "ui_mainkey931.h"
#include <QDebug>
#include <QString>
#include <QIcon>
#include <QCheckBox>
#include <QWindowsStyle>
#include <QtCore/qmath.h>
#include <QString>
#include <QStringList>
#include <stdio.h>
#include "xdata.h"

mainkey931::mainkey931(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey931)
{
    QString str,strTmp;
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    XDATA_Read_Vaild_File(DataSaveConstant );
    XDATA_Read_Vaild_File(DataSaveChange);
    ui->tabWidget->setCurrentWidget(ui->tab);
//    if(strlen((char*)&DataSave0.ConstantParamData.aucMerchantName))
    if(strlen((char*)DataSave0.ConstantParamData.aucTerminalID)!=0)
    {
        str="";
        str=str.fromAscii((char*)DataSave0.ConstantParamData.aucTerminalID);
        ui->lineEdit_9->setText(str);
    }
    if(strlen((char*)DataSave0.ConstantParamData.aucMerchantID)!=0)
    {
        str="";
        str=str.fromAscii((char*)DataSave0.ConstantParamData.aucMerchantID);
        str=str.left(15);
        ui->lineEdit_8->setText(str);
    }
    if(strlen((char*)DataSave0.ConstantParamData.aucMerchantName)!=0)
    {
        str="";
        str=str.fromAscii((char*)DataSave0.ConstantParamData.aucMerchantName);
        ui->lineEdit_7->setText(str);
    }
    str="";
    str=str.number(DataSave0.ChangeParamData.ulBatchNumber,10);
    ui->lineEdit_10->setText(str);
    str="";
    str=str.number(DataSave0.ChangeParamData.uiMaxTotalNb,10);
    ui->lineEdit_12->setText(str);
    str="";
    str=str.number(DataSave0.ChangeParamData.ulBatchNumber,10);
    ui->lineEdit_11->setText(str);
    switch(DataSave0.ConstantParamData.ucCommMode)
    {
        case 1:
            ui->radioButton_3->setChecked(true);
        break;
        case 2:
            ui->radioButton_4->setChecked(true);
            ui->stackedWidget->setCurrentWidget(ui->page_3);
        break;
        case 3:
            ui->radioButton_5->setChecked(true);
            ui->stackedWidget->setCurrentWidget(ui->page_2);
        break;
        case 4:
            ui->radioButton_6->setChecked(true);
        break;
        case 5:
            ui->radioButton_7->setChecked(true);
            ui->stackedWidget->setCurrentWidget(ui->page_2);
        break;
    default:
        break;
    }
    if(DataSave0.ConstantParamData.ucEncryptDataFlag==0x31)
        ui->checkBox_3->setChecked(true);
    else
        ui->checkBox_3->setChecked(false);
    str="";
    strTmp="";
    qDebug("2222222222222222222222222222222222222222222222222222222222");
    printf("%d",DataSave0.ConstantParamData.ulHostIP);
    qDebug("111111111111111111111111111111111111111111111111111111111111111");
    strTmp=strTmp.number(DataSave0.ConstantParamData.ulHostIP/(unsigned long int)pow(2,24));
    str=str.insert(0,strTmp);
    str=str.insert(0,".");
    strTmp=strTmp.number(DataSave0.ConstantParamData.ulHostIP%(unsigned long int)pow(2,24)/(unsigned long int)pow(2,16));
    str=str.insert(0,strTmp);
    str=str.insert(0,".");
    strTmp=strTmp.number(DataSave0.ConstantParamData.ulHostIP%(unsigned long int)pow(2,16)/(unsigned long int)pow(2,8));
    str=str.insert(0,strTmp);
    str=str.insert(0,".");
    strTmp=strTmp.number(DataSave0.ConstantParamData.ulHostIP%(unsigned long int)pow(2,8));
    str=str.insert(0,strTmp);


/*    ui->fontComboBox->removeItem(0);
    ui->fontComboBox->addItem(icon,str);
    ui->fontComboBox->setCurrentIndex(0);
*/
    ui->lineEdit_14->setText(str);
    ui->lineEdit_14->setInputMask("000.000.000.000");

    str="";
    strTmp="";

    strTmp=strTmp.number(DataSave0.ConstantParamData.ulBackIP/(unsigned long int)pow(2,24));
    str=str.insert(0,strTmp);
    str=str.insert(0,".");
    strTmp=strTmp.number(DataSave0.ConstantParamData.ulBackIP%(unsigned long int)pow(2,24)/(unsigned long int)pow(2,16));
    str=str.insert(0,strTmp);
    str=str.insert(0,".");
    strTmp=strTmp.number(DataSave0.ConstantParamData.ulBackIP%(unsigned long int)pow(2,16)/(unsigned long int)pow(2,8));
    str=str.insert(0,strTmp);
    str=str.insert(0,".");
    strTmp=strTmp.number(DataSave0.ConstantParamData.ulBackIP%(unsigned long int)pow(2,8));
    str=str.insert(0,strTmp);
/*
    ui->fontComboBox_2->removeItem(0);
    ui->fontComboBox_2->addItem(icon,str);
    ui->fontComboBox_2->setCurrentIndex(0);
*/
    ui->lineEdit_15->setText(str);
    ui->lineEdit_15->setInputMask("000.000.000.000");

//    QIcon icon("");
//    ui->comboBox->removeItem(0);
//    ui->comboBox->addItem(icon,"CMNET");
//    ui->comboBox->setCurrentIndex(0);
    str="";
    strTmp="";
    str=str.fromAscii((char*)DataSave0.ConstantParamData.aucAPN);
    str=str.left(5);
    ui->lineEdit_16->setText(str);



    str=str.number(DataSave0.ConstantParamData.uiHostPort,10);
    ui->lineEdit_5->setText(str);
    str=str.number(DataSave0.ConstantParamData.uiHostBackPort,10);
    ui->lineEdit_6->setText(str);
    if(DataSave0.ChangeParamData.bReaderSupport)
        ui->radioButton->setChecked(true);
    else
        ui->radioButton_2->setChecked(true);
    switch(DataSave0.ConstantParamData.ucTerminalType)
    {
        case 0x31:
        ui->radioButton_8->setChecked(true);
        break;
        case 0x32:
        ui->radioButton_9->setChecked(true);
        break;
        case 0x33:
        ui->radioButton_10->setChecked(true);
        break;
    }
    if(DataSave0.ConstantParamData.ucMobileFlag==0x31)
        ui->checkBox_2->setChecked(true);
    else
        ui->checkBox_2->setChecked(false);
    if(DataSave0.ConstantParamData.ucMobileFlag==0x31)
        ui->checkBox->setChecked(true);
    else
        ui->checkBox->setChecked(false);
    if(DataSave0.ConstantParamData.ucPrintCardNo==0x31)
        ui->checkBox_4->setChecked(true);
    else
        ui->checkBox_4->setChecked(false);
    if(DataSave0.ConstantParamData.ucDisAuth==0x31)
        ui->checkBox_5->setChecked(true);
    else
        ui->checkBox_5->setChecked(false);
    switch(DataSave0.ConstantParamData.ucCardtype)
    {
        case 0x30:
        ui->radioButton_11->setChecked(true);
        break;
        case 0x31:
        ui->radioButton_12->setChecked(true);
        break;
        case 0x32:
        ui->radioButton_13->setChecked(true);
        break;
    }
    //ui->showFullScreen();
//    QIcon icon;
//    ui->checkBox_3->setIcon(icon);
//    QWindowsStyle windowStyle;

//    ui->checkBox_3->setStyle(&windowStyle);
}

mainkey931::~mainkey931()
{
    delete ui;
}

void mainkey931::on_pushButton_clicked()
{
    destroy();
}

void mainkey931::on_pushButton_2_clicked()
{
    QStringList strList;
    unsigned char ipTmp[4];
    // 读取窗体数据 fuzhi

    //商户名称
    //商户号
    //终端号
//    memcpy(DataSave0.ConstantParamData.aucTerminalID,"33124040", 8);
    //    memcpy(DataSave0.ConstantParamData.aucMerchantID,"301310070118581", 15);
    //    memcpy(DataSave0.ConstantParamData.aucMerchantName,"杉德测试", 8);
    memcpy(DataSave0.ConstantParamData.aucTerminalID,ui->lineEdit_9->text().toAscii(), 8);
    memcpy(DataSave0.ConstantParamData.aucMerchantID,ui->lineEdit_8->text().toAscii(), 15);
    memcpy(DataSave0.ConstantParamData.aucMerchantName,ui->lineEdit_7->text().toAscii(), 8);
    //应用类别 PBOC :ReaderSupport = true;  EMV :ReaderSupport = false;
    if(ui->radioButton->isChecked())
    {
        DataSave0.ChangeParamData.bReaderSupport = true;
    }
    if(ui->radioButton_2->isChecked())
    {
        DataSave0.ChangeParamData.bReaderSupport = false;
    }
	DataSave0.ChangeParamData.ucReaderType =READER_SAND;

    //终端类型 TYPE_SHOP 0X31 TYPE_HOTEL	0X32 RESTAURANT	0x33
    if(ui->radioButton_8->isChecked())
    {
        DataSave0.ConstantParamData.ucTerminalType = 0X31;
    }
    if(ui->radioButton_9->isChecked())
    {
        DataSave0.ConstantParamData.ucTerminalType = 0X32;
    }
    if(ui->radioButton_10->isChecked())
    {
        DataSave0.ConstantParamData.ucTerminalType = 0X33;
    }
    //是否支持分期付款 1:是  0:否
    if(ui->checkBox_2->isChecked())
        DataSave0.ConstantParamData.ucInstallment =0X31;
    else
        DataSave0.ConstantParamData.ucInstallment =0X30;
    //是否支持手机无卡预约 1:是  0:否
    if(ui->checkBox->isChecked())
        DataSave0.ConstantParamData.ucMobileFlag = 0X31;
    else
        DataSave0.ConstantParamData.ucMobileFlag = 0X30;

    //打印屏蔽卡号  1:是  0:不是
    if(ui->checkBox_4->isChecked())
        DataSave0.ConstantParamData.ucPrintCardNo=0X31;
    else
        DataSave0.ConstantParamData.ucPrintCardNo=0X30;
    //预售权屏蔽卡号 1:是  0:不是
    if(ui->checkBox_5->isChecked())
        DataSave0.ConstantParamData.ucDisAuth = 0X31;
    else
        DataSave0.ConstantParamData.ucDisAuth = 0X30;
    //选择屏蔽卡种 0.内卡 1.外卡 2.内外卡
    if(ui->radioButton_11->isChecked())
        DataSave0.ConstantParamData.ucCardtype = 0X30;
    if(ui->radioButton_12->isChecked())
        DataSave0.ConstantParamData.ucCardtype = 0X31;
    if(ui->radioButton_13->isChecked())
        DataSave0.ConstantParamData.ucCardtype = 0X32;
    //通讯方式
    if(ui->radioButton_3->isChecked())
        DataSave0.ConstantParamData.ucCommMode=1;
    if(ui->radioButton_4->isChecked())
        DataSave0.ConstantParamData.ucCommMode=2;
    if(ui->radioButton_5->isChecked())
        DataSave0.ConstantParamData.ucCommMode=3;
    if(ui->radioButton_6->isChecked())
        DataSave0.ConstantParamData.ucCommMode=4;
    if(ui->radioButton_7->isChecked())
        DataSave0.ConstantParamData.ucCommMode=5;
    //IP
    //备份IP
    //	unsigned long	ulHostIP;
    //	unsigned long	ulBackIP;


    strList=ui->lineEdit_14->text().split(".");
    ipTmp[0]=strList.at(0).toInt(0,10);
    ipTmp[1]=strList.at(1).toInt(0,10);
    ipTmp[2]=strList.at(2).toInt(0,10);
    ipTmp[3]=strList.at(3).toInt(0,10);



    memcpy(&DataSave0.ConstantParamData.ulHostIP, ipTmp, 4);

    strList=ui->lineEdit_15->text().split(".");
    ipTmp[0]=strList.at(0).toInt(0,10);
    ipTmp[1]=strList.at(1).toInt(0,10);
    ipTmp[2]=strList.at(2).toInt(0,10);
    ipTmp[3]=strList.at(3).toInt(0,10);

    memcpy(&DataSave0.ConstantParamData.ulBackIP, ipTmp, 4);
    //	unsigned long   ulHostIPAddress1;
    //	unsigned long	ulHostIPAddress2;
    //	unsigned long	ulHostIPAddress3;
    //	unsigned long	ulHostIPGPRS1;
    //	unsigned long	ulHostIPGPRS2;
    //	unsigned long	ulHostIPGPRS3;
    //	unsigned long	ulHostIPBack1;
    //	unsigned long	ulHostIPBack2;
    //	unsigned long	ulHostIPBack3;
    //	unsigned long	ulHostIPMIS1;
    //	unsigned long	ulHostIPMIS2;
    //	unsigned long	ulHostIPMIS3;
//    memcpy(&DataSave0.ConstantParamData.ulHostIP, (unsigned char *)"\xDE\x42\x01\x86", 4);
//    memcpy(&DataSave0.ConstantParamData.ulBackIP, (unsigned char *)"\xDE\x42\x01\x86", 4);
    memcpy(&DataSave0.ConstantParamData.ulHostIPAddress1, (unsigned char *)"\xDE\x42\x01\x86", 4);
    memcpy(&DataSave0.ConstantParamData.ulHostIPAddress2, (unsigned char *)"\xDE\x42\x01\x86", 4);
    memcpy(&DataSave0.ConstantParamData.ulHostIPAddress3, (unsigned char *)"\xDE\x42\x01\x86", 4);
    //主机端口=asc_long(aucBuf,6);
//    DataSave0.ConstantParamData.uiHostPort=6001;
    DataSave0.ConstantParamData.uiHostPort=ui->lineEdit_5->text().toInt(0,10);
    //备份端口asc_long(aucBuf,6);
//    DataSave0.ConstantParamData.uiHostBackPort=6001;
    DataSave0.ConstantParamData.uiHostBackPort=ui->lineEdit_6->text().toInt(0,10);
    //APN
//    memcpy(DataSave0.ConstantParamData.aucAPN,"CMNET",5);
//    memcpy(DataSave0.ConstantParamData.aucAPN,ui->comboBox->currentText().toAscii(),5);
    memcpy(DataSave0.ConstantParamData.aucAPN,ui->lineEdit_16->text().toAscii(),5);
    //是否加密 0:否 1:是
    if(ui->checkBox_3->isChecked())
        DataSave0.ConstantParamData.ucEncryptDataFlag=0x31;
    else
        DataSave0.ConstantParamData.ucEncryptDataFlag=0x30;
    // 写文件

	printf("\n TermId:[%s]",DataSave0.ConstantParamData.aucTerminalID);

    XDATA_Write_Vaild_File(DataSaveConstant);


    //批次号
//    DataSave0.ChangeParamData.ulBatchNumber=0;
    DataSave0.ChangeParamData.ulBatchNumber=ui->lineEdit_11->text().toInt(0,10);
    //流水号
//    DataSave0.ChangeParamData.ulBatchNumber=0;
    DataSave0.ChangeParamData.ulBatchNumber=ui->lineEdit_10->text().toInt(0,10);
    //最大笔数
//    DataSave0.ChangeParamData.uiMaxTotalNb=300;
    DataSave0.ChangeParamData.uiMaxTotalNb=ui->lineEdit_12->text().toInt(0,10);

    // 打印机类型
    DataSave0.ConstantParamData.ucHeatPrintFormat=1;

    XDATA_Write_Vaild_File(DataSaveChange);




    UTIL_ClearGlobalData();
    UTIL_GetTerminalInfo();

printf("\nMainMenu TermId:[%s]",DataSave0.ConstantParamData.aucTerminalID);

destroy();
}



void mainkey931::on_checkBox_3_stateChanged(int arg1)
{
    if(arg1==Qt::Checked)
        qDebug("the checkbox is check");
    if(arg1==Qt::Unchecked)
        qDebug("the checkbox is uncheck");
}




void mainkey931::on_radioButton_5_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}



void mainkey931::on_radioButton_7_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}



void mainkey931::on_radioButton_4_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_3);
}

}
