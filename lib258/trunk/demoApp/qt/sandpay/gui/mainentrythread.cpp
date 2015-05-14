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
        qDebug()<<"~~~~~000000000~~~~~~~~~~~~~~~run**************"<<endl;
        appevent = (struct seven *)qt_main_entry_readmsg();
        qDebug()<<"~~~~~1111111~~~~~~~~~~~~~~~run**************"<<endl;
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
    qDebug()<<"~~~~~000000000~~~~~~~~~~~~~~~wakeup***************"<<endl;
    bufferNotDel.wakeAll();qDebug()<<"~~11111111111111~~~~~~~~~~~~~~~~~~wakeup***************"<<endl;
    mutex.unlock();
}
