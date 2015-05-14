/*
#include <QThread>
class  MyThread :  public  QThread {
public :
	virtual   void  run();
};
*/

#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>

class MyThread : public QThread
{
     Q_OBJECT
public:
    MyThread(int value = 0);

protected:
   void run();

signals:
   void signal_mythread();
private:
   int flag;


};

#endif // MYTHREAD_H
