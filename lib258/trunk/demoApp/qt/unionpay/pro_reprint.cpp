#include "pro_reprint.h"
#include "ui_pro_reprint.h"
#include "msginfo.h"

#include "mythread.h"
#include "include.h"
#include "global.h"
#include "toolslib.h"

Pro_Reprint::Pro_Reprint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Pro_Reprint)
{
    ui->setupUi(this);
       setWindowFlags(Qt::FramelessWindowHint);

}

Pro_Reprint::~Pro_Reprint()
{
    delete ui;
}

void Pro_Reprint::on_pushButton_clicked()
{
    destroy();
}

void Pro_Reprint::destroyWindow(int)
{
//    a->terminate();
    qDebug("5555555555555555555555555555555555555555555");
    memset((uchar*)&ProUiFace.OweDrv,0x00,sizeof(ProUiFace.OweDrv));
    memset((uchar*)&ProUiFace.ProToUi,0x00,sizeof(ProUiFace.ProToUi));
    memset((uchar*)&ProUiFace.UiToPro,0x00,sizeof(ProUiFace.UiToPro));
    destroy();
}

void Pro_Reprint::on_pushButton_2_clicked()
{
	int uiLen=0;

    uiLen = strlen(ui->line1_trace->text().toAscii());
    memset(ProUiFace.UiToPro.aucOldTrace,0,7);
    memcpy(ProUiFace.UiToPro.aucOldTrace,ui->line1_trace->text().toAscii(),uiLen);
    ProUiFace.UiToPro.ucTransType = TRANS_PRINT_BYTRACE;
//    MyThread *a=new MyThread();
//    a->start();

    MsgInfo *desk=new MsgInfo();
    connect(desk,SIGNAL(returnGrandFartherMenu(int)),this,SLOT(destroyWindow(int)));
//    desk->show();
//    desk->showFullScreen();
    desk->exec();
}
}
