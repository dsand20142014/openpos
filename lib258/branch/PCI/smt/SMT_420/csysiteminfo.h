#ifndef CSYSITEMINFO_H
#define CSYSITEMINFO_H
#include <QString>
#include <QList>

#define DAEMON      1
#define LIB         2
#define DRIVER      3
#define FONT        4
#define DATA        5
#define SHELL       6
#define UBOOT       7
#define KERNEL      8
#define CRT         9
#define CRAMFS      10
#define OTHER_BOOT  11


typedef struct{
    QString session;
    QString key;
    QString value;
    int type;
    QString brief;
    QString valueList;
    QString parent;
} DAEMON_SET;

class CSYSItemInfo
{
public:
    CSYSItemInfo();
    void resetItem();
    int getItemCount(QList<CSYSItemInfo> *itemList);
    bool removeItem(int index, QList<CSYSItemInfo> *itemList);
    bool replaceItem(int index, QList<CSYSItemInfo> *itemList);
    int isItemExist(QList<CSYSItemInfo> *itemList);
    bool addItem(QList<CSYSItemInfo> *itemList);
public:
    //类型
    int type;
    //源文件名称
    QString srcName;
    //源文件路径
    QString srcPath;
    //源文件状态
    int status;
    //应用版本号
    QString version;
    //如果是DTA包，保存DTA包路径
    QString dtaPath;
    //如果Item是KO或者Daemon，KO加载标志可用,0不加载，1加载
    int insmod;
    //KO debug 标志
    int dbg;
};

#endif // CSYSITEMINFO_H
