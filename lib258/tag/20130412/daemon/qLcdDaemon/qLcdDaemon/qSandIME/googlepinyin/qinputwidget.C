#include "qinputwidget.h"
#include "ui_qinputwidget.h"
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QWSInputMethod>
#include <QSignalMapper>

InputWidget::InputWidget(IMFrame* im)
    :QWidget(0, Qt::Tool| Qt::WindowStaysOnTopHint| Qt::FramelessWindowHint),
      ui(new Ui::InputWidget)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::NoFocus);
    connect(ui->pushButton,SIGNAL(clicked()),im,SLOT(changeImeType()));
    connect(im,SIGNAL(imeTypeChanged(ImeType)),this,SLOT(showImeType(ImeType)));
}

InputWidget::~InputWidget()
{
    delete ui;
}

void InputWidget::showImeType(ImeType t)
{
    switch(t)
    {
    case NUM:
        ui->pushButton->setText(tr("12"));
        break;
    case ABC:
        ui->pushButton->setText(tr("ab"));
        break;
    case CAPABC:
        ui->pushButton->setText(tr("AB"));
        break;
    case PINYIN:
        ui->pushButton->setText(tr("拼"));
        break;
    default:
        break;
    }
}
