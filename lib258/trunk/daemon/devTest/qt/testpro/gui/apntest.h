#ifndef APNTEST_H
#define APNTEST_H

#include <QWidget>
#include "baseForm.h"


class ApnTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit ApnTest(QWidget *parent = 0);
    ~ApnTest();
    
private:
    QTextEdit *textEdit;
    bool flag;
    char apn[50];
    char band[50];

    void getapnband();
//    void keyPressEvent(QKeyEvent *);

private slots:
    void getResult();
    void apnConnect();
    int net_interface(struct _if *if_info);
};

#endif // APNTEST_H
