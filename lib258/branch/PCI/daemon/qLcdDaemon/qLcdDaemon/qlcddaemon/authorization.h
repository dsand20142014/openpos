#ifndef AuthorizationForm_H
#define AuthorizationForm_H

#include <QtGui>
#include "lcddaemonmsgqueue.h"

class AuthorizationForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit AuthorizationForm(QWidget *parent = 0);
    ~AuthorizationForm();

protected:
    void keyPressEvent(QKeyEvent *);

private slots:
    void slot_btnOk_click();
    void slot_btnExit_click();

private:
    QLabel *label;
    QLineEdit *input;
    QRadioButton *r1;
    QRadioButton *r2;
    QRadioButton *r3;
    QRadioButton *r4;
    struct lcd_message message;
    void sendmsg2linuxpos();
    //int authorize();
};

#endif // AuthorizationForm_H
