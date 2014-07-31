#ifndef PERSISTENCEDIAGRAM_H
#define PERSISTENCEDIAGRAM_H

#include <QGraphicsScene>
#include <QtGui>



class PersistenceDiagram
{
public:
    PersistenceDiagram(QGraphicsScene* sc, double length, double scale, double zero);

    void draw_points(std::vector< std::pair<double,double> >* pairs, std::vector<double>* cycles);

private:
    //graphics items
    QGraphicsScene* scene;

    QGraphicsRectItem* bounding_rect;
    QGraphicsLineItem* diag_line;
    QGraphicsLineItem* h_line;
    QGraphicsLineItem* v_line;
    QGraphicsSimpleTextItem* inf_text;
    QGraphicsSimpleTextItem* lt_inf_text;
    QGraphicsSimpleTextItem* inf_count_text;
    QGraphicsSimpleTextItem* lt_inf_count_text;

    //functions
    void draw_frame();

    //parameters
    double scale;  //scale of the persistence diagram
    double width;  //width (and height) of the persistence diagram
    double zero_coord;  //data coordinate that we consider zero for the persistence diagram
};

#endif // PERSISTENCEDIAGRAM_H
