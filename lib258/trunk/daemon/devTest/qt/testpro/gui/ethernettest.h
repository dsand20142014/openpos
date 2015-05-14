#ifndef ETHERNETTEST_H
#define ETHERNETTEST_H

//#include <QWidget>
#include <QTimer>
#include "baseForm.h"


class EthernetTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit EthernetTest(QWidget *parent = 0);
    ~EthernetTest();


private:
    QTextEdit *textEdit;


};

#endif // ETHERNETTEST_H
