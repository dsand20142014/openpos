#ifndef PARALLAXHOME_H
#define PARALLAXHOME_H

#include <QtCore>
#include <QtGui>
#include <QtSvg/QGraphicsSvgItem>
#include "navibar.h"
#include "node.h"
#include <linux/input.h>

//#define PAGE_COUNT 5

class ParallaxHome: public QGraphicsView
{
    Q_OBJECT

public:
    ParallaxHome(QStringList, QStringList, QWidget *parent = 0);//(QWidget *parent = 0);
    ~ParallaxHome();
    void restartTimer(); //xl add for sleep
    void stopTimer();

public slots:
    void slideBy(qreal dx);
    void choosePage(int page);

private slots:
    void shiftPage(int frame);
    void slot_sleep(); //xl add for sleep

protected:
    virtual void resizeEvent(QResizeEvent *event) ;
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

private:
    void layoutScene();
    void setupScene();
    void testByItemName(const QString& itemName);
    void setSleep();

private:
    enum falg_event
    {
        Flag_Start,
        Flag_Slide
    };

    int pageCount;
    int pageIndex;
    QGraphicsScene m_scene;
    NaviBar *m_naviBar;

    QTimeLine m_pageAnimator;
    qreal m_pageOffset;
    Node* pItem;//¼ÇÂ¼node item

    QList<Node*> m_items;
    QList<QPointF> m_positions;

    QPoint oldPoint;
    QPoint curPoint;
    bool pressFlag;
    int dragFlag;
    int isHasItem;

    QStringList testNames;
    QStringList testIcons;
    int oldpage;
    QTimer sleepTimer;//xl add for sleep
    int sleepTime;
};

#endif // PARALLAXHOME_H
