#include "mydot.h"

#include <QDebug>
#include <sstream>

MyDot::MyDot(QGraphicsTextItem* coords) :
    pressed(false), coords(coords)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
}

QRectF MyDot::boundingRect() const
{
    return QRectF(-10,-10,20,20);
}

void MyDot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = boundingRect();
    QBrush brush(Qt::blue);

    if(pressed)
    {
        brush.setColor(Qt::darkCyan);

    }
//    else
//    {
//        brush.setColor(Qt::blue);
//    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);
    painter->drawEllipse(rect);
}

QVariant MyDot::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemPositionChange)
    {
        QPointF mouse = value.toPointF();
        QPointF newpos(mouse);

        if(mouse.y() > 0 && mouse.y() >= mouse.x())   //then lock dot to y-axis
        {
            newpos.setX(0);
            if(mouse.y() > 200)
                newpos.setY(200);
        }
        else if(mouse.x() > 0)   //then lock dot to x-axis
        {
            newpos.setY(0);
            if(mouse.x() > 100)
                newpos.setX(100);
        }
        else    //then place dot at origin
        {
            newpos.setX(0);
            newpos.setY(0);
        }

       std::ostringstream oss;
       oss << "(" << newpos.x() << "," << newpos.y() << ")";
       coords->setPlainText(QString::fromStdString(oss.str()));
       //coords->setPlainText(QString(oss.str().c_str()));

        return newpos;
    }
    return QGraphicsItem::itemChange(change, value);
}

void MyDot::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = true;
    update();
 //   coords->setPlainText("testing");
    QGraphicsItem::mousePressEvent(event);
}

void MyDot::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = false;
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}
