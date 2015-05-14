#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <QDir>
#include <QSettings>
#include <QProcess>
#include <QCoreApplication>
#include <QTextStream>
#include <QHeaderView>
#include <QRegExp>
#include <QToolTip>
#include <QFile>
#include <QFileInfo>
#include "tools.h"
#include "libzip.h"

Tools::Tools()
{
}

void Tools::getSerialPort(QextSerialPort *serialPort)
{
    this->serialPort = serialPort;
}

int Tools::getPrivateProfileInt(QString sessionName, QString keyName, int defValue, QString fileName)
{
    if(fileName.isEmpty())
        fileName = INI_FILE_CONFIG;
     QSettings iniSettings(fileName,QSettings::IniFormat);
     iniSettings.setIniCodec(INI_FILE_CODE);

     iniSettings.beginGroup(sessionName);
     return iniSettings.value(keyName,defValue).toInt();
}

QString Tools::getPrivateProfileString(QString sessionName, QString keyName, QString defValue, QString fileName)
{
    if(fileName.isEmpty())
        fileName = INI_FILE_CONFIG;
     QSettings iniSettings(fileName,QSettings::IniFormat);
     iniSettings.setIniCodec(INI_FILE_CODE);
     iniSettings.beginGroup(sessionName);
     return iniSettings.value(keyName,defValue).toString();

}

bool Tools::writePrivateProfileString(QString sessionName, QString keyName, QString value,QString fileName)
{
    if(fileName.isEmpty())
        fileName = INI_FILE_CONFIG;
     QSettings iniSettings(fileName,QSettings::IniFormat);
     iniSettings.setIniCodec(INI_FILE_CODE);
     iniSettings.beginGroup(sessionName);
     iniSettings.setValue(keyName,value);
     iniSettings.endGroup();
     return true;
}

int Tools::mergeDnwFile(QString curArm7Path, QString curArm9Path, QString mergeFilePath)
{
    QFile arm7File(curArm7Path);
    QFile arm9File(curArm9Path);
    QFile mergeFile(mergeFilePath);
    QDir dir;
    char tempBuf[1024];
    int count = 0;
    int length = 0;

    memset(tempBuf,0x00,sizeof(tempBuf));
    if(!dir.mkpath(QFileInfo(mergeFilePath).absolutePath()))
        return -1;
    if(!mergeFile.open(QIODevice::WriteOnly | QIODevice::Truncate) || !arm7File.open(QIODevice::ReadOnly) || !arm9File.open(QIODevice::ReadOnly))
    {
        if(mergeFile.isOpen())
            mergeFile.close();
        if(arm7File.isOpen())
            arm7File.close();
        return -1;
    }
    initMsgPorBar(strlen(DWN_FILE_HEAD)+8+arm7File.size()+arm9File.size());

    memcpy(tempBuf,DWN_FILE_HEAD,strlen(DWN_FILE_HEAD));
    count = strlen(DWN_FILE_HEAD);
    length += count;
    count = arm7File.size();
    memcpy(&tempBuf[length],(char *)&count,4);
    length += 4;
    count = arm9File.size();
    memcpy(&tempBuf[length],(char *)&count,4);
    length += 4;

    mergeFile.write(tempBuf,length);
    msgProBar->setValue(length);
    while(!arm7File.atEnd())
    {
        count = arm7File.read(tempBuf,sizeof(tempBuf));
        mergeFile.write(tempBuf,count);
        length += count;
        msgProBar->setValue(length);
    }

    while(!arm9File.atEnd())
    {
        count = arm9File.read(tempBuf,sizeof(tempBuf));
        mergeFile.write(tempBuf,count);
        length += count;
        msgProBar->setValue(length);
    }
    mergeFile.close();
    arm7File.close();
    arm9File.close();
    return length;
}

void Tools::getProgressBar(QProgressBar *proBar)
{
    this->msgProBar = proBar;
}

void Tools::getStatusLabel(QLabel *msgDis)
{
    this->msgDis = msgDis;
}

void Tools::getAppInfo(QString *appName, int *appID, int *appVersion, int *cpuType)
{
    this->appName = appName;
    this->appID = appID;
    this->appVersion = appVersion;
    this->cpuType = cpuType;
}

void Tools::getErrMsg(QString *errMsg)
{
    this->errMsg = errMsg;
}

void Tools::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents();
    }
}

