/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
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
/* map matrix class
 * stores a matrix representing a simplicial map
 */

#include "map_matrix.h"
#include "phat_mod/include/phat/representations/vector_heap_mod.h"
#include "bool_array.h"
#include "debug.h"
#include "index_matrix.h"
#include <numeric> //for std::accumulate
#include <stdexcept> //for error-checking and debugging

/********** implementation of base class MapMatrix_Base **********/

//constructor to create matrix of specified size (all entries zero)
MapMatrix_Base::MapMatrix_Base(unsigned rows, unsigned cols)
    : num_rows(rows)
{
    matrix._set_num_cols(cols);
}

//constructor to create a (square) identity matrix
MapMatrix_Base::MapMatrix_Base(unsigned size)
    : num_rows(size)
{
    matrix._set_num_cols(size);
    for (unsigned i = 0; i < size; i++) {
        //correct syntax?
        auto temp_col=std::vector<phat::index>();
        temp_col.push_back(i);
        matrix._set_col(i,temp_col);
    }
}

MapMatrix_Base::~MapMatrix_Base() = default;

//returns the number of columns in the matrix
unsigned MapMatrix_Base::width() const
{
    return matrix._get_num_cols();
}

//returns the number of rows in the matrix
unsigned MapMatrix_Base::height() const
{
    return num_rows;
}


//sets (to 1) the entry in row i, column j
//WARNING: The implementation assumes this entry has not been set yet.
void MapMatrix_Base::set(unsigned i, unsigned j)
{
    matrix._set_entry(i,j);
} //end set()

//TODO: Don't think we need this anymore.
/*
//returns true if entry (i,j) is 1, false otherwise
bool MapMatrix_Base::entry(unsigned i, unsigned j) const
{
    return matrix._is_in_matrix(i,j);
} //end entry()
*/
 
//adds column j to column k
//  RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
void MapMatrix_Base::add_to(unsigned j, unsigned k)
{
    matrix._add_to(j,k);
} //end add_to()

/********** implementation of class MapMatrix, for column-sparse matrices **********/

//constructor that sets initial size of matrix
MapMatrix::MapMatrix(unsigned rows, unsigned cols)
    : MapMatrix_Base(rows, cols)
{
}

//constructor to create a (square) identity matrix
MapMatrix::MapMatrix(unsigned size)
    : MapMatrix_Base(size)
{
}


//Mike: Fixing this constructor looks like a bit of a pain, because it is written in a way that prioritizes rows, whereas PHAT matrices prioritize columns.  Easier to just comment for now.
/*
//Mike: This constructor was copying data unnecessarily, so I fixed it.  But it as far as I know, it is not used at all in the rest of the code, except for the unit tests.
MapMatrix::MapMatrix(std::initializer_list<std::initializer_list<int>> values)
    : MapMatrix_Base(values.size(),
          std::accumulate(values.begin(), values.end(), 0U,
              [](unsigned max_so_far, const std::initializer_list<int>& row) {
                  return std::max(max_so_far, static_cast<unsigned>(row.size()));
              }))
{
    
    //TODO: Make this fast once we pick a representation.
    auto row_it = values.begin();
    for (unsigned row = 0; row < values.size(); row++) {
        auto col_it = row_it->begin();
        for (unsigned col = 0; col < row_it->size(); col++) {
            if (*col_it)
                set(row, col);
            ++col_it;
        }
        ++row_it;
    }
}
*/
 

//returns the number of columns in the matrix
unsigned MapMatrix::width() const
{
    return MapMatrix_Base::width();
}

//returns the number of rows in the matrix
unsigned MapMatrix::height() const
{
    return MapMatrix_Base::height();
}


//requests that the columns vector have enough capacity for num_cols columns
void MapMatrix::reserve_cols(unsigned num_cols)
{
    matrix._reserve_cols(num_cols);
}

