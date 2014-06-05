/****************************************************************************
* this class creates a QWidget for a slice drawing
*
* Matthew L. Wright
* 2014
****************************************************************************/

#include <QtGui>
#include <math.h>
#include "slicearea.h"

SliceArea::SliceArea(double gcdx, double gcdy, double lcmx, double lcmy, QWidget *parent)
    : QWidget(parent),
      gx(gcdx), gy(gcdy),
      lx(lcmx), ly(lcmy),
      size(400)
{
    //value for testing
    fill_pct = 0.9;

    //boundary rectangle
    double bw = round(size*(1-fill_pct)/2); //border width
    bound_rect = new QRect(bw, bw, size-2*bw, size-2*bw);

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

QSize SliceArea::minimumSizeHint() const
{
    return QSize(size, size);
}

QSize SliceArea::sizeHint() const
{
    return QSize(size, size);
}

void SliceArea::setLine(int angle, int offset)
{
    this->angle = angle;
    this->offset = offset;
    update();
}

void SliceArea::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, true);

    //draw some circles
    painter.setBrush(QBrush(QColor(0, 255, 0, 100)));   //green semi-transparent
    painter.drawEllipse(QPoint(trans_x(gx),trans_y(gy)), 12, 12);
    painter.drawEllipse(QPoint(50,100), 10, 10);
    painter.drawEllipse(QPoint(200,100), 20, 20);
    painter.drawEllipse(QPoint(220,120), 15, 15);
    painter.drawEllipse(QPoint(220,320), 10, 10);

    painter.setBrush(QBrush(QColor(255, 0, 0, 100)));   //red semi-transparent
    painter.drawEllipse(QPoint(trans_x(lx),trans_y(ly)), 10, 10);
    painter.drawEllipse(QPoint(60,100), 15, 15);
    painter.drawEllipse(QPoint(150,80), 20, 20);
    painter.drawEllipse(QPoint(280,160), 12, 12);
    painter.drawEllipse(QPoint(260,300), 18, 18);

    //draw bounding box for the diagram
    painter.setPen(QPen(QBrush(Qt::darkGray),2,Qt::DotLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(*bound_rect);


    //draw the slice line
    painter.setPen(QPen(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if(angle == 0)
    {
        painter.drawLine(QLine(0, trans_y(offset), width()-1, trans_y(offset)));
    }
    else if(angle == 90)
    {
        painter.drawLine(QLine(trans_x(-1*offset), 0, trans_x(-1*offset), height()-1));
    }
    else
    {
        //draw a line that isn't horizontal or vertical -- this part can be improved!
        double theta = angle*3.14159265/180;
        double slope = tan(theta);
        double intcpt = offset/cos(theta);
        double left = 2*gx - lx;
        double right = 2*lx + gx;

        painter.drawLine(QLine(trans_x(left), trans_y(slope*left+intcpt), trans_x(right), trans_y(slope*right+intcpt)));
    }


    //draw a border around the drawing area
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}

int SliceArea::trans_x(double x)
{
    return round(size*(fill_pct*(x-gx)/(lx-gx) + (1-fill_pct)/2));
}

int SliceArea::trans_y(double y)
{
    return round(size*((1+fill_pct)/2 - fill_pct*(y-gy)/(ly-gy)));
}