bool Tools::transmitFileToPos(QString filePath)
{
    int start = 0, len = 0;
    unsigned char readBuf[1024];
    unsigned int checkSum = 0;
    int readCount = 0, count = 0, i = 0, armType;
    bool ret = false;
    DWN_FILE_INFO curFileInfo;
    memset(readBuf,0,sizeof(readBuf));


    clearDwnFileInfo(&curFileInfo);

    if(!getFileInfo(&curFileInfo,filePath))
        return ret;

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        *errMsg = QString::fromLocal8Bit("文件\"") + QFileInfo(filePath).fileName() + QString::fromLocal8Bit("\"无法打开！");
        return ret;
    }

    msgDis->setText(QString::fromLocal8Bit("准备下载..."));
    armType = checkARM(250);

    if(curFileInfo.curMergeType == 1)
    {
        start += sizeof(DWN_FILE_HEAD) + 8;
        //ARM9
        if(armType == ARM9_CPU_TYPE)
        {
            start += curFileInfo.curArm7Len;
            len = curFileInfo.curArm9Len;
        }
        //ARM7
        else if(armType == ARM7_CPU_TYPE)
            len = curFileInfo.curArm7Len;
    }
    //兼容以前的dwn
    else if(armType == curFileInfo.curCpuType)
        len = curFileInfo.curArm7Len;
    else
    {
        *errMsg = QString::fromLocal8Bit("下载文件\"") + QFileInfo(filePath).fileName() + QString::fromLocal8Bit("\"与机器不匹配,请选择正确的文件再下载！");
        goto _exit;
    }

    len += 6;
    initMsgPorBar(len);
    msgDis->setText(QString::fromLocal8Bit("下载中..."));
    readBuf[0] = len%256;
    readBuf[1] = (len/256)%256;
    readBuf[2] = (len/(256*256))%256;
    readBuf[3] = len/(256*256*256);
    serialPort->write((char *)readBuf,4);
    file.seek(start-1);
    count += 4;
    while(count < len-2)
    {
        memset(readBuf,0,sizeof(readBuf));
        readCount = file.read((char *)readBuf,(len-count-2) > sizeof(readBuf) ? sizeof(readBuf):(len-count-2));
        serialPort->write((char *)readBuf,readCount);
        //校验位每一位相加
        for(i=0;i<readCount;i++)
            checkSum += readBuf[i];
        count += readCount;
        msgProBar->setValue(count);
        sleep(20);
    }
    //写入校验位
    memset(readBuf,0,sizeof(readBuf));
    readBuf[0] = checkSum%256;
    readBuf[1] = (checkSum/256)%256;
    serialPort->write((char *)readBuf,2);
    count += 2;
    msgProBar->setValue(count);
    ret = true;
_exit:
    file.close();
    return ret;
}


void Tools::initMsgPorBar(int maximum)
{
    msgDis->clear();
    msgProBar->setMinimum(0);
    msgProBar->setMaximum(maximum);
    msgProBar->setValue(0);
}


int Tools::checkARM(int timeOut)
{
    unsigned char readBuf[1024];
    int readCount = 0;
    while(timeOut-- > 0)
    {
        readCount = serialPort->read((char *)readBuf,sizeof(readBuf));
        if(readCount > 0 && strstr((char *)readBuf,ARM9_CMD) != NULL)
        {
            return ARM9_CPU_TYPE;
        }
        sleep(10);
    }
    return ARM7_CPU_TYPE;
}

