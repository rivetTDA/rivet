#include "control_dot.h"

#include <QtGui>
#include <QDebug>
#include <sstream>


ControlDot::ControlDot(SliceLine* line, bool left_bottom) :
    slice_line(line), pressed(false), left_bottom(left_bottom), update_lock(false)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);

    if(!left_bottom)
    {
        update_lock = true;
        setPos(slice_line->xmax, slice_line->ymax);
        update_lock = false;
    }
}

QRectF ControlDot::boundingRect() const
{
    return QRectF(-10,-10,20,20);
}

void ControlDot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = boundingRect();
    QBrush brush(QColor(0, 0, 255, 150));   //semi-transparent blue

    if(pressed)
    {
        brush.setColor(QColor(0, 200, 200, 150));      //semi-transparent cyan
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);
    painter->drawEllipse(rect);
}

QVariant ControlDot::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemPositionChange && !update_lock)
    {
        QPointF mouse = value.toPointF();
        QPointF newpos(mouse);

        if(left_bottom) //then this dot moves along the left and bottom sides of the box
        {
            if(mouse.y() > 0 && mouse.y() >= mouse.x())   //then project dot onto left side of box (the y-axis)
            {
                newpos.setX(0);     //default: orthogonal projection

                if(mouse.y() < 2*mouse.x())    //smooth transition in region around y=x
                    newpos.setY(2*(mouse.y() - mouse.x()));
                if(newpos.y() > slice_line->get_right_pt_y())     //don't let left dot go above right endpoint of line
                    newpos.setY(slice_line->get_right_pt_y());
            }
            else if(mouse.x() > 0)   //then project dot onto bottom side of box (the x-axis)
            {
                newpos.setY(0);     //default: orthongonal projection

                if(mouse.x() < 2*mouse.y())    //smooth transition in region around y=x
                    newpos.setX(2*(mouse.x() - mouse.y()));
                if(newpos.x() > slice_line->get_right_pt_x()) //don't let bottom dot go right of the top endpoint of line
                    newpos.setX(slice_line->get_right_pt_x());
            }
            else    //then place dot at origin
            {
                newpos.setX(0);
                newpos.setY(0);
            }
        }
        else    //then this dot moves along the right and top sides of the box
        {
            if( mouse.y() < slice_line->ymax  &&  (slice_line->ymax - mouse.y()) >= (slice_line->xmax - mouse.x()) )   //then project dot onto right side of box
            {
                newpos.setX(slice_line->xmax);     //default: orthogonal projection

                if( (slice_line->ymax - mouse.y()) < 2*(slice_line->xmax - mouse.x()) )    //smooth transition in region around y-ymax=x-xmax
                    newpos.setY(slice_line->ymax - 2*(slice_line->ymax - mouse.y() - slice_line->xmax + mouse.x()));
                if(newpos.y() < slice_line->pos().y())     //don't let right dot go below left endpoint of line
                    newpos.setY(slice_line->pos().y());
            }
            else if(mouse.x() < slice_line->xmax)   //then project dot onto top side of box
            {
                newpos.setY(slice_line->ymax);     //default: orthongonal projection

                if(slice_line->xmax - mouse.x() < 2*(slice_line->ymax - mouse.y()) )    //smooth transition in region around y=x
                    newpos.setX(slice_line->xmax - 2*(slice_line->xmax - mouse.x() - slice_line->ymax + mouse.y()));
                if(newpos.x() < slice_line->pos().x()) //don't let top dot go left of the bottom endpoint of line
                    newpos.setX(slice_line->pos().x());
            }
            else    //then place dot at top-right corner of box
            {
                newpos.setX(slice_line->xmax);
                newpos.setY(slice_line->ymax);
            }
        }

        //update line position
//        qDebug() << "  mouse: (" << mouse.x() << ", " << mouse.y() << "); moving line: (" << newpos.x() << ", " << newpos.y() << ")";
        QPointF delta = newpos - pos();
        if(left_bottom)
            slice_line->update_lb_endpoint(delta);
        else
            slice_line->update_rt_endpoint(delta);

        //return
        return newpos;
    }
    return QGraphicsItem::itemChange(change, value);
}

void ControlDot::set_position(const QPointF &newpos)
{
    update_lock = true;

    setPos(newpos);

    update_lock = false;
}

void ControlDot::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = true;
    update();
    QGraphicsItem::mousePressEvent(event);
}

void ControlDot::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = false;
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}
