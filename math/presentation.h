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
    
    //Throws an exception if !is_kernel_minimal.  Minimizes presentation using only column operations.  This requires finding column entries which are not necessarily pivots, so this code sorts each column first, and then finds the entries using binary search.  Columns are added in a way that maintains the order.
    void minimize();
    

    
    //Gets all bigraded Betti numbers.  Returns this data as a 3-dimensional array.
    Betti_Array get_betti();
    
    
private:
  
    bool is_minimal;
    bool is_kernel_minimal; //If is_minimal then kernel_minimal should always hold.  Note: Our algorithm for constructing a presentation from an FIRep yields a kernel minimal presentation.
    
    //TODO: First implementation does not involve any clearing.  Implement clearing later.
    //replace this matrix with one whose set of columns is a minimal set of generators for the bigraded image of the original matrix (input is in colex order; output is in lex order.)
    //computing by doing a standard reduction, copying columns which do not reduce to zero into a new matrix, and then replacing this matrix with the new matrix.
    
    //TODO: First implementation does not involve any clearing.  Implement clearing later.  Clearing will probably also require us to change the signature of this function, in order to record the columns of the kernel obtained from clearing.  
    BigradedMatrixLex min_gens_and_clear(FIRep fir);

    //Third and final function to be called
    BigradedMatrix kernel_coordinates(BigradedMatrixLex high_mat_reduced, BigradedMatrix kernel)
    
    
    /*---------------- Technical Functions ----------------*/
    //Used for min_gens_and_clear
    void min_gens_and_clear_one_bigrade(MapMatrix& red_mat, IndexMatrixLex& red_ind, const Grade& current_grade, std::vector<int>& lows)
    
    //Technical function for kernel_coordinates.  Simplest form of bigraded reduction with slave at one bigrade.
    Reduce_matrix_one_bigrade(SOMETHING HERE);
    
    //Reduce this column, putting the corresponding reduction coordinates
    kernel_coordinates_one_column(SOMETHING HERE);
    
    
    
};

#endif // __Presentation_H__
