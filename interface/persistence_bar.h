#ifndef PERSISTENCE_BAR_H
#define PERSISTENCE_BAR_H


#include <QPainter>
#include <QGraphicsItem>
#include <QDebug>

#include "config_parameters.h"
#include "slice_diagram.h"
class SliceDiagram;


class PersistenceBar : public QGraphicsItem
{
public:
    PersistenceBar(SliceDiagram* s_diagram, ConfigParameters* params, double unscaled_start, double unscaled_end, unsigned index);

    QRectF boundingRect() const;
    QPainterPath shape() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

    void set_line(double start_x, double start_y, double end_x, double end_y);

    void select();
    void deselect();

    double get_start(); //returns the unscaled x-coordinate associated with this bar
    double get_end(); //returns the unscaled y-coordinate associated with this bar
    unsigned get_index(); //returns the index of this bar (e.g. to send to the PersistenceDiagram for highlighting effects)

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    void mousePressEvent(QGraphicsSceneMouseEvent *);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *);

private:
    SliceDiagram* sdgm;
    ConfigParameters* config_params;

    double start;   //unscaled start coordinate (projection units)
    double end;     //unscaled end coordinate (projection units)
    unsigned class_index;   //index of the dot corresponding to this bar in the Barcode object

    bool pressed;
    bool hover;
    double dx;  //horizontal length of line (pixels)
    double dy;  //vertical length of line (pixels)
};


#endif // PERSISTENCE_BAR_H
