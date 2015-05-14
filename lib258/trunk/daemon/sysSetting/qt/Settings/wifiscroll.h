#ifndef WIFISCROLL_H
#define WIFISCROLL_H

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

class WifiScroll : public QGraphicsView
{
    Q_OBJECT
public:
    explicit WifiScroll(QWidget *parent = 0);
    
signals:

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

public slots:

private:
    QLabel *lAn,*lPwd,*lIp,*lMask,*lRouter;
    QLineEdit *leAn,*lePwd,*leIp,*leMask,*leRouter;
    QGroupBox *gbActive,*gbSecurity,*gbDhcp;
    QPushButton *pbCancel,*pbEnter;
    QRadioButton *rbOyes,*rbOno,*rbSwep,*rbSwpa,*rbSwpa2,*rbSwpaa,*rbDyes,*rbDno;

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

    QString strPwd;
    QSettings *settings;

    QTime time;
    unsigned char directionFlag;
    bool animationFlag;
    enum derection{INITIAL,UP,DOWN};
    bool cursorPressFlag;
    int miniWidgetPos;
    int maxiWidgetPos;
    int yPositionPrevious,yPositionNow,yPositionRelease,yPositionPress,yPositionCurrent,
            yWidgetPrivious,yWidgetCurrent,yWidgetRelease;

    void displayValue();
    void createWidgets();
    void createProxyWidgets();
    void createLayout();
    void createCentralWidget();
    void createConnections();
    
};

#endif // WIFISCROLL_H
