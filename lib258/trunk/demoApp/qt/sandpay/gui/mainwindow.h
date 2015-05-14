#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>

#include "mainentrythread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(int argc, char *argv[],QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void sysEventSlot(int);

    void on_pushButton_slots();

private:
    Ui::MainWindow *ui;
    MainEntryThread *mainEntryThread;
};


#endif // MAINWINDOW_H
