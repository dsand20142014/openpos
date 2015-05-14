#include "mainkey3.h"
#include "ui_mainkey3.h"
#include "retypebytrace.h"
#include "informationtips.h"
#include <QMessageBox>
#include <QtDebug>
#include "msginfoui.h"
#include "include.h"
#include "global.h"
#include <xdata.h>

MainKey3::MainKey3(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainKey3)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    ui->pushButton->setFocus();

}

MainKey3::~MainKey3()
{
    delete ui;
}

void MainKey3::on_pushButton_clicked()
{
    if(DataSave0.TransInfoData.TransTotal.uiTotalNb==0)
    {
        InformationTips *info = new InformationTips(this);
        info->setGeometry(10,100,220,100);
        info->setText_UI(QString(tr("空交易")));
        info->setButton(true);
        info->exec();

    }
    else
    {
        InformationTips *info = new InformationTips(this);
        info->setGeometry(10,100,220,100);
        info->setText_UI(QString(tr("正在打印")));
        info->setButton(true);
        info->show();

        PRT_Reprint();
    }
}

void MainKey3::on_pushButton_2_clicked()
{
    if(DataSave0.TransInfoData.TransTotal.uiTotalNb==0)
    {
        Uart_Printf("***********PRT_ReprintbyTrace******空交易***********\n");

        InformationTips *info = new InformationTips(this);
        info->setGeometry(10,100,220,100);
        info->setText_UI(QString(tr("空交易")));
        info->setButton(true);
        info->exec();

    }
    else
    {
        ReTypeByTrace *typeByTrace = new ReTypeByTrace(this);
        typeByTrace->exec();

    }

}

void MainKey3::on_pushButton_3_clicked()
{
    if(memcmp(DataSave0.TransInfoData.TransTotal.aucBatchNumber,
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",12)==0)
    {
        InformationTips *info = new InformationTips(this);
        info->setGeometry(10,100,220,100);
        info->setText_UI(QString(tr("无结算单!")));
        info->setButton(true,2000);
        info->exec();

    }
    else
    {
        InformationTips *info = new InformationTips(this);
        info->setGeometry(10,100,220,100);
        info->setText_UI(QString(tr("正在打印")));
        info->setButton(true);
        info->show();

        PRT_ReprintSettle();


    }

}

void MainKey3::on_pushButton_4_clicked()
{
    destroy();
}

}
