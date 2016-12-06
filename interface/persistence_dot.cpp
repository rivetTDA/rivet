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

#include "persistence_dot.h"

#include "config_parameters.h"
#include "persistence_diagram.h"

#include <QDebug>

PersistenceDot::PersistenceDot(PersistenceDiagram* p_diagram, ConfigParameters* params, double unscaled_x, double unscaled_y, unsigned multiplicity, double radius, unsigned index)
    : pdgm(p_diagram)
    , config_params(params)
    , x(unscaled_x)
    , y(unscaled_y)
    , multiplicity(multiplicity)
    , radius(radius)
    , pressed(false)
    , hover(false)
{
    indexes.push_back(index);

    setAcceptHoverEvents(true);
    //    setFlag(ItemSendsGeometryChanges);
}

QRectF PersistenceDot::boundingRect() const
{
    return QRectF(-radius, -radius, 2 * radius, 2 * radius);
}

void PersistenceDot::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*unused*/, QWidget* /*unused*/)
{
    QRectF rect = boundingRect();
    QBrush brush(config_params->persistenceColor);

    if (hover)
        painter->setPen(QPen(QBrush(config_params->persistenceHighlightColor), 3));
    else
        painter->setPen(Qt::NoPen);

    if (pressed)
        brush.setColor(config_params->persistenceHighlightColor);

    painter->setRenderHint(QPainter::Antialiasing);
    //    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);
    painter->drawEllipse(rect);
}

void PersistenceDot::select()
{
    pressed = true;
    update(boundingRect());
}

void PersistenceDot::deselect()
{
    pressed = false;
    update(boundingRect());
}

//adds another "barcode" index to this dot
void PersistenceDot::add_index(unsigned index)
{
    indexes.push_back(index);
}

//increases the multiplicity of this dot by m
void PersistenceDot::incr_multiplicity(unsigned m)
{
    multiplicity += m;
}

//returns the unscaled x-coordinate associated with this dot
double PersistenceDot::get_x()
{
    return x;
}

//returns the unscaled y-coordinate associated with this dot
double PersistenceDot::get_y()
{
    return y;
}

//returns the "barcode" indexes of this dot (e.g. to send to the SliceDiagram for highlighting effects)
std::vector<unsigned> PersistenceDot::get_indexes()
{
    return indexes;
}

//returns the multiplicity of this dot
unsigned PersistenceDot::get_multiplicity()
{
    return multiplicity;
}

//sets a new radius and re-draws the dot
void PersistenceDot::set_radius(double r)
{
    radius = r;
    update(boundingRect());
}

void PersistenceDot::hoverEnterEvent(QGraphicsSceneHoverEvent* /*unused*/)
{
    hover = true;
    update(boundingRect());
}

void PersistenceDot::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*unused*/)
{
    hover = false;
    update(boundingRect());
}

void PersistenceDot::mousePressEvent(QGraphicsSceneMouseEvent* /*unused*/)
{
    if (pressed) //then unselect this dot
    {
        pressed = false;
        //call persistence diagram unselect function
        pdgm->deselect_dot();
    } else //then select this dot
    {
        pressed = true;
        //call persistence diagram select function
        pdgm->select_dot(this);
    }

    update(boundingRect());
    //    QGraphicsItem::mousePressEvent(event);
}

void PersistenceDot::mouseReleaseEvent(QGraphicsSceneMouseEvent* /*unused*/)
{
    //    pressed = false;
    //    update();
    //    QGraphicsItem::mouseReleaseEvent(event);
}
