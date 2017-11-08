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
    matrix.set_num_cols(cols);
}

//constructor to create a (square) identity matrix
MapMatrix_Base::MapMatrix_Base(unsigned size)
    : num_rows(size)
{
    matrix.set_num_cols(size);
    for (unsigned i = 0; i < size; i++) {
        //correct syntax?
        auto temp_col=std::vector<phat::index>();
        temp_col.push_back(i);
        matrix.set_col(i,temp_col);
    }
}

MapMatrix_Base::~MapMatrix_Base() = default;

//returns the number of columns in the matrix
unsigned MapMatrix_Base::width() const
{
    return matrix.get_num_cols();
}

//returns the number of rows in the matrix
unsigned MapMatrix_Base::height() const
{
    return num_rows;
}


//sets (to 1) the entry in row i, column j
void MapMatrix_Base::set(unsigned i, unsigned j)
{
    matrix.set_entry(i,j);
} //end set()


/*
//clears (sets to 0) the entry in row i, column j
void MapMatrix_Base::clear(unsigned i, unsigned j)
{
    //make sure this entry is valid
    if (columns.size() <= j)
        throw std::runtime_error("MapMatrix_Base::clear(): attempting to clear entry in a column past end of matrix");
    if (num_rows <= i)
        throw std::runtime_error("MapMatrix_Base::clear(): attempting to clear entry in a row past end of matrix");

    //if column is empty, then do nothing
    if (columns[j] == NULL)
        return;

    //column is not empty, so get initial node pointer in the column
    MapMatrixNode* current = columns[j];

    //see if the first node is the one we want
    if (current->get_row() < i)
        return; //because the entry in row i must be zero (row entries are sorted in decreasing order)

    if (current->get_row() == i) {
        columns[j] = current->get_next();
        delete current;
        return;
    }

    //traverse the nodes in this column after the first node
    while (current->get_next() != NULL) {
        MapMatrixNode* next = current->get_next();

        if (next->get_row() < i) //then the entry in row i must be zero (row entries are sorted in decreasing order)
            return;

        if (next->get_row() == i) //then we found the row we wanted
        {
            current->set_next(next->get_next());
            delete next;
            return;
        }

        //if we are still looking, then get the next node
        current = next;
    }
} //end clear()
*/
 
 
//returns true if entry (i,j) is 1, false otherwise
bool MapMatrix_Base::entry(unsigned i, unsigned j) const
{
    return matrix.is_in_matrix(i,j);
} //end entry()

//adds column j to column k
//  RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
void MapMatrix_Base::add_to(unsigned j, unsigned k)
{
    matrix.add_to(j,k);
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
 
//TODO:I think we don't need this.  Remove.
/*
bool MapMatrix::operator==(MapMatrix& other)
{
    //TODO: make fast once we choose a representation

    if (height() != other.height() || width() != other.width())
        return false;
    for (unsigned row = 0; row < height(); row++)
        for (unsigned col = 0; col < width(); col++)
            if (entry(row, col) != other.entry(row, col))
                return false;

    return true;
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
    matrix.reserve_cols(num_cols);
}

//sets (to 1) the entry in row i, column j
void MapMatrix::set(unsigned i, unsigned j)
{
    MapMatrix_Base::set(i, j);
}


//returns true if entry (i,j) is 1, false otherwise
bool MapMatrix::entry(unsigned i, unsigned j) const
{
    return MapMatrix_Base::entry(i, j);
}

 
//returns the "low" index in the specified column, or 0 if the column is empty or does not exist
int MapMatrix::low(unsigned j) const
{
    //make sure this query is valid
    /*
    if (columns.size() <= j)
        throw std::runtime_error("MapMatrix::low(): attempting to check low number of a column past end of matrix");
     */
    return matrix.get_max_index(j);
}

//returns true iff column j is empty
bool MapMatrix::col_is_empty(unsigned j) const
{
    return matrix.is_empty(j);
}

//adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
void MapMatrix::add_column(unsigned j, unsigned k)
{
    MapMatrix_Base::add_to(j, k);
}

//adds column j from MapMatrix other to column k of this matrix
void MapMatrix::add_column(const MapMatrix* other, unsigned j, unsigned k)
{
    //make sure this operation is valid
    //if (other->columns.size() <= j || columns.size() <= k)
    //    throw std::runtime_error("MapMatrix::add_column(): attempting to access column(s) past end of matrix");
    matrix.add_to(other->matrix,j,k);
}

