#ifndef OFFTRANSUI_H
#define OFFTRANSUI_H

#include <QDialog>
#include <QtGui>
#include "msginfoui.h"
#include "mythread.h"

namespace Ui {
class OffTransUI;
}

class OffTransUI : public QDialog
{
    Q_OBJECT
    
public:
    explicit OffTransUI(QWidget *parent = 0);
    ~OffTransUI();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void destroy_ui();


private:
    Ui::OffTransUI *ui;
    long account;
    void getCardBalance();
    void setkey(double);
    MyThread *myThread;
    MsgInfoUI *desk;
    bool eventFilter(QObject *target, QEvent *event);
};

#endif // OFFTRANSUI_H