//resize the matrix to the specified number of columns
void MapMatrix::resize(unsigned num_cols)
{
    matrix._set_num_cols(num_cols);
}

//resize the matrix to the specified number of rows and columns
void MapMatrix::resize(unsigned n_rows, unsigned n_cols)
{
    resize(n_cols);
    num_rows = n_rows;
}


//sets (to 1) the entry in row i, column j
void MapMatrix::set(unsigned i, unsigned j)
{
    MapMatrix_Base::set(i, j);
}

/*
//returns true if entry (i,j) is 1, false otherwise
bool MapMatrix::entry(unsigned i, unsigned j) const
{
    return MapMatrix_Base::entry(i, j);
}
*/
 
//returns the "low" index in the specified column, or 0 if the column is empty or does not exist
int MapMatrix::low(unsigned j) const
{
    return matrix._get_max_index(j);
}

//returns the "low" index in the specified column, or 0 if the column is empty or does not exist.
//same as the above, but only valid if the column is finalized
int MapMatrix::low_finalized(unsigned j) const
{
    return matrix._get_max_index_finalized(j);
}

//same as the above, but removes the low.
int MapMatrix::remove_low(unsigned j)
{
    return matrix._remove_max(j);
}

//Assuming column l is already heapified, adds l to the column and fixes heap.
void MapMatrix::push_index(unsigned j, unsigned l)
{
    
    return matrix._push_index(j,l);
}

//returns true iff column j is empty
bool MapMatrix::col_is_empty(unsigned j) const
{
    return matrix._is_empty(j);
}

//adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
void MapMatrix::add_column(unsigned j, unsigned k)
{
    MapMatrix_Base::add_to(j, k);
}

//TODO: Probably only used to compute Betti numbers, so perhaps should move with the other specialized functions for that
//adds column j from MapMatrix other to column k of this matrix
void MapMatrix::add_column(const MapMatrix* other, unsigned j, unsigned k)
{
    //make sure this operation is valid
    //if (other->columns.size() <= j || columns.size() <= k)
    //    throw std::runtime_error("MapMatrix::add_column(): attempting to access column(s) past end of matrix");
    matrix._add_to(other->matrix,j,k);
}

//wraps the add_to_popped() function in vector_heap_mod. See that code for an explanation.
void MapMatrix::add_column_popped(unsigned j, unsigned k)
{
    matrix._add_to_popped(j,k);
}

//same as above, but column j now comes from another matrix.
void MapMatrix::add_column_popped(const MapMatrix& other, unsigned j, unsigned k)
{
    matrix._add_to_popped(other.matrix,j,k);
}

//heapify the column
void MapMatrix::prepare_col(unsigned j)
{
    matrix._heapify_col(j);
}

void MapMatrix::finalize(unsigned i)
{
    matrix._finalize(i);
}


/********* Methods used to compute a presentation *********/

 //copies with index j from other to the back of this matrix
 void MapMatrix::append_col(const MapMatrix& other, unsigned j)
 {
     matrix._append_col(*other.matrix._get_const_col_iter(j));
 }

//Move column with index source to index target, zeroing out this column source in the process.
void MapMatrix::move_col(unsigned source, unsigned target)
{
    matrix._move_col(source, target);
}

//Move the ith column of other to jth this matrix, zeroing out ith column of other in the process.
void MapMatrix::move_col(MapMatrix& other, unsigned i, unsigned j)
{
    matrix._move_col(*(other.matrix._get_col_iter(i)),j);
    
}

//TODO:I think and the related functions can be removed.
/*
//Move the jth column of other to the back of this matrix, zeroing out this column of other in the process.
void MapMatrix::append_col_and_clear(MapMatrix& other, unsigned j)
{
    matrix._append_col_and_clear(*(other.matrix._get_col_iter(j)));
    
}
*/

/********* Methods used to minimize a presentation *********/

void MapMatrix::sort_col(int i)
{
    matrix._sort_col(i);
}

