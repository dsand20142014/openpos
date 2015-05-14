#include "retypebytrace.h"
#include "ui_retypebytrace.h"
#include <QtDebug>
#include "include.h"
#include "global.h"

ReTypeByTrace::ReTypeByTrace(QWidget *parent) :
    QDialog(parent,Qt::CustomizeWindowHint),
    ui(new Ui::ReTypeByTrace)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose);


    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);


    ui->label_2->hide();
    ui->lineEdit->setFocus();

   // connect(ui->lineEdit,SIGNAL(textEdited(QString)),this,SLOT(slots_online()));
}

ReTypeByTrace::~ReTypeByTrace()
{
    delete ui;
}





void ReTypeByTrace::on_pushButton_clicked()
{
    destroy();
}

void ReTypeByTrace::on_pushButton_2_clicked()
{
    if(ui->lineEdit->text().length() == 6)
    {
        ProUiFace.UiToPro.ulTraceNum =  ui->lineEdit->text().toLong();//000100
        qDebug()<<"ulTraceNum============lineEdit->text()============"<< ProUiFace.UiToPro.ulTraceNum <<ui->lineEdit->text().toLong();
        PRT_ReprintbyTrace();
        destroy();
    }
    else
    {
        ui->label_2->show();
        ui->lineEdit->clear();
        ui->lineEdit->setFocus();
    }
}


}

