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

#include "index_matrix.h"
#include "debug.h"

#include <stdexcept> //for error-checking and debugging

IndexMatrix::IndexMatrix(unsigned rows, unsigned cols)
    : num_rows(rows)
    , num_cols(cols)
    //initialize each entry to be -1.
    , data(rows * cols,-1)
{
}

void IndexMatrix::set(unsigned row, unsigned col, int value)
{
    if (row >= num_rows || col >= num_cols)
        throw std::runtime_error("IndexMatrix.set(): matrix subscript out of bounds");
    data[num_cols * row + col] = value;
}

int IndexMatrix::get(unsigned row, unsigned col) const
{
    if (row >= num_rows || col >= num_cols)
        throw std::runtime_error("IndexMatrix.get(): matrix subscript out of bounds");
    return data[num_cols * row + col];
}

int IndexMatrix::last() const
{
    return data[num_cols * num_rows - 1];
}

unsigned IndexMatrix::width() const
{
    return num_cols;
}

unsigned IndexMatrix::height() const
{
    return num_rows;
}

void IndexMatrix::next_colex(int & row, int & col)
{
    if (row >= (int) num_rows || col >= (int) num_cols)
        throw std::runtime_error("IndexMatrix.next_colex(): matrix subscript out of bounds");
    if (col < (int) num_cols-1)
        col++;
    else
    {
        col = 0;
        row++;
    }
}

int IndexMatrix::start_index(unsigned row, unsigned col)
{
    if (row == 0 && col == 0)
        return 0;

    if (col > 0)
        return 1 + get(row,col-1);
        
    //if we get here, then row>0 and col == 0.
    return 1 + get(row-1,num_cols-1);
}

void IndexMatrix::fill_index_mx(Grade& start_grade, const Grade& end_grade, const unsigned& value)
{
    Grade& current_grade = start_grade;
    while (! (current_grade == end_grade) )
    {
        //set value of index matrix
        set(current_grade.y, current_grade.x,value);
        
        //increment w.r.t. colex order on the grid of bigrades.
        next_colex(current_grade.y, current_grade.x);
    }
}

//returns the number of columns in the bigraded matrix of index
//less than or equal to the given bigraded in the partial order on R^2
unsigned IndexMatrix::num_columns_leq(unsigned row, unsigned col)
{
    if (row==0 || col == num_cols-1)
        return get(row, col)+1;
    else
        return get(row, col)-get(row-1, num_cols-1)+get(row-1,col)+1;
}


//function to print the matrix to standard output, for testing purposes
void IndexMatrix::print() const
{
    //handle empty matrix
    if (num_rows == 0 || num_cols == 0) {
        debug() << "        (empty matrix:" << num_rows << "rows by" << num_cols << "columns)";
        return;
    }

    debug() << "        (matrix:" << num_rows << "rows by" << num_cols << "columns)";

    //print the matrix
    for (unsigned i = 0; i < num_rows; i++) {
        Debug qd = debug(true);
        qd << "        |";
        for (unsigned j = 0; j < num_cols; j++) {
            qd << " " << data[num_cols * i + j];
        }
        qd << " |\n";
    }
} //end print()


/********** Defs for IndexMatrixLex **********/

//constructor simply calls the constructor for the parent class
IndexMatrixLex::IndexMatrixLex(unsigned rows, unsigned cols)
    : IndexMatrix(rows, cols)
{}


int IndexMatrixLex::start_index(unsigned row, unsigned col)
{
    if (row == 0 && col == 0)
        return 0;
    else
    {
        if (row > 0)
            return 1 + get(row-1,col);
        //if we get here, then col > 0 and row == 0.
        else
            return 1 + get(num_rows-1,col-1);
    }
}
