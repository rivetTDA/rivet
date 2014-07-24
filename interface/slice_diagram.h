#ifndef SLICE_DIAGRAM_H
#define SLICE_DIAGRAM_H

#include <QGraphicsScene>
#include <QtGui>

#include "../visualizationwindow.h"
class VisualizationWindow;
#include "control_dot.h"
class ControlDot;
#include "slice_line.h"
class SliceLine;

//first, a little struct to organize the data used to draw the points in the SliceDiagram
struct xiPoint
{
    double x, y;    //coordinates
    int zero, one;  //multiplicity of xi_0 and xi_1 at this point

    xiPoint(double xc, double yc, double m0, double m1) : x(xc), y(yc), zero(m0), one(m1)
    { }
};


//now for the SliceArea class
class SliceDiagram
{
public:
    SliceDiagram(QGraphicsScene* sc, VisualizationWindow* vw);

    void add_point(double x_coord, double y_coord, int xi0m, int xi1m);

    void create_diagram();

    ControlDot* get_dot(bool lb);

    void update_line(QPointF& pos, bool lb);

private:
    //graphics items
    QGraphicsScene* scene;
    VisualizationWindow* window;

    QGraphicsRectItem* data_rect;
    QGraphicsSimpleTextItem* data_xmin_text;
    QGraphicsSimpleTextItem* data_xmax_text;
    QGraphicsSimpleTextItem* data_ymin_text;
    QGraphicsSimpleTextItem* data_ymax_text;

    ControlDot* dot_left;
    ControlDot* dot_right;

    SliceLine* slice_line;

    //data items
    std::vector<xiPoint> points;    //point data to be drawn in the slice area
    double xmin, xmax, ymin, ymax;  //min and max coordinates of the data
    double scale;

};

#endif // SLICE_DIAGRAM_H
