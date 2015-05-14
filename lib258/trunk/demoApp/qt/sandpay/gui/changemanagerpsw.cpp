#include "changemanagerpsw.h"
#include "ui_changemanagerpsw.h"
#include <QtDebug>
#include <QTimer>
#include "include.h"
#include "global.h"
#include "xdata.h"
ChangeManagerPsw::ChangeManagerPsw(QWidget *parent) :
    QDialog(parent,Qt::CustomizeWindowHint),
    ui(new Ui::ChangeManagerPsw)
{
    ui->setupUi(this);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    qDebug()<<"In changemanagerpsw******************************";
    ui->label_2->hide();
    ui->lineEdit->setFocus();

    connect(ui->lineEdit ,SIGNAL(textChanged(QString)),this, SLOT(slot_finished1()) );
    connect(ui->lineEdit_2 ,SIGNAL(textChanged(QString)),this, SLOT(slot_finished2()) );

}

void ChangeManagerPsw::slot_finished1()
{
    qDebug()<<"ui->lineEdit->text()"<<ui->lineEdit->text();

    if(ui->lineEdit->text().length()==6)
         ui->lineEdit_2->setFocus();


}

void ChangeManagerPsw::slot_finished2()
{
     if(ui->lineEdit_2->text().length()==6)
     {
           if((ui->lineEdit->text()==ui->lineEdit_2->text()))
           {
                ui->label_2->setText(tr("密码修改成功"));
                ui->label_2->show();

                Os__saved_copy((unsigned char *)ui->lineEdit->text().toAscii().data(),DataSave0.ConstantParamData.aucSuperPassword,6);
                XDATA_Write_Vaild_File(DataSaveConstant);

                QTimer::singleShot(2000,this,SLOT(deleteLater()));
           }
           else
           {
               ui->label_2->show();
               ui->lineEdit->clear();
               ui->lineEdit_2->clear();
               ui->lineEdit->setFocus();
           }

       }
//    else
//    {
//        ui->label_2->setText(tr("密码需要六位！请重新输入"));
//        ui->label_2->show();
//        ui->lineEdit->clear();
//        ui->lineEdit_2->clear();
//    }





}


ChangeManagerPsw::~ChangeManagerPsw()
{
    delete ui;
}

void ChangeManagerPsw::on_pushButton_clicked()
{
    destroy();
}
}
