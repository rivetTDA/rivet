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

#include "slice_diagram.h"

#include "config_parameters.h"
#include "control_dot.h"
#include "dcel/barcode.h"
#include "persistence_bar.h"
#include "slice_line.h"

#include <QDebug>
#include <QGraphicsView>

#include <algorithm> // std::min
#include <cmath> // c++ version of math.h; includes overloaded absolute value functions
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
SliceDiagram::SliceDiagram(ConfigParameters* params, std::vector<double>& x_grades, std::vector<double>& y_grades, QObject* parent)
    : QGraphicsScene(parent)
    , config_params(params)
    , dot_left(nullptr)
    , dot_right(nullptr)
    , slice_line(nullptr)
    , x_grades(x_grades)
    , y_grades(y_grades)
    , max_xi_value(0)
    , line_visible(true)
    , padding(20)
    , created(false)
    , control_dot_moved(false)
    , PI(3.14159265358979323846)

{
}

SliceDiagram::~SliceDiagram()
{
    clear(); //removes and deletes all items from the QGraphicsScene
}

// resets data structures and variables to draw new diagram
void SliceDiagram::reset()
{
    clear();
    xi0_dots.clear();
    xi1_dots.clear();
    xi2_dots.clear();
    bars.clear();
    primary_selected.clear();
    secondary_selected.clear();
    points.clear();
    hom_dim_rects.resize(boost::extents[0][0]);
    dot_left = nullptr;
    dot_right = nullptr;
    slice_line = nullptr;
    max_xi_value = 0;
    line_visible = true;
    created = false;
    control_dot_moved = false;
}

//receives an xi support point, which will be drawn when create_diagram() is called
void SliceDiagram::add_point(double x_coord, double y_coord, int xi0m, int xi1m, int xi2m)
{
    points.push_back(xiFloatingPoint(x_coord, y_coord, xi0m, xi1m, xi2m));

    if (xi0m > max_xi_value)
        max_xi_value = xi0m;
    if (xi1m > max_xi_value)
        max_xi_value = xi1m;
    if (xi2m > max_xi_value)
        max_xi_value = xi2m;
}

//removes all previously-created points from the diagram
void SliceDiagram::clear_points()
{
    points.clear();
    max_xi_value = 0;
}

