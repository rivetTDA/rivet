/**
 * \class	BigradedMatrix
 * \brief	Stores a SparseMatrix object (i.e., PHAT-type column-sparse matrix) and an IndexMatrix specifying the grade of appearance of each column.
 * \author	Michael Lesnick
 * \date	9/28/17
 */

#ifndef __Bigraded_Matrix_H__
#define __Bigraded_Matrix_H__

//forward declarations

template< class Representation>
class SparseMatrix<Representation>;
class vector_heap;
class IndexMatrix;

struct BigradedMatrix {
public:
    //column-sparse matrix
    SparseMatrix<vector_heap> mat;
    //bigrade info for each column of mat
    IndexMatrix ind;
    
    BigradedMatrix(unsigned rows, unsigned cols, unsigned num_xgr, unsigned num_ygr) : mat(SparseMatrix(rows,cols)), ind(IndexMatrix(num_ygr,num_xgr))
    {}
};

#endif // __Bigraded_Matrix_H__
