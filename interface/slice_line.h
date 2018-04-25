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

#ifndef SLICE_LINE_H
#define SLICE_LINE_H

//forward declarations
struct ConfigParameters;
class ControlDot;
class SliceDiagram;

#include <QGraphicsItem>
#include <QPainter>
#include <QVariant>
#include <QtWidgets>

class SliceLine : public QGraphicsItem {
public:
    SliceLine(SliceDiagram* sd, ConfigParameters* params); //VisualizationWindow* vw);

    void setDots(ControlDot* left, ControlDot* right);

    QRectF boundingRect() const;
    QPainterPath shape() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

    QVariant itemChange(GraphicsItemChange change, const QVariant& value);

    void update_lb_endpoint(QPointF& newpos); //updates left-bottom endpoint; called by ControlDot on move event
    void update_rt_endpoint(QPointF& newpos); //updates right-top endpoint; called by ControlDot on move event

    double get_right_pt_x(); //gets x-coordinate of right-top endpoint (units: pixels)
    double get_right_pt_y(); //gets y-coordinate of right-top endpoint (units: pixels)

    double get_slope(); //gets the slope of the line
    bool is_vertical(); //true if the line is vertical, false otherwise

    void update_bounds(double data_width, double data_height, int padding); //updates the dimensions of the on-screen box in which this line is allowed to move
    void update_position(double xpos, double ypos, bool vert, double pixel_slope); //updates position of line; called by SliceDiagram in response to change in VisualizationWindow controls

    double get_data_xmax();
    double get_data_ymax();
    double get_box_xmax();
    double get_box_ymax();

    void set_visibility(bool visible);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
    double data_xmax, data_ymax; //pixel dimensions slightly greater than the largest bounds of the current window
    double box_xmax, box_ymax; //pixel dimensions of the on-screen box in which this line is allowed to move

    bool vertical; //true if the line is currently vertical
    double slope; //current slope of the line in PIXEL units

    bool pressed; //true when the line is clicked
    bool rotating; //true when a rotation is in progress
    bool update_lock; //true when the line is being moved as result of external input; to avoid update loops

    QPointF right_point; //this is the top/right endpoint of the line

    ControlDot* left_dot;
    ControlDot* right_dot;

    SliceDiagram* sdgm;
    ConfigParameters* config_params;

    void compute_right_point(); //sets correct position of right_point, given slope of line and position of left point
};

#endif // SLICE_LINE_H
