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
 * \class   SimplexTree
 * \brief   Stores the dimension d, d + 1, and d - 1 homological data for a bifiltered simplicial complex
 * \author  Roy Zhao
 * \date    March 2017
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
    //structure which represents a generalized simplex whose information will be stored in boundary matrices
    struct Generator
    {
        int x;
        int y;
        Grade* grade; //reference to grade that this generator represents, if applicable
        std::vector<unsigned> boundary; //Boundary should be in sorted order

        Generator(Grade* set_grade) : x(set_grade->x), y(set_grade->y), grade(set_grade), boundary(std::vector<unsigned>()) {}

        Generator(int set_x, int set_y) : x(set_x), y(set_y), grade(NULL), boundary(std::vector<unsigned>()) {}

        bool operator<(Generator other) const
        {
            if (y != other.y)
                return y < other.y;
            else if (x != other.x)
                return x < other.x;
            else {
                unsigned currIndex = 0;
                unsigned otherIndex = 0;
                while (currIndex < boundary.size() && otherIndex < other.boundary.size()) {
                    if (boundary[currIndex] != other.boundary[otherIndex])
                    {
                        return boundary[currIndex] < other.boundary[otherIndex];
                    }
                    currIndex++;
                    otherIndex++;
                }
                if (otherIndex == other.boundary.size())
                {
                    return false;
                }
                return true;
            }
        }
    };

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
    void set_boundary_vector(Generator& generator, const std::vector<int>& vertices, SimplexInfo* low_simplices);

    void write_boundary_column(MapMatrix* mat, std::vector<unsigned>& entries, unsigned col); //writes boundary information given boundary entries in column col of matrix mat

    IndexMatrix* get_index_mx(AppearanceGrades& source_grades); //Gets the index matrix associated with a list of grades
};

#endif // __SimplexTree_H__
