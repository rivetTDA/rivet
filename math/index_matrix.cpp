#include "index_matrix.h"

#include <stdexcept> //for error-checking and debugging

IndexMatrix::IndexMatrix(unsigned rows, unsigned cols)
    : num_rows(rows)
    , num_cols(cols)
{
    data = new int[rows * cols];
}

IndexMatrix::~IndexMatrix()
{
    delete[] data;
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
