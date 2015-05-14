#ifndef GSMCALLTEST_H
#define GSMCALLTEST_H

//#include <QWidget>
#include <QTimer>
#include "baseForm.h"

class GSMCallTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit GSMCallTest(QWidget *parent = 0);
    ~GSMCallTest();
    
private slots:
    void slot_timer();
    void getResult();

private:
    QTextEdit *textEdit;
    QTimer timer;
    int count;
};

#endif // GSMCALLTEST_H
