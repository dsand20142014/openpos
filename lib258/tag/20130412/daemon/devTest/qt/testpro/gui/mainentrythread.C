#include "mainentrythread.h"
#include <QDebug>



extern "C"{
#include "fsync_drvs.h"
#include "osevent.h"
#include "sand_main.h"
#include "sand_debug.h"
}
extern void _print(char *fmt,...);

MainEntryThread::MainEntryThread(int c, char **v, QObject *parent)
{
    argc=c;
    argv=v;
}

void MainEntryThread::run()
{
    struct seven   *appevent;
   _print("~~~~~~~~~~~~~a\n");
    qt_main_entry_init(argc, argv);
   _print("~~~~~~~~~~~~~b\n");

    while(1)
    {
        _print("~~~~~~~~~~~~~c\n");

        appevent = (struct seven *)qt_main_entry_readmsg();
       _print("~~~~~~~~~~~~~d\n");

        mutex.lock();


        //qDebug()<<"emit syseventsignal********";

        emit sysEventSignal((int)appevent);

        //qDebug()<<"wait syseventsignal********";

        bufferNotDel.wait(&mutex);

        //qDebug()<<"end qt_main_entry_writemsg********";

        mutex.unlock();


        qt_main_entry_writemsg();

    }
}

void MainEntryThread::wakeUp()
{
   _print("~~~~~~~~~~~~~~~~~~~~~wakeup**********\n");
    mutex.lock();
    bufferNotDel.wakeAll();
    mutex.unlock();
   _print("~~~~~~~~~~~~~~~~~~~~~end wakeup**********\n");

}