void MapMatrix::prepare_col(unsigned i)
{
    matrix.prepare_col(i);
}

//copies column with index src_col from other to column dest_col in this matrix
void MapMatrix::copy_col_from(const MapMatrix* other, unsigned src_col, unsigned dest_col)
{
    matrix.set_col(dest_col,*(other->matrix.get_col_iter(src_col)));
}
//end copy_col_from()

//copies NONZERO columns with indexes in [first, last] from other, appending them to this matrix to the right of all existing columns
//  all row indexes in copied columns are increased by offset
void MapMatrix::copy_cols_from(const MapMatrix* other, int first, int last, unsigned offset)
{
    phat::index idx = matrix.get_num_cols();
    matrix.set_num_cols(idx + (last - first + 1));
    
    std::vector<phat::index> temp_col;
    for(phat::index j = first; j <= last; j++) {
        other->matrix.get_col(j, temp_col);
        if(offset > 0) {
            for(unsigned i = 0; i < temp_col.size(); i++)
                temp_col[i] += offset;
        }
        matrix.set_col(idx, temp_col);
        idx++;
    }
}

//copies columns with indexes in [first, last] from other, inserting them in this matrix with the same column indexes
void MapMatrix::copy_cols_same_indexes(const MapMatrix* other, int first, int last)
{
    //std::vector<phat::index> temp_col;
    for(phat::index j = first; j <= last; j++) {
        matrix.set_col(j,*(other->matrix.get_col_iter(j)));
    }
}

//removes zero columns from this matrix
//ind_old gives grades of columns before zero columns are removed; new grade info stored in ind_new
//NOTE: ind_old and ind_new must have the same size!
void MapMatrix::remove_zero_cols(IndexMatrix* ind_old, IndexMatrix* ind_new)
{
    phat::index new_idx = -1; //new index of rightmost column that has been moved
    phat::index cur_idx = 0; //old index of rightmost column considered for move
    std::vector<phat::index> temp_col;
    
    debug() << "REMOVING ZERO COLS: (" << matrix.get_num_cols() << "cols)";
    //        print();
    //        ind_old->print();
    
    //loop over all grades
    for(unsigned y = 0; y < ind_old->height(); y++) {
        for(unsigned x = 0; x < ind_old->width(); x++) {
            int end_col = ind_old->get(y, x); //index of rightmost column at this grade
            for(; cur_idx <= end_col; cur_idx++) { //loop over all columns at this grade
                if( !matrix.is_empty(cur_idx) ) { //then move column
                    new_idx++; //new index of this column
                    matrix.get_col(cur_idx, temp_col);
                    matrix.set_col(new_idx, temp_col);
                }
            }
            ind_new->set(y, x, new_idx); //rightmost column index for this grade
        }
    }
    
    //resize the columns vector
    //TODO: For the heap representation of a column, this resets the insert count to 0.  Is this what we want? should be okay if the columns are being finalized appropriately elsewhere, otherwise is wierd.
    matrix.set_num_cols(new_idx + 1);
    
    debug() << "RESULTING MATRIX: (" << matrix.get_num_cols() << "cols)";
    //        print();
    //        ind_new->print();
} //end remove_zero_cols

void MapMatrix::finalize(unsigned i)
{
    matrix.finalize(i);
}

//returns a copy of the matrix with columns in a specified order, and some columns possibly removed -- for vineyard-update algorithm
//    simplex_order is a map which sends each column index to its new index in the permutation.
//        if simplex_order[i] == -1, then column i is NOT represented in the matrix being built
//    num_simplices is the number of columns we keep (i.e., the number of entries in the vector that are NOT -1)
MapMatrix_Perm* MapMatrix::get_permuted_and_trimmed_mx(const std::vector<int>& coface_order, unsigned num_simplices)
{
    //create the matrix
    
    MapMatrix_Perm* mat = new MapMatrix_Perm(this->height(), num_simplices); //NOTE: must be deleted
    int order_index;
    //loop through all simplices, writing columns to the matrix
    for (unsigned i = 0; i < this->width(); i++) {
        order_index  = coface_order[i]; //index of the matrix column which will store the boundary of this simplex
        if (order_index != -1) {
            mat->copy_col_from(this,i,order_index);
        }
    }
    
    //TODO: Arguably, it would be better design to initialize the low_by_column here, rather than in the method decompose_RU().  Currently, this function gives a partially initialized MapMatrix_Perm.
    
    //return the matrix
    return mat;
} //end get_permuted_and_trimmed_mx

