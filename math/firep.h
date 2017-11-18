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
/**
 * \class   firep
 * \brief   Computes and stores an FI-Rep of the hom_dim^{th} homology module of a (possibly multicritical) bifiltered simplicial complex.  Takes either a bifiltration_data object, or a representation of the FI_Rep obtained directly from text input.
 * \author  Roy Zhao; edited by Michael Lesnick.
 * \date    March 2017; edited September 2017.
 */

#ifndef __FIRep_H__
#define __FIRep_H__

//forward declarations
class IndexMatrix;


#include "bifiltration_data.h"
#include "map_matrix.h"
#include "index_matrix.h"

#include <set>
#include <string>
#include <vector>


//used to build the hash tables for simplices in hom_dim-1 and hom_dim
struct VectorHash {
    std::size_t operator()(Simplex const* const& v) const
    {
        return boost::hash_range(v->begin(), v->end());
    }
};

struct deref_equal_fn {
    bool operator()(Simplex* const& lhs, Simplex* const& rhs) const
    {
        return *lhs == *rhs;
    }
};

typedef std::unordered_map<Simplex* const, unsigned, VectorHash, deref_equal_fn> SimplexHashLow;
typedef std::unordered_map<Simplex* const, std::vector<MidHighSimplexData>::iterator, VectorHash, deref_equal_fn> SimplexHashMid;
//no need for a hash table in the high dimension

class FIRep {

public:
    MapMatrix boundary_mx_low; //boundary matrix from dim to dim-1
    MapMatrix boundary_mx_high; //boundary matrix from dim+1 to dim
    
    IndexMatrix index_mx_low; //matrix of column indexes to accompany boundary_mx_low
    IndexMatrix index_mx_high; //matrix of column indexes to accompany boundary_mx_high
    
    FIRep(BifiltrationData& bd, int v); //constructor; requires verbosity parameter

    //This constructor is used when the FIRep is given directly as text input.
    //TODO: Minor point, but it seems a little hacky to be passing a BifiltrationData object to this constructor
    FIRep(BifiltrationData& bd, unsigned um_high_simplices, unsigned num_mid_simplices, unsigned num_low_simplices, std::vector<std::vector<unsigned>>& d2, std::vector<std::vector<unsigned>>& d1,
        const std::vector<unsigned> x_values, const std::vector<unsigned> y_values, int v); //constructor

    //TODO:Delete?
    //~FIRep(); //destructor

    //TODO: should the following two return pointers to consts?  There are places where we modify an index matrix as we zero out columns...
    


    unsigned num_x_grades(); //returns the number of unique x-coordinates of the multi-grades
    unsigned num_y_grades(); //returns the number of unique y-coordinates of the multi-grades

    void print(); //Print the matrices and appearance grades

    const unsigned verbosity; //controls display of output, for debugging

private:
    unsigned x_grades; //the number of x-grades that exist in this firep
    unsigned y_grades; //the number of y-grades that exist in this firep

    //Indexes of simplices. Grades are stored in discrete indexes, real ExactValues are stored in InputData.x_exact and y_exact

    //TODO: Get rid of these; store the index matrices themselves.
    /*
    AppearanceGrades indexes_low; //indexes of simplices in dimension dim
    AppearanceGrades indexes_high; //indexes of simplices in dimension dim+1
    */
     
    //writes boundary column.
    void write_boundary_column(MapMatrix& mat, const std::vector<unsigned>& entries, const unsigned col); //writes boundary information given boundary entries in column col of matrix mat

    /*
    IndexMatrix* get_index_mx(AppearanceGrades& source_grades); //Gets the index matrix associated with a list of grades
     */
};
     
     

#endif // __SimplexTree_H__
