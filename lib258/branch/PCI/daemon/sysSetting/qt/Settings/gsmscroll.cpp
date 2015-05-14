#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include "gsmscroll.h"
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

GsmScroll::GsmScroll(QWidget *parent) :
    QGraphicsView(parent)
{
    cursorPressFlag = false;
    yWidgetPrivious = yWidgetCurrent = yWidgetRelease = 0;
    scene = new QGraphicsScene;
    createWidgets();
    createProxyWidgets();
    createLayout();
    createCentralWidget();
    createConnections();

    settings = new QSettings("setting.ini",QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
    displayValue();
}

void GsmScroll::createWidgets()
{
    lName = new QLabel(tr("USER NAME"));
    lPwd = new QLabel(tr("PASSWORD"));
    lOther = new QLabel(tr("OTHER "));
    QFont font = lName->font();
    font.setPointSize(font.pointSize() - 2);
    lName->setFont(font);
    lPwd->setFont(font);
    lOther->setFont(font);
//    lName->setAutoFillBackground(true);
//    lName->setBackgroundRole(QPalette::Button);

    leName = new QLineEdit();
    lePwd = new QLineEdit();
    leOther = new QLineEdit();

    gbWm = new QGroupBox("WIRLESS MODULE");
    gbMaol = new QGroupBox("MODULE ALWAYS ON LINE");
    gbDb = new QGroupBox("DEFAULT BAND");
    gbApn = new QGroupBox("DAFAULT APN");
    gbWm->setFont(font);
    gbMaol->setFont(font);
    gbDb->setFont(font);
    gbApn->setFont(font);

    rbGprs = new QRadioButton("GPRS");
    rbWcdma = new QRadioButton("WCDMA");
    rbGprs->setFont(font);
    rbWcdma->setFont(font);
//    rbGprs->setParent(gbWm);
//    rbWcdma->setParent(gbWm);
    QHBoxLayout *hBoxLayout1 = new QHBoxLayout();
    hBoxLayout1->addWidget(rbGprs);
    hBoxLayout1->addWidget(rbWcdma);
    gbWm->setLayout(hBoxLayout1);

    rbYes = new QRadioButton("YES");
    rbNo = new QRadioButton("NO");
    rbYes->setFont(font);
    rbNo->setFont(font);
    QHBoxLayout *hBoxLayout2 = new QHBoxLayout();
    hBoxLayout2->addWidget(rbYes);
    hBoxLayout2->addWidget(rbNo);
    gbMaol->setLayout(hBoxLayout2);

    rb850 = new QRadioButton("850");
    rb900 = new QRadioButton("900");
    rb1800 = new QRadioButton("1800");
    rb1900 = new QRadioButton("1900");
    rb8519 = new QRadioButton("8519");
    rb918 = new QRadioButton("918");
    rb919 = new QRadioButton("919");
    rb850->setFont(font);
    rb900->setFont(font);
    rb1800->setFont(font);
    rb1900->setFont(font);
    rb8519->setFont(font);
    rb918->setFont(font);
    rb919->setFont(font);

    QGridLayout *gridLayout1 = new QGridLayout();
    gridLayout1->addWidget(rb850,0,0);
    gridLayout1->addWidget(rb900,0,1);
    gridLayout1->addWidget(rb1800,1,0);
    gridLayout1->addWidget(rb1900,1,1);
    gridLayout1->addWidget(rb8519,2,0);
    gridLayout1->addWidget(rb918,2,1);
    gridLayout1->addWidget(rb919,3,0);
    gbDb->setLayout(gridLayout1);

    rbCmnet = new QRadioButton("CMNET");
    rbWxbc = new QRadioButton("WXBC");
    rbVodafone = new QRadioButton("VODAFONE");
    rbMposcu = new QRadioButton("MPOSCU");
    rbGzylcmda = new QRadioButton("GZYLCMDA");
    rbGzdxcmda = new QRadioButton("GZDXCMDA");
    rbChinaUnicom = new QRadioButton("CHINAUNICOM");
    rbHide = new QRadioButton("HIDE");
    rbCmnet->setFont(font);
    rbWxbc->setFont(font);
    rbVodafone->setFont(font);
    rbMposcu->setFont(font);
    rbGzylcmda->setFont(font);
    rbGzdxcmda->setFont(font);
    rbChinaUnicom->setFont(font);
    rbHide->setFont(font);

    QGridLayout *gridLayout2 = new QGridLayout();
    gridLayout2->addWidget(rbCmnet,0,0);
    gridLayout2->addWidget(rbWxbc,0,1);
    gridLayout2->addWidget(rbVodafone,1,0);
    gridLayout2->addWidget(rbMposcu,1,1);
    gridLayout2->addWidget(rbGzylcmda,2,0);
    gridLayout2->addWidget(rbGzdxcmda,2,1);
    gridLayout2->addWidget(rbChinaUnicom,3,0);
    gridLayout2->addWidget(rbHide,3,1);
    gbApn->setLayout(gridLayout2);

    pbCancel = new QPushButton(tr("CANCEL"));
    pbEnter = new QPushButton(tr("ENTER"));
}

void GsmScroll::createProxyWidgets()
{
    proxyForName["lName"] = scene->addWidget(lName);
    proxyForName["lPwd"] = scene->addWidget(lPwd);
    proxyForName["lOther"] = scene->addWidget(lOther);
    proxyForName["lName"]->setMinimumHeight(25);
    proxyForName["lPwd"]->setMinimumHeight(25);
    proxyForName["lOther"]->setMinimumHeight(25);

    proxyForName["leName"] = scene->addWidget(leName);
    proxyForName["lePwd"] = scene->addWidget(lePwd);
    proxyForName["leOther"] = scene->addWidget(leOther);
    proxyForName["leName"]->setMaximumWidth(150);
    proxyForName["lePwd"]->setMaximumWidth(150);
    proxyForName["leOther"]->setMaximumWidth(165);
    proxyForName["leName"]->setMinimumHeight(25);
    proxyForName["lePwd"]->setMinimumHeight(25);
    proxyForName["leOther"]->setMinimumHeight(25);

    proxyForName["gbWm"] = scene->addWidget(gbWm);
    proxyForName["gbMaol"] = scene->addWidget(gbMaol);
    proxyForName["gbDb"] = scene->addWidget(gbDb);
    proxyForName["gbApn"] = scene->addWidget(gbApn);
    proxyForName["gbWm"]->setMaximumWidth(220);
    proxyForName["gbMaol"]->setMaximumWidth(220);
    proxyForName["gbDb"]->setMaximumWidth(220);
    proxyForName["gbApn"]->setMaximumWidth(220);

    proxyForName["pbCancel"] = scene->addWidget(pbCancel);
    proxyForName["pbEnter"] = scene->addWidget(pbEnter);
    proxyForName["pbCancel"]->setMaximumSize(95,35);
    proxyForName["pbCancel"]->setMinimumSize(95,35);
    proxyForName["pbEnter"]->setMaximumSize(95,35);
    proxyForName["pbEnter"]->setMinimumSize(95,35);
}

void GsmScroll::createLayout()
{
    QGraphicsLinearLayout *hLayout1 = new QGraphicsLinearLayout(
            Qt::Horizontal);
    hLayout1->addItem(proxyForName["lName"]);
    hLayout1->addItem(proxyForName["leName"]);

    QGraphicsLinearLayout *hLayout2 = new QGraphicsLinearLayout(
            Qt::Horizontal);
    hLayout2->addItem(proxyForName["lPwd"]);
    hLayout2->addItem(proxyForName["lePwd"]);

    QGraphicsLinearLayout *hLayout3 = new QGraphicsLinearLayout(
            Qt::Horizontal);
    hLayout3->addItem(proxyForName["lOther"]);
    hLayout3->addItem(proxyForName["leOther"]);

    QGraphicsLinearLayout *hLayout6 = new QGraphicsLinearLayout(
            Qt::Horizontal);
    hLayout6->addItem(proxyForName["pbCancel"]);
    hLayout6->setItemSpacing(0,25);
    hLayout6->addItem(proxyForName["pbEnter"]);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(
            Qt::Vertical);
    layout->setSpacing(20);
    layout->addItem(hLayout1);
    layout->addItem(hLayout2);
    layout->addItem(proxyForName["gbWm"]);
    layout->addItem(proxyForName["gbMaol"]);
    layout->addItem(proxyForName["gbDb"]);
    layout->addItem(proxyForName["gbApn"]);
    layout->addItem(hLayout3);
    layout->addItem(hLayout6);

    QPixmap pixmap("./image/lanse.png");
    pixmap = pixmap.scaled(SCREENHEIWIDTH,SCREENHEIGHT);
    scene->addPixmap(pixmap);

    centerWidget = new QGraphicsWidget;
    centerWidget->setLayout(layout);
    centerWidget->setParent(this);
    scene->addItem(centerWidget);
    centerWidget->setZValue(1);

    int width = SCREENHEIWIDTH;
    int height = SCREENHEIGHT;
    setMinimumSize(width, height);
    scene->setSceneRect(0, 0, width, height);
}

void GsmScroll::createCentralWidget()
{
    //view = new QGraphicsView(this);
    //view->setScene(scene);
    //setCentralWidget(view);
    setScene(scene);
}

void GsmScroll::pwdDeal(QString str)
{
    QString strXing;
    if(strPwd.length() - str.length() < 0)
    {
        strPwd += str.right(1);
    }
    else
    {
        strPwd = strPwd.remove(strPwd.length()-1,1);
    }
    strXing = "";
    for(int i=0; i<strPwd.length(); i++)
    {
        strXing += "*";
    }
    lePwd->setText(strXing);
}

void GsmScroll::lineEditDeal(QString str)
{
    if(str == "CMNET")
    {
        rbCmnet->setChecked(true);
    }
    else if(str == "MPOSCUPSH.SH")
    {
        rbMposcu->setChecked(true);
    }
    else if(str == "VODAFONE")
    {
        rbVodafone->setChecked(true);
    }
    else if(str == "CHINA UNICOM")
    {
        rbChinaUnicom->setChecked(true);
    }
    else if(str == "WXBC")
    {
        rbWxbc->setChecked(true);
    }
    else if(str == "GZYLCMDA")
    {
        rbGzylcmda->setChecked(true);
    }
    else if(str == "GZDXCMDA")
    {
        rbGzdxcmda->setChecked(true);
    }
    else
    {
        rbHide->setChecked(true);
    }
}

void GsmScroll::radioButton1Deal(bool isChecked)
{
    if(isChecked)
    {
        leOther->setText("CMNET");
    }
}

void GsmScroll::radioButton2Deal(bool isChecked)
{
    if(isChecked)
    {
        leOther->setText("WXBC");
    }
}

void GsmScroll::radioButton3Deal(bool isChecked)
{
    if(isChecked)
    {
        leOther->setText("VODAFONE");
    }
}

void GsmScroll::radioButton4Deal(bool isChecked)
{
    if(isChecked)
    {
        leOther->setText("CHINA UNICOM");
    }
}

void GsmScroll::radioButton5Deal(bool isChecked)
{
    if(isChecked)
    {
        leOther->setText("MPOSCUPSH.SH");
    }
}

void GsmScroll::radioButton6Deal(bool isChecked)
{
    if(isChecked)
    {
        leOther->setText("GZYLCMDA");
    }
}

void GsmScroll::radioButton7Deal(bool isChecked)
{
    if(isChecked)
    {
        leOther->setText("GZDXCMDA");
    }
}

void GsmScroll::createConnections()
{
    connect(pbCancel,SIGNAL(clicked()),this,SLOT(close()));
    connect(pbEnter,SIGNAL(clicked()),this,SLOT(saveParameters()));
    connect(pbEnter,SIGNAL(clicked()),this,SLOT(close()));
    connect(lePwd,SIGNAL(textEdited(QString)),this,SLOT(pwdDeal(QString)));

    connect(rbCmnet,SIGNAL(toggled(bool)),this,SLOT(radioButton1Deal(bool)));
    connect(rbWxbc,SIGNAL(toggled(bool)),this,SLOT(radioButton2Deal(bool)));
    connect(rbVodafone,SIGNAL(toggled(bool)),this,SLOT(radioButton3Deal(bool)));
    connect(rbMposcu,SIGNAL(toggled(bool)),this,SLOT(radioButton4Deal(bool)));
    connect(rbGzylcmda,SIGNAL(toggled(bool)),this,SLOT(radioButton5Deal(bool)));
    connect(rbGzdxcmda,SIGNAL(toggled(bool)),this,SLOT(radioButton6Deal(bool)));
    connect(rbChinaUnicom,SIGNAL(toggled(bool)),this,SLOT(radioButton7Deal(bool)));

    connect(leOther,SIGNAL(textEdited(QString)),this,SLOT(lineEditDeal(QString)));
    rbHide->hide();
}

void GsmScroll::displayValue()
{
    readSetting.clear();
    readSetting.readKeys(settings,"GSM_SETTING");
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "04.USERNAME")
        {
            if(readSetting.keysValue[i] == "NULL")
            {
                leName->setText("");
            }
            else
            {
                leName->setText(readSetting.keysValue[i]);
            }
        }
        if(readSetting.keys[i] == "05.PASSWD")
        {
            if(readSetting.keysValue[i] == "NULL")
            {
                lePwd->setText("");
            }
            else
            {
                lePwd->setText("");
            }
        }
        if(readSetting.keys[i] == "01.WIRELESS_MODULE")
        {
            if(readSetting.keysValue[i] == "0")
            {
                rbGprs->setChecked(true);
            }
            if(readSetting.keysValue[i] == "1")
            {
                rbWcdma->setChecked(true);
            }
        }
        if(readSetting.keys[i] == "02.MODULE_ALWAYS_ONLINE")
        {
            if(readSetting.keysValue[i] == "0")
            {
                rbNo->setChecked(true);
            }
            if(readSetting.keysValue[i] == "1")
            {
                rbYes->setChecked(true);
            }
        }
        if(readSetting.keys[i] == "06.DEFAULT_BAND")
        {
            if(readSetting.keysValue[i] == "1")
            {
                rb850->setChecked(true);
            }
            if(readSetting.keysValue[i] == "2")
            {
                rb900->setChecked(true);
            }
            if(readSetting.keysValue[i] == "3")
            {
                rb1800->setChecked(true);
            }
            if(readSetting.keysValue[i] == "4")
            {
                rb1900->setChecked(true);
            }
            if(readSetting.keysValue[i] == "5")
            {
                rb8519->setChecked(true);
            }
            if(readSetting.keysValue[i] == "6")
            {
                rb918->setChecked(true);
            }
            if(readSetting.keysValue[i] == "7")
            {
                rb919->setChecked(true);
            }
        }
        if(readSetting.keys[i] == "03.DEFAULT_APN")
        {
            if(readSetting.keysValue[i] == "CMNET")
            {
                rbCmnet->setChecked(true);
            }
            if(readSetting.keysValue[i] == "MPOSCUPSH.SH")
            {
                rbMposcu->setChecked(true);
            }
            if(readSetting.keysValue[i] == "VODAFONE")
            {
                rbVodafone->setChecked(true);
            }
            if(readSetting.keysValue[i] == "CHINA UNICOM")
            {
                rbChinaUnicom->setChecked(true);
            }
            if(readSetting.keysValue[i] == "WXBC")
            {
                rbWxbc->setChecked(true);
            }
            if(readSetting.keysValue[i] == "GZYLCMDA")
            {
                rbGzylcmda->setChecked(true);
            }
            if(readSetting.keysValue[i] == "GZDXCMDA")
            {
                rbGzdxcmda->setChecked(true);
            }
            leOther->setText(readSetting.keysValue[i]);
        }
    }
}

