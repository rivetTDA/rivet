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

    void set(unsigned row, unsigned col, int value);
    int get(unsigned row, unsigned col) const;

    int width();    //returns number of columns
    int height();   //returns number of rows

    void print();   //for testing

private:
    int num_rows;
    int num_cols;
    int* data;
};



#endif // __IndexMatrix_H__
