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
/*
 
 Authors: Matthew L. Wright (February 2014- ), 
          Michael Lesnick (Modifications 2017-2018)
 
 Classes: MapMatrix and related classes.
 
 Description: The classes introduces here store a column sparse matrix with Z/2Z 
 coefficients, and provide operations for persistence calculations. The original 
 version of this class was based on a custom linked list data structure for 
 columns.  This updated version does away with linked lists, and instead 
 represents matrices using a modified version of the class vector_heap from the 
 PHAT library, which stores columns as lazy heaps.  Here "lazy" means that 
 multiple copies of an element are allowed to live in the heap.  See the PHAT 
 paper for details.
 
 This file introduces four related classes:
 * MapMatrix_Base provides the basic structures and functionality; it is the 
   parent class and is not meant to be instantiated directly.
 * MapMatrix : MapMatrix_Base stores matrices in a column-sparse format, 
   designed for basic persistence calcuations, and computations of bigraded 
   betti numbers / presentations.
 * MapMatrix_Perm : MapMatrix adds functionality for row and column 
   permutations; it is designed for the reduced matrices of vineyard updates.
 * MapMatrix_RowPriority_Perm : MapMatrix_Base stores matrices in a row-sparse 
   format with row and column permutations; it is designed for the 
   upper-triangular matrices of vineyard updates.
*/

/*
 
 TODO [Mike]: Preliminary computational experiments indicate that using lazy 
 heaps makes RIVET significantly faster. However, the orginazation I have 
 introduced is probably not optimizal.  Matthew's version of the MapMatrix 
 hierarchy had 4 matrix classes.  This update still has those four, plus the 
 classes vector_heap_mod (a modification of PHAT's vector_heap class), and 
 vector_heap_perm : vector_heap_mod.  That seems to be a bit much, and the way 
 the code is written, it has been a pain to add additional functionality, 
 because vector_heap_mod and MapMatrix typically both need to be modified.  
 On top of this, minimizing a presention uses a matrix with sorted columns, and 
 it may be appropriate to have a special place for such matrices in the class 
 structure, especially since some different methods are used with these.
 
 The structure should be revisited.  Here are some specific ideas for cleaning 
 up the structure:
 
 -Get rid of the MapMatrix_Base class.  This seems to no longer be a heplful 
  abstraction, since vector_heap_mod now plays the role of the base class.
 
 -It may be wise to make the vector_heap_mod object public, in order to reduce 
  the number of wrapper classes.
 
 -Revisit the names of the functions here, in relation to the functions in 
  vector_heap_mod.
 
 -Perhaps the use of the PHAT integer typedef "index" vs. ordinary int should be 
  more systematic.
*/

#ifndef __MapMatrix_H__
#define __MapMatrix_H__

//#include "phat_mod/include/phat/boundary_matrix_mod.h"
#include "phat_mod/include/phat/representations/vector_heap_mod.h"
#include <ostream> //for testing
#include <vector>

class IndexMatrix;
class FIRep;

/*
Base class simply implements features common to all MapMatrices, whether
column-priority or row-priority. Written here using column-priority terminology,
but this class is meant to be inherited, not instantiated directly
*/

class MapMatrix_Base {
protected:
    MapMatrix_Base(unsigned rows, unsigned cols); //constructor to create matrix of specified size (all entries zero)
    MapMatrix_Base(unsigned size); //constructor to create a (square) identity matrix
    virtual ~MapMatrix_Base(); //destructor

    phat::vector_heap_mod matrix; //modified PHAT lazy heap matrix object
    unsigned num_rows; //number of rows in the matrix

    virtual unsigned width() const; //returns the number of columns in the matrix
    virtual unsigned height() const; //returns the number of rows in the matrix

    //WARNING: Current Implementation assumes the entry has not already been added.
    virtual void set(unsigned i, unsigned j); //sets (to 1) the entry in row i, column j

    virtual void add_to(unsigned j, unsigned k); //adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
};

//MapMatrix is a column-priority matrix designed for standard persistence calculations
class MapMatrix_Perm;
class MapMatrix : public MapMatrix_Base {
    friend class FIRep;
    friend class MapMatrix_Perm;

public:
    MapMatrix(unsigned rows, unsigned cols); //constructor to create matrix of specified size (all entries zero)
    MapMatrix(unsigned size); //constructor to create a (square) identity matrix

