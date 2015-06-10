#include "slice_line.h"

#include "config_parameters.h"
#include "control_dot.h"
#include "slice_diagram.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>


SliceLine::SliceLine(SliceDiagram* sd, ConfigParameters *params) :
    vertical(false), pressed(false), rotating(false), update_lock(false),
    right_point(0,0),
    sdgm(sd),
    config_params(params)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setPos(0,0);
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
    path.moveTo(0,0);
    path.lineTo(right_point);
    stroker.setWidth(20);
    return stroker.createStroke(path);
}

void SliceLine::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*unused*/, QWidget * /*unused*/)
{
    QPen pen(config_params->sliceLineColor);
    pen.setWidth(4);

    if(pressed)
    {
        pen.setColor(config_params->sliceLineHighlightColor);
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen);
    painter->drawLine(0, 0, right_point.x(), right_point.y());
}

//left-click and drag to move line, maintaining the same slope
QVariant SliceLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemPositionChange && !update_lock)
    {
        QPointF mouse = value.toPointF();   //un-adjusted position given by the mouse
        QPointF newpos(mouse);              //adjusted position to make the line stay within bounds

        if(vertical)    //handle vertical lines
        {
            //set newpos to keep left endpoint on bottom edge of box
            if(mouse.x() < 0)
                newpos.setX(0);
            else if(mouse.x() > data_xmax)
                newpos.setX(data_xmax);
            else
                newpos.setX(mouse.x());

            newpos.setY(0);

            //adjust right endpoint to stay on top edge of box
            right_point.setX(0);
            right_point.setY(box_ymax);
        }
        else        //handle non-vertical lines
        {
            //set newpos to keep left endpoint of line along left/bottom edge of box
            if( mouse.y() >= slope*mouse.x() || slope == 0 )    //then left endpoint of line is along left edge of box
            {
                double y_pos = std::min( mouse.y() - slope*mouse.x(), data_ymax );
                if(y_pos < 0)   //this can happen if slope is zero
                    y_pos = 0;
                newpos.setX(0);
                newpos.setY(y_pos);
            }
            else    //then left endpoint of line is along bottom edge of box
            {
                newpos.setX( std::min( mouse.x() - mouse.y()/slope, data_xmax ) );
                newpos.setY(0);
            }

            //adjust right endpoint of line to stay on right/top edge of box
            if( slope*(box_xmax - newpos.x()) + newpos.y() >= box_ymax ) //then right endpoint of line is along top of box
            {
                right_point.setY(box_ymax - newpos.y());
                if(slope > 0)
                    right_point.setX( (box_ymax - newpos.y())/slope );
                else    //can this ever happen?
                    right_point.setX(box_xmax);
            }
            else    //then right endpoint of line is along right edge of box
            {
                right_point.setX( box_xmax - newpos.x() );
                if(slope > 0)
                    right_point.setY( slope*(box_xmax - newpos.x()) );
                else
                    right_point.setY(0);
            }
        }

        //update control dots
        left_dot->set_position(newpos);
        right_dot->set_position(newpos + right_point);

        //update ui control objects
        sdgm->update_window_controls();

        return newpos;
    }
    return QGraphicsItem::itemChange(change, value);
}

//updates left-bottom endpoint
void SliceLine::update_lb_endpoint(QPointF& newpos)
{
    update_lock = true;

    //reposition the right endpoint
    right_point = right_point - (newpos - pos());

    //move the line so that the left endpoint is correct
    setPos(newpos);

    //calculate new slope
    if(right_point.x() <= 0.001)    //caution: floating-point comparison; if line is within 1/1000 pixel of vertical, then we consider it vertical
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
void SliceLine::update_rt_endpoint(QPointF& newpos)
{
    update_lock = true;

    //reposition the right endpoint where it should be
    right_point = newpos - pos();
    update();

    //calculate new slope
    if(right_point.x() <= 0.001)    //caution: floating-point comparison; if line is within 1/1000 pixel of vertical, then we consider it vertical
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

//udpates the dimensions of the on-screen box in which this line is allowed to move
void SliceLine::update_bounds(double data_width, double data_height, int padding)
{
    update_lock = true;

    data_xmax = data_width + padding/2;
    data_ymax = data_height + padding/2;
    box_xmax = data_width + padding;
    box_ymax = data_height + padding;

    update_lock = false;
}


//updates position of line; called by SliceDiagram in response to change in VisualizationWindow controls
void SliceLine::update_position(double xpos, double ypos, bool vert, double pixel_slope)
{
    update_lock = true;

    //update slope
    if(vert)
        vertical = true;
    else
    {
        vertical = false;
        slope = pixel_slope;
    }

    //update left endpoint
    setPos(xpos, ypos);
    left_dot->set_position(QPointF(xpos, ypos));

    //adjust right endpoint of line to stay on right/top edge of box
    compute_right_point();

    //update the picture
    right_dot->set_position(mapToScene(right_point));

    //redraw the line
    update();

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
        //compute new slope
        if(event->pos().x() <= 0)
        {
            vertical = true;
        }
        else
        {
            vertical = false;
            if(event->pos().y() > 0)
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
        right_point.setY(box_ymax - pos().y());
        right_point.setX(0);
    }
    else if( slope*(box_xmax-pos().x()) + pos().y() >= box_ymax) //then line is not vertical, but right endpoint of line is along top of box
    {
        right_point.setY(box_ymax - pos().y());
        right_point.setX( (box_ymax - pos().y())/slope );
    }
    else    //then right endpoint of line is along right edge of box
    {
        right_point.setX(box_xmax - pos().x());
        right_point.setY( slope*(box_xmax-pos().x()) );
    }
}


double SliceLine::get_data_xmax()
{
    return data_xmax;
}

double SliceLine::get_data_ymax()
{
    return data_ymax;
}

double SliceLine::get_box_xmax()
{
    return box_xmax;
}

double SliceLine::get_box_ymax()
{
    return box_ymax;
}