void GsmScroll::saveParameters()
{
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "04.USERNAME")
        {
            if(leName->text().isEmpty())
            {
                readSetting.keysValueBuffer[i] = "NULL";
            }
            else
            {
                readSetting.keysValueBuffer[i] = leName->text();
            }
        }
        if(readSetting.keys[i] == "05.PASSWD")
        {
            if(lePwd->text().isEmpty())
            {
                readSetting.keysValueBuffer[i] = "NULL";
            }
            else
            {
                readSetting.keysValueBuffer[i] = strPwd;
            }
        }
        if(readSetting.keys[i] == "01.WIRELESS_MODULE")
        {
            if(rbGprs->isChecked())
            {
                readSetting.keysValueBuffer[i] = "0";
            }
            if(rbWcdma->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
        }
        if(readSetting.keys[i] == "02.MODULE_ALWAYS_ONLINE")
        {
            if(rbYes->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
            if(rbNo->isChecked())
            {
                readSetting.keysValueBuffer[i] = "0";
            }
        }
        if(readSetting.keys[i] == "06.DEFAULT_BAND")
        {
            if(rb850->isChecked())
            {
                readSetting.keysValueBuffer[i] = "1";
            }
            if(rb900->isChecked())
            {
                readSetting.keysValueBuffer[i] = "2";
            }
            if(rb1800->isChecked())
            {
                readSetting.keysValueBuffer[i] = "3";
            }
            if(rb1900->isChecked())
            {
                readSetting.keysValueBuffer[i] = "4";
            }
            if(rb8519->isChecked())
            {
                readSetting.keysValueBuffer[i] = "5";
            }
            if(rb918->isChecked())
            {
                readSetting.keysValueBuffer[i] = "6";
            }
            if(rb919->isChecked())
            {
                readSetting.keysValueBuffer[i] = "7";
            }
        }
        if(readSetting.keys[i] == "03.DEFAULT_APN")
        {
            readSetting.keysValueBuffer[i] = leOther->text();
        }
    }
    readSetting.keysValue = readSetting.keysValueBuffer;
    readSetting.setKeys(settings,"GSM_SETTING");
    readSetting.clear();
}


