#ifndef BEEPTEST_H
#define BEEPTEST_H

//#include <QKeyEvent>
#include <QtGui>

#include "baseForm.h"

class BeepTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit BeepTest(int flag,QWidget *parent = 0);
    ~BeepTest();

private slots:
    void on_pushButton_2_clicked();
    void led_on();
    void setBrightness(int value);
    void refreshBrightness();

private:
    Button *pushButton_2;
    int flags;
    QTimer timer;
    int count;
    QSlider *slider;
    int is;
    int brightness;
    QLabel *picBeep;

protected:
    void keyPressEvent(QKeyEvent *);
};

#endif // BEEPTEST_H
