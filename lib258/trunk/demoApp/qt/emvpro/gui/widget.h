#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtGui>
//#include "iccwidonw.h"


#include "TypeDef.h"
#include "EMVDef.h"
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    
private slots:
    unsigned char getResult(void);

    void on_pushButton_clicked();

private:
    Ui::Widget *ui;
    QTimer timer;
    BOOL	bFallBack;
    TRANSTYPE	enTransType;
    void ProcessMag(BOOL bFallBack,unsigned char* bufMag);
    void keyPressEvent(QKeyEvent *);

};

#endif // WIDGET_H
