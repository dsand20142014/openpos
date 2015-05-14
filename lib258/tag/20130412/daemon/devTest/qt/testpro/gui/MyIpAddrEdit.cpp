// MyIpAddrEdit.cpp
#include "MyIpAddrEdit.h"
#include <QRegExpValidator>
#include <QLabel>
#include "MyIpPartLineEdit.h"
#include <QDebug>

MyIpAddrEdit::MyIpAddrEdit(QWidget* pParent )
: QFrame(pParent)
{
    setObjectName("MyIpAddrEdit");

    ip_part1_ = new MyIpPartLineEdit(this);
    ip_part2_ = new MyIpPartLineEdit(this);
    ip_part3_ = new MyIpPartLineEdit(this);
    ip_part4_ = new MyIpPartLineEdit(this);

    labeldot1_ = new QLabel(this);
    labeldot2_ = new QLabel(this);
    labeldot3_ = new QLabel(this);

    ip_part1_->setGeometry(QRect(2, 2, 35, 26));
    ip_part2_->setGeometry(QRect(30, 2, 35, 26));
    ip_part3_->setGeometry(QRect(60, 2, 35, 26));
    ip_part4_->setGeometry(QRect(90, 2, 35, 26));

    labeldot1_->setText(" .");
    labeldot1_->setGeometry(QRect(27, 8, 6, 18));
    labeldot1_->setAlignment(Qt::AlignCenter);

    labeldot2_->setText(" .");
    labeldot2_->setGeometry(QRect(57, 8, 6, 18));
    labeldot2_->setAlignment(Qt::AlignCenter);

    labeldot3_->setText(" .");
    labeldot3_->setGeometry(QRect(87, 8, 6, 18));
    labeldot3_->setAlignment(Qt::AlignCenter);

//    QWidget::setTabOrder(ip_part1_, ip_part2_);
//    QWidget::setTabOrder(ip_part2_, ip_part3_);
//    QWidget::setTabOrder(ip_part3_, ip_part4_);


    ip_part1_->set_nexttab_edit(ip_part2_);
    ip_part2_->set_nexttab_edit(ip_part3_);
    ip_part3_->set_nexttab_edit(ip_part4_);

    connect(ip_part1_, SIGNAL(textChanged(const QString&)), this, SLOT(textchangedslot(const QString&)));
    connect(ip_part2_, SIGNAL(textChanged(const QString&)), this, SLOT(textchangedslot(const QString&)));
    connect(ip_part3_, SIGNAL(textChanged(const QString&)), this, SLOT(textchangedslot(const QString&)));
    connect(ip_part4_, SIGNAL(textChanged(const QString&)), this, SLOT(textchangedslot(const QString&)));

    connect(ip_part1_, SIGNAL(textEdited (const QString&)), this, SLOT(texteditedslot(const QString&)));
    connect(ip_part2_, SIGNAL(textEdited (const QString&)), this, SLOT(texteditedslot(const QString&)));
    connect(ip_part3_, SIGNAL(textEdited (const QString&)), this, SLOT(texteditedslot(const QString&)));
    connect(ip_part4_, SIGNAL(textEdited (const QString&)), this, SLOT(texteditedslot(const QString&)));

  //  connect(ip_part1_, SIGNAL(editingFinished()), ip_part2_, SLOT(setFocus()));
   // connect(ip_part2_, SIGNAL(editingFinished()), ip_part3_, SLOT(setFocus()));
   // connect(ip_part3_, SIGNAL(editingFinished()), ip_part4_, SLOT(setFocus()));
    //connect(ip_part4_, SIGNAL(editingFinished()), this, SIGNAL(endEdit()));

    
}


bool MyIpAddrEdit::ip1Hasfocus()
{
    return ip_part1_->hasFocus();
}

bool MyIpAddrEdit::ip2Hasfocus()
{
    return ip_part2_->hasFocus();

}
bool MyIpAddrEdit::ip3Hasfocus()
{
    return ip_part3_->hasFocus();

}
bool MyIpAddrEdit::ip4Hasfocus()
{
    return ip_part4_->hasFocus();

}

void MyIpAddrEdit::setip2focus()
{
    ip_part2_->setFocus();
}


void MyIpAddrEdit::setip3focus()
{
    ip_part3_->setFocus();
}



void MyIpAddrEdit::setip4focus()
{
  ip_part4_->setFocus();
}


MyIpAddrEdit::~MyIpAddrEdit()
{

}



void MyIpAddrEdit::textchangedslot(const QString& /*text*/)
{
    QString ippart1, ippart2, ippart3, ippart4;
    ippart1 = ip_part1_->text();
    ippart2 = ip_part2_->text();
    ippart3 = ip_part3_->text();
    ippart4 = ip_part4_->text();

    QString ipaddr = QString("%1.%2.%3.%4")
                     .arg(ippart1)
                     .arg(ippart2)
                     .arg(ippart3)
                     .arg(ippart4);

    emit textchanged(ipaddr);
}

void MyIpAddrEdit::texteditedslot(const QString &/*text*/)
{
    QString ippart1, ippart2, ippart3, ippart4;
    ippart1 = ip_part1_->text();
    ippart2 = ip_part2_->text();
    ippart3 = ip_part3_->text();
    ippart4 = ip_part4_->text();

    QString ipaddr = QString("%1.%2.%3.%4")
        .arg(ippart1)
        .arg(ippart2)
        .arg(ippart3)
        .arg(ippart4);

    emit textedited(ipaddr);
}

void MyIpAddrEdit::settext(const QString &text)
{
    QString ippart1, ippart2, ippart3, ippart4;
    QString qstring_validate = text;

    // IPµØÖ·ÑéÖ¤
    QRegExp regexp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    QRegExpValidator regexp_validator(regexp, this);
    int nPos = 0;
    QValidator::State state = regexp_validator.validate(qstring_validate, nPos);
    // IPºÏ·¨
    if (state == QValidator::Acceptable)
    {
        QStringList ippartlist = text.split(".");
    
        int strcount = ippartlist.size();
        int index = 0;
        if (index < strcount)
        {
            ippart1 = ippartlist.at(index);
        }
        if (++index < strcount)
        {
            ippart2 = ippartlist.at(index);
        }
        if (++index < strcount)
        {
            ippart3 = ippartlist.at(index);
        }
        if (++index < strcount)
        {
            ippart4 = ippartlist.at(index);
        }
    }

    ip_part1_->setText(ippart1);
    ip_part2_->setText(ippart2);
    ip_part3_->setText(ippart3);
    ip_part4_->setText(ippart4);
}

QString MyIpAddrEdit::text()
{
    QString ippart1, ippart2, ippart3, ippart4;
    ippart1 = ip_part1_->text();
    ippart2 = ip_part2_->text();
    ippart3 = ip_part3_->text();
    ippart4 = ip_part4_->text();

    return QString("%1.%2.%3.%4")
        .arg(ippart1)
        .arg(ippart2)
        .arg(ippart3)
        .arg(ippart4);
}
#if 0
void MyIpAddrEdit::setStyleSheet(const QString &styleSheet)
{
    ip_part1_->setStyleSheet(styleSheet);
    ip_part2_->setStyleSheet(styleSheet);
    ip_part3_->setStyleSheet(styleSheet);
    ip_part4_->setStyleSheet(styleSheet);
}
#endif
