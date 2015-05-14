#ifndef SMTMAINWINDOW_H
#define SMTMAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QTreeWidgetItem>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include "cappiteminfo.h"
#include "csysiteminfo.h"
#include "qextserialport.h"
#include "sessionoptions.h"
#include "codegenerator.h"
#include "tools.h"

#define SMT_VERSION     "SMT_1.5.8"

/**************************************
    SMT 1.5.8 update at 2013.03.21 by garvey
    1.修改write和read函数,降低SMT传输过程中死掉的问题
    2.修改某一任务计划列表文件不存在时删除掉,计划任务列表索引大于10保存ini文件错误问题
    3.添加报错提示信息,软件抬头连接状态等信息
    4.解决串口连接断开后继续通信SMT报错问题

    SMT 1.5.7 update at 2013.02.02 by garvey
    1.修改daemon.sh文件中进程调试标志
    2.添加更新同步完成时提示窗口
    3.提升updateService验证版本到1.4

    SMT 1.5.6 update at 2013.01.11 by garvey
    1.对pln文件中的conf.ini添加UPDATE标志

    SMT 1.5.5 update at 2012.12.26 by Garvey
    1.添加生成USB更新包".pln"文件的功能
    2.对界面操作有些小bug进行修改

    SMT 1.5.4 update at 2012.12.18 by Garvey
    1.修整了图片和应用设置窗体
    2.解决UI脚本取消再次点选后脚本路径没有保存的bug

    SMT 1.5.3 update at 2012.12.13 by Garvey
    1.修复通过SCHEME添加已删除的应用后,POS机中应用被误删除的bug
    2.解决网络已连接上时,SMT依然显示未连接的bug
    3.添加SMT.exe文件的ico图标

    SMT_1.5.2 update at 2012.12.05 by Garvey
    1.修复中文界面时不能正确显示中文描述问题
    2.修改desc.ini文件

    SMT_1.5.1 update at 2012.12.03 by Garvey
    1.修改SCHEME中路径含有中文时不能正确添加的问题

    SMT 1.5.0 update at 2012.11.12 by Garvey
    1.添加SMT网络下载功能
    2.优化判断包头文件函数,提高捕获报的能力
    3.添加语言切换包,支持中英文

    SMT 1.4.6 update at 2012.11.09 by Garvey
    1.修改配置窗口点击取消关闭整个SMT的bug
    2.修改了获取DAEMON信息不全的bug
    3.添加读setting.ini文件中出现垃圾选项时的处理

    SMT 1.4.5 update at 2012.11.07 by Garvey
    1.修改程序下载时EDIT标志,新增为0,更新为1,覆盖为2,删除为3
    2.修改image全部删除时POS中图片不能删除的BUG

    SMT_1.4.4 update at 2012.10.30 by Garvey
    1.添加握手时校验updateService版本
    2.将UPDATE_SCHEME_PATH宏定义变为"SMT_SCHEME/",为了解决解压缩路径中含有sc报错问题

    SMT_1.4.3 update at 2012.10.30 by Garvey
    1.在SYS选项卡点击除SHELL文件时隐藏分割线和apply按钮;
    2.屏蔽tab按键
    3.修改daemon.sh和drivers.sh中的文件内容

    SMT1.4.2 update at 2012.10.26 by Garvey
    1.对于APP不获取应用详细信息时同样可以更改应用部分信息
    2.获取APP详细信息不再获取应用.bin文件
    3.UI脚本及图片删除时记录删除的信息,同步时在删除文件头加~

    SMT1.4.1 update at 2012.10.09 by Garvey
    1.下载时屏蔽右击菜单

    SMT1.4 update log at 2012.9.26
    1.优化了代码接收函数
    2.解决了不同操作系统中文乱码问题
    3.所有INI等配置文件全部统一使用UTF-8编码
**************************************/

namespace Ui {
class SMTMainWindow;
}

class SMTMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit SMTMainWindow(QWidget *parent = 0);
    ~SMTMainWindow();

