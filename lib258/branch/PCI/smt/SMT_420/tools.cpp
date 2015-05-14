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
#include <QObject>
#include "tools.h"
#include "libnn.h"
#include "librsa.h"
#include "libsha1.h"


Tools::Tools()
{
    sendDtaSuccess = TOOLS_ERROR_INITVAL;
    resourceIcoPath<<":resource/ico/app.png"<<":resource/ico/daemon.png"<<":resource/ico/lib.png"
                   <<":resource/ico/driver.png"<<":resource/ico/font.png"<<":resource/ico/other.png"
                   <<":resource/ico/shell.png"<<":resource/ico/uboot.png"<<":resource/ico/kernel.png"
                   <<":resource/ico/srk.png"<<":resource/ico/file_system.png"<<":resource/ico/other_boot.png";
    tableHeader<<"APP"<<"DAEMON"<<"LIBRARY"<<"DRIVER"<<"FONT"<<"DATA"<<"SHELL"<<"U-BOOT"
              <<"KERNEL"<<"CERTIFICATE"<<"FILE SYSTEM"<<"OTHER BOOT";
    tableDesc<<QObject::tr("APP")<<QObject::tr("DAEMON")<<QObject::tr("LIBRARY")<<QObject::tr("DRIVER")
                  <<QObject::tr("FONT")<<QObject::tr("DATA")<<QObject::tr("SHELL")<<QObject::tr("U-BOOT")
                  <<QObject::tr("KERNEL")<<QObject::tr("CERTIFICATE")<<QObject::tr("FILE SYSTEM")<<QObject::tr("OTHER BOOT");

    settingMoidfy = 0;
}
unsigned char Tools::OSSPD_tool_lrc(unsigned char *pointChar, int length)
{
    unsigned char Lrc1;
    int i= 0;
    Lrc1 = 0xFF;
    while(i++ < length)
        Lrc1 ^= *pointChar++;

    return (Lrc1);
}

int Tools::checkPakHeader(char *cmdPonitStr, int *readCount, int timeOut)
{
    unsigned char tempBuf[18];
    QString msg;
    memset(tempBuf,0x00,sizeof(tempBuf));
    int count = 0, ret = 0;
    int upsVer = 0;
    int length = 14;
    if(tempBuf[1] == DOWNCMD_REQUEST)
        length = 17;
    else if(tempBuf[1] == DOWNCMD_RESPFILE)
        length = 18;
    if(*readCount >= length)
    {
        memcpy(tempBuf,cmdPonitStr,length);

        if(tempBuf[12] == SPD_ETX && tempBuf[13] == OSSPD_tool_lrc(tempBuf,13))
        {
            //检查UpdateService版本是否正确
            upsVer = (tempBuf[2]-'0')*10 +tempBuf[3]-'0';
            if(upsVer >= UPDATESERVICE_VERSION)
                return 0;
            else
            {
                if(tempBuf[2] == '\0')
                    msg = QObject::tr("The POS version(0.0) is lower, please update!");
                else
                    msg = QObject::tr("The POS version(") + QString(tempBuf[2]) + "." + QString(tempBuf[3]) + QObject::tr(") is lower, please update!");

                msgDis->setText(msg);
                return -1;
            }
        }
    }
    else
    {
        while(*readCount < length)
        {
            if(timeOut <= 0)
                break;
            memcpy(tempBuf,cmdPonitStr,*readCount);
            sleep(200);
            count = *readCount;
            ret = read((char *)&tempBuf[count],length-count);
            if(ret < 0)
            {
                return -2;
            }
            *readCount += ret;

            memcpy(cmdPonitStr,tempBuf,*readCount);
            timeOut--;
        }
        if(*readCount >= length)
            return checkPakHeader((char *)tempBuf,readCount,0);
    }

    return -2;
}

int Tools::processRecvStream(unsigned char *recvBuf, int recvLength)
{
    unsigned char recvTempBuf[BUF_MAX_SIZE];
    unsigned int length;
    int result = -1;
    int readCount = 0;

    memset(recvTempBuf,0x00,sizeof(recvTempBuf));
    memcpy(recvTempBuf,recvBuf,recvLength);

    //请求
    if(recvTempBuf[1] == DOWNCMD_REQUEST)
    {
        if(recvLength - 14 < 1)
        {
            for(int i=0;i<10;i++)
            {
                readCount = read((char*)&recvTempBuf[recvLength],1024-recvLength);
                if(readCount > 0)
                {
                    recvLength += readCount;
                    break;
                }

                if(i == 9 || readCount < 0)
                    goto _exit;
                sleep(500);
            }
        }
        length = recvTempBuf[14];

        if(recvLength - 15 < (int)length)
        {
            for(int i=0;i<10;i++)
            {
                readCount = read((char*)&recvTempBuf[recvLength],1024-recvLength);
                if(readCount > 0)
                {
                    recvLength += readCount;
                    break;
                }

                if(i == 9 || readCount < 0)
                    goto _exit;
                sleep(100);
            }
        }
        //如果长度为1且内容为0x00，则表明握手成功
        if(length == 1 && recvTempBuf[15] == 0x00)
            ;//握手成功
        else if(length == 2 && recvTempBuf[15] == 'o' && recvTempBuf[16] == 'k')
        {
            sendDtaSuccess = TOOLS_ERROR_SUCCESS;
        }
        else if(length == 2 && recvTempBuf[15] == 'n' && recvTempBuf[16] == 'o')
        {
            sendDtaSuccess = TOOLS_ERROR_SIGNERR;
        }
    }
    //文件
    else if(recvTempBuf[1] == DOWNCMD_RESPFILE)
    {
        //文件长度,四个字节的hex码
        length = recvTempBuf[14]*256*256*256 + recvTempBuf[15]*256*256
                             + recvTempBuf[16]*256 + recvTempBuf[17];

        QFile zipFile(UPDATE_DATA_NAME);
        if(zipFile.exists())
            zipFile.remove();
        QFile tempFile(UPDATE_DATA_TEMP_NAME);
        if(tempFile.exists())
            tempFile.remove();

        int writeCount = 0;

        if(tempFile.open(QIODevice::WriteOnly) &&(recvLength-18 == tempFile.write((char *)&recvTempBuf[18],recvLength-18)))
        {
            readCount = 0;
            writeCount += (recvLength-18);
            initMsgPorBar(length);
            msgProBar->setValue(writeCount);
            int timeOutCount = 0;

            while((unsigned int)writeCount < length)
            {
                timeOutCount++;
                memset(recvTempBuf,0x00,sizeof(recvTempBuf));
                sleep(10);
                readCount = read((char*)recvTempBuf,sizeof(recvTempBuf));
                if(readCount > 0)
                {
                    if(writeCount+readCount > (int)length)
                    {
                        tempFile.write((char*)recvTempBuf,length-writeCount);
                        writeCount = length;
                    }
                    else
                    {
                        tempFile.write((char*)recvTempBuf,readCount);
                        writeCount += readCount;
                    }
                    timeOutCount = 0;
                    msgProBar->setValue(writeCount);
                }
                if(timeOutCount > 600 || readCount < 0)
                {
                    tempFile.remove();
                    tempFile.close();
                    goto _exit;
                }
            }
            tempFile.rename(UPDATE_DATA_NAME);
            tempFile.close();
            result = writeCount;
            goto _exit;
        }

    }
    //按键，只有网络状态下才有按键
    else if(recvTempBuf[1] == DOWNCMD_RESPKEY)
    {

    }
_exit:
    return result;
}

int Tools::packgeCommand(int cmdType, QString name, unsigned char cmdArray[][2], int nCmd)
{
    unsigned char commandBuf[BUF_MAX_SIZE];
    int commandLenth = 0, ret = -1;
    QFile file;
    if(!checkTransPort())
        return -1;
    memset(commandBuf,0x00,sizeof(commandBuf));

    commandBuf[0] = SPD_STX;
    commandBuf[1] = cmdType;
    commandBuf[12] = SPD_ETX;
    commandBuf[13] = OSSPD_tool_lrc(commandBuf, 13);
    commandLenth += 14;
    //请求
    if(cmdType == DOWNCMD_REQUEST)
    {
        int count = nCmd;
        commandBuf[14] = (unsigned char)(count*4 + 1);
        commandLenth++;

        for(int i=0;i<count;i++)
        {
            commandBuf[14 + 1 + 4*i] = cmdArray[i][0];
            commandBuf[14 + 2 + 4*i] = cmdArray[i][1];
            commandBuf[14 + 3 + 4*i] = name.toInt()>>8 & 0xff;
            commandBuf[14 + 4 + 4*i] = name.toInt() & 0xff;
            commandLenth += 4;
        }
        commandBuf[commandLenth] = OSSPD_tool_lrc(commandBuf+15,count*4);
        commandLenth++;
        ret = write((char *)commandBuf, commandLenth);
        if(ret < 0)
            goto _exit;
        ret = commandLenth;
    }
    //文件
    else if(cmdType == DOWNCMD_RESPFILE)
    {
        file.setFileName(name);
        //QFile file(name);
        if(!file.exists())
        {
            goto _exit;
        }
        unsigned long fileLength = file.size();

        initMsgPorBar(fileLength+18);

        commandBuf[14] = fileLength/(256*256*256);
        commandBuf[15] = fileLength/(256*256)%256;
        commandBuf[16] = fileLength/256%256;
        commandBuf[17] = fileLength%256;

        commandLenth += 4;

        //将命令头和数据的长度写入com,并将其buf和长度清空
        ret = write((char *)commandBuf, commandLenth);
        if(ret < 0)
        {
            goto _exit;
        }
        msgProBar->setValue(commandLenth);
        commandLenth = 0;
        memset(commandBuf,0x00,sizeof(commandBuf));
        if(!file.open(QIODevice::ReadOnly))
            goto _exit;;
        //将文件长度制0，用于记录写入长度
        fileLength = 0;
        while(!file.atEnd())
        {
            if((commandLenth = file.read((char *)commandBuf,sizeof(commandBuf))) > 0)
            {
                ret = write((char *)commandBuf, commandLenth);
                if(ret < 0)
                {
                    goto _exit;
                }
                fileLength += commandLenth;
            }
            msgProBar->setValue(fileLength+18);
            commandLenth = 0;
            memset(commandBuf,0x00,sizeof(commandBuf));
            sleep(3);
        }
        ret = fileLength;
    }
    //按键
    else if(cmdType == DOWNCMD_RESPKEY)
    {

    }
_exit:
    if(cmdType == DOWNCMD_RESPFILE && file.isOpen())
        file.close();
    return ret;
}

void Tools::deleteDirectory(QFileInfo fileList)
{
    if(fileList.isDir())
    {
        int childCount =0;
        QString dir = fileList.filePath();
        QDir thisDir(dir);
        childCount = thisDir.entryInfoList().count();
        QFileInfoList newFileList = thisDir.entryInfoList();
        if(childCount>2)
        {
            for(int i=0;i<childCount;i++)
            {
                if(newFileList.at(i).fileName().operator ==(".")|newFileList.at(i).fileName().operator ==(".."))
                {
                    continue;
                }
                deleteDirectory(newFileList.at(i));
            }
        }
        fileList.absoluteDir().rmpath(fileList.fileName());
    }
    else if(fileList.isFile())
    {
        fileList.absoluteDir().remove(fileList.fileName());
    }
}


bool Tools::addItemToListWidget(void *item, int type)
{
    CAPPItemInfo *appItem;
    CSYSItemInfo *sysItem;
    QIcon icon;
    QListWidgetItem *listWidgetItem;
    QColor color;
    QString itemDesc;
    QString itemName;

    if(type == 0)
    {
        appItem = (CAPPItemInfo *)item;

        //应用数量
        int itemCount = 0;
        int deleteIndex = appItem->isItemExist(itemAppDelList);
        itemCount = itemAppList->count();
        //遍历项目，查看是否有所要添加的项目已经存在
        if(appItem->isItemExist(itemAppList) >= 0)
            return false;

        if(appItem->ICOPicPath.isEmpty())
            icon = QIcon(":resource/ico/app.png");
        else
            icon = QIcon(appItem->ICOPicPath);

        if(appItem->appDesc.size() > 8)
            itemName = appItem->appDesc.leftRef(6).toString() + "..";
        else
            itemName = appItem->appDesc.leftRef(8).toString();

        appItem->addItem(itemAppList);
        listWidgetItem = new QListWidgetItem(icon,itemName,0,1000);
        listWidgetItem->setFlags(listWidgetItem->flags()&(~(Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled)));

        appListWidget->addItem(listWidgetItem);
        appListWidget->setCurrentRow(appListWidget->count()-1);
        //删除列表中有此应用，默认更新
        if(deleteIndex != -1)
        {
            appItem->editType = 1;
            itemAppDelList->removeAt(deleteIndex);
        }
        return true;
    }
    else
    {
        sysItem = (CSYSItemInfo *)item;
        int index = 0;
        int typeNumber = 0;
        int replaceIndex = sysItem->isItemExist(itemSysList);
        int deleteIndex = sysItem->isItemExist(itemSysDelList);


        typeNumber = 1000 + sysItem->type;
        icon = QIcon(resourceIcoPath.at(type));
        if(type == LIB || type == DRIVER || type == FONT)
        {
            if(sysItem->srcName.size()-3 > 8)
                itemName = sysItem->srcName.leftRef(6).toString() + "..";
            else
                itemName = sysItem->srcName.leftRef(sysItem->srcName.size()-3).toString();
        }
        else
        {
            if(sysItem->srcName.size() > 8)
                itemName = sysItem->srcName.leftRef(6).toString() + "..";
            else
                itemName = sysItem->srcName.leftRef(8).toString();
        }

        listWidgetItem = new QListWidgetItem(icon,itemName,0,typeNumber);
        listWidgetItem->setFlags(listWidgetItem->flags()&(~(Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled)));
        itemDesc = getPrivateProfileString(tableHeader.at(type),sysItem->srcName,tableDesc.at(type),INI_FILE_DESC);
        listWidgetItem->setToolTip("\n  Name:  " + sysItem->srcName + "\n  Version:  "
                                   + sysItem->version + "\n  Brief:  " + itemDesc + "\n");
        if((sysItem->status & 0x20) == 0x20)
        {
            color = QColor("#F5D0A9");
            listWidgetItem->setBackgroundColor(color);
        }
        //将Item添加到SYS中
        if(replaceIndex != -1)
        {
            //文件名称相同但源文件路径不同，更新文件
            if(sysItem->srcPath.compare(itemSysList->at(replaceIndex).srcPath) != 0)
            {
                sysItem->status |= (itemSysList->at(replaceIndex).status & 0x10);
                sysItem->replaceItem(replaceIndex,itemSysList);
                sysListWidget->insertItem(replaceIndex,listWidgetItem);
                sysListWidget->takeItem(replaceIndex+1);

                sysListWidget->setCurrentRow(replaceIndex);
            }
            else
                return false;

        }
        else
        {
            if(deleteIndex != -1)
                itemSysDelList->removeAt(deleteIndex);

            //确定插入位置
            for(int i=0;i<itemSysList->count();i++)
            {
                if(itemSysList->at(i).type <= type)
                    index++;
            }
            //如果是KO类型，并且列表中无KO文件，条件drivers.sh
            if(type == DRIVER && getItemCount(DRIVER) == 0)
            {
                CSYSItemInfo itemTemp;
                itemTemp.resetItem();
                itemTemp.type = SHELL;
                itemTemp.srcName = "drivers.sh";
                itemTemp.status = 0x30;
                addItemToListWidget(&itemTemp,SHELL);
            }
            else if(type == DAEMON && getItemCount(DAEMON) == 0)
            {
                CSYSItemInfo itemTemp;
                itemTemp.resetItem();
                itemTemp.type = SHELL;
                itemTemp.srcName = "daemon.sh";
                itemTemp.status = 0x30;
                addItemToListWidget(&itemTemp,SHELL);
            }
            sysItem->addItem(itemSysList);
            sysListWidget->insertItem(index,listWidgetItem);
            sysListWidget->setCurrentRow(index);
        }

        return true;
    }
}


void Tools::showImage(QString imagePath)
{
    QIcon icon;
    QListWidgetItem *listWidgetItem;
    QFileInfo fileInfo;
    int index = 0;
    QString name;
    if(imagePath.isEmpty())
    {
        icon = QIcon(resourceIcoPath.at(0));
        name = "app.png";
    }
    else
    {
        fileInfo = QFileInfo(imagePath);
        if(fileInfo.suffix().contains("ico") || fileInfo.fileName().contains("app"))
        {
            if(fileInfo.exists())
            {
                icon = QIcon(imagePath);
                name = "app." + fileInfo.suffix();
            }
            else
            {
                icon = QIcon(resourceIcoPath.at(0));
                name = "app.png";
            }
            if(imageListWidget->count() > 0)
                imageListWidget->takeItem(0);
        }
        else
        {
            icon = QIcon(imagePath);
            name = fileInfo.fileName();
            index = imageListWidget->count();
        }
    }
    if(name.length() > 8)
    {
        name = name.leftRef(6).toString() + "...";
    }
    listWidgetItem = new QListWidgetItem(icon,name);
    listWidgetItem->setFlags(listWidgetItem->flags()&(~(Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled)));
    imageListWidget->insertItem(index,listWidgetItem);
}


