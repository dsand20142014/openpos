#include "navibar.h"
#include <QDebug>

#define ICON_SIZE 5
#define ICON_PAD 5

NaviBar::NaviBar(const int count)
    : QGraphicsRectItem(),
      naviBarCount(count)
{
    setRect(0, 0, 240, ICON_SIZE);
    setPen(Qt::NoPen);

    x = (this->rect().width() - 20*naviBarCount)/2;

    setAllIndexPage();


    m_cursors1 = new QGraphicsRectItem;
    m_cursors1->setParentItem(this);
    m_cursors1->setRect(x, 0, 20, 2);
    m_cursors1->setPen(Qt::NoPen);
    m_cursors1->setBrush(QColor(Qt::white));
    m_cursors1->setOpacity(0.4);
    m_cursors1->setZValue(2);
}

void NaviBar::setAllIndexPage()
{
    for (int i = 0; i < naviBarCount; ++i) {
        QGraphicsRectItem *m_cursors = new QGraphicsRectItem;
        m_cursors->setParentItem(this);
        m_cursors->setRect(x + (i * 21), -10, 20, 2);
        m_cursors->setPen(Qt::NoPen);
        m_cursors->setBrush(QColor(Qt::yellow));
        m_cursors->setOpacity(0.2);
        m_cursors->setZValue(1);
      //  m_cursor.append(m_cursors);
    }
}

void NaviBar::setIndexPage(int index)
{
    m_cursors1->setPos(index * 21, -10);
}

void NaviBar::paint(QPainter * painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(Qt::white);
    painter->setOpacity(0.2);
    painter->drawRect(option->rect.adjusted(-10, ICON_PAD, 10, 0));
}


// NaviBar::~NaviBar()
//{
//   // qDebug()<<"IN ~NaviBar*************";
//    if(m_cursors1!=NULL)
//    {
//        delete m_cursors1;
//        m_cursors1 = NULL;
//    }
//   // qDebug()<<"11111111IN ~NaviBar*************";

//    qDeleteAll(m_icons.begin(),m_icons.end());
//    m_icons.clear();
//   // qDebug()<<"2222222222IN ~NaviBar*************";

//    qDeleteAll(m_cursor.begin(),m_cursor.end());
//    m_cursor.clear();
//   // qDebug()<<"3333333333IN ~NaviBar*************";

//}
