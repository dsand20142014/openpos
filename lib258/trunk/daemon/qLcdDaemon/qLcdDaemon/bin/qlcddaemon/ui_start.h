/********************************************************************************
** Form generated from reading UI file 'start.ui'
**
** Created: Tue Dec 4 10:36:28 2012
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_START_H
#define UI_START_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Start
{
public:
    QLabel *label;
    QPushButton *pushButton_1;
    QPushButton *pushButton_2;
    QPushButton *pushButton;

    void setupUi(QDialog *Start)
    {
        if (Start->objectName().isEmpty())
            Start->setObjectName(QString::fromUtf8("Start"));
        Start->setWindowModality(Qt::NonModal);
        Start->resize(240, 320);
        Start->setMinimumSize(QSize(240, 320));
        Start->setMaximumSize(QSize(240, 320));
        Start->setStyleSheet(QString::fromUtf8(""));
        label = new QLabel(Start);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(50, 10, 131, 51));
        label->setMaximumSize(QSize(320, 16777215));
        label->setStyleSheet(QString::fromUtf8(""));
        pushButton_1 = new QPushButton(Start);
        pushButton_1->setObjectName(QString::fromUtf8("pushButton_1"));
        pushButton_1->setGeometry(QRect(20, 110, 91, 41));
        pushButton_1->setMaximumSize(QSize(100, 16777215));
        pushButton_1->setContextMenuPolicy(Qt::DefaultContextMenu);
        pushButton_1->setAutoFillBackground(false);
        pushButton_1->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 100);"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/test.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_1->setIcon(icon);
        pushButton_2 = new QPushButton(Start);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(20, 160, 91, 41));
        pushButton_2->setMaximumSize(QSize(100, 16777215));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/setting.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_2->setIcon(icon1);
        pushButton = new QPushButton(Start);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(20, 230, 75, 27));
        pushButton->setMaximumSize(QSize(100, 16777215));
        pushButton->setStyleSheet(QString::fromUtf8(""));

        retranslateUi(Start);

        QMetaObject::connectSlotsByName(Start);
    } // setupUi

    void retranslateUi(QDialog *Start)
    {
        Start->setWindowTitle(QApplication::translate("Start", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Start", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#000000;\">\346\235\211\345\276\267POS\347\263\273\347\273\237\347\225\214\351\235\242</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        pushButton_1->setText(QApplication::translate("Start", "\346\265\213\350\257\225", 0, QApplication::UnicodeUTF8));
        pushButton_2->setText(QApplication::translate("Start", "\350\256\276\347\275\256", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("Start", "\345\205\263\346\234\272", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Start: public Ui_Start {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_START_H
