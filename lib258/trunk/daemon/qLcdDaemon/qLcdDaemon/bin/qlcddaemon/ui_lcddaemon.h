/********************************************************************************
** Form generated from reading UI file 'lcddaemon.ui'
**
** Created: Tue Dec 4 10:36:28 2012
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LCDDAEMON_H
#define UI_LCDDAEMON_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QScrollArea>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LcdDaemon
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QLabel *label_gprs;
    QLabel *label_signal;
    QLabel *label_modem;
    QLabel *label_camera;
    QLabel *label_usb;
    QLabel *label_sd;
    QLabel *label_connect;
    QLabel *label_power;
    QLabel *label_time;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout;

    void setupUi(QMainWindow *LcdDaemon)
    {
        if (LcdDaemon->objectName().isEmpty())
            LcdDaemon->setObjectName(QString::fromUtf8("LcdDaemon"));
        LcdDaemon->resize(240, 320);
        LcdDaemon->setStyleSheet(QString::fromUtf8("background-color: rgb(221, 233, 255);"));
        centralWidget = new QWidget(LcdDaemon);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        frame = new QFrame(centralWidget);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setStyleSheet(QString::fromUtf8("background-color: qlineargradient(spread:reflect, x1:1, y1:1, x2:1, y2:0.0113636, stop:0.0590909 rgba(75, 132, 255, 240), stop:1 rgba(19, 33, 65, 207));"));
        frame->setFrameShape(QFrame::StyledPanel);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(6, 0, 6, 0);
        label_gprs = new QLabel(frame);
        label_gprs->setObjectName(QString::fromUtf8("label_gprs"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_gprs->sizePolicy().hasHeightForWidth());
        label_gprs->setSizePolicy(sizePolicy);
        label_gprs->setMaximumSize(QSize(21, 21));
        label_gprs->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label_gprs);

        label_signal = new QLabel(frame);
        label_signal->setObjectName(QString::fromUtf8("label_signal"));
        sizePolicy.setHeightForWidth(label_signal->sizePolicy().hasHeightForWidth());
        label_signal->setSizePolicy(sizePolicy);
        label_signal->setMaximumSize(QSize(21, 21));
        label_signal->setAutoFillBackground(false);
        label_signal->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));
        label_signal->setWordWrap(false);

        horizontalLayout->addWidget(label_signal);

        label_modem = new QLabel(frame);
        label_modem->setObjectName(QString::fromUtf8("label_modem"));
        sizePolicy.setHeightForWidth(label_modem->sizePolicy().hasHeightForWidth());
        label_modem->setSizePolicy(sizePolicy);
        label_modem->setMaximumSize(QSize(21, 21));
        label_modem->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label_modem);

        label_camera = new QLabel(frame);
        label_camera->setObjectName(QString::fromUtf8("label_camera"));
        sizePolicy.setHeightForWidth(label_camera->sizePolicy().hasHeightForWidth());
        label_camera->setSizePolicy(sizePolicy);
        label_camera->setMaximumSize(QSize(21, 21));
        label_camera->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label_camera);

        label_usb = new QLabel(frame);
        label_usb->setObjectName(QString::fromUtf8("label_usb"));
        sizePolicy.setHeightForWidth(label_usb->sizePolicy().hasHeightForWidth());
        label_usb->setSizePolicy(sizePolicy);
        label_usb->setMaximumSize(QSize(21, 21));
        label_usb->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label_usb);

        label_sd = new QLabel(frame);
        label_sd->setObjectName(QString::fromUtf8("label_sd"));
        sizePolicy.setHeightForWidth(label_sd->sizePolicy().hasHeightForWidth());
        label_sd->setSizePolicy(sizePolicy);
        label_sd->setMaximumSize(QSize(21, 21));
        label_sd->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label_sd);

        label_connect = new QLabel(frame);
        label_connect->setObjectName(QString::fromUtf8("label_connect"));
        sizePolicy.setHeightForWidth(label_connect->sizePolicy().hasHeightForWidth());
        label_connect->setSizePolicy(sizePolicy);
        label_connect->setMaximumSize(QSize(21, 21));
        label_connect->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label_connect);

        label_power = new QLabel(frame);
        label_power->setObjectName(QString::fromUtf8("label_power"));
        sizePolicy.setHeightForWidth(label_power->sizePolicy().hasHeightForWidth());
        label_power->setSizePolicy(sizePolicy);
        label_power->setMaximumSize(QSize(42, 21));
        label_power->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        horizontalLayout->addWidget(label_power);

        label_time = new QLabel(frame);
        label_time->setObjectName(QString::fromUtf8("label_time"));
        label_time->setMaximumSize(QSize(42, 21));
        label_time->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);\n"
"font: 10pt \"Andale Mono\";"));

        horizontalLayout->addWidget(label_time);


        verticalLayout_2->addWidget(frame);

        scrollArea = new QScrollArea(centralWidget);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 236, 296));
        verticalLayout = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout_2->addWidget(scrollArea);

        LcdDaemon->setCentralWidget(centralWidget);

        retranslateUi(LcdDaemon);

        QMetaObject::connectSlotsByName(LcdDaemon);
    } // setupUi

    void retranslateUi(QMainWindow *LcdDaemon)
    {
        LcdDaemon->setWindowTitle(QApplication::translate("LcdDaemon", "LcdDaemon", 0, QApplication::UnicodeUTF8));
        label_gprs->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class LcdDaemon: public Ui_LcdDaemon {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LCDDAEMON_H
