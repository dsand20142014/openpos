#ifndef CARD4442TEST_H
#define CARD4442TEST_H

#include <QWidget>
#include <QTimer>

#include "baseForm.h"
#include "mfccardtest.h"

class Card4442Test : public BaseForm
{
    Q_OBJECT
    
public:
    explicit Card4442Test(QWidget *parent = 0);
    ~Card4442Test();
    
private slots:
    void getResult();
    void slotExit();
    void slotRemove();
private:
    CardThread *thread;
    QTextEdit *textEdit;
//    void keyPressEvent(QKeyEvent *);

};

#endif // CARD4442TEST_H
