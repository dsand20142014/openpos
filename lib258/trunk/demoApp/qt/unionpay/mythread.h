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
   void run();
private:

//public:
 //   void run();


};

#endif // MYTHREAD_H
