#include "mainkey6.h"
#include "ui_mainkey6.h"

MainKey6::MainKey6(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainKey6)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

}

MainKey6::~MainKey6()
{
    delete ui;
}

void MainKey6::on_pushButton_clicked()
{
    destroy();
}
