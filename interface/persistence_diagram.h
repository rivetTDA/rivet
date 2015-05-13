#ifndef PERSISTENCEDIAGRAM_H
#define PERSISTENCEDIAGRAM_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtGui>
#include <QDebug>
#include <sstream>
#include <limits>
#include <set>

#include "../visualizationwindow.h"
class VisualizationWindow;

#include "persistence_dot.h"
class PersistenceDot;

#include "barcode.h"

class PersistenceDiagram
{
public:
    PersistenceDiagram(QGraphicsScene* sc, VisualizationWindow* vw, QString* filename, int dim);

    void create_diagram();  //simply creates all objects; resize_diagram() handles positioning of objects
    void resize_diagram(double slice_length, double diagram_scale);  //resizes diagram to fill the QGraphicsView; called after every window resize

    void draw_points(double zero, Barcode* bc); //creates and draws persistence dots at the correct locations

    void update_diagram(double slice_length, double diagram_scale, double zero, Barcode *bc); //updates the diagram after a change in the slice line

    void select_dot(PersistenceDot* clicked);   //highlight the specified dot, selected in the persistence diagram, and propagate to the slice diagram
    void select_dot(unsigned index);            //highlight the specified dot, which has been selected in the slice diagram
    void deselect_dot(bool propagate);          //remove selection; if propagate, then deselect bar in the slice diagram

private:
    //graphics items
    QGraphicsScene* scene;
    VisualizationWindow* window;

    QGraphicsRectItem* bounding_rect;
    QGraphicsLineItem* diag_line;
    QGraphicsLineItem* blue_line;
    QGraphicsLineItem* h_line;
    QGraphicsLineItem* v_line;

    QGraphicsSimpleTextItem* inf_text;
    QGraphicsSimpleTextItem* lt_inf_text;
    QGraphicsSimpleTextItem* inf_count_text;
    QGraphicsSimpleTextItem* lt_inf_count_text;
    QGraphicsSimpleTextItem* file_text;
    QGraphicsSimpleTextItem* dim_text;

    std::vector<PersistenceDot*> dots;
    PersistenceDot* selected;

    //functions
//    void draw_frame();

    //parameters
    double scale;        //scale of the persistence diagram
    int diagram_size;    //width and height of the persistence diagram (the square part), in pixels
    double line_size;    //width and height of the blue line; i.e. length of slice line divided by sqrt(2)
    double zero_coord;   //data coordinate that we consider zero for the persistence diagram
    int radius;          //radius of the persistence dots
    int inf_dot_vpos;    //vertical position (y-coordinate) of dots representing essential cycles
    int lt_inf_dot_vpos; //vertical position (y-coordinate) of dots representing non-infinite pairs above the diagram

    QString* filename;  //filename, to print on the screen
    int dim;            //dimension of homology, to print on the screen
};

#endif // PERSISTENCEDIAGRAM_H
