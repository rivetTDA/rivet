//
//  presentation.cpp
//  
//
//  Created by mlesnick on 11/19/17.
//
//

#include "presentation.h"
#include "firep.h"

//Constructor
//Builds a presentation from an FI-Rep.
Presentation::Presentation(FIRep fir, Progress& progress)
    : mat(0,0)
    , col_ind(fir.high_mx.ind.height(),fir.high_mx.ind.width())
    , row_ind(fir.high_mx.ind.height(),fir.high_mx.ind.width())
    , is_minimized(false)
    , is_kernel_minimal(true)
{
    
    //ensure hom_dims is the correct size
    //note that unlike the IndexMatrices, is indexed by x-coordinate first, then y-coordinate.  This discrepancy seems a bit strange, but follows the exisiting convention.
    //TODO: Why does this use a boost multi-array, when the IndexMatrix class uses a custom multi-array?
    hom_dims.resize(boost::extents[fir.high_mx.ind.width()][fir.high_mx.ind.height()]);

    //Compute a minimal set of generators for the image of the high matrix.  Also compute pointwise ranks for the high matrix.
    //This is done by simple version of the standard reduction.
    //TODO: later, this will be modified to also yield clearing data.
    BigradedMatrixLex high_min_gens = min_gens_and_clearing_data(fir);
    
    //emit progress message
    progress.progress(40);
    
    //High matrix in the FIRep is no longer needed.  Replace it with something trivial.
    fir.high_mx.mat=MapMatrix(0,0);
    
    //Compute the kernel of the low matrix.
    //TODO: later, this will be modified to take advantage of clearing data.
    //TODO: I guess then that the kernel method might belong in the presentation class.
    BigradedMatrix low_kernel = fir.low_mx.kernel();
    
    progress.progress(55);
    
    //TODO: This step requires to copy the IndexMatrix.  Restructure to avoid this unnecessary copying?
    //One solution would be to move kernel() from BigradedMatrix to presentation.
    row_ind = low_kernel.ind;
    
    //Low MapMatrix is no longer needed.  Replace it with something trivial.
    fir.low_mx.mat=MapMatrix(0,0);
    
    //set hom_dims to its final value.
    //proceed in lexicographical order
    for (unsigned x = 0; x < col_ind.width(); x++)
    {
        for (unsigned y = 0; y < col_ind.height(); y++)
        {
            //before this line is executed hom_dims[x][y] is equal to the pointwise rank of the high matrix in the firep at (x,y)
            hom_dims[x][y]= low_kernel.ind.num_columns_leq(y,x) - hom_dims[x][y];
        }
    }
    
    //set the matrix to be the right size.
    mat = MapMatrix(low_kernel.mat.width(),high_min_gens.mat.width());
    
    //Complete computation of the presentation by re-expressing each column of high_min_gens in the low_kernel coordinates.
    //Result is stored in mat and col_ind.
    kernel_coordinates(high_min_gens, low_kernel);
    
    //emit progress message
    progress.progress(70);
}

/*
Given the high matrix in the FIRep, gives a new BigradedMatrixLex with the same image.
Also stores the pointwise ranks in hom_dim (This is an intermediate step in the calculation of the homology_dimensions)
TODO: Add clearing functionality
*/
BigradedMatrixLex Presentation::min_gens_and_clearing_data(FIRep& fir)
{
    BigradedMatrix& mx = fir.high_mx;
    BigradedMatrixLex new_high_mx(mx.mat.height(),0,mx.ind.height(),mx.ind.width());
    
    //initialize low array for the standard reduction
    std::vector<int> lows(mx.mat.height(), -1);
    
    //Visit bigrades in lex order.
    for (unsigned x = 0; x < mx.ind.width(); x++) {
        for (unsigned y = 0; y < mx.ind.height(); y++)
        {
            min_gens_and_clearing_data_one_bigrade(mx, new_high_mx, x, y, lows);
        }
    }
    return new_high_mx;
}

//Variant of the standard bigraded reduction which copies columns which are not zeroed out into a new matrix.
//TODO: Add clearing functionality.
void Presentation::min_gens_and_clearing_data_one_bigrade(BigradedMatrix& old_high, BigradedMatrixLex& new_high, unsigned curr_x, unsigned curr_y, std::vector<int>& lows)
{
    int c;
    int l;
    bool changing_column;
    MapMatrix& mx= old_high.mat;
    
    int first_col=old_high.ind.start_index(curr_y,0);
    int last_col=old_high.ind.get(curr_y,curr_x);
    
    //take each column with index in [first_col,last_col] to be a pivot column.
    for (int j = first_col; j <= last_col; j++) {
        
        if (curr_y>0)
            hom_dims[curr_x][curr_y]=hom_dims[curr_x][curr_y-1];
        
        changing_column = false;
        l=mx.low_finalized(j);
        
        if (l != -1 && lows[l] != -1 && lows[l] < j)
        {
            //if we get here then we are going to change the j^{th} column.
            changing_column = true;
            mx.remove_low(j);
        }
        
        //while column j is nonempty and its low number is found in the low array, do column operations
        while (l != -1 && lows[l] != -1 && lows[l] < j) {
            c = lows[l];
            mx.add_column_popped(c, j);
            l = mx.remove_low(j);
        }
        
        if (l != -1) //column is still nonempty
        {
            //if we changed the column, we have to put back the last entry we popped.
            if (changing_column) {
                mx.push_index(j,l);
                mx.finalize(j);
            }
            
            lows[l] = j;
            
            //copy this column into the new matrix.
            new_high.mat.append_col(mx,j);
            
            hom_dims[curr_x][curr_y]++;
            
        }
    } // reductions complete
    
    // to record the bigrades of the columns added to new_high, we update the IndexMatrix.
    new_high.ind.set(curr_y, curr_x,new_high.mat.width()-1);
    
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
    for (unsigned y = 0; y < high_mat.ind.height(); y++)
    {
        for (unsigned x = 0; x < high_mat.ind.width(); x++)
        {
            kernel_coordinates_one_bigrade(high_mat,kernel, x, y,ker_lows, num_cols_added);
        }
    }
}

