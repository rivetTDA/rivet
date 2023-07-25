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
  
 Authors: Roy Zhao (March 2017), Michael Lesnick (modifications fall 2017).
 
 
 Class: BifiltrationData
   
 Description: Computes and stores the information about a bifiltration
 needed to compute a free implicit representation for homology in fixed
 dimension hom_dim.  Together with InputManager, handles 1-critical or
 multicritical Rips bifiltrations.  Only tracks simplices in dimension hom_dim-1, 
 hom_dim, and hom_dim+1, and does not store simplices of lower dimension.
 Replaces the SimplexTree class used in earlier versions of RIVET.
 
 
 Struct: LowSimplexData
   
 Description: Stores a simplex of dimension hom_dim - 1, together
 with the greatest lower bound of all grades of appearance of that simplex.
 
 Struct: MidHighSimplexData s
 
 Description: Stores a simplex s of dimension hom_dim or
 hom_dim+1, together with the bigrades of appearance of s.  Also can store the
 column indices corresponding to copies of s, though this is only
 used for simplices of dimension hom_dim.  The reason for using a single data
 structure to handle simplces of dimension hom_dim and hom_dim+1 is that for 
 multicritical filtrations, columns in the high matrix in an FIRep can correspond 
 to either simplicies of dimension hom_dim + 1, or relations.  The relations are
 indexed by simplcies of dimension hom_dim; working with a single data structure 
 for both types is convenient for sorting the columns of the high matrix.
*/

#ifndef BIFILTRATION_DATA_H
#define BIFILTRATION_DATA_H

#include "grade.h"
#include <boost/functional/hash.hpp>
#include <set>
#include <unordered_map>
#include <vector>

//typedefs
//TODO: It is probably bettter to specify a simplex using a combinatorial
//number system, as in DIPHA or Ripser.
typedef std::vector<int> Simplex;
typedef std::vector<Grade> AppearanceGrades;

struct LowSimplexData {
    Simplex s;
    Grade gr;

    LowSimplexData(Simplex simplex, Grade grade)
        : s(simplex)
        , gr(grade)
    {
    }
};

struct MidHighSimplexData {
    Simplex s;

    //vector of grades of appearance of s
    AppearanceGrades grades_vec;

    //Vector of the column indices in the low matrix corresponding to s.
    //Used to construct high boundary matrix.
    //Only used for simplcies of dimension hom_dim, not hom_dim_1.
    std::vector<unsigned> col_inds;

    AppearanceGrades::iterator grades_it;

    //TODO: Maybe slightly cleaner to use an iterator pointing to col_ind than an
    //iterator pointing to grades_vec?
    //std::vector<unsigned>::iterator ind_it;

    bool high;

    virtual bool is_high() const
    {
        return high;
    }

    //TODO: This is a little inefficient, since high_simplex data doesn't
    //actually need the data of col_inds or grades_it.  Since building the high
    //simplices can be very demanding of memory, it is probably worthwhile to
    //optimize this more.
    MidHighSimplexData(Simplex simplex, AppearanceGrades grades, bool is_high)
        : s(simplex)
        , grades_vec(grades)
        , col_inds(std::vector<unsigned>())
        , grades_it(grades_vec.begin())
        , high(is_high)
    {
    }
};





class BifiltrationData {
    friend class FIRep;

public:
    //constructor
    BifiltrationData(unsigned dim, int verbosity);

    /* 
    build_VR_complex() builds BifiltrationData representing a bifiltered
    Vietoris-Rips complex from metric data, via a straighforward recursive
    algorithm.
    NOTE: This gives a 1-critical bifiltration, i.e., one where each simplex
    has a unique bigrade of appearance.
    
    Requires:
    -a vector of birth times (one for each point),
    -a vector of distances between pairs of points,
    -the number of grade values in x- and y-directions
    
    CONVENTION: the x-coordinate is "birth time" for points and
    the y-coordinate is "distance" between points.
    */
    void build_VR_complex(const std::vector<unsigned>& times,
        const std::vector<unsigned>& distances,
        const unsigned num_x,
        const unsigned num_y);

