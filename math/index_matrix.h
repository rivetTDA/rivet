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

/*
An IndexMatrix is a 2-D array data structure intended to store a list of colexicographically ordered (not necessarily distinct) bigrades in a compressed fashion.  This is to be used together with the MapMatrix data structure.  In fact, the IndexMatrix   will only provide a parsimonious representation of the list under the assumption that the number of distant x-and y-grades in the list is relatively small compared to the number of items on the list.  Thus, this is an efficient representation to work with when the input bifiltration has been heavily coarsened.
*/

/*Our convention for IndexMatrices is that the entry at (row, col) is to be the greatest column index of all columns that
appear at or before (row, col) in colexicographical order, or -1
if there are no such columns.  The function get() retrieves this entry.  
 
TODO: It might be natural to have a constructor to build an index matrix from a colexically ordered list of bigrades.  Right now the construction happens in the FIRep constructor and is kind of ad hoc.  On the other hand, such a constructor seems not to fit the design of the FIRep class well, which takes pains to avoid explicitly constructing big lists of colexically ordered bigrades that we ultimately do not store or use.
 */
 
class IndexMatrix {
public:
    //Initialize each entry in the matrix to be -1.
    IndexMatrix(unsigned rows, unsigned cols);

    void set(unsigned row, unsigned col, int value);
    
    //see remarks above for explanation
    int get(unsigned row, unsigned col) const;

    int last() const;

    unsigned width() const; //returns number of columns
    unsigned height() const; //returns number of rows
    
    /*
    Input: any pair of integers in [0,row-1]x[0,col-1]
    Output: next_colex(num_rows-1,num_columns-1) returns (num_rows,0).  For all other input, returns next index in [0,row-1]x[0,col-1] w.r.t. colex order.
    NOTE: directly increments row and col.
    */
    void next_colex(unsigned& row, unsigned& col);
    
    /*
    This is a sort of "dual" to get().
    Input: any pair of integers in [0,row-1]x[0,col-1]
    Output: If mat has a column of grade (row,column) then start_index(row,col) is the index of the smallest such column.
    If mat has no such columns, then start_index(row,col) returns  the smallest index of a column with grade colexicgraphically larger than (row,col), if such a column exists, or mat.width() otherwise.
    */
    virtual unsigned start_index(unsigned& row, unsigned& col);
    
    void print() const; //prints the matrix
    
private:
    unsigned num_rows;
    unsigned num_cols;
    std::vector<int> data;
};


/*As an intermediate step in the construction of a colex-ordered basis for the kernel of a bigraded matrix, we store the columns of this matrix in lex order.  We then put them in colex order.  For this intermediate step, it is useful to have a variant of the IndexMatrix ended specifically for lex-ordered lists of bigrades.  We implement this as a child class IndexMatLex

    Note: The get() function for IndexMatrixLet is inherited from IndexMatrix, but the start_index() functions are different.
 */

class IndexMatrixLex: public IndexMat {

    //constructor
    IndexMatrixLex::IndexMatrixLex(unsigned rows, unsigned cols)
    : IndMatrix(rows,cols)
    {}
    
    //analogous to start_index for IndexMa, but defined w.r.t. lex order.
    unsigned start_index(unsigned& row, unsigned& col);

    
    
    

#endif // __IndexMatrix_H__
