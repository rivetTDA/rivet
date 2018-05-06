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

 Author: Michael Lesnick (2017-2018).

 Class: Presentation

 Description: Stores a presentation of a 2-D persistence module.
 Main constructor takes an FIRep as input; destroys the FIRep as a side effect.
 Also has a method for minimizing a presentation, using only column-operations.
 Whereas constructing the presentation uses lazy heaps,
 minimization requires to sort the entries of the columns and
 perform binary searches for specific entries.
 
 This class also has a member hom_dims, which stores the Hilbert function of the
 presentation.
 
 WARNING/TODO: In the current implementation, hom_dims is stored as a
 boost::multi_array.  The copy/assignment operators for these objects do not 
 behave as you might expect; namely, the size of the matrix has to be specified
 manually before copy/assignment to get the expected behavior.  As a result, 
 the Presentation class also currently does not have the desired copy/assignment
 behavior.  The copy assignment operators are currently the defaults.  
 (See computation.cpp for an example of how to deal with this.)
 Of course, one coud simply specify the correct assignment and copy behavior,
 for the Presentiton class, and this should be easy.  But the right solution 
 seems to be to get rid of unsigned matrices in the code, at least in most places.  
 (Anywhere the unsigned_matrix typedef is used.) However, this edit involves 
 tinkering with Bryn's serialization code, so requires some care.  -Mike
*/


/**
 * \class	Presentation
 * \brief
 * \author	Michael Lesnick
 * \date	9/28/17
 */

#ifndef __Presentation_H__
#define __Presentation_H__

#include "index_matrix.h"
#include "map_matrix.h"

#include <string>
#include <interface/progress.h>

#include <boost/multi_array.hpp>
typedef boost::multi_array<unsigned, 2> unsigned_matrix;

//forward declarations
class vector_heap_mod;
class IndexMatrix;
class FIRep;
class BigradedMatrix;
class BigradedMatrixLex;


class Presentation {

public:
    
    MapMatrix mat;
    IndexMatrix col_ind;
    IndexMatrix row_ind;
    
    /* 
    Stores the hilbert function.
    It is admittedly an idiosyncratic design to have a presentation object
    store the Hilbert function by default, but for now, this is expedient.
    
    NOTE: To compute hom_dims (i.e., Hibert function), we will first store
    the pointwise ranks of the high map in this matrix, and then subtract
    this off of the pointwise nullities. 
    */
    unsigned_matrix hom_dims;

    //Constructor: Builds an empty presentation.
    Presentation();
    
    //Constructor: Builds a presentation from an FI-Rep.  Also computes Hilbert
    //function along the way.
    Presentation(FIRep& fir, Progress& progress, int verbosity);
    
    /* 
    Throws an exception if !is_kernel_minimal.  Minimizes presentation using
    only column operations.  This requires looking at column entries which are
    not necessarily pivots, so this code sorts each column first, and then finds 
    the entries using binary search.  Columns are added in a way that maintains 
    the order.
     
    When we call minimize(), we implcitly are removing rows from the matrix.  
    Thus, in our column sparse setting, after the initial minimization, the 
    entries of the matrices are correct up to an order-preserving reindexing.  
    To complete the minimization, the code then performs a reindexing of the 
    entire matrix.
    */
    void minimize(int verbosity);
    
    void print() const;
    void print_sparse() const;
    
private:
  
    /* 
    True iff this presentation has been minimized.
    Note that presentation might be minimal even if it hasn't been explicitly
    minimized 
    */
    bool is_minimized;
    
    /*
    If is_minimized then kernel_minimal should always hold.
    NOTE: Our algorithm for constructing a presentation from an FIRep yields
    a kernel minimal presentation.
     */
    bool is_kernel_minimal;
    
    /*
    min_gens_and_clearing_data(fir) replaces fir.high with a bigraded matrix 
    whose set of columns is a minimal set of generators for the image of the map 
    fir.high (the input matrix is in colex order; output is in lex order.)
    Also stores the pointwise ranks in hom_dim (This is an intermediate step in 
    the calculation of the homology_dimensions)
    TODO: Add clearing functionality
    */
    
    BigradedMatrixLex min_gens_and_clearing_data(FIRep& fir);

    //Re-express each column of high_mat in kernel coordinates.
    //Result is stored in mat and col_ind.
    void kernel_coordinates(BigradedMatrixLex& high_mat,
                            const BigradedMatrix& kernel);
    
    
    /*---------------- Technical Functions --------------*/
    
    //Used for min_gens_and_clear.
    void min_gens_and_clearing_data_one_bigrade(BigradedMatrix& old_high,
                                                BigradedMatrixLex& new_high,
                                                unsigned curr_x,
                                                unsigned curr_y,
                                                std::vector<int>& lows);

    //Reduce this column, putting the corresponding reduction coordinates
    void kernel_coordinates_one_bigrade(BigradedMatrixLex& high_mat,
                                        const BigradedMatrix& kernel,
                                        const unsigned& curr_x,
                                        const unsigned& curr_y,
                                        const std::vector<int>& ker_lows,
                                        unsigned& num_cols_added);
    
    //Technical function for constructing hom_dims at all indices from an FIRep.
    //Used by the Presentation constructor.
    void compute_hom_dims(const IndexMatrix& ind);
    
    /*
    Technical utility function used by Presentation.minimize().  Same action as 
    IndexMatrix::fill_index_mx() on col_ind, but also decrements each entry of a
    second IndexMatrix in the colex interval [start_grade,end_grade) by the 
    difference between value and the index of this index matrix.  To be called 
    on the member col_ind, with row_ind_mx as the argument.
    */
    void update_col_and_row_inds(IndexMatrix& row_ind_new,
                                 Grade& start_grade,
                                 const Grade& end_grade,
                                 const int& value);

    
    /*
    Technical function used to compute a minimal presetation.  Takes as input a 
    minimal presentation with the non-minimal column indices (which skip some 
    entries), and a vector giving the miminal (consecutive) column indices.
    Replaces all indices in the matrix with the minimal column indices.
    */
    void reindex_min_pres(const std::vector<int>& new_row_indices);
    
    //Technical function by Presentation::minimize()
    bool row_index_has_matching_bigrade(int j, unsigned row, unsigned col);
    
};

#endif // __Presentation_H__
