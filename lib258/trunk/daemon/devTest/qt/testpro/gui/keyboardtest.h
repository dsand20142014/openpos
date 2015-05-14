#ifndef KEYBOARDTEST_H
#define KEYBOARDTEST_H

#include <QKeyEvent>
#include <QtGui>
#include "baseForm.h"

class KeyBoardTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit KeyBoardTest(QWidget *parent = 0);
    ~KeyBoardTest();
    
private:
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    int preKey;
    int count;

protected:
    void keyPressEvent(QKeyEvent *);

};

#endif // KEYBOARDTEST_H
