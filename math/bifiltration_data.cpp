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

// Authors: Roy Zhao (March 2017), Michael Lesnick (modifications fall 2017).

#include "bifiltration_data.h"

#include "index_matrix.h"
#include "map_matrix.h"

#include "debug.h"

#include <algorithm> //for std::sort
#include <iostream> //for std::cout, for testing only
#include <limits> //std::numeric_limits
#include <stdexcept>

//BifiltrationData constructor; requires dimension of homology to be computed and verbosity parameter
BifiltrationData::BifiltrationData(unsigned dim, int verbosity)
    : hom_dim(dim)
    , verbosity(verbosity)
    , x_grades(0)
    , y_grades(0)
    , mid_count(0)
    , high_count(0)
{
    if (hom_dim > 5) {
        throw std::runtime_error("BifiltrationData: Dimensions greater than 5 probably don't make sense");
    }
    if (verbosity >= 8) {
        debug() << "Created BifiltrationData(" << hom_dim << ", " << verbosity << ")";
    }
}

/*
Adds a simplex to the BifiltrationData.  If the simplex does not have dimension 
hom_dim-1, hom_dim, or hom_dim+1, does nothing.
WARNING: Assumes but does not verify that multigrades are non-comparable
WARNING: Assumes that simplex has not already been added.  Does not check this.
WARNING: Assumes that simplex vertices are sorted (increasing order)
         and vector of grades of appearance is also sorted (colex order).
 */
void BifiltrationData::add_simplex(Simplex const& vertices, const AppearanceGrades& grades)
{
    if (vertices.size() == 0) {
        return;
    }

    else if (vertices.size() == hom_dim) //simplex of dimension hom_dim - 1
    {
        //For the homology computation,
        //we only need the greatest lower bound of grades
        low_simplices.push_back(LowSimplexData(vertices, Grade((grades.end() - 1)->x, grades.begin()->y)));
        return;
    } else if (vertices.size() == hom_dim + 1) //simplex of dimension hom_dim
    {
        mid_simplices.push_back(MidHighSimplexData(vertices, grades, false));
        mid_count++;
        return;

    } else if (vertices.size() == hom_dim + 2) //simplex of dimension hom_dim + 1
    {
        high_simplices.push_back(MidHighSimplexData(vertices, grades, true));
        high_count++;
        return;
    }
} //end add_faces()

void BifiltrationData::build_VR_complex(const std::vector<unsigned>& times,
    const std::vector<unsigned>& distances,
    const unsigned num_x,
    const unsigned num_y)
{
    x_grades = num_x;
    y_grades = num_y;

    //Add generation points recursively
    for (unsigned i = 0; i < times.size(); i++) {
        //recursion
        std::vector<int> vertices;
        vertices.push_back(i);

        build_VR_subcomplex(times, distances, vertices, times[i], 0);
    }
} //end build_VR_complex()

//function to add (recursively) a subcomplex of the bifiltration data
void BifiltrationData::build_VR_subcomplex(const std::vector<unsigned>& times,
    const std::vector<unsigned>& distances,
    std::vector<int>& vertices,
    const unsigned prev_time,
    const unsigned prev_dist)
{
    //Store the simplex info if it is of dimension (hom_dim - 1), hom_dim, or hom_dim+1. Dimension is vertices.size() - 1
    if (vertices.size() == hom_dim) //simplex of dimension hom_dim - 1
    {
        low_simplices.push_back(LowSimplexData(vertices, Grade(prev_time, prev_dist)));
    } else if (vertices.size() == hom_dim + 1) //simplex of dimension hom_dim - 1
    {
        mid_simplices.push_back(MidHighSimplexData(vertices, AppearanceGrades(1, Grade(prev_time, prev_dist)), false));
        mid_count++;
    } else if (vertices.size() == hom_dim + 2) //simplex of dimension hom_dim - 1
    {
        high_simplices.push_back(MidHighSimplexData(vertices, AppearanceGrades(1, Grade(prev_time, prev_dist)), true));
        high_count++;
        return;
    }

    //loop through all points that could be children of this node
    for (unsigned j = vertices.back() + 1; j < times.size(); j++) {
        //look up distances from point j to each of the vertices in the simplex
        //distance index is maximum of prev_distance and each of these distances
        unsigned current_dist = prev_dist;
        for (unsigned k = 0; k < vertices.size(); k++) {
            int p = vertices[k];
            unsigned d = distances[(j * (j - 1)) / 2 + p + 1]; //the distance between points p and j, with p < j
            if (d > current_dist)
                current_dist = d;
        }

        //see if the distance is permitted
        if (current_dist < std::numeric_limits<unsigned>::max()) //then we will add this larger simplex
        {
            //compute time index of this new node
            unsigned current_time = times[j];
            if (current_time < prev_time)
                current_time = prev_time;

            //recursion
            vertices.push_back(j); //next we will look for children of node j
            build_VR_subcomplex(times, distances, vertices, current_time, current_dist);
            vertices.pop_back(); //finished adding children of node j
        }
    }
} //end build_VR_subcomplex()

