#include "slice_diagram.h"
#include <algorithm>

#include <QDebug>
#include <sstream>

SliceDiagram::SliceDiagram(QGraphicsScene* sc, VisualizationWindow* vw) :
    scene(sc), window(vw), scale(1),
    dot_left(), dot_right(), slice_line()
{
    //TESTING ONLY
    QPen blackPen(Qt::black);
    blackPen.setWidth(2);
    QPen grayPen(Qt::gray);

    int box_height = 250;
    int box_width = 180;

    scene->addRect(0,0,box_width,box_height,blackPen);

    scene->addLine(0,0,180,180,grayPen);
    scene->addLine(0,0,100,200,grayPen);
    scene->addLine(0,0,180,90,grayPen);

    scene->addLine(0,70,180,250,grayPen);
    scene->addLine(80,50,180,250,grayPen);
    scene->addLine(0,160,180,250,grayPen);

    slice_line = new SliceLine(box_width, box_height, window);
    scene->addItem(slice_line);

    dot_left = new ControlDot(slice_line, true);
    scene->addItem(dot_left);

    dot_right = new ControlDot(slice_line, false);
    scene->addItem(dot_right);

    slice_line->setDots(dot_left, dot_right);

    window->test(0.7);

    //END TESTING
}


void SliceDiagram::add_point(double x_coord, double y_coord, int xi0m, int xi1m)
{
    //update data extents, if necessary
    if(points.size() == 0)
    {
        xmin = x_coord;
        xmax = x_coord;
        ymin = y_coord;
        ymax = y_coord;
    }
    else
    {
        if(x_coord < xmin)
            xmin = x_coord;
        if(x_coord > xmax)
            xmax = x_coord;
        if(y_coord < ymin)
            ymin = y_coord;
        if(y_coord > ymax)
            ymax = y_coord;
    }

    //add the point
    points.push_back(xiPoint(x_coord, y_coord, xi0m, xi1m));
}


void SliceDiagram::create_diagram()
{
    //pens and brushes
//    QBrush greenBrush(QColor(0, 255, 0, 100));   //green semi-transparent
//    QBrush redBrush(QColor(255, 0, 0, 100));   //red semi-transparent
//    QPen grayPen(Qt::gray);

//    //scale data up if default scale is too small (must avoid drawing features of size less than one unit, since units get converted to pixels, apparently)
//    double min_extent = std::min(xmax-xmin, ymax-ymin);
//    scale = 1;
//    if(min_extent < 1000)
//        scale = 1000/min_extent;

//    //determine radius for a dot representing an xi support point of multiplicity one
//    double unit_radius = scale*min_extent/30;
//    qDebug() << "UNIT RADIUS SET TO " << unit_radius << "\n";

//    //draw bounding rectangle
//    data_rect = scene->addRect(xmin*scale,ymin*scale,(xmax-xmin)*scale,(ymax-ymin)*scale,grayPen,Qt::NoBrush);

//    //draw labels
//    std::ostringstream s_xmin;
//    s_xmin.precision(4);
//    s_xmin << xmin;
//    data_xmin_text = scene->addSimpleText(QString(s_xmin.str().data()));
//    data_xmin_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
//    data_xmin_text->setPos(xmin*scale,ymin*scale);

//    std::ostringstream s_xmax;
//    s_xmax.precision(4);
//    s_xmax << xmax;
//    data_xmax_text = scene->addSimpleText(QString(s_xmax.str().data()));
//    data_xmax_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
//    data_xmax_text->setPos(xmax*scale, ymin*scale);

//    std::ostringstream s_ymin;
//    s_ymin.precision(4);
//    s_ymin << ymin;
//    data_ymin_text = scene->addSimpleText(QString(s_ymin.str().data()));
//    data_ymin_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
//    data_ymin_text->setPos(xmin*scale,ymin*scale);

//    std::ostringstream s_ymax;
//    s_ymax.precision(4);
//    s_ymax << ymax;
//    data_ymax_text = scene->addSimpleText(QString(s_ymax.str().data()));
//    data_ymax_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
//    data_ymax_text->setPos(xmin*scale,ymax*scale);

//    //draw points
//    for(int i=0; i<points.size(); i++)
//    {
//        if(points[i].zero > 0 && points[i].one > 0) //then this is a support point of BOTH xi_0 and xi_1
//        {
//            QBrush tempBrush(Qt::magenta);     //THIS ISN'"'T WHAT WE WANT! FIX THIS!!!
//            double radius = round(unit_radius*sqrt(points[i].zero + points[i].one));
//            QGraphicsEllipseItem* item = scene->addEllipse(points[i].x*scale - radius, points[i].y*scale - radius, 2*radius, 2*radius,Qt::NoPen,tempBrush);
//        }
//        else    //then draw a green or red disk
//        {
//            if(points[i].zero > 0)
//            {
//                double radius = round(unit_radius*sqrt(points[i].zero));
//                QGraphicsEllipseItem* item = scene->addEllipse(points[i].x*scale - radius, points[i].y*scale - radius, 2*radius, 2*radius,Qt::NoPen,greenBrush);
//            }
//            else
//            {
//                double radius = round(unit_radius*sqrt(points[i].one));
//                QGraphicsEllipseItem* item = scene->addEllipse(points[i].x*scale - radius, points[i].y*scale - radius, 2*radius, 2*radius,Qt::NoPen,redBrush);
//            }
//         }
//    }

//    //draw slice line
//    QPen bluePen(Qt::blue);
//    bluePen.setWidth(10);
//    slice_line = scene->addLine(xmin*scale, ymin*scale, xmax*scale, ymax*scale, bluePen);

//    //draw control dots
//    dot_left = new ControlDot(this, xmin*scale, xmax*scale, ymin*scale, ymax*scale);
//    dot_left->setPos(xmin*scale,ymin*scale);
//    scene->addItem(dot_left);

//    dot_right = new ControlDot(this, xmin*scale, xmax*scale, ymin*scale, ymax*scale);
//    dot_right->set_right_top();
//    dot_right->setPos(xmax*scale,ymax*scale);
//    scene->addItem(dot_right);


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
