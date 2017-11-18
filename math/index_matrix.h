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
 * \class	IndexMatrix
 * \brief	Stores a matrix of column indexes, one for each bigrade.  This, together with a MapMatrix, represents a morphism of free bigraded modules.
 * \author	Matthew L. Wright
 * \date	July 2014
 */

#ifndef __IndexMatrix_H__
#define __IndexMatrix_H__

#include <vector>

// The entry at (row, col) is to be the greatest column index of all columns that
// appear at or before (row, col) in colexicographical order, or -1
// if there are no such columns
class IndexMatrix {
public:
    //Initialize each entry in the matrix to be -1.
    IndexMatrix(unsigned rows, unsigned cols);

    void set(unsigned row, unsigned col, int value);
    int get(unsigned row, unsigned col) const;

    int last() const;

    unsigned width() const; //returns number of columns
    unsigned height() const; //returns number of rows

    void print() const; //prints the matrix

private:
    unsigned num_rows;
    unsigned num_cols;
    std::vector<int> data;
};

#endif // __IndexMatrix_H__
