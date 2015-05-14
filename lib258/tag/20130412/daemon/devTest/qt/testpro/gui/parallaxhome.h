#ifndef PARALLAXHOME_H
#define PARALLAXHOME_H

#include <QtGui>
#include <QtSvg/QGraphicsSvgItem>
#include "navibar.h"
#include "node.h"
#include "secondmenu.h"


//#define PAGE_COUNT 5

class ParallaxHome: public QGraphicsView
{
    Q_OBJECT

public:
    ParallaxHome(int argc, char *argv[],QWidget *parent = 0);//(QWidget *parent = 0);
    ~ParallaxHome();


public slots:
    void slideBy(qreal dx);
    void choosePage(int page);

private slots:
    void shiftPage(int frame);
    void sysEventSlot(int);
    void setEnterFlag();
    void gsmSlot();

protected:
    virtual void resizeEvent(QResizeEvent *event) ;
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

private:
    void layoutScene();
    void setupScene();
    void testByItemName(const QString& itemName);


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

    bool enterFLag;
    bool gsmFlag;
    bool fristFLag;
    QTimer gsmTimer;
    bool gsmIsOK;
    SecondMenu *sM_gsm ;
};

#endif // PARALLAXHOME_H
