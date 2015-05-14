#include "navibar.h"
#include <QDebug>

#define ICON_SIZE 5
#define ICON_PAD 5

NaviBar::NaviBar()
    : QGraphicsRectItem()
{
    //setRect(0,0,0,0);
    setRect(0, 0, 316, ICON_SIZE);
    setPen(Qt::NoPen);

    setAllIndexPage();

    m_cursors1 = new QGraphicsRectItem;
    m_cursors1->setParentItem(this);
    m_cursors1->setRect(60, 0, 20, 2);
    m_cursors1->setPen(Qt::NoPen);
    m_cursors1->setBrush(QColor(Qt::white));
    //modify to not display the m_cursors1
    m_cursors1->setOpacity(0.0);//0.4
    m_cursors1->setZValue(2);
}

void NaviBar::setAllIndexPage()
{
    //modify undermentioned to limit the count of navibar
    for (int i = 0; i < 5; ++i) {
        QGraphicsRectItem *m_cursors = new QGraphicsRectItem;
        m_cursors->setParentItem(this);
        m_cursors->setRect(105 + (i * 21), -10, 20, 2);
        m_cursors->setPen(Qt::NoPen);
        m_cursors->setBrush(QColor(Qt::yellow));
        //modify opacity to display the navibar
        //modify to not display the m_cursors(include m_cursors1)
        m_cursors->setOpacity(0.0);//0.2
        m_cursors->setZValue(1);
        m_cursor.append(m_cursors);
    }
}

void NaviBar::setIndexPage(int index)
{
    m_cursors1->setPos(45 + (index * 21), -10);
}

void NaviBar::paint(QPainter * painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(Qt::white);
    painter->setOpacity(0.2);
    painter->drawRect(option->rect.adjusted(-10, ICON_PAD, 10, 0));
}
