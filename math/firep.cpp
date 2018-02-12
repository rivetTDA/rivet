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

// Authors: Roy Zhao (March 2017), Michael Lesnick (modified 2017-2018)

#include "firep.h"

#include "index_matrix.h"
#include "map_matrix.h"

#include "debug.h"

#include <algorithm>
#include <ctime>
#include <limits> //std::numeric_limits
#include <sstream>
#include <stdexcept>

//constructor; requires verbosity parameter
FIRep::FIRep(Presentation pres, int vbsty)
    : low_mx(BigradedMatrix(MapMatrix(0,pres.mat.height()),pres.row_ind))
    , high_mx(BigradedMatrix(pres.mat,pres.col_ind))
    , verbosity(vbsty)
    , x_grades(pres.row_ind.width())
    , y_grades(pres.row_ind.height())
{}

//FIRep constructor; requires BifiltrationData object (which in particular specifies the homology dimension) and verbosity parameter
FIRep::FIRep(BifiltrationData& bif_data, int vbsty)
    : low_mx(0,0,bif_data.num_y_grades(),bif_data.num_x_grades())
    , high_mx(0,0,bif_data.num_y_grades(),bif_data.num_x_grades())
    , verbosity(vbsty)
    , x_grades(bif_data.num_x_grades())
    , y_grades(bif_data.num_y_grades())
{
    if (verbosity >= 10) {
        bif_data.print_bifiltration();
    }

    //We build the low matrix, then the high matrix.
    //First, process simplices of dimension hom_dim-1.

    //Sort the low_simplices in colexicographical order, using the comparator defined for Grade.  We constructed the simplices in
    //lexicographical order on vertex indices, so use of stable sort preserves this within a grade.

    std::stable_sort(bif_data.low_simplices.begin(), bif_data.low_simplices.end(), [](const LowSimplexData& left, const LowSimplexData& right) {
        return left.gr < right.gr;
    });

    //Enter the low simplex indices into a hash table.  //Used to construct the low matrix
    SimplexHashLow low_ht; //key = simplex; value = index of that simplex in Low_Simplices

    for (unsigned i = 0; i != bif_data.low_simplices.size(); i++) {
        low_ht.emplace(&(bif_data.low_simplices[i].s), i);
    }

    //Now process simplices of dimension hom_dim

    //construct a vector with one entry per simplex per grade of appearance.
    //After sorting, this will index the columns of the low matrix
    auto mid_generators = std::vector<std::pair<std::vector<MidHighSimplexData>::iterator, AppearanceGrades::iterator>>();
    mid_generators.reserve(bif_data.mid_count);

    for (auto it = bif_data.mid_simplices.begin(); it != bif_data.mid_simplices.end(); it++) {
        //set aside space for indices in each MidHighSimplexData struct.
        it->ind.reserve(it->ag.size());
        //iterate through the grades
        for (auto it2 = it->ag.begin(); it2 != it->ag.end(); it2++)

            //populate mid_generators with pairs of iterators which specify a simplex and its grade of appearance
            mid_generators.push_back(std::pair<std::vector<MidHighSimplexData>::iterator, AppearanceGrades::iterator>(it, it2));
    }

    //stably sort mid_generators according to colex order on grades
    std::stable_sort(mid_generators.begin(), mid_generators.end(), [](const auto& left, const auto& right) { return *(left.second) < *(right.second); });
    
    //create the MapMatrix of the appropriate size
    low_mx.mat = MapMatrix(bif_data.low_simplices.size(), mid_generators.size());

    if (verbosity >= 6)
        debug() << "Creating low matrix of dimension" << bif_data.low_simplices.size() << "x" << mid_generators.size();

    //loop through simplices, writing columns to the matrix
    Grade prev_grade;
    
    if (bif_data.mid_count > 0)
        prev_grade = *(mid_generators[0].second);
        
    for (unsigned i = 0; i < bif_data.mid_count; i++) {
        
        //fill in the appropriate part of the index matrix, if any
        low_mx.ind.fill_index_mx(prev_grade, *(mid_generators[i].second), i-1);
    
        //If hom_dim==0, there are no columns to fill in.
        if (bif_data.hom_dim > 0)
        {
            //reserve space to work
            Simplex face;
            face.reserve(bif_data.hom_dim);
            //call the simplex "vertices";
            Simplex& vertices = mid_generators[i].first->s;
        
            //TODO: reserve space in the column of low_mx for the entries we will add in?  Because of the nested interfaces, a few classes would have to be changed.  Probably not worth it.
        
            //find all faces of this simplex
            for (unsigned k = 0; k < vertices.size(); k++) {
                //face vertices are all vertices in verts[] except verts[k]
                for (unsigned l = 0; l < vertices.size(); l++)
                    if (l != k)
                        face.push_back(vertices[l]);
                
                //use hash table to look up the index of this face in low_simplices;
                auto face_node = low_ht.find(&face);
                if (face_node == low_ht.end())
                    throw std::runtime_error("FIRep constructor: face simplex not found.");
                
                //now write the row index to row_indices
                low_mx.mat.set(face_node->second,i);
                face.clear();
            }
        
            //heapify this column
            low_mx.mat.prepare_col(i);
        }
    }
    
    //Now complete construction of the low_mx.ind.
    if (bif_data.mid_count > 0)
        low_mx.ind.fill_index_mx(prev_grade, Grade(0,low_mx.ind.height()), mid_generators.size()-1);
    //We're done building low_mx.
    
    //We no longer need low_simplices or the hash table, so replace by something trivial
    std::vector<LowSimplexData>().swap(bif_data.low_simplices);
    SimplexHashLow().swap(low_ht);

    if (verbosity >= 6)
        debug() << "Created low matrix";

    //Next, we build the high matrix.
    //First, we need to do more processing of the simplices in dimension hom_dim

    //Record the sorted indices in the MidHighSimplexData structs.
    for (unsigned i = 0; i != mid_generators.size(); i++) {
        //this funky line uses the iterator in ag to access the corresponding element in ind, and also increments the iterator in ag
        *(mid_generators[i].first->ind.begin() + std::distance(mid_generators[i].first->ag.begin(), mid_generators[i].first->ag_it++)) = i;
    }

    //Reset all of the ag_it iterators to their start position.  This is needed for when we build the high matrix.
    for (auto it = bif_data.mid_simplices.begin(); it != bif_data.mid_simplices.end(); it++) {
        it->ag_it = it->ag.begin();
    }

    //Enter the mid_simplex indices into a hash table.
    SimplexHashMid mid_ht; //key = simplex; value = iterator pointing to that simplex in mid_simplices

    for (auto it = bif_data.mid_simplices.begin(); it != bif_data.mid_simplices.end(); it++) {
        mid_ht.emplace(&(it->s), it);
    }

    //The following vector will index the columns of the high matrix
    auto high_generators = std::vector<std::pair<std::vector<MidHighSimplexData>::iterator, AppearanceGrades::iterator>>();

    //We know in advance how many columns the high matrix will have.
    high_generators.reserve(bif_data.high_count + bif_data.mid_count - bif_data.mid_simplices.size());

    //Add the relations to high_generators
    //Relations will be represented implicity as using two pieces of data:
    //1) An AppearanceGrades::iterator it2, where the AppearanceGrades object in question is a member of a MidHighSimplexData Object m.
    //2) An iterator it pointing to m in mid_simplices.
    //The relation represented is the one between *it2 and *(it2+1).
    
    for (auto it = bif_data.mid_simplices.begin(); it != bif_data.mid_simplices.end(); it++) {
        for (AppearanceGrades::iterator it2 = it->ag.begin(); (it2 + 1) != it->ag.end(); it2++) {
            high_generators.push_back(std::pair<std::vector<MidHighSimplexData>::iterator, AppearanceGrades::iterator>(it, it2));
        }
    }

    //Add each (simplex,grade) pair of dimension (hom_dim+1) to high_generators
    for (auto it = bif_data.high_simplices.begin(); it != bif_data.high_simplices.end(); it++) {
        //iterate through the grades
        for (auto it2 = it->ag.begin(); it2 != it->ag.end(); it2++) {
            //populate high_generators with pairs
            high_generators.push_back(std::pair<std::vector<MidHighSimplexData>::iterator, AppearanceGrades::iterator>(it, it2));
        }
    }

    //Now we've built the list of high_generators, and we need to sort it.  We have stored the relations implicitly, so the comparator
    //has to compute the grade of a relation as part of the sorting.

    //TODO: In the following sorting, one could instead precompute the grades of the relations, and avoid copying/recomputing grades.  This probably would be more efficient, but not clear whether it is worth the trouble.
    std::stable_sort(high_generators.begin(), high_generators.end(),
        [](const auto& left, const auto& right) {
            if (left.first->is_high() && right.first->is_high())
                // both left and right are high simplices.  Order according to grade.  because we use a stable sort
                // and simplices are already in lexicographical order by vertex index, ties are broken by vertex index,
                // which examples have shown to be an efficient strategy in practice.
                return *(left.second) < *(right.second);
            else {
                Grade left_grade, right_grade;

                if (left.first->is_high()) {
                    left_grade = *(left.second);
                } else {
                    //The vector of appearance grades is aassumed to be in colex order, so the following gives the grade of the relation
                    left_grade = Grade(left.second->x, (left.second + 1)->y);
                }

                if (right.first->is_high()) {
                    right_grade = *(right.second);
                } else {
                    //as above, the following gives the grade of the relation
                    right_grade = Grade(right.second->x, (right.second + 1)->y);
                }

                if (!(left_grade == right_grade))
                    return left_grade < right_grade;
                else {
                    // If we get here, the grades are equal and either one operand is a simplex and the other is a relation,
                    // or else both are relation.  In the former case, we take left < right iff left is a relation and right is not.  This is a choice that (heurisitically speaking) seems likely to be efficient.

                    // In the latter case, we will take left and right to be incompable.  In either case, the following line gives what we want.
                    return left.first->is_high() < right.first->is_high();
                }
            }
        });
    //high_generators is now properly sorted.

    //Compute the matrix.  This is more complex than in the lower dimension, because (in the multicritical setting) we need to handle the relations and make a valid choice from among several possible birth grades of a boundary simplex.

    //create the MapMatrix
    high_mx.mat = MapMatrix(mid_generators.size(), high_generators.size());
    if (verbosity >= 6)
        debug() << "Creating high matrix of dimension" << mid_generators.size() << "x" << high_generators.size();

    //reserve space to work
    Simplex face;
    face.reserve(bif_data.hom_dim + 1);

    if (high_generators.size() > 0)
        prev_grade = *(high_generators[0].second);
    
    //add in columns of matrix and set the entries of the corresponding IndexMatrix
    for (unsigned i = 0; i < high_generators.size(); i++) {
        
        if (high_generators[i].first->is_high()) {
            
            //set entries of the index matrix
            high_mx.ind.fill_index_mx(prev_grade, *(high_generators[i].second), i-1);
            
            //TODO: reserve space in the column of high_mx for the entries we will add in?  Because of all the nested interfaces, a few classes would have to be changed.  Probably not worth it.

            //call the simplex "vertices";
            Simplex& vertices = high_generators[i].first->s;

            //find all faces of this simplex
            for (unsigned k = 0; k < vertices.size(); k++) {
                //face vertices are all vertices in verts[] except verts[k]
                for (unsigned l = 0; l < vertices.size(); l++)
                    if (l != k)
                        face.push_back(vertices[l]);

                //use hash table to look up the index of this face in mid_simplices;
                auto face_node = mid_ht.find(&face);

                if (face_node == mid_ht.end())
                    throw std::runtime_error("FIRep constructor: face simplex not found.");

                //face_node->second gives an iterator pointing to the face.
                //Now choose a suitable bigrade for the face.

                //A COOL TRICK: Simplices are colex ordered, and so is each AppearanceGrades vector ag.  Thus, it can be shown that if we reject a grade of appearance once, we do not need to consider it again.  This saves us time in building the FI-Rep.

                //Note however, that the construction of the matrix depends on the choice of the grade of the boundary element, and it is not clear how this choice impacts the speed of subsequent computations taking the FIRep as input.  In the absense of any insight into this, we simply make the choice that allows us to build the FI-Rep most quickly.

                bool found_match = false;
                while (!found_match) {
                    //Did we find a grade for this boundary simplex that is less than the grade of the simplex itself, in the partial order on R^2?
                    if ((face_node->second->ag_it->x <= high_generators[i].second->x) && (face_node->second->ag_it->y <= high_generators[i].second->y)) {
                        //insert the index in mid_generators corresponding to this (simplex,grade) pair.
                        
                        
                        high_mx.mat.set(*(face_node->second->ind.begin() + std::distance(face_node->second->ag.begin(), face_node->second->ag_it)),i);
                        found_match = true;
                    } else {
                        face_node->second->ag_it++;
                    }
                }
                face.clear();
            }
            high_mx.mat.prepare_col(i);
            }

        else {
            //if we are here, i indexes a relation.
            
            //Update high_mx.ind in the appropriate way.
            Grade curr_grade = Grade(high_generators[i].second->x, (high_generators[i].second + 1)->y);
            high_mx.ind.fill_index_mx(prev_grade, curr_grade, i-1);

            //add the second index of the relation to the ith column of high_mx
            high_mx.mat.set(*(high_generators[i].first->ind.begin() + std::distance(high_generators[i].first->ag.begin(), high_generators[i].second) + 1),i);
            
            //add the first index of the relation to the ith column of high_mx
            high_mx.mat.set(*(high_generators[i].first->ind.begin() + std::distance(high_generators[i].first->ag.begin(), high_generators[i].second)),i);
            
            //Note: The order in which we add the two elements matters; by construction, column is in heap order (see documentation for std::pop_heap), so no need to call high_mx.mat.prepare_col().
        }
    }
    
    //Now we complete construction of the high_mx.ind.
    if (high_generators.size() > 0)
        high_mx.ind.fill_index_mx(prev_grade, Grade(0,high_mx.ind.height()), high_generators.size()-1);
    //We're done building high_mx.
    
    //We no longer need mid_simplices or high simplices, so replace with something trivial.
    std::vector<MidHighSimplexData>().swap(bif_data.mid_simplices);
    std::vector<MidHighSimplexData>().swap(bif_data.high_simplices);
    
    //Replace hash_table with something trivial.
    SimplexHashMid().swap(mid_ht);
    
    if (verbosity >= 6)
        debug() << "Created high matrix";

    if (verbosity >= 6) {
        debug() << "Created FIRep";
    }
    if (verbosity >= 10) {
        print();
    }

}


