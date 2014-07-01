#ifndef MYLINE_H
#define MYLINE_H

#include <QPainter>
#include <QGraphicsItem>


class MyLine : public QGraphicsItem
{
public:
    MyLine(double xmin, double xmax, double ymin, double ymax);

    QRectF boundingRect() const;
    QPainterPath shape() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);


private:
    bool pressed;   //true when the line is clicked
    bool rotating;  //true when a rotation is in progress
    double xmin, xmax, ymin, ymax;
    double slope;
    QPoint left_point;
    QPoint right_point;

};

#endif // MYLINE_H
