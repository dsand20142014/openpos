#ifndef PRINTERTEST_H
#define PRINTERTEST_H

#include <QtGui>
#include "baseForm.h"

class PrinterTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit PrinterTest(bool flag,QWidget *parent = 0);
    ~PrinterTest();
    
private slots:
    void getResult();
    void on_pushButton_3_clicked();
    void slotFlag();
    void slotExit();
    void slot_connect();

private:
    QLabel *label;
    QTimer timer;
    Button *btnOne;
    Button *btnMore;
    bool hotflags;
    bool moreFlag;
    bool beginFLag;

    void keyPressEvent(QKeyEvent *);

};

#endif // PRINTERTEST_H
