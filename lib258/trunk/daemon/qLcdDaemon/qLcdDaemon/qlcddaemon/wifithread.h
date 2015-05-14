#ifndef WIFITHREAD_H
#define WIFITHREAD_H

#include <QThread>


class WifiThread : public QThread
{
    Q_OBJECT
public:
    void run();

signals:
    void disableAll(bool);

};

#endif // WIFITHREAD_H
