#include <sstream>
#include <QDebug>

#include "persistence_diagram.h"

PersistenceDiagram::PersistenceDiagram(QGraphicsScene* sc, double length, double scale, double zero) :
    scene(sc),
    width(length/sqrt(2)), scale(scale/sqrt(2)),  //divide by sqrt(2) because input gives measurements for the diagonal of the diagram
    zero_coord(zero)
{
    draw_frame();
}


void PersistenceDiagram::draw_frame()
{
    //define pens
    QPen grayPen(QBrush(Qt::darkGray),2,Qt::DotLine, Qt::RoundCap, Qt::RoundJoin);
    QPen bluePen(QBrush(Qt::blue),3,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    //draw persistence diagram structure
    bounding_rect = scene->addRect(0, 0, width, width, grayPen);
    diag_line = scene->addLine(0, 0, width, width, bluePen);

    h_line = scene->addLine(0, width + 25, width + 30, width + 25, grayPen);
    v_line = scene->addLine(width, width + 5, width, width + 45, grayPen);

    //draw text
    inf_text = scene->addSimpleText("inf");
    inf_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    inf_text->setPos(-inf_text->boundingRect().width() - 10, width + 45);

    lt_inf_text = scene->addSimpleText("<inf");
    lt_inf_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    lt_inf_text->setPos(-lt_inf_text->boundingRect().width() - 10, width + 20);
}

void PersistenceDiagram::draw_points(std::vector< std::pair<double,double> >* pairs, std::vector<double>* cycles)
{
    QBrush blackBrush(Qt::black);

    int radius = 4;                     //TODO: IMPROVE!!!

    qDebug() << "ZERO: " << zero_coord;

    //draw cycles
    int num_big_cycles = 0;
    for(int i=0; i<cycles->size(); i++)
    {
        double x = ((*cycles)[i] - zero_coord)*scale;

        if(x > width)
            num_big_cycles++;
        else
            scene->addEllipse(x - radius, width + 35 - radius, 2*radius, 2*radius, Qt::NoPen, blackBrush);
    }

    //draw pairs
    int num_big_points = 0;
    for(int i=0; i<pairs->size(); i++)
    {
        double x = ((*pairs)[i].first - zero_coord)*scale;
        double y = ((*pairs)[i].second - zero_coord)*scale;

        if(x > width)
            num_big_points++;
        else
        {
            if(y > width)
                scene->addEllipse(x - radius, width + 15 - radius, 2*radius, 2*radius, Qt::NoPen, blackBrush);
            else
                scene->addEllipse(x - radius, y - radius, 2*radius, 2*radius, Qt::NoPen, blackBrush);
        }
    }

    //draw counts
    std::ostringstream scyc;
    scyc << num_big_cycles;
    inf_count_text = scene->addSimpleText(QString(scyc.str().data()));
    inf_count_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    inf_count_text->setPos(width + 5, width + 45);

    std::ostringstream spts;
    spts << num_big_points;
    lt_inf_count_text = scene->addSimpleText(QString(spts.str().data()));
    lt_inf_count_text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    lt_inf_count_text->setPos(width + 5, width + 20);
}