    MapMatrix(); // creates an empty MapMatrix

    MapMatrix(std::initializer_list<std::initializer_list<int>>);

    virtual unsigned width() const; //returns the number of columns in the matrix
    virtual unsigned height() const; //returns the number of rows in the matrix

    //friend std::ostream& operator<<(std::ostream&, const MapMatrix&);

    //requests that the columns vector have enough capacity for num_cols columns
    void reserve_cols(unsigned num_cols);

    //resize the matrix to the specified number of columns
    void resize(unsigned num_cols);

    //resize the matrix to the specified number of columns
    void resize(unsigned n_rows, unsigned n_cols);

    //WARNING: Current implementation assumes the entry has not already been added.
    virtual void set(unsigned i, unsigned j); //sets (to 1) the entry in row i, column j

    //returns the "low" index in the specified column, or -1 if the column is empty
    virtual int low(unsigned j) const;

    //same functionality as above, but only works correctly if column is finalized.
    int low_finalized(unsigned j) const;

    //same as the above, but removes the low.
    //Used for efficient implementation of standard reduction w/ lazy heaps.
    int remove_low(unsigned j);

    //Assuming column j is already heapified, adds l to the column and fixes heap.
    //Used for efficient implementation of standard reduction w/ lazy heaps.
    void push_index(unsigned j, unsigned l);

    bool col_is_empty(unsigned j) const; //returns true iff column j is empty

    void add_column(unsigned j, unsigned k); //adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)

    //TODO: Probably only used to compute Betti numbers, so perhaps should move with the other specialized functions for that
    //adds column j from MapMatrix other to column k of this matrix
    void add_column(const MapMatrix* other, unsigned j, unsigned k); //adds column j from MapMatrix* other to column k of this matrix

    //wraps the add_to_popped() function in vector_heap_mod. See that code for an explanation.
    void add_column_popped(unsigned j, unsigned k);

    //same as above, but column j now comes from another matrix.
    void add_column_popped(const MapMatrix& other, unsigned j, unsigned k);

    void prepare_col(unsigned j);

    void finalize(unsigned i);

    void print() const;

    void print_sparse() const;

    /*** For use in the new code to compute presentations ***/

    //copies with index j from other to the back of this matrix
    void append_col(const MapMatrix& other, unsigned j);

    //Move column with index source to index target, zeroing out this column source in the process.
    void move_col(unsigned source, unsigned target);

    //Move the ith column of other to jth this matrix, zeroing out ith column of other in the process.
    void move_col(MapMatrix& other, unsigned i, unsigned j);

    /********* Methods used to minimize a presentation *********/

    void sort_col(int i);

    // reindex column col using the indices given in new_row_indices.
    void reindex_column(unsigned col, const std::vector<int>& new_row_indices);

    /*** Next functions assume that column(s) in question are sorted ***/

    //same as add_column above, but requires columns to be sorted vectors.
    void add_column_sorted(unsigned j, unsigned k);

    //returns true if entry (i,j) is 1, false otherwise
    bool entry_sorted(unsigned i, unsigned j) const;

    //returns entry with largest index, if the column is non empty.  Returns -1 otherwise.
    int low_sorted(unsigned i) const;

    /********* Tehcnical functions used in Matthew's old Betti code. *********/
    //TODO: These probably can be deleted if we phase out the old Betti algorithm

    //TODO: Make the int arguments in the next two functions unsigned?

    //copies NONZERO columns with indexes in [first, last] from other, appending them to this matrix to the right of all existing columns
    //  all row indexes in copied columns are increased by offset
    void copy_cols_from(const MapMatrix* other, int first, int last, unsigned offset);

    //copies columns with indexes in [first, last] from other, inserting them in this matrix with the same column indexes
    void copy_cols_same_indexes(const MapMatrix* other, int first, int last);

    //removes zero columns from this matrix
    //  ind_old gives grades of columns before zero columns are removed; new grade info stored in ind_new
    void remove_zero_cols(const IndexMatrix& ind_old, IndexMatrix& ind_new);
};