bool Tools::convertFileToDwn(QString binFilePath, QString dwnFilePath)
{
    bool ret = false;
    QFile srcFile, dwnFile;
    QDir dir;
    unsigned char *readBuf, *writeBuf;
    long rcn = 0, wcn = 0, bufSize = 0;
    PACK_INFO packInfo;

    memset(&packInfo,0,sizeof(PACK_INFO));


    //判断转换压缩后dwn文件保存路径是否存在
    QString str;
    str = QFileInfo(dwnFilePath).absolutePath();
    if(!QFileInfo(str).exists())
        dir.mkdir(str);

    srcFile.setFileName(binFilePath);
    if(!srcFile.open(QIODevice::ReadOnly))
    {
        *errMsg = QString::fromLocal8Bit("文件\"") + QFileInfo(binFilePath).fileName() + QString::fromLocal8Bit("\"无法打开！");
        return ret;
    }

    //打开要生成的dwn文件
    dwnFile.setFileName(dwnFilePath);
    if(dwnFile.exists())
        dwnFile.remove();
    if(!dwnFile.open(QIODevice::WriteOnly))
    {
        srcFile.close();
        return ret;
    }

    //开内存空间
    bufSize = srcFile.size();
    readBuf = (unsigned char *)calloc(bufSize,1);
    writeBuf = (unsigned char *)calloc(bufSize+500,1);

    if(readBuf == NULL || writeBuf == NULL)
    {
        *errMsg = QString::fromLocal8Bit("开辟内存空间失败！");
        goto _exit;
    }

    rcn = srcFile.read((char *)readBuf,srcFile.size());

    if(rcn != srcFile.size())
    {
        *errMsg = QString::fromLocal8Bit("文件读取出错！");
        goto _exit;
    }
    else if(readBuf[0] != 0x7F || readBuf[1] != 0x45 || readBuf[2] != 0x4C || readBuf[3] != 0x46)
    {
        *errMsg = QString::fromLocal8Bit("文件\"") + QFileInfo(binFilePath).fileName() + QString::fromLocal8Bit("\"不是可执行文件,请正确选择文件！");
        goto _exit;
    }
    initMsgPorBar(100);
    //填充PACK_INFO结构体
    strncpy((char *)packInfo.binname,QFileInfo(binFilePath).fileName().toLocal8Bit().data(),sizeof(packInfo.binname));
    strncpy((char *)packInfo.bindesc,appName->toLocal8Bit().data(),sizeof(packInfo.bindesc));
    packInfo.bintypecode = *appID;
    packInfo.bintype = 2;
    packInfo.bincputype = (unsigned char)*cpuType;
    if(*appVersion <= 9)
    {
        packInfo.binversion[0] = '0';
        packInfo.binversion[1] = *appVersion + '0';
    }
    else
    {
        packInfo.binversion[0] = *appVersion/10 + '0';
        packInfo.binversion[1] = *appVersion%10 + '0';
    }
    wcn = ZIP_NRVData2Compress(&packInfo,(char *)readBuf,rcn,(char *)writeBuf,wcn);

    dwnFile.write((char *)writeBuf,wcn);
    ret = true;
    msgProBar->setValue(100);
_exit:
    free(readBuf);
    free(writeBuf);
    srcFile.close();
    dwnFile.close();
    return ret;
}


bool Tools::getFileInfo(DWN_FILE_INFO *curFileInfo, QString filePath)
{
    QFile file(filePath);
    unsigned char readBuf[1024];
    unsigned char strBuf[50];
    unsigned char tmpTarget;
    int len = 0;
    int point = 0;
    int count = 0;
    bool ret = false;
    //默认下载程序为ARM7
    curFileInfo->curCpuType = 1;

    memset(readBuf,0x00,sizeof(readBuf));

    if(!file.open(QIODevice::ReadOnly))
    {
        *errMsg = QString::fromLocal8Bit("文件\"") + QFileInfo(filePath).fileName() + QString::fromLocal8Bit("\"无法打开！");
        return ret;
    }
    count = file.read((char *)readBuf,sizeof(readBuf)-1);

    if(strstr((char *)readBuf,DWN_FILE_HEAD) != NULL)
    {
        point += strlen(DWN_FILE_HEAD);
        curFileInfo->curArm7Len = *(int *)&readBuf[point];
        point += 4;
        curFileInfo->curArm9Len = *(int *)&readBuf[point];
        point += 4;
        curFileInfo->curMergeType = 1;

    }
    //兼容传统dwn
    else
    {
        curFileInfo->curArm7Len = file.size();
        curFileInfo->curArm9Len = file.size();
        curFileInfo->curMergeType = 0;
    }

    if(strstr((char *)&readBuf[point],"%DWN-1.0-SAND%") == NULL)
    {
        *errMsg = QString::fromLocal8Bit("下载文件\"") + QFileInfo(filePath).fileName() + QString::fromLocal8Bit("\"格式错误！");
        goto _exit;
    }

    point += 14;
    while(point < count)
    {
        tmpTarget = readBuf[point++];
        len = readBuf[point++];
        if(tmpTarget == 0x42)
        {
            curFileInfo->curAppID = 0;
            if(len > 1)
                curFileInfo->curAppID = readBuf[point++]*256;
            curFileInfo->curAppID += readBuf[point++];
        }
        else if(tmpTarget == 0x44)
        {
            memset(strBuf,0x00,sizeof(strBuf));
            memcpy(strBuf,&readBuf[point],len);
            point += len;
            curFileInfo->curAppName = QString::fromLocal8Bit((char *)strBuf,strlen((char *)strBuf));
            ret = true;
        }
        else if(tmpTarget == 0x45)
        {
            memset(strBuf,0x00,sizeof(strBuf));
            memcpy(strBuf,&readBuf[point],len);
            point += len;
            curFileInfo->curAppVersion = atoi((char *)strBuf);
        }
        else if(tmpTarget == 0x4a)
        {
            memset(strBuf,0x00,sizeof(strBuf));
            memcpy(strBuf,&readBuf[point],len);
            point += len;
            if(strBuf[0] != 0)
                curFileInfo->curCpuType = (int)strBuf[0];
            ret = true;
            goto _exit;
        }
        else
            point += len;
    }
    if(!ret)
    {
        *errMsg = QString::fromLocal8Bit("下载文件\"") + QFileInfo(filePath).fileName() + QString::fromLocal8Bit("\"格式错误！");
    }
_exit:
    file.close();
    return ret;
}

