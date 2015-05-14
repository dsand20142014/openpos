#ifndef GPSTEST_H
#define GPSTEST_H

#include <QtGui>
#include "button.h"

#define MAXSATNUM 40
namespace Ui {
class GPSTest;
}

class GPSTest : public QWidget
{
    Q_OBJECT
    
public:
    explicit GPSTest(QWidget *parent = 0);
    ~GPSTest();
    
private slots:
    void showDetail();
    void refresh_data();
    void slot_setbtn();
    void slot_detailclose();
private:
    Ui::GPSTest *ui;
    void getData();
    QTimer timer;
    QTimer timer2;
    int count;
    Button *exitBtn;
    Button *btnDetail;
    Button *setbtn;

    QProgressBar *barList[MAXSATNUM];
    QLabel *nameList[MAXSATNUM];
    int lastCount;

    void keyPressEvent(QKeyEvent *);
};

#endif // GPSTEST_H
