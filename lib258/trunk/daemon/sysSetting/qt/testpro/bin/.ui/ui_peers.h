/********************************************************************************
** Form generated from reading UI file 'peers.ui'
**
** Created: Thu Sep 27 15:36:39 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PEERS_H
#define UI_PEERS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QListView>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Peers
{
public:
    QListView *peers;
    QPushButton *closeButton;

    void setupUi(QDialog *Peers)
    {
        if (Peers->objectName().isEmpty())
            Peers->setObjectName(QString::fromUtf8("Peers"));
        Peers->resize(240, 320);
        peers = new QListView(Peers);
        peers->setObjectName(QString::fromUtf8("peers"));
        peers->setGeometry(QRect(10, 9, 221, 281));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(peers->sizePolicy().hasHeightForWidth());
        peers->setSizePolicy(sizePolicy);
        peers->setMouseTracking(true);
        peers->setEditTriggers(QAbstractItemView::NoEditTriggers);
        peers->setViewMode(QListView::IconMode);
        closeButton = new QPushButton(Peers);
        closeButton->setObjectName(QString::fromUtf8("closeButton"));
        closeButton->setGeometry(QRect(70, 291, 85, 27));

        retranslateUi(Peers);

        QMetaObject::connectSlotsByName(Peers);
    } // setupUi

    void retranslateUi(QDialog *Peers)
    {
        Peers->setWindowTitle(QApplication::translate("Peers", "Peers", 0, QApplication::UnicodeUTF8));
        closeButton->setText(QApplication::translate("Peers", "Close", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Peers: public Ui_Peers {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PEERS_H
