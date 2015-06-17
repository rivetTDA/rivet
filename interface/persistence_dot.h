#ifndef PERSISTENCE_DOT_H
#define PERSISTENCE_DOT_H

//forward declarations
struct ConfigParameters;
class PersistenceDiagram;

#include <QGraphicsItem>
#include <QtWidgets>


class PersistenceDot : public QGraphicsItem
{
public:
    PersistenceDot(PersistenceDiagram* p_diagram, ConfigParameters* params, double unscaled_x, double unscaled_y, unsigned multiplicity, double radius, unsigned index);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

    void select();
    void deselect();

    double get_x(); //returns the unscaled x-coordinate associated with this dot
    double get_y(); //returns the unscaled y-coordinate associated with this dot
    unsigned get_index(); //returns the index of this dot (e.g. to send to the SliceDiagram for highlighting effects)

    void set_radius(double r);  //sets a new radius and re-draws the dot

    const unsigned multiplicity;  //number of bars represented by this dot

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    void mousePressEvent(QGraphicsSceneMouseEvent *);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *);

private:
    PersistenceDiagram* pdgm;
    ConfigParameters* config_params;

    double x;   //unscaled x-coordinate (projection units)
    double y;   //unscaled y-coordinate (projection units)
    unsigned index;   //index of this dot in the vector of PersistenceDots
    double radius;
    bool pressed;
    bool hover;
};


#endif // PERSISTENCE_DOT_H
