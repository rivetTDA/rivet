/**
 * \class	Presentation
 * \brief	Stores a presentation of a 2-D persistence module. 
 * \author	Michael Lesnick
 * \date	9/28/17
 */

#ifndef __Bigraded_Matrix_H__
#define __Bigraded_Matrix_H__


#include "index_matrix.h"
#include "PHAT/vector_heap.h"
#include <string>

//forward declarations
template< class Representation>
class SparseMatrix<Representation>;
class vector_heap;
class IndexMatrix;

class Presentation {
public:
    
    //Note: It is up to the user to make sure that the data indeed defines a valid presentation.
    SparseMatrix<vector_heap> mat;
    IndexMatrix col_ind;
    IndexMatrix row_ind;
    
    ///member functions
    
    //A valid presentation matrix corresponds to a morphism of free 2-D persistence modules.  This tells us whether the kernel is minimal.  Our minimization procedure depends on this assumption.  In the RIVET code as we envision it, we will only need to consider such presentations, but in the future, we may look at others as well, so this is for safety, since minimization
    
    std::string minimality();
    
    void set_minimality(string s);
    
    //Throws an exception if !is_kernel_minimal
    void minimize();
    
    //Constructor
    Presentation(unsigned rows,unsigned columns, unsigned num_xgr, unsigned num_ygr) : mat(SparseMatrix(rows,cols)), col_ind(IndexMatrix(num_ygr,num_xgr)), row_ind(IndexMatrix(num_ygr,num_xgr)), minimality("undefined")
    
private:
    //valid values= "undefined", "kernel_minimal", "fully_minimal", "nonminimal"
    std::string minimality;
};

#endif // __Bigraded_Matrix_H__
