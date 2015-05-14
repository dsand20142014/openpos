#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "qimeframe.h"
#include "qinputwidget.h"
#include "pinyinime.h"
#include "pinyindialog.h"

using namespace ime_pinyin;

IMFrame::IMFrame()
{
    inputwidget = new InputWidget(this);
    inputwidget->setGeometry(200,250,30,20);

    pinyinDialog=new PinyinDialog(this);
    pinyinDialog->setGeometry(0,200,240,50);

    abcTimer = new QTimer(this);

    setImeType(NUM);
}

IMFrame::~IMFrame()
{
}

void IMFrame::setImeType(ImeType t)
{
        content.clear();
        imeType=t;
        emit imeTypeChanged(t);
}

void IMFrame::sendPreString(const QString & str)
{
    sendPreeditString(str,0);
}

void IMFrame::sendComString(const QString & str)
{
    sendCommitString(str);
}

void IMFrame::changeImeType()
{
    content.clear();
    switch(imeType)
    {
    case NUM:
        imeType=ABC;
        break;
    case ABC:
        imeType=CAPABC;
        break;
    case CAPABC:
        imeType=PINYIN;
        break;
    case PINYIN:
        imeType=NUM;
        break;
    default:
        imeType=NUM;
        break;
    }
   emit imeTypeChanged(imeType);
}

void IMFrame::sendContent(const QString& newcontent)
{
    content += newcontent;
    sendPreeditString(content,0);
}

void IMFrame::confirmContent()
{
    sendCommitString(content);
    content.clear();
}

void IMFrame::updateHandler(int type)
{
    switch(type)
    {
    case QWSInputMethod::FocusOut:
        setImeType(NUM);
        content.clear();
        inputwidget->hide();
        if(!pinyinDialog->isHidden())
        {
            pinyinDialog->hide();
        }
        break;
    case QWSInputMethod::FocusIn:
        inputwidget->show();
        break;
    default:
        break;
    }
}

bool IMFrame::filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat)
{
    if(inputwidget->isHidden())
    {
        return false;
    }
    else
    {
        switch(imeType)
        {
        case NUM:
            if(keycode == Qt::Key_F12||keycode==Qt::Key_F3||keycode==Qt::Key_F4||keycode==Qt::Key_Enter)
                return inputPinyin(unicode,keycode,modifiers,isPress,autoRepeat);
            else{
                pinyinDialog->hide();
                return inputNum(unicode,keycode,modifiers,isPress,autoRepeat);
            }
        case ABC:
            if(keycode == Qt::Key_F12||keycode==Qt::Key_F3||keycode==Qt::Key_F4||keycode==Qt::Key_Enter)
                return inputPinyin(unicode,keycode,modifiers,isPress,autoRepeat);
            else{
                pinyinDialog->hide();
                return inputAbc(unicode,keycode,modifiers,isPress,autoRepeat);
            }
        case CAPABC:
            if(keycode == Qt::Key_F12||keycode==Qt::Key_F3||keycode==Qt::Key_F4||keycode==Qt::Key_Enter)
                return inputPinyin(unicode,keycode,modifiers,isPress,autoRepeat);
            else{
                pinyinDialog->hide();
                return inputCapAbc(unicode,keycode,modifiers,isPress,autoRepeat);
            }
        case PINYIN:
            return inputPinyin(unicode,keycode,modifiers,isPress,autoRepeat);
            break;
        default:
            return false;
        }
    }
}

bool IMFrame::inputNum(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat)
{
    (void)unicode;
    (void)modifiers;
    (void)autoRepeat;
    if(isPress)
    {
        switch(keycode)
        {
        case Qt::Key_F12:
             return inputPinyin(unicode,keycode,modifiers,isPress,autoRepeat);
          //  sendCommitString("00");
           // return true;
        }
        return false;
    }
    return false;
}

