//Modified for RIVET by Matthew Wright and Michael Lesnick.

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

//File defines vector_heap_mod, a modification of PHAT's vector_heap class, a column sparse representation of a matrix with binary coefficients, using lazy heaps for columns.
//The file also defines child class vector_heap_perm, which supports row permutations, represented implicitly via a permutation vector and its inverse.

namespace phat {
    class vector_heap_mod {
    protected:
        
        std::vector< column > matrix;

        std::vector< index > inserts_since_last_prune;
    
        mutable thread_local_storage< column > temp_column_buffer;
    
    private:
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
            
            //Remark: Interesting that this reverse step is here in the original PHAT code.  Is this more efficient than just calling make_heap?  -Mike
            //TODO: Maybe experiment with deleting std::reverse?
            std::reverse( col.begin( ), col.end( ) );
            std::make_heap( col.begin( ), col.end( )  );
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
    
        index _pop_max_index( index idx )
        {
            return _pop_max_index( matrix[ idx ] );
        }

    public:
        
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
        
        //Added for use in RIVET
        void _reserve_cols( index nr_of_columns )
        {
            matrix.reserve( nr_of_columns );
        }
        
        //Not needed by RIVET; effectively replaced by _get_col_iter
        
        // replaces(!) content of 'col' with boundary of given index
        /*
        void _get_col( index idx, column& col ) const
        {
            temp_column_buffer( ) = matrix[ idx ];
            
            index max_index = _pop_max_index( temp_column_buffer(),  );
            while( max_index != -1 ) {
                col.push_back( max_index );
                max_index = _pop_max_index( temp_column_buffer( ) );
            }
            std::reverse( col.begin( ), col.end( ) );
        }
        */
        
        //Added for use in RIVET
        std::vector< column >::const_iterator _get_col_iter( index idx) const {
            return matrix.begin()+idx;
        }
    
        //Sets column idx of the matrix equal to col
        void _set_col( index idx, const column& col )
        {
            matrix[ idx ] = col;
            std::make_heap( matrix[ idx ].begin( ), matrix[ idx ].end( ) );
        }
    
        //TODO: I don't think we need this.  Delete.
        /*
        //sets column, where indices are the image of those appearing in col under the permutation map row_order.  Required by RIVET for resets during vineyard updates.  Similar to the above, but assumes that all indices in row_order are non-negative (whereas we cannot assume that in the version of _set_col above, because we may see an index -1.
        void _set_col( index idx, const column& col)
        {
            matrix[idx].clear();
            matrix[idx].shrink_to_fit();
            matrix[idx].reserve(col.size());
            for (auto it=col.begin(); it != col.end(); it++)
            {
                matrix[idx].push_back(row_order[*it]);
            }
            std::make_heap( matrix[ idx ].begin( ), matrix[ idx ].end( ), permutation_comparison );
        }
        */
         
        // _set_entry is needed by RIVET.
        // Adds an entry to a column.  Does not do any sorting or heapification.
        // NOTE: Assumes the entry has not yet been added.
        void _set_entry(index row,index col)
        {
            matrix[col].push_back(row);
        }
        
        // true iff boundary of given idx is empty
        bool _is_empty( index idx ) const
        {
            return _get_max_index( idx ) == -1;
        }
        
        /*
        //TODO:Delete?  Only need this in ROW and COLUMN permuted settings.
        //Modification of PHAT v1.5. for handling vineyards.
        // true iff index (row, col) is non-empty

        bool _is_in_matrix( index row, index col ) const {
            return std::count(matrix[col].begin(), matrix[col].end(), row) % 2;
        }
        */

