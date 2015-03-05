/**
 * \class	IndexMatrix
 * \brief	Stores a matrix of column indexes, one for each multi-grade (to acompany a MapMatrix).
 * \author	Matthew L. Wright
 * \date	July 2014
 */

#ifndef __IndexMatrix_H__
#define __IndexMatrix_H__

class IndexMatrix
{
public:
    IndexMatrix(unsigned rows, unsigned cols);
    ~IndexMatrix();

    void set(unsigned row, unsigned col, int value);    //the entry at (row, col) is to be the greatest dim_index of all simplices that appear at or before this multigrade in reverse lexicographical order, or -1 if there are no such simplices
    int get(unsigned row, unsigned col) const;

    int last() const;

    unsigned width();    //returns number of columns
    unsigned height();   //returns number of rows

    void print();   //for testing

private:
    unsigned num_rows;
    unsigned num_cols;
    int* data;
};



#endif // __IndexMatrix_H__
