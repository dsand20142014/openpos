#include "oncardui.h"
#include "ui_oncardui.h"
#include "offtransui.h"
#include <QtDebug>

OnCardUI::OnCardUI(QWidget *parent) :
    QDialog(parent,Qt::CustomizeWindowHint),
    ui(new Ui::OnCardUI)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    ui->pushButton_8->setFocus();

}

OnCardUI::~OnCardUI()
{
    delete ui;
}

//void OnCardUI::on_pushButton_clicked()//�ѻ�����
//{

//}

void OnCardUI::on_pushButton_5_clicked()
{
    emit signal_oncard();
    destroy();
}

void OnCardUI::on_pushButton_8_clicked()
{
    qDebug()<<"off trans *************";
    OffTransUI *offTransUI = new OffTransUI(this);
    offTransUI->show();

}
