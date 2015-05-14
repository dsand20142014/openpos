#ifndef SECONDMENU_H
#define SECONDMENU_H

#include "baseForm.h"
//#include <QModelIndex>
#include <QKeyEvent>
#include <QtGui>
#include "../smt/downloading.h"

class SecondMenu : public BaseForm
{
    Q_OBJECT
    
public:
    explicit SecondMenu(const QString & text = NULL, QWidget *parent = 0);
    ~SecondMenu();

    
private slots:
    void pushButton_clicked();
    void slot1(QModelIndex);

private:
    QListWidget *listWidget;
    void setMenu();
    void keyPressEvent(QKeyEvent *);

};

#endif // SECONDMENU_H
