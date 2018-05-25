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
/**
 * \class   Anchor
 * \brief   Stores an Anchor: a bigrade along with a pointer to the line representing the Anchor in the arrangement
 * \author  Matthew L. Wright
 * \date    March 2014
 */

#ifndef __ANCHOR_H__
#define __ANCHOR_H__

#include <memory>

//forward declarations
class Halfedge;
struct TemplatePointsMatrixEntry;

class Anchor {
public:
    Anchor(TemplatePointsMatrixEntry* e); //default constructor
    Anchor(unsigned x, unsigned y); //constructor, requires only x- and y-coordinates
    Anchor(); //For serialization

    bool operator==(const Anchor& other) const; //equality operator

    bool comparable(const Anchor& other) const; //tests whether two Anchors are (strongly) comparable

    unsigned get_x() const; //get the discrete x-coordinate
    unsigned get_y() const; //get the discrete y-coordinate

    void set_line(Halfedge* e); //set the pointer to the line corresponding to this Anchor in the arrangement
    Halfedge* get_line() const; //get the pointer to the line corresponding to this Anchor in the arrangement

    void set_position(unsigned p); //sets the relative position of the Anchor line at the sweep line, used for Bentley-Ottmann DCEL construction algorithm
    unsigned get_position() const; //gets the relative position of the Anchor line at the sweep line, used for Bentley-Ottmann DCEL construction algorithm

    bool is_above(); //returns true iff this Anchor is above the current slice line, used for the vineyard-update process of storing persistence data in cells of the arrangement
    void toggle(); //toggles above/below state of this Anchor; called whever the slice line crosses this Anchor in the vineyard-update process of storing persistence data

    TemplatePointsMatrixEntry* get_entry(); //accessor

    void set_weight(unsigned long w); //sets the estimate of the cost of updating the RU-decomposition when crossing this anchor
    unsigned long get_weight(); //returns estimate of the cost of updating the RU-decomposition when crossing this anchor

private:
    unsigned x_coord; //discrete x-coordinate
    unsigned y_coord; //discrete y-coordinate

    TemplatePointsMatrixEntry* entry; //TemplatePointsMatrixEntry at the position of this anchor

    Halfedge* dual_line; //pointer to left-most halfedge corresponding to this Anchor in the arrangement
    unsigned position; //relative position of Anchor line at sweep line, used for Bentley-Ottmann DCEL construction algorithm
    bool above_line; //true iff this Anchor is above the current slice line, used for the vineyard-update process of storing persistence data in cells of the arrangement
    unsigned long weight; //estimate of the cost of updating the RU-decomposition when crossing this anchor
};

template <typename T>
struct AnchorComparator {

public:
    bool operator()(const T& lhs, const T& rhs) const //returns true if lhs comes before rhs
    {
        if (lhs.get_y() > rhs.get_y()) //first compare y-coordinates (reverse order)
            return true;
        else if (lhs.get_y() < rhs.get_y())
            return false;
        return lhs.get_x() < rhs.get_x();
    }
};

struct Anchor_LeftComparator : AnchorComparator<Anchor> {
};

#endif // __ANCHOR_H__
