/**********************************************************************
Copyright 2014-2018 The RIVET Developers. See the COPYRIGHT file at
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
/*
 
 Authors: Matthew L. Wright (July 2014), Michael Lesnick (modifications 2017-2018)
 
 
 Class: IndexMatrix
 
 Description: Implicitly stores a list of bigrades in colexicographical order as a (dense) matrix.
 Typically, the list of bigrades corresponds to the columns of a matrix, stored as a MapMatrix object;
 The IndexMatrix and MapMatrix together represent a morphism of free bigraded modules.
 (See also the BigradedMatrix class).

 The entry of the IndexMatrix at (row, col) is to be the greatest column index of all columns that
 appear at or before (row, col) in colexicographical order, or -1
 if there are no such columns.  This representation can save memory when one has 
 relatively few different x-grades and y-grades and many columns.  
 Note though that this representation may be much *less* memory efficient than a naive representation 
 when one has many x- and y-grades.
 
 
 Class: IndexMatrixLex : IndexMatrix
 
 Description: Similar to the parent class, but for lists of bigrades in lexicographical order
 
*/

#ifndef __IndexMatrix_H__
#define __IndexMatrix_H__

#include <vector>
#include "grade.h"

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
    
    //print the colex-ordered vector of bigrades that this object represents
    void print_bigrades_vector() const;
    
protected:
    unsigned num_rows;
    unsigned num_cols;
    std::vector<int> data;
};

//Same as parent class but uses the lex order.
class IndexMatrixLex : public IndexMatrix {
public:
    //Initialize each entry in the matrix to be -1.
    IndexMatrixLex(unsigned rows, unsigned cols);
    
    //definition of start_index is different than for the parent class
    int start_index(unsigned row, unsigned col);
    
    //print the lex-ordered vector of bigrades that this object represents
    void print_bigrades_vector() const;
};


#endif // __IndexMatrix_H__
