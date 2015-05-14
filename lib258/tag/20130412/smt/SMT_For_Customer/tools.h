#ifndef TOOLS_H
#define TOOLS_H

#include <string.h>
#include <QString>
#include <QProgressBar>
#include <QLabel>

#include "qextserialport.h"

//定义
#define INI_FILE_CONFIG         "conf.ini"          //标准配置文件名称

#define DWN_FILE_HEAD       "SAND ARM7&ARM9 DNW"
#define ARM9_CMD            "\x02\x41\x52\x45\x59\x4F\x55\x4F\x4B\x03"
#define ARM7_CPU_TYPE       1
#define ARM9_CPU_TYPE       2

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
    bool getFileInfo(QString filePath);
    void getStatusLabel(QLabel *msgDis);
    void getProgressBar(QProgressBar *proBar);
    void getAppParam(int *appID, QString *appName, QString *appVersion, int *appSize);
    void getErrMsg(QString *errMsg);
    void sleep(unsigned int msec);
    bool sync(QString filePath);
    void initMsgPorBar(int maximum);
    int checkARM(int timeOut);

private:
    //主窗体进度条
    QProgressBar *msgProBar;
    //主窗体状态条
    QLabel *msgDis;
    //串口通讯QextSerialPort对象地址
    QextSerialPort *serialPort;

    int *appID;
    QString *appName;
    QString *appVersion;
    int *appSize;
    QString *errMsg;
    int cpuType;//1：ARM7，2：ARM9
    int mergeType;//1:merge file,0:no merge file
    int arm7Len;
    int arm9Len;
};

#endif // TOOLS_H
