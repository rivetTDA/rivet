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
#include "grade.h"

// The entry at (row, col) is to be the greatest column index of all columns that
// appear at or before (row, col) in colexicographical order, or -1
// if there are no such columns
class IndexMatrix {
public:
    //Initialize each entry in the matrix to be -1.
    IndexMatrix(unsigned rows, unsigned cols);

    void set(unsigned row, unsigned col, int value);
    
    //Returns the entry at (row,col) (see above).
    int get(unsigned row, unsigned col) const;

    int last() const;

    unsigned width() const; //returns number of columns
    unsigned height() const; //returns number of rows

    //utility function which gives the next index in the matrix, w.r.t. colex order.
    //NOTE: directly increments row and col.  For the last index (num_rows-1,num_columns-1), this will increment
    //to (num_rows,0) which is outside of the grid.  This edge case behavior is convenient in the firep class.
    // For input indices not in the grid, an error is thrown.
    
    //TODO:Make static?
    void next_colex(int & row, int & col);
    
    //returns the index of the first column whose bigrade is at least (row,col) in colex order, or one larger than the largest column index if there is no such column.
    //returns an int for consisteny with get.
    int start_index(unsigned row, unsigned col);
    
    
    //returns the number of columns in the bigraded matrix of index
    //less than or equal to the given bigraded in the partial order on R^2
    unsigned num_columns_leq(unsigned row, unsigned col);
    
    //technical utility function for setting the index matrices.
    //sets each entry of the index matrix in the colex interval [start_grade,end_grade) to value
    //As a side effect, sets start_grade equal to end_grade
    void fill_index_mx(Grade& start_grade, const Grade& end_grade, const unsigned& value);
    
    void print() const; //prints the matrix
    
protected:
    unsigned num_rows;
    unsigned num_cols;
    std::vector<int> data;
};

class IndexMatrixLex : public IndexMatrix {
public:
    //Initialize each entry in the matrix to be -1.
    IndexMatrixLex(unsigned rows, unsigned cols);
    
    //definition of start_index is different than for the parent class
    int start_index(unsigned row, unsigned col);
};


#endif // __IndexMatrix_H__
