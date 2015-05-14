#ifndef ASYNCCARDTEST_H
#define ASYNCCARDTEST_H

#include <QTimer>
#include "baseForm.h"
#include "gui/mfccardtest.h"



class AsyncCardTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit AsyncCardTest(QWidget *parent = 0);
    ~AsyncCardTest();
    
private slots:
    void getResult();
    void slotExit();
    void slotRemove();

private:
    QTextEdit *textEdit;
    CardThread *threadss;

//    void keyPressEvent(QKeyEvent *);

};

#endif // ASYNCCARDTEST_H
