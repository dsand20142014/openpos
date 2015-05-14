#ifndef AUTOLCDTEST_H
#define AUTOLCDTEST_H

#include <QWidget>
#include <QTimer>
#include "baseForm.h"

class AutoLcdTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit AutoLcdTest(QWidget *parent = 0);
    ~AutoLcdTest();

private slots:
    void refresh();

private:
    QTimer timer;
    int num;
    QPalette palette;
};

#endif // AUTOLCDTEST_H
