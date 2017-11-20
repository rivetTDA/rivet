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
    BigradedMatrix(MapMatrix map_mat, IndexMatrix ind_mat) : mat(map_mat), ind(ind_mat)
    {}
    
    //Compute the kernel of this bigraded matrix via a standard reduction
    BigradedMatrix kernel();
};


private:

    //adapted from MultiBetti::reduce_slave()
    void reduce_slave(MapMatrix& slave, const int& first_col, const int& last_col, Vector& lows,
                                  unsigned y_grade, ColumnList& zero_list, long& zero_cols)


#endif // __Bigraded_Matrix_H__
