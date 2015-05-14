/*
 * Common class
 */
#include "common.h"

messageBox::messageBox(const QString & text, QWidget *parent) :
    QMessageBox(parent)
{
    QPushButton *btnOk = new QPushButton(tr("Ok"));
    btnOk->setMinimumSize(70, 30);

    addButton(btnOk, QMessageBox::YesRole);
    setIcon(QMessageBox::Information);
    setWindowFlags(Qt::CustomizeWindowHint);
    setText(text);
}

messageBox::~messageBox()
{

}
