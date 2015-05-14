#include "informationtips.h"
#include "ui_informationtips.h"
#include <QTimer>

InformationTips::InformationTips(QWidget *parent) :
    QDialog(parent,Qt::CustomizeWindowHint),
    ui(new Ui::InformationTips)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QColor(117,117,255)));
    setPalette(palette);
    setAutoFillBackground(true);


    ui->pushButton->setFocus();
    ui->pushButton->hide();
    ui->pushButton->setEnabled(false);
}

InformationTips::~InformationTips()
{
    delete ui;
}

void InformationTips::setText_UI(QString text)
{
    ui->label->setText(text);

}


void InformationTips::setButton(bool flag,int ms)
{
    if(flag)
    {
        ui->pushButton->show();
        ui->pushButton->setEnabled(true);

    }
    else
    {
        ui->pushButton->hide();
        ui->pushButton->setEnabled(false);
        QTimer::singleShot(ms,this,SLOT(deleteLater()));


    }
}

void InformationTips::on_pushButton_clicked()
{
    destroy();
}
