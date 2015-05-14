#ifndef SCREENSETTING_H
#define SCREENSETTING_H

#include <QWidget>
#include <QtGui>
#include "baseForm.h"


#define MAXCOUNT 10

class ScreenSetting : public BaseForm
{
    Q_OBJECT
    
public:
    explicit ScreenSetting(QWidget *parent = 0);
    ~ScreenSetting();
    
private slots:
    void setBrightness(int);
    void on_pushButton_2_clicked();
    void resetBrightness();
    void findNextFocus();

private:
    void setValue();
    void getValue();


    QScrollArea *scroll;
    QWidget *widget;
    QVBoxLayout *vlayout;
    QSlider* slider;
    int brightness;

    QLabel* tabOneLabel[MAXCOUNT];
    QLineEdit *tabOneEdit[MAXCOUNT];

    void keyPressEvent(QKeyEvent *);
};

#endif // SCREENSETTING_H
