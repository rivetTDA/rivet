//
//  bigraded_matrix.cpp
//  
//
//  Created by mlesnick on 11/18/17.
//
//

#include "bigraded_matrix.h"
#include "grade.h"
#include "index_matrix.h"
#include "map_matrix.h"
#include <vector>
#include <set>

//constructor
BigradedMatrix(unsigned rows, unsigned cols, unsigned ind_rows, unsigned ind_cols)
    : mat(rows,cols), ind(ind_rows,ind_cols)
{}

//Constructor taking a BigradedMatrixLex object.  Moves the cols of lex_mat into this matrix, and builds the corresponding index matrix.
//Also trivializes lex_mat in the process
BigradedMatrix::BigradedMatrix(BigradedMatrixLex lex_mat)
    : BigradedMatrix(lex_mat.mat.height(), lex_mat.mat.width(), lex_mat.ind.height(), lex_mat.ind.width())
{
    int first_col;
    int last_col;
    
    for (unsigned y = 0; y < ind.height(); y++) {
        for (unsigned x = 0; x < ind.width(); x++) {
            
            //find the indices of the first and last columns of lex_mat at bigrade (x,y).
            first_col= lex_mat.ind.start_index(y,x);
            last_col = lex_mat.ind.get(y,x);
            
            //move all the columns of bigrade (x,y) from lex_mat to mat
            for (unsigned j = first_col; j<= last_col ; j++)
                mat.move_col(lex_mat,j);
            
            //Set the value of colex_ker.ind at x,y
            ind.set(y,x,mat.width()-1);
        }
    }
    
    //replace lex_mat with something trivial
    lex_mat=MapMatrix(0,0);
}


BigradedMatrix BigradedMatrix::kernel()
{
    //TODO: Do I need this corner case?  I will assume not.
    /*
    if (num_x_grades <= 0 || num_y_grades <= 0) {
        //TODO: Handle this corner case by returning a 0x0 matrix.
    }
    */
     
    //The following object will store a basis for the kernel in lex order.
    
    BigradedMatrixLex ker_lex = BigradedMatrixLex(mat.height(),0,ind.height(),ind.width());
    
    //A column-sparse identity matrix.  Will serve as slave in the reduction.
    reduction_matrix = MapMatrix(mat.width());
    
    //initialize low array for the standard reduction
    std::vector<int> lows(mat.height(), -1);
    
    //Compute the kernel in lex order via the standard bigraded reduction
    for (unsigned x = 0; x < ind.width(); x++) {
        for (unsigned y = 0; y < ind.height(); y++)
        {
            kernel_one_bigrade(reduction_matrix,ker_lex,curr_x,curr_y,lows);
        }
    }
    
    return BigradedMatrix(ker_lex_mat);
}

void BigradedMatrix::kernel_one_bigrade(MapMatrix& slave, BigradedMatrixLex& ker_lex, unsigned curr_x, unsigned curr_y, Vector& lows)
{
    int c;
    int l;
    bool changing_column;
    
    int first_col=ind.start_index(curr_y,0);
    int last_col=ind.get(curr_y,curr_x);
        
    //take each column with index in [first_col,last_col] to be a pivot column.
    for (int j = first_col; j <= last_col; j++) {
        
        changing_column = false;
        l=mat._get_max_index_finalized(j);
        
        if (l != -1 && lows[l] != -1 && lows[l] < j)
        {
            //if we get here then we are going to change the j^{th} column.
            changing_column = true;
            matrix._remove_max(j);
        }
    
        //while column j is nonempty and its low number is found in the low array, do column operations
        while (l != -1 && lows[l] != -1 && lows[l] < j) {
            c = lows[l];
            mat.add_column_popped(c, j);
            slave.add_column(c, j);
            l = mat.remove_max(j);
        }
    
        if (l != -1) //column is still nonempty
        {
            lows[l] = j;
            
            //if we changed the column, we have to put back the last entry we popped, and we should also finalize
            if (changing_column)
            {
                mat.push_index(j,l);
                
                //TODO: Do we really want to bother with a linear-time finalization?  If not, change the definition of add_column_popped.
                mat.finalize(j);
            }
        }
        else // column is empty
        {
            if (changing_column)
            {
                //if we are here, then column j was just zeroed out now.
                slave->finalize(j);
                
                //Add the j^{th} column of slave to the back of working_ker.mat and zero out the corresponding column in slave
                ker_lex.mat.move_col(slave,j);
            }
        }
    } // reductions complete
    
    // to record the bigrades of the newly added generators of the kernel, we update the IndexMatrix.
    ker_lex.ind.set(curr_y, curr_x) = ker_lex.mat.width()-1;

} //end kernel_one_bigrade()
    

    
    
