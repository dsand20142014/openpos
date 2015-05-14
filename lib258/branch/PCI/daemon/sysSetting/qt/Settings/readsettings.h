#ifndef READSETTINGS_H
#define READSETTINGS_H

#include <QString>
#include <QLineEdit>
#include <QSettings>
#include <QTextBrowser>
#include <QLabel>

class ReadSettings
{
public:
    ReadSettings();
    void readSetting(QSettings *settings, QTextBrowser *tbNote, QLineEdit *leValue, QLabel *lModle, QString &gStrKeys, QString &gStrValue, QString group,QString key);
};

#endif // READSETTINGS_H
