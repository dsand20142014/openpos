#ifndef MAINENTRYTHREAD_H
#define MAINENTRYTHREAD_H

#include <QThread>
#include <QMutexLocker>
#include <QWaitCondition>

class MainEntryThread : public QThread
{
    Q_OBJECT
public:
    MainEntryThread(int c, char **v,QObject *parent = 0);
   // ~MainEntryThread();
private:
    int argc;
    char **argv;
    QMutex mutex;
    QWaitCondition bufferNotDel;

protected:
    void run();

signals:
    void sysEventSignal(int);
public slots:
    void wakeUp(void);
};

#endif // MAINENTRYTHREAD_H
