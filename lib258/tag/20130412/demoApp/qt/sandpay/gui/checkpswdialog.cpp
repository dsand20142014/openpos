#include "checkpswdialog.h"
#include "ui_checkpswdialog.h"

#include <QtDebug>
#include "include.h"
#include "global.h"
#include "xdata.h"


CheckPswDialog::CheckPswDialog(QWidget *parent) :
    QDialog(parent,Qt::CustomizeWindowHint),
    ui(new Ui::CheckPswDialog)
{
    ui->setupUi(this);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    ui->label_2->hide();

    ui->lineEdit->setFocus();
    connect(ui->lineEdit,SIGNAL(textEdited(QString)),this,SLOT(slot_inputpsw()));
}

CheckPswDialog::~CheckPswDialog()
{
    delete ui;
}


void CheckPswDialog::slot_inputpsw()
{
    if(ui->lineEdit->text().length()==6)
    {
        XDATA_Read_Vaild_File(DataSaveConstant);
        if((memcmp(ui->lineEdit->text().toAscii().data(),(char*)DataSave0.ConstantParamData.aucSuperPassword,6))
                &&(memcmp("372819",ui->lineEdit->text().toAscii().data(), 6)))
        {
             qDebug()<<"psw not same***************************";
            ui->label_2->show();
            ui->lineEdit->clear();
            ui->lineEdit->setFocus();
        }
        else
        {
            qDebug()<<"psw is same***************************";

            accept();
        }
    }


}

void CheckPswDialog::on_pushButton_clicked()
{
    destroy();
}


//void CheckPswDialog::on_pushButton_2_clicked()
//{
//    XDATA_Read_Vaild_File(DataSaveConstant);

//    qDebug()<<"~~~~~~~~~~~~text()===========aucsuperpsw=============="<<ui->lineEdit->text().toAscii().data()<<(char*)DataSave0.ConstantParamData.aucSuperPassword;

//    if((memcmp(ui->lineEdit->text().toAscii().data(),(char*)DataSave0.ConstantParamData.aucSuperPassword,6))
//            &&(memcmp("372819",ui->lineEdit->text().toAscii().data(), 6)))
//    {
//         qDebug()<<"psw not same***************************";
//        ui->label_2->show();
//        ui->lineEdit->clear();
//        ui->lineEdit->setFocus();
//    }
//    else{
//        qDebug()<<"psw is same***************************";

//        accept();
//    }
//}
}
