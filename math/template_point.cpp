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

#include "template_point.h"

TemplatePoint::TemplatePoint(unsigned xc, unsigned yc, int m0, int m1, int m2)
    : x(xc)
    , y(yc)
    , zero(m0)
    , one(m1)
    , two(m2)
{
}

TemplatePoint::TemplatePoint()
    : x(0)
    , y(0)
    , zero(0)
    , one(0)
    , two(0)
{
}

bool operator==(TemplatePoint const& left, TemplatePoint const& right)
{
    return left.one == right.one
        && left.two == right.two
        && left.x == right.x
        && left.y == right.y
        && left.zero == right.zero;
}
