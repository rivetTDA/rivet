#include "control_dot.h"

ControlDot::ControlDot(SliceDiagram* sd, double xmin, double xmax, double ymin, double ymax) :
    diagram(sd), pressed(false), left_bottom(true),
    xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
}

void ControlDot::set_right_top()
{
    left_bottom = false;
}

QRectF ControlDot::boundingRect() const
{
    return QRectF(-30,-30,60,60);
}

void ControlDot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = boundingRect();
    QBrush brush(Qt::blue);

    if(pressed)
    {
        brush.setColor(Qt::darkCyan);
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);
    painter->drawEllipse(rect);
}

QVariant ControlDot::itemChange(GraphicsItemChange change, const QVariant &value)
{
    ControlDot* other = diagram->get_dot(!left_bottom); //gets a pointer to the other control dot

    if(change == QGraphicsItem::ItemPositionChange && other != NULL)
    {
        QPointF mouse = value.toPointF();
        QPointF newpos(mouse);

        if(left_bottom) //then this is a left-bottom control dot
        {
            if(mouse.y() > ymin && mouse.y()-ymin >= mouse.x()-xmin)   //then project dot to vertical line x=xmin
            {
                newpos.setX(xmin);

                if(mouse.y() > other->pos().y())     //don't let dot go above the other dot
                    newpos.setY(other->pos().y());
                else if(mouse.y()-ymin < 2*(mouse.x()-xmin))    //smooth transition in region around y-ymin=x-xmin
                    newpos.setY(2*( mouse.y()-ymin - (mouse.x()-xmin) ) + ymin);
                //otherwise, orthogonal projection onto line
            }
            else if(mouse.x() > xmin)   //then project dot to horizontal line y=ymin
            {
                newpos.setY(ymin);

                if(mouse.x() > other->pos().x()) //don't let dot go to the right of the other dot
                    newpos.setX(other->pos().x());
                else if(mouse.x()-xmin < 2*(mouse.y()-ymin))    //smooth transition in region around y-ymin=x-xmin
                    newpos.setX(2*( mouse.x()-xmin - (mouse.y()-ymin) ) + xmin);
                //otherwise, orthongonal projection onto line
            }
            else    //then place dot at (xmin,ymin)
            {
                newpos.setX(xmin);
                newpos.setY(ymin);
            }
        }//end if
        else    //then this is a top-right control dot
        {
            if(mouse.y() < ymax && mouse.y()-ymax <= mouse.x()-xmax)    //then project dot to vertical line x=xmax
            {
                newpos.setX(xmax);

                if(mouse.y() < other->pos().y())    //don't let dot go below other dot
                    newpos.setY(other->pos().y());
                else if(mouse.y()-ymax > 2*(mouse.x()-xmax))    //smooth transition region around y-ymax=x-xmax
                    newpos.setY(2*( mouse.y()-ymax - (mouse.x()-xmax) ) + ymax );
                //otherwise, orthogonal projection onto line
            }
            else if(mouse.x() < xmax)   //then project dot onto horizontal line y=ymax
            {
                newpos.setY(ymax);

                if(mouse.x() < other->pos().x())   //don't let dot go to the left of the other dot
                    newpos.setX(other->pos().x());
                else if(mouse.x()-xmax > 2*(mouse.y()-ymax))    //smooth transition region around y-ymax=x-xmax
                    newpos.setX(2*( mouse.x()-xmax - (mouse.y()-ymax) ) + xmax );
                //otherwise, orthogonal projection onto line
            }
            else    //then place dot at (xmax,ymax)
            {
                newpos.setX(xmax);
                newpos.setY(ymax);
            }
        }//end else

//       std::ostringstream oss;
//       oss << "(" << newpos.x() << "," << newpos.y() << ")";
//       coords->setPlainText(QString::fromStdString(oss.str()));

        //update the line
        diagram->update_line(newpos, left_bottom);

        return newpos;
    }
    return QGraphicsItem::itemChange(change, value);
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