// reindex column col using the indices given in new_row_indices.
void MapMatrix::reindex_column(unsigned col, const std::vector<int>& new_row_indices)
{
    matrix._reindex_column(col,new_row_indices);
}


/********* Next three methods are used only by the MultiBetti class *********/


//copies NONZERO columns with indexes in [first, last] from other, appending them to this matrix to the right of all existing columns
//  all row indexes in copied columns are increased by offset
void MapMatrix::copy_cols_from(const MapMatrix* other, int first, int last, unsigned offset)
{
    phat::index idx = matrix._get_num_cols();
    matrix._set_num_cols(idx + (last - first + 1));
    
    std::vector<phat::index> temp_col;
    for(phat::index j = first; j <= last; j++) {
        
        //it is an iterator pointing to the jth column of matrix.
        auto it=other->matrix._get_const_col_iter(j);
        for(unsigned i = 0; i < it->size(); i++)
                matrix._set_entry(*(it->begin()+i)+offset,idx);
        idx++;
    }
}

//copies columns with indexes in [first, last] from other, inserting them in this matrix with the same column indexes
void MapMatrix::copy_cols_same_indexes(const MapMatrix* other, int first, int last)
{
    //std::vector<phat::index> temp_col;
    for(phat::index j = first; j <= last; j++) {
        matrix._set_col(j,*(other->matrix._get_const_col_iter(j)));
    }
}


//TODO: It would be more natural to make this function a member of a bigraded matrix class, or as a member of the MultiBetti class
//removes zero columns from this matrix
//ind_old gives grades of columns before zero columns are removed; new grade info stored in ind_new
//NOTE: ind_old and ind_new must have the same size!
void MapMatrix::remove_zero_cols(const IndexMatrix& ind_old, IndexMatrix& ind_new)
{
    phat::index new_idx = -1; //new index of rightmost column that has been moved
    phat::index cur_idx = 0; //old index of rightmost column considered for move
    phat::index end_col;
    //debug() << "REMOVING ZERO COLS: (" << matrix._get_num_cols() << "cols)";
    //        print();
    //        ind_old->print();
    
    //loop over all grades
    for(unsigned y = 0; y < ind_old.height(); y++) {
        for(unsigned x = 0; x < ind_old.width(); x++) {
            end_col = ind_old.get(y, x); //index of rightmost column at this grade
            for(; cur_idx <= end_col; cur_idx++) { //loop over all columns at this grade
                if( !matrix._is_empty(cur_idx) ) { //then move column
                    new_idx++; //new index of this column
                    matrix._set_col(new_idx,*matrix._get_col_iter(cur_idx));
                }
            }
            ind_new.set(y, x, new_idx); //rightmost column index for this grade
        }
    }
    
    //resize the columns vector
    //TODO: For the heap representation of a column, this resets the insert count to 0.  Is this what we want? should be okay if the columns are being finalized appropriately elsewhere, otherwise is wierd.
    matrix._set_num_cols(new_idx + 1);
    
    //debug() << "RESULTING MATRIX: (" << matrix._get_num_cols() << "cols)";
    //        print();
    //        ind_new->print();
} //end remove_zero_cols

//function to print the matrix to standard output, for testing purposes
void MapMatrix::print()
{
    matrix._print(num_rows);
    //std::cout << "MapMatrix::print() finished call to _print!" << std::endl;
} //end print()


/********** methods of the class MapMatrix which assume that the column(s) in question are sorted  **********/

//same as add_column above, but requires columns to be sorted vectors.
void MapMatrix::add_column_sorted(unsigned j, unsigned k)
{
    matrix._add_to_sorted(j , k);
}


//returns true if entry (i,j) is 1, false otherwise
bool MapMatrix::entry_sorted(unsigned i, unsigned j) const {
    return matrix._is_in_matrix_sorted(i,j);
}

