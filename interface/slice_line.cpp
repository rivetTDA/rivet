#include "slice_line.h"

#include <QtGui>
#include <QDebug>
#include <algorithm>


SliceLine::SliceLine(int width, int height, SliceDiagram* sd) :
    xmax(width), ymax(height), sdgm(sd),
    vertical(false), pressed(false), rotating(false), update_lock(false),
    left_point(0,0), right_point(xmax,ymax)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setPos(0,0);

    if(xmax == 0)
        vertical = true;
    else
        slope = ymax/xmax;
}

void SliceLine::setDots(ControlDot* left, ControlDot* right)
{
    left_dot = left;
    right_dot = right;
}

QRectF SliceLine::boundingRect() const
{
    return shape().boundingRect();
}

QPainterPath SliceLine::shape() const
{
    QPainterPath path;
    QPainterPathStroker stroker;
    path.moveTo(left_point);
    path.lineTo(right_point);
    stroker.setWidth(20);
    return stroker.createStroke(path);
}

void SliceLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = boundingRect();
    QPen pen(QColor(0, 0, 255, 150));   //semi-transparent blue
    pen.setWidth(4);

    if(pressed)
    {
        pen.setColor(QColor(0, 200, 200, 150));       //semi-transparent cyan
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen);
    painter->drawLine(left_point, right_point);
}

//left-click and drag to move line, maintaining the same slope
QVariant SliceLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
//    if(change == QGraphicsItem::ItemPositionChange)
//        qDebug() << "slice line ItemPositionChange; update_lock: " << update_lock;

    if(change == QGraphicsItem::ItemPositionChange && !update_lock)
    {
        QPointF mouse = value.toPointF();
        QPointF newpos(mouse);

//        qDebug() << "mouse: (" << mouse.x() << ", " << mouse.y() << "); pos: (" << pos().x() << ", " << pos().y() << ")";

        if(vertical)    //handle vertical lines
        {
            //set newpos to keep left endpoint on bottom edge of box
            if(mouse.x() < 0)
                newpos.setX(0);
            else if(mouse.x() > xmax)
                newpos.setX(xmax);
            else
                newpos.setX(mouse.x());

            newpos.setY(0);

            //adjust right endpoint to stay on top edge of box
            right_point.setX(0);
            right_point.setY(ymax);
        }
        else        //handle non-vertical lines:
        {
            //set newpos to keep left endpoint of line along left/bottom edge of box
            if( mouse.y() >= slope*mouse.x() )    //then left endpoint of line is along left edge of box
            {
                newpos.setX(0);
                newpos.setY( std::min( mouse.y() - slope*mouse.x(), ymax ) );
            }
            else    //then left endpoint of line is along bottom edge of box
            {
                newpos.setY(0);
                if(slope > 0)
                    newpos.setX( std::min( mouse.x() - mouse.y()/slope, xmax ) );
                else
                    newpos.setX(0);
            }

            //adjust right endpoint of line to stay on right/top edge of box
            if( slope*(xmax-mouse.x()) + mouse.y() >= ymax) //then right endpoint of line is along top of box
            {
                right_point.setY(ymax - newpos.y());
                if(slope > 0)
                    right_point.setX( std::max( (ymax - mouse.y())/slope + mouse.x() - newpos.x(), 0.0 ) );
                else
                    right_point.setX(xmax);
            }
            else    //then right endpoint of line is along right edge of box
            {
                right_point.setX( xmax - newpos.x() );
                right_point.setY( std::max(slope*(xmax-mouse.x()) + mouse.y() - newpos.y(), 0.0 ) );
            }
        }

//        qDebug() << "moving line; right point: (" << right_point.x() << ", " << right_point.y() << ")";

        //update control dots
        left_dot->set_position(mapToScene(0, 0));
        right_dot->set_position(mapToScene(right_point));

        //update ui control objects
        sdgm->update_window_controls();

        return newpos;
    }
    return QGraphicsItem::itemChange(change, value);
}

//updates left-bottom endpoint
void SliceLine::update_lb_endpoint(QPointF &delta)
{
    update_lock = true;

    //reposition the right endpoint
    right_point = right_point - delta;

    //move the line so that the left endpoint is correct
    moveBy(delta.x(), delta.y());

    //calculate new slope
    if(right_point.x() == 0)
        vertical = true;
    else
    {
        vertical = false;
        slope = right_point.y()/right_point.x();
    }

    //update ui control objects
    sdgm->update_window_controls();

    update_lock = false;
}

//updates right-top endpoint
void SliceLine::update_rt_endpoint(QPointF &delta)
{
    update_lock = true;

    //reposition the right endpoint where it should be
    right_point = right_point + delta;
    update();

    //calculate new slope
    if(right_point.x() == 0)
        vertical = true;
    else
    {
        vertical = false;
        slope = right_point.y()/right_point.x();
    }

    //update ui control objects
    sdgm->update_window_controls();

    update_lock = false;
}

//gets x-coordinate of right-top endpoint
double SliceLine::get_right_pt_x()
{
    return mapToScene(right_point).x();
}

//gets y-coordinate of right-top endpoint
double SliceLine::get_right_pt_y()
{
    return mapToScene(right_point).y();;
}

//gets the slope of the line
double SliceLine::get_slope()
{
    return slope;
}

//true if the line is vertical, false otherwise
bool SliceLine::is_vertical()
{
    return vertical;
}

//updates position of line; called by SliceDiagram in response to change in VisualizationWindow controls
void SliceLine::update_position(double xpos, double ypos, bool vert, double m)
{
    update_lock = true;

    //update slope
    if(vert)
        vertical = true;
    else
    {
        vertical = false;
        slope = m;
    }

    //update left endpoint
    setPos(xpos, ypos);
    left_dot->set_position(QPointF(xpos, ypos));

    //adjust right endpoint of line to stay on right/top edge of box
    compute_right_point();

    //update the picture
    right_dot->set_position(mapToScene(right_point));

    update_lock = false;
}


void SliceLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        rotating = true;
    }

    pressed = true;
    update();
    QGraphicsItem::mousePressEvent(event);
}

void SliceLine::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        rotating = false;
    }

    pressed = false;
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}

//right-click and drag to change slope, left/bottom endpoint stays fixed
void SliceLine::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(rotating)
    {
//        qDebug() << "right button move: (" << event->pos().x() << ", " << event->pos().y() << ")";

        //compute new slope
        if(event->pos().x() <= 0)
        {
            vertical = true;
        }
        else
        {
            vertical = false;
            if(event->pos().y() <= 0)
                slope = 0;
            else
                slope = event->pos().y() / event->pos().x();
        }

        //adjust right endpoint of line to stay on right/top edge of box
        compute_right_point();

        //update the picture
        right_dot->set_position(mapToScene(right_point));
        update();

        //update ui control objects
        sdgm->update_window_controls();
    }

    QGraphicsItem::mouseMoveEvent(event);
}//end mouseMoveEvent()


//sets correct position of right_point, given slope of line and position of left point
void SliceLine::compute_right_point()
{
    if(vertical)    //then line is vertical, so right endpoint of line is along top of box
    {
        right_point.setY(ymax - pos().y());
        right_point.setX(0);
    }
    else if( slope*(xmax-pos().x()) + pos().y() >= ymax) //then line is not vertical, but right endpoint of line is along top of box
    {
        right_point.setY(ymax - pos().y());
        right_point.setX( (ymax - pos().y())/slope );
    }
    else    //then right endpoint of line is along right edge of box
    {
        right_point.setX(xmax - pos().x());
        right_point.setY( slope*(xmax-pos().x()) );
    }
}

