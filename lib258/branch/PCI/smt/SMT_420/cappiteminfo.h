#ifndef CAPPITEMINFO_H
#define CAPPITEMINFO_H
#include <QList>
#include <QString>
#include <QStringList>

#define APPLICATION     0

class CAPPItemInfo
{
public:
    CAPPItemInfo();
    void resetItem(void);
    int isItemExist(QList<CAPPItemInfo> *itemList);
    bool addItem(QList<CAPPItemInfo> *itemList);

public:
    //源文件名称
    QString srcName;
    //应用在POS终端上所显示的名称
    QString appDesc;
    //应用号
    int appNumber;
    //源文件的存储路径,来自于POS端的文件路径不存在
    QString srcPath;

    /*
    **  应用的状态,4个字节int类型变量,左边为高位右边为低位
    **
    **  8 7 6 5   4 3 2 1
    **  | | | |   | | | |
    **  x x x x   x x x x
    **
    ** 每一位代表含义如下:
    **  1)应用ICO图标修改标志,0未修改,1已经修改或变更
    **  2)应用打印或其它图片修改标志,0未修改,1已经修改或变更
    **  3)应用配置文件修改标志,0未修改,1已经修改或变更
    **  4)UI脚本修改标志,0未修改,1已经修改或变更
    **  5)应用来自于POS机标志为0,否则标志为1
    **  6)已经将修改文件成功更新标志为0,更新失败或未更新为1
    **  7)应用来自POS机时,成功从POS中获取详细信息标志为0,否则为1
    **  8)应用来自DTA文件标志为1,否则为0
    */
    int appStatus;
    //应用隐藏标志，为true时隐藏应用,在config.ini中表现为1隐藏应用
    bool hideFlag;
    //应用ICO图片路径
    QString ICOPicPath;
    //应用Logo/打印图片路径
    QStringList logoPicPath;
    //应用Logo/打印图片中已经删除的图片
    QStringList delPicture;
    //应用设置信息，包括商户号，终端号，通讯方式等等
    QString appSettings;
    //应用最后修改日期
    QString editDate;
    //项目编号
    QString itemCode;
    //应用版本号
    QString version;
    //更新或删除标志,1表示新增或更新,2表示覆盖
    int editType;
    //程序所有者
    QString owner;
    //验证码
    QString checkCode;
    //如果是dta包，保存dta包的路径
    QString dtaPath;
    //UI 脚本路径
    QString uiScriptPath;
    //删除的UI脚本名
    QString delUiScriptName;
};

#endif // CAPPITEMINFO_H
