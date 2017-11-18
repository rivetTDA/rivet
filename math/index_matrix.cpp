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

void IndexMatrix::next_colex(unsigned& row, unsigned& col)
{
    if (row >= num_rows || col >= num_cols)
        throw std::runtime_error("IndexMatrix.next_colex(): matrix subscript out of bounds");
    if (col < num_cols-1)
        col++;
    else
    {
        col = 0;
        row++;
    }
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



