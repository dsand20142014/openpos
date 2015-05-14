#ifndef MFCCARDTEST_H
#define MFCCARDTEST_H

#include <QTimer>
#include "baseForm.h"
#include <QThread>
#include <semaphore.h>


extern "C"{
#include "fsync_drvs.h"
}


class CardThread : public QThread
{
     Q_OBJECT
public:
    CardThread(int cardtype,QObject *parent = 0);

    void wakeUp();
    void changeFlag();
    NEW_DRV_TYPE  new_drv;

private:
    int cardType;
    int count;
    int runflagss;
    QWaitCondition waitCon;
    void run();

signals:
    void signalCard();
    void signalExit();
    void signalFresh();

};


class MFCCardTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit MFCCardTest(QWidget *parent = 0);
    ~MFCCardTest();
    
private slots:
    void getResult();
    void slotExit();
    void slotRemove();

private:
    QTextEdit *textEdit;
    CardThread *thread;
    QTimer timer;
    int cc;
//    void keyPressEvent(QKeyEvent *);

};

#endif // MFCCARDTEST_H
