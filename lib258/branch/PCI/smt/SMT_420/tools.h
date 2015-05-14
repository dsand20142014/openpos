#ifndef TOOLS_H
#define TOOLS_H

#include <string.h>
#include <QString>
#include <QTimer>
#include <QStringList>
#include <QFileInfo>
#include <QListWidget>
#include <QTime>
#include <QProgressBar>
#include <QLabel>
#include <QTableWidget>
#include <QTreeWidget>
#include <QtNetwork/QTcpSocket>
#include <QDebug>
#include "cappiteminfo.h"
#include "csysiteminfo.h"
#include "qextserialport.h"

#define BUF_MAX_SIZE            10240              //定义最大读取buf

#define DOWNCMD_REQUEST		0x01	//请求
#define DOWNCMD_RESPFILE	0x02	//文件
#define DOWNCMD_RESPKEY		0x03	//按键，只有网络状态下才有按键

#define TOOLS_ERROR_INITVAL     1   //错误码初始值,此值表示没有错误也没有成功
#define TOOLS_ERROR_SUCCESS     0   //成功
#define TOOLS_ERROR_TIMEOUT     -1  //超时
#define TOOLS_ERROR_SIGNERR     -2  //签名错误

// 包协议 14位
// 02+downcmd(1)+data+03+lrc
#define SPD_STX 0x02
#define SPD_ETX 0x03

#define REQUEST_FOLDERTYPE_APP			0x31	//应用程序
#define REQUEST_FOLDERTYPE_DRIVER		0x32	//驱动
#define REQUEST_FOLDERTYPE_LIB			0x33	//库
#define REQUEST_FOLDERTYPE_FONT			0x34	//字体
#define REQUEST_FOLDERTYPE_APPMANAGER	0x35	//os
#define REQUEST_FOLDERTYPE_UBOOT    	0x36	//uboot
#define REQUEST_FOLDERTYPE_KERNEL   	0x37	//kernel

#define REQUEST_FILETYPE_CONFIG			0x31	//系统级别的配置文件
#define	REQUEST_FILETYPE_SETTING		0x32	//应用级别的配置文件
#define REQUEST_FILETYPE_BIN			0x33	//程序文件
#define REQUEST_FILETYPE_BINLOGO		0x34	//程序图标及UI脚本
#define REQUEST_FILETYPE_BINIMG			0x35	//程序所用到的图片
#define REQUEST_FILETYPE_DAEMON         0x36	//系统进程文件
#define REQUEST_FILETYPE_SCRIPT			0x37	//应用程序UI脚本文件

#define PROCESS_ADD 	0x30	//新增
#define PROCESS_UPDATE	0x31	//更新
#define PROCESS_COVER	0x32	//覆盖
#define PROCESS_DEL		0x33	//删除

#define PREV    0
#define NEXT    1

//定义最终压缩包名称
#define UPDATE_PACK_NAME        "update.dta"        //更新包文件名
#define UPDATE_PACK_PATH        "tmp/update.dta"    //更新包文件路径
#define UPDATE_PACK_TEMP_PATH   "tmp/update.dta.zip"//更新包临时文件路径
#define UPDATE_DATA_NAME        "date.dta"          //接收的数据包名称
#define UPDATE_DATA_TEMP_NAME    "~date.dta"        //数据包临时存储名称
#define UPDATE_TOOL_NAME        "pack.dta"          //打包工具名称
#define UPDATE_SCHEME_PATH      "SMT_SCHEME/"       //打好的包的存放路径
#define UPDATE_SCHEME_DATA_PATH "SMT_SCHEME/update.dtas.zip"  //dtas临时包
//定义
#define INI_FILE_CONFIG         "conf.ini"          //标准配置文件名称
#define SH_FILE_DRIVERS         "drivers.sh"        //驱动SHELL文件名称
#define SH_FILE_DAEMON          "daemon.sh"         //DAEMON的SHELL文件名称
#define INI_FILE_DAEMON_SET     "setting.ini"       //DAEMON的设置文件名称
#define INI_FILE_DAEMON_LIST    "list.ini"          //DAEMON的配置列表文件名称
#define INI_FILE_DESC           "desc.ini"          //SYS中文件的简介配置名称

