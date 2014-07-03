#ifndef MYDOT_H
#define MYDOT_H

#include <QPainter>
#include <QGraphicsItem>


#include "myline.h"
class MyLine;


class MyDot : public QGraphicsItem
{
public:
    MyDot(QGraphicsTextItem* coords, MyLine *line);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void set_position(const QPointF &newpos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    MyLine* line;
    QGraphicsTextItem* coords;


    bool pressed;
    bool update_lock;   //true when the dot is being moved as result of external input; to avoid update loops

};

#endif // MYDOT_H
