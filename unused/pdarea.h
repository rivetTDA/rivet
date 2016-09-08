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

class PDArea : public QWidget {
    Q_OBJECT

public:
    PDArea(QWidget* parent = 0);

    void setData(std::vector<std::pair<double, double>>* p, std::vector<double>* c);

public slots:
    void drawDiagram();

    void setMax(double m);

    void setScale(double s);

    double fitScale();

protected:
    void paintEvent(QPaintEvent* event);

private:
    bool has_data;

    std::vector<std::pair<double, double>>* pairs;
    std::vector<double>* cycles;

    double current_max; //current max values for persistence diagram (min value is always zero)
    double default_max; //default max value for persistence diagram, determined by slice of xi support points
    double scale_multiplier; //factor by which the default scale is multiplied

    int trans_x(double x); //translates from logical to screen coordinates (horizontal)
    int trans_y(double y); //translates from logical to screen coordinates (vertical)
};

#endif
