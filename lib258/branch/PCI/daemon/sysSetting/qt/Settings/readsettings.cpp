#include "readsettings.h"
#include <QDebug>

ReadSettings::ReadSettings()
{

}

void ReadSettings::readSetting(QSettings *settings, QTextBrowser *tbNote, QLineEdit *leValue, QLabel *lModle, QString &gStrKeys, QString &gStrValue, QString group,QString key)
{
    QStringList groups = settings->childGroups();
    //settings->beginGroup(group);
    QStringList keys,value,valueTmp;
    QString strGroup,strKeys,strValue,strTmp;
//    if(groups.isEmpty())
//    {
//        tbNote->append("NoGroups");
//    }
//    else
//    {
//        tbNote->append("HasGroups");
//    }
    for(int i=0;i<groups.size();i++)
    {
        if(groups.isEmpty())
        {
            tbNote->append("NoGroup");
        }
        else
        {
            strGroup=groups.at(i);
            //qDebug()<<strGroup;
            if(strGroup==group)
            {
                //qDebug()<<strGroup;
                settings->beginGroup(group);
                keys = settings->childKeys();
                if(keys.isEmpty())
                {
                    leValue->setText("NoValue");
                    tbNote->append("NoNote");
                }
                for(int j=0;j<keys.size();j++)
                {
                    strKeys = keys.at(j);
                    if(strKeys ==key)
                    {
                        gStrKeys = strKeys;
                        lModle->setText(strKeys);
                        //qDebug()<<strKeys;
                        strValue = settings->value(strKeys).toString();
                        value = strValue.split(" ",QString::SkipEmptyParts);
                        valueTmp = value;
                        valueTmp.removeFirst();
                        gStrValue = valueTmp.join(" ");
                        for(int k=1;k<value.size();k++)
                        {
                            if(k==1)
                            {
                                strTmp = value[1];
                                tbNote->append(strTmp.right(strTmp.length()-1));
                                strTmp.clear();
                            }
                            else
                            {
                                tbNote->append(value[k]);
                            }
                        }
                        leValue->setText(value[0]);
                        strValue.clear();
                    }
                }
                settings->endGroup();
            }
        }
    }
    //settings->endGroup();
}