void Tools::addImageToListWidget(QString imagePath, QString *curIcoPath, QStringList *curLogoPath, QStringList *curDelPicture)
{
    QFileInfo fileInfo(imagePath);
    int i;
    //图标文件只能是.ico文件或名称为app.png的文件
    if(fileInfo.suffix().contains("ico") || fileInfo.fileName().contains("app"))
    {
        if(curIcoPath->isEmpty() || !curIcoPath->contains(imagePath))
        {
            *curIcoPath = imagePath;
            showImage(imagePath);
        }
    }
    else
    {
        for(i=0;i<curLogoPath->count();i++)
        {
            //图片已存在
            if(imagePath.contains(curLogoPath->at(i)))
                return;
        }
        *curLogoPath<<imagePath;
        //如果添加的文件名在删除列表中存在,则删除删除列表中的文件名

        for(i=0;i<curDelPicture->count();i++)
        {
            if(fileInfo.fileName().compare(curDelPicture->at(i)) == 0)
            {
                curDelPicture->takeAt(i);
                break;
            }
        }
        showImage(imagePath);
    }
}



/* @brief 获取itemType类型的item的个数,如果listType为0,返回已经添加的Item个数,否则返回的是已经删除的Item个数
 * @param itemType 应用的类型
 * @param listType 应用列表类型，0为已添加列表，否则为删除列表
 * @return 返回Item的个数
*/
int Tools::getItemCount(int itemType, int listType)
{
    int count = 0;
    if(itemType == 0)
    {
        if(listType == 0)
            count = itemAppList->count();
        else
            count = itemAppDelList->count();
    }
    else
    {
        if(listType == 0)
            for(int i=0;i<itemSysList->count();i++)
            {
                if(itemSysList->at(i).type == itemType)
                    count++;
            }
        else
            for(int i=0;i<itemSysDelList->count();i++)
            {
                if(itemSysDelList->at(i).type == itemType)
                    count++;
            }
    }
    return count;
}


int Tools::getUpdateItemCount(int itemType)
{
    int count = 0;

    if(itemType == 0)
    {
        for(int i=0;i<itemAppList->count();i++)
        {
            if((itemAppList->at(i).appStatus & 0x20) == 0x20)
                count++;
        }
    }
    else
    {
        for(int i=0;i<itemSysList->count();i++)
        {
            if(itemSysList->at(i).type == itemType && ((itemSysList->at(i).status & 0x20) == 0x20))
                count++;
        }
    }

    return count;
}



bool Tools::modifyItemOnListWidget(void *item, int itemType)
{
    QIcon icon;
    QColor color;
    CAPPItemInfo *appItem;
    CSYSItemInfo *sysItem;
    QListWidgetItem *listWidgetItem;
    int index = -1;

    if(itemType == APPLICATION)
    {
        //应用数量
        int itemCount = 0;
        appItem = (CAPPItemInfo *)item;
        int replaceIndex = appItem->isItemExist(itemAppList);
        int deleteIndex = appItem->isItemExist(itemAppDelList);
        index = appListWidget->currentRow();
        itemCount = itemAppList->count();
        //遍历项目，查看是否有所要添加的项目已经存在
        if(index >= itemCount)
            return false;

        if(appItem->ICOPicPath.isEmpty())
            icon = QIcon(resourceIcoPath.at(0));
        else
            icon = QIcon(appItem->ICOPicPath);


        listWidgetItem = new QListWidgetItem(icon,appItem->appDesc.leftRef(8).toString(),0,1000);
        listWidgetItem->setFlags(listWidgetItem->flags()&(~(Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled)));
        color = QColor("#F5D0A9");
        listWidgetItem->setBackgroundColor(color);

        if(index != replaceIndex && replaceIndex != -1)
        {
            itemAppList->replace(replaceIndex,*appItem);
            itemAppList->takeAt(index);
            appListWidget->insertItem(replaceIndex,listWidgetItem);
            appListWidget->takeItem(replaceIndex+1);
            appListWidget->takeItem(index);
            appListWidget->setCurrentRow(replaceIndex);
        }
        else
        {
            //删除列表中有此应用，相当于覆盖
            if(deleteIndex != -1)
            {
                appItem->editType = 2;
                itemAppDelList->removeAt(deleteIndex);
            }
            itemAppList->replace(index,*appItem);
            appListWidget->takeItem(index);
            appListWidget->insertItem(index,listWidgetItem);
            appListWidget->setCurrentRow(index);
        }
        return true;
    }
    else
    {
        sysItem = (CSYSItemInfo *)item;
        index = sysListWidget->currentRow();
        //将Item添加到SYS中
        if(index >= itemSysList->count())
            return false;
        itemSysList->replace(index,*sysItem);
        return true;
    }
}


bool Tools::swapItemOnListWidget(int moveType)
{
    int index = appListWidget->currentRow();
    int countItem = itemAppList->count();
    CAPPItemInfo itemTemp = itemAppList->at(index);
    QIcon icon;
    QColor color;

    if(itemTemp.ICOPicPath.isEmpty())
        icon = QIcon(":resource/ico/app.png");
    else
        icon = QIcon(itemTemp.ICOPicPath);
    QListWidgetItem *listWidItemTemp = new QListWidgetItem(icon,itemTemp.appDesc.leftRef(8).toString(),0,1000);
    listWidItemTemp->setFlags(listWidItemTemp->flags()&(~(Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled)));

    if(moveType == PREV)
    {
        if(index > 0)
        {
            appListWidget->insertItem(index-1,listWidItemTemp);
            appListWidget->takeItem(index+1);
            itemAppList->replace(index,itemAppList->at(index-1));
            itemAppList->replace(index-1,itemTemp);
            appListWidget->setCurrentRow(index-1);
        }
    }
    else if(moveType == NEXT)
    {
        if(index < countItem-1)
        {
            appListWidget->insertItem(index+2,listWidItemTemp);
            appListWidget->takeItem(index);
            itemAppList->replace(index,itemAppList->at(index+1));
            itemAppList->replace(index+1,itemTemp);
            appListWidget->setCurrentRow(index+1);
        }
    }

    color = QColor("#F5D0A9");
    listWidItemTemp->setBackgroundColor(color);

    return true;
}


bool Tools::removeItemOnListWidget(int itemType)
{
    int index;
    if(itemType == APPLICATION)
    {
        CAPPItemInfo itemTemp;
        index = appListWidget->currentRow();
        appListWidget->takeItem(index);
        itemTemp = itemAppList->takeAt(index);

        //移除Item后，重新设置当前选择项
        if(appListWidget->count() > 0)
        {
            if(appListWidget->count() > index)
                appListWidget->setCurrentRow(index);
            else
                appListWidget->setCurrentRow(index-1);
        }


        //如果移除的Item来自POS，将其放在删除列表中
        if((itemTemp.appStatus & 0x10) == 0x00)
        {
            int i;
            for(i=0;i<itemAppDelList->count();i++)
            {
                if(itemTemp.appNumber == itemAppDelList->at(i).appNumber)
                    break;
            }
            if(i >= itemAppDelList->count())
            {
                itemTemp.editType = 3;
                itemAppDelList->append(itemTemp);
            }
        }
        /*
        //检查删除列表中是否有被删除的Item与此ItemID相同，如果有，恢复
        else
        {
            int i;
            for(i=0;i<itemDelList->count();i++)
            {
                if(itemTemp.appNumber == itemDelList->at(i).appNumber)
                {
                    itemTemp = itemDelList->at(i);
                    itemTemp.editType = 1;
                    addItemToListWidget(listWidget,&itemTemp,itemList);
                    itemDelList->takeAt(i);
                    break;
                }
            }
        }*/
        return true;
    }
    else
    {
        index = sysListWidget->currentRow();
        CSYSItemInfo itemTemp;
        sysListWidget->takeItem(index);
        itemTemp = itemSysList->takeAt(index);
        //如果驱动为零,同时删除drivers.sh
        if(itemType == DRIVER && getItemCount(DRIVER) == 0)
        {
            for(int i=0;i<itemSysList->count();i++)
            {
                if(itemSysList->at(i).type == SHELL && itemSysList->at(i).srcName.contains("drivers.sh"))
                {
                    sysListWidget->takeItem(i);
                    itemSysList->takeAt(i);
                    break;
                }
            }
        }
        else if(itemType == DAEMON && getItemCount(DAEMON) == 0)
        {
            for(int i=0;i<itemSysList->count();i++)
            {
                if(itemSysList->at(i).type == SHELL && itemSysList->at(i).srcName.contains("daemon.sh"))
                {
                    sysListWidget->takeItem(i);
                    itemSysList->takeAt(i);
                }
            }
        }

        //删除后重新设置当前所选项
        if(sysListWidget->count() > 0)
        {
            if(sysListWidget->count() > index)
                sysListWidget->setCurrentRow(index);
            else
                sysListWidget->setCurrentRow(index-1);
        }

        //如果是来自POS将其添加到POS列表
        if((itemTemp.status & 0x10) == 0x00)
        {
            int i;
            for(i=0;i<itemSysDelList->count();i++)
            {
                if(itemTemp.srcName == itemSysDelList->at(i).srcName && itemType == itemSysDelList->at(i).type)
                    break;
            }
            if(i >= itemSysDelList->count())
            {
                itemSysDelList->append(itemTemp);
            }
        }
        /*
        //如果被删除的应用在删除列表中
        else
        {
            int i;
            for(i=0;i<itemDelList->count();i++)
            {
                if(itemTemp.srcName == itemDelList->at(i).srcName && itemType == itemDelList->at(i).type)
                {
                    itemTemp = itemDelList->at(i);
                    addItemToListWidget(listWidget,&itemTemp,itemList);
                    itemDelList->takeAt(i);
                    break;
                }
            }
        }*/

        return true;
    }
}


bool Tools::getItemInfoFromPos(unsigned char cmdArray[][2], int nCmd)
{

    if(cmdArray[0][0] == 0x35 || cmdArray[0][0] == 0x31)
    {
        int index = appListWidget->currentRow();
        QString name;
        CAPPItemInfo itemTemp;
        if(cmdArray[0][0] == 0x31 && index != -1)
        {
            itemTemp = itemAppList->at(index);
            name = QString::number(itemTemp.appNumber,10);
            if((itemTemp.appStatus & 0x50) != 0x40)
                return false;
        }
        if(packgeCommand(DOWNCMD_REQUEST,name,cmdArray,nCmd) > 0 && unpack(cmdArray))
            return true;
        else
            return false;
    }
    else
    {
        if(packgeCommand(DOWNCMD_REQUEST,NULL,cmdArray,nCmd) > 0 && unpack(cmdArray))
            return true;
        else
            return false;
    }

}


//目前只支持APP
bool Tools::addDtaToListWidget(QString dtaPath)
{
    bool result = false;
    int itemType;
    QDir dir;
    QProcess process;
    QString cmd;
    QTime curTime = QTime::currentTime();
    QDate curDate = QDate::currentDate();
    QString basePath = "DTA";
    basePath += "/" + curDate.toString("yyMMdd")+curTime.toString("hhmmsszzz");
    QFileInfo fileInfo(dtaPath);
    if(fileInfo.fileName().contains("driver_lib_font"))
        itemType = DRIVER;
    else
        itemType = APPLICATION;


    //创建文件夹
    if(!dir.mkpath(basePath))
        goto _exit;
    cmd = "DTA.exe x -ibck -y -o+ "+QFileInfo(dtaPath).absoluteFilePath()+" "+QFileInfo(basePath).absoluteFilePath();
    if(process.execute(cmd) == 0)
    {
        if(itemType == APPLICATION)
        {
            CAPPItemInfo item;
            item.dtaPath = dtaPath;
            sortingFileDir(&item,basePath);
            if(item.appNumber <= 0 || !addItemToListWidget(&item, APPLICATION))
                goto _exit;
            result = true;
        }
        else
        {
            CSYSItemInfo item;
            item.dtaPath = dtaPath;
            sortingFileDir(&item,basePath);
            result = true;
        }

    }

_exit:
    return result;
}


bool Tools::unpack(unsigned char cmdArray[][2])
{
    QFileInfo fileInfo;
    QProcess process;
    QString cmd;
    QString iniPath;
    int count = 250;
    int index = -1;
    if(cmdArray[0][0] == 0x31 || cmdArray[0][0] == 0x35)
        index = appListWidget->currentRow();
    else
        index = sysListWidget->currentRow();
    while(count-- > 0)
    {
        fileInfo = QFileInfo(UPDATE_DATA_NAME);
        if(fileInfo.exists())
            break;
        sleep(100);
        if(!checkTransPort())
            return false;
    }
    if(count < 0)
        return false;


    cmd = "DTA.exe x -ibck -y -o+ "+fileInfo.absoluteFilePath()+" "+fileInfo.absolutePath();
    if(process.execute(cmd) == 0)
    {
        //删除UPDATE_PACK_NAME
        QFile zipFile(UPDATE_DATA_NAME);
        zipFile.remove();

        if(cmdArray[0][0] == REQUEST_FOLDERTYPE_APPMANAGER)
        {
            iniPath = "./download/daemon/";
            if(cmdArray[0][1] == REQUEST_FILETYPE_CONFIG)
                readIni(APPLICATION,iniPath + INI_FILE_CONFIG);
            else if(cmdArray[0][1] == REQUEST_FILETYPE_DAEMON)
            {
                readIni(DAEMON,iniPath + INI_FILE_DAEMON_LIST);
                readIni(DAEMON,iniPath + INI_FILE_DAEMON_SET);
            }

        }
        else if(cmdArray[0][0] == REQUEST_FOLDERTYPE_APP)
        {
            CAPPItemInfo itemTemp = itemAppList->at(index);
            sortingFileDir(&itemTemp,"./download/app/"+QString::number(itemTemp.appNumber));
            itemTemp.appStatus &= ~0x40;
            itemAppList->replace(index,itemTemp);
            updateItemOnListWidget(index);
        }
        else if(cmdArray[0][0] == REQUEST_FOLDERTYPE_DRIVER)
        {
            iniPath = "./download/drivers/";
            readIni(DRIVER,iniPath + INI_FILE_CONFIG);
        }
        else if(cmdArray[0][0] == REQUEST_FOLDERTYPE_LIB)
        {
            iniPath = "./download/lib/";
            readIni(LIB,iniPath + INI_FILE_CONFIG);
        }
        else if(cmdArray[0][0] == REQUEST_FOLDERTYPE_FONT)
        {
            iniPath = "./download/fonts/";
            readIni(FONT,iniPath + INI_FILE_CONFIG);
        }
        return true;
    }

    QFile(UPDATE_DATA_NAME).remove();
    QFile(UPDATE_DATA_TEMP_NAME).remove();
    return false;
}




