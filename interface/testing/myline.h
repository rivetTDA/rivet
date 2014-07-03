#ifndef MYLINE_H
#define MYLINE_H

#include <QPainter>
#include <QGraphicsItem>

#include "mydot.h"
class MyDot;

class MyLine : public QGraphicsItem
{
public:
    MyLine(double xmin, double xmax, double ymin, double ymax);

    void setDots(MyDot* left);

    QRectF boundingRect() const;
    QPainterPath shape() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void update_lb_endpoint(QPointF &delta); //updates left-bottom endpoint

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);


private:
    bool pressed;   //true when the line is clicked
    bool rotating;  //true when a rotation is in progress
    bool update_lock;   //true when the line is being moved as result of external input; to avoid update loops

    double xmin, xmax, ymin, ymax;
    double slope;

    QPoint left_point;  //in local coordinates, this is always the origin, RIGHT???
    QPointF right_point;

    MyDot* left_dot;
};

#endif // MYLINE_H
