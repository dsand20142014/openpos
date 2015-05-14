#ifndef GSMTESTDIAL_H
#define GSMTESTDIAL_H

#include <QWidget>
#include "baseForm.h"


class GsmTestDial : public BaseForm
{
    Q_OBJECT
    
public:
    explicit GsmTestDial(QWidget *parent = 0);
    ~GsmTestDial();
    
private:
    QTextEdit *textEdit;
    QLineEdit *lineEdit;
    unsigned char buf[30];

private slots:
    void getResult();
    void showInfo();
};

#endif // GSMTESTDIAL_H