#define INI_FILE_CODE           "UTF-8"             //ini文件编码格式
#define UPDATESERVICE_VERSION   14                  //updateService版本号需要大于此版本号才能使用

#define INI_TYPE_PLAN           30                  //pln类型


class Tools
{
public:
    Tools();

    /*
     * @fn unsigned char OSSPD_tool_lrc(unsigned char *pointChar, int length)
     * @brief 计算字符穿pointChar的LRC值
     *
     * @param pointChar 需要计算lrc的字符串地址
     * @param length	字符串的长度
     *
     * @return 返回无符号字符lrc的值
     *
     */
    unsigned char OSSPD_tool_lrc(unsigned char *pointChar, int length);

    /*
     * @fn int checkPakHeader(char *cmdPonitStr, int *length, QextSerialPort *serialPort, int timeOut)
     * @brief 判断字符串是否为通讯协议头
     *
     * @param cmdPonitStr   需要判断的协议头字符串的地址
     * @param length        字符串的长度的地址
     * @Param timeOut       超时次数
     *
     * @return  如果是协议头返回0,updateService版本过低返回-1,其它错误返回-2
     *
     */
    int checkPakHeader(char *cmdPonitStr, int *length, int timeOut);

    /*
     * @fn int processRecvStream(unsigned char *recvBuf, int recvLength, QextSerialPort *serialPort)
     * @brief 接收并处理协议流
     *
     * @param recvBuf       已经接收的协议字符串的地址,此字符串已经通过了isCmdHead()函数的判断
     * @param recvLength    字符串的长度
     * @param serialPort    串口操作类的实例地址，通过此实例可读写串口数据操作串口
     *
     * @return  如果成功接收返回实际数据的长度(不包括协议头和校验位,例如文件的实际大小),否则返回-1
     *
     */
    int processRecvStream(unsigned char *recvBuf, int recvLength);

    /*
     * @fn int packgeCommand(int cmdType, QString name, unsigned char cmdArray[][2], int nCmd, QextSerialPort *serialPort)
     * @brief 对向POS请求的命令和发送给POS的数据文件进行打包发送
     *
     * @param cmdType       打包的命令类型,分为请求(0x01),文件(0x02),网络下的按键(0x03)
     * @param name          在请求类型下,如果请求APP详细信息,name为APPID,否则可以为NULL;如果请求的是文件,name为文件的路径名
     * @param cmdArray      命令的二维数组,存放详细命令参数
     * @param nCmd          命令的个数
     * @param serialPort    串口操作类的实例地址，通过此实例可读写串口数据操作串口
     *
     * @return  如果成功发送返回实际数据的长度(不包括协议头和校验位,例如文件的实际大小),否则返回-1
     *
     */
    int packgeCommand(int cmdType, QString name, unsigned char cmdArray[][2], int nCmd);

    /*
     * @fn void deleteDirectory(QFileInfo fileList)
     * @brief 递归删除文件夹及文件夹下的文件
     *
     * @param fileList  要删除文件夹的QFileInfo类型实例
     *
     * @return  无返回类型
     *
     */
    void deleteDirectory(QFileInfo fileList);

    /*
     * @fn bool addItemToListWidget(void *item, int type)
     * @brief 添加item到listWidget中
     *
     * @param item    需要添加的item的地址,APP为CAPPItemInfo类型,其他为CSYSItemInfo
     * @param type    item的类型,类型可以为APPLICATION,DAEMON,LIB,DRIVER,FONT,DATA,SHELL,UBOOT,KERNEL,CRT,CRAMFS,OTHER_BOOT
     *
     * @return  成功添加返回真,否则返回假
     *
     */
    bool addItemToListWidget(void *item, int type);

