#ifndef MAINMENUDIALOG_H
#define MAINMENUDIALOG_H

#include <QDialog>
#include <QColor>
#include "mythread.h"

namespace Ui {
class MainMenuDialog;
}

class MainMenuDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainMenuDialog(QWidget *parent = 0);
    ~MainMenuDialog();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    signals:
    void signal();

private:
    Ui::MainMenuDialog *ui;
    MyThread *myThread;

};

#endif // MAINMENUDIALOG_H
