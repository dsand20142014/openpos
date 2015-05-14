#ifndef MODULESETTING_H
#define MODULESETTING_H


#include <QSettings>
#include "button.h"
#include <QtGui>
#include "baseForm.h"

extern "C"{
#include "bar.h"
#include "ostools.h"
#include "sysparam.h"
}


#define MAXCOUNT 50

//add by xiongjian at 20130131
#define DAEMON_MAX_COUNT    10  //daemon max count
#define DRIVER_MAX_COUNT    30  //driver max count
//add end


class ModuleSetting : public BaseForm
{
    Q_OBJECT
    
public:
    explicit ModuleSetting(QWidget *parent = 0);
    ~ModuleSetting();
    
private slots:
    void on_pushButton_2_clicked();
    void slot_barcode();
    void on_scanButton_clicked();
    void setScanBtn(int);

private:
    Button *btnScan;
    QWidget *tab;
    QWidget *tab_2;
    QWidget *tab_3;

    QTabWidget *tabWidget;

    void setValue();
    void getValue();

    QScrollArea *scroll1;
    QWidget *widget1;
    QVBoxLayout *vlayout;

    QLabel* tabOneLabel[MAXCOUNT];
    QLineEdit *tabOneEdit[MAXCOUNT];

    QScrollArea *scroll2;
    QWidget *widget2;
    QGridLayout *gridlayout;

    QLabel* debugNamelabel[MAXCOUNT];
    QCheckBox* box[MAXCOUNT];
    QCheckBox* box2[MAXCOUNT];


    QScrollArea *scroll3;
    QWidget *widget3;
    QGridLayout *gridlayout3;
    QCheckBox* boxko[MAXCOUNT];
    QCheckBox* boxko2[MAXCOUNT];
    QLabel* Namekolabel[MAXCOUNT];

    void keyPressEvent(QKeyEvent *);


    //add by xiongjian at 20130131
    QString machineSerialNum; //POS Serial Number
    QString pinpad_serialNum; //POS Pin Pad Serial Number
    PROCESS_INIT_INFO daemonInitList[DAEMON_MAX_COUNT]; //Daemon Init Information list with PROCESS_INIT_INFO struct for show
    PROCESS_INIT_INFO driverInitList[DRIVER_MAX_COUNT]; //Driver Init information List whit PROCESS_INIT_INFO struct for show
    int daemonCount; //current daemon count
    int driverCount; //current driver count

    //get Init data about POS,the driver data save to driverInitList,
    //the daemon data save to daemonInitList, module data save to machineSerialNum and pinpad_serialNum
    void getInitData(void);

    //set new date to POS
    void setInitData(void);

    //draw GUI base to init data
    void drawModuleGUI(void);

    //update init data, POS Serial Number,Pin Pad Serial Number,Daemon Init Information list and Driver Init information List
    void updateInitData(void);

    //add end

    QTimer barTimer;

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent * event);

      int  lastpointY;
      bool pressflag;

      int moveY;
      int windowHeight ;

      int moveYmax;
};

#endif // MODULESETTING_H