//returns entry of column i with largest index, if the column is non empty.  Returns -1 otherwise.
int MapMatrix::low_sorted(unsigned i) const {
    return matrix._get_max_index_sorted(i);
}



/********** implementation of class MapMatrix_Perm, supports row swaps (and stores a low array) **********/

//Constructor
//Constructs a copy of the matrix with columns in a specified order, and some columns possibly removed -- for vineyard-update algorithm
//    simplex_order is a map which sends each column index to its new index in the permutation.
//        if simplex_order[i] == -1, then column i is NOT represented in the matrix being built
//    num_simplices is the number of columns we keep (i.e., the number of entries in the vector that are NOT -1)
MapMatrix_Perm::MapMatrix_Perm(const MapMatrix& mat, const std::vector<int>& coface_order, unsigned num_cofaces)
    : matrix(mat.height(),num_cofaces)
    , low_by_row(mat.height(), -1)
    , low_by_col(num_cofaces, -1) // col_perm(cols)

{
    
    int order_index;
    //loop through all simplices, writing columns to the matrix
    for (unsigned i = 0; i < mat.width(); i++) {
        order_index  = coface_order[i]; //index of the matrix column which will store the boundary of this simplex
        if (order_index != -1) {
            //NOTE: Permissions here are okay because MapMatrix is a friend class.
            matrix._set_col(order_index,*(mat.matrix._get_const_col_iter(i)));
        }
    }

} //end constructor

//Constructor
//Constructs a copy of the matrix with columns and rows in specified orders, and some rows/columns possibly removed -- for the vineyard-update algorithm
//  PARAMETERS:
//    each vector represents a map sends a row/column index to its new index in the permutation.
//        if the value of the map is -1 at index i, then the row/column at index i is NOT represented in the boundary matrix
//    each unsigned is the number of simplices in the corresponding order (i.e., the number of entries in the vector that are NOT -1)
MapMatrix_Perm::MapMatrix_Perm(const MapMatrix& mat, const std::vector<int>& face_order, unsigned num_faces, const std::vector<int>& coface_order, const unsigned num_cofaces)
    : matrix(num_faces,num_cofaces)
    , low_by_row(num_faces, -1)
    , low_by_col(num_cofaces, -1) // col_perm(cols)
    
    {
    //create the matrix
    int order_index;
    for (unsigned i = 0; i < mat.width(); i++) {
        order_index = coface_order[i]; //index of the matrix column which will store the boundary of this simplex
        if (order_index != -1) {
            matrix._set_col(order_index,*(mat.matrix._get_const_col_iter(i)),face_order);
        }
    }
} //end constructor


//returns the number of columns in the matrix
unsigned MapMatrix_Perm::width() const
{
    return matrix._get_num_cols();
}

//returns the number of rows in the matrix
unsigned MapMatrix_Perm::height() const
{
    return matrix._get_num_rows();
}

//returns true if entry (i,j) is 1, false otherwise
bool MapMatrix_Perm::entry(unsigned i, unsigned j) const
{
    return matrix._is_in_matrix(i, j);
}