//NOTE: create_diagram() simply creates all objects; resize_diagram() handles positioning of objects
void SliceDiagram::create_diagram(const QString x_text, const QString y_text, double xmin, double xmax, double ymin, double ymax, bool norm_coords, unsigned_matrix& hom_dims, bool x_reverse, bool y_reverse)
{
    //xmin is always less than xmax
    //x_reverse controls whetehr the values are shown with smallest on the left or the right
    xrev_sign = -2 * (x_reverse) + 1;
    yrev_sign = -2 * (y_reverse) + 1;

    //set data-dependent parameters
    data_xmin = xmin;
    data_xmax = xmax;
    data_ymin = ymin;
    data_ymax = ymax;
    normalized_coords = norm_coords;

    original_xmin = xmin;
    original_xmax = xmax;
    original_ymin = ymin;
    original_ymax = ymax;

    x_label_text = x_text;
    y_label_text = y_text;

    //pens and brushes
    QPen blackPen(Qt::black);
    blackPen.setWidth(2);
    QBrush xi0brush(config_params->xi0color);
    QBrush xi1brush(config_params->xi1color);
    QBrush xi2brush(config_params->xi2color);
    QPen grayPen(Qt::gray);
    QPen highlighter(QBrush(config_params->persistenceHighlightColor), config_params->sliceLineWidth / 2);

    //draw labels
    std::ostringstream s_xmin;
    s_xmin.precision(4);
    s_xmin << data_xmin;
    data_xmin_text = addSimpleText(QString(" "));
    data_xmin_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    data_xmin_text->setFont(config_params->diagramFont);

    std::ostringstream s_xmax;
    s_xmax.precision(4);
    s_xmax << data_xmax;
    data_xmax_text = addSimpleText(QString(" "));
    data_xmax_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    data_xmax_text->setFont(config_params->diagramFont);

    std::ostringstream s_ymin;
    s_ymin.precision(4);
    s_ymin << data_ymin;
    //initialize the y label with the initial value so that the
    //horizontal spacing relative to the boundary is correct
    data_ymin_text = addSimpleText(QString(s_ymin.str().data()));

    data_ymin_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    data_ymin_text->setFont(config_params->diagramFont);

    std::ostringstream s_ymax;
    s_ymax.precision(4);
    s_ymax << data_ymax;
    data_ymax_text = addSimpleText(QString(s_ymax.str().data()));
    data_ymax_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    data_ymax_text->setFont(config_params->diagramFont);

    x_label = addSimpleText(x_label_text);
    x_label->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    x_label->setFont(config_params->diagramFont);

    y_label = addSimpleText(y_label_text);
    y_label->setTransform(QTransform(0, 1, 1, 0, 0, 0));
    y_label->setFont(config_params->diagramFont);

    rect1 = addRect(0, 0, 0, 0, Qt::NoPen, QBrush(QColor(255, 255, 255)));
    rect2 = addRect(0, 0, 0, 0, Qt::NoPen, QBrush(QColor(255, 255, 255)));
    rect3 = addRect(0, 0, 0, 0, Qt::NoPen, QBrush(QColor(255, 255, 255)));
    rect4 = addRect(0, 0, 0, 0, Qt::NoPen, QBrush(QColor(255, 255, 255)));
    rect5 = addRect(0, 0, 0, 0, Qt::NoPen, QBrush(QColor(255, 255, 255)));
    rect6 = addRect(0, 0, 0, 0, Qt::NoPen, QBrush(QColor(255, 255, 255)));

    int BigZValue = 1000; //probably not the best way to do this...
    rect1->setZValue(BigZValue);
    rect2->setZValue(BigZValue);
    rect3->setZValue(BigZValue);
    rect4->setZValue(BigZValue);
    rect5->setZValue(BigZValue);
    rect6->setZValue(BigZValue);

    data_xmin_text->setZValue(BigZValue + 1);
    data_ymin_text->setZValue(BigZValue + 1);
    data_xmax_text->setZValue(BigZValue + 1);
    data_ymax_text->setZValue(BigZValue + 1);
    x_label->setZValue(BigZValue + 1);
    y_label->setZValue(BigZValue + 1);

    //create rectangles for visualizing homology dimensions
    //first, find max dimension
    unsigned max_hom_dim = 0;
    for (unsigned i = 0; i < x_grades.size(); i++)
        for (unsigned j = 0; j < y_grades.size(); j++)
            if (hom_dims[i][j] > max_hom_dim)
                max_hom_dim = hom_dims[i][j];
    if (max_hom_dim == 0)
        max_hom_dim = 1;

    //now create the rectangles
    hom_dim_rects.resize(boost::extents[x_grades.size()][y_grades.size()]);
    for (unsigned i = 0; i < x_grades.size(); i++) {
        for (unsigned j = 0; j < y_grades.size(); j++) {
            int gray_value = 255; //white
            if (hom_dims[i][j] > 0 && hom_dims[i][j] < 80)
                gray_value = (int)(220 - 50 * log(hom_dims[i][j]));
            else if (hom_dims[i][j] >= 80)
                gray_value = 0; //black

            QGraphicsRectItem* item = addRect(QRectF(), Qt::NoPen, QBrush(QColor(gray_value, gray_value, gray_value)));
            item->setToolTip(QString("dimension = ") + QString::number(hom_dims[i][j]));
            hom_dim_rects[i][j] = item;
        }
    }

    //draw bounds
    gray_line_vertical = addLine(QLineF(), grayPen); //(diagram_width, 0, diagram_width, diagram_height, grayPen);
    gray_line_horizontal = addLine(QLineF(), grayPen); //0, diagram_height, diagram_width, diagram_height, grayPen);
    gray_line_vertical_left = addLine(QLineF(), grayPen);
    gray_line_horizontal_bottom = addLine(QLineF(), grayPen);

    control_rect = addRect(QRectF(), blackPen); //0,0,diagram_width + padding,diagram_height + padding, blackPen);

    //create points
    for (unsigned i = 0; i < points.size(); i++) {
        //build tooltip string
        QString tooltip = QString("Betti(") + QString::number(points[i].x) + ", " + QString::number(points[i].y) + ") = (" + QString::number(points[i].zero) + ", " + QString::number(points[i].one) + ", " + QString::number(points[i].two) + ")";

        //create graphics items
        if (points[i].zero > 0) //then draw a xi0 disk
        {
            QGraphicsEllipseItem* item = addEllipse(QRectF(), Qt::NoPen, xi0brush);
            item->setToolTip(tooltip);
            xi0_dots.push_back(item);
        }
        if (points[i].one > 0) //then draw a xi1 disk
        {
            QGraphicsEllipseItem* item = addEllipse(QRectF(), Qt::NoPen, xi1brush);
            item->setToolTip(tooltip);
            xi1_dots.push_back(item);
        }
        if (points[i].two > 0) //then draw a xi2 disk
        {
            QGraphicsEllipseItem* item = addEllipse(QRectF(), Qt::NoPen, xi2brush);
            item->setToolTip(tooltip);
            item->setVisible(false); //NOTE: xi_2 dots are not shown initially
            xi2_dots.push_back(item);
        }
    }

    //find the max and min values of the support of xi

    double min_x_so_far = std::numeric_limits<double>::infinity();
    double min_y_so_far = std::numeric_limits<double>::infinity();
    double max_x_so_far = 0;
    double max_y_so_far = 0;

    for (unsigned i = 0; i < points.size(); i++) {
        double this_x = points[i].x;
        double this_y = points[i].y;
        if (this_x > max_x_so_far) {
            max_x_so_far = this_x;
        }
        if (this_x < min_x_so_far) {
            min_x_so_far = this_x;
        }
        if (this_y > max_y_so_far) {
            max_y_so_far = this_y;
        }
        if (this_y < min_y_so_far) {
            min_y_so_far = this_y;
        }
    }

    min_supp_xi_x = min_x_so_far;
    max_supp_xi_x = max_x_so_far;
    min_supp_xi_y = min_y_so_far;
    max_supp_xi_y = max_y_so_far;

    //add control objects
    line_vert = false; //IS IT POSSIBLE THAT THE INITIAL LINE COULD BE VERTICAL???????????????????????????????????
    line_slope = (data_ymax - data_ymin) / (data_xmax - data_xmin); //slope in data units
    line_pos = 0; //start the line at the lower left corner of the box

    slice_line = new SliceLine(this, config_params);
    addItem(slice_line);
    slice_line->hide();

    dot_left = new ControlDot(slice_line, true, config_params);
    addItem(dot_left);
    dot_left->hide();

    dot_right = new ControlDot(slice_line, false, config_params);
    addItem(dot_right);
    dot_right->hide();

    dot_left->other = dot_right;
    dot_right->other = dot_left;

    slice_line->setDots(dot_left, dot_right);

    //add highlight line object, which is hidden until it is requested
    highlight_line = addLine(0, 0, 1, 1, highlighter);
    highlight_line->hide();

    //fit scene to view -- THIS SETS POSITIONS OF ALL OBJECTS CREATED ABOVE!!!
    resize_diagram();

    //update angle and offset boxes in VisualizationWindow
    update_window_controls(false);

    //remember that the diagram has been created
    created = true;

} //end create_diagram()

void SliceDiagram::enable_slice_line() //enables the slice line and control dots
{
    slice_line->show();
    dot_left->show();
    dot_right->show();
}

//removes all graphics elements from the diagram
bool SliceDiagram::is_created()
{
    return created;
}

