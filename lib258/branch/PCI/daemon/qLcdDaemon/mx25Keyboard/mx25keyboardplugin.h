#ifndef MX25KEYBOARDPLUGIN_H
#define MX25KEYBOARDPLUGIN_H

#include <QtGui/QKbdDriverPlugin>

class Mx25KeyboardPlugin : public QKbdDriverPlugin
{
public:
    Mx25KeyboardPlugin();
    QWSKeyboardHandler *create(const QString &driver, const
                               QString &device = QString("/dev/input/keyboard0"));
    QStringList keys() const;
};

#endif // MX25KEYBOARDPLUGIN_H
