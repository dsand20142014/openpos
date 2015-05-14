#include "qclickablelabel.h"
#include "QDebug"

QClickableLabel::QClickableLabel(QWidget *parent) :
    QLabel(parent)
{
}

void QClickableLabel::mousePressEvent(QMouseEvent *)
{
    //emit clicked();
}

void QClickableLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    emit clicked();
}