//This constructor is used when the FIRep is given directly as text input.  To understand this part of code, it may be helpful to review the conventions for the firep text input format, as explained in the RIVET documentation.
FIRep::FIRep(BifiltrationData& bif_data, unsigned num_high_simplices, unsigned num_mid_simplices, unsigned num_low_simplices, std::vector<std::vector<unsigned>>& d2, std::vector<std::vector<unsigned>>& d1, const std::vector<unsigned> x_values, const std::vector<unsigned> y_values, int vbsty)
    : low_mx(num_low_simplices, num_mid_simplices, bif_data.num_y_grades(), bif_data.num_x_grades())
    , high_mx(num_mid_simplices, num_high_simplices, bif_data.num_y_grades(), bif_data.num_x_grades())
    , verbosity(vbsty)
    , x_grades(bif_data.num_x_grades())
    , y_grades(bif_data.num_y_grades())

{
    //TODO: Instead of using separate x_value and y_value vectors, it would be more efficient to use a vector of pairs.
    //Then we could just store pointers to these grades instead of copying them.
    std::vector<std::pair<Grade, unsigned>> mid_indexes;
    Grade temp_grade;
    for (unsigned i = 0; i < num_mid_simplices; i++) {
        temp_grade = Grade(x_values[i + num_high_simplices], y_values[i + num_high_simplices]);
        mid_indexes.push_back(std::pair<Grade, unsigned>(temp_grade, i));
    }

    std::sort(mid_indexes.begin(), mid_indexes.end(), [](const auto& left, const auto& right) { return left.first < right.first; });
    std::vector<unsigned> inverse_map(num_mid_simplices);
    for (unsigned i = 0; i < num_mid_simplices; i++) {
        inverse_map[mid_indexes[i].second] = i;
    }

    Grade prev_grade;
    
    if (num_mid_simplices > 0)
        prev_grade = mid_indexes[0].first;
    
    for (unsigned i = 0; i < num_mid_simplices; i++) {
        write_boundary_column(low_mx.mat, d1[mid_indexes[i].second], i);
        
        low_mx.ind.fill_index_mx(prev_grade, mid_indexes[i].first, i-1);
    }
    
    //Now we complete construction of the low_mx.ind.
    if (num_mid_simplices > 0)
        low_mx.ind.fill_index_mx(prev_grade, Grade(0,low_mx.ind.height()), mid_indexes.size()-1);
    
    std::vector<std::pair<Grade, unsigned>> high_indexes;
    for (unsigned i = 0; i < num_high_simplices; i++) {
        temp_grade = Grade(x_values[i], y_values[i]);
        high_indexes.push_back(std::pair<Grade, unsigned>(temp_grade, i));
    }

    std::sort(high_indexes.begin(), high_indexes.end(), [](const auto& left, const auto& right) { return left.first < right.first; });

    if (num_high_simplices > 0)
        prev_grade = high_indexes[0].first;
    
    for (unsigned i = 0; i < num_high_simplices; i++) {
        std::vector<unsigned> entries = d2[high_indexes[i].second];
        for (unsigned j = 0; j < entries.size(); j++) {
            entries[j] = inverse_map[entries[j]];
        }
        write_boundary_column(high_mx.mat, entries, i);
        
        high_mx.ind.fill_index_mx(prev_grade, high_indexes[i].first, i-1);
    }

    //Now we complete construction of high_mx.ind
    if (num_high_simplices > 0)
        high_mx.ind.fill_index_mx(prev_grade, Grade(0,high_mx.ind.height()), high_indexes.size()-1);
    
    if (verbosity >= 8)
        debug() << "Created FIRep";

    if (verbosity >= 10)
        print();
}

//writes boundary information given boundary entries in column col of matrix mat
void FIRep::write_boundary_column(MapMatrix& mat, const std::vector<unsigned>& entries, const unsigned col)
{
    for (unsigned k = 0; k < entries.size(); k++) {
        //for this boundary simplex, enter "1" in the appropriate cell in the matrix
        mat.set(entries[k], col);
    }
} //end write_boundary_column();

//returns the number of unique x-coordinates of the multi-grades
unsigned FIRep::num_x_grades() const
{
    return x_grades;
}

//returns the number of unique y-coordinates of the multi-grades
unsigned FIRep::num_y_grades() const
{
    return y_grades;
}

//Print the matrices and appearance grades
void FIRep::print()
{
    std::cout << "low matrix: " << std::endl;
    low_mx.print();
    std::cout << "high matrix: " << std::endl;
    high_mx.print();
}


