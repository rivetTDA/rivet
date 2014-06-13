/****************************************************************************
* this class creates a QWidget for a persistence diagram
*
* Matthew L. Wright
* 2014
****************************************************************************/

#ifndef PDAREA_H
#define PDAREA_H

#include <QBrush>
#include <QPen>
#include <QWidget>

class PDArea : public QWidget
{
    Q_OBJECT

public:
    PDArea(QWidget *parent = 0);

    void setData(std::vector< std::pair<double,double> >* p, std::vector<double>* c);

public slots:
    void drawDiagram();

protected:
    void paintEvent(QPaintEvent *event);

private:
    bool has_data;

    std::vector< std::pair<double,double> >* pairs;
    std::vector<double>* cycles;

    double min, max;  //data extents

    int trans_x(double x);   //translates from logical to screen coordinates (horizontal)
    int trans_y(double y);   //translates from logical to screen coordinates (vertical)
};

#endif
