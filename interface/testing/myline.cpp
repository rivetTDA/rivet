#include "myline.h"

#include <QtGui>
#include <QDebug>
#include <algorithm>

MyLine::MyLine(double xmin, double xmax, double ymin, double ymax) :
    xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax),
    pressed(false), rotating(false), update_lock(false),
    left_point(xmin,ymin), right_point(xmax,ymax)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setPos(xmin,ymin);

    slope = (ymax-ymin)/(xmax-xmin);

}

void MyLine::setDots(MyDot *left)
{
    left_dot = left;
}

QRectF MyLine::boundingRect() const
{
    return shape().boundingRect(); //QRectF(-120,-60,240,120);
}

QPainterPath MyLine::shape() const
{
    QPainterPath path;
    QPainterPathStroker stroker;
    path.moveTo(left_point);
    path.lineTo(right_point);
    stroker.setWidth(20);
    return stroker.createStroke(path);
}

void MyLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = boundingRect();
    QPen pen(Qt::blue);
    pen.setWidth(4);

    if(pressed)
    {
        pen.setColor(Qt::darkCyan);
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen);
    painter->drawLine(left_point, right_point);
//    pen.setWidth(1);
//    painter->drawPath(shape());
//    painter->drawRect(rect);
}

QVariant MyLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemPositionChange && !update_lock)
    {
        QPointF mouse = value.toPointF();
        QPointF newpos(mouse);

        qDebug() << "mouse: (" << mouse.x() << ", " << mouse.y() << "); pos: (" << pos().x() << ", " << pos().y() << ")";


        //TODO: handle horizontal and vertical lines!!!

        //set newpos to keep left endpoint of line along left/bottom edge of box
        if( mouse.y() >= slope*(mouse.x()-xmin) + ymin )    //then left endpoint of line is along left edge of box
        {
            newpos.setX(xmin);
            newpos.setY( std::min( slope*(xmin-mouse.x()) + mouse.y(), ymax ) );
        }
        else    //then left endpoint of line is along bottom edge of box
        {
            newpos.setY(ymin);
            newpos.setX( std::min( (ymin-mouse.y())/slope + mouse.x(), xmax ) );
        }

        //adjust right endpoint of line to stay on right/top edge of box
        if( slope*(xmax-mouse.x()) + mouse.y() >= ymax) //then right endpoint of line is along top of box
        {
            right_point.setY(ymax - newpos.y());
            right_point.setX( (ymax - mouse.y())/slope + mouse.x() - newpos.x() );
        }
        else    //then right endpoint of line is along right edge of box
        {
            right_point.setX(xmax - newpos.x());
            right_point.setY( slope*(xmax-mouse.x()) + mouse.y() - newpos.y() );
        }

        //update left dot
        left_dot->set_position(mapToScene(0, 0));

        return newpos;
    }
    return QGraphicsItem::itemChange(change, value);
}

void MyLine::update_lb_endpoint(QPointF &delta)
{
    update_lock = true;

    //move the line so that the left endpoint is correct
    moveBy(delta.x(), delta.y());

    //reposition the right endpoint where it should be
    right_point = right_point - delta;

    //calculate new slope
    slope = right_point.y()/right_point.x();

    update_lock = false;
}

void MyLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //testing
    if(event->button() == Qt::RightButton)
    {
        qDebug() << "right button: (" << event->pos().x() << ", " << event->pos().y() << ")";
        rotating = true;
    }

    pressed = true;
    update();
    QGraphicsItem::mousePressEvent(event);
}

void MyLine::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //testing
    if(event->button() == Qt::RightButton)
    {
        qDebug() << "right button released";
        rotating = false;
    }

    pressed = false;
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}

void MyLine::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(rotating)
    {
        qDebug() << "right button move: (" << event->pos().x() << ", " << event->pos().y() << ")";

        //compute new slope     TODO: HANDLE VERTICAL LINES!!!
        slope = event->pos().y() / event->pos().x();
        if(slope < 0)
            slope = 0;

        //adjust right endpoint of line to stay on right/top edge of box
        if( slope*(xmax-pos().x()) + pos().y() >= ymax) //then right endpoint of line is along top of box
        {
            right_point.setY(ymax - pos().y());
            right_point.setX( (ymax - pos().y())/slope );
        }
        else    //then right endpoint of line is along right edge of box
        {
            right_point.setX(xmax - pos().x());
            right_point.setY( slope*(xmax-pos().x()) );
        }

        update();
    }

    QGraphicsItem::mouseMoveEvent(event);
}