    /*
     * @fn void sortingFileDir(CAPPItemInfo *item, QString filePath)
     * @brief 遍历处理从POS机接收到的APP信息，并进行处理
     *
     * @param item      CAPPItemInfo类型应用地址
     * @param filePath  遍历的文件路径
     *
     * @return  无返回值
     *
     */
    void sortingFileDir(void *item, QString filePath, int type=0);

    /*
     * @fn void sleep(unsigned int msec)
     * @brief 休眠msec秒,且休眠过程中窗体能够响应外部事件
     *
     * @param msec  休眠msec秒
     *
     * @return  无返回值
     *
     */
    void sleep(unsigned int msec);

    /*
     * @fn bool updateItemOnListWidget(int index)
     * @brief 更新QListWidget上的APP应用信息,主要更新名称图片信息
     *
     * @param index     需要更新APP应用信息的索引值
     *
     * @return  成功更新返回真,否则返回假
     *
     */
    bool updateItemOnListWidget(int index);

    /*
     * @fn void getProgressBar(QProgressBar *proBar)
     * @brief 获取主窗体的进度条地址
     *
     * @param proBar    进度条控件地址
     *
     * @return  无返回值
     *
     */
    void getProgressBar(QProgressBar *proBar);

    /*
     * @fn void getStatusLabel(QLabel *msgDis, QLabel *msgLable)
     * @brief 获取主窗体的状态提示Label地址
     *
     * @param msgDis    状态提示Label地址
     * @param msgLable  通讯状态提示Label地址
     *
     * @return  无返回值
     *
     */
    void getStatusLabel(QLabel *msgDis, QLabel *msgLable);

    /*
     * @fn void getTimer(QTimer *readTimer)
     * @brief 获取主窗体的读寄存器
     *
     * @param readTimer    主窗体的读寄存器地址
     *
     * @return  无返回值
     *
     */
    void getTimer(QTimer *readTimer);

    /*
     * @fn void getTransType(int *transType)
     * @brief 获取主窗体的通讯方式
     *
     * @param transType    主窗体的通讯方式,0表示串口,1表示网络
     *
     * @return  无返回值
     *
     */
    void getTransType(int *transType);

    /*
     * @fn void getSocket(QTcpSocket *tcpSocket)
     * @brief 获取主窗体的QTcpSocket对象地址
     *
     * @param tcpSocket    主窗体的QTcpSocket对象地址
     *
     * @return  无返回值
     *
     */
    void getSocket(QTcpSocket *tcpSocket);

    /*
     * @fn void getItemAppList(QList<CAPPItemInfo> *itemAppList, QList<CAPPItemInfo> *itemAppDelList)
     * @brief 获取主窗体的APP应用的信息存储列表和删除信息存储列表地址
     *
     * @param itemAppList       APP应用的信息存储列表地址
     * @param itemAppDelList    APP应用的删除信息存储列表地址
     *
     * @return  无返回值
     *
     */
    void getItemAppList(QList<CAPPItemInfo> *itemAppList, QList<CAPPItemInfo> *itemAppDelList);

    /*
     * @fn void getItemSysList(QList<CSYSItemInfo> *itemSysList, QList<CSYSItemInfo> *itemSysDelList)
     * @brief 获取主窗体的SYS应用的信息存储列表和删除信息存储列表地址
     *
     * @param itemSysList       SYS应用的信息存储列表地址
     * @param itemSysDelList    SYS应用的删除信息存储列表地址
     *
     * @return  无返回值
     *
     */
    void getItemSysList(QList<CSYSItemInfo> *itemSysList, QList<CSYSItemInfo> *itemSysDelList);

