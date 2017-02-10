/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
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

#include "anchor.h"
#include "math/template_points_matrix.h"
#include <ostream>

Anchor::Anchor(std::shared_ptr<TemplatePointsMatrixEntry> e)
    : x_coord(e->x)
    , y_coord(e->y)
    , entry(e)
    , above_line(true)
{
}

Anchor::Anchor(unsigned x, unsigned y)
    : x_coord(x)
    , y_coord(y)
    , entry()
    , dual_line()
    , position(0)
    , above_line(false)
{
}

std::ostream& operator<<(std::ostream& stream, const Anchor& anchor)
{
    stream << "Anchor(" << anchor.get_x() << ", " << anchor.get_y() << ")";
    return stream;
}

Anchor::Anchor()
    : x_coord(0)
    , y_coord(0)
    , entry()
    , dual_line()
    , position()
    , above_line(false)
    , weight(0)
{
}

bool Anchor::operator==(const Anchor& other) const
{
    return (x_coord == other.x_coord && y_coord == other.y_coord);
}

bool Anchor::comparable(const Anchor& other) const //tests whether two Anchors are (strongly) comparable
{
    return (y_coord > other.get_y() && x_coord > other.get_x()) || (y_coord < other.get_y() && x_coord < other.get_x());
}

unsigned Anchor::get_x() const
{
    return x_coord;
}

unsigned Anchor::get_y() const
{
    return y_coord;
}

void Anchor::set_line(std::shared_ptr<Halfedge> e)
{
    dual_line = e;
}

std::shared_ptr<Halfedge> Anchor::get_line() const
{
    return dual_line;
}

void Anchor::set_position(unsigned p)
{
    position = p;
}

unsigned Anchor::get_position() const
{
    return position;
}

bool Anchor::is_above()
{
    return above_line;
}

void Anchor::toggle()
{
    above_line = !above_line;
}

std::shared_ptr<TemplatePointsMatrixEntry> Anchor::get_entry()
{
    return entry;
}

void Anchor::set_weight(unsigned long w)
{
    weight = w;
}

unsigned long Anchor::get_weight()
{
    return weight;
}
