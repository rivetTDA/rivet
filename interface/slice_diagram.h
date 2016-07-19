#ifndef SLICE_DIAGRAM_H
#define SLICE_DIAGRAM_H

//forward declarations
class Barcode;
class ControlDot;
struct ConfigParameters;
class PersistenceBar;
class SliceLine;

#include <QGraphicsScene>
#include <QtWidgets>

#include "boost/multi_array.hpp"
typedef boost::multi_array<unsigned, 2> unsigned_matrix;
typedef boost::multi_array<QGraphicsRectItem*, 2> QGRI_matrix;

#include <list>
#include <utility>  //for std::pair
#include <vector>




class SliceDiagram : public QGraphicsScene
{
    Q_OBJECT

public:
    SliceDiagram(ConfigParameters* params, std::vector<double>& x_grades, std::vector<double>& y_grades, QObject* parent = 0);
    ~SliceDiagram();

    void add_point(double x_coord, double y_coord, int xi0m, int xi1m, int xi2m); //receives an xi support point, which will be drawn when create_diagram() is called

    void create_diagram(QString x_text, QString y_text, double xmin, double xmax, double ymin, double ymax, bool norm_coords, unsigned_matrix& hom_dims);  //simply creates all objects; resize_diagram() handles positioning of objects
    void resize_diagram();   //resizes diagram to fill the QGraphicsView
    void redraw_dim_rects(); //redraws the rectangles for the homology dimension visualization
    void redraw_dots();      //redraws the support points of the multigraded Betti numbers

    void update_line(double angle, double offset);  //updates the line, in response to a change in the controls in the VisualizationWindow
    void update_window_controls(bool from_dot);   //computes new angle and offset in response to a change in the line, emits signal for the VisualizationWindow

    void draw_barcode(Barcode* bc, double zero_coord, bool show); //draws the barcode parallel to the slice line; "show" determines whether or not bars are visible
    void update_barcode(Barcode* bc, double zero_coord, bool show);  //updates the barcode (e.g. after a change in the slice line)

    void select_bar(PersistenceBar* clicked);   //highlight the specified class of bars, and propagate to the persistence diagram
    void deselect_bar();                        //remove selection and propagate to the persistence diagram

    void update_highlight();  //highlights part of the slice line

    void toggle_xi0_points(bool show);  //if "show" is true, then xi_0 support points are drawn; otherwise, they are hidden
    void toggle_xi1_points(bool show);  //if "show" is true, then xi_1 support points are drawn; otherwise, they are hidden
    void toggle_xi2_points(bool show);  //if "show" is true, then xi_2 support points are drawn; otherwise, they are hidden
    void toggle_barcode(bool show);     //if "show" is true, then barcode is drawn; otherwise, it is hidden
    void set_normalized_coords(bool toggle);    //toggles whether the diagram is drawn to scale or fills the window

    double get_slice_length();  //gets the length of the slice, for scaling the persistence diagram
    double get_pd_scale();      //gets the number of pixels per unit, for the persistence diagram

    void receive_parameter_change(QString& xtext, QString& ytext);            //updates the diagram after a change in configuration parameters

public slots:
    void receive_bar_selection(std::vector<unsigned> indexes); //highlight the specified class of bars, which has been selected externally
    void receive_bar_secondary_selection(std::vector<unsigned> indexes);    //secondary highlight, used for persistence dots that represent multiple classes of bars
    void receive_bar_deselection();             //remove bar highlighting in response to external command

signals:
    void set_line_control_elements(double angle, double offset);    //sends updates to, e.g., the VisualizationWindow
    void persistence_bar_selected(unsigned index);                  //triggered when the user selects a bar in the barcode
    void persistence_bar_deselected();                              //triggered when the user deselects a bar in the barcode

private:
  //parameters
    ConfigParameters* config_params;

  //graphics items
    QGraphicsSimpleTextItem* data_xmin_text;
    QGraphicsSimpleTextItem* data_xmax_text;
    QGraphicsSimpleTextItem* data_ymin_text;
    QGraphicsSimpleTextItem* data_ymax_text;
    QGraphicsSimpleTextItem* x_label;
    QGraphicsSimpleTextItem* y_label;

    QGraphicsRectItem* control_rect;            //control dots live on this rectangle
    QGraphicsLineItem* gray_line_vertical;      //vertical gray line at the right of the diagram
    QGraphicsLineItem* gray_line_horizontal;    //horizontal gray line at the top of the diagram
    ControlDot* dot_left;
    ControlDot* dot_right;

    SliceLine* slice_line;
    QGraphicsLineItem* highlight_line;

    std::vector<QGraphicsEllipseItem*> xi0_dots;    //pointers to all xi0 dots
    std::vector<QGraphicsEllipseItem*> xi1_dots;    //pointers to all xi1 dots
    std::vector<QGraphicsEllipseItem*> xi2_dots;    //pointers to all xi2 dots
    std::vector< std::list<PersistenceBar*> > bars; //pointers to all bars (in the barcode) -- each element of the vector stores a list of one or more identical bars that correspond to a single dot in the persistence diagram

    QGRI_matrix hom_dim_rects;     //rectangles that plot the homology dimensions

    std::vector<unsigned> primary_selected;     //indexes of classes of bars in the primary selection
    std::vector<unsigned> secondary_selected;   //indexes of classes of bars in the secondary selection

    void clear_selection();   //unselect all bars

  //data items
    struct xiFloatingPoint  ///TODO: is this a good design? consider how to most efficiently store the points
    {
        double x, y;    //floating-point coordinates
        int zero, one, two;  //multiplicity of xi_0, xi_1, xi_2 at this point

        xiFloatingPoint(double xc, double yc, int m0, int m1, int m2) : x(xc), y(yc), zero(m0), one(m1), two(m2)
        { }
    };
    std::vector<xiFloatingPoint> points;    //point data to be drawn in the slice area

    const std::vector<double>& x_grades;    //for dimension visualization
    const std::vector<double>& y_grades;    // "

    ///TODO: the next four values can be obtained from x_grades and y_grades
    double data_xmin, data_xmax, data_ymin, data_ymax;  //min and max coordinates of the data
    double line_zero; //coordinate of projection of lower-left corner of line-selection window onto selected line
    double data_infty;      //data position that is outside of the window, used for drawing bars that extend to infinity
    int max_xi_value;       //max value of the bigraded betti numbers

    int diagram_width, diagram_height;  //pixel size of the diagram
    bool normalized_coords;             //whether the user has selected "normalize coordinates"
    double scale_x, scale_y;            //x- and y-scales for drawing data points

    double line_slope;   //slope of the slice line in data units
    bool line_vert;      //true if the line is vertical, false otherwise
    double line_pos;     //relative position of left endpoint of line: 0 is lower left corner, positive values (up to 1) are along left side, negative values (to -1) are along bottom edge of box

    const int padding;  //distance between xi support point area and control rectangle (on the top and right sides)

    bool control_dot_moved; //true if line is moved by a ControlDot -- used as part of a hack to make barcode display in the proper position relative to the line

    const double PI;   //used in get_pd_scale() when the slice line is vertical

  //private functions
    std::pair<double,double> compute_endpoint(double coordinate, unsigned offset);  //computes an endpoint of a bar in the barcode
};

#endif // SLICE_DIAGRAM_H
