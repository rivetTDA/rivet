/**********************************************************************
 Copyright 2014-2018 The RIVET Developers. See the COPYRIGHT file at
 the top-level directory of this distribution.
 
 This file is part of RIVET.
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

// Author: Michael Lesnick (2017-2018)

#include "presentation.h"
#include "firep.h"
#include "timer.h"

//Constructor for empty presentation
Presentation::Presentation()
    : mat(0, 0)
    , col_ind(0, 0)
    , row_ind(0, 0)
    , is_minimized(false)
    , is_kernel_minimal(false)
{
}

//Constructor
//Builds a presentation from an FI-Rep.
Presentation::Presentation(FIRep& fir, Progress& progress, int verbosity)
    : mat(0, 0)
    , col_ind(fir.high_mx.ind.height(), fir.high_mx.ind.width())
    , row_ind(fir.high_mx.ind.height(), fir.high_mx.ind.width())
    , is_minimized(false)
    , is_kernel_minimal(true)
{

    //ensure hom_dims is the correct size.
    //NOTE: unlike the IndexMatrices, is indexed by x-coordinate first,
    //then y-coordinate.  This discrepancy is a bit strange,
    //but follows the exisiting convention in the code.
    hom_dims.resize(boost::extents[fir.high_mx.ind.width()][fir.high_mx.ind.height()]);

    if (verbosity > 8) {
        std::cout << "HIGH MATRIX:" << std::endl;
        fir.high_mx.print();
    }

    /* Compute a minimal set of generators for the image of the high matrix.
    Also compute pointwise ranks for the high matrix.
    This is done by simple version of the standard reduction.
     */
    //TODO: later, this will be modified to also yield clearing data.

    Timer timer;
    timer.restart();

    BigradedMatrixLex high_min_gens = min_gens_and_clearing_data(fir);

    if (verbosity >= 4) {
        std::cout << "  --> finding a minimal set of generators for the image of the high map took "
                  << timer.elapsed() << " milliseconds." << std::endl;
    }

    //emit progress message
    progress.progress(40);

    //High matrix in the FIRep is no longer needed.
    //Replace it with something trivial.
    fir.high_mx.mat = MapMatrix(0, 0);

    if (verbosity > 8) {
        std::cout << "REDUCED HIGH MATRIX:" << std::endl;
        high_min_gens.print();
    }

    //Compute the kernel of the low matrix.
    //TODO: later, this will be modified to take advantage of clearing data.
    //TODO: Maybe kernel method belongs in the Presentation class?

    if (verbosity > 8) {
        std::cout << "LOW MATRIX:" << std::endl;
        fir.low_mx.print();
    }

    timer.restart();

    BigradedMatrix low_kernel = fir.low_mx.kernel();

    if (verbosity >= 4) {
        std::cout << "  --> computing a basis for the kernel of the low matrix map took "
                  << timer.elapsed() << " milliseconds." << std::endl;
    }

    if (verbosity > 8) {
        std::cout << "KERNEL OF LOW MATRIX:" << std::endl;
        low_kernel.print();
    }

    progress.progress(55);

    //Low MapMatrix is no longer needed.  Replace it with something trivial.
    fir.low_mx.mat = MapMatrix(0, 0);

    //Given that hom_dims contains the pointwise ranks of the high map,
    //set each entry to its final value.
    compute_hom_dims(low_kernel.ind);

    //set the matrix to be the right size.
    mat = MapMatrix(low_kernel.mat.width(), high_min_gens.mat.width());

    timer.restart();

    //Complete computation of the presentation by re-expressing each column of
    //high_min_gens in the low_kernel coordinates. Result is stored in
    //mat and col_ind.
    kernel_coordinates(high_min_gens, low_kernel);

    if (verbosity >= 4) {
        std::cout << "  --> re-expressing the generators of the image of the high map in the basis for the kernel of the low map took "
                  << timer.elapsed() << " milliseconds." << std::endl;
    }

    /* TODO: This step requires to copy the IndexMatrix. Restructure to avoid 
       this unnecessary copying? One solution would be to move kernel() from
       BigradedMatrix to Presentation.  Perhaps a simpler solution would be
       to pass row_ind as a reference to BigradedMatrix::kernel().
    */
    row_ind = low_kernel.ind;

    //can get rid of low_kernel.ind now;
    low_kernel.ind = IndexMatrix(0, 0);

    //emit progress message
    progress.progress(70);
}

