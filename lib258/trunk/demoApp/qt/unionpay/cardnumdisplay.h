#ifndef CARDNUMDISPLAY_H
#define CARDNUMDISPLAY_H
//#include <QObject>
#include <QWidget>
//#include <QVariant>
#include <QTimer>
#include <QShowEvent>
//#include <QKeyEvent>
#include <QDialog>
#include <QString>


namespace Ui {
class CardNumDisplay;
}

class CardNumDisplay : public QDialog
{
    Q_OBJECT

public:
    CardNumDisplay(QDialog *parent = 0);
    ~CardNumDisplay();
    char aucISO2DataBack[38];
    int grandFartherFlag;
    QTimer *time;
//    QVariant aucISO2Data;
protected:
    void showEvent(QShowEvent *event);
//    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *target, QEvent *event);
private:
    Ui::CardNumDisplay *ui;
    QString pwdStr;
    unsigned char pwdCnt;
    unsigned char xingCnt;
    unsigned int pwdNum;
    QString amtStr;
    unsigned char amtCnt;
    double amtTmp;
    unsigned long amt;
private slots:
    void on_Bt_Cancel_clicked();
    void on_Bt_Enter_clicked();
    void destroyWindow(int);
//    void on_lineEdit_2_pwd_textEdited(const QString &arg1);
    void timeSlot();

signals:
    void cardNumDisplayCancel();
};

#endif // CARDNUMDISPLAY_H