//TODO: will the degree vector have to be treated differently? I don't think so, because it is actually
//the vector of degree *indexes*

//builds BifiltrationData representing a bifiltered Vietoris-Rips complex from metric data
void BifiltrationData::build_DR_complex(const unsigned num_vertices, const std::vector<unsigned>& distances, const std::vector<unsigned>& degrees, const unsigned num_x, const unsigned num_y)
{
    x_grades = num_x;
    y_grades = num_y;

    //build complex recursively
    //this also assigns global indexes to each simplex
    if (verbosity >= 6) {
        debug() << "BUILDING DEGREE-RIPS COMPLEX";
    }

    std::vector<AppearanceGrades> vertex_multigrades;
    generate_vertex_multigrades(vertex_multigrades, num_vertices, distances, degrees);

    std::vector<int> simplex_indices;
    for (unsigned i = 0; i < num_vertices; i++) {
        //Look at simplex with smallest vertex vertex i
        simplex_indices.push_back(i);

        //Determine the neighbors of vertex i
        std::vector<int> candidates;
        for (unsigned j = i + 1; j < num_vertices; j++) //Dist of (i, j) with i < j stored in distances[j(j - 1)/2 + i]
        {
            if (distances[j * (j - 1) / 2 + i + 1] < std::numeric_limits<unsigned>::max()) //if an edge is between i and j
                candidates.push_back(j);
        }
        //recursion
        build_DR_subcomplex(distances, simplex_indices, candidates, vertex_multigrades[i], vertex_multigrades);
        simplex_indices.pop_back();
    }
} //end build_DR_complex()

//function to build (recursively) a subcomplex for the DRips complex
void BifiltrationData::build_DR_subcomplex(const std::vector<unsigned>& distances, std::vector<int>& parent_vertices, const std::vector<int>& candidates, const AppearanceGrades& parent_grades, const std::vector<AppearanceGrades>& vertex_multigrades)
{
    //Store the simplex info if it is of dimension (hom_dim - 1), hom_dim, or hom_dim+1. Dimension is parent_vertices.size() - 1
    if (parent_vertices.size() == hom_dim) //simplex of dimension hom_dim - 1
    {
        //take the greatest lower bound of parent_grades, using the fact that the grades are ordered properly.

        low_simplices.push_back(LowSimplexData(parent_vertices, Grade((parent_grades.end() - 1)->x, parent_grades.begin()->y)));
    } else if (parent_vertices.size() == hom_dim + 1) //simplex of dimension hom_dim
    {
        mid_simplices.push_back(MidHighSimplexData(parent_vertices, parent_grades, false));
        mid_count += parent_grades.size();
    } else if (parent_vertices.size() == hom_dim + 2) //simplex of dimension hom_dim + 1
    {
        high_simplices.push_back(MidHighSimplexData(parent_vertices, parent_grades, true));
        high_count += parent_grades.size();
        return;
    }

    //loop through all points that could be added to form a larger simplex (candidates)
    for (std::vector<int>::const_iterator it = candidates.begin(); it != candidates.end(); it++) {
        //Determine the grades of appearance of the clique with parent_vertices and *it
        //First determine the minimal scale parameter necessary for all the edges between the clique parent_vertices, and *it to appear
        unsigned min_dist = distances[0];
        for (std::vector<int>::const_iterator it2 = parent_vertices.begin(); it2 != parent_vertices.end(); it2++)
            if (distances[(*it) * (*it - 1) / 2 + *it2 + 1] > min_dist) //By construction, each of the parent indices are strictly less than *it
                min_dist = distances[(*it) * (*it - 1) / 2 + *it2 + 1];
        AppearanceGrades new_grades;
        combine_multigrades(new_grades, parent_grades, vertex_multigrades[*it], min_dist);

        //Determine subset of candidates which are still candidates after adding *it
        std::vector<int> new_candidates;
        for (std::vector<int>::const_iterator it2 = it + 1; it2 != candidates.end(); it2++) {
            if (distances[(*it2) * (*it2 - 1) / 2 + *it + 1] < std::numeric_limits<unsigned>::max()) //We know that *it2 > *it
            {
                new_candidates.push_back(*it2); //We knew there was connection between *it2 and all of parent_index, and now also to *it as well
            }
        }

        parent_vertices.push_back(*it);
        //recurse
        build_DR_subcomplex(distances, parent_vertices, new_candidates, new_grades, vertex_multigrades);
        parent_vertices.pop_back(); //Finished looking at cliques adding *it as well
    }

} //end build_DR_subcomplex()

