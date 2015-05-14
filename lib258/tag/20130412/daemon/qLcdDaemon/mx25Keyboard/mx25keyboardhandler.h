#ifndef MX25KEYBOARD_H
#define MX25KEYBOARD_H

#include <QObject>
#include <QtGui/QWSKeyboardHandler>
#include <QSocketNotifier>

#include "mx25Keyboardhandler_global.h"

class MX25KEYBOARDSHARED_EXPORT Mx25KeyboardHandler
        :public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    Mx25KeyboardHandler(const QString &device = QString("/dev/input/keyboard0"),QObject *parent = NULL);
    ~Mx25KeyboardHandler();
private:
    int kbdFd;
    QSocketNotifier *notifier;
private slots:
    void kbdReadyRead();
};

#endif // MX25KEYBOARD_H
