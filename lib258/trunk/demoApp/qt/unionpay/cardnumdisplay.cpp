#include "cardnumdisplay.h"
#include "ui_cardnumdisplay.h"
#include <QString>
#include <QTextEdit>
#include <QDebug>

//#include <QRegExp>
#include "frame.h"
#include "msginfo.h"

#include "mythread.h"
#include <QtCore/qmath.h>
extern "C"{
#include "include.h"
#include "global.h"
}

CardNumDisplay::CardNumDisplay(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::CardNumDisplay)
{
    ui->setupUi(this);
    pwdStr="";
    pwdCnt=0;
    xingCnt=0;
    pwdNum=0;
    amt=0;
    amtStr="";
    amtCnt=0;
    amtTmp=0.0;
//    QRegExp regExp("[0-5]{0,6}");
//    ui->lineEdit_2_pwd->setValidator(new QRegExpValidator(regExp,this));
    ui->lineEdit_1_amt->installEventFilter(this);
    ui->lineEdit_2_pwd->installEventFilter(this);
/*    
	char aucISO2DataTmp[38];
    qstrncpy(aucISO2DataTmp,aucISO2DataBack,20);
    QString str(aucISO2DataTmp);
    ui->cardNumEdit->setText(str);
*/
}

void CardNumDisplay::timeSlot()
{
    time->stop();
    ui->lineEdit_1_amt->setText("0.00");
    ui->lineEdit_1_amt->setFocus();
}

void CardNumDisplay::showEvent(QShowEvent *event)
{
    char aucISO2DataTmp[38];
    char * lpData;
    lpData = qstrncpy(aucISO2DataTmp,aucISO2DataBack,20);
    QString str(aucISO2DataTmp);
    ui->cardNumEdit->setText(str);
    time=new QTimer();
    connect(time,SIGNAL(timeout()),this,SLOT(timeSlot()));
//    time->setSingleShot();
    time->start(1);
//    ui->lineEdit_1_amt->setText("0.00");

//    focusNextChild();
//    ui->lineEdit_1_amt->setFocus();
//    ui->lineEdit_1_amt->setText("0.00");
//    ui->lineEdit_1_amt->setCursor();
//    setWindowFlags(Qt::Window);
//    showFullScreen();
//    QRegExp regExp("[0-5]{0,6}");
//    ui->lineEdit_2_pwd->setValidator(new QRegExpValidator(regExp,this));
    event->accept();
}

CardNumDisplay::~CardNumDisplay()
{
    delete ui;
}

void CardNumDisplay::on_Bt_Cancel_clicked()
{
    emit(cardNumDisplayCancel());
    memset((uchar*)&ProUiFace.OweDrv,0x00,sizeof(ProUiFace.OweDrv));
    memset((uchar*)&ProUiFace.ProToUi,0x00,sizeof(ProUiFace.ProToUi));
    memset((uchar*)&ProUiFace.UiToPro,0x00,sizeof(ProUiFace.UiToPro));
    destroy();
}

void CardNumDisplay::destroyWindow(int)
{
    qDebug("6666666666666666666666666666666666666");
//    a->terminate();
    memset((uchar*)&ProUiFace.OweDrv,0x00,sizeof(ProUiFace.OweDrv));
    memset((uchar*)&ProUiFace.ProToUi,0x00,sizeof(ProUiFace.ProToUi));
    memset((uchar*)&ProUiFace.UiToPro,0x00,sizeof(ProUiFace.UiToPro));
    emit(cardNumDisplayCancel());
    destroy();
}
void CardNumDisplay::on_Bt_Enter_clicked()
{
	//emit(cardNumDisplayCancel());
	//destroy();
	qDebug("1.mainkey7...");
	//

    //密码
//	ProUiFace.UiToPro.uiPwLen = strlen(ui->lineEdit_2_pwd->text().toAscii());
//	memset(ProUiFace.UiToPro.aucPasswd,0,24);
//	memcpy(ProUiFace.UiToPro.aucPasswd,
//	           ui->lineEdit_2_pwd->text().toAscii(),
//	           ProUiFace.UiToPro.uiPwLen);
    ProUiFace.UiToPro.uiPwLen = strlen(pwdStr.toAscii());
    memset(ProUiFace.UiToPro.aucPasswd,0,24);
    memcpy(ProUiFace.UiToPro.aucPasswd,
               pwdStr.toAscii(),
               ProUiFace.UiToPro.uiPwLen);
    qDebug("9999999999999999999999999999999999999999999999999999999999999999999");
	//memcpy(ProUiFace.UiToPro.aucPasswd,"123456", iLen);
	//金额
//	ProUiFace.UiToPro.ulAmount = ui->lineEdit_1_amt->text().toInt(0,10);
//    ProUiFace.UiToPro.ulAmount = 1;
    ProUiFace.UiToPro.ulAmount = amt;

//	qDebug("1.mainkey7...[%d]", ProUiFace.UiToPro.ulAmount);
//	qDebug("2.mainkey7...[%d][%s]",ProUiFace.UiToPro.uiPwLen, ProUiFace.UiToPro.aucPasswd);
	//交易类型
    ProUiFace.UiToPro.ucTransType = TRANS_PURCHASE;
    ProUiFace.UiToPro.CardInfo.ucInputMode= TRANS_INPUTMODE_SWIPECARD;
//    MyThread*a=new MyThread();
//	a->start();
    MsgInfo *desk=new MsgInfo();
    if(grandFartherFlag == 1)
    {
        desk->grandFartherFlag = 1;
    }
    connect(desk,SIGNAL(returnGrandFartherMenu(int)),this,SLOT(destroyWindow(int)));
    desk->show();
    desk->showFullScreen();
	qDebug("3.mainkey7...");
}
/*
void CardNumDisplay::keyPressEvent(QKeyEvent *event)
{
    qDebug("the key is press");
    if(event->key()==Qt::Key_Backspace)
    {
        qDebug("the backspace key is press");
        pwdStr=pwdStr.left(pwdStr.length()-1);
        pwdCnt--;
    }
    event->accept();
}
*/

