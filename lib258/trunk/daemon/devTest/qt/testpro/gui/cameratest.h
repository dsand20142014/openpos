#ifndef CAMERATEST_H
#define CAMERATEST_H

#include <QWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QtGui>
#include "button.h"
namespace Ui {
class CameraTest;
}

class CameraTest : public QWidget
{
    Q_OBJECT
    
public:
    explicit CameraTest(QWidget *parent = 0);
    ~CameraTest();
    
private slots:
    void slot_quit(void);		//added by lichao
    void slot_rephoto(void);	//added by lichao
    void slot_changeFlagFalse();


private:
    Ui::CameraTest *ui;
    void keyPressEvent(QKeyEvent *);
    void ini_data();
    Button *exitBtn;			//added by lichao
    Button *rephotoBtn;			//added by lichao
    char *buf;
    int bufSize;
    int imgWidth ;
    int imgHeight ;
    bool flag;
    bool onceFLag;
    QLabel *informationBox;
};

#endif // CAMERATEST_H
