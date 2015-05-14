#ifndef ENDLESSLOOPTEST_H
#define ENDLESSLOOPTEST_H

#include <QWidget>
#include "qtimer.h"
#include <QKeyEvent>

namespace Ui {
class EndlessLoopTest;
}

class EndlessLoopTest : public QWidget
{
    Q_OBJECT
    
public:
    explicit EndlessLoopTest(QWidget *parent = 0);
    ~EndlessLoopTest();
    
private slots:
    void on_pushButton_clicked();


private:
    Ui::EndlessLoopTest *ui;
    void getResult();
   // void keyPressEvent(QKeyEvent *);
   // QTimer timer;
  //  bool flag;
   // unsigned char printer_test_step ;

};

#endif // ENDLESSLOOPTEST_H
