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

#include "barcode.h"

#include <debug.h>
#include <math.h>

MultiBar::MultiBar(double b, double d, unsigned m)
    : birth(b)
    , death(d)
    , multiplicity(m)
{
}

MultiBar::MultiBar(const MultiBar& other)
    : birth(other.birth)
    , death(other.death)
    , multiplicity(other.multiplicity)
{
}

//comparison operator for sorting long bars first
//uses approximate comparisons to avoid "flickering" of bars of nearly equal length
bool MultiBar::operator<(const MultiBar other) const
{
    double epsilon = pow(10, -8);

    //first, sort by length of bar
    if ((death - birth) > (other.death - other.birth + epsilon))
        return true;

    //if bars have (almost) the same length, then sort by birth time
    if ((death - birth) >= (other.death - other.birth - epsilon))
        return (birth < other.birth);

    //else
    return false;
}

Barcode::Barcode()
{
}

//adds a bar to the barcode
void Barcode::add_bar(double b, double d, unsigned m)
{
    bars.insert(MultiBar(b, d, m));
}

//shifts the barcode by adding amount to each endpoint of each bar
std::unique_ptr<Barcode> Barcode::shift(double amount)
{
    std::unique_ptr<Barcode> bc = std::unique_ptr<Barcode>(new Barcode());

    for (std::multiset<MultiBar>::iterator it = bars.begin(); it != bars.end(); ++it) {
        MultiBar mb = *it;
        bc->add_bar(mb.birth + amount, mb.death + amount, mb.multiplicity);
    }

    return bc;
}

//returns an iterator to the first bar in the barcode
std::set<MultiBar>::const_iterator Barcode::begin() const
{
    return bars.cbegin();
}

//returns an iterator to the pst-the-end element the barcode
std::set<MultiBar>::const_iterator Barcode::end() const
{
    return bars.cend();
}

//returns the number of multibars in the barcode
unsigned Barcode::size() const
{
    return bars.size();
}

//for testing only
void Barcode::print() const
{
    Debug qd = debug(true);
    qd << "      rescaled barcode: ";
    for (std::multiset<MultiBar>::iterator it = bars.begin(); it != bars.end(); ++it) {
        MultiBar b = *it;
        qd << "(" << b.birth << "," << b.death << ")x" << b.multiplicity << ", ";
    }
}
