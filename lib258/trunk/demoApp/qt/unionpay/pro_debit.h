#ifndef PRO_DEBIT_H
#define PRO_DEBIT_H

#include <QDialog>
#include <QTimer>
#include "mainentrythread.h"

namespace Ui {
class Pro_Debit;
}

class Pro_Debit : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_Debit(QWidget *parent = 0);
    ~Pro_Debit();
    QTimer *sandtimer;
private slots:
    void slottimer1GetCardInfo();

    void on_confirmButton_clicked();
    void on_cancelButton_clicked();
    void destroyWindow(int);
    void on_line1_amt_textEdited(const QString &arg1);
    void sysEventSlot(int);
protected:
    bool eventFilter(QObject *target, QEvent *event);
private:
    Ui::Pro_Debit *ui;
    QString pwdStr;
    unsigned char pwdCnt;
    unsigned char xingCnt;
    unsigned int pwdNum;
    QString amtStr;
    unsigned char amtCnt;
    double amtTmp;
    unsigned long amt;
    unsigned char reswipeFlag;
//    unsigned char  OWE_GetIsoTrack(unsigned char *pucTrack);
//    unsigned int OWE_RstErrInfo(int ret);
};

#endif // PRO_DEBIT_H
