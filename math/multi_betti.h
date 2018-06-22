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
 
 Author: Matthew L. Wright (February 2014- )
 Minor modifications by Michael Lesnick (February 2018)
 
 Class: MultiBetti
 
 Description: This class provides functionality for computing the bigraded Betti numbers xi_i
 and vector space dimensions (i.e., Hilbert function) of a bipersistence module.
 MultiBetti objects store the Betti numbers, but not the Hilbert functions, which are 
 stored elsewhere.
 
 The main method of this class, compute_koszul, computes xi_0, xi_1  
 and the Hilbert funtion from an FIRep, without computing a presentation,
 via Mike and Matthew's Koszul homology algorithm.  
 Another method, compute_xi2(), computes and stores xi_2 from the values of xi_0, xi_1 and 
 the Hilbert function.
 
 In this version of RIVET, a (rather trivial) method, read_betti(),
 is also provided to read the 0th and 1st Betti numbers off of a minimal presentation.
 (This is to be used in conjunction with the class Presentation, which can compute a minimal 
 presentation from an FIRep).
 
 */

#ifndef __MultiBetti_H__
#define __MultiBetti_H__

//forward declarations
struct ColumnList; //necessary for column reduction in MultiBetti::reduce(...)
class ComputationThread;
class IndexMatrix;
class MapMatrix;
class FIRep;
class TemplatePoint;

#include "presentation.h"
#include <interface/progress.h>

#include <vector>
typedef std::vector<int> Vector;

class MultiBetti {
public:
    //constructor: sets up the data structure but does not compute xi_0 or xi_1
    MultiBetti(const FIRep& fir);

    //computes xi_0 and xi_1 from a (possibly unminimized) presentation, using a
    //modified version of the standard reduction.  Note that the Presentation is
    //not passed by reference, but is copied.  This is intentional.
    void compute_from_pres(Presentation pres, Progress& progress);

    //computes xi_0 and xi_1, and also stores dimension of homology at each grade in the supplied matrix
    //This is the orginal RIVET algorithm, based on Koszul homology
    void compute_koszul(FIRep& fir, unsigned_matrix& hom_dims, Progress& progress);

    //Reads the 0th and 1st Betti numbers off of a minimal presentation into the multi_betti object.
    void read_betti(const Presentation& pres);

    //TODO: Implement this.
    //Compute the 0th and 1st Betti numbers from a not-necessarily-minimal presentation
    //void compute_non_min_pres(const Presentation & pres, Progress& progress);

    //computes xi_2 from the values of xi_0, xi_1 and the dimensions
    void compute_xi2(unsigned_matrix& hom_dims);

    //returns xi_0 at the specified grade
    int xi0(unsigned x, unsigned y);

    //returns xi_1 at the specified grade
    int xi1(unsigned x, unsigned y);

    //stores the xi support points in lexicographical order
    void store_support_points(std::vector<TemplatePoint>& tpts);

private:
    unsigned num_x_grades; //number of grades in primary direction
    unsigned num_y_grades; //number of grades in secondary direction
    boost::multi_array<int, 3> xi; //matrix to hold xi values; indices: xi[x][y][subscript]
    const unsigned verbosity; //controls display of output, for debugging

    //************* methods used by Koszul Betti algorithm *************

    //simple column reduction algorithm
    //  pivot columns are first_col to last_col, inclusive
    //  increments nonzero_cols by the number of columns in [first_col, last_col] that remained nonzero
    void reduce(MapMatrix* mm, int first_col, int last_col, Vector& lows, long& nonzero_cols);

    //column reduction algorithm that also performs column operations on a slave matrix
    //  pivot columns are first_col to last_col, inclusive
    //  increments zero_cols by the number of zero-columns in [first_col, last_col], regardless of whether they
    //      were zeroed out in this reduction or zero before this function was called
    //  indexes of columns that are zeroed out are inserted into zero_list
    void reduce_slave(MapMatrix* mm, MapMatrix* slave1, MapMatrix* slave2, int first_col, int last_col, Vector& lows,
        unsigned y_grade, ColumnList& zero_list, long& zero_cols);

    //column reduction of two matrices spliced together by y-grade
    //  pivot columns are:
    //      step 1: if grade_y > 0, then reduce right matrix at (0...grade_x, grade_y - 1)
    //      step 2: reduce left matrix at (0...grade_x, grade_y)
    //      step 3: if grade_x > 0, then reduce right matrix at (0...grade_x - 1, grade_y)
    //  returns values:
    //      nonzero_cols_steps12: of the columns reduced in steps 1 and 2, the number that remained nonzero
    //      nonzero_cols_step3: of the columns reduced in step 3, the number that remained nonzero.
    void reduce_spliced(MapMatrix* m_left, MapMatrix* m_right, IndexMatrix* ind_left, IndexMatrix* ind_right,
        ColumnList& right_cols, unsigned grade_x, unsigned grade_y, Vector& lows,
        long& nonzero_cols_steps12, long& nonzero_cols_step3);

    //another version of reduce_spliced for computing dim(U) for xi_1
    //  increments nonzero_cols by the number of columns that were reduced and remained nonzero
    void reduce_spliced(MapMatrix* m_left, MapMatrix* m_right, IndexMatrix* ind_left, IndexMatrix* ind_right,
        ColumnList& right_cols, unsigned grade_x, unsigned grade_y, Vector& lows, long& nonzero_cols);

    //builds the bdry2 matrix of the direct sum B + C
    //  input: matrix bdry2, corresponding index matrix ind2
    //  output: matrix bdry2sum, corresponding index matrix ind2sum
    void build_bdry2s_mx(MapMatrix* bdry2, IndexMatrix* ind2, MapMatrix* bdry2sum, IndexMatrix* ind2sum);

    //builds the split matrix
    //  input: number of columns in the matrix
    //  output: matrix containing size columns and 2*size rows; matrix is two stacked copies of the (size x size) identity matrix
    MapMatrix* build_split_mx(unsigned size);
};

#endif // __MultiBetti_H__
