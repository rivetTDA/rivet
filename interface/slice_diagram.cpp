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

    //set scene rectangle (necessary to prevent auto-scrolling)
    scene->setSceneRect(-padding, -padding, default_scale*(data_xmax - data_xmin) + 2*padding, default_scale*(data_ymax - data_ymin) + 2*padding);  //TODO: IMPROVE!!!
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
            QBrush tempBrush(Qt::magenta);     //THIS ISN'T WHAT WE WANT! FIX THIS!!!
            double radius = round(unit_radius*sqrt(points[i].zero + points[i].one));
            QGraphicsEllipseItem* item = scene->addEllipse( (points[i].x - data_xmin)*default_scale - radius, (points[i].y - data_ymin)*default_scale - radius, 2*radius, 2*radius, Qt::NoPen, tempBrush);
        }
        else    //then draw a green or red disk
        {
            if(points[i].zero > 0)
            {
                double radius = round(unit_radius*sqrt(points[i].zero));
                QGraphicsEllipseItem* item = scene->addEllipse( (points[i].x - data_xmin)*default_scale - radius, (points[i].y - data_ymin)*default_scale - radius, 2*radius, 2*radius, Qt::NoPen, greenBrush);
            }
            else
            {
                double radius = round(unit_radius*sqrt(points[i].one));
                QGraphicsEllipseItem* item = scene->addEllipse( (points[i].x - data_xmin)*default_scale - radius, (points[i].y - data_ymin)*default_scale - radius, 2*radius, 2*radius, Qt::NoPen, redBrush);
            }
         }
    }

    //add control objects
    slice_line = new SliceLine(diagram_width + padding, diagram_height + padding, this);
    scene->addItem(slice_line);

    dot_left = new ControlDot(slice_line, true);
    scene->addItem(dot_left);

    dot_right = new ControlDot(slice_line, false);
    scene->addItem(dot_right);

    slice_line->setDots(dot_left, dot_right);

    //update angle and offset boxes in VisualizationWindow
    update_window_controls();
}//end create_diagram()

//updates the line, in response to a change in the controls in the VisualizationWindow
//NOTE: angle is in DEGREES
void SliceDiagram::update_line(double angle, double offset)
{
    if(angle == 90)     //handle vertical line
    {
        int xpos = (-1*offset - data_xmin)*default_scale;                     //TODO: NORMALIZED SCALE
        slice_line->update_position(xpos, 0, true, 0);
    }
    else if(angle == 0) //handle horizontal line
    {
        int ypos = (offset - data_ymin)*default_scale;                     //TODO: NORMALIZED SCALE
        slice_line->update_position(0, ypos, false, 0);
    }
    else    //handle non-vertical and non-horizontal line
    {
        double radians = angle*3.14159265/180;
        double slope = tan(radians);
        double y_coord = slope*data_xmin + offset/cos(radians); //y-coordinate of slice line at x=data_xmin

        if(y_coord >= data_ymin)    //then slice line intersects left edge of box
        {
            slice_line->update_position(0, (y_coord - data_ymin)*default_scale, false, slope);              //TODO: NORMALIZED SCALE
        }
        else    //then slice line intersects bottom of box
        {
            double x_coord = (data_ymin - offset/cos(radians))/slope;   //x-coordinate of slice line at y=data_ymin

            slice_line->update_position( (x_coord - data_xmin)*default_scale, 0, false, slope);              //TODO: NORMALIZED SCALE
        }
    }
}

//updates controls in the VisualizationWindow
void SliceDiagram::update_window_controls()
{
    //defaults for vertical line
    double angle = 90;
    double offset = -1*(slice_line->pos().x()/default_scale + data_xmin);                      //TODO: NORMALIZED SCALE

    //handle non-vertical line
    if(!slice_line->is_vertical())
    {
        angle = atan(slice_line->get_slope());

        double y_intercept = (slice_line->pos().y()/default_scale + data_ymin - slice_line->get_slope() * (slice_line->pos().x()/default_scale + data_xmin) );       //TODO: NORMALIZED SCALE
        offset = cos(angle) * y_intercept;

        angle = angle*180/3.14159265;   //convert to degrees
    }

    window->set_line_parameters(angle, offset);
}

//gets the length of the slice, for scaling the persistence diagram
double SliceDiagram::get_slice_length()
{
    double dx = slice_line->get_right_pt_x() - slice_line->pos().x();
    double dy = slice_line->get_right_pt_y() - slice_line->pos().y();

    return sqrt(dx*dx + dy*dy);
}

//gets the number of pixels per unit, for the persistence diagram
double SliceDiagram::get_pd_scale()
{
    return default_scale;                                                            //TODO: NORMALIZED SCALE
}

//gets the coordinate on the slice line which we consider "zero" for the persistence diagram        //TODO: CHECK!! IMPROVE!!!
double SliceDiagram::get_zero()
{
    if(slice_line->is_vertical())
        return data_ymin;
    else if(slice_line->get_slope() == 0)
        return data_xmin;
    else
    {
        double x0 = slice_line->pos().x()/default_scale + data_xmin;                         //TODO: NORMALIZED SCALE
        double y0 = slice_line->pos().y()/default_scale + data_ymin;                         //TODO: NORMALIZED SCALE

        double radians = atan(slice_line->get_slope());
        double offset = cos(radians) * (y0 - tan(radians)*x0);
        double x1 = -1*offset * sin(radians);
        double y1 = offset * cos(radians);

//        qDebug() << "radians: " << radians << "; offset: " << offset << "; x1: " << x1 << "; y1: " << y1;

        return sqrt( (x0 - x1)*(x0 - x1) + (y0 - y1)*(y0 - y1) );
    }
}
