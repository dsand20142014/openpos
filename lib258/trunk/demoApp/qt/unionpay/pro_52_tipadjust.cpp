#include "pro_52_tipadjust.h"
#include "ui_pro_52_tipadjust.h"

Pro_52_TipAdjust::Pro_52_TipAdjust(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Pro_52_TipAdjust)
{
    ui->setupUi(this);
	    setWindowFlags(Qt::FramelessWindowHint);
}

Pro_52_TipAdjust::~Pro_52_TipAdjust()
{
    delete ui;
}

void Pro_52_TipAdjust::on_cancelButton_clicked()
{
    destroy();
}
