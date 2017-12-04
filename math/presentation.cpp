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
    : mat(0,0)
    , col_ind(fir.high_mx.ind.height(),fir.high_mx.ind.width())
    , row_ind(fir.high_mx.ind.height(),fir.high_mx.ind.width())
    , is_minimized(false)
    , is_kernel_minimal(true)
{
    //Compute a minimal set of generators for the image of the high matrix.  This is done by simple version of the standard reduction.
    //TODO: later, this will be modified to also yield clearing data.
    BigradedMatrixLex high_min_gens = high_mat.min_gens_and_clearing_data(fir);
    
    //Computes the kernel of the low matrix.
    //TODO: later, this will be modified to take advantage of clearing data.
    //TODO: I guess then that the kernel method might belong in the presentation class.
    BigradedMatrix low_kernel = low_mx.kernel();
    
    //TODO: This step requires to copy the IndexMatrix.  Restructure to avoid this unnecessary copying?
    //One solution would be to move kernel() from BigradedMatrix to presentation.
    row_ind = low_kernel.ind;
    
    //set the matrix to be the right size.
    mat = MapMatrix(low_kernel.mat.size(),high_min_gens.mat.size());
    
    //Complete computation of the presentation by re-expressing each column of high_min_gens in the low_kernel coordinates.
    //Result is stored in mat and col_ind.
    kernel_coordinates(high_min_gens, low_kernel);
}

//Given the high matrix in the FIRep, gives a new BigradedMatrixLex with the same image.
//TODO: Add clearing functionality
BigradedMatrixLex Presentation::min_gens_and_clearing_data(FIRep& fir)
{
    BigradedMatrix& mx = fir.mx_high;
    BigradedMatrixLex new_high_mx(mx.height(),0,mx.ind.height(),mx.ind.width());
    
    //initialize low array for the standard reduction
    std::vector<int> lows(mx.height(), -1);
    
    //Compute the kernel in lex order via the standard bigraded reduction
    for (unsigned x = 0; x < mx.ind.width(); x++) {
        for (unsigned y = 0; y < mx.ind.height(); y++)
        {
            min_gens_and_clearing_data_one_bigrade(mx, new_high_mx, x, y, lows)
        }
    }
    return new_high_mx;
}

//Variant of the standard bigraded reduction which copies which are not zeroed out into a new matrix.
//TODO: Add clearing functionality.
void min_gens_and_clearing_data_one_bigrade(BigradedMatrix& mat, BigradedMatrixLex& new_high, unsigned curr_x, unsigned curr_y, std::vector<int>& lows)
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
            l = mat.remove_max(j);
        }
        
        if (l != -1) //column is still nonempty
        {
            //if we changed the column, we have to put back the last entry we popped.
            if (changing_column) {
                mat.push_index(j,l);
                mat.finalize(j);
            }
            
            lows[l] = j;
            
            //copy this column into the new matrix.
            new_high.mat.append_column(mat,j);
        }
    } // reductions complete
    
    // to record the bigrades of the columns added to new_high, we update the IndexMatrix.
    new_high.ind.set(curr_y, curr_x) = new_high.mat.width()-1;
    
} //min_gens_and_clearing_data_one_bigrade()



void Presentation::kernel_coordinates(BigradedMatrixLex& high_mat, const BigradedMatrix& kernel)
{
    //create and initialize the low array
    std::vector<int> ker_lows(kernel.mat.height(),-1);
    for (unsigned i = 0; i < kernel.mat.height(); i++)
    {
        ker_lows[kernel.mat.low_finalized(i)] = i;
    }
    
    unsigned num_cols_added = 0;
    //proceeding in colex order on bigrades of columns of high_mat, rexpress each column of high_mat with respect to the columns of the kernel,
    //and place the resulting values into mat.
    //note that columns of high_mat is initially in lex order.
    for (unsigned y = 0; y < mat.ind.height(); y++)
    {
        for (unsigned x = 0; x < mat.ind.width(); x++)
        {
            kernel_coordinates_one_bigrade(high_mat,kernel, x, y,ker_lows, num_cols_added)
        }
    }
}

