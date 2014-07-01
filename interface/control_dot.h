#ifndef CONTROL_DOT_H
#define CONTROL_DOT_H


#include <QPainter>
#include <QGraphicsItem>

#include "slice_diagram.h"
class SliceDiagram;

class ControlDot : public QGraphicsItem
{
public:
    ControlDot(SliceDiagram* sd, double xmin, double xmax, double ymin, double ymax); //constructs a control dot that defaults to left-bottom

    void set_right_top();   //sets this to be a right-top control dot

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    SliceDiagram* diagram;

    double xmin, xmax, ymin, ymax;  //data extents
    bool pressed;   //for color change when pressed
    bool left_bottom;   //TRUE if this is a left-bottom control dot; FALSE if this is a right-top control dot

};

#endif // CONTROL_DOT_H
