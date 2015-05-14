/********************************************************************************
** Form generated from reading UI file 'wpagui.ui'
**
** Created: Thu Sep 27 15:36:38 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WPAGUI_H
#define UI_WPAGUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QTabWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>
#include "gui/eventhistory.h"
#include "gui/peers.h"
#include "gui/scanresults.h"
#include "gui/wpamsg.h"

QT_BEGIN_NAMESPACE

class Ui_WpaGui
{
public:
    QAction *fileEventHistoryAction;
    QAction *fileSaveConfigAction;
    QAction *fileExitAction;
    QAction *networkAddAction;
    QAction *networkEditAction;
    QAction *networkRemoveAction;
    QAction *networkEnableAllAction;
    QAction *networkDisableAllAction;
    QAction *networkRemoveAllAction;
    QAction *helpContentsAction;
    QAction *helpIndexAction;
    QAction *helpAboutAction;
    QAction *actionWPS;
    QAction *actionPeers;
    QWidget *widget;
    QLabel *adapterLabel;
    QComboBox *adapterSelect;
    QLabel *networkLabel;
    QComboBox *networkSelect;
    QTabWidget *wpaguiTab;
    QWidget *statusTab;
    QFrame *frame3;
    QLabel *statusLabel;
    QLabel *lastMessageLabel;
    QLabel *authenticationLabel;
    QLabel *encryptionLabel;
    QLabel *ssidLabel;
    QLabel *bssidLabel;
    QLabel *ipAddressLabel;
    QLabel *textStatus;
    QLabel *textLastMessage;
    QLabel *textAuthentication;
    QLabel *textEncryption;
    QLabel *textSsid;
    QLabel *textBssid;
    QLabel *textIpAddress;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QPushButton *scanButton;
    QWidget *networkconfigTab;
    QListWidget *networkList;
    QRadioButton *enableRadioButton;
    QPushButton *editNetworkButton;
    QPushButton *removeNetworkButton;
    QRadioButton *disableRadioButton;
    QPushButton *addNetworkButton;
    QPushButton *scanNetworkButton;
    QWidget *wpsTab;
    QLabel *label_2;
    QLabel *wpsStatusText;
    QPushButton *wpsPbcButton;
    QPushButton *wpsPinButton;
    QLabel *label;
    QLineEdit *wpsPinEdit;
    QPushButton *wpsApPinButton;
    QLabel *label_3;
    QLineEdit *wpsApPinEdit;
    QTextEdit *wpsInstructions;
    QMenuBar *MenuBar;
    QMenu *fileMenu;
    QMenu *networkMenu;
    QMenu *helpMenu;

    void setupUi(QMainWindow *WpaGui)
    {
        if (WpaGui->objectName().isEmpty())
            WpaGui->setObjectName(QString::fromUtf8("WpaGui"));
        WpaGui->resize(240, 320);
        WpaGui->setStyleSheet(QString::fromUtf8(""));
        fileEventHistoryAction = new QAction(WpaGui);
        fileEventHistoryAction->setObjectName(QString::fromUtf8("fileEventHistoryAction"));
        fileSaveConfigAction = new QAction(WpaGui);
        fileSaveConfigAction->setObjectName(QString::fromUtf8("fileSaveConfigAction"));
        fileExitAction = new QAction(WpaGui);
        fileExitAction->setObjectName(QString::fromUtf8("fileExitAction"));
        networkAddAction = new QAction(WpaGui);
        networkAddAction->setObjectName(QString::fromUtf8("networkAddAction"));
        networkEditAction = new QAction(WpaGui);
        networkEditAction->setObjectName(QString::fromUtf8("networkEditAction"));
        networkRemoveAction = new QAction(WpaGui);
        networkRemoveAction->setObjectName(QString::fromUtf8("networkRemoveAction"));
        networkEnableAllAction = new QAction(WpaGui);
        networkEnableAllAction->setObjectName(QString::fromUtf8("networkEnableAllAction"));
        networkDisableAllAction = new QAction(WpaGui);
        networkDisableAllAction->setObjectName(QString::fromUtf8("networkDisableAllAction"));
        networkRemoveAllAction = new QAction(WpaGui);
        networkRemoveAllAction->setObjectName(QString::fromUtf8("networkRemoveAllAction"));
        helpContentsAction = new QAction(WpaGui);
        helpContentsAction->setObjectName(QString::fromUtf8("helpContentsAction"));
        helpContentsAction->setEnabled(false);
        helpIndexAction = new QAction(WpaGui);
        helpIndexAction->setObjectName(QString::fromUtf8("helpIndexAction"));
        helpIndexAction->setEnabled(false);
        helpAboutAction = new QAction(WpaGui);
        helpAboutAction->setObjectName(QString::fromUtf8("helpAboutAction"));
        actionWPS = new QAction(WpaGui);
        actionWPS->setObjectName(QString::fromUtf8("actionWPS"));
        actionWPS->setEnabled(false);
        actionPeers = new QAction(WpaGui);
        actionPeers->setObjectName(QString::fromUtf8("actionPeers"));
        widget = new QWidget(WpaGui);
        widget->setObjectName(QString::fromUtf8("widget"));
        adapterLabel = new QLabel(widget);
        adapterLabel->setObjectName(QString::fromUtf8("adapterLabel"));
        adapterLabel->setGeometry(QRect(14, 3, 61, 21));
        adapterSelect = new QComboBox(widget);
        adapterSelect->setObjectName(QString::fromUtf8("adapterSelect"));
        adapterSelect->setGeometry(QRect(80, 6, 141, 21));
        networkLabel = new QLabel(widget);
        networkLabel->setObjectName(QString::fromUtf8("networkLabel"));
        networkLabel->setGeometry(QRect(10, 30, 61, 21));
        networkSelect = new QComboBox(widget);
        networkSelect->setObjectName(QString::fromUtf8("networkSelect"));
        networkSelect->setGeometry(QRect(80, 33, 141, 21));
        wpaguiTab = new QTabWidget(widget);
        wpaguiTab->setObjectName(QString::fromUtf8("wpaguiTab"));
        wpaguiTab->setGeometry(QRect(3, 59, 231, 231));
        statusTab = new QWidget();
        statusTab->setObjectName(QString::fromUtf8("statusTab"));
        frame3 = new QFrame(statusTab);
        frame3->setObjectName(QString::fromUtf8("frame3"));
        frame3->setGeometry(QRect(0, -6, 221, 171));
        frame3->setFrameShape(QFrame::NoFrame);
        frame3->setFrameShadow(QFrame::Plain);
        statusLabel = new QLabel(frame3);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));
        statusLabel->setGeometry(QRect(6, 9, 43, 17));
        lastMessageLabel = new QLabel(frame3);
        lastMessageLabel->setObjectName(QString::fromUtf8("lastMessageLabel"));
        lastMessageLabel->setGeometry(QRect(6, 35, 87, 17));
        authenticationLabel = new QLabel(frame3);
        authenticationLabel->setObjectName(QString::fromUtf8("authenticationLabel"));
        authenticationLabel->setGeometry(QRect(6, 68, 90, 17));
        encryptionLabel = new QLabel(frame3);
        encryptionLabel->setObjectName(QString::fromUtf8("encryptionLabel"));
        encryptionLabel->setGeometry(QRect(6, 91, 68, 17));
        ssidLabel = new QLabel(frame3);
        ssidLabel->setObjectName(QString::fromUtf8("ssidLabel"));
        ssidLabel->setGeometry(QRect(6, 112, 32, 17));
        bssidLabel = new QLabel(frame3);
        bssidLabel->setObjectName(QString::fromUtf8("bssidLabel"));
        bssidLabel->setGeometry(QRect(6, 133, 39, 17));
        ipAddressLabel = new QLabel(frame3);
        ipAddressLabel->setObjectName(QString::fromUtf8("ipAddressLabel"));
        ipAddressLabel->setGeometry(QRect(6, 153, 68, 17));
        textStatus = new QLabel(frame3);
        textStatus->setObjectName(QString::fromUtf8("textStatus"));
        textStatus->setGeometry(QRect(65, 5, 151, 20));
        textLastMessage = new QLabel(frame3);
        textLastMessage->setObjectName(QString::fromUtf8("textLastMessage"));
        textLastMessage->setGeometry(QRect(90, 25, 130, 40));
        textLastMessage->setWordWrap(true);
        textAuthentication = new QLabel(frame3);
        textAuthentication->setObjectName(QString::fromUtf8("textAuthentication"));
        textAuthentication->setGeometry(QRect(105, 69, 111, 17));
        textEncryption = new QLabel(frame3);
        textEncryption->setObjectName(QString::fromUtf8("textEncryption"));
        textEncryption->setGeometry(QRect(105, 92, 101, 17));
        textSsid = new QLabel(frame3);
        textSsid->setObjectName(QString::fromUtf8("textSsid"));
        textSsid->setGeometry(QRect(55, 110, 151, 20));
        textBssid = new QLabel(frame3);
        textBssid->setObjectName(QString::fromUtf8("textBssid"));
        textBssid->setGeometry(QRect(55, 133, 151, 20));
        textIpAddress = new QLabel(frame3);
        textIpAddress->setObjectName(QString::fromUtf8("textIpAddress"));
        textIpAddress->setGeometry(QRect(92, 150, 111, 20));
        connectButton = new QPushButton(statusTab);
        connectButton->setObjectName(QString::fromUtf8("connectButton"));
        connectButton->setGeometry(QRect(6, 168, 61, 27));
        disconnectButton = new QPushButton(statusTab);
        disconnectButton->setObjectName(QString::fromUtf8("disconnectButton"));
        disconnectButton->setGeometry(QRect(73, 168, 81, 27));
        scanButton = new QPushButton(statusTab);
        scanButton->setObjectName(QString::fromUtf8("scanButton"));
        scanButton->setGeometry(QRect(160, 168, 61, 27));
        wpaguiTab->addTab(statusTab, QString());
        networkconfigTab = new QWidget();
        networkconfigTab->setObjectName(QString::fromUtf8("networkconfigTab"));
        networkList = new QListWidget(networkconfigTab);
        networkList->setObjectName(QString::fromUtf8("networkList"));
        networkList->setGeometry(QRect(9, 9, 199, 94));
        networkList->setSelectionRectVisible(true);
        enableRadioButton = new QRadioButton(networkconfigTab);
        enableRadioButton->setObjectName(QString::fromUtf8("enableRadioButton"));
        enableRadioButton->setGeometry(QRect(7, 122, 71, 22));
        editNetworkButton = new QPushButton(networkconfigTab);
        editNetworkButton->setObjectName(QString::fromUtf8("editNetworkButton"));
        editNetworkButton->setGeometry(QRect(85, 120, 58, 27));
        removeNetworkButton = new QPushButton(networkconfigTab);
        removeNetworkButton->setObjectName(QString::fromUtf8("removeNetworkButton"));
        removeNetworkButton->setGeometry(QRect(149, 120, 59, 27));
        disableRadioButton = new QRadioButton(networkconfigTab);
        disableRadioButton->setObjectName(QString::fromUtf8("disableRadioButton"));
        disableRadioButton->setGeometry(QRect(7, 162, 71, 22));
        addNetworkButton = new QPushButton(networkconfigTab);
        addNetworkButton->setObjectName(QString::fromUtf8("addNetworkButton"));
        addNetworkButton->setGeometry(QRect(85, 160, 58, 27));
        scanNetworkButton = new QPushButton(networkconfigTab);
        scanNetworkButton->setObjectName(QString::fromUtf8("scanNetworkButton"));
        scanNetworkButton->setGeometry(QRect(149, 160, 59, 27));
        wpaguiTab->addTab(networkconfigTab, QString());
        wpsTab = new QWidget();
        wpsTab->setObjectName(QString::fromUtf8("wpsTab"));
        label_2 = new QLabel(wpsTab);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(9, 9, 43, 17));
        wpsStatusText = new QLabel(wpsTab);
        wpsStatusText->setObjectName(QString::fromUtf8("wpsStatusText"));
        wpsStatusText->setGeometry(QRect(76, 9, 141, 17));
        wpsPbcButton = new QPushButton(wpsTab);
        wpsPbcButton->setObjectName(QString::fromUtf8("wpsPbcButton"));
        wpsPbcButton->setGeometry(QRect(9, 32, 121, 27));
        wpsPinButton = new QPushButton(wpsTab);
        wpsPinButton->setObjectName(QString::fromUtf8("wpsPinButton"));
        wpsPinButton->setGeometry(QRect(9, 65, 81, 27));
        label = new QLabel(wpsTab);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(101, 69, 41, 17));
        wpsPinEdit = new QLineEdit(wpsTab);
        wpsPinEdit->setObjectName(QString::fromUtf8("wpsPinEdit"));
        wpsPinEdit->setEnabled(false);
        wpsPinEdit->setGeometry(QRect(150, 60, 71, 27));
        wpsPinEdit->setReadOnly(true);
        wpsApPinButton = new QPushButton(wpsTab);
        wpsApPinButton->setObjectName(QString::fromUtf8("wpsApPinButton"));
        wpsApPinButton->setEnabled(false);
        wpsApPinButton->setGeometry(QRect(9, 98, 81, 27));
        label_3 = new QLabel(wpsTab);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(101, 104, 47, 17));
        wpsApPinEdit = new QLineEdit(wpsTab);
        wpsApPinEdit->setObjectName(QString::fromUtf8("wpsApPinEdit"));
        wpsApPinEdit->setEnabled(false);
        wpsApPinEdit->setGeometry(QRect(150, 97, 71, 27));
        wpsInstructions = new QTextEdit(wpsTab);
        wpsInstructions->setObjectName(QString::fromUtf8("wpsInstructions"));
        wpsInstructions->setGeometry(QRect(10, 140, 201, 51));
        wpsInstructions->setReadOnly(true);
        wpaguiTab->addTab(wpsTab, QString());
        WpaGui->setCentralWidget(widget);
        MenuBar = new QMenuBar(WpaGui);
        MenuBar->setObjectName(QString::fromUtf8("MenuBar"));
        MenuBar->setGeometry(QRect(0, 0, 240, 27));
        fileMenu = new QMenu(MenuBar);
        fileMenu->setObjectName(QString::fromUtf8("fileMenu"));
        networkMenu = new QMenu(MenuBar);
        networkMenu->setObjectName(QString::fromUtf8("networkMenu"));
        helpMenu = new QMenu(MenuBar);
        helpMenu->setObjectName(QString::fromUtf8("helpMenu"));
        WpaGui->setMenuBar(MenuBar);

        MenuBar->addAction(fileMenu->menuAction());
        MenuBar->addAction(networkMenu->menuAction());
        MenuBar->addAction(helpMenu->menuAction());
        fileMenu->addAction(fileEventHistoryAction);
        fileMenu->addAction(fileSaveConfigAction);
        fileMenu->addAction(actionWPS);
        fileMenu->addAction(actionPeers);
        fileMenu->addSeparator();
        fileMenu->addAction(fileExitAction);
        networkMenu->addAction(networkAddAction);
        networkMenu->addAction(networkEditAction);
        networkMenu->addAction(networkRemoveAction);
        networkMenu->addSeparator();
        networkMenu->addAction(networkEnableAllAction);
        networkMenu->addAction(networkDisableAllAction);
        networkMenu->addAction(networkRemoveAllAction);
        helpMenu->addAction(helpContentsAction);
        helpMenu->addAction(helpIndexAction);
        helpMenu->addSeparator();
        helpMenu->addAction(helpAboutAction);

        retranslateUi(WpaGui);

        wpaguiTab->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(WpaGui);
    } // setupUi

    void retranslateUi(QMainWindow *WpaGui)
    {
        WpaGui->setWindowTitle(QApplication::translate("WpaGui", "wpa_gui", 0, QApplication::UnicodeUTF8));
        fileEventHistoryAction->setText(QApplication::translate("WpaGui", "Event &History", 0, QApplication::UnicodeUTF8));
        fileSaveConfigAction->setText(QApplication::translate("WpaGui", "&Save Configuration", 0, QApplication::UnicodeUTF8));
        fileExitAction->setText(QApplication::translate("WpaGui", "E&xit", 0, QApplication::UnicodeUTF8));
        networkAddAction->setText(QApplication::translate("WpaGui", "&Add", 0, QApplication::UnicodeUTF8));
        networkEditAction->setText(QApplication::translate("WpaGui", "&Edit", 0, QApplication::UnicodeUTF8));
        networkRemoveAction->setText(QApplication::translate("WpaGui", "&Remove", 0, QApplication::UnicodeUTF8));
        networkEnableAllAction->setText(QApplication::translate("WpaGui", "E&nable All", 0, QApplication::UnicodeUTF8));
        networkDisableAllAction->setText(QApplication::translate("WpaGui", "&Disable All", 0, QApplication::UnicodeUTF8));
        networkRemoveAllAction->setText(QApplication::translate("WpaGui", "Re&move All", 0, QApplication::UnicodeUTF8));
        helpContentsAction->setText(QApplication::translate("WpaGui", "&Contents...", 0, QApplication::UnicodeUTF8));
        helpIndexAction->setText(QApplication::translate("WpaGui", "&Index...", 0, QApplication::UnicodeUTF8));
        helpAboutAction->setText(QApplication::translate("WpaGui", "&About", 0, QApplication::UnicodeUTF8));
        actionWPS->setText(QApplication::translate("WpaGui", "&Wi-Fi Protected Setup", 0, QApplication::UnicodeUTF8));
        actionPeers->setText(QApplication::translate("WpaGui", "&Peers", 0, QApplication::UnicodeUTF8));
        adapterLabel->setText(QApplication::translate("WpaGui", "Adapter:", 0, QApplication::UnicodeUTF8));
        networkLabel->setText(QApplication::translate("WpaGui", "Network:", 0, QApplication::UnicodeUTF8));
        statusLabel->setText(QApplication::translate("WpaGui", "Status:", 0, QApplication::UnicodeUTF8));
        lastMessageLabel->setText(QApplication::translate("WpaGui", "Last message:", 0, QApplication::UnicodeUTF8));
        authenticationLabel->setText(QApplication::translate("WpaGui", "Authentication:", 0, QApplication::UnicodeUTF8));
        encryptionLabel->setText(QApplication::translate("WpaGui", "Encryption:", 0, QApplication::UnicodeUTF8));
        ssidLabel->setText(QApplication::translate("WpaGui", "SSID:", 0, QApplication::UnicodeUTF8));
        bssidLabel->setText(QApplication::translate("WpaGui", "BSSID:", 0, QApplication::UnicodeUTF8));
        ipAddressLabel->setText(QApplication::translate("WpaGui", "IP address:", 0, QApplication::UnicodeUTF8));
        textStatus->setText(QString());
        textLastMessage->setText(QString());
        textAuthentication->setText(QString());
        textEncryption->setText(QString());
        textSsid->setText(QString());
        textBssid->setText(QString());
        textIpAddress->setText(QString());
        connectButton->setText(QApplication::translate("WpaGui", "Connect", 0, QApplication::UnicodeUTF8));
        disconnectButton->setText(QApplication::translate("WpaGui", "Disconnect", 0, QApplication::UnicodeUTF8));
        scanButton->setText(QApplication::translate("WpaGui", "Scan", 0, QApplication::UnicodeUTF8));
        wpaguiTab->setTabText(wpaguiTab->indexOf(statusTab), QApplication::translate("WpaGui", "Current Status", 0, QApplication::UnicodeUTF8));
        enableRadioButton->setText(QApplication::translate("WpaGui", "Enabled", 0, QApplication::UnicodeUTF8));
        editNetworkButton->setText(QApplication::translate("WpaGui", "Edit", 0, QApplication::UnicodeUTF8));
        removeNetworkButton->setText(QApplication::translate("WpaGui", "Remove", 0, QApplication::UnicodeUTF8));
        disableRadioButton->setText(QApplication::translate("WpaGui", "Disabled", 0, QApplication::UnicodeUTF8));
        addNetworkButton->setText(QApplication::translate("WpaGui", "Add", 0, QApplication::UnicodeUTF8));
        scanNetworkButton->setText(QApplication::translate("WpaGui", "Scan", 0, QApplication::UnicodeUTF8));
        wpaguiTab->setTabText(wpaguiTab->indexOf(networkconfigTab), QApplication::translate("WpaGui", "Manage Networks", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("WpaGui", "Status:", 0, QApplication::UnicodeUTF8));
        wpsStatusText->setText(QString());
        wpsPbcButton->setText(QApplication::translate("WpaGui", "PBC - push button", 0, QApplication::UnicodeUTF8));
        wpsPinButton->setText(QApplication::translate("WpaGui", "Generate PIN", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("WpaGui", "PIN:", 0, QApplication::UnicodeUTF8));
        wpsApPinButton->setText(QApplication::translate("WpaGui", "Use AP PIN", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("WpaGui", "AP PIN:", 0, QApplication::UnicodeUTF8));
        wpaguiTab->setTabText(wpaguiTab->indexOf(wpsTab), QApplication::translate("WpaGui", "WPS", 0, QApplication::UnicodeUTF8));
        fileMenu->setTitle(QApplication::translate("WpaGui", "&File", 0, QApplication::UnicodeUTF8));
        networkMenu->setTitle(QApplication::translate("WpaGui", "&Network", 0, QApplication::UnicodeUTF8));
        helpMenu->setTitle(QApplication::translate("WpaGui", "&Help", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class WpaGui: public Ui_WpaGui {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WPAGUI_H
