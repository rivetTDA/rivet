/****************************************************************************
* this class creates a QWidget for a slice drawing
*
* Matthew L. Wright
* 2014
****************************************************************************/

#ifndef SLICEAREA_H
#define SLICEAREA_H

#include <QBrush>
#include <QPen>
#include <QWidget>

//first, a little struct to organize the data used to draw the points in the SliceArea
struct xiPointSD
{
    double x, y;    //coordinates
    int zero, one;  //multiplicity of xi_0 and xi_1 at this point

    xiPointSD(double xc, double yc, double m0, double m1) : x(xc), y(yc), zero(m0), one(m1)
    { }
};


//now for the SliceArea class
class SliceArea : public QWidget
{
    Q_OBJECT

public:
    SliceArea(QWidget *parent = 0);

    void setExtents(double minx, double maxx, double miny, double maxy);
    void addPoint(double x_coord, double y_coord, int xi0m, int xi1m);

public slots:
    void setLine(int angle, double offset);

protected:
    void paintEvent(QPaintEvent *event);

private:
    double fill_pct;        //horizontal and vertical percent of the square that is filled by the diagram
    double border_width;    //width of the border around the slice diagram
    double unit_radius;     //radius of a disk representing one xi support point

    double gx, gy;  //coordinates of the GCD
    double lx, ly;  //coordinates of the LCM

    std::vector<xiPointSD> points;    //point data to be drawn in the slice area

    int angle;
    double offset;  //parameters for the slice (line)

    int trans_x(double x);   //translates from logical to screen coordinates (horizontal)
    int trans_y(double y);   //translates from logical to screen coordinates (vertical)
};

#endif
