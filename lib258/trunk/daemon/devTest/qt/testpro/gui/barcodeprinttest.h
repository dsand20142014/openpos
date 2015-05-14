#ifndef BARCODEPRINTTEST_H
#define BARCODEPRINTTEST_H

#include <QWidget>
#include <QTimer>
#include "baseForm.h"


class BarcodePrintTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit BarcodePrintTest(QWidget *parent = 0);
    ~BarcodePrintTest();
    
private slots:
    void getResult();

private:
    QTimer timer;
    QTextEdit *textEdit;
    QLabel *label;
};

#endif // BARCODEPRINTTEST_H
