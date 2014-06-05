/****************************************************************************
* this class creates a QWidget for a persistence diagram
*
* Matthew L. Wright
* 2014
****************************************************************************/

#include <QtGui>

#include "pdarea.h"

PDArea::PDArea(QWidget *parent)
    : QWidget(parent)
{

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

QSize PDArea::minimumSizeHint() const
{
    return QSize(200, 200);
}

QSize PDArea::sizeHint() const
{
    return QSize(400, 400);
}

void PDArea::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    //draw items above persistence diagram
    painter.setPen(QPen(QBrush(Qt::darkGray),2,Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(QLine(10, 25, width()-11, 25));
    painter.drawLine(QLine(width()-40, 5, width()-40, 45));
    painter.drawText(QPoint(10, 20), "inf");
    painter.drawText(QPoint(width()-35, 20), "2");
    painter.drawText(QPoint(10, 45), "<inf");
    painter.drawText(QPoint(width()-35, 45), "3");

    //draw persistence diagram
    painter.setPen(QPen(QBrush(Qt::darkGray),2,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawRect(QRect(40, 50, width()-80, height()-60));
    painter.drawLine(QLine(40, height()-10, width()-40, 50));

    painter.setPen(QPen(QBrush(Qt::black),6,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    static const QPoint points[15] = {
        QPoint(80,150), QPoint(200, 170), QPoint(80, 300), QPoint(300, 100), QPoint(270, 130),
        QPoint(100,170), QPoint(260, 130), QPoint(180, 230), QPoint(330, 80), QPoint(250, 155),
        QPoint(90,12), QPoint(210, 12), QPoint(60, 38), QPoint(310, 38), QPoint(240, 38)
    };
    painter.drawPoints(points, 15);

    //draw a border around the drawing area
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}

