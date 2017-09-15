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
 * \brief   Computes and stores an FI-Rep of the dth homology module of a (possibly multicritical) bifiltered simplicial complex
 * \author  Roy Zhao; edited by Michael Lesnick.
 * \date    March 2017; edited September 2017.
 */

#ifndef __FIRep_H__
#define __FIRep_H__

//forward declarations
class IndexMatrix;
class MapMatrix;
class MapMatrix_Perm;

#include "bifiltration_data.h"

#include <set>
#include <string>
#include <vector>

//struct respresenting generalized simplex whose information will be stored in boundary matrices
//Each Gen_Simplex either corresponds to a simplex in the bifiltration, or to a relation
struct GenSimplex
{
    int x;
    int y;
    bool is_rel=false;
    Grade* grade; //reference to grade that this generator represents, if applicable

    
    //TODO: In the 1-cirtical case, many Gen_Simplexes may share the same "vertices" data
    //      Would it be better to keep this info elsewhere and store a pointer to this?
    //      Alternatively, we could store this as an integer using a combinatorial number system,
    //      as in DIPHA/Riper.
    
    std::vector<unsigned> vertices; //vertices are assumed to be in sorted order
    std::vector<unsigned> boundary; //Boundary should be in sorted order
    
    GenSimplex(Grade* set_grade) : x(set_grade->x), y(set_grade->y), grade(set_grade), boundary(std::vector<unsigned>()) {}
    
    GenSimplex(int set_x, int set_y) : x(set_x), y(set_y), grade(NULL), boundary(std::vector<unsigned>()) {}
    
    //Compares grades colexicographically.  Then, if grades are equal:
    //-simplices are lexicographically ordered by vertex index.  (In examples of Rips bifiltrations, this seems to work well.)
    //-a relation is less than a simplex.  (Because heuristically, we expect to see less fill-in if sparser columns come first, and relations have at most two entries.)
    //-relations are ordered so that the one whose pivot is smaller (i.e., higher up in the matrix) come first.  In a tie, order using the other element  (Heuristically, this makes it harder for pivots to "collide.")
    
    bool operator<(GenSimplex other) const
    {
        if (y != other.y)
            return y < other.y;
        else if (x != other.x)
            return x < other.x;
        else if (!is_rel && !other.is_rel) {
            //neither GenSimplex is a relation
            unsigned currIndex = 0;
            while (currIndex != vertices.size()) {
                if (vertices[currIndex] != other.vertices[currIndex])
                {
                    return vertices[currIndex] < other.vertices[currIndex];
                }
                currIndex++;
            }
            //In this case the the simplices are equal.
            return false;
        }
        else if (is_rel != other.is_rel)
            //exactly one GenSimplex is relation.
            return is_rel;
        else {
            //each GenSimplex is relation.
            
            //NOTE: We assume that the boundaries of the relations contains exactly two elements.
            if (boundary[1]!=other.boundary[1])
                return boundary[1]<other.boundary[1];
            else
                return boundary[0]<other.boundary[0];
        }
    }
};

class FIRep {
public:
    FIRep(BifiltrationData& bd, int v); //constructor; requires verbosity parameter
    
    FIRep(BifiltrationData& bd, int t, int s, int r, std::vector<std::vector<unsigned> >& d2, std::vector<std::vector<unsigned> >& d1,
          const std::vector<unsigned> x_values, const std::vector<unsigned> y_values, int v); //constructor
    
    ~FIRep(); //destructor
    
    //returns a matrix of boundary information for dim to dim-1
    MapMatrix* get_low_boundary_mx();
    
    //returns a matrix of boundary information for dim+1 to dim
    MapMatrix* get_high_boundary_mx();
    
    //returns a boundary matrix for hom_dim-simplices with columns in a specified order -- for vineyard-update algorithm
    MapMatrix_Perm* get_boundary_mx(std::vector<int>& coface_order, unsigned num_simplices);
    
    //returns a boundary matrix for (hom_dim+1)-simplices with columns and rows a specified orders -- for vineyard-update algorithm
    MapMatrix_Perm* get_boundary_mx(std::vector<int>& face_order, unsigned num_faces, std::vector<int>& coface_order, unsigned num_cofaces);
    
    //returns a matrix of column indexes to accompany MapMatrices
    IndexMatrix* get_low_index_mx();
    
    //returns a matrix of column indexes to accompany MapMatrices
    IndexMatrix* get_high_index_mx();
    
    unsigned num_x_grades();                     //returns the number of unique x-coordinates of the multi-grades
    unsigned num_y_grades();                     //returns the number of unique y-coordinates of the multi-grades
    
    void print(); //Print the matrices and appearance grades
    
    const unsigned verbosity; //controls display of output, for debugging
    
private:
    unsigned x_grades;  //the number of x-grades that exist in this bifiltration
    unsigned y_grades;  //the number of y-grades that exist in this bifiltration
    MapMatrix* boundary_mx_0; //boundary matrix from dim to dim-1
    MapMatrix* boundary_mx_1; //boundary matrix from dim+1 to dim
    
    //Indexes of simplices. Grades are stored in discrete indexes, real ExactValues are stored in InputData.x_exact and y_exact
    AppearanceGrades indexes_0; //indexes of simplices in dimension dim
    AppearanceGrades indexes_1; //indexes of simplices in dimension dim+1
    //TODO: Delete reference once Alex's code is merged
    BifiltrationData& bifiltration_data; //Associated bifiltration data is kept only for Alex's dendrogram code
    
    //writes boundary vector for simplex represented by sim in column col of matrix mat
    void set_boundary_vector(GenSimplex& generator, const std::vector<int>& vertices, SimplexInfo* low_simplices);
    
    void write_boundary_column(MapMatrix* mat, std::vector<unsigned>& entries, unsigned col); //writes boundary information given boundary entries in column col of matrix mat
    
    IndexMatrix* get_index_mx(AppearanceGrades& source_grades); //Gets the index matrix associated with a list of grades
};

#endif // __SimplexTree_H__
