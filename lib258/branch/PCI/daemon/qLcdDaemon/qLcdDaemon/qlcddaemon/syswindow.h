#ifndef SYSWINDOW_H
#define SYSWINDOW_H

#include <QtGui>
#include "gui/secondmenu.h"

class IButton : public QPushButton
{
public:
    explicit IButton(QString icon, QWidget *parent = 0);

};

class About : public BaseForm
{
    Q_OBJECT
public:
    explicit About(QWidget *parent = 0);
    ~About();
    QTextEdit *textedit;

    void keyPressEvent(QKeyEvent *e);
	void mousePressEvent(QMouseEvent*);
};

class SysWindow : public QWidget//QMainWindow
{
    Q_OBJECT
    
public:
    explicit SysWindow(QWidget *parent = 0, QStringList *argv = NULL);
    ~SysWindow();
    
private slots:

    void on_pushButton_enter_clicked();
#if 0
    void on_pushButton_reset_clicked();
#endif
    void on_pushButton_setting_clicked();
    void on_pushButton_test_clicked();
    void on_pushButton_download_clicked();
    void on_pushButton_about_clicked();

    void keyPressEvent(QKeyEvent *e);
    void slot_process_start();
    void slot_process_finish(int);
    //void slot_process_change(QProcess::ProcessState);

private:
    SecondMenu *smt;
    Button *pushButton_enter;
    Button *pushButton_reset;
    QLabel *label;
    QLineEdit *lineEdit;
    QWidget *btnwin;
    About *aboutWin;
    QLabel *giflabel;
    QMovie *loading;
    QProcess *myProcess;
    QStringList *sysArgv;
    //QTimer timerGif;

    void enableMyself(bool);
};

#endif // SYSWINDOW_H
