#ifndef TFTEST_H
#define TFTEST_H

#include <QtGui>
#include "baseForm.h"

class TfTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit TfTest(int flag,QWidget *parent = 0);
    ~TfTest();

private slots:
    void getResult();

private:
    int flags;
    QTextEdit *textEdit;
//    void keyPressEvent(QKeyEvent *);
};

#endif // TFTEST_H
