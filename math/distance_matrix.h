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

#ifndef _DISTANCE_MATRIX_H
#define _DISTANCE_MATRIX_H

#include "../interface/data_reader.h"
#include "../interface/file_input_reader.h"
#include "../interface/input_parameters.h"

#include <fstream>
#include <vector>

class DistanceMatrix {
public:
    DistanceMatrix(InputParameters& params, int np);
    ~DistanceMatrix(); // cleanup all data structures used in distance matrix

    void build_distance_matrix(std::vector<DataPoint>& points); // builds a distance matrix from a set of points

    void build_all_vectors(InputData* data); // builds discrete index vectors and grade vectors

    void read_distance_matrix(std::vector<exact>& values); // reads a matrix from an input file and stores it

    void ball_density_estimator(double radius); // function to add a density estimator to provided data

    // these are required by bifiltration data and hence made public
    std::vector<unsigned> dist_indexes;
    std::vector<unsigned> function_indexes;
    std::vector<unsigned> degree_indexes;

    unsigned* degree;

private:
    InputParameters& input_params;

    // some commonly used variables in the class
    // they are set from the input parameters
    bool function;
    unsigned num_points;
    exact max_dist;
    unsigned max_degree;
    unsigned dimension;
    std::string filtration;
    std::string func_type;

    unsigned max_unsigned; // infinity

    // sets storing unique values
    ExactSet dist_set;
    ExactSet degree_set;
    ExactSet function_set;

    std::pair<ExactSet::iterator, bool> ret; // for return value upon inserting to a set

    exact approx(double x);
    void build_grade_vectors(InputData& data, ExactSet& value_set, std::vector<unsigned>& indexes, std::vector<exact>& grades_exact, unsigned num_bins); //converts an ExactSets of values to the vectors of discrete values that BifiltrationData uses to build the bifiltration, and also builds the grade vectors (floating-point and exact)
};

#endif