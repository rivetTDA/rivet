#ifndef MYDOT_H
#define MYDOT_H

#include <QPainter>
#include <QGraphicsItem>


class MyDot : public QGraphicsItem
{
public:
    MyDot(QGraphicsTextItem* coords);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);


    bool pressed;
    QGraphicsTextItem* coords;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

};

#endif // MYDOT_H
