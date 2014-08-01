#ifndef PERSISTENCEDIAGRAM_H
#define PERSISTENCEDIAGRAM_H

#include <QGraphicsScene>
#include <QtGui>


class PersistenceDiagram
{
public:
    PersistenceDiagram(QGraphicsScene* sc, double length, double scale, double zero);

    void draw_points(std::vector< std::pair<double,double> >* pairs, std::vector<double>* cycles);

    void update_diagram(double length, double zero, std::vector< std::pair<double,double> >* pairs, std::vector<double>* cycles);

private:
    //graphics items
    QGraphicsScene* scene;

    QGraphicsRectItem* bounding_rect;
    QGraphicsLineItem* diag_line;
    QGraphicsLineItem* blue_line;
    QGraphicsLineItem* h_line;
    QGraphicsLineItem* v_line;

    QGraphicsSimpleTextItem* inf_text;
    QGraphicsSimpleTextItem* lt_inf_text;
    QGraphicsSimpleTextItem* inf_count_text;
    QGraphicsSimpleTextItem* lt_inf_count_text;

    std::vector<QGraphicsEllipseItem*> dots;

    //functions
    void draw_frame();

    //parameters
    double scale;       //scale of the persistence diagram
    double size;        //width and height of the persistence diagram
    double line_size;   //width and height of the blue line; i.e. length of slice line divided by sqrt(2)
    double zero_coord;  //data coordinate that we consider zero for the persistence diagram
};

#endif // PERSISTENCEDIAGRAM_H
