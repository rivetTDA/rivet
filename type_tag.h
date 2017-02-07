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

//
// Created by Bryn Keller on 7/14/16.
//

#ifndef RIVET_CONSOLE_TYPE_TAG_H
#define RIVET_CONSOLE_TYPE_TAG_H

//http://www.ilikebigbits.com/blog/2014/5/6/type-safe-identifiers-in-c
template <class Tag, class impl, impl default_value>
class ID {
public:
    static ID invalid() { return ID(); }

    // Defaults to ID::invalid()
    ID()
        : m_val(default_value)
    {
    }

    // Explicit constructor:
    explicit ID(impl val)
        : m_val(val)
    {
    }

    // Explicit conversion to get back the impl:
    explicit operator impl() const { return m_val; }

    friend bool operator==(ID a, ID b) { return a.m_val == b.m_val; }
    friend bool operator!=(ID a, ID b) { return a.m_val != b.m_val; }

    template <class Archive>
    void serialize(Archive& ar, unsigned const int /*version*/)
    {
        ar& m_val;
    }

private:
    impl m_val;
};

#endif //RIVET_CONSOLE_TYPE_TAG_H
