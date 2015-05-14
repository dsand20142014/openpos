/********************************************************************************
** Form generated from reading UI file 'syswindow.ui'
**
** Created: Tue Dec 4 10:36:28 2012
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SYSWINDOW_H
#define UI_SYSWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SysWindow
{
public:
    QWidget *centralwidget;
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *pushButton_enter;
    QPushButton *pushButton_reset;
    QLabel *label_2;
    QPushButton *pushButton_setting;
    QPushButton *pushButton_test;
    QPushButton *pushButton_download;
    QPushButton *pushButton_cancel;

    void setupUi(QMainWindow *SysWindow)
    {
        if (SysWindow->objectName().isEmpty())
            SysWindow->setObjectName(QString::fromUtf8("SysWindow"));
        SysWindow->resize(240, 320);
        SysWindow->setAutoFillBackground(false);
        SysWindow->setIconSize(QSize(60, 60));
        centralwidget = new QWidget(SysWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 110, 81, 21));
        label->setLayoutDirection(Qt::LeftToRight);
        label->setStyleSheet(QString::fromUtf8("background-color: rgba(213, 213, 213, 100);\n"
"color: rgb(177, 177, 177);\n"
"font: 12pt \"Andale Mono\";\n"
"background-color: rgba(255, 255, 255, 50);"));
        lineEdit = new QLineEdit(centralwidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(110, 110, 111, 21));
        lineEdit->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 50);"));
        lineEdit->setEchoMode(QLineEdit::Password);
        pushButton_enter = new QPushButton(centralwidget);
        pushButton_enter->setObjectName(QString::fromUtf8("pushButton_enter"));
        pushButton_enter->setGeometry(QRect(20, 150, 91, 27));
        pushButton_enter->setStyleSheet(QString::fromUtf8("font: 12pt \"Andale Mono\";\n"
"color: rgb(98, 98, 98);"));
        pushButton_reset = new QPushButton(centralwidget);
        pushButton_reset->setObjectName(QString::fromUtf8("pushButton_reset"));
        pushButton_reset->setGeometry(QRect(130, 150, 91, 27));
        pushButton_reset->setStyleSheet(QString::fromUtf8("font: 12pt \"Andale Mono\";\n"
"color: rgb(98, 98, 98);"));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 200, 201, 21));
        label_2->setLayoutDirection(Qt::LeftToRight);
        label_2->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 50);\n"
"font: 12pt \"Andale Mono\";\n"
"color: rgb(177, 177, 177);"));
        label_2->setScaledContents(false);
        pushButton_setting = new QPushButton(centralwidget);
        pushButton_setting->setObjectName(QString::fromUtf8("pushButton_setting"));
        pushButton_setting->setGeometry(QRect(10, 260, 45, 45));
        pushButton_setting->setLayoutDirection(Qt::LeftToRight);
        pushButton_setting->setAutoFillBackground(false);
        pushButton_setting->setStyleSheet(QString::fromUtf8("background-image: url(:/images/setting.png);\n"
"background-color: rgba(85, 85, 127, 100);\n"
"font: 16pt \"Andale Mono\";\n"
"color: rgb(177, 177, 177);"));
        pushButton_test = new QPushButton(centralwidget);
        pushButton_test->setObjectName(QString::fromUtf8("pushButton_test"));
        pushButton_test->setGeometry(QRect(70, 260, 45, 45));
        pushButton_test->setAutoFillBackground(false);
        pushButton_test->setStyleSheet(QString::fromUtf8("background-image: url(:/images/test.png);\n"
"background-color: rgba(85, 85, 127, 100);\n"
"font: 16pt \"Andale Mono\";\n"
"color: rgb(177, 177, 177);"));
        pushButton_download = new QPushButton(centralwidget);
        pushButton_download->setObjectName(QString::fromUtf8("pushButton_download"));
        pushButton_download->setGeometry(QRect(130, 260, 45, 45));
        pushButton_download->setAutoFillBackground(false);
        pushButton_download->setStyleSheet(QString::fromUtf8("background-image: url(:/images/download.png);\n"
"background-color: rgba(85, 85, 127, 100);\n"
"font: 16pt \"Andale Mono\";\n"
"color: rgb(177, 177, 177);"));
        pushButton_cancel = new QPushButton(centralwidget);
        pushButton_cancel->setObjectName(QString::fromUtf8("pushButton_cancel"));
        pushButton_cancel->setGeometry(QRect(190, 260, 45, 45));
        pushButton_cancel->setAutoFillBackground(false);
        pushButton_cancel->setStyleSheet(QString::fromUtf8("background-image: url(:/images/exit.png);\n"
"background-color: rgba(85, 85, 127, 100);\n"
"font: 16pt \"Andale Mono\";\n"
"color: rgb(177, 177, 177);"));
        SysWindow->setCentralWidget(centralwidget);

        retranslateUi(SysWindow);

        QMetaObject::connectSlotsByName(SysWindow);
    } // setupUi

    void retranslateUi(QMainWindow *SysWindow)
    {
        SysWindow->setWindowTitle(QApplication::translate("SysWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("SysWindow", "\350\257\267\350\276\223\345\205\245\345\257\206\347\240\201\357\274\232", 0, QApplication::UnicodeUTF8));
        pushButton_enter->setText(QApplication::translate("SysWindow", "\347\241\256\345\256\232", 0, QApplication::UnicodeUTF8));
        pushButton_reset->setText(QApplication::translate("SysWindow", "\351\207\215\347\275\256", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("SysWindow", "\350\257\267\350\276\223\345\205\245\345\257\206\347\240\201\350\277\233\345\205\245\344\270\213\345\210\227\350\217\234\345\215\225!", 0, QApplication::UnicodeUTF8));
        pushButton_setting->setText(QApplication::translate("SysWindow", "\350\256\276\347\275\256", 0, QApplication::UnicodeUTF8));
        pushButton_test->setText(QApplication::translate("SysWindow", "\346\265\213\350\257\225", 0, QApplication::UnicodeUTF8));
        pushButton_download->setText(QApplication::translate("SysWindow", "\344\270\213\350\275\275", 0, QApplication::UnicodeUTF8));
        pushButton_cancel->setText(QApplication::translate("SysWindow", "\351\200\200\345\207\272", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SysWindow: public Ui_SysWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SYSWINDOW_H
