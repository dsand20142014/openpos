#ifndef PRO_REFUND_H
#define PRO_REFUND_H

#include <QDialog>
#include "mainentrythread.h"
namespace Ui {
class Pro_Refund;
}

class Pro_Refund : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_Refund(QWidget *parent = 0);
    ~Pro_Refund();
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
    Ui::Pro_Refund *ui;
    QString amtStr;
    unsigned char amtCnt;
    double amtTmp;
    unsigned long amt;
};

#endif // PRO_REFUND_H
