#include "mainkey4.h"
#include "ui_mainkey4.h"
#include "msginfoui.h"
#include "informationtips.h"
#include <QMessageBox>
#include "querytraceui.h"
#include <QtDebug>
#include "include.h"
#include "global.h"
#include "xdata.h"
MainKey4::MainKey4(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainKey4)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    ui->pushButton->setFocus();
}

MainKey4::~MainKey4()
{
    delete ui;
}

void MainKey4::on_pushButton_clicked()
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
        PRT_Detail();

    }

}

void MainKey4::on_pushButton_2_clicked()
{

    XDATA_Read_Vaild_File(DataSaveTransInfo);
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
        QueryTraceUI *queryUI = new QueryTraceUI(this);
        queryUI->showFullScreen();
    }

    //CFG_QueryList();
}

void MainKey4::on_pushButton_3_clicked()
{
    if(memcmp(DataSave0.ChangeParamData.aucBatchNumber,
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",12)==0)
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
        PRT_TerminalTt();

    }

}

void MainKey4::on_pushButton_4_clicked()
{
    destroy();
}
}
