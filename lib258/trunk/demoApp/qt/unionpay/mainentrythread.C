#include "mainentrythread.h"
#include "new_drv.h"
#include "osevent.h"
#include "new_drv.h"
#include "sand_main.h"

MainEntryThread::MainEntryThread(int c, char **v, QObject *parent)
{
    argc=c;
    argv=v;
    qDebug("entry to mainentrythread");
}

void MainEntryThread::run()
{
    qDebug("enry to mainentry run");
    struct seven   *appevent;
        qDebug("enry to mainentry INIT");
    qt_main_entry_init(argc, argv);
    qDebug("maintry is ready to while");
    while(1)
    {
        qDebug("------------------------------------wait msg");
        appevent = (struct seven *)qt_main_entry_readmsg();
        qDebug("------------------------------------ msg arrive");
        mutex.lock();
        emit sysEventSignal((int)appevent);
        bufferNotDel.wait(&mutex);
        mutex.unlock();
        qt_main_entry_writemsg();
//        sleep(5);
    }
}

void MainEntryThread::wakeUp()
{
    mutex.lock();
    bufferNotDel.wakeAll();
    mutex.unlock();
}
