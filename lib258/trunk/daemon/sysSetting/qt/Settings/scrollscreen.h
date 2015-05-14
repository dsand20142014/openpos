#ifndef SCROLLSCREEN_H
#define SCROLLSCREEN_H

#include <QScrollArea>
#include <QMouseEvent>
#include <QTime>
#include <QTimer>

class ScrollScreen : public QScrollArea
{
    Q_OBJECT
public:
    explicit ScrollScreen(QWidget *parent = 0);
    
signals:

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
public slots:
    void loopDisplayDeal(void);
private:
    enum {MINISPEED = 0,MAXISPEED = 1000};
    bool directionFlag;
    QTime *stopWatch;
    QTimer *loopDisplay;
    int speed;
    int yPositionPrevious,yPositionNow,yPositionRelease,yPositionPress;
};

#endif // SCROLLSCREEN_H
