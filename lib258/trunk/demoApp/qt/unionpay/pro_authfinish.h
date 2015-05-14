#ifndef PRO_AUTHFINISH_H
#define PRO_AUTHFINISH_H

#include <QDialog>
#include "mainentrythread.h"
namespace Ui {
class Pro_AuthFinish;
}

class Pro_AuthFinish : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_AuthFinish(QWidget *parent = 0);
    ~Pro_AuthFinish();
    
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
    Ui::Pro_AuthFinish *ui;
    QString pwdStr;
    unsigned char pwdCnt;
    unsigned char xingCnt;
    unsigned int pwdNum;
    QString amtStr;
    unsigned char amtCnt;
    double amtTmp;
    unsigned long amt;
};

#endif // PRO_AUTHFINISH_H
