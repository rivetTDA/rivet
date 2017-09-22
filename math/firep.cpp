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

#include "firep.h"

#include "index_matrix.h"
#include "map_matrix.h"

#include "debug.h"

#include <algorithm>
#include <limits> //std::numeric_limits
#include <sstream>
#include <stdexcept>
#include <ctime>

//FIRep constructor; requires BifiltrationData object (which in particular specifies the homology dimension) and verbosity parameter
FIRep::FIRep(BifiltrationData& bd, int v)
: verbosity(v), x_grades(bd.num_x_grades()), y_grades(bd.num_y_grades()), bifiltration_data(bd)
{
    if (verbosity >= 10) {
        bd.print_bifiltration();
    }
    
    //First we build the low boundary matrix, then the high boundary matrix.
    
    //First process simplices of hom_dim-1
    
    //sort the low_simplices in colexicographical order, using the comparator defined for Grade.  We constructed the simplices in
    //lexicographical order on vertex indices, so use of stable sort preserves this within a grade.
    
    //TODO: Would probably be slightly cheaper to create a separate array for sorting.  However, these objects are not so large, so probably fine to sort in place.
    std::stable_sort(bd.low_simplices.begin(), bd.low_simplices.end(),[](const LowSimplexData& left,const LowSimplexData& right)
    {
        return left.gr<right.gr;
    });
    
    //Enter the low simplex indices into a hash table.
    SimplexHashLow low_ht; //key = simplex; value = index of that simplex in Low_Simplices
    
    //keys are pointers to simplices, values is an unsigned integer giving the index of this simplic in the sorted list.
    //Used to construct the hom_dim^{th} boundary matrix
    for (unsigned i = 0; i != bd.low_simplices.size(); i++)
    {
        low_ht.emplace(bd.low_simplices[i].s,i);
    }

    //Now deal with dimension dim
    
    //construct a vector with one entry per simplex per grade of appearance.  This is the object we sort.
    //We didn't have to do this for dimension hom_dim-1 because there we kept only one simplex per dimension.
    
    auto mid_generators = std::vector<std::pair<std::vector<MidSimplexData>::iterator , AppearanceGrades::iterator>>();
    mid_generators.reserve(bd.mid_count);
    indexes_0.reserve(mid_generators.size());
    
    for (auto it = bd.mid_simplices.begin(); it != bd.mid_simplices.end(); it++)
    {
        //set aside space for indices in each MidSimplexData struct.
        it->ind.reserve(it->ag.size());
        //iterate through the grades
        for (auto it2 =it->ag.begin(); it2 != it->ag.end(); it2++)
            //populate mid_generators with pairs which specify a simplex and its grade of appearance
            mid_generators.push_back(std::pair<std::vector<MidSimplexData>::iterator,AppearanceGrades::iterator>(it,it2));
    }

    
    //sort mid_generators according to colex order on grades
    std::stable_sort(mid_generators.begin(),mid_generators.end(),[](const auto& left,const auto& right) {return *(left.second)<*(right.second);});
    
    //create the MapMatrix and the list of low grades indexes_0;
    boundary_mx_0 = new MapMatrix(low_simplices.size(), mid_generators.size());
    
    
    if (verbosity >= 6)
        debug() << "Creating boundary matrix of dimension" << low_simplices.size() << "x" << bd.mid_count.size();
    
    //loop through simplices, writing columns to the matrix
    
    //if hom_dim==0, we don't need to do anything
    if (hom_dim>0)
    {
        //reserve space to work
        std::vector<unsigned> row_indices;
        row_indices.reserve(bd.hom_dim+1);
        
        Simplex std::vector<unsigned> face;
        face.reserve(bd.hom_dim);
        //
        
        for (unsigned i = 0; i < mid_generators.size(); i++)
        {
            indexes_0.push_back(*(mid_generators[i].second));
            
            //call the simplex "vertices";
            Simplex& vertices = mid_generators[i].first->s;

            //find all faces of this simplex
            for (unsigned k = 0; k < vertices.size(); k++) {
                //face vertices are all vertices in verts[] except verts[k]
                for (unsigned l = 0; l < vertices.size(); l++)
                    if (l != k)
                        face.push_back(vertices[l]);
            
                //use hash table to look up the index of this face in low_simplices;
                auto face_node=low_ht.find(face);
                if (face_node == low_ht->end())
                    throw std::runtime_error("FIRep::write_boundary_column(): face simplex not found.");
            
                //now write the row index to row_indices
                row_indices.push_back(face_node->second);
                
                face.clear();
            }
            //We sort because it take O(n) time to insert if the indices are sorted, and O(n^2) for a random ordering
            std::sort(row_indices.begin(), row_indices.end());
            
            //add in the boundary column
            //TODO: In an array implementation of columns, this step would be cheaper.  We may want this eventually.
            write_boundary_column(boundary_mx_0, row_indices, i);
            row_indices.clear();
        }
    }
    //We're done building one of the matrices, except we haven't stored the grade information that RIVET requires.
    //TODO: Must add this in by modifying the relevant function below.
    
    //We no longer need low_simplices
    std::vector<LowSimplexData>().swap(low_simplices); //"Free" memory of low_generators
    
    if (verbosity >= 6)
        debug() << "Created low boundary matrix";
    
    //Next, we build the high boundary matrix.
    //First, we need to do more processing of the simplices in dimension hom_dim
    
    //Place the sorted indices in the MidSimplexData structs.
    for(unsigned i=0; i!= mid_generators.size(); i++)
    {
        //this funky line uses the iterator in ag to access the corresponding element in ind, and also increments the iterator in ag
        *(mid_generators[i].first.ind.begin()+std::distance(ag.begin(),ag_it++))=i;
    }
    
    //Reset all of the ag_it iterators to their start position.  This is needed for when we build the high boundary matrix.
    for (auto it = bd.mid_simplices.begin(); it != bd.mid_simplices.end(); it++)
    {
        it->ag_it=ag.begin();
    }
    
    //Enter the mid_simplex indices into a hash table.
    SimplexHashMid mid_ht; //keylist = vector representing a simplex; value = iterator pointing to that simplex in mid_simplices

    //key is a simplex, value is an unsigned integer giving the index of this simplic in the sorted list.
    for (unsigned i  = 0; i != bd.mid_simplices.size(); i++)
    {
        mid_ht.emplace(bd.mid_simplices[i].s,i);
    }
    
    //Build the list of relations in dimension hom_dim+1:
    
    //Note: The reason we use a pointer below, not an iterator, is technical; it has to do with the fact that we want to store (and sort) both HighSimplexData and MidSimplex objects, which are related by inheriticace.  I believe that iterators do not have the right polymorphic behavior for our needs.
    auto high_generators = std::vector<std::pair<HighSimplexData*,AppearanceGrades::iterator>>();
    
    //We know in advance how many columns the high boundary matrix will have.
    high_generators.reserve(bd.high_count+bd.mid_count-bd.mid_simplices.size());
    indexes_1.reserve(high_generators.size());
    
    //Add the relations to high_generators
    //Relations will be represented implicity as AppearanceGrades::iterator objects, where the AppearanceGrade in question is
    //a member of a MidSimplexData Object m.
    //This iterator is enough to carry out the sorting we need to do.
    //We also store a pointer to m, since the simplex itself wil be needed to construct the boundary
    
    for (auto it = mid_simplices.begin(); it != mid_simplices.end(); it++)
    {
        for (AppearanceGrades::iterator it2 = it->ag.begin(); (it2 + 1) != it->ag.end(); it2++)
        {
            high_generators.push_back(std::pair<HighSimplexData*,AppearanceGrades::iterator>(&(*it),it2));
        }
    }
    
    //Add the simplices in hom_dim+1 to high_generators
    for (auto it = bd.high_simplices.begin(); it != bd.high_simplices.end(); it++)
    {
        //iterate through the grades
        for (auto it2 =it->ag.begin(); it2 != it->ag.end(); it2++)
        {
            //populate mid_simplices_mult with pairs which specify a simplex and its grade of appearance
            high_generators.push_back(std::pair<HighSimplexData*,AppearanceGrades::iterator>(&(*it),it2));
        }
    }
    
    //Now we've build the list of high_generators, and we need to sort it.  We have stored the relations implicitly, so the comparator
    //has to compute the grade of a relation as part of the sorting.
    
    //TODO: In the following sorting, one could instead precompute the grades of the relations.  This is probably more efficient, though it's not clear that this sorting will be a bottleneck anyway, compared to constructing the matrices.
    std::stable_sort(high_generators.begin(),high_generators.end(),
                     [](const auto& left,const auto& right)
                     {
                         if (left.first->is_high && right.first->is_high)
                             // both left and right are high simplices.  Order according to grade.  because we use a stable sort
                             // and simplices are already in lexicographical order by vertex index, ties are broken by vertex index,
                             // which examples have shown to be an efficient strategy in practice.
                             return *(left.second) < *(right.second);
                         else
                         {
                             if (left.first->is_high)
                             {
                                 Grade& left_grade = *(left.second);
                             }
                             else
                             {
                                 //The vector of appearance grades is aassumed to be in colex order, so the following gives the grade of the relation
                                 Grade left_grade=Grade(left.second->x,(left.second+1)->y)
                             }
                         
                            if (right.first->is_high)
                            {
                                 Grade& right_grade = *(right.second);
                            }
                            else
                            {
                             //as above, the following gives the grade of the relation
                                 Grade right_grade=Grade(right.second->x,(right.second+1)->y)
                            }
                         
                             if (left_grade!=right_grade)
                                 return *(left_grade.second) < *(right_grade.second);
                             else if (left.first->is_high!=right.first->is_high)
                             {
                                 // If we get here, the grades are equal and
                                 // one operand is a simplex, the other is a relation.
                                 // we take left < right iff left is a relation and right is not.
                                 // This is a choice that (heurisitically speaking) seems likely to be efficient.
                                 return left.first->is_high < right.first->is_high;
                             }
                             else
                                 // If we get here, the grades are equal and
                                 // both left and right are relations
                                 // In this case, we take left and right to be incomparable
                                 // TODO: Would a more refined sorting strategy yield better results?
                                 
                                 return false;
                         }
                     }
                     );
    //high_generators is now properly sorted.
    
    //Compute the boundary matrix.  This is more complex than in the lower dimension, because we need to handle the relations, and need to find which of several copies of a mid_simplex is suitable to use.
    
    //create the MapMatrix
    boundary_mx_1 = new MapMatrix(mid_generators.size(), high_generators.size());
    if (verbosity >= 6)
        debug() << "Creating boundary matrix of dimension" << mid_generators.size() << "x" << high_generators.size();
    
    //reserve space to work
    std::vector<unsigned> row_indices;
    row_indices.reserve(bd.hom_dim+2);
        
    Simplex std::vector<unsigned> face;
    face.reserve(bd.hom_dim+1);

    //add in columns of boundary matrix and create list of grades.
    for (unsigned i = 0; i < high_generators.size(); i++)
    {
        if (high_generators[i].first->is_high())
        {
            //if we are here, i indexes a simplex in dimension hom_dim+1, not a relation
            indexes_1.push_back(*(high_generators[i].second));
            
            //call the simplex "vertices";
            Simplex& vertices = high_generators[i].first->s;
        
            //find all faces of this simplex
            for (unsigned k = 0; k < vertices.size(); k++)
            {
                //face vertices are all vertices in verts[] except verts[k]
                for (unsigned l = 0; l < vertices.size(); l++)
                    if (l != k)
                        face.push_back(vertices[l]);
            
                //use hash table to look up the index of this face in low_simplices;
                auto face_node=mid_ht.find(face);
                if (face_node == mid_ht->end())
                    throw std::runtime_error("FIRep::write_boundary_column(): face simplex not found.");
            
                //face_node->second gives the boundary simplex, but there may be several different grades of appearance.  Choose one.
                
                //Simplices are colex ordered.  So are the multiple grades of appearance of a given simplex.  Thus, it can be shown that if we reject a grade of appearance once, we do not need to consider it again.  This can save us time.
                
                //Note however, that the construction of the boundary matrix depends on the chocie of the grade, and it is not yet clear how this choice impacts the performance of the algorithm.
                    
                bool found_match=false;
                while (!found_match)
                {
                    //Did we find a grade for this boundary simplex that is less than the grade of the simplex itself?
                    if (*(face_node.second->ag_it) <= *(high_generators[i].second))
                    {
                        //insert the index in mid_generators corresponding to this grade
                        row_indices.push_back(*(ind.begin()+std::distance(ag.begin(),ag_it)));
                        found_match=true;
                    }
                    else
                        ag_it++;
                }
                face.clear();
            }
                
            //We sort because it take O(n) time to insert if the indices are sorted, and O(n^2) for a random ordering
            std::sort(row_indices.begin(), row_indices.end());
            
            //add in the boundary column
            write_boundary_column(boundary_mx_1, row_indices, i);
            row_indices.clear();
        }
        
        else
        {
            //if we are here, i indexes a relation.
            indexes_1.push_back(Grade(high_generators[i].second->x, (high_generators[i].second+1)->y));
            
            //add the first index of the relation to row_indices
            row_indices.pushback(row_indices.push_back(*(ind.begin()+std::distance(ag.begin(),high_generators[i].second))));
            
            //add the second index of the relation to row_indices
            row_indices.pushback(row_indices.push_back(*(ind.begin()+std::distance(ag.begin(),high_generators[i].second)+1)));
        
            //sort a list of two elements
            std::sort(row_indices.begin(), row_indices.end());
            row_indices.clear();
        }
    }


    //We're done building one of the matrices, except we haven't stored the grade information that RIVET requires.
    //TODO: Must add this in by modifying the relevant function below.
    
    
    if (verbosity >= 6)
        debug() << "Created boundary matrix";

    if (verbosity >= 6) {
        debug() << "Created FIRep";
    }
    if (verbosity >= 10) {
        print();
    }

    if (verbosity >= 6)
    {
        if (!std::is_sorted(indexes_0.begin(), indexes_0.end()))
        {
            debug() << "ERROR: Low indexes not sorted.";
        }
        if (!std::is_sorted(indexes_1.begin(), indexes_1.end()))
        {
            debug() << "ERROR: High indexes not sorted.";
        }
    }
}