bool CardNumDisplay::eventFilter(QObject *target, QEvent *event)
{
     QString str;
    if(target==ui->lineEdit_1_amt)
    {
        if(event->type()==QEvent::KeyPress)
        {
//            amtCnt=strlen(amtStr.toAscii());

            QKeyEvent*keyEvent=static_cast<QKeyEvent*>(event);
            switch(keyEvent->key())
            {
            case Qt::Key_Enter:
                focusNextChild();
                break;
            case Qt::Key_Backspace:
                qDebug("the key_back is cheak");
                amtStr=amtStr.left(amtCnt-1);
                break;
            case Qt::Key_0:
                qDebug("the key_0 is cheak");

                if(amtCnt<10)
                {
                    amtStr=amtStr+"0";
                }
                break;
            case Qt::Key_1:

                qDebug("the key_1 is cheak");

                if(amtCnt<10)
                {
                    amtStr=amtStr+"1";
                }
                break;
            case Qt::Key_2:
                qDebug("the key_2 is cheak");
                if(amtCnt<10)
                {
                    amtStr=amtStr+"2";
                }
                break;
            case Qt::Key_3:
                qDebug("the key_3 is cheak");
                if(amtCnt<10)
                {
                    amtStr=amtStr+"3";
                }
                break;
            case Qt::Key_4:
                qDebug("the key_4 is cheak");
                if(amtCnt<10)
                {
                    amtStr=amtStr+"4";
                }
                break;
            case Qt::Key_5:
                if(amtCnt<10)
                {
                    amtStr=amtStr+"5";
                }
                break;
            case Qt::Key_6:
                if(amtCnt<10)
                {
                    amtStr=amtStr+"6";
                }
                break;
            case Qt::Key_7:
                if(amtCnt<10)
                {
                    amtStr=amtStr+"7";
                }
                break;
            case Qt::Key_8:
                if(amtCnt<10)
                {
                    amtStr=amtStr+"8";
                }
                break;
            case Qt::Key_9:
                if(amtCnt<10)
                {
                    amtStr=amtStr+"9";
                }
                break;
            }

            qDebug()<<amtStr;
            amtCnt=amtStr.length();
            switch(amtCnt)
            {
            case 0:
                ui->lineEdit_1_amt->setText("0.00");
                break;
            case 1:
                ui->lineEdit_1_amt->setText("0.0"+amtStr);
                break;
            case 2:
                ui->lineEdit_1_amt->setText("0."+amtStr);
                break;
            default:
                ui->lineEdit_1_amt->setText(amtStr.left(amtCnt-2)+"."+amtStr.right(2));
                break;
            }

            qDebug()<<amtStr;

            return true;
        }
    }
    if(target==ui->lineEdit_2_pwd)
    {
        if(event->type()==QEvent::KeyPress)
        {
//            if(pwdCnt>=0 && pwdCnt<=5)
//            {
                QKeyEvent*keyEvent=static_cast<QKeyEvent*>(event);

                switch(keyEvent->key())
                {
                    case Qt::Key_Enter:
                        on_Bt_Enter_clicked();
                        break;
                    case Qt::Key_Backspace:
                        qDebug("the key_back is cheak");
                        pwdStr=pwdStr.left(pwdStr.length()-1);
//                        qDebug()<<pwdStr;
//                        pwdCnt--;
                        break;
                    case Qt::Key_0:
                        qDebug("the key_0 is cheak");
                        if(pwdCnt<12)
                        {
                            pwdStr+="0";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_1:
                        qDebug("the key_1 is cheak");
                        if(pwdCnt<12)
                        {
                            pwdStr+="1";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_2:
                        if(pwdCnt<12)
                        {
                            pwdStr+="2";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_3:
                        if(pwdCnt<12)
                        {
                            pwdStr+="3";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_4:
                        if(pwdCnt<12)
                        {
                            pwdStr+="4";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_5:
                        if(pwdCnt<12)
                        {
                            pwdStr+="5";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_6:
                        if(pwdCnt<12)
                        {
                            pwdStr+="6";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_7:
                        if(pwdCnt<12)
                        {
                            pwdStr+="7";
                        }
                        pwdCnt++;
                        break;
                    case Qt::Key_8:
                        if(pwdCnt<12)
                        {
                            pwdStr+="8";
                        }
    //                    pwdCnt++;
                        break;
                    case Qt::Key_9:
                        if(pwdCnt<12)
                        {
                            pwdStr+="9";
                        }
    //                    pwdCnt++;
                        break;
                }
                pwdCnt=strlen(pwdStr.toAscii());
                switch(pwdCnt)
                {
                case 0:
                    ui->lineEdit_2_pwd->setText("");
                    break;
                case 1:
                    ui->lineEdit_2_pwd->setText("*");
                    break;
                case 2:
                    ui->lineEdit_2_pwd->setText("**");
                    break;
                case 3:
                    ui->lineEdit_2_pwd->setText("***");
                    break;
                case 4:
                    ui->lineEdit_2_pwd->setText("****");
                    break;
                case 5:
                    ui->lineEdit_2_pwd->setText("*****");
                    break;
                case 6:
                    ui->lineEdit_2_pwd->setText("******");
                    break;
                case 7:
                    ui->lineEdit_2_pwd->setText("*******");
                    break;
                case 8:
                    ui->lineEdit_2_pwd->setText("********");
                    break;
                case 9:
                    ui->lineEdit_2_pwd->setText("*********");
                    break;
                case 10:
                    ui->lineEdit_2_pwd->setText("**********");
                    break;
                case 11:
                    ui->lineEdit_2_pwd->setText("***********");
                    break;
                case 12:
                    ui->lineEdit_2_pwd->setText("************");
                    break;
                }
                return true;
    //            if(keyEvent->key()==Qt::Key_Backspace)
    //            {
    //                qDebug("the filter is cheak");
    //                pwdStr=pwdStr.left(pwdStr.length()-1);
    //                pwdCnt--;
    //                return true;
    //            }
    //            if(event->type()==Qt::Key_0||event->type()==Qt::Key_1
    //                    ||event->type()==Qt::Key_2||event->type()==Qt::Key_3
    //                    ||event->type()==Qt::Key_4||event->type()==Qt::Key_5
    //                    ||event->type()==Qt::Key_6||event->type()==Qt::Key_7
    //                    ||event->type()==Qt::Key_8||event->type()==Qt::Key_9)
    //            {

    //            }
//            }
        }

    }
    return QDialog::eventFilter(target,event);
}
/*
void CardNumDisplay::on_lineEdit_2_pwd_textEdited(const QString &arg1)
{
    if(pwdCnt>=0 && pwdCnt<=6)
    {
        pwdStr+=arg1.right(1);
        pwdCnt=strlen(pwdStr.toAscii());
    //    unsigned char tmp[12];
    //    memset(tmp,' ',12);
    //    memcpy(tmp,'*',12);
    //    qDebug("%s",pwdStr);
        qDebug()<<arg1;
        qDebug()<<pwdStr;
        qDebug("___________________________________the num of pwd is %d",pwdCnt);
        switch(pwdCnt)
        {
        case 1:
            ui->lineEdit_2_pwd->setText("*");
            break;
        case 2:
            ui->lineEdit_2_pwd->setText("**");
            break;
        case 3:
            ui->lineEdit_2_pwd->setText("***");
            break;
        case 4:
            ui->lineEdit_2_pwd->setText("****");
            break;
        case 5:
            ui->lineEdit_2_pwd->setText("*****");
            break;
        case 6:
            ui->lineEdit_2_pwd->setText("******");
            break;
        }
    //    xingCnt++;
    //    ui->lineEdit_2_pwd->setText(tmp);
    }
    else
    {
//        qDebug("#");
    }
}
*/
}
