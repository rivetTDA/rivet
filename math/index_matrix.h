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
    
    //for (row,col) in [0,row-1]x[0,col-1], returns the entry at (row,col), as described above.
    //for (row,col) = (-1,0), returns
    int get(unsigned row, unsigned col) const;

    int last() const;

    unsigned width() const; //returns number of columns
    unsigned height() const; //returns number of rows

    //Input: any pair of integers in [0,row-1]x[0,col-1]
    //Output: next_colex(num_rows-1,num_columns-1) returns (num_rows,0).  For all other input, returns next index in [0,row-1]x[0,col-1] w.r.t. colex order.
    //For (row,col) not in the grid, an error is thrown.
    //NOTE: directly increments row and col.
    void next_colex(unsigned& row, unsigned& col);
    
    //Input: any pair of integers in [0,row-1]x[0,col-1]
    //Output: If mat has a column of grade (row,column) then start_index(row,col) is the index of the smallest such column.
    // If mat has no such columns, then start_index(row,col) returns  the smallest index of a column with grade colexicgraphically larger than (row,col), if such a column exists, or mat.width() otherwise.
    unsigned start_index(unsigned& row, unsigned& col);
    
    //Input: any pair of integers in [0,row-1]x[0,col-1]
    //Output: prev_colex(0,0) returns (-1,0).  For all other input, returns previous index in [0,row-1]x[0,col-1] w.r.t. colex order.
    //For (row,col) not in the grid, an error is thrown.
    //NOTE: directly decrements row and col.
    void prev_colex(unsigned& row, unsigned& col);
    
    void print() const; //prints the matrix
    
private:
    unsigned num_rows;
    unsigned num_cols;
    std::vector<int> data;
    

    
};

#endif // __IndexMatrix_H__