//This constructor is used when the FIRep is given directly as text input.  To understand this part of code, it may be helpful to review the conventions for the FIRep input format, as explained in the RIVET documentaiton.
FIRep::FIRep(BifiltrationData& bd, int t, int s, int r, std::vector<std::vector<unsigned> >& d2, std::vector<std::vector<unsigned> >& d1, const std::vector<unsigned> x_values, const std::vector<unsigned> y_values, int v)
            : verbosity(v), x_grades(bd.num_x_grades()), y_grades(bd.num_y_grades()), bifiltration_data(bd)

{
    //TODO: Instead of using separate x_value and y_value vectors, it would be more efficient to use a vector of pairs.
    //Then we could just store pointers to these grades instead of copying them.
    vector<std::pair<Grade,unsigned>> mid_indexes;
    Grade temp_grade;
    for (int i = 0; i < s; i++)
    {
        temp_grade = Grade(x_values[i + t], y_values[i + t]);
        mid_indexes.push_back(std::pair<Grade,unsigned>(temp_grade,i));
    }
    
    std::sort(mid_indexes.begin(), mid_indexes.end(),[](const auto& left,const auto& right) {return left.first < right.first;});
    std::vector<unsigned> inverse_map(s);
    for (int i = 0; i < s; i++) {
        inverse_map[mid_indexes[i].second] = i;
    }

    boundary_mx_0 = new MapMatrix(r, s);
    for (int i = 0; i < s; i++)
    {
        write_boundary_column(boundary_mx_0, d1[mid_indexes[i].second], i);
        indexes_0.push_back(mid_indexes[i].first);
    }

    
    vector<std::pair<Grade,unsigned>> high_indexes;
    for (int i = 0; i < t; i++)
    {
        temp_grade = Grade(x_values[i], y_values[i]);
        high_indexes.push_back(std::pair<Grade,unsigned>(temp_grade,i));
    }
    
    std::sort(high_indexes.begin(), high_indexes.end(),[](const auto& left,const auto& right) {return left.first < right.first;});

    boundary_mx_1 = new MapMatrix(s, t);
    for (int i = 0; i < t; i++)
    {
        std::vector<unsigned> entries = d2[high_indexes[i].second];
        for (unsigned j = 0; j < entries.size(); j++)
        {
            entries[j] = inverse_map[entries[j]];
        }
        write_boundary_column(boundary_mx_1, entries, i);
        indexes_1.push_back(high_indexes[i].first);
    }

    if (verbosity >= 8)
        debug() << "Created FIRep";

    if (verbosity >= 10) {
        print();
    }

    if (verbosity >= 6)
    {
        if (!std::is_sorted(indexes_0.begin(), indexes_0.end()))
        {
            debug() << "Low indexes not sorted.";
        }
        if (!std::is_sorted(indexes_1.begin(), indexes_1.end()))
        {
            debug() << "High indexes not sorted.";
        }
    }
}

