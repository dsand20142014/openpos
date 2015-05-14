#ifndef SECONDMENU_H
#define SECONDMENU_H

#include <QtGui>
#include <QModelIndex>
#include <QKeyEvent>
namespace Ui {
class SecondMenu;
}

class SecondMenu : public QWidget
{
    Q_OBJECT
    
public:
    explicit SecondMenu(QString title,int curnum,QStringList items,QWidget *parent = 0);
    ~SecondMenu();


    
private slots:
    void listSlot(QModelIndex);
    void slot1(QModelIndex);
    void slot2(QModelIndex);
    void slot3(QModelIndex);
    void slot4(QModelIndex);
    void slot5(QModelIndex);
    void slot6(QModelIndex);



private:
    Ui::SecondMenu *ui;
    int screenWidth;
    int screenHeight;
    int curNum;
    void keyPressEvent(QKeyEvent *);

};

#endif // SECONDMENU_H