//For each point in a degree-Rips bifiltration, generates an array of incomparable grades of appearance. distances should be of size vertices(vertices - 1)/2
//Degrees are stored in negative form to align with correct ordering on R
//Stores result in the vector container "multigrades". Each vector of grades is sorted in reverse lexicographic order

void BifiltrationData::generate_vertex_multigrades(std::vector<AppearanceGrades>& multigrades, const unsigned vertices, const std::vector<unsigned>& distances, const std::vector<unsigned>& degrees)
{
    for (unsigned i = 0; i < vertices; i++) {
        std::vector<unsigned> neighbor_dists; //Generate list of how far the neighbors are, vertex of the neighbor does not matter
        for (unsigned j = 0; j < vertices; j++) //Dist of (i, j) with i < j stored in distances[j(j - 1)/2 + i]
        {
            if (j < i && distances[i * (i - 1) / 2 + j + 1] < std::numeric_limits<unsigned>::max()) //If i and j are neighbors
            {
                neighbor_dists.push_back(distances[i * (i - 1) / 2 + j + 1]);
            } else if (j > i && distances[j * (j - 1) / 2 + i + 1] < std::numeric_limits<unsigned>::max()) {
                neighbor_dists.push_back(distances[j * (j - 1) / 2 + i + 1]);
            }
        }
        std::sort(neighbor_dists.begin(), neighbor_dists.end());
        AppearanceGrades i_grades; //Stores grades of appearance for vertex i
        unsigned min_scale;
        i_grades.push_back(Grade(degrees[0], distances[0])); //Every point has a grade of appearance at degree = 0, scale = 0
        for (unsigned j = 0; j < neighbor_dists.size();) {
            min_scale = neighbor_dists[j];
            while (j < neighbor_dists.size() && neighbor_dists[j] == min_scale)
                j++; //Iterate until the next distance is > minScale
            i_grades.push_back(Grade(degrees[j], min_scale)); //If the scale parameter is >= minScale, then vertex i has at least neighbor_dists.size() - (j + 1) neighbors.
        }
        update_grades(i_grades); //Makes sure all of them are incomparable after the binning
        multigrades.push_back(i_grades);
    }
} //end generate_vertex_multigrades()

