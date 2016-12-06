/**********************************************************************
Copyright 2014-2016 The RIVET Devlopers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "persistence_bar.h"

#include "config_parameters.h"
#include "slice_diagram.h"

PersistenceBar::PersistenceBar(SliceDiagram* s_diagram, ConfigParameters* params, double unscaled_start, double unscaled_end, unsigned index)
    : sdgm(s_diagram)
    , config_params(params)
    , start(unscaled_start)
    , end(unscaled_end)
    , class_index(index)
    , pressed(false)
    , secondary(false)
    , hover(false)
{
    setAcceptHoverEvents(true);
}

QRectF PersistenceBar::boundingRect() const
{
    return shape().boundingRect();
}

QPainterPath PersistenceBar::shape() const
{
    QPainterPath path;
    QPainterPathStroker stroker;
    path.moveTo(0, 0);
    path.lineTo(dx, dy);
    stroker.setWidth(10);
    return stroker.createStroke(path);
}

void PersistenceBar::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*unused*/, QWidget* /*unused*/)
{
    QPen pen(config_params->persistenceColor);
    pen.setWidth(4);

    if (pressed) {
        pen.setColor(config_params->persistenceHighlightColor);
    }
    if (secondary) {
        pen.setStyle(Qt::DashLine);
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen);
    painter->drawLine(0, 0, dx, dy);
}

void PersistenceBar::set_line(double start_x, double start_y, double end_x, double end_y)
{
    setPos(start_x, start_y);
    dx = end_x - start_x;
    dy = end_y - start_y;
}

void PersistenceBar::select()
{
    pressed = true;
    update();
}

void PersistenceBar::select_secondary()
{
    pressed = true;
    secondary = true;
    update();
}

void PersistenceBar::deselect()
{
    pressed = false;
    secondary = false;
    update();
}

double PersistenceBar::get_start()
{
    return start;
}

double PersistenceBar::get_end()
{
    return end;
}

unsigned PersistenceBar::get_index()
{
    return class_index;
}

void PersistenceBar::hoverEnterEvent(QGraphicsSceneHoverEvent* /*unused*/)
{
    hover = true;
    update();
}

void PersistenceBar::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*unused*/)
{
    hover = false;
    update();
}

void PersistenceBar::mousePressEvent(QGraphicsSceneMouseEvent* /*unused*/)
{
    if (pressed) //then unselect this bar
    {
        pressed = false;
        //call slice diagram unselect function
        sdgm->deselect_bar();
    } else //then select this bar
    {
        pressed = true;
        //call slice diagram select function
        sdgm->select_bar(this);
    }

    //    update();
    //    QGraphicsItem::mousePressEvent(event);
}

void PersistenceBar::mouseReleaseEvent(QGraphicsSceneMouseEvent* /*unused*/)
{
    //    pressed = false;
    //    update();
    //    QGraphicsItem::mouseReleaseEvent(event);
}
