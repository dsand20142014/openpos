#ifndef MAINKEY5_H
#define MAINKEY5_H

#include <QDialog>
#include "mythread.h"

namespace Ui {
class MainKey5;
}

class MainKey5 : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainKey5(QWidget *parent = 0);
    ~MainKey5();
    
private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::MainKey5 *ui;
    MyThread *myThread;
};

#endif // MAINKEY5_H
