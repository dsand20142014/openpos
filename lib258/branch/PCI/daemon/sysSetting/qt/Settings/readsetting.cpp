#include <QDebug>
#include "readsetting.h"

ReadSetting readSetting;

ReadSetting::ReadSetting()
{

}

unsigned char ReadSetting::readKeys(QSettings *settings, QString group)
{
    unsigned char successFlag;
    QString strTmp;
    QStringList keysTmp,valueTmp;
    settings->beginGroup(group);
    keysTmp = settings->childKeys();
    if(keysTmp.isEmpty())
    {
        successFlag = FAILURE;
    }
    else
    {
        keys = keysTmp;
        for(int i=0; i<keysTmp.size(); i++)
        {
            strTmp = settings->value(keysTmp[i]).toString();
            valueTmp = strTmp.split(" ",QString::SkipEmptyParts);
            keysValue.append(valueTmp[0]);
            keysValueBuffer.append(valueTmp[0]);
            valueTmp.removeFirst();
            keysValueNote.append(valueTmp.join(" "));
            valueTmp.clear();
        }
        successFlag = SUCCESS;
    }
    settings->endGroup();
    return successFlag;
}

void ReadSetting::setKeys(QSettings *settings, QString group)
{
    settings->beginGroup(group);
    for(int i=0; i<keys.size(); i++)
    {
        keysValue[i] = keysValue[i]+" "+keysValueNote[i];
        settings->setValue(keys[i],keysValue[i]);
    }
    settings->endGroup();
}

void ReadSetting::clear()
{
    keys.clear();
    keysValue.clear();
    keysValueBuffer.clear();
}
