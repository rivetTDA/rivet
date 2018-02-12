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
 
 Authors: Roy Zhao (March 2017), Michael Lesnick (modified 2017-2018)
 
 
 Class: FIRep
 
 Description: Computes and stores an FI-Rep of the hom_dim^{th} homology module
 of a (possibly multicritical) bifiltered simplicial complex.  
 Constructore takes as input a BifiltrationData object,
 a representation of the FI_Rep obtained directly from text input, or
 (at least for now) a presentation. 
 In the future, we probably won't need the constructor taking a presentation.
 The constructor taking a bifiltration data object can handle the multicritical case, using a
 simple "trick" described in a paper of Scolamiero, Chacholski, and Vaccarino.

 
 Structs: VectorHash, deref_equal_fn
 
 Description:  To construct an FIRep from a BifiltrationData object, 
 we construct hash tables whose keys are pointers to simplices.  These are used, respectively,
 define the hash function and to compare compare two keys for equality.
 
*/

#ifndef __FIRep_H__
#define __FIRep_H__

//forward declarations
class IndexMatrix;
#include "presentation.h"

#include "bifiltration_data.h"
#include "map_matrix.h"
#include "index_matrix.h"
#include "bigraded_matrix.h"

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

//used to compare compare two keys in the hash tables for equality.
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
    
    //Notes that grades are stored in discrete indexes, real ExactValues are stored in InputData.x_exact and y_exact
    BigradedMatrix low_mx; //low matrix in the FIRep
    BigradedMatrix high_mx; //high matrix in the FIRep
    
    //constructor; requires verbosity parameter
    //replaces the bif_data object with something trivial
    FIRep(BifiltrationData& bif_data, int vbsty);
    
    /*
    constructor taking a presentation.  High matrix is set to a copy of the presentation matrix;
    low matrix is set to zero.
    TODO: We need this only because persistence updater currently takes an FIRep as input.
    Note also that this is inefficient, since we are making a copy fo the presentation matrix,
    though that is not necessary. In the future, we can give persistence updater a presentation directly,
    and this will be much simpler. Once that is implemented, we can remove this constructor,
    and perhaps also remove an associated constructor in the BigradedMatrix class.
     */
    FIRep(Presentation pres, int vbsty);
    
    //This constructor is used when the FIRep is given directly as text input.
    //TODO: It seems a little hacky to be passing a BifiltrationData object to this constructor
    FIRep(BifiltrationData& bif_data, unsigned num_high_simplices, unsigned num_mid_simplices, unsigned num_low_simplices, std::vector<std::vector<unsigned>>& d2, std::vector<std::vector<unsigned>>& d1,
        const std::vector<unsigned> x_values, const std::vector<unsigned> y_values, int vbsty);
    
    unsigned num_x_grades() const; //returns the number of unique x-coordinates of the multi-grades
    unsigned num_y_grades() const; //returns the number of unique y-coordinates of the multi-grades

    void print(); //Print the matrices and appearance grades

    const unsigned verbosity; //controls display of output, for debugging

private:
    unsigned x_grades; //the number of x-grades that exist in this firep
    unsigned y_grades; //the number of y-grades that exist in this firep
    
    //writes boundary information given boundary entries in column col of matrix mat
    void write_boundary_column(MapMatrix& mat, const std::vector<unsigned>& entries, const unsigned col);
};
     
     

#endif // __FIRep_H__
