#ifndef GSMSETTING_H
#define GSMSETTING_H

#include <QtGui>
#include "baseForm.h"

#define MAXCOUNT 10

class GsmSetting : public BaseForm
{
    Q_OBJECT
    
public:
    explicit GsmSetting(QWidget *parent = 0);
    ~GsmSetting();
    
private slots:
    void on_pushButton_2_clicked();
    void setValue();

private:
    void getValue();

    QScrollArea *myscroll;
    QWidget *widget;
    QVBoxLayout *vlayout;


    QLabel* tabOneLabel[MAXCOUNT];
    QLineEdit *tabOneEdit[MAXCOUNT];
    QComboBox* tabOneComB[MAXCOUNT];

    bool eventFilter(QObject *, QEvent *);


protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event );

      int lastpointY;
      bool pressflag;

      int moveY;
      int windowHeight ;

      int moveYmax;
};

#endif // GSMSETTING_H
