#ifndef GPSWIN_H
#define GPSWIN_H

#include "baseForm.h"
#include <QtGui>


class GpsWin : public BaseForm
{
    Q_OBJECT
    
public:
    explicit GpsWin(QWidget *parent = 0);
    ~GpsWin();
    
private:
    QTimer timer;
    QFile *file;
    QTextEdit *textEdit;

private slots:
    void slot_timer();
};

#endif // GPSWIN_H
