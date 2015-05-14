#include "macwin.h"
//#include "ui_macwin.h"

MacWin::MacWin(QWidget *parent) :
    BaseForm(tr("MAC WINDOW"),parent)
{
    btnOk->setText(tr("Burning"));
    btnOk->show();

    QTextEdit *textEdit = new QTextEdit;
    textEdit->setReadOnly(true);

    label1 = new QLabel(tr("Cur Mac:"));
    label2 = new QLabel();

    QVBoxLayout *vlay = new QVBoxLayout();

    vlay->addWidget(label1);
    vlay->addWidget(label2);
    vlay->setContentsMargins(5,5,5,20);
    vlay->addSpacerItem(new QSpacerItem(10,60));

    textEdit->setLayout(vlay);


    layout->addWidget(textEdit);

    connect(btnOk,SIGNAL(clicked()),SLOT(slotOK()));

}

MacWin::~MacWin()
{
}

void MacWin::slotOK()
{

}
