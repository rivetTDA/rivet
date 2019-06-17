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

//TODO: revert back to original convention where length in pixels of blue line matches
//length in pixels of line in slice diagram
//this seems almost done-should check and clean up (in particular, get rid of max_line_length)

#include "persistence_diagram.h"

#include "config_parameters.h"
#include "dcel/barcode.h"
#include "persistence_dot.h"

//#include <QDebug>
#include <QGraphicsView>

#include <cmath> //for std::round()
#include <iostream> //only needed for debugging
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <unordered_set>
#include <utility> // for std::pair
PersistenceDiagram::PersistenceDiagram(ConfigParameters* params, QObject* parent)
    : QGraphicsScene(parent)
    , config_params(params)
    , selected(NULL)
    , barcode()
{
    setItemIndexMethod(NoIndex); //not sure why, but this seems to fix the dot update issue (#7 in the issue tracker)
}

void PersistenceDiagram::reset(ConfigParameters* params, QObject* parent)
{
    clear();
    all_dots.clear();
    dots_by_bc_index.clear();
    selected = NULL;
    barcode = new Barcode();

    // PersistenceDiagram(params, parent);
}

//simply creates all objects; resize_diagram() handles positioning of objects
void PersistenceDiagram::create_diagram()
{
    //define pens and brushes
    QPen grayPen(QBrush(Qt::darkGray), 2, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin);
    QPen thinPen(QBrush(Qt::darkGray), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen slicePen(QBrush(config_params->sliceLineColor), config_params->sliceLineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QBrush purpleBrush(QColor(config_params->persistenceColor.rgb()));

    //create persistence diagram structure
    bounding_rect = addRect(QRectF(), grayPen);
    diag_line = addLine(QLineF(), thinPen);
    blue_line = addLine(QLineF(), slicePen);

    h_line = addLine(QLineF(), grayPen);
    v_line = addLine(QLineF(), grayPen);
    left_v_line = addLine(QLineF(), grayPen);
    top_left_hline = addLine(QLineF(), grayPen);
    top_left_vline = addLine(QLineF(), grayPen);
    bottom_left_hline = addLine(QLineF(), grayPen);
    bottom_left_vline = addLine(QLineF(), grayPen);

    //TODO: change the position of the text objects
    //create text objects
    inf_text = addSimpleText("inf");
    inf_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    inf_text->setFont(config_params->diagramFont);

    lt_inf_text = addSimpleText("<inf");
    lt_inf_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    lt_inf_text->setFont(config_params->diagramFont);

    //TODO: rotate this text 90 degrees

    gt_neg_inf_text = addSimpleText("-inf<");
    gt_neg_inf_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    gt_neg_inf_text->setFont(config_params->diagramFont);

    inf_count_text = addSimpleText("0");
    inf_count_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    inf_count_text->setBrush(purpleBrush);
    inf_count_text->setFont(config_params->diagramFont);

    lt_inf_count_text = addSimpleText("0");
    lt_inf_count_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    lt_inf_count_text->setBrush(purpleBrush);
    lt_inf_count_text->setFont(config_params->diagramFont);

    gt_neg_inf_count_text = addSimpleText("0");
    gt_neg_inf_count_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    gt_neg_inf_count_text->setBrush(purpleBrush);
    gt_neg_inf_count_text->setFont(config_params->diagramFont);

    big_nonessential_count_text = addSimpleText("0");
    big_nonessential_count_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    big_nonessential_count_text->setBrush(purpleBrush);
    big_nonessential_count_text->setFont(config_params->diagramFont);

    big_essential_count_text = addSimpleText("0");
    big_essential_count_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    big_essential_count_text->setBrush(purpleBrush);
    big_essential_count_text->setFont(config_params->diagramFont);

} //end create_diagram()

//resizes diagram to fill the QGraphicsView; called after every window resize
void PersistenceDiagram::resize_diagram(double slice_length, double diagram_scale)
{

    line_size = slice_length / sqrt(2); //divide by sqrt(2) because the line is drawn at a 45-degree angle
    scale = diagram_scale / sqrt(2); //similarly, divide by sqrt(2)

    resize_diagram();
}

//resizes diagram to fill the QGraphicsView; called after every window resize
void PersistenceDiagram::resize_diagram()
{
    //parameters
    int scene_padding = 10; //pixels (minimum white space between diagram objects and edge of viewing window)
    int text_padding = 4; //pixels (white space on each side of text items)
    int number_space = 30; //pixels (horizontal space reserved for counts of points above diagram)

    //get dimensions of the QGraphicsView
    QList<QGraphicsView*> view_list = views();
    int view_width = view_list[0]->width();
    int view_height = view_list[0]->height();

    //the space occupied by the labels/counters on each side of the main window
    int space_above = 2 * lt_inf_text->boundingRect().height() + 4 * text_padding + scene_padding;
    int space_below = gt_neg_inf_count_text->boundingRect().height() + 2 * text_padding + scene_padding;
    int space_left = gt_neg_inf_text->boundingRect().width() + 2 * text_padding + scene_padding;
    int space_right = number_space + scene_padding;

    //the (fixed) sizes of the labels in the directions which limit the min size of the diagram
    //these shouldn't ever be the limiting factors, due to the minimum size constraint imposed in the
    //visualizationwindow.ui file, but included just for safety
    int min_width = lt_inf_text->boundingRect().width() + 2 * text_padding;
    int min_height = gt_neg_inf_count_text->boundingRect().height() + 2 * text_padding;

    //compute size of main diagram (not including bars/counters on left and top)
    int available_width = std::max(view_width - (space_left + space_right), min_width + space_left + space_right);
    int available_height = std::max(view_height - (space_above + space_below), min_height + space_left + space_right);

    diagram_size = std::min(available_height, available_width);

    //the bottom left point of the main diagram, chosen so that the entire diagram (including bars/counters) is centered (i.e., equal whitespace above/below and left/right)
    diagram_origin = QPointF((space_right - space_left) / 2, (space_below - space_above) / 2);

    //resize frame
    bounding_rect->setRect(0, 0, diagram_size, diagram_size);
    diag_line->setLine(0, 0, diagram_size, diagram_size);

    blue_line->setLine(0, 0, line_size, line_size);

    //draw lines delineating counters and labels
    int v_space = lt_inf_text->boundingRect().height() + 2 * text_padding;
    int h_space = -gt_neg_inf_text->boundingRect().width();

    h_line->setLine(-space_left + scene_padding, diagram_size + v_space, diagram_size + space_right - scene_padding, diagram_size + v_space);
    v_line->setLine(diagram_size, diagram_size + text_padding, diagram_size, diagram_size + space_above - scene_padding);
    top_left_hline->setLine(-space_left + scene_padding, diagram_size, 0, diagram_size);
    top_left_vline->setLine(0, diagram_size, 0, diagram_size + space_above - scene_padding);
    bottom_left_hline->setLine(-space_left + scene_padding, 0, 0, 0);
    bottom_left_vline->setLine(0, 0, 0, -space_below + scene_padding);

    //translate everything to origin
    bounding_rect->setPos(diagram_origin);
    diag_line->setPos(diagram_origin);
    blue_line->setPos(diagram_origin);

    std::unordered_set<QGraphicsLineItem*> graphics = { h_line, v_line, top_left_hline, top_left_vline, bottom_left_hline, bottom_left_vline };
    for (auto g : graphics) {
        g->setPos(diagram_origin);
    }

    //remove old dots
    selected = NULL; //remove any current selection
    dots_by_bc_index.clear(); //clear index data
    while (!all_dots.empty()) //delete all dots
    {
        removeItem(all_dots.back());
        all_dots.pop_back();
    }

    //draw new dots
    lt_inf_dot_vpos = diagram_size + v_space / 2;
    inf_dot_vpos = lt_inf_dot_vpos + v_space;
    gt_neg_inf_dot_hpos = h_space / 2;
    if (barcode != NULL) {
        draw_dots();
    }

    //move text items
    double inf_text_vpos = diagram_size + v_space + text_padding + inf_text->boundingRect().height();
    double lt_inf_text_vpos = diagram_size + text_padding + lt_inf_text->boundingRect().height();

    inf_text->setPos(-inf_text->boundingRect().width() - text_padding + diagram_size / 2, inf_text_vpos);
    lt_inf_text->setPos(-lt_inf_text->boundingRect().width() - text_padding + diagram_size / 2, lt_inf_text_vpos);

    gt_neg_inf_text->setPos(-gt_neg_inf_text->boundingRect().width(), diagram_size / 2);

    gt_neg_inf_text->setTransformOriginPoint(QPointF(0, -gt_neg_inf_text->boundingRect().height() / 2));
    gt_neg_inf_text->setRotation(-90);

    inf_count_text->setPos(diagram_size + text_padding, inf_text_vpos);
    lt_inf_count_text->setPos(diagram_size + text_padding, lt_inf_text_vpos);

    gt_neg_inf_count_text->setPos(-gt_neg_inf_text->boundingRect().width() + text_padding, -text_padding);

    big_nonessential_count_text->setPos(-gt_neg_inf_text->boundingRect().width() + text_padding, lt_inf_text_vpos);
    big_essential_count_text->setPos(-gt_neg_inf_text->boundingRect().width() + text_padding, inf_text_vpos);

    //translate to origin
    std::unordered_set<QGraphicsSimpleTextItem*> texts = { inf_text, lt_inf_text, gt_neg_inf_text, inf_count_text, lt_inf_count_text, gt_neg_inf_count_text, big_nonessential_count_text, big_essential_count_text };
    for (auto g : texts) {
        g->setPos(g->pos() + diagram_origin);
    }

    //set scene rectangle (necessary to prevent auto-scrolling)
    int scene_rect_x = -space_left + scene_padding + diagram_origin.x();
    int scene_rect_y = -space_below + scene_padding + diagram_origin.y();
    int scene_rect_w = diagram_size + space_right + space_left - 2 * scene_padding;
    int scene_rect_h = diagram_size + space_above + space_below - 2 * scene_padding;
    setSceneRect(scene_rect_x, scene_rect_y, scene_rect_w, scene_rect_h);

} //end resize_diagram()

//sets the barcode
void PersistenceDiagram::set_barcode(const Barcode& bc)
{
    barcode = &bc;
}

//creates and draws persistence dots at the correct locations, using current parameters

void PersistenceDiagram::draw_dots()
{

    //counters
    unsigned bc_index = 0;
    unsigned num_big_cycles = 0; //nonessential cycle born after
    unsigned num_big_points = 0;
    unsigned num_short_cycles = 0; //cycles that die before the visible portion of the diagram
    unsigned num_long_nonessential_points = 0; //nonessential cycles which span the entire line
    unsigned num_long_essential_points = 0; //essential cycles which span the entire line

    //maps to help combine dots in the upper horizontal strips
    //  in each, the key is the x-position relative to origin, rounded to the nearest pixel

    std::map<int, PersistenceDot*> lt_inf_dot_map; //includes nonessential cycles with visible birth, but invisible death
    std::map<int, PersistenceDot*> inf_dot_map; //includes all essential cycles

    //same, but for the vertical strip, key is y-position relative to origin
    std::map<int, PersistenceDot*> gt_neg_inf_dot_map; //includes nonessential cycles with invisible birth, but visible death

    //map for long nonessential points; the key is the pair of coordinates
    std::map<std::pair<int, int>, PersistenceDot*> long_nonessential_dot_map; //includes nonessential cycles with invisible birth and death

    double radius_scale = diagram_size / 600.0; //used to change the radius in accordance with a change in size of application window
    //"magic number"-probably bad

    double eps = pow(10.0, -3.0); //"wiggle room" to account for rounding errors
    //dots will be shown on the persistence diagram if they lie in the epsilon-neighborhood of the diagram
    //otherwise they will appear on one of the counters

    //first calculate the positions of the dots relative to the origin of the diagram

    //loop over all bars
    for (std::multiset<MultiBar>::iterator it = barcode->begin(); it != barcode->end(); ++it) {

        if (it->death == std::numeric_limits<double>::infinity()) //essential cycle (visualized in the upper horizontal strip of the persistence diagram)
        {
            double birth = it->birth;
            //check to see if a dot already exists in its position
            int x_pixel = std::round(birth * scale);

            std::map<int, PersistenceDot*>::iterator dot_it = inf_dot_map.find(x_pixel);

            if (dot_it == inf_dot_map.end()) //then no such dot exists, so create a new dot
            {
                //create dot object
                PersistenceDot* dot = new PersistenceDot(this, config_params, it->birth, it->death, it->multiplicity, config_params->persistenceDotRadius * sqrt((double)(it->multiplicity) * radius_scale), bc_index);
                dot->setToolTip(QString::number(it->multiplicity));
                addItem(dot);
                all_dots.push_back(dot);
                dots_by_bc_index.push_back(dot);
                inf_dot_map.insert(std::pair<int, PersistenceDot*>(x_pixel, dot));

                //position dot properly
                if (birth - dist_to_origin > (diagram_size / scale) + eps) //it is an essential cycle born too late
                {
                    num_big_cycles += it->multiplicity;
                    inf_dot_map.insert(std::pair<int, PersistenceDot*>(x_pixel, dot));
                    dot->setVisible(false);
                } else if (birth < dist_to_origin - eps) //it is an essential cycle born too early
                {
                    num_long_essential_points += it->multiplicity;
                    gt_neg_inf_dot_map.insert(std::pair<int, PersistenceDot*>(x_pixel, dot));
                    dot->setVisible(false);
                }

                else //the birth is visible
                {
                    dot->setPos((birth - dist_to_origin) * scale, inf_dot_vpos);
                }
            } else //then such dot exists, so add to its multiplicity
            {
                PersistenceDot* dot = dot_it->second;
                dot->incr_multiplicity(it->multiplicity);
                dot->setToolTip(QString::number(dot->get_multiplicity()));
                dot->set_radius(config_params->persistenceDotRadius * sqrt(dot->get_multiplicity() * radius_scale));
                dots_by_bc_index.push_back(dot); //necessary for dot lookup when persistence bars are highlighted
                dot->add_index(bc_index);

                if (birth - dist_to_origin > (diagram_size / scale) + eps) //it is an essential cycle born too late
                {
                    num_big_cycles += it->multiplicity;
                } else if (birth < dist_to_origin - eps) //it is an essential cycle born too early
                {
                    num_long_essential_points += it->multiplicity;
                }
            }
        } else //finite bar
        {
            double birth = it->birth;
            double death = it->death;

            if (birth < dist_to_origin - eps && death - dist_to_origin < (diagram_size / scale) + eps) //dot is in the gt_neg_inf strip
            { //born too early, but does not die too late (might die too early)

                int y_pixel = std::round(death * scale);
                std::map<int, PersistenceDot*>::iterator dot_it = gt_neg_inf_dot_map.find(y_pixel);
                if (dot_it == gt_neg_inf_dot_map.end()) //then no such dot exists, so create a new dot
                {
                    //create dot object
                    PersistenceDot* dot = new PersistenceDot(this, config_params, birth, death, it->multiplicity, config_params->persistenceDotRadius * sqrt(it->multiplicity) * radius_scale, bc_index);
                    dot->setToolTip(QString::number(it->multiplicity));
                    addItem(dot);
                    all_dots.push_back(dot);
                    dots_by_bc_index.push_back(dot);
                    gt_neg_inf_dot_map.insert(std::pair<int, PersistenceDot*>(y_pixel, dot));

                    if (death < dist_to_origin - eps) //dies too early
                    {
                        num_short_cycles += it->multiplicity;
                        dot->setVisible(false);
                    } else //then dot will be visible in the left strip
                    {
                        dot->setPos(gt_neg_inf_dot_hpos, scale * (death - dist_to_origin));
                    }

                } else //then such dot exists, so add to its multiplicity
                {
                    PersistenceDot* dot = dot_it->second;
                    dot->incr_multiplicity(it->multiplicity);
                    dot->setToolTip(QString::number(dot->get_multiplicity()));
                    dot->set_radius(config_params->persistenceDotRadius * sqrt(dot->get_multiplicity() * radius_scale));
                    dots_by_bc_index.push_back(dot); //necessary for dot lookup when persistence bars are highlighted
                    dot->add_index(bc_index);

                    if (death < dist_to_origin - eps) //dies too early
                    {
                        num_short_cycles += it->multiplicity;
                    }
                }

            }

            else if (birth > dist_to_origin - eps && death - dist_to_origin > (diagram_size / scale) + eps) //dies too late and not born too early (might be born too late)
            {

                //check to see if a dot already exists in its position
                int x_pixel = std::round(birth * scale);
                std::map<int, PersistenceDot*>::iterator dot_it = lt_inf_dot_map.find(x_pixel);

                if (dot_it == lt_inf_dot_map.end()) //then no such dot exists, so create a new dot
                {
                    //create dot object
                    PersistenceDot* dot = new PersistenceDot(this, config_params, birth, death, it->multiplicity, config_params->persistenceDotRadius * sqrt(it->multiplicity) * radius_scale, bc_index);
                    dot->setToolTip(QString::number(it->multiplicity));
                    addItem(dot);
                    all_dots.push_back(dot);
                    dots_by_bc_index.push_back(dot);
                    lt_inf_dot_map.insert(std::pair<int, PersistenceDot*>(x_pixel, dot));

                    if (birth - dist_to_origin > (diagram_size / scale) + eps) //born too late
                    {
                        num_big_points += it->multiplicity;
                        dot->setVisible(false);
                    } else //then dot will be visible in the top <inf strip
                    {
                        dot->setPos((birth - dist_to_origin) * scale, lt_inf_dot_vpos);
                    }

                } else //then such dot exists, so add to its multiplicity
                {
                    PersistenceDot* dot = dot_it->second;
                    dot->incr_multiplicity(it->multiplicity);
                    dot->setToolTip(QString::number(dot->get_multiplicity()));
                    dot->set_radius(config_params->persistenceDotRadius * sqrt(dot->get_multiplicity() * radius_scale));
                    dots_by_bc_index.push_back(dot); //necessary for dot lookup when persistence bars are highlighted
                    dot->add_index(bc_index);

                    if (birth - dist_to_origin > (diagram_size / scale) + eps) //born too late
                    {
                        num_big_points += it->multiplicity;
                    }
                }
            } else if (birth < dist_to_origin - eps && death - dist_to_origin > (diagram_size / scale) + eps) //born too early, dies too late
            { //dot is a long nonessential cycle, and contributes to the counter at the intersection of lt_inf and gt_neg_inf

                int x_pixel = std::round(birth * scale);
                int y_pixel = std::round(death * scale);
                std::pair<int, int> coords = std::pair<int, int>(x_pixel, y_pixel);
                std::map<std::pair<int, int>, PersistenceDot*>::iterator dot_it = long_nonessential_dot_map.find(coords);
                if (dot_it == long_nonessential_dot_map.end()) //then no such dot exists, so create a new dot
                {
                    //create dot object
                    PersistenceDot* dot = new PersistenceDot(this, config_params, birth, death, it->multiplicity, config_params->persistenceDotRadius * sqrt(it->multiplicity) * radius_scale, bc_index);
                    dot->setToolTip(QString::number(it->multiplicity));
                    addItem(dot);
                    all_dots.push_back(dot);
                    dots_by_bc_index.push_back(dot);
                    long_nonessential_dot_map.insert(std::pair<std::pair<int, int>, PersistenceDot*>(coords, dot));
                    num_long_nonessential_points += it->multiplicity;
                    dot->setVisible(false);
                } else //then the dot exists
                {
                    PersistenceDot* dot = dot_it->second;
                    dot->incr_multiplicity(it->multiplicity);
                    dot->setToolTip(QString::number(dot->get_multiplicity()));
                    dot->set_radius(config_params->persistenceDotRadius * sqrt(dot->get_multiplicity() * radius_scale));
                    dots_by_bc_index.push_back(dot); //necessary for dot lookup when persistence bars are highlighted
                    dot->add_index(bc_index);
                    num_long_nonessential_points += it->multiplicity;
                }

            }

            else //born in time, dies in time, visible in main portion of diagram
            {
                //create dot object
                PersistenceDot* dot = new PersistenceDot(this, config_params, birth, death, it->multiplicity, config_params->persistenceDotRadius * sqrt(it->multiplicity) * radius_scale, bc_index);
                dot->setToolTip(QString::number(it->multiplicity));
                addItem(dot);
                all_dots.push_back(dot);
                dots_by_bc_index.push_back(dot);

                //position dot properly
                dot->setPos((birth - dist_to_origin) * scale, (death - dist_to_origin) * scale);
            }
        }

        bc_index++; //don't forget to increment the barcode index
    }

    //the dot positions are now correct relative to the origin

    //translate the dots so they are in the correct absolute position
    //TODO: would it be better to do this above, rather than make a separate steP?
    for (auto dot : all_dots) {
        if (dot->isVisible()) {
            dot->setPos(dot->pos() + diagram_origin);
        }
    }

    //draw counts
    std::ostringstream scyc;
    scyc << num_big_cycles;
    inf_count_text->setText(QString(scyc.str().data()));

    std::ostringstream spts;
    spts << num_big_points;
    lt_inf_count_text->setText(QString(spts.str().data()));

    std::ostringstream shortcyc;
    shortcyc << num_short_cycles;
    gt_neg_inf_count_text->setText(QString(shortcyc.str().data()));

    std::ostringstream long_noness;
    long_noness << num_long_nonessential_points;
    big_nonessential_count_text->setText(QString(long_noness.str().data()));

    std::ostringstream long_ess;
    long_ess << num_long_essential_points;
    big_essential_count_text->setText(QString(long_ess.str().data()));

} //end draw_dots()

//redraws persistence dots; e.g. used after a change in parameters
void PersistenceDiagram::redraw_dots()
{
    for (std::vector<PersistenceDot*>::iterator it = all_dots.begin(); it != all_dots.end(); ++it) {
        PersistenceDot* dot = *it;
        dot->set_radius(config_params->persistenceDotRadius * sqrt(dot->get_multiplicity()));
        //NOTE: PersistenceDot::set_radius() also calls QGraphicsItem::update() to redraw the dots
    }
} //void redraw_dots()

//updates the diagram after a change in the slice line
void PersistenceDiagram::update_diagram(double slice_length, double diagram_scale, const Barcode& bc)
{
    //update parameters
    line_size = slice_length / sqrt(2); //divide by sqrt(2) because the line is drawn at a 45-degree angle
    scale = diagram_scale / sqrt(2); //similarly, divide by sqrt(2)
    barcode = &bc;

    blue_line->setLine(0, 0, line_size, line_size);

    //remove old dots
    selected = NULL; //remove any current selection
    dots_by_bc_index.clear(); //clear index data
    while (!all_dots.empty()) //delete all dots
    {
        removeItem(all_dots.back());
        all_dots.pop_back();
    }

    //draw new dots
    draw_dots();
} //end update_diagram()

//updates only the slice length and diagram scale, called after a change in window bounds
void PersistenceDiagram::update_diagram(double slice_length_pix, double diagram_scale, double slice_dist_dat, bool is_visible, const Barcode& bc)
{

    barcode = &bc;
    //update parameters

    if (is_visible) {
        line_size = slice_length_pix / sqrt(2); //divide by sqrt(2) because the line is drawn at a 45-degree angle
    } else {
        line_size=0;
    }
    dist_to_origin = slice_dist_dat;
    scale = diagram_scale / sqrt(2); //similarly to line_size, divide by sqrt(2)
    blue_line->setLine(0, 0, line_size, line_size);

    //remove old dots
    selected = NULL; //remove any current selection
    dots_by_bc_index.clear(); //clear index data
    while (!all_dots.empty()) //delete all dots
    {
        removeItem(all_dots.back());
        all_dots.pop_back();
    }

    //draw new dots

    draw_dots();
    //note it still makes sense to draw the dots even if the line is not visible
    //the only difference is that no portion of the diagonal line will be shaded blue

} //end update_diagram()

//highlight the specified dot, selected in the persistence diagram, and propagate to the slice diagram
void PersistenceDiagram::select_dot(PersistenceDot* clicked)
{
    //remove old selection
    if (selected != NULL && clicked != selected)
        selected->deselect();

    //remember current selection
    selected = clicked;

    //highlight part of the persistence diagram
    emit persistence_dot_selected(clicked->get_indexes());
} //end select_dot()

//remove selection; if propagate, then deselect bar in the slice diagram
void PersistenceDiagram::deselect_dot()
{
    //remove selection
    if (selected != NULL) {
        selected->deselect();
        selected = NULL;
    }

    //remove highlighting from slice diagram
    emit persistence_dot_deselected();
} //end deselect_dot()

//highlight the specified dot, which has been selected externally
// index refers to the "barcode" index
void PersistenceDiagram::receive_dot_selection(unsigned index)
{
    //remove old selection
    if (selected != NULL && dots_by_bc_index[index] != selected)
        selected->deselect();

    //remember current selection
    selected = dots_by_bc_index[index];
    selected->select();

    //propagate secondary selection back to the slice diagram
    std::vector<unsigned> primary = selected->get_indexes();
    std::vector<unsigned> secondary;
    for (unsigned i = 0; i < primary.size(); i++) {
        if (primary[i] != index)
            secondary.push_back(primary[i]);
    }

    if (secondary.size() > 0)
        emit persistence_dot_secondary_selection(secondary);
} //end receive_dot_selection()

//remove dot highlighting in response to external command
void PersistenceDiagram::receive_dot_deselection()
{
    if (selected != NULL) {
        selected->deselect();
        selected = NULL;
    }
}

//updates the diagram after a change in the configuration parameters
void PersistenceDiagram::receive_parameter_change()
{
    //update the line highlight
    QPen slicePen(QBrush(config_params->sliceLineColor), config_params->sliceLineWidth);
    blue_line->setPen(slicePen);

    //update persistence dots
    redraw_dots();

    //update text
    QBrush persistenceBrush(QColor(config_params->persistenceColor.rgb()));
    inf_count_text->setBrush(persistenceBrush);
    lt_inf_count_text->setBrush(persistenceBrush);

    //update fonts
    inf_text->setFont(config_params->diagramFont);
    lt_inf_text->setFont(config_params->diagramFont);
    inf_count_text->setFont(config_params->diagramFont);
    lt_inf_count_text->setFont(config_params->diagramFont);

    //update diagram
    resize_diagram();
}
