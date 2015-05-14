#ifndef WIFITHREAD_H
#define WIFITHREAD_H

#include <QThread>

#define KEY_SYS_SET     0x52

class WifiThread : public QThread
{
    Q_OBJECT
public:
    void run();

signals:
    void disableAll(bool);

};

#endif // WIFITHREAD_H