//Determines the grades of appearance of when both simplices exist subject to some minimal distance parameter min_dist
//Grade arrays are assumed to be sorted in reverse lexicographic order, output will be sorted in reverse lexicographic order
//Takes the intersection of the grades of appearances and the half plane y >= min_dist
void BifiltrationData::combine_multigrades(AppearanceGrades& merged, const AppearanceGrades& grades1, const AppearanceGrades& grades2, const unsigned min_dist)
{
    AppearanceGrades::const_iterator it1 = grades1.begin();
    AppearanceGrades::const_iterator it2 = grades2.begin();
    int y1 = it1->y, y2 = it2->y, max_x;
    int curr_y_max = std::max(std::max(y1, y2), (int)min_dist);
    Grade last_grade;
    while (it1 != grades1.end() || it2 != grades2.end()) {
        max_x = std::numeric_limits<int>::min();
        //Consider the reverse lexicographically first point
        if (it1 != grades1.end()) {
            max_x = it1->x;
        }
        if (it2 != grades2.end() && max_x <= it2->x) {
            max_x = it2->x;
            it2++;
            if (it2 != grades2.end()) {
                y2 = it2->y;
            } else {
                y2 = std::numeric_limits<int>::max();
            }
        }
        if (it1 != grades1.end() && max_x == it1->x) {
            it1++;
            if (it1 != grades1.end()) {
                y1 = it1->y;
            } else {
                y1 = std::numeric_limits<int>::max();
            }
        }
        int new_y_max = std::max(std::max(y1, y2), (int)min_dist);
        if (new_y_max > curr_y_max) {
            last_grade.x = max_x;
            last_grade.y = curr_y_max;
            merged.push_back(last_grade);
            curr_y_max = new_y_max;
        }
    }
} //end combine_multigrades

//Given a list of multigrades, sort them and remove all comparable multigrades
void BifiltrationData::update_grades(AppearanceGrades& grades)
{
    //Sort grades
    std::sort(grades.begin(), grades.end());

    //Iterate through the sorted grades and make sure they are all incomparable, delete the ones that are not
    for (AppearanceGrades::iterator it = grades.begin(); it != grades.end() - 1;) {
        if (it->x <= (it + 1)->x && it->y <= (it + 1)->y) //grades are comparable
            it = grades.erase(it + 1) - 1;
        else
            it++;
    }
}

//sets x_grades and y_grades as necessary
void BifiltrationData::set_xy_grades(unsigned num_x, unsigned num_y)
{
    x_grades = num_x;
    y_grades = num_y;
}

//returns the number of unique x-coordinates of the multi-grades
unsigned BifiltrationData::num_x_grades()
{
    return x_grades;
}

//returns the number of unique y-coordinates of the multi-grades
unsigned BifiltrationData::num_y_grades()
{
    return y_grades;
}

//returns the number of simplices of dimension (hom_dim-1), hom_dim, or (hom_dim+1).
//Assumes dim is non-negative
int BifiltrationData::get_size(unsigned dim)
{
    if (hom_dim > 0 && dim == hom_dim - 1)
        return low_simplices.size();
    else if (dim == hom_dim)
        return mid_simplices.size();
    else if (dim == hom_dim + 1)
        return high_simplices.size();
    else
        return -1;
}

//print bifiltration in the RIVET bifiltration input format
//prints simplices in no particular order, grades are in reverse-lexicographic order
void BifiltrationData::print_bifiltration()
{

    //these three similar nested loops could be combined, but it's not such a big deal.
    for (std::vector<LowSimplexData>::const_iterator it = low_simplices.begin(); it != low_simplices.end(); it++) {
        for (Simplex::const_iterator it2 = it->s.begin(); it2 != it->s.end(); it2++) {
            std::cout << *it2 << " ";
        }
        std::cout << "; "; //Separator of vertex and grade info
        std::cout << it->gr.x << " " << it->gr.y << std::endl;
    }

    for (std::vector<MidHighSimplexData>::const_iterator it = mid_simplices.begin(); it != mid_simplices.end(); it++) {
        for (Simplex::const_iterator it2 = it->s.begin(); it2 != it->s.end(); it2++) {
            std::cout << *it2 << " ";
        }
        std::cout << "; "; //Separator of vertex and grade info
        for (AppearanceGrades::const_iterator it2 = it->grades_vec.begin(); it2 != it->grades_vec.end(); it2++) {
            std::cout << it2->x << " " << it2->y << " ";
        }
        std::cout << std::endl;
    }

    for (std::vector<MidHighSimplexData>::const_iterator it = high_simplices.begin(); it != high_simplices.end(); it++) {
        for (Simplex::const_iterator it2 = it->s.begin(); it2 != it->s.end(); it2++) {
            std::cout << *it2 << " ";
        }
        std::cout << "; "; //Separator of vertex and grade info
        for (AppearanceGrades::const_iterator it2 = it->grades_vec.begin(); it2 != it->grades_vec.end(); it2++) {
            std::cout << it2->x << " " << it2->y << " ";
        }
        std::cout << std::endl;
    }
}
