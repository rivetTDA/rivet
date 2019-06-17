/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

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
#include <utility> //for std::pair
#include <vector>

class SliceDiagram : public QGraphicsScene {
    Q_OBJECT

    //todo: make the gray outline of the gradings visible
public:
    SliceDiagram(ConfigParameters* params, std::vector<double>& x_grades, std::vector<double>& y_grades, QObject* parent = 0);
    ~SliceDiagram();

    void add_point(double x_coord, double y_coord, int xi0m, int xi1m, int xi2m); //receives an xi support point, which will be drawn when create_diagram() is called
    void clear_points(); //removes all previously-created points from the diagram

    void create_diagram(const QString x_text, const QString y_text, double xmin, double xmax, double ymin, double ymax, bool norm_coords, unsigned_matrix& hom_dims, bool x_reverse = false, bool y_reverse = false); //simply creates all objects; resize_diagram() handles positioning of objects
    void enable_slice_line(); //enables the slice line and control dots
    bool is_created(); //true if the diagram has been created; false otherwise
    void resize_diagram(); //resizes diagram to fill the QGraphicsView
    void redraw_dim_rects(); //redraws the rectangles for the homology dimension visualization
    void redraw_dots(); //redraws the support points of the multigraded Betti numbers
    void redraw_labels(); //redraws axis labels in same position on top of rectangles

    void zoom_diagram(double angle, double offset, double distance_to_origin); //redraws diagram in response to a change in bounds

    void update_line(double angle, double offset, double distance_to_origin); //updates the line, in response to a change in the controls in the VisualizationWindow
    void update_window_controls(bool from_dot); //computes new angle and offset in response to a change in the line, emits signal for the VisualizationWindow

    void update_BottomX(double bottom_x, double distance_to_origin, bool visible); //called when the window bounds are changed; distance to origin and visible refer to the corresponding values in the new window
    void update_BottomY(double bottom_y, double distance_to_origin, bool visible);
    void update_TopX(double top_x, double distance_to_origin, bool visible);
    void update_TopY(double top_y, double distance_to_origin, bool visible);

    void draw_barcode(const Barcode& bc, bool show); //draws the barcode parallel to the slice line; "show" determines whether or not bars are visible
    void update_barcode(const Barcode& bc, bool show); //updates the barcode (e.g. after a change in the slice line)

    void select_bar(PersistenceBar* clicked); //highlight the specified class of bars, and propagate to the persistence diagram
    void deselect_bar(); //remove selection and propagate to the persistence diagram

    void update_highlight(); //highlights part of the slice line

    void toggle_xi0_points(bool show); //if "show" is true, then xi_0 support points are drawn; otherwise, they are hidden
    void toggle_xi1_points(bool show); //if "show" is true, then xi_1 support points are drawn; otherwise, they are hidden
    void toggle_xi2_points(bool show); //if "show" is true, then xi_2 support points are drawn; otherwise, they are hidden
    void toggle_barcode(bool show); //if "show" is true, then barcode is drawn; otherwise, it is hidden
    void set_normalized_coords(bool toggle); //toggles whether the diagram is drawn to scale or fills the window

    double get_slice_length(); //gets the length of the slice, for scaling the persistence diagram
    double get_pd_scale(); //gets the number of pixels per unit, for the persistence diagram

    double get_original_xmin();
    double get_original_xmax();
    double get_original_ymin();
    double get_original_ymax();

    double get_min_supp_xi_x();
    double get_max_supp_xi_x();
    double get_min_supp_xi_y();
    double get_max_supp_xi_y();

    bool get_line_visible(); //true if the line is visible

    void receive_parameter_change(); //updates the diagram after a change in configuration parameters

    //functions used in the visualization window, to detect when the line goes out of bounds
    int control_width() { return control_rect->rect().width(); }; //the width of the region in which the line is allowed to move (in pixels)
    int control_height() { return control_rect->rect().height(); };
    int get_diagram_width() { return diagram_width; }; //the width of the above region corresponding to the displayed window bounds
    int get_diagram_height() { return diagram_height; };

    void reset(ConfigParameters* params, std::vector<double>& x_grades, std::vector<double>& y_grades, QObject* parent);

public slots:
    void receive_bar_selection(std::vector<unsigned> indexes); //highlight the specified class of bars, which has been selected externally
    void receive_bar_secondary_selection(std::vector<unsigned> indexes); //secondary highlight, used for persistence dots that represent multiple classes of bars
    void receive_bar_deselection(); //remove bar highlighting in response to external command

signals:
    void set_line_control_elements(double angle, double offset); //sends updates to, e.g., the VisualizationWindow
    void persistence_bar_selected(unsigned index); //triggered when the user selects a bar in the barcode
    void persistence_bar_deselected(); //triggered when the user deselects a bar in the barcode

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

