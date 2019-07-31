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

#include "distance_matrix.h"
#include "../numerics.h"
#include "../debug.h"

#include <math.h>
#include <string>

DistanceMatrix::DistanceMatrix(InputParameters& params, int np)
	: input_params(params)
	, max_dist(input_params.max_dist)
	, num_points(np)
	, dimension(input_params.dimension)
{
	function = input_params.old_function || input_params.new_function;
	if (!function) {
        degree = new unsigned[num_points]();
    }

    max_unsigned = std::numeric_limits<unsigned>::max();
}

void DistanceMatrix::build_distance_matrix(std::vector<DataPoint>& points)
{
	ret = dist_set.insert(ExactValue(exact(0))); //distance from a point to itself is always zero
    (ret.first)->indexes.push_back(0); //store distance 0 at 0th index
    //consider all points
    for (unsigned i = 0; i < num_points; i++) {
        if (function) {
            //store time value, if it doesn't exist already
            ret = function_set.insert(ExactValue(points[i].birth));

            //remember that point i has this birth time value
            (ret.first)->indexes.push_back(i);
        }

        //compute (approximate) distances from this point to all following points
        for (unsigned j = i + 1; j < num_points; j++) {
            //compute (approximate) distance squared between points[i] and points[j]
            double fp_dist_squared = 0;
            for (unsigned k = 0; k < dimension; k++) {
                double kth_dist = points[i].coords[k] - points[j].coords[k];
                fp_dist_squared += (kth_dist * kth_dist);
            }

            //find an approximate square root of fp_dist_squared, and store it as an exact value
            exact cur_dist(0);
            if (fp_dist_squared > 0)
                cur_dist = approx(sqrt(fp_dist_squared)); //OK for now...

            if (max_dist == -1 || cur_dist <= max_dist) //then this distance is allowed
            {
                //store distance value, if it doesn't exist already
                ret = dist_set.insert(ExactValue(cur_dist));

                //remember that the pair of points (i,j) has this distance value, which will go in entry j(j-1)/2 + i + 1
                (ret.first)->indexes.push_back((j * (j - 1)) / 2 + i + 1);

                //need to keep track of degree for degree-Rips complex
                if (!function) {
                    //there is an edge between i and j so update degree
                    degree[i]++;
                    degree[j]++;
                }
            }
        }
    } //end for
}

void DistanceMatrix::build_all_vectors(InputData* data)
{
	if (!function) {
        //determine the max degree
        max_degree = 0;
        for (unsigned i = 0; i < num_points; i++) {
            if (max_degree < degree[i])
                max_degree = degree[i];
        }

        //build vector of discrete degree indices from 0 to max_degree and bins those degree values
        //WARNING: assumes that the number of distinct degree grades will be equal to max_degree which may not hold
        for (unsigned i = 0; i <= max_degree; i++) {
            ret = degree_set.insert(ExactValue(max_degree - i)); //store degree -i because degree is wrt opposite ordering on R
            (ret.first)->indexes.push_back(i); //degree i is stored at index i
        }
        //make degrees
        degree_indexes = std::vector<unsigned>(max_degree + 1, 0);
        build_grade_vectors(*data, degree_set, degree_indexes, data->x_exact, input_params.x_bins);
    }
    //X axis is given by function in Vietoris-Rips complex
    else {
        //vector of discrete time indexes for each point; max_unsigned shall represent undefined time (is this reasonable?)
        function_indexes = std::vector<unsigned>(num_points, max_unsigned);
        build_grade_vectors(*data, function_set, function_indexes, data->x_exact, input_params.x_bins);
    }

    //second, distances
    dist_indexes = std::vector<unsigned>((num_points * (num_points - 1)) / 2 + 1, max_unsigned); //discrete distance matrix (triangle); max_unsigned shall represent undefined distance
    build_grade_vectors(*data, dist_set, dist_indexes, data->y_exact, input_params.y_bins);
}

