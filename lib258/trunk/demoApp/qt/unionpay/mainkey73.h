#ifndef MAINKEY73_H
#define MAINKEY73_H

#include <QDialog>
#include "mythread.h"

namespace Ui {
class MainKey73;
}

class MainKey73 : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainKey73(QWidget *parent = 0);
    ~MainKey73();
    QTimer *sandtimer;
private slots:
    void slottimer1Done();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::MainKey73 *ui;
    int gb2312toutf8(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);
    MyThread *myThread;
    MyThread *myThread1;
    MyThread *myThread2;
    MyThread *myThread3;
    unsigned char myThreadFlag;
};

#endif // MAINKEY73_H
