//Modified for RIVET by Matthew Wright and Michael Lesnick

/*  Copyright 2013 IST Austria
Contributed by: Jan Reininghaus

This file is part of PHAT.

PHAT is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

PHAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with PHAT.  If not, see <http://www.gnu.org/licenses/>. */

#pragma once

#include "../helpers/misc.h"

namespace phat {
    class vector_heap_mod {

    protected:
        
        //RIVET doesn't require this.
        //std::vector< dimension > dims;
        
        std::vector< column > matrix;

        std::vector< index > inserts_since_last_prune;
    
        mutable thread_local_storage< column > temp_column_buffer;

    protected:
        void _prune( index idx )
        {
            column& col = matrix[ idx ];
            column& temp_col = temp_column_buffer();
            temp_col.clear();
            index max_index = _pop_max_index( col );
            while( max_index != -1 ) {
                temp_col.push_back( max_index );
                max_index = _pop_max_index( col );
            }
            col = temp_col;
            std::reverse( col.begin( ), col.end( ) );
            std::make_heap( col.begin( ), col.end( ) );
            inserts_since_last_prune[ idx ] = 0;
        }

        index _pop_max_index( column& col ) const
        {
            if( col.empty( ) )
                return -1;
            else {
                index max_element = col.front( );
                std::pop_heap( col.begin( ), col.end( ) );
                col.pop_back( );
                while( !col.empty( ) && col.front( ) == max_element ) {
                    std::pop_heap( col.begin( ), col.end( ) );
                    col.pop_back( );
                    if( col.empty( ) )
                        return -1;
                    else {
                        max_element = col.front( );
                        std::pop_heap( col.begin( ), col.end( ) );
                        col.pop_back( );
                    }
                }
                return max_element;
            }
        }
        
        

    public:
        
        //NOTE: Moved to a public function in this modification for RIVET.
        index _pop_max_index( index idx )
        {
            return _pop_max_index( matrix[ idx ] );
        }
        
        // overall number of cells in boundary_matrix
        index _get_num_cols( ) const
        {
            return (index)matrix.size( );
        }
        void _set_num_cols( index nr_of_columns )
        {
            //dims.resize( nr_of_columns );
            matrix.resize( nr_of_columns );
            inserts_since_last_prune.assign( nr_of_columns, 0 );
        }
        
        void _reserve_cols( index nr_of_columns )
        {
            matrix.reserve( nr_of_columns );
        }

        //RIVET doesn't require this
        /*
        // dimension of given index
        dimension _get_dim( index idx ) const
        {
            return dims[ idx ];
        }
        void _set_dim( index idx, dimension dim )
        {
            dims[ idx ] = dim;
        }
        */

        // replaces(!) content of 'col' with boundary of given index
        void _get_col( index idx, column& col ) const
        {
            temp_column_buffer( ) = matrix[ idx ];
            
            index max_index = _pop_max_index( temp_column_buffer() );
            while( max_index != -1 ) {
                col.push_back( max_index );
                max_index = _pop_max_index( temp_column_buffer( ) );
            }
            std::reverse( col.begin( ), col.end( ) );
        }
        
        std::vector< column >::const_iterator _get_col_iter( index idx) const {
            return matrix.begin()+idx;
        }
        
        void _set_col( index idx, const column& col )
        {
            matrix[ idx ] = col;
            std::make_heap( matrix[ idx ].begin( ), matrix[ idx ].end( ) );
        }
        
        //TODO: Combine the two functions here?  That's probably a little cleaner, but I am encountering
        //some type conversion troubles; some arrays elsewhere that are more naturally valued in unsigned ints would have to be ints. -Mike
        
        //sets column, where indices are the image of those appearing in col under the permutation map row_order.  Required by RIVET for the initialization of the vineyard update process
        //TODO:This is probably temporary.  Replace by code which makes the indices the exactly those appearing in col, but takes the heap order to be determined by row order.  This will allow for lazy resets.
        void _set_col( index idx, const column& col, const std::vector<int>& row_order)
        {
            matrix[idx].clear();
            matrix[idx].shrink_to_fit();
            matrix[idx].reserve(col.size());
            for (auto it=col.begin(); it != col.end(); it++)
            {
                if(row_order[*it]>=0)
                    matrix[idx].push_back(row_order[*it]);
            }
            std::make_heap( matrix[ idx ].begin( ), matrix[ idx ].end( ) );
        }
        
        //sets column, where indices are the image of those appearing in col under the permutation map row_order.  Required by RIVET for resets during vineyard updates.  Similar to the above, but assumes that all indices in row_order are non-negative (whereas we cannot assume that in the version of _set_col above, because we may see an index -1.
        void _set_col( index idx, const column& col, const std::vector<unsigned>& row_order)
        {
            matrix[idx].clear();
            matrix[idx].shrink_to_fit();
            matrix[idx].reserve(col.size());
            for (auto it=col.begin(); it != col.end(); it++)
            {
                matrix[idx].push_back(row_order[*it]);
            }
            std::make_heap( matrix[ idx ].begin( ), matrix[ idx ].end( ) );
        }
        
