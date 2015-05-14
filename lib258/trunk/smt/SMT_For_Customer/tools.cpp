#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <QSettings>
#include <QProcess>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QTime>
#include "tools.h"

Tools::Tools()
{
}

void Tools::getSerialPort(QextSerialPort *serialPort)
{
    cpuType = 0;
    mergeType = 0;
    arm7Len = 0;
    arm9Len = 0;
    this->serialPort = serialPort;
}

int Tools::getPrivateProfileInt(QString sessionName, QString keyName, int defValue, QString fileName)
{
    if(fileName.isEmpty())
        fileName = INI_FILE_CONFIG;
     QSettings iniSettings(fileName,QSettings::IniFormat);
     iniSettings.setIniCodec("GB2312");

     iniSettings.beginGroup(sessionName);
     return iniSettings.value(keyName,defValue).toInt();
}

QString Tools::getPrivateProfileString(QString sessionName, QString keyName, QString defValue, QString fileName)
{
    if(fileName.isEmpty())
        fileName = INI_FILE_CONFIG;
     QSettings iniSettings(fileName,QSettings::IniFormat);
     iniSettings.setIniCodec("GB2312");
     iniSettings.beginGroup(sessionName);
     return iniSettings.value(keyName,defValue).toString();

}

bool Tools::writePrivateProfileString(QString sessionName, QString keyName, QString value,QString fileName)
{
    if(fileName.isEmpty())
        fileName = INI_FILE_CONFIG;
     QSettings iniSettings(fileName,QSettings::IniFormat);
     iniSettings.setIniCodec("GB2312");
     iniSettings.beginGroup(sessionName);
     iniSettings.setValue(keyName,value);
     iniSettings.endGroup();
     return true;
}


bool Tools::getFileInfo(QString filePath)
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
    cpuType = 1;

    memset(readBuf,0x00,sizeof(readBuf));

    if(!file.open(QIODevice::ReadOnly))
    {
        *errMsg = QString::fromLocal8Bit("文件\"") + QFileInfo(filePath).fileName() + QString::fromLocal8Bit("\"无法打开！");
        return ret;
    }
    count = file.read((char *)readBuf,sizeof(readBuf)-1);
    *appSize = file.size();
    if(strstr((char *)readBuf,DWN_FILE_HEAD) != NULL)
    {
        point += strlen(DWN_FILE_HEAD);
        arm7Len = *(int *)&readBuf[point];
        point += 4;
        arm9Len = *(int *)&readBuf[point];
        point += 4;
        mergeType = 1;

    }
    //兼容传统dwn
    else
    {
        arm7Len = file.size();
        arm9Len = file.size();
        mergeType = 0;
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
            *appID = 0;
            if(len > 1)
                *appID = readBuf[point++]*256;
            *appID += readBuf[point++];
        }
        else if(tmpTarget == 0x44)
        {
            memset(strBuf,0x00,sizeof(strBuf));
            memcpy(strBuf,&readBuf[point],len);
            point += len;
            *appName = QString::fromLocal8Bit((char *)strBuf,strlen((char *)strBuf));
            ret = true;
        }
        else if(tmpTarget == 0x45)
        {
            memset(strBuf,0x00,sizeof(strBuf));
            memcpy(strBuf,&readBuf[point],len);
            point += len;
            *appVersion = QString::fromLocal8Bit((char *)strBuf,strlen((char *)strBuf));
        }
        else if(tmpTarget == 0x4a)
        {
            memset(strBuf,0x00,sizeof(strBuf));
            memcpy(strBuf,&readBuf[point],len);
            point += len;
            if(strBuf[0] != 0)
                cpuType = (int)strBuf[0];
            ret = true;
            goto _exit;
        }
        else
            point += len;
    }
    if(!ret)
    {

    }
_exit:
    file.close();
    return ret;
}


void Tools::getErrMsg(QString *errMsg)
{
    this->errMsg = errMsg;
}

void Tools::getAppParam(int *appID, QString *appName, QString *appVersion, int *appSize)
{
    this->appID = appID;
    this->appName = appName;
    this->appVersion = appVersion;
    this->appSize = appSize;
}


void Tools::getProgressBar(QProgressBar *proBar)
{
    this->msgProBar = proBar;
}

void Tools::getStatusLabel(QLabel *msgDis)
{
    this->msgDis = msgDis;
}



void Tools::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents();
    }
}

bool Tools::sync(QString filePath)
{
    int start = 0, len = 0;
    unsigned char readBuf[2048];
    unsigned int checkSum = 0;
    int readCount = 0, count = 0, i = 0, armType;
    bool ret = false;
    memset(readBuf,0,sizeof(readBuf));
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        *errMsg = QString::fromLocal8Bit("文件\"") + QFileInfo(filePath).fileName() + QString::fromLocal8Bit("\"无法打开！");
        return ret;
    }

    msgDis->setText(QString::fromLocal8Bit("准备下载..."));
    armType = checkARM(100);

    if(mergeType == 1)
    {
        start += sizeof(DWN_FILE_HEAD) + 8;
        //ARM9
        if(armType == ARM9_CPU_TYPE)
        {
            start += arm7Len;
            len = arm9Len;
        }
        //ARM7
        else if(armType == ARM7_CPU_TYPE)
            len = arm7Len;
    }
    //兼容以前的dwn
    else if(armType == cpuType)
        len = arm7Len;
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
        sleep(1);
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