    /*
     * @fn void getListWidget(QListWidget *appListWidget, QListWidget *sysListWidget, QListWidget *imageListWidget)
     * @brief 获取主窗体的appListWidget,sysListWidget,imageListWidget控件的地址
     *
     * @param appListWidget     APP应用的窗体存储容器地址
     * @param sysListWidget     SYS应用的窗体存储容器地址
     * @param imageListWidget   APP图片的窗体存储容器地址
     *
     * @return  无返回值
     *
     */
    void getListWidget(QListWidget *appListWidget, QListWidget *sysListWidget, QListWidget *imageListWidget);

    /*
     * @fn void getTableWidget(QTableWidget *tableWidget)
     * @brief 获取主窗体的tableWidget控件的地址
     *
     * @param tableWidget   显示SYS列表数据的控件容器地址
     *
     * @return  无返回值
     *
     */
    void getTableWidget(QTableWidget *tableWidget);

    /*
     * @fn void getSerialPort(QextSerialPort *serialPort)
     * @brief 获取主窗体的QextSerialPort对象的地址,用于对串口进行读写操作
     *
     * @param serialPort    父类中QextSerialPort对象的地址
     *
     * @return  无返回值
     *
     */
    void getSerialPort(QextSerialPort *serialPort);

    /*
     * @fn void getDaemonSet(QList<DAEMON_SET> *daemonSet)
     * @brief 获取主窗体的DAEMON_SET类型变量的地址,此变量用于记录setting.ini文件中DAEMON的设置信息
     *
     * @param daemonSet DAEMON_SET类型变量的地址
     *
     * @return  无返回值
     *
     */
    void getDaemonSet(QList<DAEMON_SET> *daemonSet);

    /*
     * @fn void getTreeWidget(QTreeWidget *treeWidget)
     * @brief 获取主窗体的QTreeWidget类的对象地址,此对象地址用于记录用户添加的应用的scheme
     *
     * @param treeWidget    QTreeWidget类的对象地址
     *
     * @return  无返回值
     *
     */
    void getTreeWidget(QTreeWidget *treeWidget);

    /*
     * @fn bool drawTable(int type)
     * @brief 根据应用类型绘制不同的表格并显示Item的信息
     *
     * @param type  应用的类型
     *
     * @return  成功绘制返回真,否则返回假
     *
     */
    bool drawTable(int type);

    /*
     * @fn bool creatIni(QString iniName, int itemType, void *item = NULL)
     * @brief 创建配置文件,配置文件内容根据item信息生成
     *

     * @param itemType      应用类型
     * @param iniName       配置文件路径和名称
     * @param item          应用信息类对象地址
     *
     * @return  成功返回真,否则返回假
     *
     */
    bool creatIni(QString iniName, int itemType, void *item = NULL);

    /*
     * @fn bool creatShell(QString shellName, int itemType)
     * @brief 创建SHELL文件,根据Item类型的不同创建相应的SHELL文件
     *
     * @param shellName     SHELL文件名称
     * @param itemType      应用类型
     *
     * @return  成功返回真,否则返回假
     *
     */
    bool creatShell(QString shellName, int itemType);

    /*
     * @fn bool updateAllItem(void)
     * @brief 更新所有应用到POS
     *
     * @param 无参数
     *
     * @return  成功返回真,否则返回假
     *
     */
    bool updateAllItem(void);

    /*
     * @fn bool updateItemSortInfo(void)
     * @brief 更新所有APP应用的排序信息并将其发送到POS
     *
     * @param 无参数
     *
     * @return  成功返回真,否则返回假
     *
     */
    bool updateItemSortInfo(void);

    /*
     * @fn void updateInsmodList(int itemType)
     * @brief 更新insmod列表到相应应用的信息列表中
     *
     * @param itemType  应用类型
     *
     * @return  无返回值
     *
     */
    void updateInsmodList(int itemType);

    /*
     * @fn void creatDelFile(QString path, int itemType)
     * @brief 创建需要删除的SYS应用的文件
     *
     * @param path      删除的文件的保存路径
     * @param itemType  应用类型
     *
     * @return  无返回值
     *
     */
    void creatDelFile(QString path, int itemType);

