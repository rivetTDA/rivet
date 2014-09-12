/****************************************************************************
* this class creates a QWidget for a persistence diagram
*
* Matthew L. Wright
* 2014
****************************************************************************/

#include <QtGui>
#include <sstream>

#include "pdarea.h"



PDArea::PDArea(QWidget *parent)
    : QWidget(parent), has_data(false),
      default_max(10), scale_multiplier(1)
{
    setBackgroundRole(QPalette::Light);
}
void PDArea::setData(std::vector< std::pair<double,double> >* p, std::vector<double>* c)
{
    pairs = p;
    cycles = c;
    has_data = true;
}

void PDArea::drawDiagram()
{
    update();
}

void PDArea::setMax(double m)
{
    default_max = m;
}

void PDArea::setScale(double s)
{
    scale_multiplier = s;
}

double PDArea::fitScale()
{
    //find the max data value       //TODO: HANDLE THE CASE WHERE THERE IS ONLY ONE POINT (i.e. min = max)
    double max = 0;
    if(pairs->size() > 0)
    {
       max = (*pairs)[0].second;

       for(int i=1; i<pairs->size(); i++)
       {
           if((*pairs)[i].second > max)
               max = (*pairs)[i].second;
       }
    }
    else
    {
       max = (*cycles)[0];
    }
    for(int i=0; i<cycles->size(); i++)
    {
       if((*cycles)[i] > max)
           max = (*cycles)[i];
    }

    scale_multiplier = max/default_max;

    qDebug() << "\n max set to: " << max << "\n";
    \
    return scale_multiplier;
}

void PDArea::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if(!has_data)
    {
        painter.setPen(QColor(80, 80, 80));
        painter.setFont(QFont(painter.font().family(), 50));
        painter.drawText(QRect(0,170,400,50),Qt::AlignCenter,"no data");
    }
    else    //then draw a persistence diagram
    {
        //make sure there is data
        if(pairs->size() == 0 && cycles->size() == 0)
        {
            painter.drawText(QRect(0,170,400,50),Qt::AlignCenter,"empty diagram");
            return;
        }

        //set max value
        current_max = scale_multiplier*default_max;

        //define pens
        QPen gray(QBrush(Qt::darkGray),2,Qt::DotLine, Qt::RoundCap, Qt::RoundJoin);
        QPen thickblack(QBrush(Qt::black),6,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        //draw persistence diagram structure
        painter.setPen(gray);
        painter.drawLine(QLine(10, 25, width()-11, 25));
        painter.drawLine(QLine(width()-40, 5, width()-40, 45));
        painter.drawText(QPoint(10, 20), "inf");
        painter.drawText(QPoint(10, 45), "<inf");

        painter.drawRect(QRect(40, 60, width()-80, height()-80));
        painter.drawLine(QLine(40, height()-20, width()-40, 60));

        //draw dots representing cycles
        painter.setPen(thickblack);
        int num_big_cycles = 0;
        for(int i=0; i<cycles->size(); i++)
        {
            if( (*cycles)[i] > current_max )
                num_big_cycles++;
            else
                painter.drawPoint(trans_x((*cycles)[i]), 12);
        }

        //draw persistence diagram
        int num_big_points = 0;
        for(int i=0; i<pairs->size(); i++)
        {
            if((*pairs)[i].first > current_max )
                num_big_points++;
            else
            {
                if( (*pairs)[i].second > current_max )
                    painter.drawPoint( trans_x((*pairs)[i].first), 37);
                else
                    painter.drawPoint( trans_x((*pairs)[i].first), trans_y((*pairs)[i].second) );
            }
        }

        //draw labels
        painter.setPen(gray);
        std::ostringstream scyc;
        scyc << num_big_cycles;
        painter.drawText(QRect(width()-40, 5, 40, 20), Qt::AlignCenter, QString(scyc.str().data()));

        std::ostringstream spts;
        spts << num_big_points;
        painter.drawText(QRect(width()-40, 25, 40, 20), Qt::AlignCenter, QString(spts.str().data()));

        painter.setFont(QFont(painter.font().family(), 10));

        std::ostringstream smax;
        smax.precision(4);
        smax << current_max;
        painter.drawText(QRect(width()-80, height()-20, 80, 20), Qt::AlignCenter, QString(smax.str().data()));
        painter.drawText(QRect(0, 50, 40, 20), Qt::AlignCenter, QString(smax.str().data()));

        painter.drawText(QRect(0, height()-20, 80, 20), Qt::AlignCenter, "0");
        painter.drawText(QRect(0, height()-30, 40, 20), Qt::AlignCenter, "0");

    }

    //draw a border around the drawing area
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}

int PDArea::trans_x(double x)
{
    return round( 40 + (width()-80)*x/current_max );
}

int PDArea::trans_y(double y)
{
    return round( height() - 20 - (height()-80)*y/current_max );
}