void Tools::readIni(int itemType, QString iniPath)
{
    int i,j,k;
    if(itemType == APPLICATION)
    {
        //生成conf.ini应用排序配置文件
        QSettings cfgSetting(iniPath,QSettings::IniFormat);
        cfgSetting.setIniCodec(INI_FILE_CODE);
        QStringList cfgKeyList = cfgSetting.childGroups();
        CAPPItemInfo itemTemp;
        if(itemAppList->count()>0)
            return;
        //APP INI Config
        if(cfgKeyList.count() > 0)
        {
            for(i=0;i<cfgKeyList.count();i++)
            {
                cfgSetting.beginGroup("APP"+QString::number(i+1));
                itemTemp.resetItem();
                itemTemp.appDesc = cfgSetting.value("NAME").toString();
                itemTemp.appNumber = cfgSetting.value("ID").toInt();
                itemTemp.hideFlag = cfgSetting.value("HIDDEN").toInt()==0?false:true;
                itemTemp.version = cfgSetting.value("VERSION").toString();
                itemTemp.itemCode = cfgSetting.value("ITEMCODE").toString();
                itemTemp.editDate = cfgSetting.value("EDITDATE").toString();
                itemTemp.editType = cfgSetting.value("EDIT",0).toInt();
                itemTemp.appStatus = 0x40;
                addItemToListWidget(&itemTemp,0);
                cfgSetting.endGroup();
            }
        }
    }
    //读取setting.ini文件中的内容
    else if(itemType == DAEMON && iniPath.contains(INI_FILE_DAEMON_SET))
    {
        QSettings cfgSetting(iniPath,QSettings::IniFormat);
        cfgSetting.setIniCodec(INI_FILE_CODE);
        QStringList sessionList = cfgSetting.childGroups();
        QStringList keyList;
        QStringList childSessionList;
        CSYSItemInfo itemTemp;
        DAEMON_SET daemonSetTemp;
        itemTemp.status = 0x00;

        for(i=0;i<sessionList.count();i++)
        {
            cfgSetting.beginGroup(sessionList.at(i));
            keyList = cfgSetting.childKeys();
            //消除垃圾key
            for(j=0;j<keyList.count();j++)
            {
                if(keyList.at(j).isEmpty())
                {
                    keyList.takeAt(j);
                    j--;
                }
            }
            childSessionList = cfgSetting.childGroups();
            for(j=0;j<keyList.count();j++)
            {
                QString valueTemp = cfgSetting.value(keyList.at(j)).toString();

                daemonSetTemp.brief.clear();
                daemonSetTemp.key.clear();
                daemonSetTemp.parent.clear();
                daemonSetTemp.session.clear();
                daemonSetTemp.type = 0;
                daemonSetTemp.value.clear();
                daemonSetTemp.valueList.clear();

                daemonSetTemp.session = sessionList.at(i);
                daemonSetTemp.key = keyList.at(j);
                daemonSetTemp.value = valueTemp.left(valueTemp.indexOf("#"));
                daemonSetTemp.value = daemonSetTemp.value.simplified();
                valueTemp = valueTemp.right(valueTemp.size() - valueTemp.indexOf("#"));
                daemonSetTemp.type = analysisDesc(valueTemp,&daemonSetTemp.brief,&daemonSetTemp.valueList);
                daemonSetTemp.parent = "";
                *daemonSet<<daemonSetTemp;
            }
            if(childSessionList.count() > 0)
            {
                for(j=0;j<childSessionList.count();j++)
                {
                    cfgSetting.beginGroup(childSessionList.at(j));
                    keyList = cfgSetting.childKeys();
                    for(k=0;k<keyList.count();k++)
                    {
                        QString valueTemp = cfgSetting.value(keyList.at(k)).toString();

                        daemonSetTemp.brief.clear();
                        daemonSetTemp.key.clear();
                        daemonSetTemp.parent.clear();
                        daemonSetTemp.session.clear();
                        daemonSetTemp.type = 0;
                        daemonSetTemp.value.clear();
                        daemonSetTemp.valueList.clear();

                        daemonSetTemp.session = sessionList.at(k);
                        daemonSetTemp.key = keyList.at(k);
                        daemonSetTemp.value = valueTemp.left(valueTemp.indexOf("#"));
                        valueTemp = valueTemp.right(valueTemp.size() - valueTemp.indexOf("#"));
                        daemonSetTemp.type = analysisDesc(valueTemp,&daemonSetTemp.brief,&daemonSetTemp.valueList);
                        daemonSetTemp.parent = childSessionList.at(j);
                        *daemonSet<<daemonSetTemp;
                    }
                    cfgSetting.endGroup();
                }
            }
            cfgSetting.endGroup();
        }
        if(sessionList.count() > 0)
        {
            itemTemp.srcName = INI_FILE_DAEMON_SET;
            itemTemp.type = SHELL;
            itemTemp.srcPath = iniPath;
            addItemToListWidget(&itemTemp,itemTemp.type);
        }
    }
    //生成其他系统应用的配置文件,如ko,lib等
    else
    {
        QSettings cfgSetting(iniPath,QSettings::IniFormat);
        cfgSetting.setIniCodec(INI_FILE_CODE);
        QStringList cfgKeyList = cfgSetting.childGroups();
        CSYSItemInfo itemTemp;
        //INI Config
        if(cfgKeyList.count() > 0)
        {
            for(i=0;i<cfgKeyList.count();i++)
            {
                if(itemType == DRIVER)
                    cfgSetting.beginGroup("KO"+QString::number(i+1));
                else if (itemType == LIB)
                    cfgSetting.beginGroup("LIB"+QString::number(i+1));
                else if (itemType == DAEMON)
                    cfgSetting.beginGroup("DAEMON"+QString::number(i+1));
                else if (itemType == FONT)
                    cfgSetting.beginGroup("FONT"+QString::number(i+1));
                else if (itemType == DATA)
                    cfgSetting.beginGroup("DATA"+QString::number(i+1));
                else
                    return;
                itemTemp.resetItem();
                itemTemp.type = itemType;
                itemTemp.srcName = cfgSetting.value("NAME").toString();
                if(itemType == DRIVER || itemType == LIB || itemType == FONT || itemType == DAEMON)
                    itemTemp.version = cfgSetting.value("VERSION").toString();
                if(itemType == DRIVER || itemType == DAEMON)
                {
                    itemTemp.insmod = cfgSetting.value("INSMOD").toInt();
                    itemTemp.dbg = cfgSetting.value("DBG").toInt();
                }
                itemTemp.status = 0x00;
                addItemToListWidget(&itemTemp,itemType);
                cfgSetting.endGroup();
            }
        }
    }
}


//type 0 表示读app, 1表示manager
void Tools::sortingFileDir(void *item, QString filePath, int type)
{
    CAPPItemInfo *appItemTemp;
    CSYSItemInfo *sysItemTemp;
    QDir dir;
    dir.setPath(filePath);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst | QDir::Name);

    const QFileInfoList fileList = dir.entryInfoList();

    for (int i = 0; i < fileList.size(); i++)
    {
        QFileInfo fileInfo = fileList.at(i);
        QString fileName = fileInfo.fileName();
        QString subName = fileInfo.suffix();

        if(fileInfo.isDir())
        {
            // 查询子目录
            if(fileInfo.fileName().contains("daemon"))
                sortingFileDir(item, fileInfo.filePath(),DAEMON);
            else
                sortingFileDir(item, fileInfo.filePath(),APPLICATION);
        }
        else
        {
            //如果为conf.ini文件
            if(fileName.contains(INI_FILE_CONFIG))
            {
                if(type == APPLICATION)
                {
                    QFile iniFile(fileInfo.filePath());
                    appItemTemp = (CAPPItemInfo *)item;

                    if(iniFile.open(QIODevice::ReadOnly))
                    {
                        QTextStream in(&iniFile);
                        in.setCodec("UTF-8");
                        appItemTemp->appSettings = in.readAll();
                        appItemTemp->appSettings.replace("[SYS]\r\n","");
                        iniFile.close();
                    }
                }
                else if(type == DAEMON)
                {
                    QSettings cfgSetting(fileInfo.absoluteFilePath(),QSettings::IniFormat);
                    appItemTemp = (CAPPItemInfo *)item;
                    cfgSetting.setIniCodec(INI_FILE_CODE);
                    //APP INI Config
                    cfgSetting.beginGroup("APP0");
                    appItemTemp->appDesc = cfgSetting.value("NAME").toString();
                    appItemTemp->appNumber = cfgSetting.value("ID").toInt();
                    appItemTemp->hideFlag = cfgSetting.value("HIDDEN").toInt()==0?false:true;
                    appItemTemp->version = cfgSetting.value("VERSION").toString();
                    appItemTemp->itemCode = cfgSetting.value("ITEMCODE").toString();
                    appItemTemp->editDate = cfgSetting.value("EDITDATE").toString();
                    appItemTemp->editType = cfgSetting.value("EDIT",0).toInt();
                    appItemTemp->appStatus = 0xB0;
                    cfgSetting.endGroup();
                }
            }
            //图标
            else if((subName.contains("png") || subName.contains("ico")) && fileName.contains("app"))
            {
                appItemTemp = (CAPPItemInfo *)item;
                appItemTemp->ICOPicPath = fileInfo.filePath();
            }
            //图片
            else if(subName.contains("png") || subName.contains("jpg") || subName.contains("bmp"))
            {
                appItemTemp = (CAPPItemInfo *)item;
                appItemTemp->logoPicPath<<fileInfo.filePath();
            }
            //UI脚本文件
            else if(subName.contains("qss"))
            {
                appItemTemp = (CAPPItemInfo *)item;
                appItemTemp->uiScriptPath = fileInfo.filePath();
            }
            //bin文件
            else if(subName.isEmpty() && fileName.toInt() == QFileInfo(fileInfo.absolutePath()).fileName().toInt())
            {
                appItemTemp = (CAPPItemInfo *)item;
                appItemTemp->srcPath = fileInfo.absoluteFilePath();
                appItemTemp->srcName = fileInfo.fileName();
            }
            //库文件
            else if(fileName.contains(".so.") || subName.contains("so"))
            {
                sysItemTemp = (CSYSItemInfo *)item;
                sysItemTemp->type = LIB;
                sysItemTemp->srcPath = fileInfo.absoluteFilePath();
                sysItemTemp->status = 0xB0;
                if(fileName.contains(".so."))
                {
                    sysItemTemp->version = fileName.right(fileName.size()-fileName.lastIndexOf(".so.")-4);
                    sysItemTemp->srcName = fileName.left(fileName.lastIndexOf(".so.")+3);
                }
                else
                {
                    sysItemTemp->version = "0";
                    sysItemTemp->srcName = fileName;
                }
                addItemToListWidget(sysItemTemp,LIB);
            }
            //驱动文件
            else if(fileName.contains(".ko.") || subName.contains("ko"))
            {
                sysItemTemp = (CSYSItemInfo *)item;
                sysItemTemp->type = DRIVER;
                sysItemTemp->status = 0xB0;
                sysItemTemp->srcPath = fileInfo.absoluteFilePath();
                if(fileName.contains(".ko."))
                {
                    sysItemTemp->version = fileName.right(fileName.size()-fileName.lastIndexOf(".ko.")-4);
                    sysItemTemp->srcName = fileName.left(fileName.lastIndexOf(".ko.")+3);
                }
                else
                {
                    sysItemTemp->version = "0";
                    sysItemTemp->srcName = fileName;
                }

                addItemToListWidget(sysItemTemp,DRIVER);
            }
            //字体
            else if(fileName.contains(".ft.") || subName.contains("ft"))
            {
                sysItemTemp = (CSYSItemInfo *)item;
                sysItemTemp->type = FONT;
                sysItemTemp->status = 0xB0;
                sysItemTemp->srcPath = fileInfo.absoluteFilePath();
                if(fileName.contains(".ft."))
                {
                    sysItemTemp->version = fileName.right(fileName.size()-fileName.lastIndexOf(".ft.")-4);
                    sysItemTemp->srcName = fileName.left(fileName.lastIndexOf(".ft.")+3);
                }
                else
                {
                    sysItemTemp->version = "0";
                    sysItemTemp->srcName = fileName;
                }
                addItemToListWidget(sysItemTemp,FONT);
            }
            else if(subName.contains("dta"))
            {
                addDtaToListWidget(fileInfo.filePath());
            }
        }
    }
}


bool Tools::creatIni(QString iniName, int itemType, void *item)
{
    QSettings iniSetting(iniName,QSettings::IniFormat);
    iniSetting.setIniCodec(INI_FILE_CODE);

    if(itemType == APPLICATION)
    {
        char IDBuf[5];
        CAPPItemInfo *itemTemp;
        itemTemp = (CAPPItemInfo *)item;
        memset(IDBuf,0x0,sizeof(IDBuf));
        //APP INI Config
        sprintf(IDBuf,"%05d",itemTemp->appNumber);
        iniSetting.beginGroup("APP0");
        iniSetting.setValue("ID",IDBuf);
        iniSetting.setValue("NAME",itemTemp->appDesc);
        iniSetting.setValue("EDITDATE",itemTemp->editDate);
        iniSetting.setValue("HIDDEN",itemTemp->hideFlag?1:0);
        iniSetting.setValue("VERSION",itemTemp->version);
        iniSetting.setValue("ITEMCODE",itemTemp->itemCode);
        iniSetting.setValue("OWNER",itemTemp->owner);
        iniSetting.setValue("CHECKCODE",itemTemp->checkCode);
        iniSetting.setValue("EDIT",itemTemp->editType);
        iniSetting.endGroup();
        return true;
    }
    else if(itemType == DAEMON)
    {
        int count = daemonSet->count();
        QString session, parent, strTemp;
        for(int i=0;i<count;i++)
        {
            session = daemonSet->at(i).session;
            parent = daemonSet->at(i).parent;
            if(!parent.isEmpty())
                iniSetting.beginGroup(parent);
            iniSetting.beginGroup(session);
            //组装value
            if(daemonSet->at(i).value.isEmpty())
                strTemp = "NULL";
            else
                strTemp = daemonSet->at(i).value;
            strTemp += " #" + QString::number(daemonSet->at(i).type);
            //添加brief
            if(!daemonSet->at(i).brief.isEmpty())
                strTemp += " " + daemonSet->at(i).brief;
            //添加value list
            if(!daemonSet->at(i).valueList.isEmpty())
            {
                strTemp += "|" + daemonSet->at(i).valueList;
                strTemp = strTemp.replace(";"," ");
            }
            iniSetting.setValue(daemonSet->at(i).key,strTemp);
            iniSetting.endGroup();
            if(!parent.isEmpty())
                iniSetting.endGroup();
        }
        sleep(20);
        QFile file(iniName);
        if(file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            strTemp = file.readAll();
            file.resize(0);
            file.write("#######################\n# note:\n# #0 scan by carcode\n# ");
            file.write("#1 text\n# #2 radio\n# #3 checkbox\n# #4 just for display\n");
            file.write("# #5 passwd\n# #6 range value\n#######################\n\n");
            file.write(strTemp.toLatin1().data());
            file.close();
        }
    }
    else if(itemType == INI_TYPE_PLAN)
    {
        char IDBuf[5];
        int i;
        QList<CAPPItemInfo> *itemList;
        QString appSerNum;
        itemList = (QList<CAPPItemInfo> *)item;
        for(i=0;i<itemList->count();i++)
        {
            memset(IDBuf,0x0,sizeof(IDBuf));
            //APP INI Config
            sprintf(IDBuf,"%05d",itemList->at(i).appNumber);
            appSerNum = "APP" + QString::number(i+1);
            iniSetting.beginGroup(appSerNum);
            iniSetting.setValue("ID",IDBuf);
            iniSetting.setValue("NAME",itemList->at(i).appDesc);
            iniSetting.setValue("EDITDATE",itemList->at(i).editDate);
            iniSetting.setValue("HIDDEN",itemList->at(i).hideFlag?1:0);
            iniSetting.setValue("VERSION",itemList->at(i).version);
            iniSetting.setValue("ITEMCODE",itemList->at(i).itemCode);
            iniSetting.setValue("OWNER",itemList->at(i).owner);
            iniSetting.setValue("CHECKCODE",itemList->at(i).checkCode);
            iniSetting.setValue("EDIT",itemList->at(i).editType);
            iniSetting.setValue("UPDATE",1);
            iniSetting.endGroup();
        }
        return true;
    }
    return false;
}
bool Tools::creatShell(QString shellName, int itemType)
{
    FILE *fp;
    QString strTemp;
    QString insmodShell;
    QStringList insmodSort;
    QString daemonParam;
    QList <CSYSItemInfo> itemList;
    int i = 0, j = 0;
    if(itemType == DRIVER)
    {
        if((fp = fopen(shellName.toLatin1().data(),"wb")) == NULL)
            return false;

        //加载驱动的Shell语句
        strTemp = "#!/bin/sh\x0A## driver.sh";
        fwrite(strTemp.toLatin1().data(),strTemp.size(),1,fp);
        fwrite("\x0A",1,1,fp);

        insmodSort = getChildValues("SORT_DRIVER",INI_FILE_DESC);
        //将driver类型的信息保存到itemList中
        for(i=0;i<itemSysList->count();i++)
            if(itemSysList->at(i).type == DRIVER)
                itemList<<itemSysList->at(i);

        for(i=0;i<insmodSort.count();i++)
        {
            for(j=0;j<itemList.count();j++)
            {
                if(insmodSort.at(i).compare(itemList.at(j).srcName) == 0)
                {
                    insmodShell = "insmod /drivers/" + itemList.at(j).srcName;
                    //如果版本不为空，将版本添加到文件名后面
                    if(!itemList.at(j).version.isEmpty())
                        insmodShell += "." + itemList.at(j).version;
                    if(itemList.at(j).insmod == 0)
                        insmodShell = "#" + insmodShell;
                    else
                        insmodShell = " " + insmodShell;
                    insmodShell += " dbg=" + QString::number(itemList.at(j).dbg);
                    fwrite(insmodShell.toLatin1().data(),insmodShell.size(),1,fp);
                    fwrite("\x0A",1,1,fp);
                    itemList.takeAt(j);
                    break;
                }
            }
        }

        //将剩余的itemList中的应用信息不分顺序写入到shell中
        for(i=0;i<itemList.count();i++)
        {
            insmodShell = "insmod /drivers/" + itemList.at(i).srcName;
            //如果版本不为空，将版本添加到文件名后面
            if(!itemList.at(i).version.isEmpty())
                insmodShell += "." + itemList.at(i).version;
            if(itemList.at(i).insmod == 0)
                insmodShell = "#" + insmodShell;
            else
                insmodShell = " " + insmodShell;
            insmodShell += " dbg=" + QString::number(itemList.at(i).dbg);
            fwrite(insmodShell.toLatin1().data(),insmodShell.size(),1,fp);
            fwrite("\x0A",1,1,fp);
        }

        fclose(fp);
    }
    else if(itemType == DAEMON)
    {

        if((fp = fopen(shellName.toLatin1().data(),"wb")) == NULL)
            return false;

        //加载驱动的Shell语句
        strTemp = "#!/bin/sh\x0A## daemon.sh";
        fwrite(strTemp.toLatin1().data(),strTemp.size(),1,fp);
        fwrite("\x0A",1,1,fp);

        insmodSort = getChildValues("SORT_DAEMON",INI_FILE_DESC);
        //将daemon类型的信息保存到itemList中
        for(i=0;i<itemSysList->count();i++)
            if(itemSysList->at(i).type == DAEMON)
                itemList<<itemSysList->at(i);

        for(i=0;i<insmodSort.count();i++)
        {
            for(j=0;j<itemList.count();j++)
            {
                if(insmodSort.at(i).compare(itemList.at(j).srcName) == 0)
                {
                    insmodShell = "/daemon/" + itemList.at(j).srcName;
                    //如果版本不为空，将版本添加到文件名后面
                    if(!itemList.at(j).version.isEmpty())
                        insmodShell += "." + itemList.at(j).version;
                    if(itemList.at(j).insmod == 0)
                        insmodShell = "#" + insmodShell;
                    else
                        insmodShell = " " + insmodShell;

                    //如果有参数添加参数
                    daemonParam = getPrivateProfileString("PARAM_DAEMON",itemList.at(j).srcName,"",INI_FILE_DESC);

                    if(!daemonParam.isEmpty())
                        insmodShell += " " + daemonParam;

                    insmodShell += " dbg=" + QString::number(itemList.at(j).dbg);
                    //if(itemList.at(j).dbg == 0)
                    //    insmodShell += " >/dev/null 2>&1";
                    //设置只有updateService.dm能够前台启动
                    //if(itemList.at(j).srcName.compare("updateService.dm") != 0)
                    //    insmodShell += " &";

                    //所有程序都后台启动
                    insmodShell += " &";
                    fwrite(insmodShell.toLatin1().data(),insmodShell.size(),1,fp);
                    fwrite("\x0A",1,1,fp);
                    itemList.takeAt(j);
                    break;
                }
            }
        }

        //将剩余的itemList中的应用信息不分顺序写入到shell中
        for(i=0;i<itemList.count();i++)
        {
            insmodShell = "/daemon/" + itemList.at(i).srcName;
            //如果版本不为空，将版本添加到文件名后面
            if(!itemList.at(i).version.isEmpty())
                insmodShell += "." + itemList.at(i).version;
            if(itemList.at(i).insmod == 0)
                insmodShell = "#" + insmodShell;
            else
                insmodShell = " " + insmodShell;

            //如果有参数添加参数
            daemonParam = getPrivateProfileString("PARAM_DAEMON",itemList.at(i).srcName,"",INI_FILE_DESC);

            if(!daemonParam.isEmpty())
                insmodShell += " " + daemonParam;

            insmodShell += " dbg=" + QString::number(itemList.at(i).dbg);
            //if(itemList.at(i).dbg == 0)
            //    insmodShell += " >/dev/null 2>&1";
            //设置只有updateService.dm能够前台启动
            //if(itemList.at(i).srcName.compare("updateService.dm") != 0)
            //    insmodShell += " &";

            //所有程序都后台启动
            insmodShell += " &";
            fwrite(insmodShell.toLatin1().data(),insmodShell.size(),1,fp);
            fwrite("\x0A",1,1,fp);
        }

        fclose(fp);
    }
    return true;
}

