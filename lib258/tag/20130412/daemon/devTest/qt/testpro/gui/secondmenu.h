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
    explicit SecondMenu(QWidget *parent = 0);
    ~SecondMenu();

    void setCurWidget(QString , int );

    
private slots:
    void slot1(QModelIndex);
    void slot2(QListWidgetItem*);
    void slot3(QModelIndex);
    void slot4(QModelIndex);
    void slot5(QModelIndex);
    void slot6(QModelIndex);


private:
    Ui::SecondMenu *ui;
    void keyPressEvent(QKeyEvent *);

};

#endif // SECONDMENU_H
