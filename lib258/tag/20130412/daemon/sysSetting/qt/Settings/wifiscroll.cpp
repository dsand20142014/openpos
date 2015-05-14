#include "wifiscroll.h"
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSizeF>
#include <QFont>
#include <QPixmap>
#include <QDebug>
#include <cmath>
#include <QGraphicsRectItem>

const int DURATION = 200;
const int BASESPEED = 500;
const int SCREENHEIGHT = 320;
const int SCREENHEIWIDTH = 240;

WifiScroll::WifiScroll(QWidget *parent) :
    QGraphicsView(parent)
{
    cursorPressFlag = false;
    yWidgetPrivious = yWidgetCurrent = yWidgetRelease = 0;
    scene = new QGraphicsScene;
    createWidgets();
    createProxyWidgets();
//    createLayout();
//    createCentralWidget();
//    createConnections();

    settings = new QSettings("setting.ini",QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
    displayValue();
}

void WifiScroll::createWidgets()
{
    lAn = new QLabel(tr("AP NAME"));
    lPwd = new QLabel(tr("PASSWORD"));
    lIp = new QLabel(tr("IP "));
    lMask = new QLabel(tr("MASK "));
    lRouter = new QLabel(tr("ROUTER"));
    QFont font = lAn->font();
    font.setPointSize(font.pointSize() - 2);
    lAn->setFont(font);
    lPwd->setFont(font);
    lIp->setFont(font);
    lMask->setFont(font);
    lRouter->setFont(font);

    leAn = new QLineEdit();
    lePwd = new QLineEdit();
    leIp = new QLineEdit();
    leMask = new QLineEdit();
    leRouter = new QLineEdit();

    gbActive = new QGroupBox("ACTIVE");
    gbSecurity = new QGroupBox("SECURITY");
    gbDhcp = new QGroupBox("DHCP");
    gbActive->setFont(font);
    gbSecurity->setFont(font);
    gbDhcp->setFont(font);

    rbOyes = new QRadioButton("YES");
    rbOno = new QRadioButton("NO");
    rbOyes->setFont(font);
    rbOno->setFont(font);
    QHBoxLayout *hBoxLayout1 = new QHBoxLayout();
    hBoxLayout1->addWidget(rbOyes);
    hBoxLayout1->addWidget(rbOno);
    gbActive->setLayout(hBoxLayout1);

    rbDyes = new QRadioButton("YES");
    rbDno = new QRadioButton("NO");
    rbDyes->setFont(font);
    rbDno->setFont(font);
    QHBoxLayout *hBoxLayout2 = new QHBoxLayout();
    hBoxLayout2->addWidget(rbDyes);
    hBoxLayout2->addWidget(rbDno);
    gbDhcp->setLayout(hBoxLayout2);

    rbSwep = new QRadioButton("WEP");
    rbSwpa = new QRadioButton("WPA");
    rbSwpa2 = new QRadioButton("WPA2");
    rbSwpaa = new QRadioButton("WPAA");
    rbSwep->setFont(font);
    rbSwpa->setFont(font);
    rbSwpa2->setFont(font);
    rbSwpaa->setFont(font);

    QGridLayout *gridLayout1 = new QGridLayout();
    gridLayout1->addWidget(rbSwep,0,0);
    gridLayout1->addWidget(rbSwpa,0,1);
    gridLayout1->addWidget(rbSwpa2,1,0);
    gridLayout1->addWidget(rbSwpaa,1,1);
    gbSecurity->setLayout(gridLayout1);

    pbCancel = new QPushButton(tr("CANCEL"));
    pbEnter = new QPushButton(tr("ENTER"));
}

void WifiScroll::createProxyWidgets()
{
    proxyForName["lAn"] = scene->addWidget(lAn);
    proxyForName["lPwd"] = scene->addWidget(lPwd);
    proxyForName["lIp"] = scene->addWidget(lIp);
    proxyForName["lMask"] = scene->addWidget(lMask);
    proxyForName["lRouter"] = scene->addWidget(lRouter);
    proxyForName["lAn"]->setMinimumHeight(25);
    proxyForName["lPwd"]->setMinimumHeight(25);
    proxyForName["lIp"]->setMinimumHeight(25);
    proxyForName["lMask"]->setMinimumHeight(25);
    proxyForName["lRouter"]->setMinimumHeight(25);

    proxyForName["leAn"] = scene->addWidget(leAn);
    proxyForName["lePwd"] = scene->addWidget(lePwd);
    proxyForName["leIp"] = scene->addWidget(leIp);
    proxyForName["leMask"] = scene->addWidget(leMask);
    proxyForName["leRouter"] = scene->addWidget(leRouter);
    proxyForName["leAn"]->setMaximumWidth(150);
    proxyForName["lePwd"]->setMaximumWidth(150);
    proxyForName["leIp"]->setMaximumWidth(165);
    proxyForName["leMask"]->setMaximumWidth(165);
    proxyForName["leRouter"]->setMaximumWidth(165);
    proxyForName["leAn"]->setMinimumHeight(25);
    proxyForName["lePwd"]->setMinimumHeight(25);
    proxyForName["leIp"]->setMinimumHeight(25);
    proxyForName["leMask"]->setMinimumHeight(25);
    proxyForName["leRouter"]->setMinimumHeight(25);

    proxyForName["gbActive"] = scene->addWidget(gbActive);
    proxyForName["gbSecurity"] = scene->addWidget(gbSecurity);
    proxyForName["gbDhcp"] = scene->addWidget(gbDhcp);
    proxyForName["gbActive"]->setMaximumWidth(220);
    proxyForName["gbSecurity"]->setMaximumWidth(220);
    proxyForName["gbDhcp"]->setMaximumWidth(220);

    proxyForName["pbCancel"] = scene->addWidget(pbCancel);
    proxyForName["pbEnter"] = scene->addWidget(pbEnter);
    proxyForName["pbCancel"]->setMaximumSize(95,35);
    proxyForName["pbCancel"]->setMinimumSize(95,35);
    proxyForName["pbEnter"]->setMaximumSize(95,35);
    proxyForName["pbEnter"]->setMinimumSize(95,35);
}



