#ifndef MAINKEY7_H
#define MAINKEY7_H

#include <QDialog>
#include "mythread.h"
namespace Ui {
class MainKey7;
}

class MainKey7 : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainKey7(QWidget *parent = 0);
    ~MainKey7();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainKey7 *ui;
    MyThread *myThread;
};

#endif // MAINKEY7_H
