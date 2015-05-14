/********************************************************************************
** Form generated from reading UI file 'pinyindialog.ui'
**
** Created: Tue Dec 4 10:36:17 2012
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PINYINDIALOG_H
#define UI_PINYINDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QVBoxLayout>
#include "qclickablelabel.h"

QT_BEGIN_NAMESPACE

class Ui_PinyinDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *line_2;
    QLineEdit *lineEdit;
    QFrame *line;
    QHBoxLayout *horizontalLayout;
    QClickableLabel *label;
    QFrame *line_3;
    QClickableLabel *label_2;
    QFrame *line_4;
    QClickableLabel *label_3;
    QFrame *line_5;
    QClickableLabel *label_4;
    QFrame *line_6;
    QClickableLabel *label_5;

    void setupUi(QDialog *PinyinDialog)
    {
        if (PinyinDialog->objectName().isEmpty())
            PinyinDialog->setObjectName(QString::fromUtf8("PinyinDialog"));
        PinyinDialog->resize(282, 51);
        PinyinDialog->setStyleSheet(QString::fromUtf8("background-color: rgba(124, 166, 255, 180);"));
        verticalLayout = new QVBoxLayout(PinyinDialog);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        line_2 = new QFrame(PinyinDialog);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_2);

        lineEdit = new QLineEdit(PinyinDialog);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        verticalLayout->addWidget(lineEdit);

        line = new QFrame(PinyinDialog);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(3, 0, 3, 0);
        label = new QClickableLabel(PinyinDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label);

        line_3 = new QFrame(PinyinDialog);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::VLine);
        line_3->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_3);

        label_2 = new QClickableLabel(PinyinDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label_2);

        line_4 = new QFrame(PinyinDialog);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setFrameShape(QFrame::VLine);
        line_4->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_4);

        label_3 = new QClickableLabel(PinyinDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label_3);

        line_5 = new QFrame(PinyinDialog);
        line_5->setObjectName(QString::fromUtf8("line_5"));
        line_5->setFrameShape(QFrame::VLine);
        line_5->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_5);

        label_4 = new QClickableLabel(PinyinDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label_4);

        line_6 = new QFrame(PinyinDialog);
        line_6->setObjectName(QString::fromUtf8("line_6"));
        line_6->setFrameShape(QFrame::VLine);
        line_6->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_6);

        label_5 = new QClickableLabel(PinyinDialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label_5);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(PinyinDialog);

        QMetaObject::connectSlotsByName(PinyinDialog);
    } // setupUi

    void retranslateUi(QDialog *PinyinDialog)
    {
        PinyinDialog->setWindowTitle(QApplication::translate("PinyinDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QString());
        label_2->setText(QString());
        label_3->setText(QString());
        label_4->setText(QString());
        label_5->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class PinyinDialog: public Ui_PinyinDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PINYINDIALOG_H
