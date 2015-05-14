/********************************************************************************
** Form generated from reading UI file 'topform.ui'
**
** Created: Tue Dec 4 10:36:28 2012
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TOPFORM_H
#define UI_TOPFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TopForm
{
public:
    QLabel *lb_gprs;
    QLabel *lb_signal;
    QLabel *lb_modem;
    QLabel *lb_camera;
    QLabel *lb_usb;
    QLabel *lb_sd;
    QLabel *lb_connect;
    QLabel *lb_power;
    QLabel *lb_time;

    void setupUi(QWidget *TopForm)
    {
        if (TopForm->objectName().isEmpty())
            TopForm->setObjectName(QString::fromUtf8("TopForm"));
        TopForm->resize(240, 25);
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(TopForm->sizePolicy().hasHeightForWidth());
        TopForm->setSizePolicy(sizePolicy);
        TopForm->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 80);"));
        lb_gprs = new QLabel(TopForm);
        lb_gprs->setObjectName(QString::fromUtf8("lb_gprs"));
        lb_gprs->setGeometry(QRect(8, 2, 21, 16));
        sizePolicy.setHeightForWidth(lb_gprs->sizePolicy().hasHeightForWidth());
        lb_gprs->setSizePolicy(sizePolicy);
        lb_gprs->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));
        lb_signal = new QLabel(TopForm);
        lb_signal->setObjectName(QString::fromUtf8("lb_signal"));
        lb_signal->setGeometry(QRect(29, 2, 21, 16));
        lb_signal->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));
        lb_modem = new QLabel(TopForm);
        lb_modem->setObjectName(QString::fromUtf8("lb_modem"));
        lb_modem->setGeometry(QRect(50, 2, 21, 16));
        lb_modem->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));
        lb_camera = new QLabel(TopForm);
        lb_camera->setObjectName(QString::fromUtf8("lb_camera"));
        lb_camera->setGeometry(QRect(71, 2, 21, 16));
        lb_camera->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));
        lb_usb = new QLabel(TopForm);
        lb_usb->setObjectName(QString::fromUtf8("lb_usb"));
        lb_usb->setGeometry(QRect(92, 2, 21, 16));
        lb_usb->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));
        lb_sd = new QLabel(TopForm);
        lb_sd->setObjectName(QString::fromUtf8("lb_sd"));
        lb_sd->setGeometry(QRect(113, 2, 21, 16));
        lb_sd->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));
        lb_connect = new QLabel(TopForm);
        lb_connect->setObjectName(QString::fromUtf8("lb_connect"));
        lb_connect->setGeometry(QRect(134, 2, 21, 16));
        lb_connect->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));
        lb_power = new QLabel(TopForm);
        lb_power->setObjectName(QString::fromUtf8("lb_power"));
        lb_power->setGeometry(QRect(155, 2, 39, 16));
        lb_power->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));
        lb_time = new QLabel(TopForm);
        lb_time->setObjectName(QString::fromUtf8("lb_time"));
        lb_time->setGeometry(QRect(194, 2, 38, 16));
        lb_time->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        retranslateUi(TopForm);

        QMetaObject::connectSlotsByName(TopForm);
    } // setupUi

    void retranslateUi(QWidget *TopForm)
    {
        TopForm->setWindowTitle(QApplication::translate("TopForm", "Form", 0, QApplication::UnicodeUTF8));
        lb_gprs->setText(QString());
        lb_signal->setText(QString());
        lb_modem->setText(QString());
        lb_camera->setText(QString());
        lb_usb->setText(QString());
        lb_sd->setText(QString());
        lb_connect->setText(QString());
        lb_power->setText(QString());
        lb_time->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class TopForm: public Ui_TopForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TOPFORM_H