//returns a copy of the matrix with columns and rows in specified orders, and some rows/columns possibly removed -- for vineyard-update algorithm
//  PARAMETERS:
//    each vector represents a map sends a row/column index to its new index in the permutation.
//        if the value of the map is -1 at index i, then the row/column at index i is NOT represented in the boundary matrix
//    each unsigned is the number of simplices in the corresponding order (i.e., the number of entries in the vector that are NOT -1)
MapMatrix_Perm* MapMatrix::get_permuted_and_trimmed_mx(const std::vector<int>& face_order, unsigned num_faces, const std::vector<int>& coface_order, const unsigned num_cofaces)
{
    //create the matrix
    MapMatrix_Perm* mat = new MapMatrix_Perm(num_faces, num_cofaces); //NOTE: must be deleted
    int order_index;
    for (unsigned i = 0; i < this->width(); i++) {
        order_index = coface_order[i]; //index of the matrix column which will store the boundary of this simplex
        if (order_index != -1) {
            mat->matrix.set_col(order_index,*(this->matrix.get_col_iter(i)),face_order);
        }
    }
    //return the matrix
    return mat;
} //end get_permuted_and_trimmed_mx


//TODO: Mike I don't want to bother implementing this
/*
std::ostream& operator<<(std::ostream& out, const MapMatrix& matrix)
{
    //handle empty matrix
    if (matrix.num_rows == 0 || matrix.columns.size() == 0) {
        out << "        (empty matrix:" << matrix.num_rows << "rows by" << matrix.columns.size() << "columns)";
        return out;
    }

    //create a 2D array of booleans to temporarily store the matrix
    bool_array mx(matrix.num_rows, matrix.columns.size());
    for (unsigned i = 0; i < matrix.num_rows; i++)
        for (unsigned j = 0; j < matrix.columns.size(); j++)
            mx.at(i, j) = false;

    //traverse the linked lists in order to fill the 2D array
    MapMatrix::MapMatrixNode* current;
    for (unsigned j = 0; j < matrix.columns.size(); j++) {
        current = matrix.columns[j];
        while (current != NULL) {
            int row = current->get_row();
            mx.at(row, j) = true;
            current = current->get_next();
        }
    }

    for (unsigned i = 0; i < matrix.num_rows; i++) {
        out << "        |";
        for (unsigned j = 0; j < matrix.columns.size(); j++) {
            if (mx.at(i, j))
                out << " 1";
            else
                out << " 0";
        }
        out << " |\n";
    }
    return out;
}
*/
 

//function to print the matrix to standard output, for testing purposes
void MapMatrix::print()
{
    matrix.print(num_rows);
} //end print()

 
/*
//check for inconsistencies in matrix column, for testing purposes
void MapMatrix::assert_cols_correct()
{
    for (unsigned j = 0; j < columns.size(); j++) {
        //consider the first node
        MapMatrixNode* current = columns[j];
        MapMatrixNode* next_node;

        //consider all following nodes
        while (current != NULL) {
            next_node = current->get_next();
            if (next_node != NULL && current->get_row() <= next_node->get_row()) {
                debug() << "===>>> ERROR IN COLUMN " << j;
            }
            current = next_node;
        }
    }
}

*/
 
/********** implementation of class MapMatrix_Perm, supports row swaps (and stores a low array) **********/

MapMatrix_Perm::MapMatrix_Perm(unsigned rows, unsigned cols)
    : MapMatrix(rows, cols)
    , perm(rows)
    , mrep(rows)
    , low_by_row(rows, -1)
    , low_by_col(cols, -1) // col_perm(cols)
{
    //initialize permutation vectors to the identity permutation
    for (unsigned i = 0; i < rows; i++) {
        perm[i] = i;
        mrep[i] = i;
    }
}

MapMatrix_Perm::MapMatrix_Perm(unsigned size)
    : MapMatrix(size)
    , perm(size)
    , mrep(size)
    , low_by_row(size, -1)
    , low_by_col(size, -1)
{
    //initialize permutation vectors to the identity permutation
    for (unsigned i = 0; i < size; i++) {
        perm[i] = i;
        mrep[i] = i;
    }
}