    /* 
    build_DR_complex() builds BifiltrationData representing a degree-Rips 
     complex from metric data.  The algorithm for this uses a sweepline
     procedure designed by Roy.
     
    NOTE: This gives a multi-critical bifiltration, i.e., one where each simplex
    may have multiple grades of appearance.
     
    requires:
    -the number of vertices,
    -a vector of distances between pairs of points,
    -a vector for degree to y value exchange,
    -number of grade values in x- and y-directions
    /
     /CONVENTION: the x-coordinate is "scale parameter" for points 
     and the y-coordinate is "degree parameter"
    */
    void build_DR_complex(const unsigned num_vertices,
        const std::vector<unsigned>& distances,
        const std::vector<unsigned>& degrees,
        const unsigned num_x,
        const unsigned num_y);

    /*
    add_simplex() adds a simplex to BifiltrationData.
    grades is a vector of appearance grades
    NOTE: Changed behavior of add_simplex so that it no longer recursively
          adds in faces.
    */
    void add_simplex(const std::vector<int>& vertices,
        const AppearanceGrades& grades);

    //Sets x_grades and y_grades.
    //Used when reading in a bifiltration.
    void set_xy_grades(unsigned num_x, unsigned num_y);

    //returns the number of unique x-coordinates of the multi-grades
    unsigned num_x_grades();

    //returns the number of unique y-coordinates of the multi-grades
    unsigned num_y_grades();

    //returns the number of simplices of dimension (hom_dim-1), hom_dim,
    //or (hom_dim+1).  Assumes dim is non-negative.  Returns -1 if invalid dim.
    int get_size(unsigned dim);

    //the dimension of homology to be computed.
    //hom_dim+1 is the max dimension of simplices in bifiltration_data
    const unsigned hom_dim;

    //controls display of output, for debugging
    const int verbosity;

    //Sorts the grades of appearance in reverse lexicographic order
    //and makes sure they are all incomparable
    void update_grades(AppearanceGrades& grades);
    
    //print bifiltration in the RIVET bifiltration input format
    void print_bifiltration();

private:
    //number of distinct x-grades (y-grades) appearing this bifiltration
    unsigned x_grades;
    unsigned y_grades;

    std::vector<LowSimplexData> low_simplices;
    std::vector<MidHighSimplexData> mid_simplices, high_simplices;

    //recursive function used in build_VR_complex()
    void build_VR_subcomplex(const std::vector<unsigned>& times,
        const std::vector<unsigned>& distances,
        std::vector<int>& vertices,
        const unsigned prev_time,
        const unsigned prev_dist);

    //recursive function used in build_DR_complex()
    void build_DR_subcomplex(const std::vector<unsigned>& distances,
        std::vector<int>& parent_indexes,
        const std::vector<int>& candidates,
        const AppearanceGrades& parent_grades,
        const std::vector<AppearanceGrades>& vertex_multigrades);

    //Generates required multigrades for build_DR_complex()
    void generate_vertex_multigrades(std::vector<AppearanceGrades>& multigrades,
        const unsigned vertices,
        const std::vector<unsigned>& distances,
        const std::vector<unsigned>& degrees);

    //Finds the grades of appearance of when both simplices exist.
    //subject to minimum scale parameter. Used in build_DR_complex()
    void combine_multigrades(AppearanceGrades& merged,
        const AppearanceGrades& grades1,
        const AppearanceGrades& grades2,
        unsigned mindist);

    //total number of simplces of dimensions hom_dim and hom_dim+1,
    //counting mutiplicity in grades of appearance.
    //Used to avoid unnecessary resizing of arrays in firep constructor.
    unsigned mid_count;
    unsigned high_count;
};

#endif // BIFILTRATION_DATA_H