        // append copy of column to back of matrix, while clearing the original column
        void _append_col_and clear(column& col)
        {
            matrix.push_back(column());
            matrix[size()-1].swap(col);
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
        
        // largest row index of given column idx.
        // NOTE: Only works correctly when this column has no repeat entries, i.e. if nothing has been added to the column since it was itialized or finalized.
        // But in this case this is a bit faster than get_max_index.
        index _get_max_index_finalized( index idx ) const
        {
            return matrix[ idx ].front();
        }
        
        // RIVET modification; part of an optimization of the standard reduction when columns are lazy heaps.
        void _push_index(index col_idx,index entry)
        {
            column& col = matrix[ col_idx ];
            col.push_back( entry );
            std::push_heap( col.begin( ), col.end( ) );
        }
    
        // removes the maximal index of a column
        index _remove_max( index idx )
        {
            return _pop_max_index( idx );
        }

        // clears given column
        void _clear( index idx )
        {
            column().swap(matrix[ idx ]);
        }

        //TODO: Delete Don't need, I think.
        // syncronizes all data structures (essential for openmp stuff)
        /*
        void _sync( ) {}
        */
        
        // adds column 'source' to column 'target'
        //TODO: If the two vectors are similar length, it is probably more efficient to just concatenate and then re-heapify.
        //PHAT does not exploit this optimization, but when we do have to add vectors, this is probably usually the case.
        //Also below.
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
        
        //TODO: Can I get rid of this?  Perhaps we are going to only use the vector_heap_perm version...
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
        
        
        // _heapify_col is needed by RIVET
        void _heapify_col(index idx)
        {
            std::make_heap(matrix[idx].begin( ),matrix[idx].end( ) );
        }
        
        //TODO: Remove and only have a version for child classes?
        /*
        //This is needed by RIVET.
        void _swap_columns(index idx) {
            matrix[idx].swap(matrix[idx+1]);
        }
        */
        
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
    
/*********************** vector_heap_perm ***********************/
    
    class vector_heap_perm : public vector_heap_mod {
        
    protected:
        
        //Stores the permuation of the row indices.
        std::vector< unsigned > perm;
        
        //the inverse permutation
        std::vector< unsigned > mrep;
    
        //we redefine some of the functions above to use the permuted order.
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
            std::make_heap( col.begin( ), col.end( ), [this](const index left, const index right) { return perm[left]<perm[right]; } );
            inserts_since_last_prune[ idx ] = 0;
        }
        
        index _pop_max_index( column& col ) const
        {
            if( col.empty( ) )
                return -1;
            else {
                index max_element = col.front( );
                std::pop_heap( col.begin( ), col.end( ), [this](const index left, const index right) { return perm[left]<perm[right]; } );
                col.pop_back( );
                while( !col.empty( ) && col.front( ) == max_element ) {
                    std::pop_heap( col.begin( ), col.end( ), [this](const index left, const index right) { return perm[left]<perm[right]; } );
                    col.pop_back( );
                    if( col.empty( ) )
                        return -1;
                    else {
                        max_element = col.front( );
                        std::pop_heap( col.begin( ), col.end( ), [this](const index left, const index right) { return perm[left]<perm[right]; } );
                        col.pop_back( );
                    }
                }
                return max_element;
            }
        }
        
        index _pop_max_index( index idx )
        {
            return _pop_max_index( matrix[ idx ] );
        }

    
    public:
        
        //constructor
        //initializes empty rows x cols matrix, with the identity permutation on rows.
        vector_heap_perm(unsigned rows, unsigned cols)
        : perm(rows)
        , mrep(rows)
        {
            _set_num_cols(cols);
            //Initialize permutations to identity
            for (unsigned i = 0; i < rows; i++)
            {
                perm[i] = i;
                mrep[i] = i;
            }
        }
        // end of def. of constructor.
        
        //constructor
        //initializes a size x size identity matrix, with the identity permutation on rows.
        vector_heap_perm(unsigned size)
        : perm(size)
        , mrep(size)
        {
            _set_num_cols(size);
            //Initialize permutations to identity
            for (unsigned i = 0; i < size; i++)
            {
                _set_entry(i,i);
                perm[i] = i;
                mrep[i] = i;
            }
            
        }
        // end of def. of constructor.
        
        
        void _set_perm(const std::vector<unsigned>& row_order)
        {
            perm=row_order;
            //assumes that mrep is already sized properly.
            for (unsigned i = 0; i < row_order.size(); i++)
            {
                mrep[perm[i]]=i;
            }
        }
        
        // overall number of cells in boundary_matrix
        index _get_num_rows( ) const
        {
            return (index) perm.size( );
        }
        
        //Added for use in RIVET
        void _reserve_cols( index nr_of_columns )
        {
            matrix.reserve( nr_of_columns );
        }
        
        void _set_col( index idx, const column& col )
        {
            matrix[ idx ] = col;
            std::make_heap( matrix[ idx ].begin( ), matrix[ idx ].end( ),[this](const index left, const index right) { return perm[left]<perm[right];} );
        }
        
        //sets column, where indices are the image of those appearing in col under the permutation map row_order.  Required by RIVET for the initialization of the barcode template computations.
        //NOTE: This actually explicitly stores the permuted indices.
        void _set_col( index idx, const column& col, const std::vector<int>& row_perm_order)
        {
            matrix[idx].clear();
            matrix[idx].shrink_to_fit();
            matrix[idx].reserve(col.size());
            for (auto it=col.begin(); it != col.end(); it++)
            {
                if(row_perm_order[*it]>=0)
                    matrix[idx].push_back(row_perm_order[*it]);
            }
            std::make_heap( matrix[ idx ].begin( ), matrix[ idx ].end( ) );
        }
        
        // TODO: Do we need this function in this child class?  Commenting for now.
        // Adds an entry to a column.  Does not do any sorting or heapification.
        // NOTE: Assumes the entry has not yet been added.
        /*
        void _set_entry(index row,index col)
        {
            matrix[col].push_back(mrep[row]);
        }
        */
         
        //Used for handling vineyards.
        // returns true iff index (row, col) is non-empty
        bool _is_in_matrix( index row, index col ) const {
            return std::count(matrix[col].begin(), matrix[col].end(), mrep[row] ) % 2;
        }
        
        // largest row index of given column idx
        index _get_max_index( index idx ) const
        {
            column& col = const_cast< column& >( matrix[ idx ] );
            index max_element = _pop_max_index( col );
            col.push_back( max_element );
            std::push_heap( col.begin( ), col.end( ), [this](const index left, const index right) { return perm[left]<perm[right]; });
            if ((int) max_element == -1)
                return -1;
            return perm[max_element];
        }
        
        // largest row index of given column idx.
        // NOTE: Only works correctly when this column has no repeat entries, i.e. if nothing has been added to the column since it was itialized or finalized.
        // But in this case this is a bit faster than get_max_index.
        index _get_max_index_finalized( index idx ) const
        {
            return perm[matrix[ idx ].front()];
        }
        
        // pops the maximum index of a column
        index _remove_max( index idx )
        {
            index raw_index=_pop_max_index( idx );
            if ((int) raw_index == -1)
                return -1;
            return perm[raw_index];
        }
        
        void _push_index(index col_idx,index entry)
        {
            matrix[ col_idx ].push_back(mrep[entry]);
            std::push_heap( matrix[ col_idx ].begin( ), matrix[ col_idx ].end( ), [this](const index left, const index right) { return perm[left]<perm[right]; } );
        }
        
        // adds column 'source' to column 'target'
        // TODO: As above, add and then heapify if the column is large, instead of maintaining the heap entry by entry?
        void _add_to( index source, index target )
        {
            for( index idx = 0; idx < (index)matrix[ source ].size( ); idx++ ) {
                matrix[ target ].push_back( matrix[ source ][ idx ] );
                std::push_heap( matrix[ target ].begin(), matrix[ target ].end(), [this](const index left, const index right) { return perm[left]<perm[right]; } );
            }
            inserts_since_last_prune[ target ] += matrix[ source ].size();
            
            if( 2 * inserts_since_last_prune[ target ] > ( index )matrix[ target ].size() )
                _prune( target );
        }
        
        //TODO: I think this should not be needed by vector_heap_perm.
        /*
        // this version of _add_to() is needed by RIVET
        // adds column 'source' from 'other' matrix to column 'target' in this matrix
        void _add_to(const  vector_heap_mod& other, index source, index target ) {
            for( index idx = 0; idx < (index) other.matrix[ source ].size( ); idx++ ) {
                matrix[ target ].push_back( other.matrix[ source ][ idx ] );
                std::push_heap( matrix[ target ].begin(), matrix[ target ].end(), permutation_comparison );
            }
            inserts_since_last_prune[ target ] += other.matrix[ source ].size();
            
            if( 2 * inserts_since_last_prune[ target ] > ( index )matrix[ target ].size() )
                _prune( target );
        }
        */
        
        // this technical method is used by RIVET in an optimization of the standard reduction.  It assumes that
        //1)the column at index target has already had its pivot p popped off
        //2)the pivot of the column of at index source is also p
        //3)the column target is finalized, i.e., each entry appears at most once.
        //TODO: As above, if the source column is nearly as large as the target column, then this can be faster.
        void _add_to_popped(index source, index target ) {
            //in the implementation of the heap used here, the pivot is stored at the 0th index, so we start the addition from index 1.
            for( index idx = 1; idx < (index) matrix[ source ].size( ); idx++ ) {
                matrix[ target ].push_back( matrix[ source ][ idx ] );
                std::push_heap( matrix[ target ].begin(), matrix[ target ].end(), [this](const index left, const index right) { return perm[left]<perm[right]; } );
            }
            inserts_since_last_prune[ target ] += matrix[ source ].size()-1;
            
            if( 2 * inserts_since_last_prune[ target ] > ( index )matrix[ target ].size() )
                _prune( target );
        }
        
        //TODO: Delete?  I don't think this is necessary.
        /*
        void _heapify_col(index idx)
        {
            std::make_heap(matrix[idx].begin( ),matrix[idx].end( ), [this](const index left, const index right) { return perm[left]<perm[right]; } );
        }
        */
        
        //swap column a with column b
        void _swap_columns(index a, index b) {
            matrix[a].swap(matrix[b]);
        }
        
        //(implicitly) swap row idx with row idx+1
        //NOTE: This code has been moved from MapMatrix_Perm::swap_rows.
        void _swap_rows(index idx) {
            //get original row indexes of these rows
            unsigned a = mrep[idx];
            unsigned b = mrep[idx + 1];
            
            //swap entries in permutation and inverse permutation arrays
            perm[a] = idx+1;
            perm[b] = idx;
            
            mrep[idx] = b;
            mrep[idx + 1] = a;
        }
        
        // finalizes column idx.
        // I redefine this, relative to the base class, since _prune is now different.
        // TODO: Is it good style to redefine this, rather than making _finalize virtual in the base class?  My intent is to choose at compile time which version of finalize to use.
        void _finalize( index idx ) {
            _prune( idx );
        }
        
        // print the matrix in dense format.
        void _print() {
            unsigned num_rows=perm.size();
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
                    //The only difference between the print function for the base class and this one is the following line.
                    dense_mat[i][perm[matrix[i][j]]]=1;
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
        
        // print the non-zero entries of a column of the matrix.
        // for debugging
        void _print_sparse(index idx) {
            auto col=std::vector<index>();
            //_finalize(idx);
            for (unsigned j=0; j< matrix[idx].size(); j++)
            {
                col.push_back(matrix[idx][j]);
            }
            std::sort(col.begin(),col.end(),[this](const index left, const index right) { return perm[left]<perm[right]; });
            for (unsigned j=0; j < col.size(); j++)
                std::cout << perm[col[j]] << " ";
            std::cout << std::endl;
            for (unsigned j=0; j < col.size(); j++)
                std::cout << col[j] << " ";
            std::cout << std::endl;
        }
    
    };
}
