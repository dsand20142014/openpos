#ifndef ACTIVEWIN_H
#define ACTIVEWIN_H

#include <QtGui>
#include "baseForm.h"


#define NUM 10
class ActiveWin : public BaseForm
{
    Q_OBJECT
    
public:
    explicit ActiveWin(QWidget *parent = 0);
    ~ActiveWin();
    
private:
    int count;
    int clickNum[NUM];
    int oldNum[NUM];
    QLabel *label[NUM];
    QPushButton *btn[NUM];
    QHBoxLayout *hlayout[NUM];
    QSignalMapper* signalMapper;
    QVBoxLayout *vlayout ;
    QScrollArea *myscroll;
    QWidget *widget;

    QStringList titleList;

    void getValue();
    bool getDataInfo(char* group , char* key);
    void setDataInfo(char* group , char* key,int i);

private slots:
    void btnOkClick();
    void setNum(int);
    void showInfo();

};

#endif // ACTIVEWIN_H