void DistanceMatrix::read_distance_matrix(std::ifstream& stream, std::vector<exact>& values)
{
	FileInputReader reader(stream);

	std::pair<std::vector<std::string>, unsigned> line_info;
	unsigned expectedNumTokens = num_points;

	ret = dist_set.insert(ExactValue(exact(0))); //distance from a point to itself is always zero
    //store distance 0 at index 0
    (ret.first)->indexes.push_back(0);

    //consider all points
    for (unsigned i = 0; i < num_points; i++) {
        if (function) {
            //store value, if it doesn't exist already
            ret = function_set.insert(ExactValue(values[i]));

            //remember that point i has this value
            (ret.first)->indexes.push_back(i);
        }

        // TODO: Add error more error handling?

        //read distances from this point to all following points
        if (i < num_points - 1) //then there is at least one point after point i, and there should be another line to read
        {
            if (reader.has_next_line())
                line_info = reader.next_line(0);

            std::vector<std::string> tokens;

            // skip lower triangle if full distance matrix is supplied
            if (line_info.first.size() == num_points && num_points == expectedNumTokens) {
                for (unsigned k = i + 1; k < num_points; k++)
                    tokens.push_back(line_info.first[k]);
            } else if (line_info.first.size() == expectedNumTokens - 1) {
                for (unsigned k = 0; k < expectedNumTokens - 1; k++)
                    tokens.push_back(line_info.first[k]);
                expectedNumTokens--;
            } else {
                throw std::runtime_error("Expected " + std::to_string(expectedNumTokens - 1) + " numbers, got " + std::to_string(line_info.first.size()) + " at line " + std::to_string(line_info.second));
            }

            try {
                for (unsigned j = i + 1; j < num_points; j++) {

                    std::string str = tokens[j - i - 1];

                    exact cur_dist = str_to_exact(str);

                    if (max_dist == -1 || cur_dist <= max_dist) //then this distance is allowed
                    {
                        //store distance value, if it doesn't exist already
                        ret = dist_set.insert(ExactValue(cur_dist));

                        //remember that the pair of points (i,j) has this distance value, which will go in entry j(j-1)/2 + i + 1
                        (ret.first)->indexes.push_back((j * (j - 1)) / 2 + i + 1);

                        //need to keep track of degree for degree-Rips complex
                        if (!function) {
                            //there is an edge between i and j so update degree
                            degree[i]++;
                            degree[j]++;
                        }
                    }
                }
            } catch (std::exception& e) {
                throw InputError(line_info.second, e.what());
            }
        }
    } //end for

}

void DistanceMatrix::build_grade_vectors(InputData& data,
    ExactSet& value_set,
    std::vector<unsigned>& discrete_indexes,
    std::vector<exact>& grades_exact,
    unsigned num_bins)
{
    if (num_bins == 0 || num_bins >= value_set.size()) //then don't use bins
    {

        grades_exact.reserve(value_set.size());

        unsigned c = 0; //counter for indexes
        for (ExactSet::iterator it = value_set.begin(); it != value_set.end(); ++it) //loop through all UNIQUE values
        {
            grades_exact.push_back(it->exact_value);

            for (unsigned i = 0; i < it->indexes.size(); i++) //loop through all point indexes for this value
                discrete_indexes[it->indexes[i]] = c; //store discrete index

            c++;
        }
    } else //then use bins: then the number of discrete indexes will equal
    // the number of bins, and exact values will be equally spaced
    {
        //compute bin size
        exact min = value_set.begin()->exact_value;
        exact max = value_set.rbegin()->exact_value;
        exact bin_size = (max - min) / num_bins;

        //store bin values
        data.x_exact.reserve(num_bins);

        ExactSet::iterator it = value_set.begin();
        for (unsigned c = 0; c < num_bins; c++) //loop through all bins
        {
            ExactValue cur_bin(static_cast<exact>(min + (c + 1) * bin_size)); //store the bin value (i.e. the right endpoint of the bin interval)
            grades_exact.push_back(cur_bin.exact_value);

            //store bin index for all points whose time value is in this bin
            while (it != value_set.end() && *it <= cur_bin) {
                for (unsigned i = 0; i < it->indexes.size(); i++) //loop through all point indexes for this value
                    discrete_indexes[it->indexes[i]] = c; //store discrete index
                ++it;
            }
        }
    }
} //end build_grade_vectors()

exact DistanceMatrix::approx(double x)
{
	int d = 7; //desired number of significant digits
    int log = (int)floor(log10(x)) + 1;

    if (log >= d)
        return exact((int)floor(x));

    long denom = pow(10, d - log);
    return exact((long)floor(x * denom), denom);
}