bool IMFrame::inputAbc(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat)
{
    (void)unicode;
    (void)modifiers;
    (void)autoRepeat;
    static int pushNum;
    static int preKeyCode;

    if(isPress)
    {
        if(preKeyCode!=keycode)
        {
            abcTimer->stop();
            confirmContent();
            preKeyCode=keycode;
        }
        if(!abcTimer->isActive())
        {
            abcTimer->setInterval(1000);
            abcTimer->start();
            abcTimer->setSingleShot(true);
            connect(abcTimer,SIGNAL(timeout()),this,SLOT(confirmContent()));
            pushNum=0;
        }
        else
        {
            abcTimer->setInterval(1000);
            pushNum++;
        }
        switch(keycode)
        {
        case Qt::Key_0:
            switch(pushNum%3)
            {
            case 0:
                content=" ";
                break;
            case 1:
                content="*";
                break;
            case 2:
                content="0";
                break;
            }
            break;
        case Qt::Key_1:
            switch(pushNum%3)
            {
            case 0:
                content="q";
                break;
            case 1:
                content="z";
                break;
            case 2:
                content="1";
                break;
            }
            break;
        case Qt::Key_2:
            switch(pushNum%4)
            {
            case 0:
                content="a";
                break;
            case 1:
                content="b";
                break;
            case 2:
                content="c";
                break;
            case 3:
                content="2";
                break;
            }
            break;
        case Qt::Key_3:
            switch(pushNum%4)
            {
            case 0:
                content="d";
                break;
            case 1:
                content="e";
                break;
            case 2:
                content="f";
                break;
            case 3:
                content="3";
                break;
            }
            break;
        case Qt::Key_4:
            switch(pushNum%4)
            {
            case 0:
                content="g";
                break;
            case 1:
                content="h";
                break;
            case 2:
                content="i";
                break;
            case 3:
                content="4";
                break;
            }
            break;
        case Qt::Key_5:
            switch(pushNum%4)
            {
            case 0:
                content="j";
                break;
            case 1:
                content="k";
                break;
            case 2:
                content="l";
                break;
            case 3:
                content="5";
                break;
            }
            break;
        case Qt::Key_6:
            switch(pushNum%4)
            {
            case 0:
                content="m";
                break;
            case 1:
                content="n";
                break;
            case 2:
                content="o";
                break;
            case 3:
                content="6";
                break;
            }
            break;
        case Qt::Key_7:
            switch(pushNum%4)
            {
            case 0:
                content="p";
                break;
            case 1:
                content="r";
                break;
            case 2:
                content="s";
                break;
            case 3:
                content="7";
                break;
            }
            break;
        case Qt::Key_8:
            switch(pushNum%4)
            {
            case 0:
                content="t";
                break;
            case 1:
                content="u";
                break;
            case 2:
                content="v";
                break;
            case 3:
                content="8";
                break;
            }
            break;
        case Qt::Key_9:
            switch(pushNum%4)
            {
            case 0:
                content="w";
                break;
            case 1:
                content="x";
                break;
            case 2:
                content="y";
                break;
            case 3:
                content="9";
                break;
            }
            break;
        case Qt::Key_F12://00
            switch(pushNum%3)
            {
            case 0:
                content=".";
                break;
            case 1:
                content=",";
                break;
            case 2:
                content="00";
                break;
            }
            break;
        case Qt::Key_Enter:
            abcTimer->stop();
            confirmContent();
            break;
        default:
            abcTimer->stop();
            return false;
            break;
        }
        sendPreeditString(content,0);
        return true;
    }
    else
    {
        return false;
    }
}

bool IMFrame::inputCapAbc(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat)
{
    (void) unicode;
    (void) modifiers;
    (void) autoRepeat;
    static int pushNum;
    static int preKeyCode;

    if(isPress)
    {
        if(preKeyCode!=keycode)
        {
            abcTimer->stop();
            confirmContent();
            preKeyCode=keycode;
        }
        if(!abcTimer->isActive())
        {
            abcTimer->setInterval(1000);
            abcTimer->start();
            abcTimer->setSingleShot(true);
            connect(abcTimer,SIGNAL(timeout()),this,SLOT(confirmContent()));
            pushNum=0;
        }
        else
        {
            abcTimer->setInterval(1000);
            pushNum++;
        }
        switch(keycode)
        {
        case Qt::Key_0:
            switch(pushNum%3)
            {
            case 0:
                content=" ";
                break;
            case 1:
                content="*";
                break;
            case 2:
                content="0";
                break;
            }
            break;
        case Qt::Key_1:
            switch(pushNum%3)
            {
            case 0:
                content="Q";
                break;
            case 1:
                content="Z";
                break;
            case 2:
                content="1";
                break;
            }
            break;
        case Qt::Key_2:
            switch(pushNum%4)
            {
            case 0:
                content="A";
                break;
            case 1:
                content="B";
                break;
            case 2:
                content="C";
                break;
            case 3:
                content="2";
                break;
            }
            break;
        case Qt::Key_3:
            switch(pushNum%4)
            {
            case 0:
                content="D";
                break;
            case 1:
                content="E";
                break;
            case 2:
                content="F";
                break;
            case 3:
                content="3";
                break;
            }
            break;
        case Qt::Key_4:
            switch(pushNum%4)
            {
            case 0:
                content="G";
                break;
            case 1:
                content="H";
                break;
            case 2:
                content="I";
                break;
            case 3:
                content="4";
                break;
            }
            break;
        case Qt::Key_5:
            switch(pushNum%4)
            {
            case 0:
                content="J";
                break;
            case 1:
                content="K";
                break;
            case 2:
                content="L";
                break;
            case 3:
                content="5";
                break;
            }
            break;
        case Qt::Key_6:
            switch(pushNum%4)
            {
            case 0:
                content="M";
                break;
            case 1:
                content="N";
                break;
            case 2:
                content="O";
                break;
            case 3:
                content="6";
                break;
            }
            break;
        case Qt::Key_7:
            switch(pushNum%4)
            {
            case 0:
                content="P";
                break;
            case 1:
                content="R";
                break;
            case 2:
                content="S";
                break;
            case 3:
                content="7";
                break;
            }
            break;
        case Qt::Key_8:
            switch(pushNum%4)
            {
            case 0:
                content="T";
                break;
            case 1:
                content="U";
                break;
            case 2:
                content="V";
                break;
            case 3:
                content="8";
                break;
            }
            break;
        case Qt::Key_9:
            switch(pushNum%4)
            {
            case 0:
                content="W";
                break;
            case 1:
                content="X";
                break;
            case 2:
                content="Y";
                break;
            case 3:
                content="9";
                break;
            }
            break;
        case Qt::Key_F12://00
            switch(pushNum%3)
            {
            case 0:
                content=".";
                break;
            case 1:
                content=",";
                break;
            case 2:
                content="00";
                break;
            }
            break;
        case Qt::Key_Enter:
            abcTimer->stop();
            confirmContent();
            break;
        default:
            abcTimer->stop();
            return false;
            break;
        }
        sendPreeditString(content,0);
        return true;
    }
    else
    {
        return false;
    }
}

bool IMFrame::inputPinyin(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat)
{
    return pinyinDialog->keyPush(unicode, keycode, modifiers, isPress, autoRepeat);
}
