#ifndef GSMDETAILTEST_H
#define GSMDETAILTEST_H

#include <QtGui>
#include "baseForm.h"



class GsmDetailTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit GsmDetailTest(QWidget *parent = 0);
    ~GsmDetailTest();

private slots:
    void slot1();
    
private:
    QScrollArea* myscroll;
//    QWidget *widget;
    QTimer timer;
    //  bool flag;
      QProgressBar *bar;


};

#endif // GSMDETAILTEST_H
