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
// Created by Bryn Keller on 6/29/16.
//

#ifndef RIVET_CONSOLE_POINTER_COMPARATOR_H
#define RIVET_CONSOLE_POINTER_COMPARATOR_H

template <typename T, typename Comparator>
class PointerComparator {

public:
    Comparator comparator;

    bool operator()(const std::unique_ptr<T> &lhs, const std::unique_ptr<T> &rhs) const //returns true if lhs comes before rhs
    {
        return comparator(*lhs, *rhs);
    }
    bool operator()(const std::shared_ptr<T> lhs, const std::shared_ptr<T> rhs) const //returns true if lhs comes before rhs
    {
        return comparator(*lhs, *rhs);
    }
    bool operator()(const T* lhs, const T* rhs) const //returns true if lhs comes before rhs
    {
        return comparator(*lhs, *rhs);
    }
};
#endif //RIVET_CONSOLE_POINTER_COMPARATOR_H
