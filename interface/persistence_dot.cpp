#include "persistence_dot.h"




PersistenceDot::PersistenceDot(PersistenceDiagram *p_diagram, ConfigParameters* params, double unscaled_x, double unscaled_y, double radius, unsigned index) :
    pdgm(p_diagram),
    config_params(params),
    x(unscaled_x), y(unscaled_y),
    index(index),
    radius(radius),
    pressed(false), hover(false)
{
    setAcceptHoverEvents(true);
//    setFlag(ItemSendsGeometryChanges);
}

QRectF PersistenceDot::boundingRect() const
{
    return QRectF(-radius, -radius, 2*radius, 2*radius);
}

void PersistenceDot::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*unused*/, QWidget * /*unused*/)
{
    QRectF rect = boundingRect();
    QBrush brush(config_params->persistenceColor);

    if(hover)
        painter->setPen(QPen(QBrush(config_params->persistenceHighlightColor),3));
    else
        painter->setPen(Qt::NoPen);

    if(pressed)
        brush.setColor(config_params->persistenceHighlightColor);

    painter->setRenderHint(QPainter::Antialiasing);
//    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);
    painter->drawEllipse(rect);
}

void PersistenceDot::select()
{
    pressed = true;
    update();
}

void PersistenceDot::deselect()
{
    pressed = false;
    update();
}

//returns the unscaled x-coordinate associated with this dot
double PersistenceDot::get_x()
{
    return x;
}

//returns the unscaled y-coordinate associated with this dot
double PersistenceDot::get_y()
{
    return y;
}

//returns the index of this dot (e.g. to send to the SliceDiagram for highlighting effects)
unsigned PersistenceDot::get_index()
{
    return index;
}

void PersistenceDot::hoverEnterEvent(QGraphicsSceneHoverEvent * /*unused*/)
{
    hover = true;
    update();
}

void PersistenceDot::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*unused*/)
{
    hover = false;
    update();
}

void PersistenceDot::mousePressEvent(QGraphicsSceneMouseEvent * /*unused*/)
{
    if(pressed) //then unselect this dot
    {
        pressed = false;
        //call persistence diagram unselect function
        pdgm->deselect_dot();
    }
    else    //then select this dot
    {
        pressed = true;
        //call persistence diagram select function
        pdgm->select_dot(this);
    }

    update();
//    QGraphicsItem::mousePressEvent(event);
}

void PersistenceDot::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*unused*/)
{
//    pressed = false;
//    update();
//    QGraphicsItem::mouseReleaseEvent(event);
}
