#include "persistence_dot.h"




PersistenceDot::PersistenceDot(PersistenceDiagram *p_diagram, double unscaled_x, double unscaled_y, int radius, unsigned index) :
    pdgm(p_diagram),
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

void PersistenceDot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = boundingRect();
    QBrush brush(QColor(160, 0, 200, 127));   //semi-transparent purple

    if(hover)
        painter->setPen(QPen(QBrush(QColor(255, 140, 0, 150)),3));
    else
        painter->setPen(Qt::NoPen);

    if(pressed)
        brush.setColor(QColor(255, 140, 0, 220));      //semi-transparent orange

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
double PersistenceDot::get_index()
{
    return index;
}

void PersistenceDot::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hover = true;
    update();
}

void PersistenceDot::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    hover = false;
    update();
}

void PersistenceDot::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(pressed) //then unselect this dot
    {
        pressed = false;
        //call persistence diagram unselect function
        pdgm->deselect_dot(true);
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

void PersistenceDot::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
//    pressed = false;
//    update();
//    QGraphicsItem::mouseReleaseEvent(event);
}