//Default copy constructor is the right one.
/*
//copy constructor
MapMatrix_Perm::MapMatrix_Perm(const MapMatrix_Perm& other)
    : MapMatrix(other.height(), other.width())
    , perm(other.perm)
    , mrep(other.mrep)
    , low_by_row(other.low_by_row)
    , low_by_col(other.low_by_col)
{
    //copy all matrix entries
    for (unsigned j = 0; j < other.width(); j++) {
        MapMatrixNode* other_node = other.columns[j];
        if (other_node != NULL) {
            //create the first node in this column
            MapMatrixNode* cur_node = new MapMatrixNode(other_node->get_row());
            columns[j] = cur_node;

            //create all other nodes in this column
            other_node = other_node->get_next();
            while (other_node != NULL) {
                MapMatrixNode* new_node = new MapMatrixNode(other_node->get_row());
                cur_node->set_next(new_node);
                cur_node = new_node;
                other_node = other_node->get_next();
            }
        }
    }
}
*/

//TODO:No longer need this
/*
MapMatrix_Perm::~MapMatrix_Perm()
{
}
*/
 
//TODO:Don't need this
/*
//sets (to 1) the entry in row i, column j
//NOTE: to be used for matrix construction only; does not update low array
void MapMatrix_Perm::set(unsigned i, unsigned j)
{
    MapMatrix::set(mrep[i], j);
}
*/

//returns true if entry (i,j) is 1, false otherwise
bool MapMatrix_Perm::entry(unsigned i, unsigned j) const
{
    return MapMatrix::entry(mrep[i], j);
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
    bool changing_column = false;
    
    //loop through columns of this matrix
    for (unsigned j = 0; j < width(); j++) {
        //while column j is nonempty and its low number is found in the low array, do column operations
        
        //NOTE: We don't call MapMatrix_Perm::low() because in our application of this method, low_by_col has not yet been properly initialized.  As a side effect, this method is initializing low_by_col.  This seems to be poor design, but it makes little difference in terms of computational efficiency.  I won't fix it right now. -Mike
        l=matrix.pop_max_index(j);
        if (l>=0 && low_by_row[l] >= 0)
            //if we get here then we are going to change the j^{th} column.
            changing_column = true;

        while (l>=0 && low_by_row[l] >= 0) {
            c = low_by_row[l];
            //special version of add_column which knows that column c has been finalized and the pivot of column j has been popped.
            matrix.add_to_popped(c, j);
            U->add_row(j, c); //perform the opposite row operation on U
            l=matrix.pop_max_index(j);
        }
        
        if (l>=0) //then column is still nonempty, so put back the pivot we popped off last, update lows
        {
            matrix.push_max_index(j,l);
            low_by_col[j] = l;
            low_by_row[l] = j;
        }
        
        //if we changed the column, it might not be finalized anymore, so finalize it.
        if (changing_column)
        {
            finalize(j);
            changing_column = false;
        }
    }
    //return the matrix U
    return U;
} //end decompose_RU()

//WARNING: This only behaves in the expected way once low_by_col has been properly initialized.
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