/*
Given the high matrix in the FIRep, gives a new BigradedMatrixLex with the same 
image.
Also stores the pointwise ranks in hom_dim.  (This is an intermediate
step in the calculation of the homology_dimensions.)
TODO: Add clearing functionality
*/
BigradedMatrixLex Presentation::min_gens_and_clearing_data(FIRep& fir)
{
    BigradedMatrix& mx = fir.high_mx;
    BigradedMatrixLex new_high_mx(mx.mat.height(), 0, mx.ind.height(), mx.ind.width());

    //initialize low array for the standard reduction
    std::vector<int> lows(mx.mat.height(), -1);

    //Visit bigrades in lex order.
    for (unsigned x = 0; x < mx.ind.width(); x++) {
        for (unsigned y = 0; y < mx.ind.height(); y++) {
            min_gens_and_clearing_data_one_bigrade(mx, new_high_mx, x, y, lows);
        }
    }
    return new_high_mx;
}

//Variant of the standard bigraded reduction which copies columns which are not
//zeroed out into a new matrix.
//TODO: Add clearing functionality.
void Presentation::min_gens_and_clearing_data_one_bigrade(BigradedMatrix& old_high,
    BigradedMatrixLex& new_high,
    unsigned curr_x,
    unsigned curr_y,
    std::vector<int>& lows)
{
    int c;
    int l;
    bool changing_column;
    MapMatrix& mx = old_high.mat;

    int first_col = old_high.ind.start_index(curr_y, 0);
    int first_col_curr_bigrade = old_high.ind.start_index(curr_y, curr_x);
    int last_col = old_high.ind.get(curr_y, curr_x);

    if (curr_y > 0)
        hom_dims[curr_x][curr_y] = hom_dims[curr_x][curr_y - 1];

    //take each column with index in [first_col,last_col] to be a pivot column.
    for (int j = first_col; j <= last_col; j++) {

        changing_column = false;
        l = mx.low_finalized(j);

        if (l != -1 && lows[l] != -1 && lows[l] < j) {
            //if we get here then we are going to change the j^{th} column.
            changing_column = true;
            mx.remove_low(j);
        }

        //while column j is nonempty and its low number is found
        //in the low array, do column operations
        while (l != -1 && lows[l] != -1 && lows[l] < j) {
            c = lows[l];
            mx.add_column_popped(c, j);
            l = mx.remove_low(j);
        }

        if (l != -1) //column is still nonempty
        {
            //if we changed the column, we must put back the last popped entry.
            if (changing_column) {
                mx.push_index(j, l);
                mx.finalize(j);
            }

            lows[l] = j;

            //NOTE: Could be *slightly* more efficient if the for loop was split
            //into two parts, so that we didn't have to check this condition,
            //but it is probably worth not changing.
            if (j >= first_col_curr_bigrade) {
                //copy this column into the new matrix.
                new_high.mat.append_col(mx, j);
            }

            hom_dims[curr_x][curr_y]++;
        }
    } // reductions complete

    // to record the bigrades of the columns added to new_high, we update the IndexMatrix.
    new_high.ind.set(curr_y, curr_x, new_high.mat.width() - 1);

} //min_gens_and_clearing_data_one_bigrade()

void Presentation::kernel_coordinates(BigradedMatrixLex& high_mat,
    const BigradedMatrix& kernel)
{
    //create and initialize the low array
    std::vector<int> ker_lows(kernel.mat.height(), -1);

    for (unsigned i = 0; i < kernel.mat.width(); i++) {
        ker_lows[kernel.mat.low_finalized(i)] = i;
    }

    unsigned num_cols_added = 0;
    /* proceeding in colex order on bigrades of columns of high_mat, re-express 
     each column of high_mat with respect to the columns of the kernel, and 
     place the resulting values into mat. note that columns of high_mat are 
     initially in lex order.
    */
    for (unsigned y = 0; y < high_mat.ind.height(); y++) {
        for (unsigned x = 0; x < high_mat.ind.width(); x++) {
            kernel_coordinates_one_bigrade(high_mat, kernel, x, y, ker_lows, num_cols_added);
        }
    }
}

