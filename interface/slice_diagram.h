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


//now for the SliceDiagram class
class SliceDiagram
{
public:
    SliceDiagram(QGraphicsScene* sc, VisualizationWindow* vw, double xmin, double xmax, double ymin, double ymax, int width, int height);

    void add_point(double x_coord, double y_coord, int xi0m, int xi1m);

    void create_diagram();

    void update_line(double angle, double offset);  //updates the line, in response to a change in the controls in the VisualizationWindow
    void update_window_controls();   //updates controls in the VisualizationWindow, in response to a change in the line

    double get_slice_length();  //gets the length of the slice, for scaling the persistence diagram
    double get_pd_scale();      //gets the number of pixels per unit, for the persistence diagram
    double get_zero();          //gets the coordinate on the slice line which we consider "zero" for the persistence diagram

private:
    //graphics items
    QGraphicsScene* scene;
    VisualizationWindow* window;

    QGraphicsSimpleTextItem* data_xmin_text;
    QGraphicsSimpleTextItem* data_xmax_text;
    QGraphicsSimpleTextItem* data_ymin_text;
    QGraphicsSimpleTextItem* data_ymax_text;

    QGraphicsRectItem* control_rect;    //control dots live on this rectangle
    ControlDot* dot_left;
    ControlDot* dot_right;

    SliceLine* slice_line;

    //data items
    std::vector<xiPoint> points;    //point data to be drawn in the slice area
    double data_xmin, data_xmax, data_ymin, data_ymax;  //min and max coordinates of the data

    //parameters
    int box_width, box_height;
    double default_scale, normalized_scale_x, normalized_scale_y;
    const int unit_radius;  //radius for a dot representing a xi support point of multiplicity 1
    const int padding;  //distance between xi support point area and control rectangle
};

#endif // SLICE_DIAGRAM_H
