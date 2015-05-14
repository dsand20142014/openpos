#include "mx25keyboardhandler.h"
#include "scancode.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <QDebug>

Mx25KeyboardHandler::Mx25KeyboardHandler(const QString &device,QObject *parent)
    :QObject(parent),QWSKeyboardHandler()
{
    setObjectName("MX25 4x5 Keyboard Handler");
    this->kbdFd = ::open(device.toAscii().constData(),O_RDONLY);
    if(this->kbdFd<0)
        return;
    this->notifier = new QSocketNotifier(this->kbdFd,QSocketNotifier::Read, this);
    connect(this->notifier,SIGNAL(activated(int)),this,SLOT(kbdReadyRead()));
}

Mx25KeyboardHandler::~Mx25KeyboardHandler()
{
    if(this->kbdFd>=0)
        ::close(this->kbdFd);
}

void Mx25KeyboardHandler::kbdReadyRead()
{
    struct input_event key;
    if(read(kbdFd, &key, sizeof(key))!=sizeof(key))
        return;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    int unicode=0;
    int keycode=0;

    switch(key.code)
    {
    case GKEY_0:
        keycode=Qt::Key_0;
        unicode='0';
        break;
    case GKEY_1:
        keycode=Qt::Key_1;
        unicode='1';
        break;
    case GKEY_2:
        keycode=Qt::Key_2;
        unicode='2';
        break;
    case GKEY_3:
        keycode=Qt::Key_3;
        unicode='3';
        break;
    case GKEY_4:
        keycode=Qt::Key_4;
        unicode='4';
        break;
    case GKEY_5:
        keycode=Qt::Key_5;
        unicode='5';
        break;
    case GKEY_6:
        keycode=Qt::Key_6;
        unicode='6';
        break;
    case GKEY_7:
        keycode=Qt::Key_7;
        unicode='7';
        break;
    case GKEY_8:
        keycode=Qt::Key_8;
        unicode='8';
        break;
    case GKEY_9:
        keycode=Qt::Key_9;
        unicode='9';
        break;
    case GKEY_F1:
        keycode=Qt::Key_F1;
        break;
    case GKEY_F2:
        keycode=Qt::Key_F2;
        break;
    case GKEY_F3:
        keycode=Qt::Key_F3;
        break;
    case GKEY_F4:
        keycode=Qt::Key_F4;
        break;
    case GKEY_CLEAR:
        keycode=Qt::Key_Backspace;
        break;
    case GKEY_CANCEL:
        keycode=Qt::Key_Escape;
        break;
    case GKEY_FEED:
        keycode=Qt::Key_F9;
        break;
    case GKEY_ENTER:
        keycode=Qt::Key_Enter;
        break;
    case GKEY_CAMERA:
        keycode=Qt::Key_Camera;
        break;
    case GKEY_00:
        keycode=Qt::Key_F12;
        break;
    case GKEY_POWER:
        keycode=Qt::Key_PowerOff;
        break;
    default:
        break;
    }
    processKeyEvent(unicode,keycode,modifiers,key.value,false);
}