        // _set_entry is needed by RIVET.
        // Adds an entry to a column.  Does not do any sorting or heapification.
        void _set_entry(index row,index col)
        {
            matrix[col].push_back(row);
        }
        
        // true iff boundary of given idx is empty
        bool _is_empty( index idx ) const
        {
            return _get_max_index( idx ) == -1;
        }
        
        //Modification of PHAT v1.5. for handling vineyards.
        // true iff index (row, col) is non-empty
        bool _is_in_matrix( index row, index col ) const {
            return std::count(matrix[col].begin(), matrix[col].end(), row) % 2;
        }

        // largest row index of given column idx (new name for lowestOne())
        index _get_max_index( index idx ) const
        {
            column& col = const_cast< column& >( matrix[ idx ] );
            index max_element = _pop_max_index( col );
            col.push_back( max_element );
            std::push_heap( col.begin( ), col.end( ) );
            return max_element;
        }
        
        // RIVET modification; part of an optimization of the standard reduction for heaps.
        void _push_max_index(index col_idx,index entry)
        {
            column& col = matrix[ col_idx ];
            col.push_back( entry );
            std::push_heap( col.begin( ), col.end( ) );
        }
    

        // removes the maximal index of a column
        void _remove_max( index idx )
        {
            _pop_max_index( idx );
        }

        // clears given column
        void _clear( index idx )
        {
            column().swap(matrix[ idx ]);
        }

        // syncronizes all data structures (essential for openmp stuff)
        void _sync( ) {}

        // adds column 'source' to column 'target'
        void _add_to( index source, index target )
        {              
            for( index idx = 0; idx < (index)matrix[ source ].size( ); idx++ ) {
                matrix[ target ].push_back( matrix[ source ][ idx ] );
                std::push_heap( matrix[ target ].begin(), matrix[ target ].end() );
            }
            inserts_since_last_prune[ target ] += matrix[ source ].size();

            if( 2 * inserts_since_last_prune[ target ] > ( index )matrix[ target ].size() )
                _prune( target );
        }
        
        // this version of _add_to() is needed by RIVET
        // adds column 'source' from 'other' matrix to column 'target' in this matrix
        void _add_to(const  vector_heap_mod& other, index source, index target ) {
            for( index idx = 0; idx < (index) other.matrix[ source ].size( ); idx++ ) {
                matrix[ target ].push_back( other.matrix[ source ][ idx ] );
                std::push_heap( matrix[ target ].begin(), matrix[ target ].end() );
            }
            inserts_since_last_prune[ target ] += other.matrix[ source ].size();
            
            if( 2 * inserts_since_last_prune[ target ] > ( index )matrix[ target ].size() )
                _prune( target );
        }
        
        // this technical method is used by RIVET in an optimization of the standard reduction.  It assumes that
        //1)the column at index target has already had its pivot p popped off
        //2)the pivot of the column of at index source is also p
        //3)p is finalized, i.e., each entry appears at most once.
        void _add_to_popped(index source, index target ) {
            //in the implementation of the heap used here, the pivot is stored at the 0th index, so we start the addition from index 1.
            for( index idx = 1; idx < (index) matrix[ source ].size( ); idx++ ) {
                matrix[ target ].push_back( matrix[ source ][ idx ] );
                std::push_heap( matrix[ target ].begin(), matrix[ target ].end() );
            }
            inserts_since_last_prune[ target ] += matrix[ source ].size()-1;
            
            if( 2 * inserts_since_last_prune[ target ] > ( index )matrix[ target ].size() )
                _prune( target );
        }
        
        
        // prepare_col is needed by RIVET
        void _prepare_col(index idx)
        {
            std::make_heap(matrix[idx].begin( ),matrix[idx].end( ) );
        }
        
        
        //This is needed by RIVET.
        void _swap_columns(index idx) {
            matrix[idx].swap(matrix[idx+1]);
        }
        
        
        // finalizes given column
        void _finalize( index idx ) {
            _prune( idx );
        }

        // print the matrix.  since a PHAT matrix doesn't know the number of rows, this has to be passed as an argument.
        void _print( index num_rows ) {
            //Print matrix dimensions
            std::cout << num_rows << " x " << matrix.size() << " matrix:" << std::endl;
            //First we convert to a dense matrix
            //Step 1: Build a dense matrix of the appropriate size
            std::vector<std::vector<index>> dense_mat= std::vector<std::vector<index>>(matrix.size());
            for (unsigned i=0; i < dense_mat.size(); i++)
            {
                //set column of to vector of num_rows zeros
                dense_mat[i].resize(num_rows);
            }
            //Step 2: Set the entries of the matrix
            for (unsigned i=0; i < dense_mat.size(); i++)
            {
                _finalize(i);
                for (unsigned j=0; j< matrix[i].size(); j++)
                {
                    dense_mat[i][matrix[i][j]]=1;
                }
            }
            //Step 3: Print
            for (unsigned i=0; i != num_rows; i++)
            {
                for (unsigned j=0; j != dense_mat.size(); j++)
                    std::cout << dense_mat[j][i] << " ";
                std::cout << std::endl;
            }
            
        }
    };
}