//resizes diagram to fill the QGraphicsView
void SliceDiagram::resize_diagram()
{

    //parameters
    int scene_padding = 30; //pixels
    int text_padding = 5; //pixels

    //get dimensions of the QGraphicsView
    QList<QGraphicsView*> view_list = views();
    int view_width = view_list[0]->width();
    int view_height = view_list[0]->height();
    view_length = view_width + view_height;

    //determine scale
    double left_text_width = std::max(data_ymin_text->boundingRect().width(), data_ymax_text->boundingRect().width());
    double diagram_max_width = view_width - padding - 2 * scene_padding - text_padding - left_text_width;
    double lower_text_height = std::max(data_xmin_text->boundingRect().height(), data_xmax_text->boundingRect().height());
    double diagram_max_height = view_height - padding - 2 * scene_padding - text_padding - lower_text_height;

    if (data_xmax > data_xmin)
        scale_x = diagram_max_width / (data_xmax - data_xmin);
    else //then there is only one x-grade
        scale_x = 100; //only matters if there is one x-grade and a few y-grades

    if (data_ymax > data_ymin)
        scale_y = diagram_max_height / (data_ymax - data_ymin);
    else //then there is only one x-grade
        scale_y = 100; //only matters if there is one y-grade and a few x-grades

    if (!normalized_coords) //then we want scale_x and scale_y to be the same (choose the smaller of the two)
    {
        if (scale_y < scale_x)
            scale_x = scale_y;
        else
            scale_y = scale_x;
    }

    //determine diagram size
    diagram_width = scale_x * (data_xmax - data_xmin); //units: pixels
    diagram_height = scale_y * (data_ymax - data_ymin); //units: pixels

    //determine automatic dot sizes, if necessary
    if (config_params->autoDotSize && max_xi_value > 0) {
        int x_grid = diagram_width / x_grades.size();
        int y_grid = diagram_height / y_grades.size();
        int min_grid = (x_grid < y_grid) ? x_grid : y_grid;

        int auto_radius = (int)min_grid / sqrt(max_xi_value);
        int max_radius = (int)(std::min(diagram_max_height, diagram_max_width) / 20);
        if (auto_radius < 3)
            auto_radius = 3;
        if (auto_radius > max_radius)
            auto_radius = max_radius;

        config_params->bettiDotRadius = auto_radius;
        config_params->persistenceDotRadius = auto_radius;
    }

    //reposition reference objects
    control_rect->setRect(0, 0, diagram_width + padding, diagram_height + padding);

    double gray_box_xmin = fmax(original_xmin - data_xmin, 0.0) / (data_xmax - data_xmin); //top left corner of gray box, relative units
    double gray_box_xmax = fmax(original_xmax - data_xmin, 0.0) / (data_xmax - data_xmin);
    double gray_box_ymin = fmax(original_ymin - data_ymin, 0.0) / (data_ymax - data_ymin);
    double gray_box_ymax = fmax(original_ymax - data_ymin, 0.0) / (data_ymax - data_ymin);

    gray_line_vertical->setLine(diagram_width * gray_box_xmax, diagram_height * gray_box_ymin, diagram_width * gray_box_xmax, diagram_height * gray_box_ymax);
    gray_line_horizontal->setLine(diagram_width * gray_box_xmin, diagram_height * gray_box_ymax, diagram_width * gray_box_xmax, diagram_height * gray_box_ymax);
    gray_line_vertical_left->setLine(diagram_width * gray_box_xmin, diagram_height * gray_box_ymin, diagram_width * gray_box_xmin, diagram_height * gray_box_ymax);
    gray_line_horizontal_bottom->setLine(diagram_width * gray_box_xmin, diagram_height * gray_box_ymin, diagram_width * gray_box_xmax, diagram_height * gray_box_ymin);

    //reposition dimension rectangles
    redraw_dim_rects();

    //reposition xi points
    redraw_dots();

    //reposition slice line
    slice_line->update_bounds(diagram_width, diagram_height, padding);
    double x = 0, y = 0;
    if (line_pos < 0) //then left-bottom endpoint is along bottom edge of box
        x = -1 * line_pos * diagram_width;
    else //then left-bottom endpoint is along left edge of box
        y = line_pos * diagram_height;
    slice_line->update_position(x, y, line_vert, line_slope * scale_y / scale_x);

    //reposition bars

    unsigned count = 1;
    for (unsigned i = 0; i < bars.size(); i++) {
        for (std::list<PersistenceBar*>::iterator it = bars[i].begin(); it != bars[i].end(); ++it) {
            double start = (*it)->get_start();
            double end = (*it)->get_end();
            std::pair<double, double> p1 = compute_endpoint(start, count);
            std::pair<double, double> p2 = compute_endpoint(end, count);
            (*it)->set_line(p1.first, p1.second, p2.first, p2.second);
            count++;
        }
    }
    //redraw labels
    redraw_labels();

    //clear selection (because resizing window might combine or split dots in the upper strip of the persistence diagram)
    clear_selection();
    highlight_line->hide();

    //reposition highlighting
    if (primary_selected.size() > 0)
        update_highlight();
    //set scene rectangle (necessary to prevent auto-scrolling)
    double scene_rect_x = -left_text_width - text_padding;
    double scene_rect_y = -lower_text_height - text_padding;
    double scene_rect_w = diagram_width + padding + text_padding + left_text_width;
    double scene_rect_h = diagram_height + padding + text_padding + lower_text_height;
    redraw_labels();
    setSceneRect(scene_rect_x, scene_rect_y, scene_rect_w, scene_rect_h);

} //end resize_diagram()

//redraws the rectangles for the homology dimension visualization
void SliceDiagram::redraw_dim_rects()
{
    for (unsigned i = 0; i < x_grades.size(); i++) {
        for (unsigned j = 0; j < y_grades.size(); j++) {
            double left = (x_grades[i] - data_xmin) * scale_x;
            double right = diagram_width + padding;
            if (i + 1 < x_grades.size())
                right = (x_grades[i + 1] - data_xmin) * scale_x + 1;

            double bottom = (y_grades[j] - data_ymin) * scale_y;
            double top = diagram_height + padding;
            if (j + 1 < y_grades.size())
                top = (y_grades[j + 1] - data_ymin) * scale_y + 1;

            left = fmin(fmax(0, left), diagram_width + padding);
            bottom = fmin(fmax(0, bottom), diagram_height + padding);
            right = fmin(fmax(0, right), diagram_width + padding);
            top = fmin(fmax(0, top), diagram_height + padding);
            //ensure the corners are never drawn outside of the window

            hom_dim_rects[i][j]->setRect(left, bottom, right - left, top - bottom);
        }
    }
} //end redraw_dim_rects()