void Presentation::kernel_coordinates_one_bigrade(BigradedMatrixLex& high_mat, const BigradedMatrix& kernel, const unsigned& curr_x, const unsigned& curr_y,const std::vector<int>& ker_lows,  unsigned& num_cols_added)
{
    int c;
    int l;
    MapMatrix& hi_mx = high_mat.mat;
    
    int first_col = high_mat.ind.start_index(curr_y,0);
    int last_col = high_mat.ind.get(curr_y,curr_x);
    
    //take each column with index in [first_col,last_col] to be a pivot column.
    for (int j = first_col; j <= last_col; j++) {

        l=hi_mx.remove_low(j);
        
        //reduce the jth column to zero, storing the reduction indices in ker_mx.
        while (l != -1 ) {
            c = ker_lows[l];
            
            //add column c of kernel matrix to column j of the high_matrix.
            //TODO: WRONG FUNCTION.
            hi_mx.add_column_popped(kernel.mat,c, j);
            
            //add c to num_cols_added^{th} column of mat.
            mat.set(c,num_cols_added);
            
            //remove next element of jth column of high_mat
            l = hi_mx.remove_low(j);
        }
        
        //column of high_mat is now empty and the corresponding column has been added to mat.
        
        
        //now heapify that column?  In fact if we are just going to minimize presentation, there is no point in heapifying, because the minimization procedure will
        //sort rows.
        //mat.prepare_col(num_cols_added);
        
        num_cols_added++;
    }
    
    // to record the bigrades of the columns added to kernel, we update the IndexMatrix col_ind
    col_ind.set(curr_y, curr_x, num_cols_added-1);
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
    
    //new_row_indices[i] stores the new index in the minimal presentation corresponding to the row index i in the unminimized presentation.
    std::vector<int> new_row_indices(mat.height(),0);
    
    //sort each column of presentation
    for (unsigned i = 0; i < mat.width(); i++)
        mat.sort_col(i);
    
    //stores the next place we should move a column that we are not minimizing.
    unsigned num_cols_kept = 0;
    
    //variables to work with index matrices
    Grade curr_grade(0,0);
    Grade prev_grade(0,0);
    unsigned last_index = -1;
    
    int pivot_i;
    
    //for each column i with bigrade equal to that of pivot_i,
    //add column i to columns to its right, ensure that pivot_i does not appear as a nonzero entry to the right of column i.
    
    for (unsigned i = 0; i < mat.width(); i++)
    {
        //get the grade of the ith column
        while (col_ind.get(curr_grade.y,curr_grade.x) < (int) i)
            col_ind.next_colex(curr_grade.y,curr_grade.x);
        //after exiting the while loop, curr_grade is equal to the bigrade of column i.
        
        //decide whether the grade of ith column and that of pivot_i are equal
        pivot_i = mat.low(i);
        
        if ( pivot_i >= row_ind.start_index(curr_grade.y,curr_grade.x)) {
            //if we're here then the grades of the column and pivot match
            
            //mark pivot_i for removal from row indices
            new_row_indices[pivot_i] = -1;
            
            //zero out the part of the row pivot_i to the right of i
            for(unsigned j = i+1; j < mat.width(); j++ ) {
                
                if (mat.entry_sorted(pivot_i,j)) {
                    //if we're here then pivot_i is contained in column j
                    //add column i to column j to clear pivot_i.
                    mat.add_column_sorted(i,j);
                }
            }
        }
        else
        {
            
            //if we get here then the grades of the column and pivot don't match
            //We move this column to the appropriate place and update in IndexMatrix accordingly
            
            
            //move column i to index num_cols_kept
            mat.move_col(i,num_cols_kept);
            
            //Now update the IndexMatrix row_ind.
            update_col_and_row_inds(prev_grade, curr_grade, num_cols_kept);
            
            num_cols_kept++;
            last_index = i;
        }
    }
    
    unsigned new_height=mat.height()-(mat.width()-num_cols_kept);
    
    unsigned next_index=0;
    for (unsigned i=0; i<new_row_indices.size(); i++) {
        if (new_row_indices[i] != -1)
        {
            new_row_indices[i]=next_index;
            next_index++;
        }
    }
    
    //resize the matrix
    mat.resize(new_height,num_cols_kept);
    
    //replace row indices with new ones
    reindex_min_pres(new_row_indices);
    
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

void Presentation::reindex_min_pres(const std::vector<int>& new_row_indices)
{
    for (unsigned i=0; i < mat.width(); i++)
    {
        mat.reindex_column(i,new_row_indices);
    }
}



