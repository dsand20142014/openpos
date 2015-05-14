#include <QtGui>
#include "node.h"

#define WW 73
#define HH 83
#define HTOP 0.037
#define HROBOT  0.30
//HTOP：最上面空隙 3.7% 图片62.5% 文字25%
#define MAPWW  50
#define MAPHH  50

Node::Node()
{
    //init
    myText.clear();
    myTextColor = Qt::black;
    myItemPoint = QPointF(0,0);
    myItemPixmap.fill(Qt::transparent);
    myBackgroundColor = Qt::transparent;

    setFlags(ItemIsMovable | ItemIsSelectable);
}

Node::~Node()
{
}

void Node::setText(const QString &text)
{
    myText = text;
    update();
}

QString Node::text() const
{
    return myText;
}

void Node::setTextColor(const QColor &color)
{
    myTextColor = color;
    update();
}

QColor Node::textColor() const
{
    return myTextColor;
}

void Node::setItemPixmap(QPixmap &pixmap)
{
    myItemPixmap = pixmap;
    myItemPixmap = myItemPixmap.scaled(MAPWW, MAPHH);
    update();
}

QPixmap Node::getItemPixmap()const
{
    return myItemPixmap;
}

void Node::setItemPoint(QPointF point)
{
    myItemPoint = point;
}

QPointF Node::getItemPoint()
{
    return myItemPoint;
}

void Node::clearItem()
{
    myText.clear();
    myItemPixmap.fill(Qt::transparent);
}

bool Node::isNull()
{
    return myText.isEmpty();
}

void Node::setBackgroundColor(const QColor &color)
{
    myBackgroundColor = color;
    update();
}

QColor Node::backgroundColor() const
{
    return myBackgroundColor;
}

QRectF Node::boundingRect() const
{
    const int Margin = 1;
    return outlineRect().adjusted(-Margin, -Margin, +Margin, +Margin);
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                 QWidget *parent)
{
    Q_UNUSED(option);
    Q_UNUSED(parent);

    QPen pen;
    pen.setStyle(Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(myBackgroundColor);

    QRectF rect = outlineRect();
    painter->drawRoundRect(rect, roundness(rect.width()),
                           roundness(rect.height()));

    painter->drawPixmap(-MAPWW/2.0, 40-HH*(1.0-0.038), myItemPixmap);

    painter->setPen(myTextColor);
    QRectF rect2 = QRectF(-WW/2.0, (40-HH*0.3)-2, WW, 30);
    painter->drawText(rect2, Qt::AlignHCenter|Qt::AlignTop, myText);

}

QRectF Node::outlineRect() const
{
    QRect rect = QRect(0, 0, WW, HH);
    rect.translate(-rect.center());
    return rect;
}

int Node::roundness(double size) const
{
    const int Diameter = 12;
    return 100 * Diameter / int(size);
}