//redraws the support points of the multigraded Betti numbers
void SliceDiagram::redraw_dots()
{
    //NOTE: this should be fine, but if it is too slow, we could store the radius of each dot so that we don't have to compute it on each resize
    std::vector<QGraphicsEllipseItem*>::iterator it0 = xi0_dots.begin();
    std::vector<QGraphicsEllipseItem*>::iterator it1 = xi1_dots.begin();
    std::vector<QGraphicsEllipseItem*>::iterator it2 = xi2_dots.begin();
    bool visible; //whether the center of the dot is visible in the window
    for (unsigned i = 0; i < points.size(); i++) {
        if (points[i].zero > 0) //then draw a xi_0 dot
        {
            double radius = round(config_params->bettiDotRadius * sqrt(points[i].zero));
            visible = (data_xmin <= points[i].x) && (points[i].x <= data_xmax + padding / scale_x) && (data_ymin <= points[i].y) && (points[i].y <= data_ymax + padding / scale_y);
            if (visible) {
                (*it0)->setRect((points[i].x - data_xmin) * scale_x - radius, (points[i].y - data_ymin) * scale_y - radius, 2 * radius, 2 * radius);
            } else {
                (*it0)->setRect(0, 0, 0, 0);
            }
            ++it0;
        }
        if (points[i].one > 0) //then draw a xi_1 dot
        {
            double radius = round(config_params->bettiDotRadius * sqrt(points[i].one));
            visible = (data_xmin <= points[i].x) && (points[i].x <= data_xmax + padding / scale_x) && (data_ymin <= points[i].y) && (points[i].y <= data_ymax + padding / scale_y);
            if (visible) {
                (*it1)->setRect((points[i].x - data_xmin) * scale_x - radius, (points[i].y - data_ymin) * scale_y - radius, 2 * radius, 2 * radius);
            } else {
                (*it1)->setRect(0, 0, 0, 0);
            }
            ++it1;
        }
        if (points[i].two > 0) //then draw a xi_2 dot
        {
            double radius = round(config_params->bettiDotRadius * sqrt(points[i].two));
            visible = (data_xmin <= points[i].x) && (points[i].x <= data_xmax + padding / scale_x) && (data_ymin <= points[i].y) && (points[i].y <= data_ymax + padding / scale_y);
            if (visible) {
                (*it2)->setRect((points[i].x - data_xmin) * scale_x - radius, (points[i].y - data_ymin) * scale_y - radius, 2 * radius, 2 * radius);
            } else {
                (*it2)->setRect(0, 0, 0, 0);
            }
            ++it2;
        }
    }
} //end redraw_dots()

//zoom in or out in response to a user-defined change in window bounds
void SliceDiagram::zoom_diagram(double angle, double offset, double distance_to_origin)
{

    //int text_padding = 5; //pixels

    dist_to_origin = distance_to_origin;

    //reposition dimension rectangles
    redraw_dim_rects();

    //reposition xi points
    redraw_dots();

    double gray_box_xmin = fmax(original_xmin - data_xmin, 0.0) / (data_xmax - data_xmin); //top left corner of gray box, relative units
    double gray_box_xmax = fmax(original_xmax - data_xmin, 0.0) / (data_xmax - data_xmin);
    double gray_box_ymin = fmax(original_ymin - data_ymin, 0.0) / (data_ymax - data_ymin);
    double gray_box_ymax = fmax(original_ymax - data_ymin, 0.0) / (data_ymax - data_ymin);

    gray_line_vertical->setLine(diagram_width * gray_box_xmax, diagram_height * gray_box_ymin, diagram_width * gray_box_xmax, diagram_height * gray_box_ymax);
    gray_line_horizontal->setLine(diagram_width * gray_box_xmin, diagram_height * gray_box_ymax, diagram_width * gray_box_xmax, diagram_height * gray_box_ymax);
    gray_line_vertical_left->setLine(diagram_width * gray_box_xmin, diagram_height * gray_box_ymin, diagram_width * gray_box_xmin, diagram_height * gray_box_ymax);
    gray_line_horizontal_bottom->setLine(diagram_width * gray_box_xmin, diagram_height * gray_box_ymin, diagram_width * gray_box_xmax, diagram_height * gray_box_ymin);

    double intrinsic_y_int = offset / cos(angle * PI / 180);
    double intrinsic_slope = tan(angle * PI / 180);

    line_slope = intrinsic_slope;

    double x = 0, y = 0;
    if (line_vert) {
        double relative_intercept_horz = (-offset - data_xmin) / (data_xmax - data_xmin); //vertical line has negative offset
        x = relative_intercept_horz * diagram_width;
        line_visible = (0 <= relative_intercept_horz && relative_intercept_horz <= 1 + float(padding) / diagram_width);
        slice_line->update_position(x, y, line_vert, 0);
        slice_line->set_visibility(line_visible); //don't plot the line if it lies outisde of the viewing window
        line_pos = -1 * relative_intercept_horz;
    }

    else {
        double relative_intercept_vert = (data_xmin * intrinsic_slope + intrinsic_y_int - data_ymin) / (data_ymax - data_ymin);
        if (relative_intercept_vert < 0) //then left-bottom endpoint is along bottom edge of box
        {
            //the corresponding x point satisfies y_int+slope*(x+xmin)=ymin;
            double relative_intercept_horz = -data_xmin + (data_ymin - intrinsic_y_int) / (intrinsic_slope);
            relative_intercept_horz /= data_xmax - data_xmin;
            x = relative_intercept_horz * diagram_width;
            line_visible = relative_intercept_horz < 1 + float(padding) / diagram_width;
            line_pos = -1 * relative_intercept_horz;
        } else //then left-bottom endpoint is along left edge of box
        {
            y = relative_intercept_vert * diagram_height;
            line_visible = relative_intercept_vert < 1 + float(padding) / diagram_height;
            line_pos = relative_intercept_vert;
        }

        slice_line->update_position(x, y, line_vert, intrinsic_slope * scale_y / scale_x);
        slice_line->set_visibility(line_visible);
    }

    //reposition bars
    unsigned count = 1;
    for (unsigned i = 0; i < bars.size(); i++) {
        for (std::list<PersistenceBar*>::iterator it = bars[i].begin(); it != bars[i].end(); ++it) {
            double start = (*it)->get_start();
            double end = (*it)->get_end();
            std::pair<double, double> p1 = compute_endpoint(start, count);
            std::pair<double, double> p2 = compute_endpoint(end, count);
            (*it)->set_line(p1.first, p1.second, p2.first, p2.second);

            count++;
        }
    }

    //draw the labels
    redraw_labels();

    //clear selection (because resizing window might combine or split dots in the upper strip of the persistence diagram)
    clear_selection();
    highlight_line->hide();

    //reposition highlighting
    if (primary_selected.size() > 0)
        update_highlight();

} //end zoom_diagram()