//destructor
FIRep::~FIRep()
{
    delete(boundary_mx_0);
    delete(boundary_mx_1);
}

//returns a matrix of boundary information for simplices of dimension dim (with multi-grade info)
//columns ordered according to dimension index (reverse-lexicographic order with respect to multi-grades)
MapMatrix* FIRep::get_low_boundary_mx()
{
    MapMatrix* src = boundary_mx_0;

    //create the MapMatrix
    MapMatrix* mat = new MapMatrix(src->height(), src->width()); //DELETE this object later!

    //Copy boundary matrix to new matrix
    mat->copy_cols_same_indexes(src, 0, src->width() - 1);
    return mat;
} //end get_low_boundary_mx()

//returns a matrix of boundary information for simplices of dimension dim+1 (with multi-grade info)
//columns ordered according to dimension index (reverse-lexicographic order with respect to multi-grades)
MapMatrix* FIRep::get_high_boundary_mx()
{
    MapMatrix* src = boundary_mx_1;

    //create the MapMatrix
    MapMatrix* mat = new MapMatrix(src->height(), src->width()); //DELETE this object later!

    //Copy boundary matrix to new matrix
    mat->copy_cols_same_indexes(src, 0, src->width() - 1);
    return mat;
} //end get_low_boundary_mx()

