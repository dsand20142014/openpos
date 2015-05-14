#include "scrollscreen.h"
#include <QScrollBar>
#include <QDebug>
#include <qmath.h>

ScrollScreen::ScrollScreen(QWidget *parent) :
    QScrollArea(parent)
{
    stopWatch = new QTime;
    loopDisplay = new QTimer(this);
    connect(loopDisplay,SIGNAL(timeout()),this,SLOT(loopDisplayDeal()));
}

void ScrollScreen::mousePressEvent(QMouseEvent *event)
{
    yPositionPress = event->y();
    //qDebug("press%d",yPositionPress);
    yPositionPrevious = event->y();
    yPositionNow = event->y();
    stopWatch->start();
//    qDebug("%d",verticalScrollBar()->maximum());
//    qDebug("%d",verticalScrollBar()->minimum());
}

void ScrollScreen::mouseMoveEvent(QMouseEvent *event)
{
    int moveDistance,scrollBarPosition;
    yPositionNow = event->y();
    moveDistance = yPositionNow - yPositionPrevious;
    scrollBarPosition = verticalScrollBar()->value() + moveDistance;
    verticalScrollBar()->setValue(scrollBarPosition);
    yPositionPrevious = yPositionNow;
}

void ScrollScreen::mouseReleaseEvent(QMouseEvent *event)
{
    int releaseDistance,releaseTime;
    yPositionRelease = event->y();
    //qDebug("release%d",yPositionRelease);
//    qDebug("%d",verticalScrollBar()->height());
//    qDebug("%d",verticalScrollBar()->pageStep());
//    qDebug("%d",verticalScrollBar()->maximum());
//    qDebug("%d",verticalScrollBar()->minimum());
//    qDebug("%d",verticalScrollBar()->value());
//    qDebug("%d",yPositionPress);
//    qDebug("%d",yPositionRelease);
    if(yPositionRelease - yPositionPress > 0)
    {
        directionFlag = true;
    }
    else
    {
        directionFlag = false;
    }
    releaseDistance = qAbs(yPositionRelease - yPositionPress);
    releaseTime = stopWatch->elapsed();
    speed = releaseDistance / releaseTime * 1000;
    if(speed > MAXISPEED)
    {
        speed = MAXISPEED;
    }
    if(speed < MINISPEED)
    {
        speed = MINISPEED;
    }
    loopDisplay->start(10);
}

void ScrollScreen::loopDisplayDeal(void)
{
    int dy,scrollBarPosition,maxMinDistance;
    if(--speed < 0)
    {
        speed = 0;
    }
    qDebug("%d",speed);
    maxMinDistance = verticalScrollBar()->maximum() - verticalScrollBar()->minimum();
    dy = speed * maxMinDistance / MAXISPEED;
    if(dy > 0)
    {
        if(directionFlag)
        {
            scrollBarPosition = verticalScrollBar()->value() + dy;
            if(scrollBarPosition > verticalScrollBar()->maximum())
            {
                scrollBarPosition = verticalScrollBar()->maximum();
                loopDisplay->stop();
            }
        }
        else
        {
            scrollBarPosition = verticalScrollBar()->value() - dy;
            if(scrollBarPosition < verticalScrollBar()->minimum())
            {
                scrollBarPosition = verticalScrollBar()->minimum();
                loopDisplay->stop();
            }
        }

        verticalScrollBar()->setValue(scrollBarPosition);
    }
    else
    {
        loopDisplay->stop();
    }
}
