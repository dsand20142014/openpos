#ifndef MAINKEY1_H
#define MAINKEY1_H

#include <QDialog>

#include "mythread.h"

namespace Ui {
class MainKey1;
}

class MainKey1 : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainKey1(QWidget *parent = 0);
    ~MainKey1();
    
private slots:
    void on_pushButton_5_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainKey1 *ui;
    MyThread *myThread;

};

#endif // MAINKEY1_H
