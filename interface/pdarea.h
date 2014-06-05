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

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public slots:
    //void setPen(const QPen &pen);

protected:
    void paintEvent(QPaintEvent *event);

private:
    //QPen pen;

};

#endif
