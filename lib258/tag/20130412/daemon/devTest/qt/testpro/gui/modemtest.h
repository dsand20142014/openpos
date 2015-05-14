#ifndef MODEMTEST_H
#define MODEMTEST_H

//#include <QWidget>
#include "baseForm.h"


class ModemTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit ModemTest(QWidget *parent = 0);
    ~ModemTest();
    
private slots:
    void getResult();

private:

    QTextEdit *textEdit;
};

#endif // MODEMTEST_H
