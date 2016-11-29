/**
 * \class	IndexMatrix
 * \brief	Stores a matrix of column indexes, one for each multi-grade (to accompany a MapMatrix).
 * \author	Matthew L. Wright
 * \date	July 2014
 */

#ifndef __IndexMatrix_H__
#define __IndexMatrix_H__

// Holds information about the simplices of one particular dimension that exist at each
// multigrade. Note that an IndexMatrix instance is specific to one simplex dimension,
// e.g. it might track 0-simplices or 3-simplices. The indices themselves
// are meaningful only in relation to a particular SimplexTree instance.
//
// The entry at (row, col) is to be the greatest dim_index of all simplices that
// appear at or before this multigrade in reverse lexicographical order, or -1
// if there are no such simplices
class IndexMatrix {
public:
    IndexMatrix(unsigned rows, unsigned cols);
    ~IndexMatrix();

    void set(unsigned row, unsigned col, int value);
    int get(unsigned row, unsigned col) const;

    int last() const;

    unsigned width() const; //returns number of columns
    unsigned height() const; //returns number of rows

private:
    unsigned num_rows;
    unsigned num_cols;
    int* data;
};

#endif // __IndexMatrix_H__
