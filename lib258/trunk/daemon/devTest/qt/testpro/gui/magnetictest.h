#ifndef MAGNETICTEST_H
#define MAGNETICTEST_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include "baseForm.h"
#include "gui/mfccardtest.h"

class MagneticTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit MagneticTest(QWidget *parent = 0);
    ~MagneticTest();
    
private slots:
      void getResult();
      void getStatics();
      void on_pushButton_3_clicked();
      void slotExit();
      void slotFresh();

private:
    CardThread *thread;
    QTimer timer;
    QTimer timer2;
    int total;
    int iOkIso1;
    int iOkIso2;
    int iOkIso3;

     unsigned char aucMag1[200];
     unsigned char aucMag2[200];
     unsigned char aucMag3[200];
     unsigned char ucmaglen1;
     unsigned char ucmaglen2;
     unsigned char ucmaglen3;
     unsigned char ucmag1status;
     unsigned char ucmag2status;
     unsigned char ucmag3status;
//    void keyPressEvent(QKeyEvent *);

     Button *myBtnExit;
    Button *pushButton_3;
    QTableWidget *tableWidget;
    QTextEdit *textEdit ;
    QStackedWidget* stackedWidget;
    void keyPressEvent(QKeyEvent *);
};

#endif // MAGNETICTEST_H
