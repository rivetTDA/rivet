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

    FIRep(BifiltrationData& bd, int t, int s, int r, const std::vector<std::vector<unsigned> >& d2, const std::vector<std::vector<unsigned> >& d1,
            const std::vector<unsigned> x_values, const std::vector<unsigned> y_values, int v); //constructor

    ~FIRep(); //destructor

    //returns a matrix of boundary information for simplices
    MapMatrix* get_boundary_mx(int dim); 

    //returns a boundary matrix for hom_dim-simplices with columns in a specified order -- for vineyard-update algorithm
    MapMatrix_Perm* get_boundary_mx(std::vector<int>& coface_order, unsigned num_simplices); 

    //returns a boundary matrix for (hom_dim+1)-simplices with columns and rows a specified orders -- for vineyard-update algorithm
    MapMatrix_Perm* get_boundary_mx(std::vector<int>& face_order, unsigned num_faces, std::vector<int>& coface_order, unsigned num_cofaces); 

    //returns a matrix of column indexes to accompany MapMatrices
    IndexMatrix* get_index_mx(int dim); 

    unsigned get_size(int dim); //returns the number of simplices of dimension (hom_dim-1), hom_dim, or (hom_dim+1)

    unsigned num_x_grades();                     //returns the number of unique x-coordinates of the multi-grades
    unsigned num_y_grades();                     //returns the number of unique y-coordinates of the multi-grades

    const int hom_dim;      //the dimension of homology to be computed; max dimension of simplices is one more than this
    const unsigned verbosity; //controls display of output, for debugging

private:
    unsigned x_grades;  //the number of x-grades that exist in this bifiltration
    unsigned y_grades;  //the number of y-grades that exist in this bifiltration
    MapMatrix* boundary_mx_0; //boundary matrix from dim to dim-1
    MapMatrix* boundary_mx_1; //boundary matrix from dim+1 to dim
    AppearanceGrades indexes_0; //indexes of simplices in dimension dim
    AppearanceGrades indexes_1; //indexes of simplices in dimension dim+1
    BifiltrationData& bifiltration_data;

    void write_boundary_column(MapMatrix* mat, const std::vector<int>& vertices, SimplexInfo* low_simplices, int col); //writes boundary information for simplex represented by sim in column col of matrix mat

    void write_boundary_column(MapMatrix* mat, const std::vector<unsigned>& entries, unsigned col); //writes boundary information given boundary entries in column col of matrix mat
};

#endif // __SimplexTree_H__