bool Tools::updateItemToPos(int itemType, int index, int updateType)
{
    bool result = false;
    QString basePath = "tmp/download/";
    QString iniPath;
    FILE *fp;

    if(itemType == APPLICATION)
    {
        CAPPItemInfo itemTemp = itemAppList->at(index);

        QString binPath = basePath + "app/";
        int packFlag = 0;
        int imageCount = itemTemp.logoPicPath.count();
        if((itemTemp.appStatus & 0x20) == 0x20 && itemTemp.appNumber > 0)
        {
            char appID[6];
            QDir dir;
            memset(appID,0x0,sizeof(appID));
            sprintf(appID,"%05d",itemTemp.appNumber);
            binPath += QString(appID)+"/";
            //创建文件夹
            if(QFileInfo("tmp").exists())
                deleteDirectory(QFileInfo("tmp"));
            sleep(100);
            if(dir.mkpath(binPath))
            {
                sleep(100);
                dir.setPath(basePath);
                dir.mkdir("daemon");
            }
            else
                return false;
            //修改状态
            if((itemTemp.appStatus & 0x10) == 0x10)
            {
                packFlag = 0x0F;
            }
            else
            {
                packFlag = itemTemp.appStatus;
            }

            //拷贝ICO
            if((packFlag & 0x1) == 0x1 && !itemTemp.ICOPicPath.isEmpty())
            {
                QFile::copy(itemTemp.ICOPicPath,binPath+"app."+QFileInfo(itemTemp.ICOPicPath).suffix());
            }

            //拷贝qss
            if((packFlag & 0x8) == 0x8)
            {
                if(!itemTemp.uiScriptPath.isEmpty())
                    QFile::copy(itemTemp.uiScriptPath,binPath+QFileInfo(itemTemp.uiScriptPath).fileName());
                if(!itemTemp.delUiScriptName.isEmpty())
                {
                    fp = fopen(QString(binPath+"~"+itemTemp.delUiScriptName).toLocal8Bit().data(),"w");
                    if(fp != NULL)
                        fclose(fp);
                }
            }

            //拷贝Image
            if((packFlag & 0x2) == 0x2)
            {
                dir.setPath(binPath);
                dir.mkdir("image");
                for(int i=0;i<imageCount;i++)
                {
                    QString logoPath = itemTemp.logoPicPath.at(i);
                    QFile::copy(logoPath,binPath+"image/"+QFileInfo(logoPath).fileName());
                }
                //生成删除图片

                for(int i=0;i<itemTemp.delPicture.count();i++)
                {
                    fp = fopen(QString(binPath+"image/~"+itemTemp.delPicture.at(i)).toLocal8Bit().data(),"w");
                    if(fp != NULL)
                        fclose(fp);
                }
            }
            //拷贝配置文件
            sleep(100);

            QFile iniFile(binPath + INI_FILE_CONFIG);
            if(iniFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            {
                //iniFile.write("[SYS]\n");
                QTextStream out(&iniFile);
                out.setCodec(INI_FILE_CODE);
                out<<"[SYS]\n";
                out<<itemTemp.appSettings;
                out<<"\0";

                iniFile.close();
            }

            //拷贝源文件
            if(!itemTemp.srcPath.isEmpty())
            {
                QTime curTime = QTime::currentTime();
                QDate curDate = QDate::currentDate();
                QString editDate;
                editDate = curDate.toString("yyyy.MM.dd")+" "+curTime.toString("hh:mm");
                itemTemp.editDate = editDate;
                signApp(itemTemp.srcPath,binPath+QString(appID));
                //QFile::copy(itemTemp.srcPath,binPath+QString(appID));
            }

            //生成系统配置文件
            sleep(100);
            iniPath = basePath + "daemon/";
            iniPath += INI_FILE_CONFIG;
            creatIni(iniPath,APPLICATION,&itemTemp);

            QProcess process;
            QFileInfo zipFileInfo(UPDATE_PACK_TEMP_PATH);
            QString cmd = "DTA.exe a -ep1 -m5 -r -ibck -o+  "+zipFileInfo.absoluteFilePath()+" "+QFileInfo(basePath).absoluteFilePath();

            if(process.execute(cmd) == 0)
            {
                //修改文件名
                QFile(UPDATE_PACK_TEMP_PATH).rename(UPDATE_PACK_PATH);
                sleep(200);
                //打包完成后传输
                if(updateType == 0)
                {
                    //传输完成后，查看结果
                    sendDtaSuccess = TOOLS_ERROR_INITVAL;
                    if(packgeCommand(DOWNCMD_RESPFILE,UPDATE_PACK_PATH,NULL,0) == QFile(UPDATE_PACK_PATH).size())
                    {
                        int i = 0;
                        while(i++ < 100)
                        {
                            //传输成功
                            if(sendDtaSuccess == TOOLS_ERROR_SUCCESS)
                            {
                                itemTemp.appStatus &= ~0x20;
                                itemTemp.delPicture.clear();
                                itemAppList->replace(index,itemTemp);
                                result = true;
                                sendDtaSuccess = TOOLS_ERROR_INITVAL;
                                break;
                            }
                            else if(sendDtaSuccess == TOOLS_ERROR_SIGNERR)
                            {
                                result = false;
                                sendDtaSuccess = TOOLS_ERROR_INITVAL;
                                break;
                            }
                            sleep(100);
                        }
                        goto _exit;
                    }
                }
                //只打包不传输,用于生成DTA包
                else
                {
                    if(!itemTemp.srcPath.isEmpty())
                    {
                        if(!QFile(UPDATE_SCHEME_PATH).exists())
                        {
                            dir.setPath("./");
                            dir.mkdir(UPDATE_SCHEME_PATH);
                        }
                        QString packagePath = QFileInfo(UPDATE_SCHEME_PATH).absoluteFilePath()+"/"+QString::number(itemTemp.appNumber)+".dta";
                        QFile packageFile(packagePath);
                        if(packageFile.exists())
                            packageFile.remove();
                        QFile::copy(UPDATE_PACK_PATH,packagePath);
                    }
                    msgProBar->setMaximum(100);
                    msgProBar->setValue(100);
                    result = true;
                    goto _exit;
                }
            }
        }
    }
    else
    {
        CSYSItemInfo itemTemp;
        //用于表示Kernel或uboot的文件名
        QString fileName;

        QDir dir;
        //创建文件夹
        if(QFileInfo("tmp").exists())
            deleteDirectory(QFileInfo("./tmp"));
        sleep(100);
        if(!dir.mkpath(basePath))
            return false;
        dir.setPath(basePath);
        if(itemType == DAEMON)
        {
            dir.mkdir("daemon");
            basePath += "daemon/";
        }
        else if(itemType == LIB)
        {
            dir.mkdir("lib");
            basePath += "lib/";
        }
        else if(itemType == DRIVER)
        {
            dir.mkdir("drivers");
            basePath += "drivers/";
        }
        else if(itemType == FONT)
        {
            dir.mkdir("fonts");
            basePath += "fonts/";
        }
        else if(itemType == DATA)
        {
            dir.mkdir("data");
            basePath += "data/";
        }
        else if(itemType == UBOOT)
        {
            dir.mkdir("uboot");
            basePath += "uboot/";
            fileName += "uboot.rsa";
        }
        else if(itemType == KERNEL)
        {
            dir.mkdir("kernel");
            basePath += "kernel/";
            fileName += "uImage.rsa";
        }
        else if(itemType == CRT)
        {
            dir.mkdir("crt");
            basePath += "crt/";
        }
        else if(itemType == CRAMFS)
        {
            dir.mkdir("cramfs");
            basePath += "cramfs/";
        }

        //遍历itemList，将相同类别的文件拷贝在同一个目录中
        for(int i=0;i<itemSysList->count();i++)
        {
            if(itemSysList->at(i).type == itemType)
            {
                itemTemp = itemSysList->at(i);
                if((itemTemp.status & 0x20) == 0x20 && !itemTemp.srcPath.isEmpty())
                {
                    //拷贝源文件
                    if(itemType == KERNEL || itemType == UBOOT || itemType == CRT || itemType == CRAMFS)
                    {
                        if(itemType == CRT || itemType == CRAMFS)
                        {
                            fileName = itemTemp.srcName + ".rsa";
                        }
                        if(creatRSAFile(&itemTemp))
                        {
                            QFile::copy(UPDATE_SCHEME_PATH+fileName,basePath+fileName);
                        }
                         break;
                    }

                    if(itemTemp.version.isEmpty())
                        QFile::copy(itemTemp.srcPath,basePath+itemTemp.srcName);
                    else
                        QFile::copy(itemTemp.srcPath,basePath+itemTemp.srcName+"."+itemTemp.version);
                }
            }
        }
        if(itemType == DRIVER)
            creatShell(basePath+SH_FILE_DRIVERS, DRIVER);
        else if(itemType == DAEMON)
        {
            creatIni(basePath+INI_FILE_DAEMON_SET,DAEMON);
            creatShell(basePath+SH_FILE_DAEMON,DAEMON);
        }

        creatDelFile(basePath,itemType);

        QProcess process;
        QFileInfo zipFileInfo(UPDATE_PACK_TEMP_PATH);
        QString cmd = "DTA.exe a -ep1 -m5 -r -ibck -o+  "+zipFileInfo.absoluteFilePath()+" "+QFileInfo(basePath).absolutePath();

        if(process.execute(cmd) == 0)
        {
            //修改文件名
            QFile(UPDATE_PACK_TEMP_PATH).rename(UPDATE_PACK_PATH);
            sleep(100);

            //传输更新包
            sendDtaSuccess = TOOLS_ERROR_INITVAL;
            if(packgeCommand(DOWNCMD_RESPFILE,UPDATE_PACK_PATH,NULL,0) == QFile(UPDATE_PACK_PATH).size())
            {
                int j = 0;
                while(j++ < 100)
                {
                    //传输成功
                    //遍历itemList，修改Item状态
                    if(sendDtaSuccess == TOOLS_ERROR_SUCCESS)
                    {
                        for(int i=0;i<itemSysList->count();i++)
                        {
                            if(itemSysList->at(i).type == itemType)
                            {
                                itemTemp = itemSysList->at(i);
                                if((itemTemp.status & 0x20) == 0x20)
                                {
                                    itemTemp.status &= ~0x20;
                                    itemSysList->replace(i,itemTemp);
                                }
                            }
                        }
                        //删除DeleteList中的对于应用
                        int delCount = itemSysDelList->count();
                        for(int i=delCount;i>0;i--)
                        {
                            if(itemSysDelList->at(i-1).type == itemType)
                                itemSysDelList->takeAt(i-1);
                        }
                        result = true;
                        break;
                    }
                    sleep(100);
                }
                goto _exit;
            }
        }
    }

_exit:
    deleteDirectory(QFileInfo("tmp"));
    return result;
}


void Tools::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents();
    }
}

bool Tools::updateItemOnListWidget(int index)
{
    if(index > -1)
    {
        QIcon icon;
        QListWidgetItem *listWidgetItem;

        if(itemAppList->at(index).ICOPicPath.isEmpty())
            icon = QIcon(resourceIcoPath.at(0));
        else
            icon = QIcon(itemAppList->at(index).ICOPicPath);

        listWidgetItem = new QListWidgetItem(icon,itemAppList->at(index).appDesc.leftRef(8).toString(),0,1000);
        listWidgetItem->setFlags(listWidgetItem->flags()&(~(Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled)));
        appListWidget->takeItem(index);
        appListWidget->insertItem(index,listWidgetItem);
        appListWidget->setCurrentRow(index);
        return true;
    }
    return false;
}

void Tools::getProgressBar(QProgressBar *proBar)
{
    this->msgProBar = proBar;
}

void Tools::getTimer(QTimer *readTimer)
{
    this->readTimer = readTimer;
}

void Tools::getSocket(QTcpSocket *tcpSocket)
{
    this->tcpSocket = tcpSocket;
}

void Tools::getTransType(int *transType)
{
    this->transType = transType;
}

void Tools::getStatusLabel(QLabel *msgDis, QLabel *msgLable)
{
    this->msgDis = msgDis;
    this->msgLable = msgLable;
}

void Tools::getItemAppList(QList<CAPPItemInfo> *itemAppList, QList<CAPPItemInfo> *itemAppDelList)
{
    this->itemAppList = itemAppList;
    this->itemAppDelList = itemAppDelList;
}

void Tools::getItemSysList(QList<CSYSItemInfo> *itemSysList, QList<CSYSItemInfo> *itemSysDelList)
{
    this->itemSysList = itemSysList;
    this->itemSysDelList = itemSysDelList;
}

void Tools::getListWidget(QListWidget *appListWidget, QListWidget *sysListWidget, QListWidget *imageListWidget)
{
    this->appListWidget = appListWidget;
    this->sysListWidget = sysListWidget;
    this->imageListWidget = imageListWidget;
}

void Tools::getTableWidget(QTableWidget *tableWidget)
{
    this->tableWidget = tableWidget;
}

void Tools::getSerialPort(QextSerialPort *serialPort)
{
    this->serialPort = serialPort;
}

void Tools::getDaemonSet(QList<DAEMON_SET> *daemonSet)
{
    this->daemonSet = daemonSet;
}

void Tools::getTreeWidget(QTreeWidget *treeWidget)
{
    this->treeWidget = treeWidget;
}

