#ifndef PRO_CANCEL_H
#define PRO_CANCEL_H

#include <QDialog>
#include "mythread.h"
#include "mainentrythread.h"
namespace Ui {
class Pro_Cancel;
}

class Pro_Cancel : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_Cancel(QWidget *parent = 0);
    ~Pro_Cancel();
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
    Ui::Pro_Cancel *ui;
    int gb2312toutf8(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);
    unsigned char ucTransOkFlag;
    QString pwdStr;
    unsigned char pwdCnt;
    unsigned char xingCnt;
    unsigned int pwdNum;
};

#endif // PRO_CANCEL_H