private slots:
    //读取串口中的信息
    void onReadyRead();
    //打开串口或网络
    void on_actionConnect_triggered();
    //断开串口或网络
    void on_actionDisconnect_triggered();
    //过滤器函数
    bool eventFilter(QObject *,QEvent *);
    //打开配置窗口
    void on_actionConfig_triggered();
    //更新应用到POS
    void on_actionUpdate_triggered();
    //SMT和POS进行握手,获取POS中信息
    void on_actionShake_triggered();
    //清屏
    void on_actionWip_triggered();
    //切换成网络模式
    void on_actionNetWork_Server_triggered();
    //切换成串口模式
    void on_actionSerial_Port_triggered();
    //编译
    void on_actionArm_GCC_triggered();
    //显示软件版本
    void on_actionVersion_triggered();
    //退出软件
    void on_actionExit_triggered();
    //将读到的信息以HTML语言格式进行显示
    void insertHtmlToWidget();
    //资源列表中单击遍历下一级目录
    void on_treeResource_itemClicked(QTreeWidgetItem *item, int column);
    //资源列表中双击添加应用
    void on_treeResource_itemDoubleClicked(QTreeWidgetItem *item, int column);
    //单击APP显示控件中的item显示item的详细信息
    void on_listWgtApplication_itemClicked(QListWidgetItem *item);
    //单击SYS显示控件中的item显示item的详细信息
    void on_listWgtSystem_itemClicked(QListWidgetItem *item);
    //APP前移
    void prevMoveItem();
    //APP后移
    void nextMoveItem();
    //移除item
    void removeItem();
    //移除资源中的item
    void removeTreeItem();
    //刷新资源中的item
    void refreshTreeItem();
    //发布APP中的指定item,打成DTA包存储在schem目录下
    void releaseItem();
    //发布SCHEME中的APP进行USB更新,打包成DTAS包
    void releasePln();
    //保存item的修改
    void on_btnApply_clicked();
    //确保APPID输入值正确
    void on_editAppID_textEdited(const QString &arg1);
    //双击item获取POS机中的更详细的信息
    void on_listWgtApplication_itemDoubleClicked(QListWidgetItem *item);
    //双击图片删除
    void on_listWgtImage_itemDoubleClicked(QListWidgetItem *item);
    //保存scheme的配置计划
    void on_btnCreateScheme_clicked();
    //双击scheme中的item添加到现有控件中
    void on_treeScheme_itemDoubleClicked(QTreeWidgetItem *item, int column);

    //UI脚本状态发生改变
    void on_chkBoxUIScript_stateChanged(int arg1);

    //创建一个QTcpSocket新连接
    void creatNewConnect(void);
    //网络断开
    void disconnectNetwork(void);
    //删除多选的应用 未完成
    //void removeMultipleItems(void);

private:
    void writeCommand(int key);
    void insertCmdChar(int key);
    void resizeEvent(QResizeEvent *e);
    void showItemInfo(int type);
    void clear(void);
    void clearAll(void);
    void contextMenuEvent(QContextMenuEvent * event);
    
private:
    Ui::SMTMainWindow *ui;
    QTimer *readTimer;
    QTimer *writeTimer;
    QextSerialPort *serialPort;
    QStringList strCmdList;
    QString strCmd;
    Tools tools;
    char tempstr[20];
    QLabel *msgPartL;
    QLabel *msgTty;
    QLabel *msgLable;
    QLabel *msgSpace;
    QLabel *msgPos;
    QLabel *msgPosType;
    QLabel *msgDis;
    QPalette pal;
    QProgressBar *msgProBar;
    QList<CAPPItemInfo> itemAPPInfo;
    QList<CSYSItemInfo> itemSYSInfo;
    QList<CAPPItemInfo> itemDelList;
    QList<CSYSItemInfo> itemSysDelList;
    QList<DAEMON_SET>   daemonSet;
    QString curIcoPath;
    QStringList curLogoPath;
    QStringList curDelPicture;
    QString curUIScriptPath;
    QString curDelUIScriptName;
    QTcpSocket *tcpSocket;
    QTcpServer *tcpServer;
    int appIndex;
    int sysIndex;
    bool lock;
    int transType;
    bool shakeFlag;
};

#endif // SMTMAINWINDOW_H
