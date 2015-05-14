#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTextBrowser>
#include <QLineEdit>

#include "setdialog.h"

SetDialog::SetDialog(QWidget *parent) :
    QDialog(parent)
{
    //displayDialog();
    //show();
}

void SetDialog::displayDialog()
{
    lTitle = new QLabel(this);
    lTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    tbNote = new QTextBrowser(this);
    leValue = new QLineEdit(this);
    cancelButton = new QPushButton("Cancel",this);
    enterButton = new QPushButton("Enter",this);

    QVBoxLayout *hLayout = new QVBoxLayout(this);
    hLayout->addWidget(lTitle);
    hLayout->addWidget(tbNote);
    hLayout->addWidget(leValue);

    QHBoxLayout *vLayout = new QHBoxLayout(this);
    vLayout->addWidget(cancelButton);
    vLayout->addWidget(enterButton);

    hLayout->addLayout(vLayout);

    setLayout(hLayout);
    setGeometry(0,0,240,320);

    lTitle->setText(slTitle);
//    for(int i=0; i<slNote.size(); i++)
//    {
//        tbNote->append();
//    }
    tbNote->setText(slNote);
    leValue->setText(sValue);

    show();
}

void SetDialog::setTitle(QString title)
{
    slTitle = title;
}

void SetDialog::setNote(QString note)
{
    slNote = note;
}

void SetDialog::setValue(QString value)
{
    sValue = value;
}
