#ifndef TOOLS_H
#define TOOLS_H

#include <string.h>
#include <QString>
#include <QTimer>
#include <QStringList>
#include <QFileInfo>
#include <QTime>
#include <QProgressBar>
#include <QLabel>
#include <QPlainTextEdit>
#include <QStringList>
#include "qextserialport.h"

//定义
#define DWN_SAVE_PATH       "DWN/"              //未指定存储目录时,DWN文件保存目录
#define DWN_SAVE_NAME       "~dwnTemp.dwn"      //只传输时的dwn临时文件名
#define INI_FILE_CONFIG         "conf.ini"          //标准配置文件名称
#define DWN_FILE_HEAD       "SAND ARM7&ARM9 DNW"
#define ARM9_CMD            "\x02\x41\x52\x45\x59\x4F\x55\x4F\x4B\x03"
#define INI_FILE_CODE           "UTF-8"             //ini文件编码格式
#define ARM7_CPU_TYPE       1
#define ARM9_CPU_TYPE       2

typedef struct
{
    QString curAppName;
    int curAppID;
    int curAppVersion;
    int curCpuType;//1-ARM7,2-ARM9
    int curMergeType;//1-merge,0-no merge
    int curArm7Len;
    int curArm9Len;
}DWN_FILE_INFO;


typedef struct
{
    QString appName;
    QString appID;
    QString appVersion;
    int cpuType;
}BIN_FILE_INFO;


class Tools
{
public:
    Tools();
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


    void getSerialPort(QextSerialPort *serialPort);
    int mergeDnwFile(QString curArm7Path, QString curArm9Path, QString mergeFilePath);
    void getStatusLabel(QLabel *msgDis);
    void getProgressBar(QProgressBar *proBar);

    void sleep(unsigned int msec);
    bool transmitFileToPos(QString filePath);
    void initMsgPorBar(int maximum);
    void getAppInfo(QString *appName, int *appID, int *appVersion, int *cpuType);
    void getErrMsg(QString *errMsg);

    int checkARM(int timeOut);
    bool convertFileToDwn(QString binFilePath, QString dwnFilePath);
    bool getFileInfo(DWN_FILE_INFO *curFileInfo, QString filePath);
    void clearDwnFileInfo(DWN_FILE_INFO *curFileInfo);
    bool getBinFileInfo(BIN_FILE_INFO *binFileInfo);
    int setBinFileInfo(BIN_FILE_INFO *binFileInfo);
    void getBinFileInfoList(QStringList *binFileNameList);

private:
    //主窗体进度条
    QProgressBar *msgProBar;
    //主窗体状态条
    QLabel *msgDis;
    //串口通讯QextSerialPort对象地址
    QextSerialPort *serialPort;
    //显示串口调试信息
    QPlainTextEdit *plnEditSerialPort;

    //应用名称
    QString *appName;
    //应用ID
    int *appID;
    //应用版本
    int *appVersion;
    //应用CPU类型:1 ARM7,2 ARM9
    int *cpuType;


    //操作错误信息提示
    QString *errMsg;
};

#endif // TOOLS_H
