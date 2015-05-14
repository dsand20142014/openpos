#ifndef SMT_FOR_CUSTOMER_H
#define SMT_FOR_CUSTOMER_H

#include <QMainWindow>
#include <QTimer>
#include "tools.h"

#define     SMT_VERSION     "SMT For Customer 1.0.2"
/*****************************************************************
  SMT For Customer 1.0.2 update at 2013.03.18 by garvey
  1.修改获取dwn文件cpu类型时如果cpu类型为零,则默认为1

  SMT For Customer 1.0.1 update at 2013.03.01 by garvey
  1.修复中文乱码
  2.默认情况下,dwn文件为cpu类型为ARM7格式

  SMT For Customer 1.0.0 update at 2013.02.28 by garvey
  1.为客户提供应用下载功能

*****************************************************************/

namespace Ui {
class SMT_For_Customer;
}

class SMT_For_Customer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit SMT_For_Customer(QWidget *parent = 0);
    ~SMT_For_Customer();

private slots:
    //读取串口中的信息
    void onReadyRead();
    //打开串口或网络
    void on_actionConnect_triggered();
    //断开串口或网络
    void on_actionDisconnect_triggered();
    //软件版本
    void on_actionVersion_triggered();
    //退出软件
    void on_actionExit_triggered();


    //同步下载
    void on_actionSync_triggered();


    void on_pushButtonAppPath_clicked();

    void on_comboBoxCom_currentIndexChanged(const QString &arg1);

private:
    void clear(void);


private:
    Ui::SMT_For_Customer *ui;
    Tools tools;
    QLabel *msgPartL;
    QLabel *msgTty;
    QLabel *msgLable;
    QLabel *msgSpace;
    QLabel *msgPos;
    QLabel *msgPosType;
    QLabel *msgDis;
    QPalette pal;
    QProgressBar *msgProBar;
    QTimer *readTimer;
    QextSerialPort *serialPort;

    int appID;
    QString appName;
    QString appVersion;
    int appSize;
    QString appPath;
    QString errMsg;
    bool lock;
};

#endif // SMT_FOR_CUSTOMER_H
