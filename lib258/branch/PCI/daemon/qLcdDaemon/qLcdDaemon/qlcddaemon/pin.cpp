#include "pin.h"
#include <QHBoxLayout>
#include <QTimer>

/*
 * time(ms): 10000~60000
 */
PinForm::PinForm(QString text, QWidget *p) :
    QWidget(p, Qt::FramelessWindowHint)
{   
    setAttribute(Qt::WA_DeleteOnClose);
    label = new QLabel();
    label->setStyleSheet("font: bold 20px;");

    if (!text.isEmpty()) {
        label->setText(text);
    } 

    line = new QLineEdit();
    line->setEchoMode(QLineEdit::Password);
    line->setMaxLength(90);
    line->setStyleSheet("font: 16px; border: 2px solid gray; border-radius: 4px;");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(10);
    layout->setContentsMargins(20, 30, 20, 30);
    layout->addWidget(label);
    layout->addWidget(new QLabel(tr("Please input password:")));
    layout->addWidget(line, 0, Qt::AlignTop);

    setLayout(layout);

    QLabel *pix = new QLabel(this);
    pix->setGeometry(0, 0, 240, 320-25);
    pix->setPixmap(QPixmap("image/pin.png"));
}

void PinForm::keyPressEvent(QKeyEvent *e)
{    
    switch(e->key())
    {
        case Qt::Key_Escape:
            //qDebug("PinForm::keyPressEvent: %p", e->key());
            hide();
            line->clear();
            emit getPin(QString("-"));
            break;

        case Qt::Key_Enter:
            hide();
            emit getPin(line->text());
            line->clear();
            break;

        default:
            break;
    }
    e->accept();
}

PinForm::~PinForm()
{
}