//transposes rows i and i+1
//NOTE: this causes low array to be incorrect iff there are columns k and l with low(k)=i, low(l)=i+1, and M[i,l]=1  (as in Vineyards, Case 1.1)
//      the user must detect this and do a column operation to restore the matrix to a reduced state!
void MapMatrix_Perm::swap_rows(unsigned i, bool update_lows)
{
    //get original row indexes of these rows
    unsigned a = mrep[i];
    unsigned b = mrep[i + 1];

    //swap entries in permutation and inverse permutation arrays
    unsigned temp = perm[a]; ///TODO: why do I do this? isn't temp == i?  Mike: Good question, looks so.
    perm[a] = perm[b];
    perm[b] = temp;

    mrep[i] = b;
    mrep[i + 1] = a;

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
    matrix.swap_columns(j);

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
    for (unsigned i = 0; i < matrix.get_num_cols(); i++) {matrix.clear(i);}

    //reset low arrays
    for (unsigned i = 0; i < num_rows; i++)
        low_by_row[i] = -1;
    for (unsigned j = 0; j < matrix.get_num_cols(); j++)
        low_by_col[j] = -1;

    //reset permutation vectors
    for (unsigned i = 0; i < num_rows; i++) {
        perm[i] = i;
        mrep[i] = i;
    }

    //build the new matrix
    for (unsigned j = 0; j < matrix.get_num_cols(); j++) {
        //copy column j from reference into column col_order[j] of this matrix
        matrix.set_col(col_order[j],*(reference->matrix.get_col_iter(j)));
    }
} //end rebuild()

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
    for (unsigned i = 0; i < matrix.get_num_cols(); i++) {matrix.clear(i);}

    //reset low arrays
    for (unsigned i = 0; i < num_rows; i++)
        low_by_row[i] = -1;
    for (unsigned j = 0; j < matrix.get_num_cols(); j++)
        low_by_col[j] = -1;

    //reset permutation vectors
    for (unsigned i = 0; i < num_rows; i++) {
        perm[i] = i;
        mrep[i] = i;
    }

    //build the new matrix
    for (unsigned j = 0; j < matrix.get_num_cols(); j++) {
        //copy column j from reference into column col_order[j] of this matrix, with the appropriate reindexing to account for the row permutation:
        std::vector<phat::index> temp_col;
        //TODO:Keep the permuatation as is, but change the order for heapification.  This would us to avoid touching some of the columns, for a more efficient reset.
        //temp_col=std::vector<index>();
        temp_col.reserve((reference->matrix.get_col_iter(j))->size());
        
        //copy column j from reference into column col_order[j] of this matrix, replacing each index with its image under the permutation row_order
        
        //TODO: Reserve space for this column?
        matrix.set_col(col_order[j],*(reference->matrix.get_col_iter(j)),row_order);
    }

} //end rebuild()

/*
//function to print the matrix to standard output, for testing purposes
void MapMatrix_Perm::print()
{
    //handle empty matrix
    if (num_rows == 0 || columns.size() == 0) {
        debug() << "        (empty matrix:" << num_rows << "rows by" << columns.size() << "columns)";
        return;
    }

    //create a 2D array of booleans to temporarily store the matrix
    bool_array mx(num_rows, columns.size());
    for (unsigned i = 0; i < num_rows; i++)
        for (unsigned j = 0; j < columns.size(); j++)
            mx.at(i, j) = false;

    //traverse the linked lists in order to fill the 2D array
    MapMatrixNode* current;
    for (unsigned j = 0; j < columns.size(); j++) {
        current = columns[j];
        while (current != NULL) {
            int row = current->get_row();
            mx.at(perm[row], j) = true;
            current = current->get_next();
        }
    }

    //print the matrix
    for (unsigned i = 0; i < num_rows; i++) {
        Debug qd = debug(true);
        qd << "        |";
        for (unsigned j = 0; j < columns.size(); j++) {
            if (mx.at(i, j))
                qd << " 1";
            else
                qd << " 0";
        }
        qd << " |";
    }
} //end print()
*/
 
/*
//check for inconsistencies in low arrays, for testing purposes
void MapMatrix_Perm::check_lows()
{
    for (unsigned i = 0; i < num_rows; i++) {
        if (low_by_row[i] != -1) {
            if (low_by_col[low_by_row[i]] != static_cast<int>(i))
                debug() << "===>>> ERROR: INCONSISTNECY IN LOW ARRAYS";
        }
    }
    for (unsigned j = 0; j < columns.size(); j++) {
        //find the lowest entry in column j
        int lowest = -1;
        if (columns[j] != NULL) {
            //consider the first node
            MapMatrixNode* current = columns[j];
            lowest = perm[current->get_row()];

            //consider all following nodes
            current = current->get_next();
            while (current != NULL) {
                if (static_cast<int>(perm[current->get_row()]) > lowest)
                    lowest = perm[current->get_row()];

                current = current->get_next();
            }
        }

        //does this match low_by_col[j]?
        if (lowest != low_by_col[j])
            debug() << "===>>> ERROR IN low_by_col[" << j << "]";
        else if (lowest != -1) {
            if (low_by_row[lowest] != static_cast<int>(j))
                debug() << "===>>> ERROR: INCONSISTNECY IN LOW ARRAYS";
        }
    }
}
*/

/********** implementation of class MapMatrix_RowPriority_Perm **********/

MapMatrix_RowPriority_Perm::MapMatrix_RowPriority_Perm(unsigned size)
    : MapMatrix_Base(size)
    , perm(size)
    , mrep(size)
{
    //initialize permutation vectors to the identity permutation
    for (unsigned i = 0; i < size; i++) {
        perm[i] = i;
        mrep[i] = i;
    }
}

