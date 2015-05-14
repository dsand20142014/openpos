#include "moden.h"
#include "ui_moden.h"

Moden::Moden(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Moden)
{
    ui->setupUi(this);
    ui->pbEnter->setEnabled(false);
}

Moden::~Moden()
{
    delete ui;
}

void Moden::on_pbCancel_clicked()
{
    close();
}

void Moden::on_pbEnter_clicked()
{

}
