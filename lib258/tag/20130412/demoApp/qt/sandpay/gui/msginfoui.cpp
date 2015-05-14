#include "msginfoui.h"
#include "ui_msginfoui.h"

#include <QDebug>
#include "include.h"
#include "global.h"

MsgInfoUI::MsgInfoUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MsgInfoUI)
{
    //setAttribute(Qt::WA_DeleteOnClose);
    qDebug()<<"IN msg infor*********";
    ui->setupUi(this);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    update();

    connect(&sandtimer,SIGNAL(timeout()),this,SLOT(slottimer1Done()));
    sandtimer.start(200);
}

MsgInfoUI::~MsgInfoUI()
{
    sandtimer.stop();
    delete ui;
}


void MsgInfoUI::slottimer1Done()
{

    if((ProUiFace.ProToUi.uiLines >= 1)&&(strlen((char*)ProUiFace.ProToUi.aucLine1))){

        ui->label_2->setText(QString((char*)ProUiFace.ProToUi.aucLine1));
        ui->label_2->update();

        qDebug()<<"1111111111ui->MsgRst->toPlainText()~~~~~~~~~~~~~~~~~~~~~~"<<ui->label_2->text();
    }

    if((ProUiFace.ProToUi.uiLines >= 2)&&(strlen((char*)ProUiFace.ProToUi.aucLine2))){
        ui->label_3->setText(QString((char*)ProUiFace.ProToUi.aucLine2));

    }

    if((ProUiFace.ProToUi.uiLines >= 3)&&(strlen((char*)ProUiFace.ProToUi.aucLine3))){

        ui->label_4->setText(QString((char*)ProUiFace.ProToUi.aucLine3));
    }

    if((ProUiFace.ProToUi.uiLines >= 4)&&(strlen((char*)ProUiFace.ProToUi.aucLine4))){
        ui->label_5->setText(QString((char*)ProUiFace.ProToUi.aucLine4));

    }

    ProUiFace.ProToUi.uiLines=0;

}


void MsgInfoUI::on_pushButton_clicked()
{
    destroy();
}
}
