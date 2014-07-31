#ifndef SLICE_LINE_H
#define SLICE_LINE_H


#include <QPainter>
#include <QGraphicsItem>

#include "control_dot.h"
class ControlDot;
class SliceDiagram;
//class VisualizationWindow;

class SliceLine : public QGraphicsItem
{
public:
    SliceLine(int width, int height, SliceDiagram* sd); //VisualizationWindow* vw);

    void setDots(ControlDot* left, ControlDot* right);

    QRectF boundingRect() const;
    QPainterPath shape() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void update_lb_endpoint(QPointF &delta); //updates left-bottom endpoint; called by ControlDot on move event
    void update_rt_endpoint(QPointF &delta); //updates right-top endpoint; called by ControlDot on move event

    double get_right_pt_x();    //gets x-coordinate of right-top endpoint (units: pixels)
    double get_right_pt_y();    //gets y-coordinate of right-top endpoint (units: pixels)

    double get_slope();         //gets the slope of the line
    bool is_vertical();         //true if the line is vertical, false otherwise

    void update_position(double xpos, double ypos, bool vert, double m);  //updates position of line; called by SliceDiagram in response to change in VisualizationWindow controls

    double xmax, ymax;  //pixel dimensions of the on-screen box in which this line is allowed to move
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

    double slope;   //current slope of the line

    QPointF left_point;  //this is always (0,0)
    QPointF right_point; //this is the top/right endpoint of the line

    ControlDot* left_dot;
    ControlDot* right_dot;

    SliceDiagram* sdgm;
//    VisualizationWindow* window;

//    void update_control_boxes();   //sends angle and offset parameters to the SliceDiagram (and then to the VisualizationWindow)

    void compute_right_point(); //sets correct position of right_point, given slope of line and position of left point
};


#endif // SLICE_LINE_H