//reduces this matrix and returns the corresponding upper-triangular matrix for the RU-decomposition
//NOTE -- only to be called before any rows are swapped!
//NOTE -- this is just the standard persistence algorithm, but with some tweaks
MapMatrix_RowPriority_Perm* MapMatrix_Perm::decompose_RU()
{
    
    //Create U
    MapMatrix_RowPriority_Perm* U = new MapMatrix_RowPriority_Perm(width()); //NOTE: must be deleted
    
    int c;
    int l;
    bool changing_column;
    
    //loop through columns of this matrix
    for (unsigned j = 0; j < width(); j++) {
        //while column j is nonempty and its low number is found in the low array, do column operations
        
        //NOTE: We don't call MapMatrix_Perm::low() because in our application of this method, low_by_col has not yet been properly initialized.
        
        changing_column = false;
        l=matrix._get_max_index_finalized(j);
        
        if (l != -1 && low_by_row[l] != -1 )
        {
            //if we get here then we are going to change the j^{th} column.
            changing_column = true;
            matrix._remove_max(j);
        }
        
        while (l != -1  && low_by_row[l] != -1 )
        {
            c = low_by_row[l];
            
            //For efficiency, we use a special version of add_column which knows that column c has been finalized and the pivot of column j has been popped.
            
            matrix._add_to_popped(c, j);
            
            U->add_row(j, c); //perform the opposite row operation on U
            l=matrix._remove_max(j);
        }
        
        if (l != -1 ) //then column is still nonempty.
        {
            //Update lows
            low_by_col[j] = l;
            low_by_row[l] = j;
            if (changing_column)
            {
                //if we changed the column, put back the pivot we popped off last and finalize.
                matrix._push_index(j,l);
                matrix._finalize(j);
            }
        }
    }
    //return the matrix U
    return U;
} //end decompose_RU()

//returns the row index of the lowest entry in the specified column, or -1 if the column is empty
int MapMatrix_Perm::low(unsigned j) const
{
    return low_by_col[j];
}

//returns the index of the column with low l, or -1 if there is no such column
int MapMatrix_Perm::find_low(unsigned l) const
{
    return low_by_row[l];
}

//returns true iff column j is empty
bool MapMatrix_Perm::col_is_empty(unsigned j) const
{
    return matrix._is_empty(j);
}

//adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
void MapMatrix_Perm::add_column(unsigned j, unsigned k)
{
    matrix._add_to(j, k);
}

//transposes rows i and i+1
//NOTE: this causes low array to be incorrect iff there are columns k and l with low(k)=i, low(l)=i+1, and M[i,l]=1  (as in Vineyards, Case 1.1)
//      the user must detect this and do a column operation to restore the matrix to a reduced state!
void MapMatrix_Perm::swap_rows(unsigned i, bool update_lows)
{
    //swap rows
    matrix._swap_rows(i);

    //update low arrays
    if (update_lows) {
        int l = low_by_row[i];
        int k = low_by_row[i + 1];

        low_by_row[i] = k;
        low_by_row[i + 1] = l;

        if (l != -1)
            low_by_col[l] = i + 1;
        if (k != -1)
            low_by_col[k] = i;
    }
} //end swap_rows()

//transposes columns j and j+1
void MapMatrix_Perm::swap_columns(unsigned j, bool update_lows)
{
    //swap columns
    matrix._swap_columns(j,j+1);

    //update low arrays
    if (update_lows) {
        int l = low_by_col[j];
        int k = low_by_col[j + 1];

        low_by_col[j] = k;
        low_by_col[j + 1] = l;

        if (l != -1)
            low_by_row[l] = j + 1;
        if (k != -1)
            low_by_row[k] = j;
    }
}

//clears the matrix, then rebuilds it from reference with columns permuted according to col_order
//  NOTE: reference should have the same size as this matrix!
//  col_order is a map: (column index in reference matrix) -> (column index in rebuilt matrix)
void MapMatrix_Perm::rebuild(MapMatrix_Perm* reference, const std::vector<unsigned>& col_order)
{
    //clear the matrix
    for (unsigned i = 0; i < matrix._get_num_cols(); i++) {matrix._clear(i);}

    //reset low arrays
    for (unsigned i = 0; i < matrix._get_num_rows(); i++)
        low_by_row[i] = -1;
    for (unsigned j = 0; j < matrix._get_num_cols(); j++)
        low_by_col[j] = -1;
    
    //TODO: Why was this block of code here, anyway? This is for rebuilding the lower matrix in an FIRep, and for that, the permutation of the rows is trivial.
    //TODO: Relatedly, it might be better design to actually take the lower matrix to be a MapMatrix, not MapMatrix perm, though that would be a  disruptive change.  It won't matter in the end, since the code will eventually work on a presentation matrix.
    /*
    //reset permutation vectors
    for (unsigned i = 0; i < num_rows; i++) {
        perm[i] = i;
        mrep[i] = i;
    }
    */

    //build the new matrix
    for (unsigned j = 0; j < matrix._get_num_cols(); j++) {
        //copy column j from reference into column col_order[j] of this matrix
        matrix._set_col(col_order[j],*(reference->matrix._get_col_iter(j)));
    }
} //end rebuild()

