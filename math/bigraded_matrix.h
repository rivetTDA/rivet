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
   
 Author: Michael Lesnick (2017)
 
 
 Class: BigradedMatrix
 
 Description: This is a wrapper class which stores a MapMatrix
 and an associated IndexMatrix.  Most important method is kernel().
 Used when columns are sorted in colex order of their bigrades of appearance.
   
 
 Class: BigradedMatrixLex
 
 Description: This is a similar wrapper class which stores a MapMatrix and an 
 associated IndexMatrixLex.  Used when columns are sorted in lex order of their 
 bigrades of appearance.
 
*/

#ifndef __Bigraded_Matrix_H__
#define __Bigraded_Matrix_H__

#include "index_matrix.h"
#include "map_matrix.h"

//forward declarations
class MapMatrix;
class vector_heap;
class IndexMatrix;
class IndexMatrixLex;

class BigradedMatrixLex;
class BigradedMatrix {
public:
    //column-sparse matrix
    MapMatrix mat;
    //bigrade info for each column of mat.  Columns are assumed to be in colex
    //order.
    IndexMatrix ind;

    //Constructor.  Builds an zero matrix of the appropropriate dimensions
    BigradedMatrix(unsigned rows, unsigned cols, unsigned ind_rows, unsigned ind_cols);

    //Constructor.  Copies the arguments.
    BigradedMatrix(const MapMatrix& m, const IndexMatrix& i);

    /*
    Constructor taking a BigradedMatrixLex object.  Moves the cols of lex_mat
    into this matrix, and builds the corresponding index matrix.
    Also trivializes lex_mat in the process.
    */
    BigradedMatrix(BigradedMatrixLex& lex_mat);

    //Compute the kernel of this bigraded matrix via a standard reduction:
    //NOTE: This destroys the matrix.
    BigradedMatrix kernel();

    void print();

private:
    /*
     Performs a step of the kernel computation at a single bigrade. This is a 
     variant on the standard bigraded reduction.  When a column in mat is zeroed
     out, the corresponding column of slave is appended to the back 
     working_ker.mat, and then zeroed out in the slave.  The function also 
     records the bigrades of the generators for the kernel by updating 
     working_ker.ind.
     */
    void kernel_one_bigrade(MapMatrix& slave,
        BigradedMatrixLex& ker_lex,
        unsigned curr_x,
        unsigned curr_y,
        std::vector<int>& lows);
};

//Similar to BigradedMatrix, but columns are assumed to be in lex order.
class BigradedMatrixLex {
public:
    MapMatrix mat;
    IndexMatrixLex ind;

    //constructor
    BigradedMatrixLex(unsigned rows,
        unsigned cols,
        unsigned ind_rows,
        unsigned ind_cols)
        : mat(rows, cols)
        , ind(ind_rows, ind_cols)
    {
    }

    void print();
};

#endif // __Bigraded_Matrix_H__
