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
#include <QTimer>
class MyThread : public QThread
{
     Q_OBJECT
public:
    MyThread(int value = 0);
    ~MyThread();
protected:
   void run();

 private slots:
   void slot_checkValue();
signals:
   void signal_mythread();
   void value_Change(uchar * );
private:
   int flag;
   QTimer timer;
   unsigned char tempValue[64];

};

#endif // MYTHREAD_H
