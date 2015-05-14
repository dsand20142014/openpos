#include "mainentrythread.h"
#include "new_drv.h"
#include "osevent.h"
#include "new_drv.h"
#include "sand_main.h"

MainEntryThread::MainEntryThread(int c, char **v, QObject *parent)
{
    argc=c;
    argv=v;
}

void MainEntryThread::run()
{
    struct seven   *appevent;
    qt_main_entry_init(argc, argv);
    while(1)
    {
        appevent = (struct seven *)qt_main_entry_readmsg();
        mutex.lock();
        emit sysEventSignal((int)appevent);
        bufferNotDel.wait(&mutex);
        mutex.unlock();
        qt_main_entry_writemsg();
    }
}

void MainEntryThread::wakeUp()
{
    mutex.lock();
    bufferNotDel.wakeAll();
    mutex.unlock();
}
