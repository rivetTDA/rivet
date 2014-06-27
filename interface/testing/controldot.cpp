#include "controldot.h"

ControlDot::ControlDot(QGraphicsItem *parent) :
    QGraphicsEllipseItem(parent)
{
}

ControlDot::ControlDot(qreal x, qreal y, qreal width, qreal height, QGraphicsItem* parent = 0) :
    QGraphicsEllipseItem(x, y, width, height, parent)
{
}