double SliceDiagram::get_original_xmax()
{
    return original_xmax;
}

double SliceDiagram::get_original_xmin()
{
    return original_xmin;
}

double SliceDiagram::get_original_ymax()
{
    return original_ymax;
}

double SliceDiagram::get_original_ymin()
{
    return original_ymin;
}

double SliceDiagram::get_min_supp_xi_x()
{
    return min_supp_xi_x;
}

double SliceDiagram::get_max_supp_xi_x()
{
    return max_supp_xi_x;
}

double SliceDiagram::get_min_supp_xi_y()
{
    return min_supp_xi_y;
}

double SliceDiagram::get_max_supp_xi_y()
{
    return max_supp_xi_y;
}

bool SliceDiagram::get_line_visible()
{
    return line_visible;
}
//updates the diagram after a change in configuration parameters
void SliceDiagram::receive_parameter_change()
{
    //update colors of the xi dots (necessary because I didn't override their paint() function)
    QBrush xi0brush(config_params->xi0color);
    QBrush xi1brush(config_params->xi1color);
    QBrush xi2brush(config_params->xi2color);
    for (std::vector<QGraphicsEllipseItem*>::iterator it = xi0_dots.begin(); it != xi0_dots.end(); ++it)
        (*it)->setBrush(xi0brush);
    for (std::vector<QGraphicsEllipseItem*>::iterator it = xi1_dots.begin(); it != xi1_dots.end(); ++it)
        (*it)->setBrush(xi1brush);
    for (std::vector<QGraphicsEllipseItem*>::iterator it = xi2_dots.begin(); it != xi2_dots.end(); ++it)
        (*it)->setBrush(xi2brush);

    //update width of persistence bars
    //  this seems excessive, but it's necessary to call prepareGeometryChange() on each bar, and set_width() does this
    for (unsigned i = 0; i < bars.size(); i++) {
        for (std::list<PersistenceBar*>::iterator it = bars[i].begin(); it != bars[i].end(); ++it) {
            (*it)->set_width(config_params->persistenceBarWidth);
        }
    }

    //update the slice line highlight
    QPen highlighter(QBrush(config_params->persistenceHighlightColor), config_params->sliceLineWidth / 2);
    highlight_line->setPen(highlighter);

    //update axis labels
    x_label->setText(config_params->xLabel);
    y_label->setText(config_params->yLabel);

    //update fonts
    data_xmin_text->setFont(config_params->diagramFont);
    data_xmax_text->setFont(config_params->diagramFont);
    data_ymin_text->setFont(config_params->diagramFont);
    data_ymax_text->setFont(config_params->diagramFont);
    x_label->setFont(config_params->diagramFont);
    y_label->setFont(config_params->diagramFont);

    //update diagram
    resize_diagram();
} //end receive_parameter_change()

//updates the line, in response to a change in the controls in the VisualizationWindow
//NOTE: angle is in DEGREES
void SliceDiagram::update_line(double angle, double offset, double distance_to_origin)
{

    dist_to_origin = distance_to_origin;

    if (angle == 90) //handle vertical line
    {
        //update SliceDiagram data values
        line_vert = true;
        line_pos = offset / (data_xmax - data_xmin); //relative units

        line_visible = (-1 <= line_pos && line_pos <= float(padding) / diagram_width); //vertical line has negative offset

        //update the SliceLine

        int xpos = (-1 * offset - data_xmin) * scale_x; //pixel units

        slice_line->update_position(xpos, 0, true, 0);

    } else if (angle == 0) //handle horizontal line
    {
        //update SliceDiagram data values
        line_vert = false;
        line_slope = 0;
        line_pos = offset / (data_ymax - data_ymin); //relative units
        line_visible = (0 <= line_pos && line_pos <= 1 + float(padding) / diagram_height);

        //update the SliceLine
        int ypos = (offset - data_ymin) * scale_y; //pixel units
        slice_line->update_position(0, ypos, false, 0);

    } else //handle non-vertical and non-horizontal line
    {
        //update SliceDiagram data values
        line_vert = false;
        double radians = angle * PI / 180;
        line_slope = tan(radians);

        //update line_pos and the SliceLine
        double y_coord = line_slope * data_xmin + offset / cos(radians); //y-coordinate of slice line at x=data_xmin; data units
        if (y_coord >= data_ymin) //then slice line intersects left edge of box
        {
            line_pos = (y_coord - data_ymin) / (data_ymax - data_ymin); //relative units
            line_visible = line_pos < 1 + float(padding) / diagram_height;
            slice_line->update_position(0, (y_coord - data_ymin) * scale_y, false, line_slope * scale_y / scale_x);

        } else //then slice line intersects bottom of box
        {
            double x_coord = (data_ymin - offset / cos(radians)) / line_slope; //x-coordinate of slice line at y=data_ymin; data units
            line_pos = -1 * (x_coord - data_xmin) / (data_xmax - data_xmin); //relative units
            line_visible = -1 - float(padding) / diagram_width < line_pos;

            slice_line->update_position((x_coord - data_xmin) * scale_x, 0, false, line_slope * scale_y / scale_x);
        }
    }
    slice_line->set_visibility(line_visible);
    highlight_line->hide(); //since the line has changed, the highlighting is no longer valid
} //end update_line()

