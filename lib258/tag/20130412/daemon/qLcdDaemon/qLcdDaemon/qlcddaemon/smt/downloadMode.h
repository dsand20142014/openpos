#ifndef DownloadMode_H
#define DownloadMode_H

#include <QtGui>
#include <QSettings>
#include "../gui/baseForm.h"
#include "downloading.h"
#include "download.h"


class DownloadMode : public BaseForm
{
    Q_OBJECT
    
public:
    explicit DownloadMode(QWidget *parent = 0);
    ~DownloadMode();

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void btnOk_clicked();

private:
    Download *dl;
    Downloading *dlcom;
    QRadioButton *rcom;
    QRadioButton *rusb;
    QRadioButton *rwifi;
    QRadioButton *rnet;
    QRadioButton *rtf;
    QRadioButton *rotg;
    QStringList keyList;
    int checked;
    int getMode();
    int setMode();
    void setValue(QSettings *set, QString key, QString value);
    QStringList syslist;
};

#endif // DownloadMode_H