//TODO: Is this necessary?
MapMatrix_RowPriority_Perm::~MapMatrix_RowPriority_Perm() = default;

//Default copy constructor is the right one.
/*
//copy constructor
MapMatrix_RowPriority_Perm::MapMatrix_RowPriority_Perm(const MapMatrix_RowPriority_Perm& other)
    : MapMatrix_Base(other.height())
    , perm(other.perm)
    , mrep(other.mrep)
{
    //copy all matrix entries
    for (unsigned j = 0; j < other.height(); j++) {
        MapMatrixNode* other_node = other.columns[j];
        if (other_node != NULL) {
            //create the first node in this column
            MapMatrixNode* cur_node = new MapMatrixNode(other_node->get_row());
            columns[j] = cur_node;

            //create all other nodes in this column
            other_node = other_node->get_next();
            while (other_node != NULL) {
                MapMatrixNode* new_node = new MapMatrixNode(other_node->get_row());
                cur_node->set_next(new_node);
                cur_node = new_node;
                other_node = other_node->get_next();
            }
        }
    }
}
*/

unsigned MapMatrix_RowPriority_Perm::width() const
{
    return MapMatrix_Base::height();
}

unsigned MapMatrix_RowPriority_Perm::height() const
{
    return MapMatrix_Base::width();
}

/*
void MapMatrix_RowPriority_Perm::set(unsigned i, unsigned j)
{
    MapMatrix_Base::set(mrep[j], i);
}
*/
 
//We should be able to get away with out this clear function
/*
void MapMatrix_RowPriority_Perm::clear(unsigned i, unsigned j)
{
    MapMatrix_Base::clear(mrep[j], i);
}
*/
 
bool MapMatrix_RowPriority_Perm::entry(unsigned i, unsigned j) const
{
    return MapMatrix_Base::entry(mrep[j], i);
}

//adds row j to row k; RESULT: row j is not changed, row k contains sum of rows j and k (with mod-2 arithmetic)
void MapMatrix_RowPriority_Perm::add_row(unsigned j, unsigned k)
{
    return MapMatrix_Base::add_to(j, k);
}

//transposes rows i and i+1
void MapMatrix_RowPriority_Perm::swap_rows(unsigned i)
{
    matrix.swap_columns(i);
}

//transposes columns j and j+1
void MapMatrix_RowPriority_Perm::swap_columns(unsigned j)
{
    //get original indexes of these columns
    unsigned a = mrep[j];
    unsigned b = mrep[j + 1];

    //swap perm[a] and perm[b]
    unsigned temp = perm[a];
    perm[a] = perm[b];
    perm[b] = temp;

    //swap mrep[i] and mrep[i+1]
    mrep[j] = b;
    mrep[j + 1] = a;
}

/*
//prints the matrix to debug(), for testing
//this function is identical to MapMatrix::print(), with rows and columns transposed
void MapMatrix_RowPriority_Perm::print()
{
    //handle empty matrix
    if (num_rows == 0 || columns.size() == 0) {
        debug() << "        (empty matrix:" << columns.size() << "rows by" << num_rows << "columns)";
        return;
    }

    //create a 2D array of booleans to temporarily store the matrix
    bool_array mx(columns.size(), num_rows);
    for (unsigned i = 0; i < columns.size(); i++)
        for (unsigned j = 0; j < num_rows; j++)
            mx.at(i, j) = false;

    //traverse the linked lists in order to fill the 2D array
    MapMatrixNode* current;
    for (unsigned j = 0; j < columns.size(); j++) {
        current = columns[j];
        while (current != NULL) {
            unsigned row = current->get_row();
            mx.at(j, perm[row]) = true;
            current = current->get_next();
        }
    }

    //print the matrix
    for (unsigned i = 0; i < num_rows; i++) {
        Debug qd = debug(true);
        qd << "        |";
        for (unsigned j = 0; j < columns.size(); j++) {
            if (mx.at(i, j))
                qd << " 1";
            else
                qd << " 0";
        }
        qd << " |";
    }
} //end print()
*/

/*
//prints the permutation vectors to debug() for testing
void MapMatrix_RowPriority_Perm::print_perm()
{
    Debug qd = debug();
    qd << " ==== Perm:";
    for (unsigned i = 0; i < perm.size(); i++)
        qd << perm[i];
    qd << "\n ==== Mrep:";
    for (unsigned i = 0; i < mrep.size(); i++)
        qd << mrep[i];
}
*/
