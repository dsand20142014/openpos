#ifndef NODE_H
#define NODE_H

#include <QApplication>
#include <QColor>
#include <QGraphicsItem>
#include <QSet>

class Node : public QGraphicsItem, QObject
{
public:
    Node();
    ~Node();

    void setText(const QString &text);
    QString text() const;

    void setTextColor(const QColor &color);
    QColor textColor() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    void setItemPixmap(QPixmap &pixmap);
    QPixmap getItemPixmap()const;

    void setItemPoint(QPointF point);
    QPointF getItemPoint();

    void clearItem();

    bool isNull();

    QRectF boundingRect() const;

protected:
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QRectF outlineRect() const;
    int roundness(double size) const;

private:
    QString myText;
    QPixmap myItemPixmap;
    QColor myTextColor;
    QColor myBackgroundColor;
    QColor myOutlineColor;
    QPointF myItemPoint;
};

#endif

