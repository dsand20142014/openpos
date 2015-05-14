#ifndef MSGINFO_H
#define MSGINFO_H

#include <QWidget>
#include <QThread>
#include <QtGui>

namespace Ui {
class MsgInfo;
}

class MyThread : public QThread
{
     Q_OBJECT
public:
    MyThread(int flag,QObject *parent = 0);

public:
    void run();
    bool begFlag;

 private:
    int flags;

};

class MsgInfo : public QWidget
{
    Q_OBJECT
    
public:
    explicit MsgInfo(int flag,QWidget *parent = 0);
    ~MsgInfo();
    
private slots:
   // void on_pushButton_clicked();
    void slot_showMsg();
private:
    Ui::MsgInfo *ui;
     MyThread *thread;
     QTimer timer;
     int gb2312toutf8(char *sourcebuf, size_t sourcelen, char *destbuf, size_t destlen);

//     void keyPressEvent(QKeyEvent *);
};

#endif // MSGINFO_H
