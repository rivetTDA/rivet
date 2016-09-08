/****************************************************************************
* this class creates a QWidget for a slice drawing
*
* Matthew L. Wright
* 2014
****************************************************************************/

#include "slicearea.h"
#include <QtGui>
#include <math.h>
#include <sstream>

SliceArea::SliceArea(QWidget* parent)
    : QWidget(parent)
    , fill_pct(0.9)
    , unit_radius(10)
{
    setBackgroundRole(QPalette::Light);

    angle = 45; //TESTING
    offset = 0;
}

void SliceArea::setExtents(double minx, double maxx, double miny, double maxy)
{
    gx = minx;
    lx = maxx;
    gy = miny;
    ly = maxy;
}

void SliceArea::addPoint(double x_coord, double y_coord, int xi0m, int xi1m)
{
    points.push_back(xiPointSD(x_coord, y_coord, xi0m, xi1m));
}

void SliceArea::setLine(int angle, double offset)
{
    this->angle = angle;
    this->offset = offset;
    update();
}

void SliceArea::paintEvent(QPaintEvent* /* event */)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (points.size() == 0) //(!has_data)
    {
        painter.setPen(QColor(80, 80, 80));
        painter.setFont(QFont(painter.font().family(), 50));
        painter.drawText(QRect(0, 170, 400, 50), Qt::AlignCenter, "no data");
    } else {
        //draw points
        painter.setPen(Qt::NoPen);
        QBrush green(QColor(0, 255, 0, 100)); //green semi-transparent
        QBrush red(QColor(255, 0, 0, 100)); //red semi-transparent

        for (int i = 0; i < points.size(); i++) {
            if (points[i].zero > 0 && points[i].one > 0) //then this is a support point of BOTH xi_0 and xi_1
            {
                painter.setBrush(QBrush(QColor(0, 0, 255, 100))); //THIS ISN'T WHAT WE WANT
            } else //then draw a green or red disk
            {
                int mult = 0;
                if (points[i].zero > 0) {
                    mult = points[i].zero;
                    painter.setBrush(green);
                } else {
                    mult = points[i].one;
                    painter.setBrush(red);
                }
                int radius = round(unit_radius * sqrt(mult));
                painter.drawEllipse(QPoint(trans_x(points[i].x), trans_y(points[i].y)), radius, radius);
            }
        }

        //draw bounding box for the diagram
        painter.setPen(QPen(QBrush(Qt::darkGray), 2, Qt::DotLine));
        painter.setBrush(Qt::NoBrush);
        border_width = round(width() * (1 - fill_pct) / 2);
        painter.drawRect(QRect(border_width, border_width, width() - 2 * border_width, height() - 2 * border_width));

        //draw scale labels
        painter.setFont(QFont(painter.font().family(), 10));

        painter.drawText(QRect(width() / 2 - 50, height() - border_width, 100, border_width), Qt::AlignCenter, "time");

        std::ostringstream sgx;
        sgx.precision(4);
        sgx << gx;
        painter.drawText(QRect(0, height() - border_width, border_width * 2, border_width), Qt::AlignCenter, QString(sgx.str().data()));

        std::ostringstream slx;
        slx.precision(4);
        slx << lx;
        painter.drawText(QRect(width() - 2 * border_width, height() - border_width, border_width * 2, border_width), Qt::AlignCenter, QString(slx.str().data()));

        painter.save();
        painter.translate(border_width, height() / 2);
        painter.rotate(270);
        painter.drawText(QRect(-50, -border_width, 100, border_width), Qt::AlignCenter, "distance");
        painter.restore();

        painter.save();
        painter.translate(border_width, height() - border_width);
        painter.rotate(270);
        std::ostringstream sgy;
        sgy.precision(4);
        sgy << gy;
        painter.drawText(QRect(-50, -1 * border_width, 100, border_width), Qt::AlignCenter, QString(sgy.str().data()));
        painter.restore();

        painter.save();
        painter.translate(border_width, border_width);
        painter.rotate(270);
        std::ostringstream sly;
        sly.precision(4);
        sly << ly;
        painter.drawText(QRect(-50, -1 * border_width, 100, border_width), Qt::AlignCenter, QString(sly.str().data()));
        painter.restore();

        //draw the slice line
        painter.setPen(QPen(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        if (angle == 0) {
            //painter.drawLine(QLine(0, offset, width()-1, offset));
            painter.drawLine(QLine(0, trans_y(offset), width() - 1, trans_y(offset)));
        } else if (angle == 90) {
            painter.drawLine(QLine(trans_x(-1 * offset), 0, trans_x(-1 * offset), height() - 1));
        } else {
            //draw a line that isn't horizontal or vertical -- this part can be improved!
            double theta = angle * 3.14159265 / 180;
            double slope = tan(theta);
            double intcpt = offset / cos(theta);
            double left = 2 * gx - lx;
            double right = 2 * lx + gx;

            painter.drawLine(QLine(trans_x(left), trans_y(slope * left + intcpt), trans_x(right), trans_y(slope * right + intcpt)));
        }
    }

    //draw a border around the drawing area
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}

int SliceArea::trans_x(double x)
{
    return round(width() * (fill_pct * (x - gx) / (lx - gx) + (1 - fill_pct) / 2));
}

int SliceArea::trans_y(double y)
{
    return round(height() * ((1 + fill_pct) / 2 - fill_pct * (y - gy) / (ly - gy)));
}
