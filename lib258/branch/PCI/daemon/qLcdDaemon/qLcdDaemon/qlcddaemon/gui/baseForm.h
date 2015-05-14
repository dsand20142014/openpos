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

    void keyPressEvent(QKeyEvent *);
    //void mousePressEvent(QMouseEvent *);
};

#endif // BaseForm_H
