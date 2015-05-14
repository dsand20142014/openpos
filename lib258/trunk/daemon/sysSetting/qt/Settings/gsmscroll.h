#ifndef GSMSCROLL_H
#define GSMSCROLL_H

#include <QGraphicsView>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QGraphicsProxyWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPoint>
#include <QTime>
#include <QSettings>
#include <QStringList>
#include <QString>
#include "readsetting.h"

class GsmScroll : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GsmScroll(QWidget *parent = 0);
    
signals:
    
public slots:

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void saveParameters();
    void pwdDeal(QString);
    void radioButton1Deal(bool);
    void radioButton2Deal(bool);
    void radioButton3Deal(bool);
    void radioButton4Deal(bool);
    void radioButton5Deal(bool);
    void radioButton6Deal(bool);
    void radioButton7Deal(bool);
    void lineEditDeal(QString);

private:
    QLabel *lName,*lPwd,*lOther;
    QLineEdit *lePwd,*leName,*leOther;
    QGroupBox *gbWm,*gbMaol,*gbDb,*gbApn;
    QPushButton *pbCancel,*pbEnter;
    QRadioButton *rbGprs,*rbWcdma,*rbYes,*rbNo,
                *rb850,*rb900,*rb1800,*rb1900,*rb8519,*rb918,*rb919,
                *rbCmnet,*rbWxbc,*rbVodafone,*rbMposcu,*rbGzylcmda,
                *rbGzdxcmda,*rbChinaUnicom,*rbHide;

    QPropertyAnimation *animationDown;
    QPropertyAnimation *animationUp;
    int animationDuration;
    QPoint scrollStartPos;
    QPoint scrollCurrentPos;
    QPoint scrollEndPos;
    int dist;

    QGraphicsWidget *centerWidget;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QHash<QString, QGraphicsProxyWidget*> proxyForName;

    QTime time;
    unsigned char directionFlag;
    bool animationFlag;
    enum derection{INITIAL,UP,DOWN};
    bool cursorPressFlag;
    int miniWidgetPos;
    int maxiWidgetPos;
    int yPositionPrevious,yPositionNow,yPositionRelease,yPositionPress,yPositionCurrent,
            yWidgetPrivious,yWidgetCurrent,yWidgetRelease;

    QString strPwd;
    QSettings *settings;

    void displayValue();
    void createWidgets();
    void createProxyWidgets();
    void createLayout();
    void createCentralWidget();
    void createConnections();
};

#endif // GSMSCROLL_H
