#ifndef PRO_CANCELAUTH_H
#define PRO_CANCELAUTH_H

#include <QDialog>
#include "mainentrythread.h"
namespace Ui {
class Pro_CancelAuth;
}

class Pro_CancelAuth : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_CancelAuth(QWidget *parent = 0);
    ~Pro_CancelAuth();
    
    QTimer *sandtimer;

private slots:
    void slottimer1GetCardInfo();
    void on_cancelButton_clicked();
    void destroyWindow(int);
    void on_confirmButton_clicked();
    void sysEventSlot(int);
protected:
    bool eventFilter(QObject *target, QEvent *event);
private:
    Ui::Pro_CancelAuth *ui;
    QString pwdStr;
    unsigned char pwdCnt;
    unsigned char xingCnt;
    unsigned int pwdNum;
    QString amtStr;
    unsigned char amtCnt;
    double amtTmp;
    unsigned long amt;
};

#endif // PRO_CANCELAUTH_H