void Presentation::kernel_coordinates_one_bigrade(BigradedMatrixLex& high_mat,
    const BigradedMatrix& kernel,
    const unsigned& curr_x,
    const unsigned& curr_y,
    const std::vector<int>& ker_lows,
    unsigned& num_cols_added)
{
    int c;
    int l;
    MapMatrix& hi_mx = high_mat.mat;

    int first_col = high_mat.ind.start_index(curr_y, curr_x);
    int last_col = high_mat.ind.get(curr_y, curr_x);

    //take each column with index in [first_col,last_col] to be a pivot column.
    for (int j = first_col; j <= last_col; j++) {

        l = hi_mx.remove_low(j);

        //reduce the jth column to zero, storing the reduction indices in ker_mx.
        while (l != -1) {
            c = ker_lows[l];

            //add column c of kernel matrix to column j of the high_matrix.
            hi_mx.add_column_popped(kernel.mat, c, j);

            //add c to num_cols_added^{th} column of mat.
            mat.set(c, num_cols_added);

            //remove next element of jth column of high_mat
            l = hi_mx.remove_low(j);
        }

        //column of high_mat is now empty and the corresponding column has been
        //added to mat.

        //now heapify that column?
        //In fact, if we are just going to minimize presentation,
        //there is no point in heapifying, because the minimization procedure will
        //sort rows.
        //mat.prepare_col(num_cols_added);

        num_cols_added++;
    }

    //to record the bigrades of the columns added to kernel, we update the
    //IndexMatrix col_ind
    col_ind.set(curr_y, curr_x, num_cols_added - 1);
} //kernel_coordinates_one_bigrade()

/*
 Throws an exception if !is_kernel_minimal.
 Minimizes presentation using only column operations.
 This requires finding column entries which are not necessarily pivots, so this 
 code sorts each column first, and then finds the entries using binary search.
 Columns are added in a way that maintains the sorted order.
 */
void Presentation::minimize(int verbosity)
{
    if (!is_kernel_minimal)
        throw std::runtime_error("Presentation::minimize() : Presentation is not kernel minimal.\n");

    //new index matrix
    IndexMatrix row_ind_new(row_ind.height(), row_ind.width());

    //new_row_indices[i] stores the new index in the minimal presentation
    //corresponding to the row index i in the unminimized presentation.
    std::vector<int> new_row_indices(mat.height(), 0);

    Timer timer;
    timer.restart();

    //sort each column of presentation
    for (unsigned i = 0; i < mat.width(); i++)
        mat.sort_col(i);

    if (verbosity >= 4) {
        std::cout << "  --> sorting columns of unminimized presentation took "
                  << timer.elapsed() << " milliseconds." << std::endl;
    }

    timer.restart();

    //stores the next place we should move a column that we are not minimizing.
    int num_cols_kept = 0;

    //variables to work with index matrices
    Grade curr_grade(0, 0);
    Grade prev_grade(0, 0);

    int pivot_i;

    //for each column i with bigrade equal to that of pivot_i, add column i to
    //columns to its right, in order to ensure that pivot_i does not appear as a
    //nonzero entry to the right of column i.

    for (unsigned i = 0; i < mat.width(); i++) {

        //get the grade of the ith column
        while (col_ind.get(curr_grade.y, curr_grade.x) < (int)i)
            col_ind.next_colex(curr_grade.y, curr_grade.x);
        //after exiting the while loop, curr_grade is equal to the bigrade of column i.

        //decide whether the grade of ith column and that of pivot_i are equal
        pivot_i = mat.low_sorted(i);

        if (row_index_has_matching_bigrade(pivot_i, curr_grade.y, curr_grade.x)) {
            //if we're here then the grades of the column and pivot match

            //mark pivot_i for removal from row indices
            new_row_indices[pivot_i] = -1;

//zero out the part of the row pivot_i to the right of i.
//The part of the row to the left is already zero.
#pragma omp parallel for
            for (unsigned j = i + 1; j < mat.width(); j++) {

                if (mat.entry_sorted(pivot_i, j)) {
                    //if we're here then pivot_i is contained in column j
                    //add column i to column j to clear pivot_i.

                    mat.add_column_sorted(i, j);
                }
            }

            //NOTE: this function also sets prev_grade to curr_grade
            update_col_and_row_inds(row_ind_new, prev_grade, curr_grade, num_cols_kept - 1);
        }

        else {

            //if we get here then the grades of the column and pivot don't match.
            //We move this column to the appropriate place and update
            //IndexMatrix accordingly

            //move column i to index num_cols_kept
            mat.move_col(i, num_cols_kept);

            //Note that this function also updates prev_grade to be equal to curr_grade
            update_col_and_row_inds(row_ind_new, prev_grade, curr_grade, num_cols_kept - 1);

            //Now update the IndexMatrix row_ind.
            num_cols_kept++;
        }
    }

    //finish updating the row and column indices
    update_col_and_row_inds(row_ind_new, curr_grade, Grade(0, col_ind.height()), num_cols_kept - 1);
    row_ind = row_ind_new;

    if (verbosity >= 4) {
        std::cout << "  --> the column operations to minimize the presentation took "
                  << timer.elapsed() << " milliseconds." << std::endl;
    }

    timer.restart();

    unsigned new_height = mat.height() - (mat.width() - num_cols_kept);

    //Compute the reindexing vector.
    unsigned next_index = 0;
    for (unsigned i = 0; i < new_row_indices.size(); i++) {
        if (new_row_indices[i] != -1) {
            new_row_indices[i] = next_index;
            next_index++;
        }
    }

    //trim the unneeded columns
    mat.resize(mat.height(), num_cols_kept);

    //replace old row indices with new ones
    reindex_min_pres(new_row_indices);

    //trim the unneeded rows.  (This amounts to just changing a stored number.)
    mat.resize(new_height, num_cols_kept);

    if (verbosity >= 4) {
        std::cout << "  --> resizing and reindexing the minimal presentation took "
                  << timer.elapsed() << " milliseconds." << std::endl;
    }

    //we've finished minimizing the presentation
    is_minimized = true;
}

