#ifndef SLICE_LINE_H
#define SLICE_LINE_H


#include <QPainter>
#include <QGraphicsItem>

#include "control_dot.h"
class ControlDot;
class VisualizationWindow;

class SliceLine : public QGraphicsItem
{
public:
    SliceLine(int width, int height, VisualizationWindow* vw);

    void setDots(ControlDot* left, ControlDot* right);

    QRectF boundingRect() const;
    QPainterPath shape() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void update_lb_endpoint(QPointF &delta); //updates left-bottom endpoint
    void update_rt_endpoint(QPointF &delta); //updates right-top endpoint

    double get_right_pt_x();    //gets x-coordinate of right-top endpoint
    double get_right_pt_y();    //gets y-coordinate of right-top endpoint

    double xmax, ymax;  //these determine the on-screen box in which this line is allowed to move
    //xmin = 0, ymin = 0

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);


private:
    bool vertical;  //true if the line is currently vertical
    bool pressed;   //true when the line is clicked
    bool rotating;  //true when a rotation is in progress
    bool update_lock;   //true when the line is being moved as result of external input; to avoid update loops

    double slope;

    QPointF left_point;  //this is always (0,0)
    QPointF right_point; //this is the top/right endpoint of the line

    ControlDot* left_dot;
    ControlDot* right_dot;

    VisualizationWindow* window;

    void update_window();   //sends angle and offset parameters to the VisualizationWindow
};


#endif // SLICE_LINE_H