    /*
     * @fn bool creatRSAFile(CSYSItemInfo *item)
     * @brief 创建RSA文件
     *
     * @param item      需要创建RSA文件的应用信息对象地址
     *
     * @return  成功返回真,失败返回假
     *
     */
    bool creatRSAFile(CSYSItemInfo *item);

    /*
     * @fn void rsaSignData(unsigned char *srcData, int length, unsigned char *rsa_signdata)
     * @brief 对需要签名的数据文件进行数据签名
     *
     * @param srcData       需要签名的源数据地址
     * @param length        源数据长度
     * @param rsa_signdata  签名完成后数据存储地址
     *
     * @return  无返回值
     *
     */
    void rsaSignData(unsigned char *srcData, int length, unsigned char *rsa_signdata);

    /*
     * @fn void initMsgPorBar(int maximum)
     * @brief 初始化进度条,并将进度条的最大值设置为maximum
     *
     * @param maximum   进度条的最大值
     *
     * @return  无返回值
     *
     */
    void initMsgPorBar(int maximum);

    /*
     * @fn bool updateOtherBoot(void)
     * @brief 更新s19文件到终端
     *
     * @param 无参数
     *
     * @return  成功返回真,否则返回假
     *
     */
    bool updateOtherBoot(void);

    /*
     * @fn int packData(unsigned char sendBuf[], int sendBufLen, int packNum, unsigned char dataBuf[], int dataLen, int packType)
     * @brief 组包，将包头信息，数据长度，包数据和校验位进行组装，返回组装后的数据的长度
     *
     * @param sendBuf       包组好后的存放地址
     * @param sendBufLen    发送包的最大长度
     * @param packNum       包标号
     * @param dataBuf       源数据地址
     * @param dataLen       源数据的长度
     * @param packType      报类型,0x01表示请求命令类型,0x02表示请求文件类型
     *
     * @return  返回组装后的数据的长度
     *
     */
    int packData(unsigned char sendBuf[], int sendBufLen, int packNum, unsigned char dataBuf[], int dataLen, int packType);

    /*
     * @fn bool waitReply(int timeOut, int *packNum, bool *replay)
     * @brief 返回包发送后对方的回复结果
     *
     * @param timeOut   接收应答的超时次数
     * @param packNum   对方接收包的包号
     * @param replay    对方接收的包号码为packNum的接收结果,如果对方返回ok为真,返回ng为假
     *
     * @return  如果接收到回复信息返回真,否则返回假
     *
     */
    bool waitReply(int timeOut, int *packNum, bool *replay);

    /*
     * @fn int getItemCount(int itemType, int listType = 0)
     * @brief 获取列表中itemType类型应用的个数
     *
     * @param itemType  应用类型
     * @param listType  应用列表类型,默认为0,为1时表示删除列表
     *
     * @return  列表中itemType类型应用的个数
     *
     */
    int getItemCount(int itemType, int listType = 0);

    /*
     * @fn int getUpdateItemCount(int itemType)
     * @brief 获取列表中itemType类型需要下载的应用个数
     *
     * @param itemType  应用的类型
     *
     * @return  列表中itemType类型需要下载的应用个数
     *
     */
    int getUpdateItemCount(int itemType);

    /*
     * @fn bool modifyItemOnListWidget(void *item, int itemType)
     * @brief 修改item并将信息保存到列表中
     *
     * @param item    需要修改的item的地址,APP为CAPPItemInfo类型,其他为CSYSItemInfo
     * @param type    item的类型,类型可以为APPLICATION,DAEMON,LIB,DRIVER,FONT,DATA,SHELL,UBOOT,KERNEL,CRT,CRAMFS,OTHER_BOOT
     *
     * @return  修改成功返回真,否则返回假
     *
     */
    bool modifyItemOnListWidget(void *item, int itemType);