//updates controls in the VisualizationWindow in response to a change in the line (also update SliceDiagram data values)
void SliceDiagram::update_window_controls(bool from_dot)
{
    //update SliceDiagram data values
    line_vert = slice_line->is_vertical();
    line_slope = slice_line->get_slope() * scale_x / scale_y; //convert pixel units to data units
    if (slice_line->pos().x() > 0) {
        if (diagram_width > 0)
            line_pos = -1 * slice_line->pos().x() / diagram_width;
        else //can this ever happen?
            line_pos = 0;
    } else {
        if (diagram_height > 0)
            line_pos = slice_line->pos().y() / diagram_height;
        else
            line_pos = 0;
    }

    //update VisualizatoinWindow control objects
    //defaults for vertical line
    double angle = 90;
    double offset = -1 * (slice_line->pos().x() / scale_x + data_xmin); //data units

    //handle non-vertical line
    if (!line_vert) {
        angle = atan(line_slope); //radians

        double y_intercept = (slice_line->pos().y() / scale_y + data_ymin - line_slope * (slice_line->pos().x() / scale_x + data_xmin)); //data units
        offset = cos(angle) * y_intercept;

        angle = angle * 180 / PI; //convert to degrees
    }

    //remember whether the source of this change is the move of a ControlDot
    control_dot_moved = from_dot;

    //send updates
    emit set_line_control_elements(angle, offset);

    //since the line has changed, the highlighting is no longer valid
    highlight_line->hide();
} //end update_window_controls()

void SliceDiagram::update_BottomX(double bottom_x, double distance_to_origin, bool visible)
{

    scale_x *= (data_xmax - data_xmin) / (data_xmax - bottom_x);
    data_xmin = bottom_x;
    line_visible = visible;
    dist_to_origin = distance_to_origin;
}

void SliceDiagram::update_BottomY(double bottom_y, double distance_to_origin, bool visible)
{
    scale_y *= (data_ymax - data_ymin) / (data_ymax - bottom_y);
    data_ymin = bottom_y;
    line_visible = visible;
    dist_to_origin = distance_to_origin;
}

void SliceDiagram::update_TopX(double top_x, double distance_to_origin, bool visible)
{
    scale_x *= (data_xmax - data_xmin) / (top_x - data_xmin);
    data_xmax = top_x;
    line_visible = visible;
    dist_to_origin = distance_to_origin;
}

void SliceDiagram::update_TopY(double top_y, double distance_to_origin, bool visible)
{
    scale_y *= (data_ymax - data_ymin) / (top_y - data_ymin);
    data_ymax = top_y;
    line_visible = visible;
    dist_to_origin = distance_to_origin;
}

//draws the barcode parallel to the slice line
void SliceDiagram::draw_barcode(Barcode const& bc, bool show)
{
    bars.resize(bc.size());
    int num_bars = 1;
    unsigned index = 0;
    for (std::multiset<MultiBar>::iterator it = bc.begin(); it != bc.end(); ++it) {
        double start = it->birth;
        double end = it->death;

        for (unsigned i = 0; i < it->multiplicity; i++) {
            std::pair<double, double> p1 = compute_endpoint(start, num_bars);
            std::pair<double, double> p2 = compute_endpoint(end, num_bars);

            PersistenceBar* bar = new PersistenceBar(this, config_params, start, end, index);
            bar->set_line(p1.first, p1.second, p2.first, p2.second);
            bar->setVisible(show);
            addItem(bar);
            bars[index].push_back(bar);
            num_bars++;
        }
        index++;
    }
} //end draw_barcode()

//updates the barcode (e.g. after a change in the slice line)
//TODO: would it be better to move bars, instead of deleting and re-creating them?
void SliceDiagram::update_barcode(Barcode const& bc, bool show)
{
    //remove any current selection
    primary_selected.clear();
    secondary_selected.clear();

    //remove old bars
    for (std::vector<std::list<PersistenceBar*>>::iterator it = bars.begin(); it != bars.end(); ++it) {
        while (!it->empty()) {
            removeItem(it->back());
            delete it->back();
            it->pop_back();
        }
    }

    //draw new bars
    draw_barcode(bc, show);
}

//computes an endpoint of a bar in the barcode
std::pair<double, double> SliceDiagram::compute_endpoint(double coordinate, unsigned offset)
{
    //the commented code is for keeping the spacing constant in data units
    //as is, the spacing is constant in pixel units

    //difference in offset between consecutive bars (pixel units)
    //int old_step_size = config_params->persistenceBarWidth + config_params->persistenceBarSpace;

    //this is expressed in terms of the original window
    //double dx = data_xmax - data_xmin;
    //double dy = data_ymax - data_ymin;
    //double dx0 = original_xmax - original_xmin;
    //double dy0 = original_ymax - original_ymin;
    //double offset_angle = PI / 2;
    //if (!line_vert) {
    //    offset_angle = PI / 2 + atan(line_slope);
    //}
    //double conversion = sqrt((pow(cos(offset_angle) / dx, 2.0) + pow(sin(offset_angle) / dy, 2.0)) / (pow(cos(offset_angle) / dx0, 2.0) + pow(sin(offset_angle) / dy0, 2.0)));

    //int step_size = old_step_size * conversion;

    int step_size = config_params->persistenceBarWidth + config_params->persistenceBarSpace;
    //compute x and y relative to slice line (pixel units)
    double x = 0;
    double y = 0;
    if (line_vert) {
        if (coordinate == std::numeric_limits<double>::infinity()) { //should change to upper bound of view
            //choose y outside of the viewable window
            //if coordinate is much larger than max_coord, then coord should be infinite, but is not due to numerical error
            y = view_length;
        } else {
            //find y along the line
            y = (coordinate - dist_to_origin) * scale_y;
        }

        //offset from slice line
        x = -1 * (int)(step_size * offset);
    } else {
        double angle = atan(line_slope); //angle (data)      NOTE: it would be slightly more efficient to only compute this once per barcode update

        if (coordinate == std::numeric_limits<double>::infinity() || coordinate * std::min(scale_x, scale_y) > pow(10.0, 7.0)) {
            //set coordinate so that it will be outside the viewable window
            //the finite cutoff seems to patch over the issue with phantom barcodes-not sure why
            coordinate = dist_to_origin + view_length / std::min(scale_x, scale_y);
        }

        //find (x,y) along the line
        x = (coordinate - dist_to_origin) * cos(angle) * scale_x;
        y = (coordinate - dist_to_origin) * sin(angle) * scale_y;

        //offset from slice line
        double pixel_angle = atan(line_slope * scale_y / scale_x); //angle (pixels)    NOTE: it would be slightly more efficient to only compute this once per barcode update
        x -= step_size * offset * sin(pixel_angle);
        y += step_size * offset * cos(pixel_angle);
    }

    //adjust for position of slice line
    if (control_dot_moved) {
        x += slice_line->pos().x();
        y += slice_line->pos().y();
    } else {
        x += dot_left->pos().x();
        y += dot_left->pos().y();
    }

    //is it a problem to draw barcodes outside the window?

    return std::pair<double, double>(x, y);
} //end compute_endpoint()