bool Tools::updateAllItem(void)
{
    int curCount = 0;//已经更新的个数
    int totalCount = 0;//需要更新的总个数
    bool appManagerFlag = false;
    bool darmonFlag = false;
    bool libFlag = false;
    bool driverFlag = false;
    bool fontFlag = false;
    bool dataFlag = false;
    bool ubootFlag = false;
    bool kernelFlag = false;
    bool result = true;
    int timeOut = 0;


    //如果有OTHER_BOOT类型的数据出现，只传递此类型
    if(getItemCount(OTHER_BOOT) > 0)
    {
        result = updateOtherBoot();
        goto _exit;
    }

    totalCount = getUpdateItemCount(APPLICATION);//需要更新APP的个数

    if(getItemCount(APPLICATION) > 0 || getItemCount(APPLICATION,1) > 0)
    {
        totalCount += 1;
        appManagerFlag = true;
    }
    if(getUpdateItemCount(DAEMON) > 0 || getItemCount(DAEMON,1) > 0 || settingMoidfy == 1)
    {
        totalCount += 1;
        darmonFlag = true;
    }
    if(getUpdateItemCount(LIB) > 0 || getItemCount(LIB,1) > 0)
    {
        totalCount += 1;
        libFlag = true;
    }
    if(getUpdateItemCount(DRIVER) > 0 || getItemCount(DRIVER,1) > 0)
    {
        totalCount += 1;
        driverFlag = true;
    }
    if(getUpdateItemCount(FONT) > 0 || getItemCount(FONT,1) > 0)
    {
        totalCount += 1;
        fontFlag = true;
    }
    if(getUpdateItemCount(DATA) > 0 || getItemCount(DATA,1) > 0)
    {
        totalCount += 1;
        dataFlag = true;
    }
    if(getUpdateItemCount(UBOOT) > 0)
    {
        totalCount += 1;
        ubootFlag = true;
    }
    if(getUpdateItemCount(KERNEL) > 0)
    {
        totalCount += 1;
        kernelFlag = true;
    }

    //挨个传输应用
    for(int i=0;i<itemAppList->count();i++)
    {
        if((itemAppList->at(i).appStatus & 0x20) == 0x20)
        {
            curCount++;
            msgDis->setText(QString::number(curCount)+"/"+QString::number(totalCount));
            result = updateItemToPos(APPLICATION,i);
            if(!result)
                break;
            sleep(2000);
        }
    }
    if(!result)
        goto _exit;

    //传输APP排序
    if(appManagerFlag)
    {
        curCount++;
        msgDis->setText(QString::number(curCount)+"/"+QString::number(totalCount));
        result = updateItemSortInfo();
        if(!result)
            goto _exit;
        sleep(2000);
    }
    //传输MANAGER
    if(darmonFlag)
    {

        curCount++;
        msgDis->setText(QString::number(curCount)+"/"+QString::number(totalCount));
        result = updateItemToPos(DAEMON);
        if(!result)
            goto _exit;
        sleep(2000);
    }
    //传输LIB
    if(libFlag)
    {

        curCount++;
        msgDis->setText(QString::number(curCount)+"/"+QString::number(totalCount));
        result = updateItemToPos(LIB);
        if(!result)
            goto _exit;
        sleep(2000);
    }
    //传输DRIVER
    if(driverFlag)
    {

        curCount++;
        msgDis->setText(QString::number(curCount)+"/"+QString::number(totalCount));
        result = updateItemToPos(DRIVER);
        if(!result)
            goto _exit;
        sleep(2000);
    }
    //传输FONT
    if(fontFlag)
    {

        curCount++;
        msgDis->setText(QString::number(curCount)+"/"+QString::number(totalCount));
        result = updateItemToPos(FONT);
        if(!result)
            goto _exit;
        sleep(2000);
    }


    //传输DATA
    if(dataFlag)
    {

        curCount++;
        msgDis->setText(QString::number(curCount)+"/"+QString::number(totalCount));
        result = updateItemToPos(DATA);
        if(!result)
            goto _exit;
        sleep(2000);
    }

    //传输UBOOT
    if(ubootFlag)
    {

        curCount++;
        msgDis->setText(QString::number(curCount)+"/"+QString::number(totalCount));
        result = updateItemToPos(UBOOT);
        if(!result)
            goto _exit;
        timeOut = 0;
        sleep(2000);
    }
    //传输KERNEL
    if(kernelFlag)
    {

        curCount++;
        msgDis->setText(QString::number(curCount)+"/"+QString::number(totalCount));
        result = updateItemToPos(KERNEL);
        if(!result)
            goto _exit;
        timeOut = 0;
        sleep(2000);
    }
_exit:
    if(result)
        msgDis->setText(QObject::tr("FINISH!"));
    else
        msgDis->setText(QObject::tr("ERROR!"));
    return result;
}


bool Tools::updateItemSortInfo(void)
{
    QString basePath = "./tmp/download/daemon/";
    QDir dir;
    int index = 1;
    int itemCount = 0;
    bool result = false;
    if(!dir.mkpath(basePath))
        return result;

    QSettings itemSort(basePath + INI_FILE_CONFIG, QSettings::IniFormat);
    itemSort.setIniCodec(INI_FILE_CODE);
    char IDBuf[6];
    memset(IDBuf,0x0,sizeof(IDBuf));
    //排序应用
    itemCount = itemAppList->count();
    for(int i = 0;i < itemCount;i++)
    {
        if((itemAppList->at(i).appStatus & 0x20) == 0x00)
        {
            QString session = "APP" + QString::number(index);

            sprintf(IDBuf,"%05d",itemAppList->at(i).appNumber);
            itemSort.beginGroup(session);
            itemSort.setValue("ID",IDBuf);
            itemSort.setValue("NAME",itemAppList->at(i).appDesc);
            itemSort.setValue("EDITDATE",itemAppList->at(i).editDate);
            itemSort.setValue("HIDDEN",itemAppList->at(i).hideFlag?1:0);
            itemSort.setValue("VERSION",itemAppList->at(i).version);
            itemSort.setValue("ITEMCODE",itemAppList->at(i).itemCode);
            itemSort.setValue("OWNER",itemAppList->at(i).owner);
            itemSort.setValue("CHECKCODE",itemAppList->at(i).checkCode);
            itemSort.setValue("EDIT",itemAppList->at(i).editType);
            itemSort.endGroup();
            index++;
        }
    }


    //添加删除的应用项
    itemCount = itemAppDelList->count();
    for(int i=0;i<itemCount;i++)
    {
        QString session = "APP" + QString::number(index);

        sprintf(IDBuf,"%05d",itemAppDelList->at(i).appNumber);
        itemSort.beginGroup(session);
        itemSort.setValue("ID",IDBuf);
        itemSort.setValue("NAME",itemAppDelList->at(i).appDesc);
        itemSort.setValue("EDITDATE",itemAppDelList->at(i).editDate);
        itemSort.setValue("HIDDEN",itemAppDelList->at(i).hideFlag?1:0);
        itemSort.setValue("VERSION",itemAppDelList->at(i).version);
        itemSort.setValue("ITEMCODE",itemAppDelList->at(i).itemCode);
        itemSort.setValue("OWNER",itemAppDelList->at(i).owner);
        itemSort.setValue("CHECKCODE",itemAppDelList->at(i).checkCode);
        itemSort.setValue("EDIT",3);
        itemSort.endGroup();
        index++;
    }
    sleep(500);
    QProcess process;
    QFileInfo zipFileInfo(UPDATE_PACK_TEMP_PATH);
    QString cmd = "DTA.exe a -ep1 -m5 -r -ibck -o+  "+zipFileInfo.absoluteFilePath()+" "+QFileInfo(basePath).absolutePath();

    if(process.execute(cmd) == 0)
    {
        //修改文件名
        QFile(UPDATE_PACK_TEMP_PATH).rename(UPDATE_PACK_PATH);
        sleep(100);

        //传输更新包
        sendDtaSuccess = TOOLS_ERROR_INITVAL;
        if(packgeCommand(DOWNCMD_RESPFILE,UPDATE_PACK_PATH,NULL,0) == QFile(UPDATE_PACK_PATH).size())
        {
            //传输成功
            int i = 0;
            while(i++ < 100)
            {
                //传输成功
                if(sendDtaSuccess == TOOLS_ERROR_SUCCESS)
                {
                    result = true;
                    sendDtaSuccess = TOOLS_ERROR_INITVAL;
                    break;
                }
                sleep(100);
            }
        }
    }

    deleteDirectory(QFileInfo("tmp"));
    return result;
}

void Tools::updateInsmodList(int itemType)
{
    int count = getItemCount(itemType);
    CSYSItemInfo itemTemp;

    for(int i=0;i<itemSysList->count();i++)
    {
        if(itemSysList->at(i).type == itemType)
        {
            for(int j=0;j<count;j++)
            {
                itemTemp = itemSysList->at(i+j);
                itemTemp.insmod = tableWidget->item(j,1)->checkState()==Qt::Unchecked ? 0 : 1;
                if(itemType == DRIVER || itemType == DAEMON)
                    itemTemp.dbg = tableWidget->item(j,2)->checkState()==Qt::Unchecked ? 0 : 1;
                itemTemp.status |= 0x20;
                itemSysList->replace(i+j,itemTemp);
            }
            break;
        }
    }
}


bool Tools::drawTable(int type )
{
    int index = 0;
    int itemCount = 0;
    int i = 0, j = 0;
    QTableWidgetItem *tmpItem;
    QStringList headerLabels;
    QColor color;
    QString itemDesc;
    QStringList valueList;

    tableWidget->clear();
    if(type == 0)
    {
        index = appListWidget->currentRow();
    }
    else if(type == SHELL)
    {
        index = sysListWidget->currentRow();
        if(itemSysList->at(index).srcName.contains("drivers.sh"))
        {
            itemCount = getItemCount(DRIVER);
            if(tableWidget->y() != 50)
                tableWidget->setGeometry(0,50,341,tableWidget->height()-35);
            tableWidget->setColumnCount(3);
            tableWidget->setRowCount(itemCount);
            //tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
            headerLabels<<QObject::tr("Name")<<QObject::tr("Insmod")<<QObject::tr("Debug");
            tableWidget->setHorizontalHeaderLabels(headerLabels);

            //使行列头可以用户自定义宽度
            tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
            tableWidget->verticalHeader()->setResizeMode(QHeaderView::Interactive);
            tableWidget->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
            tableWidget->verticalHeader()->show();
            tableWidget->horizontalHeader()->resizeSection(1,56);
            tableWidget->horizontalHeader()->resizeSection(2,50);

            for(i=0;i<itemSysList->count();i++)
            {
                if(itemSysList->at(i).type == DRIVER)
                {
                    for(j=0;j<itemCount;j++)
                    {
                        tableWidget->verticalHeader()->resizeSection(j,30);
                        tmpItem = new QTableWidgetItem(itemSysList->at(i+j).srcName);
                        tmpItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                        tableWidget->setItem(j, 0, tmpItem);
                        tmpItem = new QTableWidgetItem();
                        tmpItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                        if(itemSysList->at(i+j).insmod == 1)
                            tmpItem->setCheckState(Qt::Checked);
                        else
                            tmpItem->setCheckState(Qt::Unchecked);
                        tableWidget->setItem(j, 1, tmpItem);
                        tmpItem = new QTableWidgetItem();
                        tmpItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                        if(itemSysList->at(i+j).dbg == 1)
                            tmpItem->setCheckState(Qt::Checked);
                        else
                            tmpItem->setCheckState(Qt::Unchecked);
                        tableWidget->setItem(j, 2, tmpItem);
                    }
                    break;
                }
            }
        }
        else if(itemSysList->at(index).srcName.contains("daemon.sh"))
        {
            itemCount = getItemCount(DAEMON);
            if(tableWidget->y() != 50)
                tableWidget->setGeometry(0,50,341,tableWidget->height()-35);
            tableWidget->setColumnCount(3);
            tableWidget->setRowCount(itemCount);
            headerLabels<<QObject::tr("Name")<<QObject::tr("Insmod")<<QObject::tr("Debug");
            tableWidget->setHorizontalHeaderLabels(headerLabels);

            //使行列头可以用户自定义宽度
            tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
            tableWidget->verticalHeader()->setResizeMode(QHeaderView::Interactive);
            tableWidget->verticalHeader()->show();
            tableWidget->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
            tableWidget->horizontalHeader()->resizeSection(1,56);
            tableWidget->horizontalHeader()->resizeSection(2,50);

            for(i=0;i<itemSysList->count();i++)
            {
                if(itemSysList->at(i).type == DAEMON)
                {
                    for(j=0;j<itemCount;j++)
                    {
                        tableWidget->verticalHeader()->resizeSection(j,30);
                        tmpItem = new QTableWidgetItem(itemSysList->at(i+j).srcName);
                        tmpItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                        tableWidget->setItem(j, 0, tmpItem);

                        tmpItem = new QTableWidgetItem();
                        tmpItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                        if(itemSysList->at(i+j).insmod == 1)
                            tmpItem->setCheckState(Qt::Checked);
                        else
                            tmpItem->setCheckState(Qt::Unchecked);
                        tableWidget->setItem(j, 1, tmpItem);

                        tmpItem = new QTableWidgetItem();
                        tmpItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                        if(itemSysList->at(i+j).dbg == 1)
                            tmpItem->setCheckState(Qt::Checked);
                        else
                            tmpItem->setCheckState(Qt::Unchecked);
                        tableWidget->setItem(j, 2, tmpItem);
                    }
                    break;
                }
            }
        }
        else if(itemSysList->at(index).srcName.contains("setting.ini"))
        {
            QString contents;
            QString name;
            QString session;
            int count = 0;

            if(tableWidget->y() != 50)
                tableWidget->setGeometry(0,50,341,tableWidget->height()-35);
            tableWidget->setColumnCount(2);
            itemCount = daemonSet->count();
            tableWidget->setRowCount(itemCount + getSessionCount());
            headerLabels<<QObject::tr("NAME")<<QObject::tr("VALUE");
            tableWidget->setHorizontalHeaderLabels(headerLabels);

            //使行列头自适应宽度，所有列平均分来填充空白部分
            tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
            tableWidget->verticalHeader()->setResizeMode(QHeaderView::Interactive);
            tableWidget->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
            tableWidget->verticalHeader()->hide();
            tableWidget->horizontalHeader()->resizeSection(1,100);

            for(i=0;i<itemCount;i++)
            {
                if(session.compare(daemonSet->at(i).session) != 0)
                {
                    tableWidget->verticalHeader()->resizeSection(i+count,30);
                    session = daemonSet->at(i).session;
                    tableWidget->setSpan(i+count,0,1,2);
                    tmpItem = new QTableWidgetItem(session,2000);
                    tmpItem->setFlags(Qt::ItemIsEnabled);
                    tmpItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                    color = QColor("#FF8080");
                    tmpItem->setBackgroundColor(color);
                    tableWidget->setItem(i+count, 0, tmpItem);
                    count++;
                }
                name = daemonSet->at(i).key;
                //name = name.right(name.size() - name.indexOf(".") - 1);
                contents = "  Name:  " + name + "\nBrief:  " + daemonSet->at(i).brief;
                contents = contents.replace("\n","\n  ");
                valueList = daemonSet->at(i).valueList.split(";");
                j = 0;
                if(valueList.count() > 1)
                {
                    contents += "\n  ********************************  ";


                    for(j=0;j<valueList.count();j++)
                    {
                        if(valueList.at(j).isEmpty())
                            break;
                        contents += "\n            " + valueList.at(j);
                    }

                    contents += "\n  ********************************  ";
                }
                tableWidget->verticalHeader()->resizeSection(i+count,70 + j*25);
                tmpItem = new QTableWidgetItem(contents,1000+i);
                tmpItem->setFlags(Qt::ItemIsEnabled);
                if(i%2 == 0)
                    color = QColor("#D0FFFF");
                else
                    color = QColor("#FFFFD0");
                tmpItem->setBackgroundColor(color);

                tableWidget->setItem(i+count, 0, tmpItem);

                tmpItem = new QTableWidgetItem(daemonSet->at(i).value,1000+i);
                tmpItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                tmpItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
                if(i%2 == 0)
                    color = QColor("#D0FFFF");
                else
                    color = QColor("#FFFFD0");
                tmpItem->setBackgroundColor(color);
                tableWidget->setItem(i+count, 1, tmpItem);
            }
        }
    }
    else
    {
        index = sysListWidget->currentRow();
        itemCount = getItemCount(type);
        if(tableWidget->y() != 15)
            tableWidget->setGeometry(0,15,341,tableWidget->height()+35);
        tableWidget->setRowCount(itemCount);
        tableWidget->setColumnCount(1);
        tableWidget->setHorizontalHeaderLabels(QStringList(tableDesc.at(type)));
        //使行列头自适应宽度，所有列平均分来填充空白部分
        tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        tableWidget->verticalHeader()->setResizeMode(QHeaderView::Interactive);
        tableWidget->verticalHeader()->show();

        QString contents;



        for(i=0;i<itemSysList->count();i++)
        {
            if(itemSysList->at(i).type == type)
            {
                for(j=0;j<itemCount;j++)
                {
                    tableWidget->verticalHeader()->resizeSection(j,80);
                    itemDesc = getPrivateProfileString(tableHeader.at(type),itemSysList->at(i+j).srcName,tableDesc.at(type),INI_FILE_DESC);
                    contents = "Name:    " + itemSysList->at(i+j).srcName + "\nVersion:    "
                            + itemSysList->at(i+j).version + "\nBrief:    " + itemDesc;
                    tmpItem = new QTableWidgetItem(QIcon(resourceIcoPath.at(type)),contents);
                    tmpItem->setFlags(Qt::ItemIsEnabled);
                    if(index == i+j)
                    {
                        color = QColor("#FF8080");
                        tmpItem->setBackgroundColor(color);
                    }
                    tableWidget->setItem(j, 0, tmpItem);
                }
                sleep(10);
                tableWidget->setCurrentCell(index-i,0);

                break;
            }
        }
    }
    return true;
}


