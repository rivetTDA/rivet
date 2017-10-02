/**
 * \class	SparseMatrix
 * \brief	Child of PHAT's boundry_matrix Class.  Adds additional functionality needed by RIVET.
 * \author	Matthew Wright and Michael Lesnick
 * \date	July 2016, Oct. 2017.
 */

//This class is adapted from Matthew's extension of phat::boundary_matrix in July 2016.  In order to minimize confusion, it seems best to keep PHAT and RIVET separate to the fullest extent possible, so we have transfered the additional functionalilty of the extension to a child class of boundary_matrix.

#pragma once

/********** TESTING PHAT MATRIX STRUCTURES **********/

#include "phat/boundary_matrix.h"
#include "helpers/misc.h"
#include "phat/representations/vector_heap.h"
//#include "phat/representations/vector_list.h"
//#include "phat/representations/vector_set.h"
//#include "phat/representations/vector_vector.h"
//#include "phat/representations/bit_tree_pivot_column.h"


template< class Representation = phat::vector_heap >
class SparseMatrix : public phat::boundary_matrix< phat::vector_heap >
{
    
protected:
    index matrix_height;
    
public:
    // height
    unsigned height() const { return (unsigned) matrix_height; }
    
    // width
    unsigned width() const { return (unsigned) rep._get_num_cols(); }
    
    // low
    int low( unsigned i ) const { return (int) rep._get_max_index( (index) i ); }
    
    // add_column() for RIVET compatibility
    void add_column(unsigned source, unsigned target ) { rep._add_to( (index) source, (index) target ); }

    // operators / constructors
public:
    SparseMatrix() : boundary_matrix() {}
    
    //constructor for a matrix of a certain size
    sparse_matrix(unsigned rows, unsigned cols) : boundary_matrix(), matrix_height( (index)rows )
    {
        set_num_cols( (index)cols );
    }
    
    //constructor for an identity matrix of a certain size
    
    SparseMatrix (unsigned size) : boundary_matrix(), matrix_height( (index) size )
    {
        set_num_cols( (index) size );
        std::vector<index> temp_col;
        for(index i = 0; i < matrix_height; i++) {
            temp_col.push_back(i);
            set_col(i, temp_col);
            temp_col.clear();
        }
    }
    
    //output for testing RIVET
    template< typename index_type >
    void save_dense( std::vector< std::vector< index_type > >& output_matrix) {
        const index nr_of_columns = get_num_cols();
        output_matrix.resize( nr_of_columns );
        column temp_col;
        for( index cur_col = 0; cur_col <  nr_of_columns; cur_col++ ) {
            get_col( cur_col, temp_col );
            output_matrix[ cur_col ].clear();
            output_matrix[ cur_col ].resize( matrix_height, 0 );
            for( index cur_row = 0; cur_row <  temp_col.size(); cur_row++ )
                output_matrix[ cur_col ][ temp_col[cur_row] ] = 1;
        }
    }
    
};



