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

#ifndef PERSISTENCEDIAGRAM_H
#define PERSISTENCEDIAGRAM_H

//forward declarations
struct ConfigParameters;
class PersistenceDot;
class Barcode;

#include <QGraphicsScene>
#include <QtWidgets>

#include <vector>

class PersistenceDiagram : public QGraphicsScene {
    Q_OBJECT

public:
    PersistenceDiagram(ConfigParameters* params, QObject* parent = 0);

    void create_diagram(); //simply creates all objects; resize_diagram() handles positioning of objects
    void resize_diagram(double slice_length, double diagram_scale); //resizes diagram to fill the QGraphicsView; called after every window resize
    void resize_diagram();

    void set_barcode(const Barcode& bc); //sets the barcode
    void draw_dots(); //creates and draws persistence dots at the correct locations, using current parameters
    void redraw_dots(); //redraws persistence dots; e.g. used after a change in parameters

    void update_diagram(double slice_length, double diagram_scale, const Barcode& bc); //updates the diagram after a change in the slice line
    void update_diagram(double slice_length_pix, double diagram_scale, double slice_dist_dat, bool is_visible, const Barcode& bc); //updates the diagram after a change in the window bounds

    void select_dot(PersistenceDot* clicked); //highlight the specified dot, selected in the persistence diagram, and propagate to the slice diagram
    void deselect_dot(); //remove selection and propagate to the slice diagram

    void receive_parameter_change(); //updates the diagram after a change in the configuration parameters
    void reset(); // clears up data structures and variable values

public slots:
    void receive_dot_selection(unsigned index); //highlight the specified dot, which has been selected externally
    void receive_dot_deselection(); //remove dot highlighting in response to external command

signals:
    void persistence_dot_selected(std::vector<unsigned> indexes);
    void persistence_dot_secondary_selection(std::vector<unsigned> indexes);
    void persistence_dot_deselected();

private:
    //graphics items
    ConfigParameters* config_params;

    QGraphicsRectItem* bounding_rect;
    QGraphicsLineItem* diag_line;
    QGraphicsLineItem* blue_line;
    QGraphicsLineItem* h_line;
    QGraphicsLineItem* v_line;
    QGraphicsLineItem* left_v_line;
    QGraphicsLineItem* top_left_hline; //the short lines circumscribing the counters on the left
    QGraphicsLineItem* top_left_vline;
    QGraphicsLineItem* bottom_left_hline;
    QGraphicsLineItem* bottom_left_vline;

    QGraphicsSimpleTextItem* inf_text;
    QGraphicsSimpleTextItem* lt_inf_text;
    QGraphicsSimpleTextItem* inf_count_text;
    QGraphicsSimpleTextItem* lt_inf_count_text;
    QGraphicsSimpleTextItem* gt_neg_inf_text;
    QGraphicsSimpleTextItem* gt_neg_inf_count_text; //barcodes that die before the visible window

    QGraphicsSimpleTextItem* big_nonessential_count_text; //barcodes that are born before the visible window, die after, but not essential
    QGraphicsSimpleTextItem* big_essential_count_text; //essential cycles that are born before the visible window

    QGraphicsSimpleTextItem* file_text;
    QGraphicsSimpleTextItem* dim_text;

    std::vector<PersistenceDot*> all_dots; //pointers to all dots (one pointer per dot)
    std::vector<PersistenceDot*> dots_by_bc_index; //pointer to dots for each "multibar" index -- used for highlighting dots
    PersistenceDot* selected;

    //parameters
    double scale; //scale of the persistence diagram
    int diagram_size; //width and height of the persistence diagram (the square part), in pixels
    double line_size; //width and height of the blue line; i.e. length of slice line divided by sqrt(2)
    int inf_dot_vpos; //vertical position (y-coordinate) of dots representing essential cycles
    int lt_inf_dot_vpos; //vertical position (y-coordinate) of dots representing non-infinite pairs above the diagram
    int gt_neg_inf_dot_hpos;
    double dist_to_origin; //the distance from the left bottom dot in the diagram to the origin in the coordinate system defined in the paper, expressed in data units
    QPointF diagram_origin; //the position, in pixels, of the bottom left corner of the persistence diagram; used for centering graphics items

    const Barcode* barcode; //reference to the barcode displayed in the persistence diagram

    QString* filename; //filename, to print on the screen
    int dim; //dimension of homology, to print on the screen
};

#endif // PERSISTENCEDIAGRAM_H
