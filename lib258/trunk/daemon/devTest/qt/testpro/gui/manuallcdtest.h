#ifndef MANUALLCDTEST_H
#define MANUALLCDTEST_H

#include <QWidget>
#include <QKeyEvent>
#include "baseForm.h"
#if 0
namespace Ui {
class ManualLcdTest;
}
#endif
class ManualLcdTest : public BaseForm//QWidget
{
    Q_OBJECT
    
public:
    explicit ManualLcdTest(QWidget *parent = 0);
    ~ManualLcdTest();
    
private slots:
    void on_btnOk_clicked();

private:
    int num;
    QPalette palette;

//protected:
//    void keyPressEvent(QKeyEvent *);
};

#endif // MANUALLCDTEST_H