    /*
     * @fn bool swapItemOnListWidget(int moveType)
     * @brief 移动选中的应用，提供向前和向后两种移动，此函数只适合APP的移动
     *
     * @param moveType  应用移动类型，PRVE向前移动，NEXT向后移动
     *
     * @return  成功移动应用返回真，否则返回假
     *
     */
    bool swapItemOnListWidget(int moveType);

    /*
     * @fn bool removeItemOnListWidget(int itemType)
     * @brief 从listWidget中移除应用，如果应用来自POS，被删除的应用信息保存在删除列表中
     *
     * @param itemType  所要移除的应用的类型
     *
     * @return  成功删除应用返回真，否则返回假
     *
     */
    bool removeItemOnListWidget(int itemType);

    /*
     * @fn bool getItemInfoFromPos(unsigned char cmdArray[][2], int nCmd)
     * @brief 从POS机获取APP列表信息，并将信息添加到listWidget中
     *
     * @param cmdArray      向POS机发送的请求列表信息
     * @param nCmd          请求命令列表的个数
     *
     * @return  成功获取应用返回真，否则返回假
     *
     */
    bool getItemInfoFromPos(unsigned char cmdArray[][2], int nCmd);

    /*
     * @fn bool unpack(unsigned char cmdArray[][2])
     * @brief 对接收到的包进行解压缩，并将相应信息添加到QListWidget中
     *
     * @param cmdArray  命令列表
     *
     * @return  成功返回真，失败返回假
     *
     */
    bool unpack(unsigned char cmdArray[][2]);

    /*
     * @fn void readIni(int itemType, QString iniPath)
     * @brief 读取INI文件中的配置信息
     *
     * @param itemType  应用类型
     * @param iniPath   INI文件路径
     *
     * @return  成功返回真，失败返回假
     *
     */
    void readIni(int itemType, QString iniPath);

    /*
     * @fn bool updateItemToPos(int itemType, int index = -1, int updateType = 0)
     * @brief 更新itemType类型的应用到POS中
     *
     * @param itemType      应用类型
     * @param index         APP应用索引号,只有当itemType=0时才起作用
     * @param updateType    更新类型,默认0表示更新到POS机,其他值表示只是打包发布,只有当itemType=0时才起作用
     *
     * @return  无返回值
     *
     */
    bool updateItemToPos(int itemType, int index = -1, int updateType = 0);

    /*
     * @fn bool addDtaToListWidget(QString dtaPath)
     * @brief 将DTA包中的内容解压缩后添加到QListWidget中
     *
     * @param dtaPath   DTA文件路径
     *
     * @return  成功返回真，失败返回假
     *
     */
    bool addDtaToListWidget(QString dtaPath);

    /*
     * @fn void addImageToListWidget(QString imagePath, QString *curIcoPath, QStringList *curLogoPath, QStringList *curDelPicture)
     * @brief 将图片添加到图片显示容器中，主要使用于APP的ICO，Logo和打印图片
     *
     * @param imagePath     需要添加的图片的地址
     * @param curIcoPath    当前Item的ICO图标地址
     * @param curLogoPath   当前Item的Logo图片地址列表
     * @param curDelPicture 当前Item已经删除的图片名称列表
     *
     * @return  无返回值
     *
     */
    void addImageToListWidget(QString imagePath, QString *curIcoPath, QStringList *curLogoPath, QStringList *curDelPicture);

    /*
     * @fn void showImage(QString imagePath)
     * @brief 解析图片类型,并将其添加到相应的位置并显示
     *
     * @param imagePath     需要添加的图片的地址
     *
     * @return  无返回值
     *
     */
    void showImage(QString imagePath);

    /*
     * @fn void initTreeWidget(QString iniPath)
     * @brief 初始化Scheme Tree Widget空间
     *
     * @param iniPath   包含有Scheme信息的配置文件
     *
     * @return  无返回值
     *
     */
    void initTreeWidget(QString iniPath);

    /*
     * @fn bool saveScheme(char *iniName)
     * @brief 保存Scheme信息
     *
     * @param iniName   schemeINI文件名称
     *
     * @return  成功返回真,失败返回假
     *
     */
    bool saveScheme(char *iniName);

