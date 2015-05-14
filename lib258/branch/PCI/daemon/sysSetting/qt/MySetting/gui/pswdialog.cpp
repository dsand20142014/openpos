#include "pswdialog.h"
#include "ui_pswdialog.h"
#include <QDesktopWidget>

extern void _print(char *fmt,...);

PswDialog::PswDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PswDialog)
{
    ui->setupUi(this);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QColor(0,104,208)));
    setPalette(palette);
    setAutoFillBackground(true);


    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);

    int screenW = qApp->desktop()->screen(0)->width();
    int screenH = qApp->desktop()->screen(0)->height();

    this->setGeometry((screenW-width())/2,(screenH-height())/2,width(),height());

    ui->lineEdit->setFocus();

    ui->label_2->hide();

    connect(ui->lineEdit,SIGNAL(editingFinished()),this,SLOT(slotPsw()));

}

PswDialog::~PswDialog()
{
    delete ui;
}

void PswDialog::slotPsw()
{
    _print("text====%s\n",ui->lineEdit->text().toAscii().data());
    int rec = memcmp(ui->lineEdit->text().toAscii().data(),"111111",6);
    _print("rec=====%d\n",rec);
    if(!rec){
        done(1);
    }else{
        ui->label_2->show();
        ui->lineEdit->clear();
    }
}
