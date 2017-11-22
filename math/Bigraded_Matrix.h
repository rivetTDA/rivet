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

class BigradedMatrix {
public:
    //column-sparse matrix
    MapMatrix mat;
    //bigrade info for each column of mat
    IndexMatrix ind;
    
    //constructor
    BigradedMatrix(unsigned rows, unsigned cols, unsigned ind_rows, unsigned ind_cols) : mat(rows,cols), ind(ind_rows,ind_cols)
    {}
    
    //Compute the kernel of this bigraded matrix via a standard reduction:
    //Note: This destroys the matrix.
    BigradedMatrix kernel();

};

private:

     /*
     Performs a step of the kernel computation at a single bigrade.
     This is a variant on the standard bigraded reduction.
     When a column in mat is zeroed out, the corresponding column of slave is appended to the back working_ker.mat, and then zeroed out in the slave.
     The function also records the bigrades of the generators for the kernel by updating working_ker.ind.
     */
    void compute_kernel_one_bigrade(MapMatrix& slave, MapMatrix ker_mat, IndexMatrixLex ker_ind, const Grade current_grade, Vector& lows)


#endif // __Bigraded_Matrix_H__