void Presentation::kernel_coordinates_one_bigrade(BigradedMatrixLex& high_mat, const BigradedMatrix& kernel, const unsigned& curr_x, const unsigned& curry_y,const std::vector<unsigned>& ker_lows,  unsigned num_cols_added&)
{
    int c;
    int l;
    
    int first_col = high_mat.ind.start_index(curr_y,0);
    int last_col = high_mat.ind.get(curr_y,curr_x);
    
    //take each column with index in [first_col,last_col] to be a pivot column.
    for (int j = first_col; j <= last_col; j++) {

        l=high_mat._remove_max(j);
        
        //reduce the jth column to zero, storing the reduction indices in mat.
        while (l != -1 ) {
            c = ker_lows[l];
            
            //add column c of kernel matrix to column j of the high matrix.
            high_mat.add_column_popped(c, j);
            
            //add c to num_cols_added^{th} column of mat.
            mat.set(c,num_cols_added);
            
            //remove next element of jth column of high_mat
            l = mat.remove_max(j);
        }
        
        //column of high_mat is now empty and the corresponding column has been added to mat.
        //now heapify that column.
        mat.prepare_col(num_cols_added);
        
        num_cols_added++;
    }
    
    // to record the bigrades of the columns added to new_high, we update the IndexMatrix.
    mat.ind.set(curr_y, curr_x) = num_cols_added-1;
} //kernel_coordinates_one_bigrade()


/*
 Throws an exception if !is_kernel_minimal.
 Minimizes presentation using only column operations.
 This requires finding column entries which are not necessarily pivots, so this code sorts each column first, and then finds the entries using binary search.
 Columns are added in a way that maintains the order.
 */
void Presentation::minimize()
{
    if (! is_kernel_minimal)
        throw std::runtime_error("Presentation::minimize() : Presentation is not kernel minimal.\n");
    
    //new_row_indices[] stores the new index corresponding to
    std::vector<int> new_row_indices(mat.height(),0);
    
    //sort each column of presentation
    for (unsigned i = 0; i < mat.width(); i++)
        mat[i].sort();
    
    //stores the next place we should move a column that we are not minimizing.
    unsigned num_cols_kept = 0;
    
    //variables to work with index matrices
    Grade curr_grade(0,0);
    Grade prev_grade(0,0);
    unsigned last_index = -1;
    
    unsigned pivot_i;
    
    //for each column i with bigrade equal to that of pivot_i,
    //add column i to columns to its right, ensure that pivot_i does not appear as a nonzero entry to the right of column i.
    
    for (unsigned i = 0; i < mat.width(); i++)
    {
        //get the grade of the ith column
        while (col_ind.get(curr_grade.y,curr_grade.x) < i)
            col_ind.next_colex(curr_grade.y,curr_grade.x);
        //after exiting the while loop, curr_grade is equal to the bigrade of column i.
        
        //decide whether the grade of ith column and that of pivot_i are equal
        pivot_i = mat[i].back();
        
        if ( pivot_i >= row_ind.start_index(curr_grade.y,curr_grade.x)) {
            //if we're here then the grades of the column and pivot match
            
            //mark pivot_i for removal from row indices
            new_row_indices[pivot_i] = -1;
            
            //zero out the part of the row pivot_i to the right of i
            for(unsigned j = i+1; j < mat.width(); j++ ) {
                
                if entry_sorted(pivot_i,j) {
                    //if we're here then pivot_i is contained in column j
                    //add column i to column j to clear pivot_i.
                    add_column_sorted(i,j);
                }
            }
        }
        else
        {
            
            //if we get here then the grades of the column and pivot don't match
            //We move this column to the appropriate place and update in IndexMatrix accordingly
            
            //move column i to indenx num_cols_kept
            move_column(i,num_cols_kept);
            
            //Now update the row_index matrix.
            update_col_and_row_inds(prev_grade, curr_Grade, num_cols_kept)
            
            num_cols_kept++;
            last_index = i;
        }
    }
    
    unsigned new_height=mat.height()-(mat.width()-num_cols_kept);
    
    unsigned next_index=0;
    for (unsigned i=0; row_indices_to_remove.size(); i++) {
        if (new_row_indices[i] != -1)
        {
            new_row_indices[i]=next_index;
            next_index++;
        }
    }
    
    //resize the matrix
    mat.resize(new_height,num_cols_kept);
    
    //we've finished minimizing the presentation
    is_minimized = true;
    
}

void Presentation::update_col_and_row_inds(Grade& start_grade, const Grade& end_grade, const unsigned& value)
{
    Grade& current_grade = start_grade;
    while (! (current_grade == end_grade) )
    {
        //decrement value of row_ind at (x,y) by the appropriate amount
        row_ind.set(current_grade.y, current_grade.x, row_ind.get(current_grade.y,current_grade.x)-((col_ind.get(current_grade.y,current_grade.x)-value)));
        
        //set value of col_ind at (x,y)
        col_ind.set(current_grade.y, current_grade.x,value);
        
        //increment w.r.t. colex order on the grid of bigrades.
        col_ind.next_colex(current_grade.y, current_grade.x);
    }
}





