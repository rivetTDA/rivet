#include "persistence_diagram.h"

#include "barcode.h"
#include "config_parameters.h"
#include "persistence_dot.h"

#include <QDebug>
#include <QGraphicsView>

#include <limits>
#include <set>
#include <sstream>


PersistenceDiagram::PersistenceDiagram(ConfigParameters* params, QObject* parent) :
    QGraphicsScene(parent),
    config_params(params),
    selected(NULL),
    radius(5)
{ }

//simply creates all objects; resize_diagram() handles positioning of objects
void PersistenceDiagram::create_diagram(QString* filename, int dim)
{
    //define pens and brushes
    QPen grayPen(QBrush(Qt::darkGray),2,Qt::DotLine, Qt::RoundCap, Qt::RoundJoin);
    QPen thinPen(QBrush(Qt::darkGray),1,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen bluePen(QBrush(Qt::blue),3,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QBrush purpleBrush(QColor(160, 0, 200));

    //create persistence diagram structure
    bounding_rect = addRect(QRectF(), grayPen);
    diag_line = addLine(QLineF(), thinPen);
    blue_line = addLine(QLineF(), bluePen);

    h_line = addLine(QLineF(), grayPen);
    v_line = addLine(QLineF(), grayPen);

    //create text objects
    inf_text = addSimpleText("inf");
    inf_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    lt_inf_text = addSimpleText("<inf");
    lt_inf_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    inf_count_text = addSimpleText("0");
    inf_count_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    inf_count_text->setBrush(purpleBrush);

    lt_inf_count_text = addSimpleText("0");
    lt_inf_count_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    lt_inf_count_text->setBrush(purpleBrush);

    file_text = addSimpleText(*filename);
    file_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    std::ostringstream sdim;
    sdim << "homology dimension: " << dim;
    dim_text = addSimpleText(QString(sdim.str().data()));
    dim_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
}//end create_diagram()

//resizes diagram to fill the QGraphicsView; called after every window resize
void PersistenceDiagram::resize_diagram(double slice_length, double diagram_scale)
{
    //parameters
    int scene_padding = 10; //pixels (minimum white space between diagram objects and edge of viewing window)
    int text_padding = 4;   //pixels (white space on each side of text items)
    int number_space = 30;  //pixels (horizontal space reserved for counts of points above diagram)

    line_size = slice_length/sqrt(2);   //divide by sqrt(2) because the line is drawn at a 45-degree angle
    scale = diagram_scale/sqrt(2);      //similarly, divide by sqrt(2)

    //get dimensions of the QGraphicsView
    QList<QGraphicsView*> view_list = views();
    int view_width = view_list[0]->width();
    int view_height = view_list[0]->height();

    //compute diagram size
    int available_width = view_width - (lt_inf_text->boundingRect().width() + text_padding + number_space + 2*scene_padding);
    int available_height = view_height - (2*lt_inf_text->boundingRect().height() + 4*text_padding + 2*scene_padding);
    diagram_size = std::min(available_height, available_width);

    //resize frame
    bounding_rect->setRect(0, 0, diagram_size, diagram_size);
    diag_line->setLine(0, 0, diagram_size, diagram_size);
    blue_line->setLine(0, 0, line_size, line_size);

    int v_space = lt_inf_text->boundingRect().height() + 2*text_padding;
    h_line->setLine(-lt_inf_text->boundingRect().width(), diagram_size + v_space, diagram_size + number_space, diagram_size + v_space);
    v_line->setLine(diagram_size, diagram_size + text_padding, diagram_size, diagram_size + 2*v_space - text_padding);

    //move dots
    lt_inf_dot_vpos = diagram_size + v_space/2;
    inf_dot_vpos = lt_inf_dot_vpos + v_space;

    int num_big_cycles = 0;
    int num_big_points = 0;

    for(std::vector<PersistenceDot*>::iterator it = dots.begin(); it != dots.end(); ++it)
    {
        PersistenceDot* dot = *it;
        double x = dot->get_x();
        double y = dot->get_y();

        if(y == std::numeric_limits<double>::infinity()) //then this dot represents a cycle
        {
            if(x*scale > diagram_size)
            {
                num_big_cycles++;
                dot->setVisible(false);
            }
            else
            {
                dot->setPos(x*scale, inf_dot_vpos);
                dot->setVisible(true);
            }
        }
        else    //then this dot represents a pair
        {
            if(x*scale > diagram_size)
            {
                num_big_points++;
                dot->setVisible(false);
            }
            else
            {
                if(y*scale > diagram_size)
                    dot->setPos(x*scale, lt_inf_dot_vpos);
                else
                    dot->setPos(x*scale, y*scale);
                dot->setVisible(true);
            }
        }
    }

    //update counts
    std::ostringstream scyc;
    scyc << num_big_cycles;
    inf_count_text->setText(QString(scyc.str().data()));

    std::ostringstream spts;
    spts << num_big_points;
    lt_inf_count_text->setText(QString(spts.str().data()));

    //move text items
    double inf_text_vpos = diagram_size + v_space + text_padding + inf_text->boundingRect().height();
    double lt_inf_text_vpos = diagram_size + text_padding + lt_inf_text->boundingRect().height();

    inf_text->setPos(-inf_text->boundingRect().width() - text_padding, inf_text_vpos);
    lt_inf_text->setPos(-lt_inf_text->boundingRect().width() - text_padding, lt_inf_text_vpos);

    inf_count_text->setPos(diagram_size + text_padding, inf_text_vpos);
    lt_inf_count_text->setPos(diagram_size + text_padding, lt_inf_text_vpos);

    file_text->setPos(diagram_size - file_text->boundingRect().width() - text_padding, file_text->boundingRect().height() + text_padding);
    dim_text->setPos(diagram_size - dim_text->boundingRect().width() - text_padding, file_text->pos().y() + dim_text->boundingRect().height() + text_padding);

    //set scene rectangle (necessary to prevent auto-scrolling)
    double scene_rect_x = -lt_inf_text->boundingRect().width() - text_padding;
    double scene_rect_y = 0;
    double scene_rect_w = diagram_size + number_space - scene_rect_x;
    double scene_rect_h = diagram_size + 2*v_space - scene_rect_y;
    setSceneRect(scene_rect_x, scene_rect_y, scene_rect_w, scene_rect_h);
}//end resize_diagram()

//creates and draws persistence dots at the correct locations
void PersistenceDiagram::draw_points(double zero, Barcode* bc)
{
    zero_coord = zero;

    //counters
    unsigned num_dots = 0;
    unsigned num_big_cycles = 0;
    unsigned num_big_points = 0;

    //loop over all bars
    for(std::multiset<MultiBar>::iterator it = bc->begin(); it != bc->end(); ++it)
    {
        if(it->death == std::numeric_limits<double>::infinity())    //essential cycle
        {
            //shift coordinate
            double birth = it->birth - zero_coord;

            //create dot object
            PersistenceDot* dot = new PersistenceDot(this, config_params, birth, it->death, radius*sqrt((double) (it->multiplicity)), num_dots);
            addItem(dot);
            dots.push_back(dot);
            num_dots++;

            //position dot properly
            if(birth*scale > diagram_size)
            {
                num_big_cycles++;
                dot->setVisible(false);
            }
            else
            {
                dot->setPos(birth*scale, inf_dot_vpos);
            }
        }
        else    //finite bar
        {
            //shift coordinates
            double birth = it->birth - zero_coord;
            double death = it->death - zero_coord;

            //create dot object
            PersistenceDot* dot = new PersistenceDot(this, config_params, birth, death, radius*sqrt(it->multiplicity), num_dots);
            addItem(dot);
            dots.push_back(dot);
            num_dots++;

            //position dot properly
            if(birth*scale > diagram_size)
            {
                num_big_points++;
                dot->setVisible(false);
            }
            else
            {
                if(death*scale > diagram_size)
                    dot->setPos(birth*scale, lt_inf_dot_vpos);
                else
                    dot->setPos(birth*scale, death*scale);
            }
        }
    }

    //draw counts
    std::ostringstream scyc;
    scyc << num_big_cycles;
    inf_count_text->setText(QString(scyc.str().data()));

    std::ostringstream spts;
    spts << num_big_points;
    lt_inf_count_text->setText(QString(spts.str().data()));
}//end draw_points()

//updates the diagram after a change in the slice line
void PersistenceDiagram::update_diagram(double slice_length, double diagram_scale, double zero, Barcode* bc)
{
    //update parameters
    line_size = slice_length/sqrt(2);   //divide by sqrt(2) because the line is drawn at a 45-degree angle
    scale = diagram_scale/sqrt(2);      //similarly, divide by sqrt(2)

    //modify frame
    blue_line->setLine(0, 0, line_size, line_size);

    //remove old dots
    selected = NULL;    //remove any current selection
    while(!dots.empty())
    {
        removeItem(dots.back());
        dots.pop_back();
    }

    //draw new dots
    draw_points(zero, bc);

}//end update_diagram()

//highlight the specified dot, selected in the persistence diagram, and propagate to the slice diagram
void PersistenceDiagram::select_dot(PersistenceDot* clicked)
{
    //remove old selection
    if(selected != NULL && clicked != selected)
        selected->deselect();

    //remember current selection
    selected = clicked;

    //highlight part of the persistence diagram
    emit persistence_dot_selected(clicked->get_index());
}

//remove selection; if propagate, then deselect bar in the slice diagram
void PersistenceDiagram::deselect_dot()
{
    //remove selection
    if(selected != NULL)
    {
        selected->deselect();
        selected = NULL;
    }

    //remove highlighting from slice diagram
    emit persistence_dot_deselected();
}

//highlight the specified dot, which has been selected externally
void PersistenceDiagram::receive_dot_selection(unsigned index)
{
    //remove old selection
    if(selected != NULL && dots[index] != selected)
        selected->deselect();

    //remember current selection
    selected = dots[index];
    selected->select();
}

//remove dot highlighting in response to external command
void PersistenceDiagram::receive_dot_deselection()
{
    if(selected != NULL)
    {
        selected->deselect();
        selected = NULL;
    }
}
