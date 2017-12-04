/**
 * \class	Presentation
 * \brief	Stores a presentation of a 2-D persistence module.  Constructor takes an FIRep as input.
 * \author	Michael Lesnick
 * \date	9/28/17
 */

#ifndef __Presentation_H__
#define __Presentation_H__


#include "index_matrix.h"
#include "map_matrix.h"
#include <string>

//forward declarations
template< class Representation>
class SparseMatrix<Representation>;
class vector_heap;
class IndexMatrix;
class FIRep;
class BigradedMatrix


//Class stores 2-parameter peristence module.  Columns are assumed to be in colex order.
//Constructor builds a presentation from an FIRep, which is one of the main steps in the new implementation of RIVET.
class Presentation {
public:
    
    MapMatrix mat;
    IndexMatrix col_ind;
    IndexMatrix row_ind;
    
    //Constructor
    //Builds a presentation from an FI-Rep.
    Presentation(FIRep fir);
    
    /* 
    Throws an exception if !is_kernel_minimal.  Minimizes presentation using only column operations.
    This requires looking at column entries which are not necessarily pivots, so this code sorts each column first, and then finds the entries using binary search.
    Columns are added in a way that maintains the order.
     
    When we call minimize(), we implcitly are removing rows from the matrix.
    Thus, in our column sparse setting, after minimization the entries of the matrices will only only be correct up to an order-preserving reindexing.
    Of course, we could perform that reindexing explicitly, and if all we want a minimal presentation, that is the right thing to do.
    But given the way the rest of RIVET is structured, to compute barcode templates, it is not efficient to explictly perform the reindexing;
    instead we reindex implictly with a vector new_row_indices, of size the height of the unminimized matrix.
    If row i in the minimized presentation is removed, new_row_indices[i]=-1.
    Otherwise, new_row_indices[i] is the index of row i in the minimized presentation.
    */
    
    void minimize();
    std::vector<int> new_row_indices;
    
private:
  
    bool is_minimized; //Has this presentation been minimized?  Note that it might be minimal even if it hasn't been explicitly minimized
    
    //If is_minimized then kernel_minimal should always hold.
    //Note: Our algorithm for constructing a presentation from an FIRep yields a kernel minimal presentation.
    bool is_kernel_minimal;
    
    /*
    min_gens_and_clearing_data(fir) does two things.  
    First, it replaces fir.high with a bigraded matrix whose set of columns is a minimal set of generators for the image of the map fir.low (the input matrix is in colex order; output is in lex order.)  
     
    TODO: First implementation does not involve any clearing.  Implement clearing later.
    TODO: Finish/clean up this comment!
    Second, it outputs a matrix with the same number of columns as fir.low, each corresponding to an element of the kernel.  A map from bigades to lists of indices is also output, specifying when each
     another lex-ordered bigraded matrix VKer whose column at grade i are the reduction matrix columns which enter at index i.  Each column of VKer j is also given an index, which specifies the column of V to which VKer corresponds.
    */
    
    BigradedMatrixLex min_gens_and_clearing_data(FIRep& fir);

    //Re-express each column of high_mat in kernel coordinates.
    //Result is stored in mat and col_ind.
    void kernel_coordinates(BigradedMatrixLex& high_mat, const BigradedMatrix& kernel)
    
    
    /*---------------- Technical Functions ----------------*/
    
    //Used for min_gens_and_clear.
    void min_gens_and_clearing_data_one_bigrade(MapMatrix& red_mat, IndexMatrixLex& red_ind, unsigned curr_x, unsigned curr_y, std::vector<int>& lows)
    
    //Technical function for kernel_coordinates.  Simplest form of bigraded reduction with slave at one bigrade.
    //Reduce_matrix_one_bigrade(SOMETHING HERE);
    
    //Reduce this column, putting the corresponding reduction coordinates
    void kernel_coordinates_one_bigrade(BigradedMatrixLex& high_mat, const BigradedMatrix& kernel, const unsigned& curr_x, const unsigned& curr_y, const std::vector<unsigned>& ker_lows, unsigned& num_cols_added);
    
    //technical utility function used by Presentation.minimize()
    //Same action as IndexMatrix::fill_index_mx() on col_ind, but also decrements each entry of a second IndexMatrix
    //in the colex interval [start_grade,end_grade) by the difference between value and the index of this index matrix.
    //To be called on the member col_ind, with row_ind_mx as the argument.
    void IndexMatrix::update_col_and_row_inds(Grade& start_grade, const Grade& end_grade, const unsigned& value)
    
    
    
};

#endif // __Presentation_H__