void Tools::creatDelFile(QString path, int itemType)
{
    CSYSItemInfo itemTemp;
    FILE *fp;
    QString fileName;
    for(int i=0;i<itemSysDelList->count();i++)
    {
        if(itemSysDelList->at(i).type == itemType)
        {
            itemTemp = itemSysDelList->at(i);
            fileName = path + "~" + itemTemp.srcName + ".";
            if(itemTemp.version.isEmpty())
                fileName += "0";
            else
                fileName += itemTemp.version;
            fp = fopen(fileName.toLatin1().data(),"w");
            if(fp != NULL)
                fclose(fp);
        }
    }
}

bool Tools::creatRSAFile(CSYSItemInfo *item)
{
    int length = 0;
    unsigned char sigData[128];
    unsigned char *srcData;
    int blank = 0;
    bool result = false;
    QString fileName;

    memset(sigData,0x00,sizeof(sigData));
    QDir dir;
    dir.mkdir(UPDATE_SCHEME_PATH);

    if(!item->srcPath.isEmpty())
    {
        fileName = UPDATE_SCHEME_PATH;
        if(item->type == KERNEL)
            fileName += "uImage.rsa";
        else if(item->type == UBOOT)
            fileName += "uboot.rsa";
        else if(item->type == CRT)
            fileName += item->srcName + ".rsa";
        else if(item->type == CRAMFS)
            fileName += item->srcName + ".rsa";
        QFile srcFile(item->srcPath);
        QFile destFile(fileName);
        if(destFile.exists())
            destFile.remove();

        if(srcFile.exists() && srcFile.open(QIODevice::ReadOnly) && destFile.open(QIODevice::WriteOnly))
        {
            length = srcFile.size();
            blank = 4 - length%4;
            if(blank % 4 > 0)
                length += blank;
            srcData = (unsigned char *)calloc(length,1);
            if(srcData == NULL)
                goto _exit;
            //memcpy(sigData,(char *)&length,4);
            /*
            sigData[0] = length/(256*256*256);
            sigData[1] = length/(256*256)%256;
            sigData[2] = length/256%256;
            sigData[3] = length%256;*/

            //destFile.write((char *)sigData,4);

            if(srcFile.read((char *)srcData,srcFile.size()) != srcFile.size())
            {
                free(srcData);
                goto _exit;
            }
            destFile.write((char *)srcData,length);
            //加入签名信息
            memset(sigData,0x00,sizeof(sigData));
            rsaSignData(srcData,length,sigData);
            destFile.write((char*)sigData,128);

            destFile.close();
            srcFile.close();
            result = true;
            free(srcData);
            goto _exit;
        }
    }
 _exit:
    return result;
}


void Tools::rsaSignData(unsigned char *srcData, int length,unsigned char *rsa_signdata)
{
    unsigned char private_exponet[128] = {  0x87,0x11,0x5f,0x1b,0xd8,0x75,0x1b,0x6b,0x09,0x3f,0x28,0x16,0x27,0x26,
                                            0x49,0x42,0xac,0xb5,0x53,0xcc,0x17,0xd3,0x2d,0xe6,0x81,0xed,0xa9,0x03,0xdc,
                                            0xd9,0xd3,0xb5,0x39,0x61,0x9a,0xae,0x62,0xde,0x78,0xc9,0xd7,0x8d,0xda,0x17,
                                            0x23,0xcb,0xf1,0xd8,0xa3,0xab,0x49,0x5a,0x05,0x50,0xce,0xae,0x76,0x5b,0xd6,
                                            0x38,0xf4,0xeb,0x0b,0x9e,0x47,0xe9,0xc5,0x07,0x15,0x9a,0x12,0xb9,0x9c,0x74,
                                            0xc6,0x5e,0xfd,0xea,0x7e,0x99,0x6e,0xbf,0x2a,0x03,0x88,0xc9,0x21,0xf6,0xcc,
                                            0xdf,0xd6,0xf3,0x3c,0xdb,0xe7,0x4d,0xf0,0x5f,0x10,0x1e,0x7e,0xb5,0xbc,0x77,
                                            0x10,0x8b,0x43,0x27,0x2e,0x80,0x61,0xda,0x2f,0xa2,0x7f,0x5c,0x80,0x1f,0x35,
                                            0x8a,0x96,0x33,0x95,0xbe,0x3e,0x32,0xa1,0x01};

    unsigned char ucPubKey_modulus[128] = { 0x94, 0x01, 0x88, 0x7a, 0xf6, 0xe5, 0x3d, 0x17, 0x07, 0xfc, 0xec, 0x81,
                                            0x17, 0xae, 0x4c, 0x54, 0xe1, 0x25, 0xab, 0x5b, 0x8b, 0xfb, 0x5e, 0x12,
                                            0x8f, 0x15, 0xb0, 0x38, 0x75, 0x19, 0xd2, 0x50, 0xde, 0x8a, 0x27, 0x3e,
                                            0x09, 0xa4, 0x9f, 0x0f, 0x46, 0x18, 0x0b, 0xa9, 0xf9, 0x07, 0x14, 0x04,
                                            0x79, 0xa5, 0xca, 0x9a, 0x91, 0x19, 0x6c, 0xe3, 0xab, 0x1f, 0x25, 0x7b,
                                            0x75, 0x89, 0xee, 0xc9, 0x95, 0x3f, 0x94, 0x9c, 0xdd, 0x79, 0x5c, 0x2b,
                                            0xac, 0xed, 0x00, 0x6f, 0x83, 0xf5, 0xf3, 0xf2, 0x07, 0x2f, 0xca, 0x39,
                                            0xa8, 0x37, 0xc5, 0x88, 0x87, 0x28, 0x1b, 0xcb, 0xe5, 0x2e, 0xf3, 0xc0,
                                            0xd1, 0xaa, 0xea, 0x48, 0x57, 0xd2, 0xeb, 0x55, 0x89, 0x7d, 0xda, 0xff,
                                            0xe8, 0xb6, 0x9e, 0x3a, 0xbb, 0x37, 0xaf, 0x37, 0x6e, 0xf9, 0xd4, 0xc5,
                                            0x34, 0x86, 0x72, 0xce, 0xb1, 0xcc, 0x9b, 0x0f};

    unsigned char rsa_data[128];
    unsigned char hash_data[20];
    memset(rsa_data,0xff,128);
    SHA1_Compute(srcData,length,hash_data);  //计算hash值
    rsa_data[0]=0;
    rsa_data[1]=1;
    memcpy(&rsa_data[108],hash_data,20);  //构造签名前数据
    RSA_PublicDecrypt(ucPubKey_modulus,128,private_exponet,128,rsa_data,128,rsa_signdata);
}


void Tools::initMsgPorBar(int maximum)
{
    msgProBar->setMinimum(0);
    msgProBar->setMaximum(maximum);
    msgProBar->setValue(0);
}

bool Tools::updateOtherBoot(void)
{
    bool result = false;
    unsigned char tempBuf[1280];
    unsigned char dataBuf[1024];
    int index = 0;
    int curPackNum = 0;
    int timeOut = 0;
    int length = 0;
    int recvPackNum = 0;
    bool recvReplay = false;
    int count = 0;
    int sendCount = 0;
    double fileLength = 0;
    QFile file;

    memset(tempBuf,0x00,sizeof(tempBuf));
    memset(dataBuf,0x00,sizeof(dataBuf));

    if(getItemCount(OTHER_BOOT) < 1 || !checkTransPort())
        goto _exit;
    for(int i=0;i<itemSysList->count();i++)
    {
        if(itemSysList->at(i).type == OTHER_BOOT)
        {
            index = i;
            break;
        }
    }
    if(itemSysList->at(index).srcPath.isEmpty())
        goto _exit;
    file.setFileName(itemSysList->at(index).srcPath);
    if(!file.open(QIODevice::ReadOnly))
        goto _exit;

    fileLength = file.size();
    //握手,发送握手包并且1秒钟内要接收到握手确认信息，否则重复10次，10次内握手不成功退出
    timeOut = 0;
    msgDis->setText(QObject::tr("READY TO SHAKE"));
    while(timeOut++ < 10)
    {
        //生成握手包头信息
        length = packData(tempBuf,sizeof(tempBuf),0,NULL,0,DOWNCMD_REQUEST);
        write((char *)tempBuf,length);
        //等待握手确认信息
        result = waitReply(100,&recvPackNum,&recvReplay);
        if(result)
        {
            if(recvPackNum == 0 && recvReplay)
                break;
            else
                result = false;
        }
    }
    if(!result)
        goto _exit;
    msgDis->setText(QObject::tr("SHAKE OK"));
    //发包，读取数据包信息，每次发送14byte包头信息+2byte数据长度+最多1024byte数据+1byte校验
    //直到数据结尾,发送分为若干个数据包，通过获得获得返回的数据包的结果确定发送数据
    initMsgPorBar(file.size());
    while(!file.atEnd())
    {
        memset(dataBuf,0x00,sizeof(dataBuf));
        //seek到要发送的数据包处
        timeOut = 0;
        while(timeOut++ < 3)
        {
            result = file.seek(curPackNum*1024);
            if(result)
                break;
        }
        if(!result)
            break;

        count = file.read((char *)dataBuf,sizeof(dataBuf));
        curPackNum++;

        //变更下载状态
        msgDis->setText(QString::number(curPackNum) + "/" + QString::number(ceil(fileLength/1024.00)));

        //组包，将包头信息，数据长度，包数据和校验位进行组装，返回组装后的数据的长度
        length = packData(tempBuf,sizeof(tempBuf),curPackNum,dataBuf,count,DOWNCMD_RESPFILE);
        //如果三次数据交互都失败，则此次通讯失败
        timeOut = 0;
        while(timeOut++ < 3)
        {
            //将组好的包进行发送
            write((char *)tempBuf,length);
            //因为发送第一个包时，设备端需要擦除整个flash，时间需要7~8秒，所以第一次等待回复超时间为10秒
            if(curPackNum == 1)
                result = waitReply(4000,&recvPackNum,&recvReplay);
            else
                result = waitReply(100,&recvPackNum,&recvReplay);
            //如果准确获得了回复信息，对恢复信息进行详细分析处理
            if(result)
            {
                //如果回复信息是OK
                if(recvReplay)
                {
                    //如果回复的当前包号不等于发送的当前包号,以得到的包号为准
                    if(recvPackNum != curPackNum)
                        curPackNum = recvPackNum;
                    break;
                }
                //回复信息是NG
                else
                {
                    //如果回复的当前包号不等于发送的当前包号,将发送的当前保号变更为回复的包号-1
                    //如果本此发送对方接收有错误，重新发送
                    if(recvPackNum != curPackNum)
                    {
                        curPackNum = recvPackNum - 1;
                        break;
                    }
                    result = false;
                }
            }
        }
        if(!result && timeOut >= 3)
            break;
        else
        {
            sendCount = curPackNum*1024 > file.size() ? file.size():curPackNum*1024;
            msgProBar->setValue(sendCount);
        }
    }
    if(result)
    {
        //数据包发送完毕发送结束包
        curPackNum++;
        length = packData(tempBuf,sizeof(tempBuf),curPackNum,NULL,0,DOWNCMD_RESPFILE);
        timeOut = 0;
        while(timeOut++ < 3)
        {
            write((char *)tempBuf,length);
            result = waitReply(100,&recvPackNum,&recvReplay);
            if(result && recvReplay)
                break;
            else
                result = false;
        }
    }

_exit:
    if(file.isOpen())
        file.close();
    return result;
}


int Tools::packData(unsigned char sendBuf[], int sendBufLen, int packNum, unsigned char dataBuf[], int dataLen, int packType)
{
    int count = 0;
    memset(sendBuf,0x00,sendBufLen);
    //打包请求类数据信息
    if(packType == DOWNCMD_REQUEST)
    {
        sendBuf[0] = 0x02;
        sendBuf[1] = 0x01;
        sendBuf[12] = 0x03;
        sendBuf[13] = OSSPD_tool_lrc(sendBuf,13);
        sendBuf[14] = dataLen+1;
        if(dataLen > 0)
            memcpy(&sendBuf[15],dataBuf,dataLen);
        sendBuf[15+dataLen] = OSSPD_tool_lrc(dataBuf,dataLen);
        count = 16 + dataLen;
    }
    //打包数据类请求信息
    else if(packType == DOWNCMD_RESPFILE)
    {
        sendBuf[0] = 0x02;
        sendBuf[1] = 0x02;
        sendBuf[10] = packNum/256;
        sendBuf[11] = packNum%256;
        sendBuf[12] = 0x03;
        sendBuf[13] = OSSPD_tool_lrc(sendBuf,13);
        sendBuf[14] = (dataLen+1)/256;
        sendBuf[15] = (dataLen+1)%256;
        if(dataLen > 0)
            memcpy(&sendBuf[16],dataBuf,dataLen);
        sendBuf[16+dataLen] = OSSPD_tool_lrc(dataBuf,dataLen);
        count = 17+dataLen;
    }
    return count;
}

bool Tools::waitReply(int timeOut, int *packNum, bool *replay)
{
    unsigned char tempBuf[1024];
    int count = 0;//每次从端口读到的字节数
    int length = 0;//已经读到的字符串的总长度
    bool result = false;
    memset(tempBuf,0x00,sizeof(tempBuf));

    for(int i=0;i<(timeOut/10);i++)
    {
        count = read((char *)&tempBuf[length],1024-length);
        if(count > 0)
        {
            length += count;
            if(tempBuf[0] != 0x02 || length < 15 || length < (15 + tempBuf[14]))
            {
                //如果应答包在垃圾数据中间，则从垃圾数据中找出应答包，基本上不会用到此段代码
                if(tempBuf[0] != 0x02)
                {
                    int i;
                    for(i=1;i<length;i++)
                    {
                        if(tempBuf[i] == 0x02)
                        {
                            unsigned char temp[1024];
                            memset(temp,0x00,1024);
                            memcpy(temp,&tempBuf[i],length-i);
                            memset(tempBuf,0x00,sizeof(tempBuf));
                            memcpy(tempBuf,temp,length-i);
                            length -= i;
                            break;
                        }
                        if(i == length-1)
                        {
                            memset(tempBuf,0x00,sizeof(tempBuf));
                            length = 0;
                        }
                    }
                }
                sleep(10);
                continue;
            }
            if(tempBuf[1] != 0x01 || tempBuf[12] != 0x03 || tempBuf[13] != OSSPD_tool_lrc(tempBuf,13))
                break;
            *packNum = tempBuf[10]*256 + tempBuf[11];
            if(tempBuf[14] == 0x03)
            {
                if(tempBuf[15] == 'o' && tempBuf[16] == 'k')
                    *replay = true;
                else if(tempBuf[15] == 'n' && tempBuf[16] == 'g')
                    *replay = false;
            }
            result = true;
            break;
        }
        sleep(10);
    }

    return result;
}

void Tools::initTreeWidget(QString iniPath)
{
    QStringList rootPath;
    int i = 0, j = 0, count = 0;
    QString iniNumber;
    QStringList allKeyList;
    QStringList saveKeyList;
    bool removeFlag = false;//初始化时是否有垃圾选项
    rootPath<<QObject::tr("SCHEME");
    rootTreeWdtItem = new QTreeWidgetItem(rootPath);


    rootTreeWdtItem->setIcon(0, QIcon(":resource/ico/ws.png"));
    treeWidget->addTopLevelItem(rootTreeWdtItem);

    //初始化时加载配置中的目录
    QSettings iniSettings(iniPath,QSettings::IniFormat);
    iniSettings.setIniCodec(INI_FILE_CODE);
    iniSettings.beginGroup("SCHEME");
    allKeyList = iniSettings.allKeys();
    saveKeyList = allKeyList;
    count = allKeyList.count();
    for(i=0;i<count;i++)
    {
        if(!addSchemeToWidget(iniSettings.value(allKeyList.at(i)).toString()))
        {
            saveKeyList.replace(i,"");
            removeFlag = true;
        }
    }

    if(removeFlag)
    {
        for(i=0,j=0;i<count;i++)
        {
            if(!saveKeyList.at(i).isEmpty())
            {
                iniNumber.clear();
                if(j <= 9)
                    iniNumber = "0";
                iniNumber += QString::number(j);
                iniSettings.setValue(iniNumber,iniSettings.value(saveKeyList.at(i)).toString());
                j++;
            }
        }
        for(;j<count;j++)
        {
            iniNumber.clear();
            if(j <= 9)
                iniNumber = "0";
            iniNumber += QString::number(j);
            iniSettings.remove(iniNumber);
        }
    }
    iniSettings.endGroup();

    rootTreeWdtItem->setExpanded(true);
}


