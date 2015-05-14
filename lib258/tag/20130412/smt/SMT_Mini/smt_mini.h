#ifndef SMT_MINI_H
#define SMT_MINI_H

#include <QMainWindow>
#include "tools.h"

#define     SMT_VERSION     "SMT_Mini 2.0.0"
/*****************************************************************
  SMT_Mini 2.0.0 update at 2013.03.15 by garvey
  1.重新设置界面
  2.代码重新编写
  3.功能重新定制

  SMT_Mini 1.2.3 update at 2012.12.19 by garvey
  1.修改合并密码

  SMT_Mini 1.2.2 update at 2012.12.19 by garvey
  1.修改界面布局
  2.修改空文件是点击合并按钮出错问题

  SMT_Mini 1.2.1 update at 2012.12.17 by garvey
  1.增加对杉德DNW软件直接压缩生成的dwn文件的支持
  2.添加对bin文件进行打包压缩处理成dwn文件的功能

  SMT_Mini 1.2 update at 2012.11.14 by garvey
  1.对传输报的包头和包尾进行添加
  2.修改能够不关闭SMT就能传输ARM7和ARM9的APP
  3.添加软件版本和退出菜单

  SMT_Mini 1.1 update at 2012.11.12 by garvey
  1.解决获取dwn文件的ID和名称不准确的问题
  SMT_Mini 1.0 create at 2012.11.12 by Garvey


*****************************************************************/
namespace Ui {
class SMT_Mini;
}

class SMT_Mini : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit SMT_Mini(QWidget *parent = 0);
    ~SMT_Mini();

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
    //过滤器函数
    bool eventFilter(QObject *,QEvent *);
    //清屏
    void on_actionWipe_triggered();
    //将读到的信息以HTML语言格式进行显示
    void insertHtmlToWidget();

    //同步下载
    void on_actionSync_triggered();

    void on_actionCOM1_triggered();
    void on_actionCOM2_triggered();
    void on_actionCOM3_triggered();
    void on_actionCOM4_triggered();
    void on_actionCOM5_triggered();


    //确保APPID输入值正确
    void on_lineEditAppID_textEdited(const QString &arg1);

    //确保APP Version输入值正确
    void on_lineEditVersion_textEdited(const QString &arg1);

    void on_pushButtonArm7FileSelect_clicked();

    void on_pushButtonArm9FileSelect_clicked();

    void on_pushButtonMergeFileSavePath_clicked();

    void on_pushButtonDwnFileSavePath_clicked();

    void on_pushButtonBinFileSelect_clicked();

    void on_radioButtonTransmit_clicked();

    void on_radioButtonConvert_clicked();

    void on_radioButtonConvertAndTransmit_clicked();

    void on_tabMain_currentChanged(int index);

    void on_comboBoxCPUType_currentIndexChanged(int index);

    void on_radioButtonMerge_clicked();

    void on_radioButtonMergeTransmit_clicked();

    void on_pushButtonDwnSelectFilePath_clicked();

    void on_comboBoxAppName_currentIndexChanged(const QString &arg1);

private:
    void writeCommand(int key);
    void insertCmdChar(int key);
    //void resizeEvent(QResizeEvent *e);
    void clearAppInfo(int type);
    void setAppInfo(void);
    void resetActionComPortIco(void);
    void updateAppNameList(void);

private:
    Ui::SMT_Mini *ui;
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
    QTimer *writeTimer;
    QextSerialPort *serialPort;
    QStringList strCmdList;
    QString strCmd;
    bool lock;

    //应用名称
    QString appName;
    //应用ID
    int appID;
    //应用版本
    int appVersion;
    //应用CPU类型:1 ARM7,2 ARM9
    int cpuType;
    //bin文件路径
    QString binPath;
    //转换后dwn文件名称
    QString dwnFileName;
    //转换后dwn文件保存路径
    QString dwnFilePath;
    //文件转换后操作类型:0只转换,1只传输,2转换保存后再传输
    int convertOption;

    //Arm7文件路径
    QString arm7FilePath;
    //Arm7应用名称
    QString arm7AppName;
    //Arm7应用ID
    int arm7AppID;

    //Arm9文件路径
    QString arm9FilePath;
    //Arm9应用名称
    QString arm9AppName;
    //Arm7应用ID
    int arm9AppID;

    //合并后的dwn文件保存路径
    QString mergeFilePath;
    //合并后的dwn文件名称
    QString mergeFileName;
    //合并后文件操作:0只合并文件,1合并保存文件后再传输
    int mergeOption;

    QString dwnTransmitPath;//dwn文件下载界面中文件路径

    //操作错误信息提示
    QString errMsg;
};

#endif // SMT_MINI_H