//MapMatrix with row/column permutations and low array, designed for "vineyard updates."
class MapMatrix_RowPriority_Perm; //forward declaration
class MapMatrix_Perm {
public:
    //Constructors

    //Special constructor to create the initial low matrix used in the
    //computation of barcode templates.  Permutes and trims columns as described
    //in section 6 of the RIVET paper.
    MapMatrix_Perm(const MapMatrix& mat, const std::vector<int>& coface_order, unsigned num_cofaces);

    //Special constructor to create the initial high matrix used in the
    //computation of barcode templates.  Permutes and trims rows and columns as
    //described in section 6 of the RIVET paper.
    MapMatrix_Perm(const MapMatrix& mat, const std::vector<int>& face_order, unsigned num_faces, const std::vector<int>& coface_order, const unsigned num_cofaces);

    unsigned width() const; //returns the number of columns in the matrix
    unsigned height() const; //returns the number of rows in the matrix

    bool entry(unsigned i, unsigned j) const; //returns true if entry (i,j) is 1, false otherwise

    //reduces this matrix, fills the low array, and returns the corresponding upper-triangular matrix for the RU-decomposition
    //  NOTE: only to be called before any rows are swapped!
    MapMatrix_RowPriority_Perm* decompose_RU();

    int low(unsigned j) const; //returns the "low" index in the specified column, or -1 if the column is empty
    int find_low(unsigned l) const; //returns the index of the column with low l, or -1 if there is no such column

    bool col_is_empty(unsigned j) const; //returns true iff column j is empty

    void add_column(unsigned j, unsigned k); //adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)

    void swap_rows(unsigned i, bool update_lows); //transposes rows i and i+1, optionally updates low array
    void swap_columns(unsigned j, bool update_lows); //transposes columns j and j+1, optionally updates low array

    //clears the matrix, then rebuilds it from reference with columns permuted according to col_order
    //NOTE: This functoon and the next do not remove columns or rows.
    //TODO: Add lazy versions of these functions which don't redo the whole computation
    void rebuild(MapMatrix_Perm* reference, const std::vector<unsigned>& col_order);

    //clears the matrix, then rebuilds it from reference with columns permuted
    //according to col_order and rows permuted according to row_order
    void rebuild(MapMatrix_Perm* reference, const std::vector<unsigned>& col_order, const std::vector<unsigned>& row_order);

    //TODO: Add a finalize method here?

    void print(); //prints the matrix to standard output (for testing)

private:
    phat::vector_heap_perm matrix; // modified version of phat's vector_heap class which supports implicit ordering of rows.

    std::vector<int> low_by_row; //stores index of column with each low number, or -1 if no such column exists -- NOTE: only accurate after decompose_RU() is called
    std::vector<int> low_by_col; //stores the low number for each column, or -1 if the column is empty -- NOTE: only accurate after decompose_RU() is called
};

//MapMatrix stored in row-priority format, with row/column permutations,
//designed for upper-triangular matrices in vineyard updates.
//NOTE: In this new version of the code, the MapMatrix_RowPriority_Perm class is just a simple interface for the vector_heap_perm class.
class MapMatrix_RowPriority_Perm {
public:
    MapMatrix_RowPriority_Perm(unsigned size); //constructs the identity matrix of specified size
    //MapMatrix_RowPriority_Perm(const MapMatrix_RowPriority_Perm& other); //copy constructor
    ~MapMatrix_RowPriority_Perm();

    unsigned width() const; //returns the number of columns in the matrix
    unsigned height() const; //returns the number of rows in the matrix

    bool entry(unsigned i, unsigned j) const; //returns true if entry (i,j) is 1, false otherwise

    void add_row(unsigned j, unsigned k); //adds row j to row k; RESULT: row j is not changed, row k contains sum of rows j and k (with mod-2 arithmetic)

    void swap_rows(unsigned i); //transposes rows i and i+1
    void swap_columns(unsigned j); //transposes columns j and j+1

    ///FOR TESTING ONLY
    //void print(); //prints the matrix to qDebug() for testing
    //void print_perm(); //prints the permutation vectors to qDebug() for testing

private:
    phat::vector_heap_perm matrix; // modified version of phat's vector_heap class which supports implicit ordering of rows.
};

#endif // __MapMatrix_H__