//Technical function for constructing hom_dims at all indices from an FIRep.
//Used by the Presentation constructor.
void Presentation::compute_hom_dims(const IndexMatrix& ind)
{
    //to compute the pointwise dimension of the kernel at index (i,j), I need
    //this quantity at (i,j-1).  We store the values at the previous row in the
    //following vector.
    std::vector<unsigned> ker_dims_prev_row(col_ind.width(), 0);

    //compute hom dims_along bottom row, and initialize ker_dims_prev_row
    for (unsigned x = 0; x < col_ind.width(); x++) {
        ker_dims_prev_row[x] = ind.get(0, x) + 1;
        hom_dims[x][0] = ker_dims_prev_row[x] - hom_dims[x][0];
    }

    for (unsigned y = 1; y < col_ind.height(); y++) {
        for (unsigned x = 0; x < col_ind.width(); x++) {
            //before this line is executed, hom_dims[x][y] is equal to the
            //pointwise rank of the high matrix in the firep at (x,y)
            ker_dims_prev_row[x] = ind.get(y, x) - ker_dims_prev_row[col_ind.width() - 1] + ker_dims_prev_row[x] + 1;
            hom_dims[x][y] = ker_dims_prev_row[x] - hom_dims[x][y];
        }
    }
}

void Presentation::update_col_and_row_inds(IndexMatrix& row_ind_new,
    Grade& start_grade,
    const Grade& end_grade,
    const int& value)
{
    Grade& gr = start_grade;
    while (!(gr == end_grade)) {
        //decrement value of row_ind at (x,y) by the appropriate amount
        row_ind_new.set(gr.y, gr.x, row_ind.get(gr.y, gr.x) - ((col_ind.get(gr.y, gr.x) - value)));

        //set value of col_ind at (x,y)
        col_ind.set(gr.y, gr.x, value);

        //increment w.r.t. colex order on the grid of bigrades.
        col_ind.next_colex(gr.y, gr.x);
    }
}

//Technical function used by Presentation::minimize()
void Presentation::reindex_min_pres(const std::vector<int>& new_row_indices)
{
    for (unsigned i = 0; i < mat.width(); i++) {
        mat.reindex_column(i, new_row_indices);
    }
}

//Technical function by Presentation::minimize()
//NOTE: Works correctly only in the setting in which the function is used.
//In that setting curr_row_index will have bigrade AT MOST (row, col) w.r.t the
//order on R^2.
bool Presentation::row_index_has_matching_bigrade(int curr_row_index, unsigned row, unsigned col)
{
    if (curr_row_index != row_ind.last()) {
        return curr_row_index >= row_ind.start_index(row, col);
    } else {
        //it could be that curr_row_index is the only row_index appearing at bigrade (row,col).
        if (col > 0) {
            return curr_row_index > row_ind.get(row, col - 1);
        } else {
            if (row > 0) {
                return curr_row_index > row_ind.get(row - 1, row_ind.width() - 1);
            } else {
                //If bigrade of curr_row_index is at most (0,0), then it is exactly (0,0)
                return true;
            }
        }
    }
}

void Presentation::print() const
{
    std::cout << "Number of rows:" << row_ind.last() + 1 << std::endl;
    std::cout << "Row bigrades:" << std::endl;
    row_ind.print_bigrades_vector();
    std::cout << "Number of columns:" << col_ind.last() + 1 << std::endl;
    std::cout << "Column bigrades:" << std::endl;
    col_ind.print_bigrades_vector();
    mat.print();
}

void Presentation::print_sparse() const
{
    std::cout << "Number of rows:" << row_ind.last() + 1 << std::endl;
    std::cout << "Row bigrades:" << std::endl;
    row_ind.print_bigrades_vector();
    std::cout << "Number of columns:" << col_ind.last() + 1 << std::endl;
    std::cout << "Column bigrades:" << std::endl;
    col_ind.print_bigrades_vector();
    mat.print_sparse();
}