//highlight the specified bar, selected in the slice diagram, and propagate to the persistence diagram
void SliceDiagram::select_bar(PersistenceBar* clicked)
{
    //remove old selection
    clear_selection();

    //remember current selection
    unsigned index = clicked->get_index();
    primary_selected.push_back(index);

    //select all bars with the current index
    for (std::list<PersistenceBar*>::iterator it = bars[index].begin(); it != bars[index].end(); ++it)
        (*it)->select();

    //highlight part of slice line
    update_highlight();

    //highlight part of the persistence diagram
    emit persistence_bar_selected(index);
} //end select_bar()

//remove selection; if propagate, then deselect dot in the persistence diagram
void SliceDiagram::deselect_bar()
{
    //remove selection
    clear_selection();

    //remove highlighted portion of slice line
    highlight_line->hide();

    //remove highlighting from slice diagram
    emit persistence_bar_deselected();
} //end deselect_bar()

//highlight the specified class of bars, which has been selected externally (they become the primary selection)
void SliceDiagram::receive_bar_selection(std::vector<unsigned> indexes)
{
    //remove old selection
    clear_selection();

    //remember new selection
    primary_selected = indexes;

    //select all bars with the new indexes
    for (std::vector<unsigned>::iterator it = primary_selected.begin(); it != primary_selected.end(); ++it) {
        unsigned index = *it;
        for (std::list<PersistenceBar*>::iterator it = bars[index].begin(); it != bars[index].end(); ++it)
            (*it)->select();
    }

    //highlight part of slice line
    update_highlight();
} //end receive_bar_selection()

//secondary highlight, used for persistence dots that represent multiple classes of bars
void SliceDiagram::receive_bar_secondary_selection(std::vector<unsigned> indexes)
{
    //remember this selection
    secondary_selected = indexes;

    //select all bars with the new indexes
    for (std::vector<unsigned>::iterator it = secondary_selected.begin(); it != secondary_selected.end(); ++it) {
        unsigned index = *it;
        for (std::list<PersistenceBar*>::iterator it = bars[index].begin(); it != bars[index].end(); ++it)
            (*it)->select_secondary();
    }

    //update highlight
    update_highlight();
} //end receive_bar_secondary_selection()

//remove bar highlighting in response to external command
void SliceDiagram::receive_bar_deselection()
{
    //remove selection
    clear_selection();

    //remove highlighted portion of slice line
    highlight_line->hide();
} //end receive_bar_deselection()

//unselect all bars
void SliceDiagram::clear_selection()
{
    //clear primary selection
    for (std::vector<unsigned>::iterator it = primary_selected.begin(); it != primary_selected.end(); ++it) {
        unsigned index = *it;
        for (std::list<PersistenceBar*>::iterator it = bars[index].begin(); it != bars[index].end(); ++it)
            (*it)->deselect();
    }
    primary_selected.clear();

    //clear secondary selection
    for (std::vector<unsigned>::iterator it = secondary_selected.begin(); it != secondary_selected.end(); ++it) {
        unsigned index = *it;
        for (std::list<PersistenceBar*>::iterator it = bars[index].begin(); it != bars[index].end(); ++it)
            (*it)->deselect();
    }
    secondary_selected.clear();
} //end clear_selection()

//highlights part of the slice line
void SliceDiagram::update_highlight()
{
    if (primary_selected.empty()) //then no highlighting
        return;

    //determine interval to be highlighted
    PersistenceBar* cur_bar = bars[primary_selected[0]].front();
    double start = cur_bar->get_start();
    double end = cur_bar->get_end();
    for (unsigned i = 1; i < primary_selected.size(); i++) {
        PersistenceBar* cur_bar = bars[primary_selected[i]].front();
        if (cur_bar->get_start() < start)
            start = cur_bar->get_start();
        if (cur_bar->get_end() > end)
            end = cur_bar->get_end();
    }
    for (unsigned i = 0; i < secondary_selected.size(); i++) {
        PersistenceBar* cur_bar = bars[secondary_selected[i]].front();
        if (cur_bar->get_start() < start)
            start = cur_bar->get_start();
        if (cur_bar->get_end() > end)
            end = cur_bar->get_end();
    }

    //highlight the interval
    std::pair<double, double> p1 = compute_endpoint(start, 0);
    std::pair<double, double> p2 = compute_endpoint(end, 0);

    highlight_line->setLine(p1.first, p1.second, p2.first, p2.second);
    highlight_line->show();
} //end update_highlight()