//returns a boundary matrix for hom_dim-simplices with columns in a specified order -- for vineyard-update algorithm
//    simplex_order is a map : dim_index --> order_index for simplices of the given dimension
//        if simplex_order[i] == -1, then simplex with dim_index i is NOT represented in the boundary matrix
//    num_simplices is the number of simplices in the order (i.e., the number of entries in the vector that are NOT -1)
MapMatrix_Perm* FIRep::get_boundary_mx(std::vector<int>& coface_order, unsigned num_simplices)
{
    //create the matrix
    MapMatrix_Perm* mat = new MapMatrix_Perm(boundary_mx_0->height(), num_simplices);

    //loop through all simplices, writing columns to the matrix
    for (unsigned i = 0; i < boundary_mx_0->width(); i++)
    {
        int order_index = coface_order[i]; //index of the matrix column which will store the boundary of this simplex
        if (order_index != -1) {
            mat->copy_cols_from(boundary_mx_0, i, order_index);
        }
    }

    //return the matrix
    return mat;
} //end get_boundary_mx(int, vector<int>)

//returns a boundary matrix for (hom_dim+1)-simplices with columns and rows a specified orders -- for vineyard-update algorithm
//  PARAMETERS:
//    each vector is a map : dim_index --> order_index for simplices of the given dimension
//        if order[i] == -1, then simplex with dim_index i is NOT represented in the boundary matrix
//    each unsigned is the number of simplices in the corresponding order (i.e., the number of entries in the vector that are NOT -1)
MapMatrix_Perm* FIRep::get_boundary_mx(std::vector<int>& face_order, unsigned num_faces, std::vector<int>& coface_order, unsigned num_cofaces)
{
    //create the matrix
    MapMatrix_Perm* mat = new MapMatrix_Perm(num_faces, num_cofaces);

    for (unsigned i = 0; i < boundary_mx_1->width(); i++)
    {
        int order_index = coface_order[i]; //index of the matrix column which will store the boundary of this simplex
        if (order_index != -1) {
            MapMatrix::MapMatrixNode* node = boundary_mx_1->columns[i];
            while (node != NULL)
            {
                mat->set(face_order[node->get_row()], order_index);
                node = node->get_next();
            }
        }
    }

    //return the matrix
    return mat;
} //end get_boundary_mx(int, vector<int>, vector<int>)


