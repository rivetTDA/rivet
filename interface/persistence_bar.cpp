#include "persistence_bar.h"

PersistenceBar::PersistenceBar(SliceDiagram *s_diagram, double unscaled_start, double unscaled_end, unsigned index) :
    sdgm(s_diagram),
    start(unscaled_start), end(unscaled_end),
    class_index(index),
    pressed(false), hover(false)
{
    setAcceptHoverEvents(true);
}

QRectF PersistenceBar::boundingRect() const
{
    return shape().boundingRect();
}

QPainterPath PersistenceBar::shape() const
{
    QPainterPath path;
    QPainterPathStroker stroker;
    path.moveTo(0,0);
    path.lineTo(dx,dy);
    stroker.setWidth(10);
    return stroker.createStroke(path);
}

void PersistenceBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen(QColor(160, 0, 200, 127));   //semi-transparent purple
    pen.setWidth(4);

    if(pressed)
    {
        pen.setColor(QColor(255, 140, 0, 220));      //semi-transparent orange
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen);
    painter->drawLine(0, 0, dx, dy);
}

void PersistenceBar::set_line(double start_x, double start_y, double end_x, double end_y)
{
    setPos(start_x, start_y);
    dx = end_x - start_x;
    dy = end_y - start_y;
}

void PersistenceBar::select()
{
    pressed = true;
    update();
}

void PersistenceBar::deselect()
{
    pressed = false;
    update();
}

double PersistenceBar::get_start()
{
    return start;
}

double PersistenceBar::get_end()
{
    return end;
}

double PersistenceBar::get_index()
{
    return class_index;
}

void PersistenceBar::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hover = true;
    update();
}

void PersistenceBar::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    hover = false;
    update();
}

void PersistenceBar::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(pressed) //then unselect this bar
    {
        pressed = false;
        //call slice diagram unselect function
        sdgm->deselect_bar(true);
    }
    else    //then select this bar
    {
        pressed = true;
        //call slice diagram select function
        sdgm->select_bar(this);
    }

//    update();
//    QGraphicsItem::mousePressEvent(event);
}

void PersistenceBar::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
//    pressed = false;
//    update();
//    QGraphicsItem::mouseReleaseEvent(event);
}
