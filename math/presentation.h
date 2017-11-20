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

class Presentation {
public:
    
    MapMatrix mat;
    IndexMatrix col_ind;
    IndexMatrix row_ind;
    
    ///member functions
    
    //A valid presentation matrix corresponds to a morphism of free 2-D persistence modules.  This tells us whether the kernel is minimal.  Our minimization procedure depends on this assumption.  In the RIVET code as we envision it, we will only need to consider such presentations, but in the future, we may look at others as well, so this is for safety, since minimization
    
    std::string minimality();
    
    //Throws an exception if !is_kernel_minimal
    void minimize();
    
    //Constructor
    //Builds a presentation from an FI-Rep.
    Presentation(FIRep fir);
    
    //Gets all bigraded Betti numbers.  Returns this data as a 3-dimensional array.
    Betti_Array get_betti();
    
    
private:
  
    bool is_minimal;
    bool is_kernel_minimal; //If is_minimal then kernel_minimal should always hold.  Note: Our algorithm for constructing a presentation from an FIRep yields a kernel minimal presentation.
    
    //description here.
    BigradedMatrix kernel_coordinates(BigradedMatrix fir_high, BigradedMatrix kernel)
    
};

#endif // __Presentation_H__