bool Tools::saveScheme(char *iniName)
{
    QString iniPath = UPDATE_SCHEME_PATH + QString::fromLocal8Bit(iniName) + ".ini";
    CAPPItemInfo appItemTemp;
    CSYSItemInfo sysItemTemp;
    QString iniNumber;
    //用于记录需要保存的信息数
    int count = 0;

    //判断文件是否存在
    if(QFileInfo(iniPath).exists())
        return false;
    //记录已经有的配置文件名称
    QSettings iniSetting(INI_FILE_CONFIG,QSettings::IniFormat);
    iniSetting.setIniCodec(INI_FILE_CODE);
    //开始写配置文件
    QSettings setting(iniPath,QSettings::IniFormat);
    setting.setIniCodec(INI_FILE_CODE);
    //将APP的信息保存到配置中
    setting.beginGroup("APP");
    for(int i=0;i<itemAppList->count();i++)
    {
        if((itemAppList->at(i).appStatus & 0xB0) == 0x30)
        {
            QString logoPicTemp;
            appItemTemp = itemAppList->at(i);
            setting.beginGroup(QString::number(appItemTemp.appNumber));
            setting.setValue("NAME",appItemTemp.appDesc);
            setting.setValue("ID",appItemTemp.appNumber);
            setting.setValue("STATUS",appItemTemp.appStatus);
            setting.setValue("HIDE",appItemTemp.hideFlag?1:0);
            setting.setValue("ITEM_CODE",appItemTemp.itemCode);
            setting.setValue("VERSION",appItemTemp.version);
            setting.setValue("SETTING",appItemTemp.appSettings);
            setting.setValue("IOC_PATH",appItemTemp.ICOPicPath);
            setting.setValue("SRC_PATH",appItemTemp.srcPath);
            setting.setValue("OWNER",appItemTemp.owner);
            for(int j=0;j<appItemTemp.logoPicPath.count();j++)
            {
                logoPicTemp += appItemTemp.logoPicPath.at(j) + ";";
            }
            setting.setValue("LOGO_PIC_PATH",logoPicTemp);
            setting.endGroup();
            count++;
        }
    }
    setting.endGroup();

    //将SYS的信息保存到配置中
    setting.beginGroup("SYS");
    for(int i=0;i<itemSysList->count();i++)
    {
        if((itemSysList->at(i).status & 0xB0) == 0x30)
        {
            sysItemTemp = itemSysList->at(i);

            if(sysItemTemp.srcName.compare("drivers.sh") == 0
                    || sysItemTemp.srcName.compare("daemon.sh") == 0)
                continue;

            setting.beginGroup(sysItemTemp.srcName);
            setting.setValue("NAME",sysItemTemp.srcName);
            setting.setValue("STATUS",sysItemTemp.status);
            setting.setValue("TYPE",sysItemTemp.type);
            setting.setValue("INSMOD",sysItemTemp.insmod);
            setting.setValue("VERSION",sysItemTemp.version);
            setting.setValue("DEBUG",sysItemTemp.dbg);
            setting.setValue("SRC_PATH",sysItemTemp.srcPath);
            setting.endGroup();
            count++;
        }
    }
    setting.endGroup();

    //将应用DTA的信息保存到配置中
    setting.beginGroup("DTA");

    for(int i=0;i<itemAppList->count();i++)
    {
        if((itemAppList->at(i).appStatus & 0x80) == 0x80)
        {
            appItemTemp = itemAppList->at(i);
            setting.beginGroup(appItemTemp.srcName);
            setting.setValue("NAME",appItemTemp.srcName);
            setting.setValue("SRC_PATH",appItemTemp.dtaPath);
            setting.endGroup();
            count++;
        }
    }

    //将SYS中DTA的信息保存到配置中
    QStringList dtaPathList;
    int dtaCount = 0;
    for(int i=0;i<itemSysList->count();i++)
    {
        if((itemSysList->at(i).status & 0x80) == 0x80)
        {
            sysItemTemp = itemSysList->at(i);

            dtaCount = dtaPathList.count();
            if(dtaCount == 0)
                dtaPathList<<sysItemTemp.dtaPath;
            else
            {
                int j;
                for(j=0;j<dtaCount;j++)
                {
                    if(dtaPathList.at(j).compare(sysItemTemp.dtaPath) == 0)
                        break;
                }
                if(j <= dtaCount)
                    continue;
            }
            setting.beginGroup(QFileInfo(sysItemTemp.dtaPath).fileName());
            setting.setValue("NAME",QFileInfo(sysItemTemp.dtaPath).fileName());
            setting.setValue("SRC_PATH",sysItemTemp.dtaPath);
            setting.endGroup();
            count++;
        }
    }
    setting.endGroup();

    //只有Count大于0时才写配置文件
    if(count > 0)
    {
        int addIndex = -1;
        iniSetting.beginGroup("SCHEME");
        //确定插入位置
        addIndex = iniSetting.allKeys().count();
        iniNumber.clear();
        if(addIndex <= 9)
            iniNumber = "0";
        iniNumber += QString::number(addIndex);
        iniSetting.setValue(iniNumber,iniPath);
        iniSetting.endGroup();
        sleep(200);
        //添加完成后更新SCHEME
        addSchemeToWidget(iniPath);
    }

    return true;
}


bool Tools::addSchemeToWidget(QString iniPath)
{
    QFileInfo fileInfo(iniPath);
    QTreeWidgetItem *treeWidgetItem;
    QStringList childList;
    QString content;
    if(!fileInfo.exists())
        return false;
    //将配置文件名添加到scheme中
    treeWidgetItem = new QTreeWidgetItem(QStringList(fileInfo.fileName().left(fileInfo.fileName().size()-4)));
    treeWidgetItem->setIcon(0, QIcon(":resource/ico/scm.png"));
    rootTreeWdtItem->addChild(treeWidgetItem);

    saveCfgList<<fileInfo.fileName().left(fileInfo.fileName().size()-4);


    //开始写配置文件
    QSettings setting(iniPath,QSettings::IniFormat);
    setting.setIniCodec(INI_FILE_CODE);
    //将APP的信息从配置中读取到Scheme
    setting.beginGroup("APP");
    childList = setting.childGroups();
    for(int i=0;i<childList.count();i++)
    {
        setting.beginGroup(childList.at(i));
        treeWidgetItem = new QTreeWidgetItem(setting.value("NAME").toStringList());
        content = "NAME: " + setting.value("NAME").toString() + "\nID: " + setting.value("ID").toString()
                + "VERSION: " + setting.value("VERSION").toString();
        treeWidgetItem->setToolTip(0,content);
        treeWidgetItem->setIcon(0, QIcon(resourceIcoPath.at(APPLICATION)));
        rootTreeWdtItem->child(rootTreeWdtItem->childCount()-1)->addChild(treeWidgetItem);

        setting.endGroup();
    }
    setting.endGroup();

    //将SYS的信息从配置中读取到Scheme
    setting.beginGroup("SYS");
    childList = setting.childGroups();
    for(int i=0;i<childList.count();i++)
    {
        setting.beginGroup(childList.at(i));
        treeWidgetItem = new QTreeWidgetItem(setting.value("NAME").toStringList());
        content = "NAME: " + setting.value("NAME").toString() + "\nVERSION: " + setting.value("VERSION").toString()
                + "\nBRIEF: " ;
        content += getPrivateProfileString(tableHeader.at(setting.value("TYPE").toInt()),
                        setting.value("NAME").toString(),tableDesc.at(setting.value("TYPE").toInt()),INI_FILE_DESC);
        treeWidgetItem->setToolTip(0,content);
        treeWidgetItem->setIcon(0, QIcon(resourceIcoPath.at(setting.value("TYPE").toInt())));
        rootTreeWdtItem->child(rootTreeWdtItem->childCount()-1)->addChild(treeWidgetItem);
        setting.endGroup();
    }
    setting.endGroup();

    //将DTA的信息从配置中读取到Scheme
    setting.beginGroup("DTA");
    childList = setting.childGroups();
    for(int i=0;i<childList.count();i++)
    {
        setting.beginGroup(childList.at(i));
        treeWidgetItem = new QTreeWidgetItem(setting.value("NAME").toStringList());
        content = "NAME: " + setting.value("NAME").toString() + "\nPATH: " + setting.value("SRC_PATH").toString();
        treeWidgetItem->setToolTip(0,content);
        treeWidgetItem->setIcon(0, QIcon(":resource/ico/zip.png"));
        rootTreeWdtItem->child(rootTreeWdtItem->childCount()-1)->addChild(treeWidgetItem);
        setting.endGroup();
    }
    setting.endGroup();
    return true;
}


bool Tools::removeScheme(QTreeWidgetItem *curTreeWdtItem)
{
    int i = 0;
    QSettings iniSettings(INI_FILE_CONFIG,QSettings::IniFormat);
    iniSettings.setIniCodec(INI_FILE_CODE);
    QTreeWidgetItem *tempTreeItem = curTreeWdtItem;
    QString iniNumber;
    int index;
    while(tempTreeItem->parent() != rootTreeWdtItem)
    {
        tempTreeItem = tempTreeItem->parent();
    }
    index = rootTreeWdtItem->indexOfChild(tempTreeItem);
    iniNumber.clear();
    if(index <= 9)
        iniNumber = "0";
    iniNumber += QString::number(index);
    saveCfgList.removeAt(index);
    rootTreeWdtItem->takeChild(index);

    iniSettings.beginGroup("SCHEME");
    //删除配置文件
    QFile file(iniSettings.value(iniNumber).toString());
    if(file.exists())
        file.remove();
    iniSettings.remove(iniNumber);
    QStringList allKeyList = iniSettings.childKeys();

    for(i=index;i<allKeyList.count();i++)
    {
        iniNumber.clear();
        if(i <= 9)
            iniNumber = "0";
        iniNumber += QString::number(i);
        iniSettings.setValue(iniNumber,iniSettings.value(allKeyList.at(i)));
    }
    if(iniSettings.childKeys().count() > 0)
    {
        //删除最后一个多余的key
        iniNumber.clear();
        if(i <= 9)
            iniNumber = "0";
        iniNumber += QString::number(i);
        iniSettings.remove(iniNumber);
    }
    iniSettings.endGroup();
    return true;
}

bool Tools::addItemFromScheme(QTreeWidgetItem *curTreeWdtItem)
{
    QSettings iniSettings(INI_FILE_CONFIG,QSettings::IniFormat);
    iniSettings.setIniCodec(INI_FILE_CODE);
    QTreeWidgetItem *tempTreeItem = curTreeWdtItem;
    QString iniPath;
    QString srcPath;
    QStringList childList;
    QString iniNumber;
    int index;
    //查找顶层项
    while(tempTreeItem->parent() != rootTreeWdtItem)
    {
        tempTreeItem = tempTreeItem->parent();
    }
    index = rootTreeWdtItem->indexOfChild(tempTreeItem);
    if(index <= 9)
        iniNumber = "0";
    iniNumber += QString::number(index);
    iniSettings.beginGroup("SCHEME");
    iniPath = iniSettings.value(iniNumber).toString();
    iniSettings.endGroup();

    if(iniPath.isEmpty() || !QFile(iniPath).exists())
        return false;
    QSettings iniScheme(iniPath,QSettings::IniFormat);
    iniScheme.setIniCodec(INI_FILE_CODE);

    //读取APP中的信息
    iniScheme.beginGroup("APP");
    childList = iniScheme.childGroups();
    for(int i=0;i<childList.count();i++)
    {
        CAPPItemInfo itemTemp;

        iniScheme.beginGroup(childList.at(i));

        itemTemp.appDesc = iniScheme.value("NAME").toString();
        itemTemp.appNumber = iniScheme.value("ID").toInt();
        itemTemp.appStatus = iniScheme.value("STATUS").toInt();
        itemTemp.hideFlag = iniScheme.value("HIDE").toBool();
        itemTemp.itemCode = iniScheme.value("ITEM_CODE").toString();
        itemTemp.version = iniScheme.value("VERSION").toString();
        itemTemp.appSettings = iniScheme.value("SETTING").toString();
        itemTemp.ICOPicPath = iniScheme.value("IOC_PATH").toString();
        itemTemp.logoPicPath = iniScheme.value("LOGO_PIC_PATH").toString().split(";");
        //去掉logoPicPath中的无效数据
        for(int j=0;j<itemTemp.logoPicPath.count();j++)
        {
            if(itemTemp.logoPicPath.at(j).isEmpty())
            {
                itemTemp.logoPicPath.removeAt(j);
                j--;
            }
        }

        itemTemp.srcPath = iniScheme.value("SRC_PATH").toString();
        itemTemp.owner = iniScheme.value("OWNER").toString();
        addItemToListWidget(&itemTemp,0);
        iniScheme.endGroup();
    }
    iniScheme.endGroup();

    //读取SYS中的信息
    iniScheme.beginGroup("SYS");
    childList = iniScheme.childGroups();
    for(int i=0;i<childList.count();i++)
    {
        CSYSItemInfo itemTemp;
        iniScheme.beginGroup(childList.at(i));

        itemTemp.srcName = iniScheme.value("NAME").toString();
        itemTemp.status = iniScheme.value("STATUS").toInt();
        itemTemp.type = iniScheme.value("TYPE").toInt();
        itemTemp.version = iniScheme.value("VERSION").toString();
        itemTemp.insmod = iniScheme.value("INSMOD").toInt();
        itemTemp.dbg = iniScheme.value("DEBUG").toInt();
        itemTemp.srcPath = iniScheme.value("SRC_PATH").toString();
        addItemToListWidget(&itemTemp,itemTemp.type);
        iniScheme.endGroup();
    }
    iniScheme.endGroup();

    //读取DTA中的信息
    iniScheme.beginGroup("DTA");
    childList = iniScheme.childGroups();
    for(int i=0;i<childList.count();i++)
    {
        iniScheme.beginGroup(childList.at(i));
        srcPath = iniScheme.value("SRC_PATH").toString();
        if(!srcPath.isEmpty() && QFile(srcPath).exists())
            addDtaToListWidget(srcPath);
        iniScheme.endGroup();
    }

    iniScheme.endGroup();

    return true;
}

int Tools::analysisDesc(QString desc, QString *brief, QString *valueList)
{
    int type = 0;
    //去掉两端空格
    QString strTemp = desc;
    QString valueTemp;
    QRegExp regExp("[0-9]{1,2}-");
    //判断第一个字符是否为#号
    if(strTemp.left(1).compare("#") != 0)
        return -1;
    //将#号去掉
    strTemp = strTemp.right(strTemp.size() - 1);
    //获取类型，即#号后的数值
    type = strTemp.left(strTemp.indexOf(" ")).toInt();
    //将已经获取的字符串去掉
    if(strTemp.contains(" "))
        strTemp = strTemp.right(strTemp.size() - strTemp.indexOf(" ") -1);
    else
        strTemp = "";
    //判断是否有|
    if(strTemp.contains("|"))
    {
        //|前的内容都是简介
        *brief = strTemp.left(strTemp.indexOf("|") - 1);
        //去掉|前的内容
        strTemp = strTemp.right(strTemp.size() - strTemp.indexOf("|") - 1);
        //循环判断是否有正则表达式"[0-9]{1,2}-"
        while(strTemp.contains(regExp))
        {
            //去掉两端的空格
            strTemp = strTemp.simplified();
            //判断中间是否有空格，多个不同的值需要用空格隔开
            if(strTemp.count(regExp) > 1)
            {
                //截取-后和空格前的内容
                valueTemp = strTemp.left(strTemp.indexOf(regExp,3) - 1);
                *valueList += valueTemp + ";";
                //去掉已经截取的信息
                strTemp = strTemp.right(strTemp.size() - strTemp.indexOf(regExp,3));
            }
            else
            {
                //没有空格说明是最后一个值，取值后退出
                valueTemp = strTemp;
                *valueList += valueTemp;
                break;
            }
        }
    }
    //没有|时后面内容都是简介
    else
        *brief = strTemp;

    return type;
}

int Tools::getSessionCount(void)
{
    int result = 0;
    int count = daemonSet->count();
    QString session;
    for(int i=0;i<count;i++)
    {
        if(session.compare(daemonSet->at(i).session) != 0)
        {
            session = daemonSet->at(i).session;
            result++;
        }
    }
    return result;
}

