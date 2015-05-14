#ifndef KEYTEST_H
#define KEYTEST_H

#include "baseForm.h"
#include "button.h"

//namespace Ui {
//class KeyTest;
//}

class KeyTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit KeyTest(QWidget *parent = 0);
    ~KeyTest();
    
private:
    Button *btndes;
    Button *btn3des;

    void keyPressEvent(QKeyEvent *);

private slots:
    void btndes_slot();
    void btn3des_slot();


};

#endif // KEYTEST_H
