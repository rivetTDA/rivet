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
#include "persistence_bar.h"
class PersistenceBar;
#include "config_parameters.h"


class SliceDiagram
{
public:
    SliceDiagram(QGraphicsScene* sc, VisualizationWindow* vw, ConfigParameters* params, double xmin, double xmax, double ymin, double ymax, bool norm_coords);

    void add_point(double x_coord, double y_coord, int xi0m, int xi1m);

    void create_diagram(QString x_text, QString y_text);  //simply creates all objects; resize_diagram() handles positioning of objects
    void resize_diagram();  //resizes diagram to fill the QGraphicsView

    void update_line(double angle, double offset);  //updates the line, in response to a change in the controls in the VisualizationWindow
    void update_window_controls();   //updates controls in the VisualizationWindow, in response to a change in the line

    void draw_barcode(Barcode* bc, bool show); //draws the barcode parallel to the slice line; "show" determines whether or not bars are visible
    void update_barcode(Barcode* bc, bool show);  //updates the barcode (e.g. after a change in the slice line)

    void select_bar(PersistenceBar* clicked);   //highlight the specified bar, selected in the slice diagram, and propagate to the persistence diagram
    void select_bar(unsigned index);            //highlight the specified bar, which has been selected in the persistence diagram
    void deselect_bar(bool propagate);          //remove selection; if propagate, then deselect dot in the persistence diagram

    void update_highlight();  //highlights part of the slice line

    void toggle_xi0_points(bool show);  //if "show" is true, then xi_0 support points are drawn; otherwise, they are hidden
    void toggle_xi1_points(bool show);  //if "show" is true, then xi_1 support points are drawn; otherwise, they are hidden
    void toggle_barcode(bool show);     //if "show" is true, then barcode is drawn; otherwise, it is hidden
    void set_normalized_coords(bool toggle);

    double get_slice_length();  //gets the length of the slice, for scaling the persistence diagram
    double get_pd_scale();      //gets the number of pixels per unit, for the persistence diagram
    double get_zero();          //gets the coordinate on the slice line which we consider "zero" for the persistence diagram

private:
  //graphics items
    QGraphicsScene* scene;
    VisualizationWindow* window;

    ConfigParameters* config_params;

    QGraphicsSimpleTextItem* data_xmin_text;
    QGraphicsSimpleTextItem* data_xmax_text;
    QGraphicsSimpleTextItem* data_ymin_text;
    QGraphicsSimpleTextItem* data_ymax_text;
    QGraphicsSimpleTextItem* x_label;
    QGraphicsSimpleTextItem* y_label;

    QGraphicsRectItem* control_rect;    //control dots live on this rectangle
    QGraphicsLineItem* gray_line_vertical;      //vertical gray line at the right of the diagram
    QGraphicsLineItem* gray_line_horizontal;    //horizontal gray line at the top of the diagram
    ControlDot* dot_left;
    ControlDot* dot_right;

    SliceLine* slice_line;
    QGraphicsLineItem* highlight_line;

    std::vector<QGraphicsEllipseItem*> xi0_dots;    //pointers to all green dots
    std::vector<QGraphicsEllipseItem*> xi1_dots;    //pointers to all red dots
    std::vector< std::list<PersistenceBar*> > bars; //pointers to all bars (in the barcode) -- each element of the vector stores a list of one or more identical bars that correspond to a single dot in the persistence diagram

    unsigned selected;              //index of the class of selected bars
    const unsigned NOT_SELECTED;    //set to max_unsigned

  //data items
    struct xiFloatingPoint  ///TODO: is this a good design? consider how to most efficiently store the points
    {
        double x, y;    //floating-point coordinates
        int zero, one;  //multiplicity of xi_0 and xi_1 at this point

        xiFloatingPoint(double xc, double yc, int m0, int m1) : x(xc), y(yc), zero(m0), one(m1)
        { }
    };
    std::vector<xiFloatingPoint> points;    //point data to be drawn in the slice area


    double data_xmin, data_xmax, data_ymin, data_ymax;  //min and max coordinates of the data
    double data_infty;      //data position that is outside of the window, used for drawing bars that extend to infinity

    int diagram_width, diagram_height;  //pixel size of the diagram
    bool normalized_coords;             //whether the user has selected "normalize coordinates"
    double scale_x, scale_y;            //x- and y-scales for drawing data points

    double line_slope;   //slope of the slice line in data units
    bool line_vert;      //true if the line is vertical, false otherwise
    double line_pos;     //relative position of left endpoint of line: 0 is lower left corner, positive values (up to 1) are along left side, negative values (to -1) are along bottom edge of box

    const int unit_radius;  //radius for a dot representing a xi support point of multiplicity 1
    const int padding;  //distance between xi support point area and control rectangle (on the top and right sides)



    //private functions
    std::pair<double,double> compute_endpoint(double coordinate, unsigned offset);  //computes an endpoint of a bar in the barcode
};

#endif // SLICE_DIAGRAM_H
