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

class SliceArea : public QWidget
{
    Q_OBJECT

public:
    SliceArea(double gcdx, double gcdy, double lcmx, double lcmy, QWidget *parent = 0);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public slots:
    void setLine(int angle, int offset);

protected:
    void paintEvent(QPaintEvent *event);

private:
    const int size;   //default size is a square with this side length
    double fill_pct;        //horizontal and vertical percent of the square that is filled by the diagram

    double gx, gy;  //coordinates of the GCD
    double lx, ly;  //coordinates of the LCM

    int angle, offset;  //parameters for the slice (line)

    QRect *bound_rect;

    int trans_x(double x);   //translates from logical to screen coordinates (horizontal)
    int trans_y(double y);   //translates from logical to screen coordinates (vertical)
};

#endif
