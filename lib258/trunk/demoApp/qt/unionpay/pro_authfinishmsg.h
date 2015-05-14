#ifndef PRO_AUTHFINISHMSG_H
#define PRO_AUTHFINISHMSG_H

#include <QDialog>
#include "mainentrythread.h"
namespace Ui {
class Pro_AuthFinishMsg;
}

class Pro_AuthFinishMsg : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_AuthFinishMsg(QWidget *parent = 0);
    ~Pro_AuthFinishMsg();
    QTimer *sandtimer;
private slots:
    void slottimer1GetCardInfo();
    void on_cancelButton_clicked();

    void on_confirmButton_clicked();
    void sysEventSlot(int);
private:
    Ui::Pro_AuthFinishMsg *ui;
};

#endif // PRO_AUTHFINISHMSG_H
