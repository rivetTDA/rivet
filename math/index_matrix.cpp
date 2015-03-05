#include "index_matrix.h"

#include <stdexcept>    //for error-checking and debugging
#include <iostream>     //for testing only

IndexMatrix::IndexMatrix(unsigned rows, unsigned cols) :
    num_rows(rows), num_cols(cols)
{
    data = new int[rows*cols];
}

IndexMatrix::~IndexMatrix()
{
    delete[] data;
}

void IndexMatrix::set(unsigned row, unsigned col, int value)
{
    if(row >= num_rows || col >= num_cols)
        throw std::runtime_error("IndexMatrix.set(): matrix subscript out of bounds");
    data[num_cols*row + col] = value;
}

int IndexMatrix::get(unsigned row, unsigned col) const
{
    if(row >= num_rows || col >= num_cols)
        throw std::runtime_error("IndexMatrix.get(): matrix subscript out of bounds");
    return data[num_cols*row + col];
}

int IndexMatrix::last() const
{
    return data[num_cols*num_rows - 1];
}

unsigned IndexMatrix::width()
{
    return num_cols;
}

unsigned IndexMatrix::height()
{
    return num_rows;
}

//function to print the matrix to standard output, mainly for testing purposes
void IndexMatrix::print()
{
//    std::cout << "    multi-grade data stored in end_cols:\n";
    for(int i=num_rows-1; i>=0; i--)
    {
        std::cout << "        |";
        for(int j=0; j<num_cols; j++)
        {
            int n = data[num_cols*i + j];
            if(n>=0 && n<10)
                std::cout << " ";
            std::cout << " " << n;
        }
        std::cout << " |\n";
    }
}