    /*
     * @fn bool addSchemeToWidget(QString iniPath)
     * @brief 添加Scheme信息到QtreeWidget
     *
     * @param iniName   schemeINI文件名称
     *
     * @return  成功返回真,失败返回假
     *
     */
    bool addSchemeToWidget(QString iniPath);

    /*
     * @fn bool removeScheme(QTreeWidgetItem *curTreeWdtItem)
     * @brief 移除Scheme信息从QtreeWidget
     *
     * @param iniName   schemeINI文件名称
     *
     * @return  成功返回真,失败返回假
     *
     */
    bool removeScheme(QTreeWidgetItem *curTreeWdtItem);

    /*
     * @fn bool addItemFromScheme(QTreeWidgetItem *curTreeWdtItem)
     * @brief 将指定Scheme信息添加到QListWidget
     *
     * @param curTreeWdtItem    Scheme信息所对应的QTreeWidgetItem对象地址
     *
     * @return  成功返回真,失败返回假
     *
     */
    bool addItemFromScheme(QTreeWidgetItem *curTreeWdtItem);

    /*
     * @fn int analysisDesc(QString desc, QString *brief, QString *valueList)
     * @brief 分析DAEMON的setting.ini文件中的信息,并返回信息类型
     *
     * @param desc      待解析的描述字符串
     * @param brief     选项的简述
     * @param valueList 设置选项的值的列表信息
     *
     * @return  0表示scan by carcode,1表示text,2表示radio,3表示checkbox,4表示just for display,5表示passwd
     *
     */
    int analysisDesc(QString desc, QString *brief, QString *valueList);

    /*
     * @fn int getSessionCount(void)
     * @brief 获取daemonSet中的不同的session的个数
     *
     * @param 无参数
     *
     * @return  返回daemonSet中的不同的session的个数
     *
     */
    int getSessionCount(void);

    /*
     * @fn bool updateDaemonSet(void)
     * @brief 更新daemonSet
     *
     * @param 无参数
     *
     * @return  成功返回真,失败返回假
     *
     */
    bool updateDaemonSet(void);

    /*
     * @fn int getPrivateProfileInt(QString sessionName, QString keyName, int defValue, QString fileName)
     * @brief 获取INI文件中session为sessionName,key值为keyName的键值数值信息
     *
     * @param sessionName   INI文件中的session
     * @param keyName       INI文件中的key
     * @param defValue      键值的默认值
     * @param fileName      INI文件名称
     *
     * @return  返回需要获取的键值
     *
     */
    int getPrivateProfileInt(QString sessionName, QString keyName, int defValue, QString fileName);

    /*
     * @fn QString getPrivateProfileString(QString sessionName, QString keyName, QString defValue, QString fileName)
     * @brief 获取INI文件中session为sessionName,key值为keyName的键值字符串信息
     *
     * @param sessionName   INI文件中的session
     * @param keyName       INI文件中的key
     * @param defValue      键值的默认值
     * @param fileName      INI文件名称
     *
     * @return  返回需要获取的键值
     *
     */
    QString getPrivateProfileString(QString sessionName, QString keyName, QString defValue, QString fileName);

    /*
     * @fn bool writePrivateProfileString(QString sessionName, QString keyName, QString value,QString fileName)
     * @brief 将键值为value写入INI文件中,其中session为sessionName,key值为keyName
     *
     * @param sessionName   INI文件中的session
     * @param keyName       INI文件中的key
     * @param value         键值
     * @param fileName      INI文件名称
     *
     * @return  成功返回真,失败返回假
     *
     */
    bool writePrivateProfileString(QString sessionName, QString keyName, QString value,QString fileName);

