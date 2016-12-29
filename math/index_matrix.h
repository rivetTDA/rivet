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
/**
 * \class	IndexMatrix
 * \brief	Stores a matrix of column indexes, one for each multi-grade (to accompany a MapMatrix).
 * \author	Matthew L. Wright
 * \date	July 2014
 */

#ifndef __IndexMatrix_H__
#define __IndexMatrix_H__

// Holds information about the simplices of one particular dimension that exist at each
// multigrade. Note that an IndexMatrix instance is specific to one simplex dimension,
// e.g. it might track 0-simplices or 3-simplices. The indices themselves
// are meaningful only in relation to a particular SimplexTree instance.
//
// The entry at (row, col) is to be the greatest dim_index of all simplices that
// appear at or before this multigrade in reverse lexicographical order, or -1
// if there are no such simplices
class IndexMatrix {
public:
    IndexMatrix(unsigned rows, unsigned cols);
    ~IndexMatrix();

    void set(unsigned row, unsigned col, int value);
    int get(unsigned row, unsigned col) const;

    int last() const;

    unsigned width() const; //returns number of columns
    unsigned height() const; //returns number of rows

private:
    unsigned num_rows;
    unsigned num_cols;
    int* data;
};

#endif // __IndexMatrix_H__
