#ifndef DATETIMESETTING_H
#define DATETIMESETTING_H

#include <QWidget>
#include <QtGui>
#include "baseForm.h"

class DateTimeSetting : public BaseForm//QWidget
{
    Q_OBJECT
    
public:
    explicit DateTimeSetting(QWidget *parent = 0);
    ~DateTimeSetting();
    
private slots:
    void on_btnSave_clicked();


private:
    QDateEdit *lineEdit;
    QTimeEdit *lineEdit_2;
    void setValue();
    void getValue();
    bool eventFilter(QObject *, QEvent *);

};

#endif // DATETIMESETTING_H
