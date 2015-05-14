/********************************************************************************
** Form generated from reading UI file 'qinputwidget.ui'
**
** Created: Tue Dec 4 10:36:17 2012
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QINPUTWIDGET_H
#define UI_QINPUTWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InputWidget
{
public:
    QVBoxLayout *verticalLayout;
    QPushButton *pushButton;

    void setupUi(QWidget *InputWidget)
    {
        if (InputWidget->objectName().isEmpty())
            InputWidget->setObjectName(QString::fromUtf8("InputWidget"));
        InputWidget->resize(22, 22);
        InputWidget->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 253, 220,0);"));
        verticalLayout = new QVBoxLayout(InputWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        pushButton = new QPushButton(InputWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setMaximumSize(QSize(22, 22));
        pushButton->setStyleSheet(QString::fromUtf8("background-color: rgba(75, 132, 255, 200);\n"
"color: rgb(255, 255, 255);"));

        verticalLayout->addWidget(pushButton);


        retranslateUi(InputWidget);

        QMetaObject::connectSlotsByName(InputWidget);
    } // setupUi

    void retranslateUi(QWidget *InputWidget)
    {
        InputWidget->setWindowTitle(QApplication::translate("InputWidget", "Form", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("InputWidget", " \346\225\260", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class InputWidget: public Ui_InputWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QINPUTWIDGET_H
