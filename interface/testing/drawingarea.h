#ifndef DRAWINGAREA_H
#define DRAWINGAREA_H

#include <QBrush>
#include <QPen>
#include <QWidget>
#include <QPainter>

class DrawingArea : public QWidget
{
    Q_OBJECT

public:
    DrawingArea(QWidget *parent = 0);



public slots:

protected:
    void paintEvent(QPaintEvent *event);

private:
    int num;
};

#endif // DRAWINGAREA_H
