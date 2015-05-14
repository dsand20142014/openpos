#ifndef NAVIBAR_H
#define NAVIBAR_H

#include <QtCore>
#include <QtGui>
#include <QtSvg/QGraphicsSvgItem>

//#define PAGE_COUNT 5

class NaviBar : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    NaviBar(const int count);
   // ~NaviBar();

    void setIndexPage(int index);

signals:
    void pageSelected(int page);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

private:
    void setAllIndexPage();

private:
    QList<QGraphicsSvgItem*> m_icons;
    QList<QGraphicsRectItem *> m_cursor;
    QGraphicsRectItem *m_cursors1;
    int indexIcon;
    int naviBarCount;
    int x;
};

#endif // NAVIBAR_H
