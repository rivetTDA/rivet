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

BigradedMatrix::kernel()
{
    if (num_x_grades <= 0 || num_y_grades <= 0) {
        //TODO: Handle this corner case by returning a 0x0 matrix.
    }
    
    //The following two objects will store a basis for the kernel in lex order.
    MapMatrix ker_lex_mat = MapMatrix(mat.height(),0)
    
    IndexMatrixLex ker_lex_ind  =  IndexMatrixLex(ind.height(),ind.width());
    
    //A column-sparse identity matrix.  Will serve as slave in the reduction.
    reduction_matrix = MapMatrix(mat.width());
    
    //initialize low array for the standard reduction
    std::vector<int> lows(mat.height(), -1);

    //Compute the kernel in lex order via the standard bigraded reduction
    for (unsigned x = 0; x < ind.width(); x++) {
        for (unsigned y = 0; y < ind.height(); y++) { //reduce bdry2 at (x,y) and record rank
            compute_kernel_one_bigrade(reduction_matrix,ker_lex_mat,ker_lex_ind,Grade(x,y),lows);
        }
    }
    
    BigradedMatrix colex_ker = BigradedMatrix(mat.height(),ker_lex_mat.width(),ind.height(),ind.width());
    
    int first_col;
    int last_col;
    
    //Move lex_ker.mat into ker.mat, sorting the columns in colex order, and construct the corresponding IndexMatrix
    for (unsigned y = 0; y < ind.height(); y++) {
        for (unsigned x = 0; x < ind.width(); x++) { //reduce bdry2 at (x,y) and record rank
            
            //find the indices of the first and last columns of lex_ker_mat at bigrade (x,y).
            first_col= ker_lex_ind.start_index(y,x);
            last_col = ker_lex_ind.get(y,x);
            
            //move all the columns of bigrade (x,y) from lex_ker_mat to colex_ker.mat
            for (unsigned j = first_col; j<= last_col ; j++)
                colex_ker.mat.move_col(lex_ker_mat,j);
            
            //Set the value of colex_ker.ind at x,y
            colex_ker.ind.set(y,x,colex_ker.mat.width()-1);
        }
    }
    return colex_ker;
}

void BigradedMatrix::compute_kernel_one_bigrade(MapMatrix& slave, MapMatrix ker_mat, IndexMatrixLex ker_ind, const Grade current_grade, Vector& lows)
{
    int c;
    int l;
    bool changing_column = false;
    
    //TODO: Since this start_index(row,col) is only called when cols=0, that function could be simpified further.
    int first_col=ind.start_index(current_grade.y,0);
    int last_col=ind.get(curr_grade.y,current_grade.x);
        
    //take each column with index in [first_col,last_col] to be a pivot column.
    for (int j = first_col; j <= last_col; j++) {
        
        l = mat->remove_low(j);
        
        if (l != -1 && lows[l] != -1 && lows[l] < j)
        {
            //if we get here then we are going to change the j^{th} column.
            changing_column = true;
        }
    
        //while column j is nonempty and its low number is found in the low array, do column operations
        while (l != -1 && lows[l] != -1 && lows[l] < j) {
            c = lows[l];
            mat.add_column_popped(c, j);
            slave.add_column(c, j);
            l = mat.remove_low(j);
        }
    
        if (l != -1) //column is still nonempty, so push the low entry of column j back into the column and update lows
        {
            mat.push_index(j,l);
            lows[l] = j;
        }
        else
        {
            if (changing_column)
            {
                //if we are here, then column j was just zeroed out now.
                slave->finalize(j);
                
                //Add the j^{th} column of slave to the back of working_ker.mat and zero out the corresponding column in slave
                ker_mat.move_col(slave,j);
            }
        }
    
        if (changing_column)
        {
            mm->finalize(j);
            //TODO: Finalize slave here?  Doesn't seem worth the trouble; better to finalize later.
            //slave->finalize(j);
            changing_column = false;
        }
    
    } // reductions complete
    
    // to record the bigrades of the newly added generators of the kernel, we update the IndexMatrix.
    ker_ind.set(current_grade.y, current_grade.x) = ker_mat.width()-1;
    
    
    
    
    
} //end reduce_slave()
    
    
    
    
