#include "slice_diagram.h"
#include <algorithm>

#include <QDebug>
#include <sstream>

SliceDiagram::SliceDiagram(QGraphicsScene* sc, VisualizationWindow* vw, double xmin, double xmax, double ymin, double ymax, int width, int height) :
    scene(sc), window(vw),
    data_xmin(xmin), data_xmax(xmax), data_ymin(ymin), data_ymax(ymax),
    normalized_scale_x(1), normalized_scale_y(1),
    unit_radius(8), padding(20),
    dot_left(), dot_right(), slice_line()
{
    //determine scales
    if(data_xmax > data_xmin)
        normalized_scale_x = (width - 2*padding)/(data_xmax - data_xmin);

    if(data_ymax > data_ymin)
        normalized_scale_y = (height - 2*padding)/(data_ymax - data_ymin);

    default_scale = normalized_scale_x;
    if(normalized_scale_y < normalized_scale_x)
        default_scale = normalized_scale_y;
}


void SliceDiagram::add_point(double x_coord, double y_coord, int xi0m, int xi1m)
{
    points.push_back(xiPoint(x_coord, y_coord, xi0m, xi1m));
}


void SliceDiagram::create_diagram()
{
    //pens and brushes
    QPen blackPen(Qt::black);
    blackPen.setWidth(2);
    QBrush greenBrush(QColor(0, 255, 0, 100));   //green semi-transparent, for xi_0 support dots
    QBrush redBrush(QColor(255, 0, 0, 100));   //red semi-transparent, for xi_1 support dots
    QPen grayPen(Qt::gray);

    //determine diagram size
    double diagram_width = default_scale*(data_xmax - data_xmin);
    double diagram_height = default_scale*(data_ymax - data_ymin);

    //draw bounds
    scene->addLine(diagram_width, 0, diagram_width, diagram_height, grayPen);
    scene->addLine(0, diagram_height, diagram_width, diagram_height, grayPen);
    control_rect = scene->addRect(0,0,diagram_width + padding,diagram_height + padding, blackPen);

    //draw labels
    std::ostringstream s_xmin;
    s_xmin.precision(4);
    s_xmin << data_xmin;
    data_xmin_text = scene->addSimpleText(QString(s_xmin.str().data()));
    data_xmin_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    data_xmin_text->setPos(data_xmin_text->boundingRect().width()/(-2), padding/(-2));

    std::ostringstream s_xmax;
    s_xmax.precision(4);
    s_xmax << data_xmax;
    data_xmax_text = scene->addSimpleText(QString(s_xmax.str().data()));
    data_xmax_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    data_xmax_text->setPos(diagram_width - data_xmax_text->boundingRect().width()/2, padding/(-2));

    std::ostringstream s_ymin;
    s_ymin.precision(4);
    s_ymin << data_ymin;
    data_ymin_text = scene->addSimpleText(QString(s_ymin.str().data()));
    data_ymin_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    data_ymin_text->setPos(padding/(-2) - data_ymin_text->boundingRect().width(), data_ymin_text->boundingRect().height()/2);

    std::ostringstream s_ymax;
    s_ymax.precision(4);
    s_ymax << data_ymax;
    data_ymax_text = scene->addSimpleText(QString(s_ymax.str().data()));
    data_ymax_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    data_ymax_text->setPos(padding/(-2) - data_ymax_text->boundingRect().width(), diagram_height + data_ymax_text->boundingRect().height()/2);

    //draw points
    for(int i = 0; i < points.size(); i++)
    {
        if(points[i].zero > 0 && points[i].one > 0) //then this is a support point of BOTH xi_0 and xi_1
        {
            QBrush tempBrush(Qt::magenta);     //THIS ISN'"'T WHAT WE WANT! FIX THIS!!!
            double radius = round(unit_radius*sqrt(points[i].zero + points[i].one));
//            QGraphicsEllipseItem* item = scene->addEllipse(points[i].x*default_scale - radius, points[i].y*default_scale - radius, 2*radius, 2*radius,Qt::NoPen,tempBrush);
        }
        else    //then draw a green or red disk
        {
            if(points[i].zero > 0)
            {
                double radius = round(unit_radius*sqrt(points[i].zero));
                QGraphicsEllipseItem* item = scene->addEllipse( (points[i].x - data_xmin)*default_scale - radius, (points[i].y - data_ymin)*default_scale - radius, 2*radius, 2*radius,Qt::NoPen,greenBrush);
            }
            else
            {
                double radius = round(unit_radius*sqrt(points[i].one));
                QGraphicsEllipseItem* item = scene->addEllipse( (points[i].x - data_xmin)*default_scale - radius, (points[i].y - data_ymin)*default_scale - radius, 2*radius, 2*radius,Qt::NoPen,redBrush);
            }
         }
    }


    //add control objects
    slice_line = new SliceLine(diagram_width + padding, diagram_height + padding, window);
    scene->addItem(slice_line);

    dot_left = new ControlDot(slice_line, true);
    scene->addItem(dot_left);

    dot_right = new ControlDot(slice_line, false);
    scene->addItem(dot_right);

    slice_line->setDots(dot_left, dot_right);


}//end create_diagram()

ControlDot* SliceDiagram::get_dot(bool lb)
{
    if(lb)
        return dot_left;
    return dot_right;
}

void SliceDiagram::update_line(QPointF &pos, bool lb)
{
    qDebug() << "inside update_line()";
    if(slice_line != NULL && dot_right != NULL && dot_left != NULL) //make sure all of the essential objects have been initialized (non-null pointers)
    {
        qDebug() << "non-null objects";
//        double angle = 90;

        if(lb)  //left-bottom dot has moved
        {
            QPointF rpos = dot_right->pos();
          /* FIX:  slice_line->setLine(pos.x(), pos.y(), rpos.x(), rpos.y());*/
//            if(pos.x() != rpos.x())
//                angle = atan( (rpos.y()-pos.y())/(rpos.x()-pos.x()) );
        }
        else    //top-right dot has moved
        {
            QPointF lpos = dot_left->pos();
          /* FIX:  slice_line->setLine(lpos.x(), lpos.y(), pos.x(), pos.y());*/
//            if(pos.x() != lpos.x())
//                angle = atan( (pos.y()-lpos.y())/(pos.x()-lpos.x()) );
        }
//       qDebug() << angle;
    }
}
