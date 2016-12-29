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

#ifndef RIVET_DEBUG_H
#define RIVET_DEBUG_H

#include <iostream>
#include <memory>

#ifdef QT_CORE_LIB

#include <QDebug>
using Debug = QDebug;

Debug debug(bool nospace = false);

#else

struct Debug {
    std::ostream& os;
    bool space;
    bool alive;

    Debug(std::ostream& os = std::clog, bool space = true)
        : os(os)
        , space(space)
        , alive(true)
    {
    }
    Debug(Debug&& rhs)
        : os(rhs.os)
        , space(rhs.space)
        , alive(true)
    {
        rhs.alive = false;
    }
    Debug(Debug& rhs)
        : os(rhs.os)
        , space(rhs.space)
        , alive(true)
    {
        rhs.alive = false;
    }
    ~Debug()
    {
        if (alive && space)
            os << std::endl;
    }
};

template <typename T>
Debug operator<<(Debug&& d, T const& x)
{
    (d.os) << x;
    if (d.space)
        d.os << " ";
    return std::move(d);
}

template <typename T>
Debug operator<<(Debug& d, T const& x)
{
    d.os << x;
    if (d.space)
        d.os << " ";
    return std::move(d);
}

Debug debug(bool nospace = false, std::ostream& out = std::clog);

#endif
#endif