void Tools::clearDwnFileInfo(DWN_FILE_INFO *curFileInfo)
{
    curFileInfo->curAppID = 0;
    curFileInfo->curAppName.clear();
    curFileInfo->curAppVersion = 0;
    curFileInfo->curArm7Len = 0;
    curFileInfo->curArm9Len = 0;
    curFileInfo->curCpuType = 1;
    curFileInfo->curMergeType = 0;
}

bool Tools::getBinFileInfo(BIN_FILE_INFO *binFileInfo)
{
     QSettings iniSettings(INI_FILE_CONFIG,QSettings::IniFormat);
     QStringList childkey;
     int count = 0, i = 0;
     QString name;
     iniSettings.setIniCodec(INI_FILE_CODE);

     iniSettings.beginGroup("BinFileInfo");
     childkey = iniSettings.childGroups();
     count = childkey.count();
     for(i=0;i < count;i++)
     {
         iniSettings.beginGroup(QString::number(i));
         name = iniSettings.value("NAME").toString();
         if(!name.isEmpty() && binFileInfo->appName.compare(name) == 0)
         {
             binFileInfo->appID = iniSettings.value("ID").toString();
             binFileInfo->appVersion = iniSettings.value("VERSION").toString();
             binFileInfo->cpuType = iniSettings.value("CPUTYPE","1").toInt();
             return true;
         }
         iniSettings.endGroup();
     }
     iniSettings.endGroup();

    return false;
}

int Tools::setBinFileInfo(BIN_FILE_INFO *binFileInfo)
{
    QSettings iniSettings(INI_FILE_CONFIG,QSettings::IniFormat);
    QStringList childkey;
    int count = 0, i = 0;
    QString name;
    iniSettings.setIniCodec(INI_FILE_CODE);
    iniSettings.beginGroup("BinFileInfo");
    childkey = iniSettings.childGroups();
    count = childkey.count();

    for(i=0;i<count;i++)
    {
        iniSettings.beginGroup(QString::number(i));
        name = iniSettings.value("NAME").toString();
        if(!name.isEmpty() && name.compare(binFileInfo->appName) == 0)
        {
            iniSettings.setValue("ID",binFileInfo->appID);
            iniSettings.setValue("VERSION",binFileInfo->appVersion);
            iniSettings.setValue("CPUTYPE",QString::number(binFileInfo->cpuType));
            return i;
        }
        iniSettings.endGroup();
    }
    if(i > 9)
    {
        iniSettings.beginGroup("9");
    }
    else
        iniSettings.beginGroup(QString::number(i));
    iniSettings.setValue("NAME",binFileInfo->appName);
    iniSettings.setValue("ID",binFileInfo->appID);
    iniSettings.setValue("VERSION",binFileInfo->appVersion);
    iniSettings.setValue("CPUTYPE",QString::number(binFileInfo->cpuType));
    iniSettings.endGroup();
    iniSettings.endGroup();
    if(i>9)
        return 9;
    else
        return i;
}

void Tools::getBinFileInfoList(QStringList *binFileNameList)
{
    QSettings iniSettings(INI_FILE_CONFIG,QSettings::IniFormat);
    int count = 0, i = 0;
    QString name;
    iniSettings.setIniCodec(INI_FILE_CODE);
    iniSettings.beginGroup("BinFileInfo");
    count = iniSettings.childGroups().count();
    if(count > 0)
        *binFileNameList<<"";
    for(i=0;i<count;i++)
    {
        iniSettings.beginGroup(QString::number(i));
        name = iniSettings.value("NAME").toString();
        *binFileNameList<<name;
        iniSettings.endGroup();
    }
    iniSettings.endGroup();
}
