/**
 * \class	IndexMatrix
 * \brief	Stores a matrix of column indexes, one for each multi-grade (to acompany a MapMatrix).
 * \author	Matthew L. Wright
 * \date	July 2014
 */

#ifndef __IndexMatrix_H__
#define __IndexMatrix_H__

#include <stdexcept>

class IndexMatrix
{
public:
    IndexMatrix(unsigned rows, unsigned cols);
    ~IndexMatrix();

    void set(unsigned row, unsigned col, int value);
    int get(unsigned row, unsigned col) const;

    void print();   //for testing

private:
    int num_rows;
    int num_cols;
    int* data;
};

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
        throw std::runtime_error("matrix subscript out of bounds");
    data[num_cols*row + col] = value;
}

int IndexMatrix::get(unsigned row, unsigned col) const
{
    if(row >= num_rows || col >= num_cols)
        throw std::runtime_error("matrix subscript out of bounds");
    return data[num_cols*row + col];
}

//function to print the matrix to standard output, mainly for testing purposes
void IndexMatrix::print()
{
    std::cout << "    multi-grade data stored in end_cols:\n";
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

#endif // __IndexMatrix_H__
