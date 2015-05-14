#ifndef SECONDMENU_H
#define SECONDMENU_H

#include <QtGui>
#include <QModelIndex>
#include <QKeyEvent>
#include "baseForm.h"

class SecondMenu : public BaseForm
{
    Q_OBJECT
    
public:
    explicit SecondMenu(QStringList curitems,QWidget *parent = 0);
    ~SecondMenu();

    void setCurWidget(QString ,int );
    
private slots:
    void slot1(QModelIndex);

private:
    QListWidget *listWidget;
    QStringList items;
    int curPage;


protected:
    void keyPressEvent(QKeyEvent *);

};

#endif // SECONDMENU_H
