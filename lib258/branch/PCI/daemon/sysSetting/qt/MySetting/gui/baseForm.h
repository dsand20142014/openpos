#ifndef BaseForm_H
#define BaseForm_H

#include <QtGui>
#include "button.h"

class BaseForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit BaseForm(const QString & text = NULL, QWidget *parent = 0);
    ~BaseForm();
    
protected:
    QVBoxLayout *layout;
    QHBoxLayout *btnlayout;
    Button *btnExit;
    Button *btnOk;
    QLabel *title;
    QLabel *informationBox;

protected:
    void keyPressEvent(QKeyEvent *);
    void showInforText(QString text,bool moveFLag = false,int w =0,int h =0);
};

#endif // BaseForm_H
