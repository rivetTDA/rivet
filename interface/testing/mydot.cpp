#include "mydot.h"

#include <QtGui>
#include <QDebug>
#include <sstream>

MyDot::MyDot(QGraphicsTextItem* coords, MyLine* line) :
    pressed(false), coords(coords), line(line), update_lock(false)
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

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);
    painter->drawEllipse(rect);
}

QVariant MyDot::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemPositionChange && !update_lock)
    {
        QPointF mouse = value.toPointF();
        QPointF newpos(mouse);

        if(mouse.y() > 0 && mouse.y() >= mouse.x())   //then project dot to y-axis
        {
            newpos.setX(0);

            if(mouse.y() > 200)     //don't let dot go off top of box
                newpos.setY(200);
            else if(mouse.y() < 2*mouse.x())    //smooth transition in region around y=x
                newpos.setY(2*(mouse.y()-mouse.x()));
            //otherwise, orthogonal projection onto y-axis
        }
        else if(mouse.x() > 0)   //then project dot to x-axis
        {
            newpos.setY(0);

            if(mouse.x() > 180) //don't let dot go off right side of box
                newpos.setX(180);
            else if(mouse.x() < 2*mouse.y())    //smooth transition in region around y=x
                newpos.setX(2*(mouse.x()-mouse.y()));
            //otherwise, orthongonal projection onto x-axis
        }
        else    //then place dot at origin
        {
            newpos.setX(0);
            newpos.setY(0);
        }

        //coordinates, for testing
        std::ostringstream oss;
        oss << "(" << newpos.x() << "," << newpos.y() << ")";
        coords->setPlainText(QString::fromStdString(oss.str()));
        //coords->setPlainText(QString(oss.str().c_str()));

        //update line position
        qDebug() << "  moving line: (" << newpos.x() << ", " << newpos.y() << ")";
        QPointF delta = newpos - pos();
        line->update_lb_endpoint(delta);

        //return
        return newpos;
    }
    return QGraphicsItem::itemChange(change, value);
}

void MyDot::set_position(const QPointF &newpos)
{
    update_lock = true;

    setPos(newpos);

    update_lock = false;
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