//writes boundary information given boundary entries in column col of matrix mat
void FIRep::write_boundary_column(MapMatrix* mat, std::vector<unsigned>& entries, unsigned col)
{
    for (unsigned k = 0; k < entries.size(); k++) {
        //for this boundary simplex, enter "1" in the appropriate cell in the matrix
        mat->set(entries[k], col);
    }
} //end write_boundary_column();

//returns a matrix of column indexes to accompany MapMatrices
//  entry (i,j) gives the last column of the MapMatrix that corresponds to multigrade (i,j)
IndexMatrix* FIRep::get_low_index_mx()
{
    return get_index_mx(indexes_0);
}

//returns a matrix of column indexes to accompany MapMatrices
//  entry (i,j) gives the last column of the MapMatrix that corresponds to multigrade (i,j)
IndexMatrix* FIRep::get_high_index_mx()
{
    return get_index_mx(indexes_1);
}

//returns a matrix of column indexes to accompany MapMatrices
//  entry (i,j) gives the last column of the MapMatrix that corresponds to multigrade (i,j)
IndexMatrix* FIRep::get_index_mx(AppearanceGrades& grades)
{
    //create the IndexMatrix
    unsigned x_size = x_grades;
    unsigned y_size = y_grades;
    IndexMatrix* mat = new IndexMatrix(y_size, x_size); //DELETE this object later!

    if (!grades.empty()) //then there is at least one simplex
    {
        //initialize to -1 the entries of end_col matrix before multigrade of the first simplex
        unsigned cur_entry = 0; //tracks previously updated multigrade in end-cols

        unsigned cur_grade = 0;
        for (; cur_entry < grades[0].x + grades[0].y * x_size; cur_entry++)
            mat->set(cur_entry / x_size, cur_entry % x_size, -1);

        //loop through simplices
        for (; cur_grade < grades.size(); cur_grade++) {
            int cur_x = grades[cur_grade].x;
            int cur_y = grades[cur_grade].y;

            //if some multigrades were skipped, store previous column number in skipped cells of end_col matrix
            for (; cur_entry < cur_x + cur_y * x_size; cur_entry++)
                mat->set(cur_entry / x_size, cur_entry % x_size, cur_grade - 1);

            //store current column number in cell of end_col matrix for this multigrade
            mat->set(cur_y, cur_x, cur_grade);
        }

        //store final column number for any cells in end_cols not yet updated
        for (; cur_entry < x_size * y_size; cur_entry++)
            mat->set(cur_entry / x_size, cur_entry % x_size, cur_grade - 1);
    } else //then there are no simplices, so fill the IndexMatrix with -1 values
    {
        for (unsigned i = 0; i < x_size * y_size; i++)
            mat->set(i / x_size, i % x_size, -1);
    }

    //return the matrix
    return mat;
} //end get_index_mx()

//returns the number of unique x-coordinates of the multi-grades
unsigned FIRep::num_x_grades()
{
    return x_grades;
}

//returns the number of unique y-coordinates of the multi-grades
unsigned FIRep::num_y_grades()
{
    return y_grades;
}

//Print the matrices and appearance grades
void FIRep::print()
{
    boundary_mx_0->print();
    get_low_index_mx()->print();
    boundary_mx_1->print();
    get_high_index_mx()->print();
}
