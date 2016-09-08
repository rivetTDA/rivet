#ifndef PERSISTENCE_DOT_H
#define PERSISTENCE_DOT_H

//forward declarations
struct ConfigParameters;
class PersistenceDiagram;

#include <QGraphicsItem>
#include <QtWidgets>

class PersistenceDot : public QGraphicsItem {
public:
    PersistenceDot(PersistenceDiagram* p_diagram, ConfigParameters* params, double unscaled_x, double unscaled_y, unsigned multiplicity, double radius, unsigned index);

    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

    void select();
    void deselect();

    void add_index(unsigned index); //adds another "barcode" index to this dot
    void incr_multiplicity(unsigned m); //increases the multiplicity of this dot by m

    double get_x(); //returns the unscaled x-coordinate associated with this dot
    double get_y(); //returns the unscaled y-coordinate associated with this dot
    std::vector<unsigned> get_indexes(); //returns the "barcode" indexes of this dot (e.g. to send to the SliceDiagram for highlighting effects)
    unsigned get_multiplicity(); //returns the multiplicity of this dot

    void set_radius(double r); //sets a new radius and re-draws the dot

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
    void mousePressEvent(QGraphicsSceneMouseEvent*);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*);

private:
    PersistenceDiagram* pdgm;
    ConfigParameters* config_params;

    double x; //unscaled x-coordinate (projection units)
    double y; //unscaled y-coordinate (projection units)
    std::vector<unsigned> indexes; //indexes of this dot in the vector PersistenceDiagram::dots_by_bc_index (more than one index are possible because dots get combined in the upper horizontal strip of the persistence diagram)
    unsigned multiplicity; //number of bars represented by this dot
    double radius;
    bool pressed;
    bool hover;
};

#endif // PERSISTENCE_DOT_H
