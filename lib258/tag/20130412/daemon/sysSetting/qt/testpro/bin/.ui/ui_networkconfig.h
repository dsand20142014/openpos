/********************************************************************************
** Form generated from reading UI file 'networkconfig.ui'
**
** Created: Thu Sep 27 15:36:39 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NETWORKCONFIG_H
#define UI_NETWORKCONFIG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QWidget>
#include <QtGui/QTreeWidget>

QT_BEGIN_NAMESPACE

class Ui_NetworkConfig
{
public:
    QPushButton *cancelButton;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *useWpsButton;
    QTabWidget *tabWidget;
    QWidget *tab;
    QLabel *ssidLabel;
    QLineEdit *ssidEdit;
    QLineEdit *pskEdit;
    QLabel *authLabel;
    QLabel *pskLabel;
    QComboBox *encrSelect;
    QLabel *encrLabel;
    QComboBox *authSelect;
    QLabel *passwordLabel;
    QLineEdit *cacertEdit;
    QComboBox *eapSelect;
    QLabel *identityLabel;
    QLineEdit *passwordEdit;
    QLabel *cacertLabel;
    QLineEdit *identityEdit;
    QLabel *eapLabel;
    QWidget *tab_2;
    QLineEdit *wep3Edit;
    QLineEdit *wep1Edit;
    QLineEdit *wep0Edit;
    QRadioButton *wep2Radio;
    QLineEdit *wep2Edit;
    QRadioButton *wep3Radio;
    QRadioButton *wep1Radio;
    QRadioButton *wep0Radio;
    QWidget *widget;
    QLineEdit *idstrEdit;
    QSpinBox *prioritySpinBox;
    QLabel *phase2Label;
    QLabel *priorityLabel;
    QComboBox *phase2Select;
    QLabel *idstrLabel;

    void setupUi(QDialog *NetworkConfig)
    {
        if (NetworkConfig->objectName().isEmpty())
            NetworkConfig->setObjectName(QString::fromUtf8("NetworkConfig"));
        NetworkConfig->resize(240, 320);
        cancelButton = new QPushButton(NetworkConfig);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(160, 290, 71, 27));
        addButton = new QPushButton(NetworkConfig);
        addButton->setObjectName(QString::fromUtf8("addButton"));
        addButton->setGeometry(QRect(80, 289, 61, 27));
        removeButton = new QPushButton(NetworkConfig);
        removeButton->setObjectName(QString::fromUtf8("removeButton"));
        removeButton->setEnabled(false);
        removeButton->setGeometry(QRect(160, 289, 71, 27));
        useWpsButton = new QPushButton(NetworkConfig);
        useWpsButton->setObjectName(QString::fromUtf8("useWpsButton"));
        useWpsButton->setEnabled(false);
        useWpsButton->setGeometry(QRect(10, 289, 51, 27));
        tabWidget = new QTabWidget(NetworkConfig);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(6, 10, 230, 276));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        ssidLabel = new QLabel(tab);
        ssidLabel->setObjectName(QString::fromUtf8("ssidLabel"));
        ssidLabel->setGeometry(QRect(5, 4, 71, 20));
        ssidEdit = new QLineEdit(tab);
        ssidEdit->setObjectName(QString::fromUtf8("ssidEdit"));
        ssidEdit->setGeometry(QRect(92, 3, 132, 23));
        pskEdit = new QLineEdit(tab);
        pskEdit->setObjectName(QString::fromUtf8("pskEdit"));
        pskEdit->setEnabled(false);
        pskEdit->setGeometry(QRect(92, 91, 132, 23));
        pskEdit->setEchoMode(QLineEdit::Password);
        authLabel = new QLabel(tab);
        authLabel->setObjectName(QString::fromUtf8("authLabel"));
        authLabel->setGeometry(QRect(5, 31, 81, 20));
        pskLabel = new QLabel(tab);
        pskLabel->setObjectName(QString::fromUtf8("pskLabel"));
        pskLabel->setGeometry(QRect(5, 87, 81, 17));
        encrSelect = new QComboBox(tab);
        encrSelect->setObjectName(QString::fromUtf8("encrSelect"));
        encrSelect->setGeometry(QRect(92, 60, 132, 23));
        encrLabel = new QLabel(tab);
        encrLabel->setObjectName(QString::fromUtf8("encrLabel"));
        encrLabel->setGeometry(QRect(5, 60, 81, 17));
        authSelect = new QComboBox(tab);
        authSelect->setObjectName(QString::fromUtf8("authSelect"));
        authSelect->setGeometry(QRect(92, 30, 132, 23));
        passwordLabel = new QLabel(tab);
        passwordLabel->setObjectName(QString::fromUtf8("passwordLabel"));
        passwordLabel->setGeometry(QRect(5, 183, 81, 20));
        cacertEdit = new QLineEdit(tab);
        cacertEdit->setObjectName(QString::fromUtf8("cacertEdit"));
        cacertEdit->setEnabled(false);
        cacertEdit->setGeometry(QRect(92, 215, 132, 23));
        eapSelect = new QComboBox(tab);
        eapSelect->setObjectName(QString::fromUtf8("eapSelect"));
        eapSelect->setEnabled(false);
        eapSelect->setGeometry(QRect(92, 122, 132, 23));
        identityLabel = new QLabel(tab);
        identityLabel->setObjectName(QString::fromUtf8("identityLabel"));
        identityLabel->setGeometry(QRect(5, 150, 81, 17));
        passwordEdit = new QLineEdit(tab);
        passwordEdit->setObjectName(QString::fromUtf8("passwordEdit"));
        passwordEdit->setEnabled(false);
        passwordEdit->setGeometry(QRect(92, 183, 132, 23));
        passwordEdit->setEchoMode(QLineEdit::Password);
        cacertLabel = new QLabel(tab);
        cacertLabel->setObjectName(QString::fromUtf8("cacertLabel"));
        cacertLabel->setGeometry(QRect(5, 216, 80, 17));
        identityEdit = new QLineEdit(tab);
        identityEdit->setObjectName(QString::fromUtf8("identityEdit"));
        identityEdit->setEnabled(false);
        identityEdit->setGeometry(QRect(92, 152, 132, 23));
        eapLabel = new QLabel(tab);
        eapLabel->setObjectName(QString::fromUtf8("eapLabel"));
        eapLabel->setGeometry(QRect(5, 117, 81, 20));
        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        wep3Edit = new QLineEdit(tab_2);
        wep3Edit->setObjectName(QString::fromUtf8("wep3Edit"));
        wep3Edit->setEnabled(false);
        wep3Edit->setGeometry(QRect(80, 130, 112, 27));
        wep1Edit = new QLineEdit(tab_2);
        wep1Edit->setObjectName(QString::fromUtf8("wep1Edit"));
        wep1Edit->setEnabled(false);
        wep1Edit->setGeometry(QRect(80, 64, 112, 27));
        wep0Edit = new QLineEdit(tab_2);
        wep0Edit->setObjectName(QString::fromUtf8("wep0Edit"));
        wep0Edit->setEnabled(false);
        wep0Edit->setGeometry(QRect(80, 31, 112, 27));
        wep2Radio = new QRadioButton(tab_2);
        wep2Radio->setObjectName(QString::fromUtf8("wep2Radio"));
        wep2Radio->setEnabled(false);
        wep2Radio->setGeometry(QRect(12, 99, 62, 22));
        wep2Edit = new QLineEdit(tab_2);
        wep2Edit->setObjectName(QString::fromUtf8("wep2Edit"));
        wep2Edit->setEnabled(false);
        wep2Edit->setGeometry(QRect(80, 97, 112, 27));
        wep3Radio = new QRadioButton(tab_2);
        wep3Radio->setObjectName(QString::fromUtf8("wep3Radio"));
        wep3Radio->setEnabled(false);
        wep3Radio->setGeometry(QRect(12, 132, 62, 22));
        wep1Radio = new QRadioButton(tab_2);
        wep1Radio->setObjectName(QString::fromUtf8("wep1Radio"));
        wep1Radio->setEnabled(false);
        wep1Radio->setGeometry(QRect(12, 66, 62, 22));
        wep0Radio = new QRadioButton(tab_2);
        wep0Radio->setObjectName(QString::fromUtf8("wep0Radio"));
        wep0Radio->setEnabled(false);
        wep0Radio->setGeometry(QRect(12, 33, 62, 22));
        tabWidget->addTab(tab_2, QString());
        widget = new QWidget();
        widget->setObjectName(QString::fromUtf8("widget"));
        idstrEdit = new QLineEdit(widget);
        idstrEdit->setObjectName(QString::fromUtf8("idstrEdit"));
        idstrEdit->setGeometry(QRect(88, 43, 112, 27));
        prioritySpinBox = new QSpinBox(widget);
        prioritySpinBox->setObjectName(QString::fromUtf8("prioritySpinBox"));
        prioritySpinBox->setGeometry(QRect(88, 155, 111, 27));
        prioritySpinBox->setMaximum(10000);
        prioritySpinBox->setSingleStep(10);
        phase2Label = new QLabel(widget);
        phase2Label->setObjectName(QString::fromUtf8("phase2Label"));
        phase2Label->setGeometry(QRect(11, 100, 71, 20));
        priorityLabel = new QLabel(widget);
        priorityLabel->setObjectName(QString::fromUtf8("priorityLabel"));
        priorityLabel->setGeometry(QRect(10, 160, 71, 20));
        phase2Select = new QComboBox(widget);
        phase2Select->setObjectName(QString::fromUtf8("phase2Select"));
        phase2Select->setEnabled(false);
        phase2Select->setGeometry(QRect(88, 93, 111, 31));
        idstrLabel = new QLabel(widget);
        idstrLabel->setObjectName(QString::fromUtf8("idstrLabel"));
        idstrLabel->setGeometry(QRect(10, 47, 71, 20));
        tabWidget->addTab(widget, QString());
        addButton->raise();
        removeButton->raise();
        useWpsButton->raise();
        tabWidget->raise();
        cancelButton->raise();
        QWidget::setTabOrder(addButton, removeButton);
        QWidget::setTabOrder(removeButton, cancelButton);

        retranslateUi(NetworkConfig);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(NetworkConfig);
    } // setupUi

    void retranslateUi(QDialog *NetworkConfig)
    {
        NetworkConfig->setWindowTitle(QApplication::translate("NetworkConfig", "NetworkConfig", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("NetworkConfig", "Cancel", 0, QApplication::UnicodeUTF8));
        addButton->setText(QApplication::translate("NetworkConfig", "Add", 0, QApplication::UnicodeUTF8));
        removeButton->setText(QApplication::translate("NetworkConfig", "Remove", 0, QApplication::UnicodeUTF8));
        useWpsButton->setText(QApplication::translate("NetworkConfig", "WPS", 0, QApplication::UnicodeUTF8));
        ssidLabel->setText(QApplication::translate("NetworkConfig", "SSID", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        ssidEdit->setToolTip(QApplication::translate("NetworkConfig", "Network name (Service Set IDentifier)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        ssidEdit->setText(QString());
#ifndef QT_NO_TOOLTIP
        pskEdit->setToolTip(QApplication::translate("NetworkConfig", "WPA/WPA2 pre-shared key or passphrase", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_WHATSTHIS
        pskEdit->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        authLabel->setText(QApplication::translate("NetworkConfig", "Authentication", 0, QApplication::UnicodeUTF8));
        pskLabel->setText(QApplication::translate("NetworkConfig", "PSK", 0, QApplication::UnicodeUTF8));
        encrSelect->clear();
        encrSelect->insertItems(0, QStringList()
         << QApplication::translate("NetworkConfig", "None", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("NetworkConfig", "WEP", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("NetworkConfig", "TKIP", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("NetworkConfig", "CCMP", 0, QApplication::UnicodeUTF8)
        );
        encrLabel->setText(QApplication::translate("NetworkConfig", "Encryption", 0, QApplication::UnicodeUTF8));
        authSelect->clear();
        authSelect->insertItems(0, QStringList()
         << QApplication::translate("NetworkConfig", "Plaintext(open/no authentication)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("NetworkConfig", "Static WEP(no authentication)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("NetworkConfig", "Static WEP(Shared Key authentication)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("NetworkConfig", "IEEE 802.1X", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("NetworkConfig", "WPA-Personal (PSK)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("NetworkConfig", "WPA-Enterprise (EAP)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("NetworkConfig", "WPA2-Personal (PSK)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("NetworkConfig", "WPA2-Enterprise (EAP)", 0, QApplication::UnicodeUTF8)
        );
        passwordLabel->setText(QApplication::translate("NetworkConfig", "Password", 0, QApplication::UnicodeUTF8));
        identityLabel->setText(QApplication::translate("NetworkConfig", "Identity", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        passwordEdit->setToolTip(QApplication::translate("NetworkConfig", "Password for EAP methods", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        cacertLabel->setText(QApplication::translate("NetworkConfig", "CA certificate", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        identityEdit->setToolTip(QApplication::translate("NetworkConfig", "Username/Identity for EAP methods", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        eapLabel->setText(QApplication::translate("NetworkConfig", "EAP method", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("NetworkConfig", "Tab 1", 0, QApplication::UnicodeUTF8));
        wep2Radio->setText(QApplication::translate("NetworkConfig", "key 2", 0, QApplication::UnicodeUTF8));
        wep3Radio->setText(QApplication::translate("NetworkConfig", "key 3", 0, QApplication::UnicodeUTF8));
        wep1Radio->setText(QApplication::translate("NetworkConfig", "key 1", 0, QApplication::UnicodeUTF8));
        wep0Radio->setText(QApplication::translate("NetworkConfig", "key 0", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("NetworkConfig", "WEP keys", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        idstrEdit->setToolTip(QApplication::translate("NetworkConfig", "Network Identification String", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        prioritySpinBox->setToolTip(QApplication::translate("NetworkConfig", "Network Priority", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        phase2Label->setText(QApplication::translate("NetworkConfig", "Inner auth", 0, QApplication::UnicodeUTF8));
        priorityLabel->setText(QApplication::translate("NetworkConfig", "Priority", 0, QApplication::UnicodeUTF8));
        idstrLabel->setText(QApplication::translate("NetworkConfig", "IDString", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(widget), QApplication::translate("NetworkConfig", "Optional Settings", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class NetworkConfig: public Ui_NetworkConfig {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NETWORKCONFIG_H