    /*
     * @fn QStringList getChildValues(QString sessionName, QString fileName)
     * @brief 获取childSession下的value值列表
     *
     * @param sessionName   session名称
     * @param fileName      配置文件名称
     *
     * @return  返回childSession下的value值列表
     *
     */
    QStringList getChildValues(QString sessionName, QString fileName);

    /*
     * @fn int read(char *readBuf, int bufSize)
     * @brief 读取串口或网络的数据,返回读到的字节数
     *
     * @param readBuf   读取字符串的buf
     * @param bufSize   buf大小
     *
     * @return  返回读取的字节数
     *
     */
    int read(char *readBuf, int bufSize);

    /*
     * @fn int write(char *writeBuf, int bufSize)
     * @brief 将数据写入串口或网络,返回实际写入的字节数
     *
     * @param writeBuf   要写入的字符串
     * @param bufSize   buf大小
     *
     * @return  返回实际写入的字节数
     *
     */
    int write(char *writeBuf, int bufSize);

    /*
     * @fn bool checkTransPort(void)
     * @brief 检查传输端口是否可用,端口分为串口和网络两种
     *
     * @return  端口可用返回真,否则返回假
     *
     */
    bool checkTransPort(void);

    /*
     * @fn bool openSerialPort(void)
     * @brief 打开串口
     *
     * @return  成功打开串口返回真,否则返回假
     *
     */
    bool openSerialPort(void);

    /*
     * @fn bool openSerialPort(void)
     * @brief 关闭串口
     *
     * @return  成功关闭串口返回真,否则返回假
     *
     */
    void closeSerialPort(void);
    //bool connectNetWork(void);
    //void disconnectNetWork(void);

    void formatKey(QString *keyBuf);
    bool releasePlnToUSB(QTreeWidgetItem *treeWidgetItem);
    bool packPln(QList<CAPPItemInfo> *itemList, QString schemeName);
    bool unpackPln(QString dtasPath);
    void repeatRsaSignData(unsigned char *srcData, int length, unsigned char offsetData[128], unsigned char *rsa_signdata);
    bool signApp(QString srcPath, QString signPath);

private:
    //主窗体进度条
    QProgressBar *msgProBar;
    //主窗体状态条
    QLabel *msgDis;
    //当前通讯状态
    QLabel *msgLable;
    //读信息计时器
    QTimer *readTimer;
    //APP应用信息存储列表
    QList<CAPPItemInfo> *itemAppList;
    //SYS应用信息存储列表
    QList<CSYSItemInfo> *itemSysList;
    //APP已经删除应用的存储列表
    QList<CAPPItemInfo> *itemAppDelList;
    //SYS已经删除应用的存储列表
    QList<CSYSItemInfo> *itemSysDelList;
    //DAEMON的设置信息存储列表
    QList<DAEMON_SET>   *daemonSet;
    //主窗体APP应用显示控件地址
    QListWidget *appListWidget;
    //主窗体SYS应用显示控件地址
    QListWidget *sysListWidget;
    //主窗体图片显示控件地址
    QListWidget *imageListWidget;
    //内部ICO图标路径列表信息
    QStringList resourceIcoPath;
    //主窗体SYS应用信息列表显示空间地址
    QTableWidget *tableWidget;
    //主窗体SYS应用信息列表显示的标题头
    QStringList tableHeader;
    //用于多语言界面的翻译
    QStringList tableDesc;
    //串口通讯QextSerialPort对象地址
    QextSerialPort *serialPort;
    //主窗体scheme信息显示控件地址
    QTreeWidget *treeWidget;
    //scheme中已存在的信息列表
    QStringList saveCfgList;
    //通讯类型标识,0表示串口,1表示网络
    int *transType;
    //TcpSocket对象地址
    QTcpSocket *tcpSocket;
    //scheme中顶级目录QTreeWidgetItem
    QTreeWidgetItem *rootTreeWdtItem;
    //setting.ini文件修改标志,未修改为0，修改为1
    int settingMoidfy;
    //发送dta文件成功标志
    int sendDtaSuccess;

};

#endif // TOOLS_H
