#include "mainkey92.h"
#include "ui_mainkey92.h"
#include "xdata.h"
mainkey92::mainkey92(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainkey92)
{
    QString str;
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
        //ui->showFullScreen();
    XDATA_Read_Vaild_File(DataSaveConstant );
    if(strlen((char*)DataSave0.ConstantParamData.aucMerchantName)!=0)
    {
        str="";
        str=str.fromAscii((char*)DataSave0.ConstantParamData.aucMerchantName);
        ui->lineEdit->setText(str);
    }
}

mainkey92::~mainkey92()
{
    delete ui;
}

void mainkey92::on_pushButton_8_clicked()
{
    destroy();
}

void mainkey92::on_pushButton_9_clicked()
{
    memcpy(DataSave0.ConstantParamData.aucMerchantName,ui->lineEdit->text().toAscii(), 8);
    destroy();
}

}
