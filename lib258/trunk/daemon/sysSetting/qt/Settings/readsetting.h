#ifndef READSETTING_H
#define READSETTING_H

#include <QString>
#include <QSettings>
#include <QStringList>



class ReadSetting
{
public:
    QStringList keys, keysValue,keysValueNote;
    QStringList keysValueBuffer;
    enum {SUCCESS,FAILURE};

    ReadSetting();
    void clear();
    unsigned char readKeys(QSettings *settings, QString group);
    void setKeys(QSettings *settings, QString group);
};

extern ReadSetting readSetting;
#endif // READSETTING_H
