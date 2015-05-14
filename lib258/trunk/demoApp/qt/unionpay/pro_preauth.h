#ifndef PRO_PREAUTH_H
#define PRO_PREAUTH_H

#include <QDialog>
#include "mainentrythread.h"
namespace Ui {
class Pro_PreAuth;
}

class Pro_PreAuth : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_PreAuth(QWidget *parent = 0);
    ~Pro_PreAuth();
    
    QTimer *sandtimer;
private slots:
    void slottimer1GetCardInfo();
    void on_confirmButton_clicked();

    void on_cancelButton_clicked();
    void destroyWindow(int);
    void sysEventSlot(int);
protected:
    bool eventFilter(QObject *target, QEvent *event);
private:
    Ui::Pro_PreAuth *ui;
    QString pwdStr;
    unsigned char pwdCnt;
    unsigned char xingCnt;
    unsigned int pwdNum;
    QString amtStr;
    unsigned char amtCnt;
    double amtTmp;
    unsigned long amt;
};

#endif // PRO_PREAUTH_H
