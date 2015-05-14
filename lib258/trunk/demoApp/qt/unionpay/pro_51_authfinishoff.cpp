#include "pro_51_authfinishoff.h"
#include "ui_pro_51_authfinishoff.h"

Pro_51_AuthFinishOff::Pro_51_AuthFinishOff(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Pro_51_AuthFinishOff)
{
    ui->setupUi(this);
	    setWindowFlags(Qt::FramelessWindowHint);
}

Pro_51_AuthFinishOff::~Pro_51_AuthFinishOff()
{
    delete ui;
}

void Pro_51_AuthFinishOff::on_cancelButton_clicked()
{
    destroy();
}
