/****************************************************************************
* this class creates a QWidget for a persistence diagram
*
* Matthew L. Wright
* 2014
****************************************************************************/

#include <QtGui>

#include "pdarea.h"



PDArea::PDArea(QWidget *parent)
    : QWidget(parent), has_data(false),
      min(0), max(100)
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

        //set the default extents       //TODO: HANDLE THE CASE WHERE THERE IS ONLY ONE POINT (i.e. min = max)
        if(pairs->size() > 0)
        {
            min = (*pairs)[0].first;
            max = (*pairs)[0].second;

            for(int i=1; i<pairs->size(); i++)
            {
                if((*pairs)[i].first < min)
                    min = (*pairs)[i].first;
                if((*pairs)[i].second > max)
                    max = (*pairs)[i].second;
            }
        }
        else
        {
            min = (*cycles)[0];
            max = (*cycles)[0];
        }
        for(int i=0; i<cycles->size(); i++)
        {
            if((*cycles)[i] < min)
                min = (*cycles)[i];
            if((*cycles)[i] > max)
                max = (*cycles)[i];
        }

        qDebug() << "\n min: " << min << ", max: " << max << "\n";


        //define pens
        QPen gray(QBrush(Qt::darkGray),2,Qt::DotLine, Qt::RoundCap, Qt::RoundJoin);
        QPen thickblack(QBrush(Qt::black),6,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        //draw items above persistence diagram
        painter.setPen(gray);
        painter.drawLine(QLine(10, 25, width()-11, 25));
        painter.drawLine(QLine(width()-40, 5, width()-40, 45));

        painter.drawText(QPoint(10, 20), "inf");
        painter.setPen(thickblack);
        for(int i=0; i<cycles->size(); i++)
        {
            painter.drawPoint(trans_x((*cycles)[i]), 12);
        }
//        painter.drawText(QPoint(width()-35, 20), "2");
        painter.setPen(gray);
        painter.drawText(QPoint(10, 45), "<inf");
//        painter.drawText(QPoint(width()-35, 45), "3");

        //draw persistence diagram
//        painter.setPen(QPen(QBrush(Qt::darkGray),2,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawRect(QRect(40, 50, width()-80, height()-60));
        painter.drawLine(QLine(40, height()-10, width()-40, 50));
        painter.setPen(thickblack);
        for(int i=0; i<pairs->size(); i++)
        {
            painter.drawPoint( trans_x((*pairs)[i].first), trans_y((*pairs)[i].second) );
        }
//        painter.setPen(QPen(QBrush(Qt::black),6,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//        static const QPoint points[15] = {
//            QPoint(80,150), QPoint(200, 170), QPoint(80, 300), QPoint(300, 100), QPoint(270, 130),
//            QPoint(100,170), QPoint(260, 130), QPoint(180, 230), QPoint(330, 80), QPoint(250, 155),
//            QPoint(90,12), QPoint(210, 12), QPoint(60, 38), QPoint(310, 38), QPoint(240, 38)
//        };
//        painter.drawPoints(points, 15);
    }

    //draw a border around the drawing area
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}

int PDArea::trans_x(double x)
{
    return round( 40 + (width()-80)*(x-min)/(max-min) );
}

int PDArea::trans_y(double y)
{
    return round( height() - 10 - (height()-60)*(y-min)/(max-min) );
}
