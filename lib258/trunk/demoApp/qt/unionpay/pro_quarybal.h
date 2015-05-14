#ifndef PRO_QUARYBAL_H
#define PRO_QUARYBAL_H

#include <QDialog>

namespace Ui {
class Pro_QuaryBal;
}

class Pro_QuaryBal : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_QuaryBal(QWidget *parent = 0);
    ~Pro_QuaryBal();
    QTimer *sandtimer;
private slots:
    void slottimer1GetCardInfo();

    void on_cancelButton_clicked();

    void on_confirmButton_clicked();
    void destroyWindow(int);
    void sysEventSlot(int sysEvent) ;
protected:
    bool eventFilter(QObject *target, QEvent *event);
private:
    Ui::Pro_QuaryBal *ui;
    QString pwdStr;
    unsigned char pwdCnt;
    unsigned char xingCnt;
    unsigned int pwdNum;
};

#endif // PRO_QUARYBAL_H
