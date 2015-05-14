#ifndef PRO_CANCELAUTHFINISH_H
#define PRO_CANCELAUTHFINISH_H

#include <QDialog>
#include "mainentrythread.h"
namespace Ui {
class Pro_CancelAuthFinish;
}

class Pro_CancelAuthFinish : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_CancelAuthFinish(QWidget *parent = 0);
    ~Pro_CancelAuthFinish();
    
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
    Ui::Pro_CancelAuthFinish *ui;
    QString pwdStr;
    unsigned char pwdCnt;
    unsigned char xingCnt;
    unsigned int pwdNum;
    unsigned char ucTransOkFlag;
    int gb2312toutf8(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);
};

#endif // PRO_CANCELAUTHFINISH_H