//TODO: Implement a more efficient rebuild procudure.  This one does unnecessary work in the front and back of the matrix.
//clears the matrix, then rebuilds it from reference with columns permuted according to col_order and rows permuted according to row_order
//  NOTE: reference should have the same size as this matrix!
//  col_order is a map: (column index in reference matrix) -> (column index in rebuilt matrix) and similarly for row_order
void MapMatrix_Perm::rebuild(MapMatrix_Perm* reference, const std::vector<unsigned>& col_order, const std::vector<unsigned>& row_order)
{
    ///TESTING: check the permutation
    //std::vector<bool> check(columns.size(), false);
    //for (unsigned j = 0; j < columns.size(); j++)
    //    check[col_order[j]] = true;
    //for (unsigned j = 0; j < columns.size(); j++)
    //    if (check[j] == false) {
    //        debug() << "ERROR: column permutation skipped" << j;
    //    }
    
    //clear the matrix
    for (unsigned i = 0; i < matrix._get_num_cols(); i++) {matrix._clear(i);}
    
    //reset low arrays
    for (unsigned i = 0; i < matrix._get_num_rows(); i++)
        low_by_row[i] = -1;
    for (unsigned j = 0; j < matrix._get_num_cols(); j++)
        low_by_col[j] = -1;
    
    //update implicit row order.
    //TODO: Could be more efficient; shouldn't have to completely copy the permutation over.
    matrix._set_perm(row_order);
    
    //build the new matrix
    for (unsigned j = 0; j < matrix._get_num_cols(); j++) {
        //NOTE: We reorder rows implicitly now, so this is quite simple.
        matrix._set_col(col_order[j],*(reference->matrix._get_col_iter(j)));
    }
} //end rebuild()


//function to print the matrix to standard output, for testing purposes
void MapMatrix_Perm::print()
{
    matrix._print();
}

/********** implementation of class MapMatrix_RowPriority_Perm **********/

//Initializes this matrix to the identity matrix.
MapMatrix_RowPriority_Perm::MapMatrix_RowPriority_Perm(unsigned size)
    : matrix(size)
    {}

//TODO: Is this necessary?
MapMatrix_RowPriority_Perm::~MapMatrix_RowPriority_Perm() = default;

unsigned MapMatrix_RowPriority_Perm::width() const
{
    return matrix._get_num_rows();
}

unsigned MapMatrix_RowPriority_Perm::height() const
{
    return matrix._get_num_cols();
}


/*
void MapMatrix_RowPriority_Perm::clear(unsigned i, unsigned j)
{
    MapMatrix_Base::clear(mrep[j], i);
}
*/
 
bool MapMatrix_RowPriority_Perm::entry(unsigned i, unsigned j) const
{
    return matrix._is_in_matrix(j, i);
}

//adds row j to row k; RESULT: row j is not changed, row k contains sum of rows j and k (with mod-2 arithmetic)
void MapMatrix_RowPriority_Perm::add_row(unsigned j, unsigned k)
{
    return matrix._add_to(j, k);
}

//transposes rows i and i+1
void MapMatrix_RowPriority_Perm::swap_rows(unsigned i)
{
    matrix._swap_columns(i,i+1);
}

//transposes columns j and j+1
void MapMatrix_RowPriority_Perm::swap_columns(unsigned j)
{
    matrix._swap_rows(j);
}


