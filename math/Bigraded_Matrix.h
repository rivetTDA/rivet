/**
 * \class	BigradedMatrix
 * \brief	Stores a SparseMatrix object (i.e., PHAT-type column-sparse matrix) and an IndexMatrix specifying the grade of appearance of each column.
 * \author	Michael Lesnick and Matthew Wright
 * \date	9/28/17
 */

#ifndef __Bigraded_Matrix_H__
#define __Bigraded_Matrix_H__

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
    //bigrade info for each column of mat.  Columns are assumed to be in colex order.
    IndexMatrix ind;
    
    //constructor
    BigradedMatrix(unsigned rows, unsigned cols, unsigned ind_rows, unsigned ind_cols);
        : mat(rows,cols), ind(ind_rows,ind_cols)
    {}
    
    //Constructor taking a BigradedMatrixLex object.  Moves the cols of lex_mat into this matrix, and builds the corresponding index matrix.
    //Also trivialized lex_mat in the process
    BigradedMatrix::BigradedMatrix(BigradedMatrixLex lex_mat);
    
    //Compute the kernel of this bigraded matrix via a standard reduction:
    //Note: This destroys the matrix.
    BigradedMatrix kernel();
    
private:

     /*
     Performs a step of the kernel computation at a single bigrade.
     This is a variant on the standard bigraded reduction.
     When a column in mat is zeroed out, the corresponding column of slave is appended to the back working_ker.mat, and then zeroed out in the slave.
     The function also records the bigrades of the generators for the kernel by updating working_ker.ind.
     */
    void kernel_one_bigrade(MapMatrix& slave, BigradedMatrixLex& ker_lex, unsigned curr_x, unsigned curr_y, std::vector<int>& lows)

};

//Similar to BigradedMatrix, but columns are assumed to be in lex order.
class BigradedMatrixLex
{
    MapMatrix mat;
    IndexMatrixLex ind;
    
    BigradedMatrixLex(unsigned rows, unsigned cols, unsigned ind_rows, unsigned ind_cols)
        : mat(rows,cols), ind(ind_rows,ind_cols)
    {}
};



#endif // __Bigraded_Matrix_H__