void GsmScroll::mousePressEvent(QMouseEvent *event)
{
    cursorPressFlag = true;
    yPositionPress = event->y();
    yPositionPrevious = yPositionPress;
    time.start();
    QGraphicsView::mousePressEvent(event);
}

void GsmScroll::mouseMoveEvent(QMouseEvent *event)
{
    miniWidgetPos = -(centerWidget->size().height() - SCREENHEIGHT);
    maxiWidgetPos = 0;
    if(cursorPressFlag)
    {
        yPositionCurrent = event->y();
        int cursorMoveDist = yPositionCurrent-yPositionPrevious;
        yWidgetCurrent = yWidgetPrivious + cursorMoveDist;
        if(yWidgetCurrent < miniWidgetPos - 50)
        {
            yWidgetCurrent = miniWidgetPos - 50;
        }
        else if(yWidgetCurrent >= maxiWidgetPos + 50)
        {
            yWidgetCurrent = maxiWidgetPos + 50;
        }
        centerWidget->setPos(0,yWidgetCurrent);
        yPositionPrevious = yPositionCurrent;
        yWidgetPrivious = yWidgetCurrent;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GsmScroll::mouseReleaseEvent(QMouseEvent *event)
{
     animationDuration = DURATION;
     yPositionRelease = event->y();
     int elapsedTime = time.elapsed();
     miniWidgetPos = -(centerWidget->size().height() - SCREENHEIGHT);
     maxiWidgetPos = 0;
     int speed = qAbs(yPositionRelease - yPositionPress) * 1000 / elapsedTime;
     int cursorMoveDist = yPositionRelease - yPositionPress;

     if(cursorMoveDist > 5)
     {
         directionFlag = DOWN;
     }
     if(cursorMoveDist < -5)
     {
         directionFlag = UP;
     }
     if(directionFlag == UP)
     {
         animationUp = new QPropertyAnimation(centerWidget, "pos");
         animationUp->setDuration(animationDuration);
         scrollStartPos.setY(yWidgetCurrent);
         animationUp->setStartValue(scrollStartPos);
         if(speed >= BASESPEED)
         {
             if(yWidgetPrivious + cursorMoveDist*(speed/BASESPEED) >= miniWidgetPos)
             {
                 scrollEndPos.setY(yWidgetPrivious + cursorMoveDist*(speed/BASESPEED));
                 yWidgetCurrent = yWidgetPrivious + cursorMoveDist*(speed/BASESPEED);
             }
             else
             {
                 scrollEndPos.setY(miniWidgetPos);
                 yWidgetCurrent = miniWidgetPos;
                 yWidgetPrivious = yWidgetCurrent;
             }
         }
         if(speed < BASESPEED)
         {
             if(yWidgetPrivious >= miniWidgetPos)
             {
                 scrollEndPos.setY(yWidgetPrivious);
                 yWidgetCurrent = yWidgetPrivious;
             }
             else
             {
                 scrollEndPos.setY(miniWidgetPos);
                 yWidgetCurrent = miniWidgetPos;
                 yWidgetPrivious = yWidgetCurrent;
             }
         }
         animationUp->setEndValue(scrollEndPos);
         animationUp->setEasingCurve(QEasingCurve::InOutCubic);
         animationUp->start();
     }
     if(directionFlag == DOWN)
     {
         animationDown = new QPropertyAnimation(centerWidget, "pos");
         animationDown->setDuration(animationDuration);
         scrollStartPos.setY(yWidgetCurrent);
         animationDown->setStartValue(scrollStartPos);
         if(speed >= BASESPEED)
         {
             if(yWidgetPrivious + cursorMoveDist*(speed/BASESPEED) <= maxiWidgetPos)
             {
                 scrollEndPos.setY(yWidgetPrivious + cursorMoveDist*(speed/BASESPEED));
                 yWidgetCurrent = yWidgetPrivious + cursorMoveDist*(speed/BASESPEED);
             }
             else
             {
                 scrollEndPos.setY(maxiWidgetPos);
                 yWidgetCurrent = maxiWidgetPos;
                 yWidgetPrivious = yWidgetCurrent;
             }
         }
         if(speed < BASESPEED)
         {
             if(yWidgetPrivious <= maxiWidgetPos)
             {
                 scrollEndPos.setY(yWidgetPrivious);
                 yWidgetCurrent = yWidgetPrivious;
             }
             else
             {
                 scrollEndPos.setY(maxiWidgetPos);
                 yWidgetCurrent = maxiWidgetPos;
                 yWidgetPrivious = yWidgetCurrent;
             }
         }
         animationDown->setEndValue(scrollEndPos);
         animationDown->setEasingCurve(QEasingCurve::InOutCubic);
         animationDown->start();
     }

     cursorPressFlag = false;
     directionFlag = INITIAL;
     QGraphicsView::mouseReleaseEvent(event);
}


