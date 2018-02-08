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
#include <interface/progress.h>

#include <boost/multi_array.hpp>
typedef boost::multi_array<unsigned, 2> unsigned_matrix;

//forward declarations
class vector_heap_mod;
class IndexMatrix;
class FIRep;
class BigradedMatrix;
class BigradedMatrixLex;

//Class stores a presentation of 2-parameter peristence module, as well as its Hilbert function.  Columns are assumed to be in colex order.
//Constructor builds a presentation from an FIRep, which is one of the main steps in the new implementation of RIVET.
class Presentation {

public:
    
    MapMatrix mat;
    IndexMatrix col_ind;
    IndexMatrix row_ind;
    
    //Stores the hilbert function.
    //It is admittedly an idiosyncratic design to have a presentation object store the Hilbert function by default, but for now, this is expedient.
    
    //Note: To compute hom_dims (i.e., Hibert function), we will first store the pointwise ranks of the high map in this matrix, and then subtract this off of the pointwise nullities.
    unsigned_matrix hom_dims;

    
    //Constructor
    //Builds a presentation from an FI-Rep.  Also computes Hilbert function along the way.
    Presentation(FIRep fir, Progress& progress);
    
    /* 
    Throws an exception if !is_kernel_minimal.  Minimizes presentation using only column operations.
    This requires looking at column entries which are not necessarily pivots, so this code sorts each column first, and then finds the entries using binary search.
    Columns are added in a way that maintains the order.
     
   When we call minimize(), we implcitly are removing rows from the matrix.  Thus, in our column sparse setting, after the initial minimization, the entries of the matrices are correct up to an order-preserving reindexing.  To complete the minimization, the code then performs a reindexing of the entire matrix.
    
    TODO: It may be that for computing barcode templates, it is more efficient to avoid redinexing the matrix and
    simply maintain an array which stores the updated incides.  On the other hand, the cost of reindexing is probably negligible relative to eveything else.
    */
    
    void minimize();
    
    void print();
    
private:
  
    bool is_minimized; //True iff this presentation has been minimized.  Note that presentation might be minimal even if it hasn't been explicitly minimized
    
    //If is_minimized then kernel_minimal should always hold.
    //Note: Our algorithm for constructing a presentation from an FIRep yields a kernel minimal presentation.
    bool is_kernel_minimal;
    
    /*
    min_gens_and_clearing_data(fir) replaces fir.high with a bigraded matrix whose set of columns is a minimal set of generators for the image of the map fir.high (the input matrix is in colex order; output is in lex order.)
    Also stores the pointwise ranks in hom_dim (This is an intermediate step in the calculation of the homology_dimensions)
    TODO: Add clearing functionality
    */
    
    BigradedMatrixLex min_gens_and_clearing_data(FIRep & fir);

    //Re-express each column of high_mat in kernel coordinates.
    //Result is stored in mat and col_ind.
    void kernel_coordinates(BigradedMatrixLex& high_mat, const BigradedMatrix& kernel);
    
    
    /*---------------- Technical Functions ----------------*/
    
    //Used for min_gens_and_clear.
    void min_gens_and_clearing_data_one_bigrade(BigradedMatrix& old_high, BigradedMatrixLex& new_high, unsigned curr_x, unsigned curr_y, std::vector<int>& lows);
    
    //TODO: Once min_gens_and_clearing_data_one_bigrade(...) is working properly, delete this line and the next!!!
    //(MapMatrix& red_mat, IndexMatrixLex& red_ind, unsigned curr_x, unsigned curr_y, std::vector<int>& lows);
    
    //Reduce this column, putting the corresponding reduction coordinates
    void kernel_coordinates_one_bigrade(BigradedMatrixLex& high_mat, const BigradedMatrix& kernel, const unsigned& curr_x, const unsigned& curr_y, const std::vector<int>& ker_lows, unsigned& num_cols_added);
    
    //Technical function for constructing hom_dims at all indices from an FIRep.  Used by the Presentation constructor.
    void compute_hom_dims(const IndexMatrix& ind);
    
    //technical utility function used by Presentation.minimize()
    //Same action as IndexMatrix::fill_index_mx() on col_ind, but also decrements each entry of a second IndexMatrix
    //in the colex interval [start_grade,end_grade) by the difference between value and the index of this index matrix.
    //To be called on the member col_ind, with row_ind_mx as the argument.
    void update_col_and_row_inds(IndexMatrix& row_ind_new, Grade& start_grade, const Grade& end_grade, const int& value);

    
    //Technical function used to compute a minimal presetation.
    //Takes as input a minimal presentation with the non-minimal column indices (which skip some entries), and a vector giving the miminal (consecutive) column indices.
    //Replaces all indices in the matrix with the minimal column indices.
    void reindex_min_pres(const std::vector<int>& new_row_indices);
    
    //Technical function by Presentation::minimize()
    bool row_index_has_matching_bigrade(int j, unsigned row, unsigned col);
    
};

#endif // __Presentation_H__
