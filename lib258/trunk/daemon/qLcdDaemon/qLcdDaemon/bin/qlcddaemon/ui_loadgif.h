/********************************************************************************
** Form generated from reading UI file 'loadgif.ui'
**
** Created: Tue Dec 4 10:36:28 2012
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOADGIF_H
#define UI_LOADGIF_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoadGif
{
public:
    QLabel *label_gif;

    void setupUi(QWidget *LoadGif)
    {
        if (LoadGif->objectName().isEmpty())
            LoadGif->setObjectName(QString::fromUtf8("LoadGif"));
        LoadGif->resize(240, 320);
        LoadGif->setMinimumSize(QSize(240, 320));
        LoadGif->setMaximumSize(QSize(240, 320));
        LoadGif->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 100);"));
        label_gif = new QLabel(LoadGif);
        label_gif->setObjectName(QString::fromUtf8("label_gif"));
        label_gif->setGeometry(QRect(0, 0, 124, 124));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_gif->sizePolicy().hasHeightForWidth());
        label_gif->setSizePolicy(sizePolicy);
        label_gif->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));

        retranslateUi(LoadGif);

        QMetaObject::connectSlotsByName(LoadGif);
    } // setupUi

    void retranslateUi(QWidget *LoadGif)
    {
        LoadGif->setWindowTitle(QApplication::translate("LoadGif", "Form", 0, QApplication::UnicodeUTF8));
        label_gif->setText(QApplication::translate("LoadGif", "TextLabel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class LoadGif: public Ui_LoadGif {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOADGIF_H
