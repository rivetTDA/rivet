#ifndef CONTROL_DOT_H
#define CONTROL_DOT_H


#include <QPainter>
#include <QGraphicsItem>

#include "config_parameters.h"
class SliceDiagram;
#include "slice_diagram.h"
class SliceLine;
#include "slice_line.h"



class ControlDot : public QGraphicsItem
{
public:
    ControlDot(SliceLine* line, bool left_bottom, ConfigParameters* params);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void set_position(const QPointF &newpos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    SliceLine* slice_line;
    QGraphicsTextItem* coords;
    ConfigParameters* config_params;

    bool pressed;
    bool left_bottom;   //TRUE if this is a left-bottom control dot, FALSE if this is a right-top control dot
    bool update_lock;   //TRUE when the dot is being moved as result of external input; to avoid update loops

};

#endif // CONTROL_DOT_H
