#ifndef MACWIN_H
#define MACWIN_H

#include <baseForm.h>
#include <QTextEdit>
//namespace Ui {
//class MacWin;
//}

class MacWin : public BaseForm
{
    Q_OBJECT
    
public:
    explicit MacWin(QWidget *parent = 0);
    ~MacWin();
    
private:
    QLabel *label1;
    QLabel *label2;

private slots:
    void slotOK();

};

#endif // MACWIN_H