    QGraphicsRectItem* control_rect; //control dots live on this rectangle
    QGraphicsLineItem* gray_line_vertical; //vertical gray line at the right of the grading rectangle
    QGraphicsLineItem* gray_line_horizontal; //horizontal gray line at the top of the grading rectangle
    QGraphicsLineItem* gray_line_vertical_left; //vertical gray line at the left of the grading rectangle
    QGraphicsLineItem* gray_line_horizontal_bottom; //horizontal gray line at the bottom of the grading rectangle

    QGraphicsRectItem* rect1;
    QGraphicsRectItem* rect2;
    QGraphicsRectItem* rect3;
    QGraphicsRectItem* rect4;
    QGraphicsRectItem* rect5;
    QGraphicsRectItem* rect6;

    ControlDot* dot_left;
    ControlDot* dot_right;

    SliceLine* slice_line;
    QGraphicsLineItem* highlight_line;

    std::vector<QGraphicsEllipseItem*> xi0_dots; //pointers to all xi0 dots
    std::vector<QGraphicsEllipseItem*> xi1_dots; //pointers to all xi1 dots
    std::vector<QGraphicsEllipseItem*> xi2_dots; //pointers to all xi2 dots
    std::vector<std::list<PersistenceBar*>> bars; //pointers to all bars (in the barcode) -- each element of the vector stores a list of one or more identical bars that correspond to a single dot in the persistence diagram

    QGRI_matrix hom_dim_rects; //rectangles that plot the homology dimensions

    std::vector<unsigned> primary_selected; //indexes of classes of bars in the primary selection
    std::vector<unsigned> secondary_selected; //indexes of classes of bars in the secondary selection

    void clear_selection(); //unselect all bars

    //data items
    struct xiFloatingPoint ///TODO: is this a good design? consider how to most efficiently store the points
    {
        double x, y; //floating-point coordinates
        int zero, one, two; //multiplicity of xi_0, xi_1, xi_2 at this point

        xiFloatingPoint(double xc, double yc, int m0, int m1, int m2)
            : x(xc)
            , y(yc)
            , zero(m0)
            , one(m1)
            , two(m2)
        {
        }
    };
    std::vector<xiFloatingPoint> points; //point data to be drawn in the slice area

    const std::vector<double>& x_grades; //for dimension visualization
    const std::vector<double>& y_grades; // "

    ///TODO: the next four values can be obtained from x_grades and y_grades
    double original_xmin, original_xmax, original_ymin, original_ymax; //default bounds-these are unchanged after initialization
    double data_xmin, data_xmax, data_ymin, data_ymax; //min and max coordinates of the CURRENT WINDOW
    int view_length; //width + height of the QGraphicsView that displays the diagram; used for drawing infinite bars
    int max_xi_value; //max value of the bigraded betti numbers

    QString x_label_text;
    QString y_label_text;

    double min_supp_xi_x; //the minimal x value in the support of the betti numbers
    double max_supp_xi_x;
    double min_supp_xi_y;
    double max_supp_xi_y;

    int diagram_width, diagram_height; //pixel size of the diagram
    bool normalized_coords; //whether the user has selected "normalize coordinates"
    double scale_x, scale_y; //x- and y-scales for drawing data points
    double xrev_sign, yrev_sign; //these are 1/-1 depending on whether the corresponding axis
    //is shown in reverse order

    double line_slope; //slope of the slice line in data units
    bool line_vert; //true if the line is vertical, false otherwise
    double line_pos; //relative position of left endpoint of line: 0 is lower left corner, positive values (up to 1) are along left side, negative values (to -1) are along bottom edge of box

    bool line_visible; //true if the line is visible in the current window

    double dist_to_origin; //signed distance in data units from the bottom left corner to the origin
    const int padding; //distance between xi support point area and control rectangle (on the top and right sides)

    bool created; //true once the diagram has been created
    bool control_dot_moved; //true if line is moved by a ControlDot -- used as part of a hack to make barcode display in the proper position relative to the line

    const double PI; //used in get_pd_scale() when the slice line is vertical

    //private functions
    std::pair<double, double> compute_endpoint(double coordinate, unsigned offset); //computes an endpoint of a bar in the barcode
};

#endif // SLICE_DIAGRAM_H
