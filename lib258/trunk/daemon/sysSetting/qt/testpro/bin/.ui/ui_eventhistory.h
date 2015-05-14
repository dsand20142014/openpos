/********************************************************************************
** Form generated from reading UI file 'eventhistory.ui'
**
** Created: Thu Sep 27 15:36:39 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EVENTHISTORY_H
#define UI_EVENTHISTORY_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QTreeView>
#include "gui/wpamsg.h"

QT_BEGIN_NAMESPACE

class Ui_EventHistory
{
public:
    QTreeView *eventListView;
    QPushButton *closeButton;

    void setupUi(QDialog *EventHistory)
    {
        if (EventHistory->objectName().isEmpty())
            EventHistory->setObjectName(QString::fromUtf8("EventHistory"));
        EventHistory->resize(240, 320);
        eventListView = new QTreeView(EventHistory);
        eventListView->setObjectName(QString::fromUtf8("eventListView"));
        eventListView->setGeometry(QRect(10, 10, 221, 271));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(eventListView->sizePolicy().hasHeightForWidth());
        eventListView->setSizePolicy(sizePolicy);
        eventListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        eventListView->setSelectionMode(QAbstractItemView::NoSelection);
        closeButton = new QPushButton(EventHistory);
        closeButton->setObjectName(QString::fromUtf8("closeButton"));
        closeButton->setGeometry(QRect(70, 289, 85, 27));

        retranslateUi(EventHistory);

        QMetaObject::connectSlotsByName(EventHistory);
    } // setupUi

    void retranslateUi(QDialog *EventHistory)
    {
        EventHistory->setWindowTitle(QApplication::translate("EventHistory", "Event history", 0, QApplication::UnicodeUTF8));
        closeButton->setText(QApplication::translate("EventHistory", "Close", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class EventHistory: public Ui_EventHistory {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EVENTHISTORY_H
