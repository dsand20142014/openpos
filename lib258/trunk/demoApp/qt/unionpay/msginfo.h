#ifndef MSGINFO_H
#define MSGINFO_H

#include <QDialog>
#include <QProgressBar>
#include <QGridLayout>
#include <QTimer>
#include "mythread.h"
namespace Ui {
class MsgInfo;
}

class MsgInfo : public QDialog
{
    Q_OBJECT
    
public:
    explicit MsgInfo(QWidget *parent = 0);
    ~MsgInfo();
   int grandFartherFlag;
    QTimer *sandtimer;
    QTimer *sandtimer2;
    
private slots:
    void on_pushButton_clicked();
	void slottimer1Done();   
    void slottimer2Done();
private:
    Ui::MsgInfo *ui;    	    
   	QProgressBar *progressBar;
    unsigned char timeOutCnt;
    int gb2312toutf8(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);
    //QGridLayout *mainLayout;
    MyThread *myThread;
signals:
    void returnGrandFartherMenu(int grandFartherFlag);
    void returnFromRefund();
};

#endif // MSGINFO_H
