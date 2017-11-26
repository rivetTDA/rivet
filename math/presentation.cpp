//
//  presentation.cpp
//  
//
//  Created by mlesnick on 11/19/17.
//
//

#include "presentation.h"

//Constructor
//Builds a presentation from an FI-Rep.
Presentation::Presentation(FIRep fir)
    :
{
    BigradedMatrix & low_mat = fir.low_mat;
    BigradedMatrixLex high_mat = high_mat.Min_gens();
    BigradedMatrix low_lernel = low_mat.kernel();

    /*
    Next, find the presentation, i.e., a set of generators for im(high_mat) in the coordinates of the kernel.
    To do this, we run a modfied verison of the standard bigraded reduction w/ slave on the matrix obtained by splicing low_kernel and high_mat by bigrade, with the columns of low_kernel to the left.
    
    This is very similar to the kernel computation we do elsewhere, with one difference:
    1)In the part of the slave for a column of high_mat, we initialize the column to a zero column, not a standard basis vector.  This simplifies reading off the representation of this column in the kernel coordinates.

    Note: I do need to also reduce Low_Kernel; although the columns are all independent to start, they may not give a reduced matrix, and that is necessary.
    
}
