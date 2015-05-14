#include "pro_53_offcancel.h"
#include "ui_pro_53_offcancel.h"

Pro_53_OffCancel::Pro_53_OffCancel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Pro_53_OffCancel)
{
    ui->setupUi(this);
	    setWindowFlags(Qt::FramelessWindowHint);
}

Pro_53_OffCancel::~Pro_53_OffCancel()
{
    delete ui;
}

void Pro_53_OffCancel::on_cancelButton_clicked()
{
    destroy();
}