bool Tools::updateDaemonSet(void)
{
    int count = tableWidget->rowCount();
    int type = 0;
    DAEMON_SET daemonTemp;
    settingMoidfy = 1;
    for(int i=0;i<count;i++)
    {
        type = tableWidget->item(i,0)->type();
        if(type == 2000)
            continue;
        daemonTemp = daemonSet->at(type%1000);
        daemonTemp.value = tableWidget->item(i,1)->text();
        daemonSet->replace(type%1000,daemonTemp);
    }
    return true;
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


QStringList Tools::getChildValues(QString sessionName, QString fileName)
{
    QStringList values;
    QStringList keys;
    QSettings setting(fileName,QSettings::IniFormat);
    setting.setIniCodec(INI_FILE_CODE);
    setting.beginGroup(sessionName);
    keys = setting.childKeys();
    for(int i=0;i<keys.count();i++)
        values<<setting.value(keys.at(i)).toString();
    setting.endGroup();

    return values;
}

int Tools::read(char *readBuf, int bufSize)
{
    int ret = 0;

    if(!checkTransPort())
    {
        ret = -2;
        return ret;
    }

    if(*transType == 0)
    {
        if(serialPort->bytesAvailable())
        {
            ret = serialPort->read(readBuf,bufSize);
        }
    }
    else if(*transType == 1)
    {
        ret = tcpSocket->read(readBuf,bufSize);
    }
    return ret;
}

int Tools::write(char *writeBuf, int bufSize)
{
    int ret = 0;
    if(!checkTransPort())
    {
        ret = -2;
        return ret;
    }
    if(*transType == 0)
    {
        ret = serialPort->write(writeBuf,bufSize);
        serialPort->waitForBytesWritten(-1);
    }
    else if(*transType == 1)
    {
        ret = tcpSocket->write(writeBuf,bufSize);
    }


    return ret;
}

bool Tools::checkTransPort(void)
{
    bool ret = false;
    if(*transType == 0 && serialPort->isOpen())
    {
        ret = true;
    }
    else if(*transType == 1 && tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        ret = true;
    }
    return ret;
}

bool Tools::openSerialPort(void)
{
    QString port,baudRate;
    port = getPrivateProfileString("PortSettings","port","COM1",NULL);
    baudRate = getPrivateProfileString("PortSettings","baudRate","115200",NULL);
    //msgLable->setText("  "+port+" :"+baudRate);
    serialPort->setPortName(port);
    serialPort->setBaudRate((BaudRateType)getPrivateProfileInt("PortSettings","baudRate",115200,NULL));
    serialPort->setDataBits((DataBitsType)getPrivateProfileInt("PortSettings","dataBits",8,NULL));
    serialPort->setParity((ParityType)getPrivateProfileInt("PortSettings","parity",0,NULL));
    serialPort->setStopBits((StopBitsType)getPrivateProfileInt("PortSettings","stopBIts",0,NULL));
    if(!serialPort->isOpen())
    {
        serialPort->open(QIODevice::ReadWrite);
    }

    if(serialPort->isOpen())
    {

        if(serialPort->queryMode() == QextSerialPort::Polling)
        {
            readTimer->start();
        }
        msgLable->setStyleSheet("QLabel { background-color : #a3cf62;}");
        return true;
    }

    return false;
}

void Tools::closeSerialPort(void)
{
    if(serialPort->isOpen())
        serialPort->close();

    msgLable->setStyleSheet("QLabel { background-color : #f58f98;}");
}
/*
bool Tools::connectNetWork(void)
{
    QString IP;
    int Port, i;

    IP = getPrivateProfileString("PortSettings","IP","192.168.1.100",NULL);
    Port = getPrivateProfileInt("PortSettings","IPPort",8000,NULL);
    msgLable->setText("  "+IP+":"+QString::number(Port));
    tcpSocket->abort();
    tcpSocket->connectToHost(IP,Port);
    for(i=0;i<20;i++)
    {
        sleep(100);
        if(checkTransPort())
        {
            msgLable->setStyleSheet("QLabel { background-color : #a3cf62;}");
            return true;
        }
    }
    return false;
}

void Tools::disconnectNetWork(void)
{
    if(tcpSocket->state() == QAbstractSocket::ConnectedState)
        tcpSocket->disconnectFromHost();
     msgLable->setStyleSheet("QLabel { background-color : #f58f98;}");
}*/

void Tools::formatKey(QString *keyBuf)
{
    QString strBuf;
    int i = 0, count = 0;
    keyBuf->replace(" ","");
    strBuf = keyBuf->toUpper();
    keyBuf->clear();
    count = strBuf.count();
    for(i=0;i<count;i++)
    {
        if(i != 0 && i%4 == 0)
            *keyBuf += " ";
        *keyBuf += strBuf.mid(i,1);
    }
}




bool Tools::releasePlnToUSB(QTreeWidgetItem *treeWidgetItem)
{
    QStringList childList;
    QString settingPath;
    QList<CAPPItemInfo> appItemList;
    int i, j, index;

    //获取setting的路径
    while(treeWidgetItem->parent() != rootTreeWdtItem)
        treeWidgetItem = treeWidgetItem->parent();
    index = rootTreeWdtItem->indexOfChild(treeWidgetItem);

    QSettings cfgSetting(INI_FILE_CONFIG,QSettings::IniFormat);
    cfgSetting.setIniCodec(INI_FILE_CODE);
    cfgSetting.beginGroup("SCHEME");
    QString strIndex;
    if(index < 10)
        strIndex = "0";
    strIndex += QString::number(index);
    settingPath = cfgSetting.value(strIndex).toString();
    cfgSetting.endGroup();

    QSettings setting(settingPath,QSettings::IniFormat);
    setting.setIniCodec(INI_FILE_CODE);

    appItemList.clear();
    setting.beginGroup("APP");
    childList = setting.childGroups();

    //处理APP
    for(i=0;i<childList.count();i++)
    {
        CAPPItemInfo itemTemp;

        setting.beginGroup(childList.at(i));

        itemTemp.appDesc = setting.value("NAME").toString();
        itemTemp.appNumber = setting.value("ID").toInt();
        itemTemp.appStatus = setting.value("STATUS").toInt();
        itemTemp.hideFlag = setting.value("HIDE").toBool();
        itemTemp.itemCode = setting.value("ITEM_CODE").toString();
        itemTemp.version = setting.value("VERSION").toString();
        itemTemp.appSettings = setting.value("SETTING").toString();
        itemTemp.ICOPicPath = setting.value("IOC_PATH").toString();
        itemTemp.logoPicPath = setting.value("LOGO_PIC_PATH").toString().split(";");
        //去掉logoPicPath中的无效数据
        for(j=0;j<itemTemp.logoPicPath.count();j++)
        {
            if(itemTemp.logoPicPath.at(j).isEmpty())
            {
                itemTemp.logoPicPath.removeAt(j);
                j--;
            }
        }

        itemTemp.srcPath = setting.value("SRC_PATH").toString();
        itemTemp.owner = setting.value("OWNER").toString();
        itemTemp.uiScriptPath = setting.value("UI_SCRIPT_PATH").toString();
        appItemList<<itemTemp;
        setting.endGroup();
    }
    setting.endGroup();

    //处理DTA
    setting.beginGroup("DTA");
    childList = setting.childGroups();
    for(i=0;i<childList.count();i++)
    {
        CAPPItemInfo itemTemp;
        QString cmd;
        QString dtaPath;
        QString basePath;
        QTime curTime;
        QDate curDate;
        QProcess process;
        QDir dir;

        setting.beginGroup(childList.at(i));

        dtaPath = setting.value("SRC_PATH").toString();

        if(!QFileInfo(dtaPath).exists() || QFileInfo(dtaPath).fileName().contains("driver_lib_font"))
        {
            setting.endGroup();
            continue;
        }

        curTime = QTime::currentTime();
        curDate = QDate::currentDate();
        basePath = "DTA/" + curDate.toString("yyMMdd")+curTime.toString("hhmmsszzz");
        //创建文件夹
        dir.mkpath(basePath);
        cmd = "DTA.exe x -ibck -y -o+ "+QFileInfo(dtaPath).absoluteFilePath()+" "+QFileInfo(basePath).absoluteFilePath();
        if(process.execute(cmd) == 0)
        {
            sortingFileDir(&itemTemp,basePath);
            if(itemTemp.appNumber > 0)
            {
                appItemList<<itemTemp;
            }
        }
        setting.endGroup();
    }
    setting.endGroup();

    if(appItemList.count() <= 0)
    {
        //判断是否还有SYS选项，如果没有则删除
        setting.beginGroup("SYS");
        childList = setting.childGroups();
        setting.endGroup();
        if(childList.count() <= 0)
            removeScheme(treeWidgetItem);
        return false;
    }
    //打包任务计划列表中的应用,生成pln文件
    return packPln(&appItemList,QFileInfo(settingPath).fileName().replace(".ini",""));
}


bool Tools::packPln(QList<CAPPItemInfo> *itemList, QString schemeName)
{
    bool result = false;
    QString basePath = "tmp/download/";
    QString iniPath;
    int i, j, imageCount, itemCount;
    char appID[6];
    QDir dir;
    CAPPItemInfo appTemp;
    QString binPath;
    QProcess process;
    QFileInfo zipFileInfo(UPDATE_PACK_TEMP_PATH);
    QString cmd;

    itemCount = itemList->count();

    initMsgPorBar(itemCount);

    //创建文件夹
    if(QFileInfo("tmp").exists())
        deleteDirectory(QFileInfo("tmp"));

    if(!dir.mkpath(basePath))
        return result;

    for(i=0;i<itemCount;i++)
    {
        binPath = basePath + "app/";
        appTemp = itemList->at(i);
        memset(appID,0x0,sizeof(appID));
        //创建文件夹
        sprintf(appID,"%05d",appTemp.appNumber);
        binPath += QString(appID)+"/";
        dir.setPath("./");
        if(dir.mkpath(binPath))
        {
            sleep(100);
            dir.setPath(basePath);
            dir.mkdir("daemon");
        }
        else
            return result;
        //拷贝ICO
        if(!appTemp.ICOPicPath.isEmpty())
            QFile::copy(appTemp.ICOPicPath,binPath+"app."+QFileInfo(appTemp.ICOPicPath).suffix());
        //拷贝qss
        if(!appTemp.uiScriptPath.isEmpty())
            QFile::copy(appTemp.uiScriptPath,binPath+QFileInfo(appTemp.uiScriptPath).fileName());

        //拷贝Image
        dir.setPath(binPath);
        dir.mkdir("image");
        imageCount = appTemp.logoPicPath.count();
        for(j=0;j<imageCount;j++)
        {
            QString logoPath = appTemp.logoPicPath.at(j);
            QFile::copy(logoPath,binPath+"image/"+QFileInfo(logoPath).fileName());
        }

        //拷贝配置文件
        sleep(100);
        QFile iniFile(binPath + INI_FILE_CONFIG);
        if(iniFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            //iniFile.write("[SYS]\n");
            QTextStream out(&iniFile);
            out.setCodec(INI_FILE_CODE);
            out<<"[SYS]\n";
            out<<appTemp.appSettings;
            out<<"\0";

            iniFile.close();
        }

        //拷贝源文件
        if(!appTemp.srcPath.isEmpty())
        {
            QTime curTime = QTime::currentTime();
            QDate curDate = QDate::currentDate();
            QString editDate;
            editDate = curDate.toString("yyyy.MM.dd")+" "+curTime.toString("hh:mm");
            appTemp.editDate = editDate;
            signApp(appTemp.srcPath,binPath+QString(appID));
            //QFile::copy(appTemp.srcPath,binPath+QString(appID));
        }

        //生成系统配置文件
        sleep(100);
        iniPath = basePath + "daemon/";
        iniPath += INI_FILE_CONFIG;
        creatIni(iniPath,APPLICATION,&appTemp);
        //打包
        cmd = "DTA.exe a -ep1 -m5 -r -ibck -o+  "+zipFileInfo.absoluteFilePath()+" "+QFileInfo(basePath).absoluteFilePath();
        if(process.execute(cmd) == 0)
        {
            //修改文件名
            QString dtaName = "tmp/" + QString::number(appTemp.appNumber) + ".dta";
            QFile(UPDATE_PACK_TEMP_PATH).rename(dtaName);
            itemList->replace(i,appTemp);
            sleep(100);
        }
        else
            return result;

        deleteDirectory(QFileInfo("tmp/download"));
        msgProBar->setValue(i+1);
        sleep(100);
    }

    zipFileInfo.setFile("tmp/");
    creatIni("tmp/conf.ini",INI_TYPE_PLAN,itemList);
    cmd = "DTA.exe a -ep1 -m5 -r -ibck -o+  "+QFileInfo(UPDATE_SCHEME_DATA_PATH).absoluteFilePath()+" "+zipFileInfo.absoluteFilePath()+"/*";
    if(process.execute(cmd) == 0)
    {
        //修改文件名
        QString dtasName = UPDATE_SCHEME_PATH + schemeName + ".pln";
        if(QFileInfo(dtasName).exists())
            QFile(dtasName).remove();
        sleep(100);
        QFile(UPDATE_SCHEME_DATA_PATH).rename(dtasName);
        result = true;
    }
    if(QFileInfo("tmp").exists())
        deleteDirectory(QFileInfo("tmp"));
    return result;
}


bool Tools::unpackPln(QString plnPath)
{
    QString basePath = UPDATE_SCHEME_PATH;
    QDir dir;
    QString cmd;
    QProcess process;

    //设置pln解压缩到的文件夹路径
    basePath += QFileInfo(plnPath).baseName();

    //创建文件夹
    if(QFileInfo(basePath).exists())
        deleteDirectory(QFileInfo(basePath));
    sleep(100);
    dir.mkpath(basePath);
    //创建文件夹
    if(!dir.mkpath(basePath))
        return false;
    cmd = "DTA.exe x -ibck -y -o+ "+QFileInfo(plnPath).absoluteFilePath()+" "+QFileInfo(basePath).absoluteFilePath();
    if(process.execute(cmd) == 0)
    {
        //遍历文件
        basePath += "/";
        if(QFileInfo(basePath+INI_FILE_CONFIG).exists())
            QFile(basePath+INI_FILE_CONFIG).remove();
        sleep(100);
        sortingFileDir(NULL,basePath);
        return true;
    }
    return false;
}

void Tools::repeatRsaSignData(unsigned char *srcData, int length, unsigned char offsetData[128], unsigned char *rsa_signdata)
{
    unsigned char tmpBuf[1024];
    int i = 0, offset[4], cn = 0, c = 0;

    memset(tmpBuf,0,sizeof(tmpBuf));

    //计算偏移量并且将偏移后的数据读出来,偏移量分为四部分,根据前一次签名的128字节签名数据得到,
    for(i=0;i<4;i++)
    {
        offset[i] = (int)OSSPD_tool_lrc(offsetData + i*32,32);
        if(i > 0)
            offset[i] += offset[i-1] +256;
        if(offset[i]/length > 0)
            offset[i] %= length;
        //从偏移量位置已经拷贝的字节数使用c计数,共拷贝256个字节
        c = 0;
        while(c != 256)
        {
            //如果未读字节大于偏移后的字节
            if(256-c > length - offset[i])
            {
                memcpy(&tmpBuf[cn],srcData+offset[i],length - offset[i]);
                c += length - offset[i];
                cn += length - offset[i];
                offset[i] = 0;
            }
            else
            {
                memcpy(&tmpBuf[cn],srcData+offset[i],256-c);
                cn += 256-c;
                c = 256;
            }
        }
    }

    rsaSignData(tmpBuf,sizeof(tmpBuf),rsa_signdata);
}

bool Tools::signApp(QString srcPath, QString signPath)
{
    int length = 0;
    unsigned char sigData[128];
    unsigned char repeatSignData[128];
    unsigned char *srcData;
    int blank = 0;
    bool result = false;
    QFile srcFile(srcPath);
    QFile destFile(signPath);
    if(destFile.exists())
        destFile.remove();
    if(srcFile.exists() && srcFile.open(QIODevice::ReadOnly) && destFile.open(QIODevice::WriteOnly))
    {
        length = srcFile.size();
        blank = 4 - length%4;
        if(blank % 4 > 0)
            length += blank;
        srcData = (unsigned char *)calloc(length,1);
        if(srcData == NULL)
            goto _exit;
        //memcpy(sigData,(char *)&length,4);
        /*
        sigData[0] = length/(256*256*256);
        sigData[1] = length/(256*256)%256;
        sigData[2] = length/256%256;
        sigData[3] = length%256;*/

        //destFile.write((char *)sigData,4);

        if(srcFile.read((char *)srcData,srcFile.size()) != srcFile.size())
        {
            free(srcData);
            goto _exit;
        }
        destFile.write((char *)srcData,length);
        //加入签名信息
        memset(sigData,0x00,sizeof(sigData));
        rsaSignData(srcData,length,sigData);
        destFile.write((char*)sigData,128);

        memset(repeatSignData,0,sizeof(repeatSignData));
        repeatRsaSignData(srcData,length,sigData,repeatSignData);
        destFile.write((char*)repeatSignData,128);
        destFile.close();
        srcFile.close();
        result = true;
        free(srcData);
        goto _exit;
    }
_exit:
   return result;
}