//if "show" is true, then xi_0 support points are drawn; otherwise, they are hidden
void SliceDiagram::toggle_xi0_points(bool show)
{
    for (std::vector<QGraphicsEllipseItem*>::iterator it = xi0_dots.begin(); it != xi0_dots.end(); ++it)
        (*it)->setVisible(show);
}

//if "show" is true, then xi_1 support points are drawn; otherwise, they are hidden
void SliceDiagram::toggle_xi1_points(bool show)
{
    for (std::vector<QGraphicsEllipseItem*>::iterator it = xi1_dots.begin(); it != xi1_dots.end(); ++it)
        (*it)->setVisible(show);
}

//if "show" is true, then xi_2 support points are drawn; otherwise, they are hidden
void SliceDiagram::toggle_xi2_points(bool show)
{
    for (std::vector<QGraphicsEllipseItem*>::iterator it = xi2_dots.begin(); it != xi2_dots.end(); ++it)
        (*it)->setVisible(show);
}

//if "show" is true, then barcode is drawn; otherwise, it is hidden
void SliceDiagram::toggle_barcode(bool show)
{
    for (std::vector<std::list<PersistenceBar*>>::iterator it1 = bars.begin(); it1 != bars.end(); ++it1)
        for (std::list<PersistenceBar*>::iterator it2 = it1->begin(); it2 != it1->end(); ++it2)
            (*it2)->setVisible(show);
}

//sets normalized coordinates or default coordinates
void SliceDiagram::set_normalized_coords(bool toggle)
{
    normalized_coords = toggle;
}

//gets the length of the slice, for scaling the persistence diagram
double SliceDiagram::get_slice_length()
{
    //handle edge case where the line is vertical but on the left or right edge
    //in this case, the control dots can move up or down, but the line is drawn over the entire edge
    if (line_vert) {
        return slice_line->get_box_ymax();
    }

    //if we get here, then the line is not vertical, and the length is simply the distance
    //between the control dots
    double dx = slice_line->get_right_pt_x() - slice_line->pos().x();
    double dy = slice_line->get_right_pt_y() - slice_line->pos().y();

    if (dx < 0 || dy < 0) { //this happens if the line is not visible in the window
        return 0;
    }
    return sqrt(dx * dx + dy * dy);
}

//gets the number of pixels per unit, for the persistence diagram
double SliceDiagram::get_pd_scale()
{
    double angle = PI / 2; //default, for vertical line
    if (!line_vert)
        angle = atan(line_slope * scale_y / scale_x); //line_slope is in data units, so first convert to pixel units
    double sine = sin(angle);
    double cosine = cos(angle);
    double denominator = sqrt(scale_x * scale_x * sine * sine + scale_y * scale_y * cosine * cosine);
    return scale_x * scale_y / denominator;
}

//draws labels on top of white rectangles, so they don't get obscured by other graphics
void SliceDiagram::redraw_labels()
{

    int text_padding = 15; //pixels
    data_xmin_text->setPos(data_xmin_text->boundingRect().width() / (-2), -1 * text_padding);
    data_xmax_text->setPos(diagram_width - data_xmax_text->boundingRect().width() / 2, -1 * text_padding);
    data_ymin_text->setPos(-1 * text_padding - data_ymin_text->boundingRect().width(), data_ymin_text->boundingRect().height() / 2);
    data_ymax_text->setPos(-1 * text_padding - data_ymax_text->boundingRect().width(), diagram_height + data_ymax_text->boundingRect().height() / 2);

    x_label->setPos((diagram_width - x_label->boundingRect().width()) / 2, -1 * text_padding);
    y_label->setPos(-1 * text_padding - y_label->boundingRect().height(), (diagram_height - y_label->boundingRect().width()) / 2);

    std::ostringstream s_xmin;
    s_xmin.precision(4);
    // fix to sometimes showing "-0"
    s_xmin << (data_xmin == 0 ? 0 : data_xmin * xrev_sign);
    data_xmin_text->setText(QString(s_xmin.str().data()));

    rect1->setRect(0, 0, data_xmin_text->sceneBoundingRect().width(), data_xmin_text->sceneBoundingRect().height());
    rect1->setPos(data_xmin_text->pos().x(), data_xmin_text->pos().y() - data_xmin_text->boundingRect().height());

    std::ostringstream s_ymin;
    s_ymin.precision(4);
    s_ymin << (data_ymin == 0 ? 0 : data_ymin * yrev_sign);
    data_ymin_text->setText(QString(s_ymin.str().data()));

    rect2->setRect(0, 0, data_ymin_text->sceneBoundingRect().width(), data_ymin_text->sceneBoundingRect().height());
    rect2->setPos(data_ymin_text->pos().x(), data_ymin_text->pos().y() - data_ymin_text->boundingRect().height());

    std::ostringstream s_xmax;
    s_xmax.precision(4);
    s_xmax << (data_xmax == 0 ? 0 : data_xmax * xrev_sign);
    data_xmax_text->setText(QString(s_xmax.str().data()));

    rect3->setRect(0, 0, data_xmax_text->sceneBoundingRect().width(), data_xmax_text->sceneBoundingRect().height());
    rect3->setPos(data_xmax_text->pos().x(), data_xmax_text->pos().y() - data_xmax_text->boundingRect().height());

    std::ostringstream s_ymax;
    s_ymax.precision(4);
    s_ymax << (data_ymax == 0 ? 0 : data_ymax * yrev_sign);
    data_ymax_text->setText(QString(s_ymax.str().data()));

    rect4->setRect(0, 0, data_ymax_text->sceneBoundingRect().width(), data_ymax_text->sceneBoundingRect().height());
    rect4->setPos(data_ymax_text->pos().x(), data_ymax_text->pos().y() - data_ymax_text->boundingRect().height());

    rect5->setRect(0, 0, x_label->boundingRect().width(), x_label->boundingRect().height());
    rect5->setPos(x_label->pos().x(), x_label->pos().y() - x_label->boundingRect().height());

    rect6->setRect(0, 0, y_label->boundingRect().height(), y_label->boundingRect().width());
    rect6->setPos(y_label->pos().x(), y_label->pos().y());
}
