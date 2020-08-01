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
#include "../debug.h"
#include "../numerics.h"

#include <math.h>
#include <string>

DistanceMatrix::DistanceMatrix(InputParameters& params, int np)
    : input_params(params)
    , max_dist(input_params.max_dist)
    , num_points(np)
    , dimension(input_params.dimension)
    , filtration(input_params.bifil)
    , func_type(input_params.function_type)
{

    // we make the degree filtration only when
    // we are building a degree-Rips
    if (filtration == "degree") {
        degree = new unsigned[num_points]();
    }

    max_unsigned = std::numeric_limits<unsigned>::max();
}

DistanceMatrix::~DistanceMatrix()
{
    // guaranteed way to clear memory allocated to vectors
    std::vector<unsigned>().swap(dist_indexes);
    std::vector<unsigned>().swap(function_indexes);
    std::vector<unsigned>().swap(degree_indexes);

    std::set<ExactValue, ExactValueComparator>().swap(dist_set);
    std::set<ExactValue, ExactValueComparator>().swap(function_set);
    std::set<ExactValue, ExactValueComparator>().swap(degree_set);

    if (filtration == "degree")
        delete degree;
}

void DistanceMatrix::ball_density_estimator(double radius)
{
    // first we reconstruct the actual distance matrix from the distance set
    // it is a diagonal matrix
    unsigned size = (num_points * (num_points - 1)) / 2 + 1;
    double* distance_matrix = new double[size];
    double* gvalues = new double[num_points];
    double total = 0;

    ExactSet::iterator it;

    // look at ExactValue struct to understand better
    for (it = dist_set.begin(); it != dist_set.end(); it++) {
        ExactValue curr_dist = *it;
        double val = curr_dist.double_value;
        for (unsigned j = 0; j < curr_dist.indexes.size(); j++) {
            unsigned index = curr_dist.indexes[j];
            distance_matrix[index] = val;
        }
    }

    // set a default radius if no radius parameter is supplied
    if (radius == 0) {
        std::vector<double> sorted; // sorted will hold sorted values of distance matrix
        for (unsigned i = 0; i < size; i++)
            sorted.push_back(distance_matrix[i]);
        std::sort(sorted.begin(), sorted.end());
        int twenty_percentile = (int)(sorted.size() * 0.2); // we select the 20th percentile
        radius = sorted[twenty_percentile];
        std::vector<double>().swap(sorted); // free up the memory
    }

    // check if distance less than supplied radius
    // if yes, increase density value by 1
    for (unsigned i = 0; i < num_points; i++) {
        double d = 0;
        for (unsigned j = 0; j < num_points; j++) {
            if ((i == j)
                || (j > i && (distance_matrix[(j * (j - 1)) / 2 + i + 1] <= radius))
                || (i > j && (distance_matrix[(i * (i - 1)) / 2 + j + 1] <= radius))) {
                d = d + 1;
            }
        }
        gvalues[i] = d;
        total += d;
    }

    for (unsigned i = 0; i < num_points; i++) {
        gvalues[i] /= total;
        exact value = gvalues[i];
        if (input_params.x_reverse)
            value = -1*value;
        ret = function_set.insert(ExactValue(value));
        (ret.first)->indexes.push_back(i);
    }

    delete[] gvalues;
    delete[] distance_matrix; // free up the memory
}

void DistanceMatrix::gaussian_estimator(double s)
{
    // first we reconstruct the actual distance matrix from the distance set
    // it is a diagonal matrix
    unsigned size = (num_points * (num_points - 1)) / 2 + 1;
    double* distance_matrix = new double[size];
    double* gvalues = new double[num_points];
    double total = 0;

    ExactSet::iterator it;

    // look at ExactValue struct to understand better
    for (it = dist_set.begin(); it != dist_set.end(); it++) {
        ExactValue curr_dist = *it;
        double val = curr_dist.double_value;
        for (unsigned j = 0; j < curr_dist.indexes.size(); j++) {
            unsigned index = curr_dist.indexes[j];
            distance_matrix[index] = val;
        }
    }

    if (s == 0)
        s = 1.0;

    for (unsigned i = 0; i < num_points; i++) {
        double d = 0;
        for (unsigned j = 0; j < num_points; j++) {
            if (i > j)
                d += exp(-1*pow(distance_matrix[(i * (i - 1)) / 2 + j + 1], 2)/s);
            if (j > i)
                d += exp(-1*pow(distance_matrix[(j * (j - 1)) / 2 + i + 1], 2)/s);
            if (i == j)
                d += 1;
        }
        gvalues[i] = d;
        total += d;
    }

    for (unsigned i = 0; i < num_points; i++) {
        gvalues[i] /= total;
        exact value = gvalues[i];
        if (input_params.x_reverse)
            value = -1*value;
        ret = function_set.insert(ExactValue(value));
        (ret.first)->indexes.push_back(i);
    }

    delete[] gvalues;
    delete[] distance_matrix; // free up the memory
}

void DistanceMatrix::knn_density_estimator(int k)
{
    // first we reconstruct the actual distance matrix from the distance set
    // it is a diagonal matrix
    unsigned size = (num_points * (num_points - 1)) / 2 + 1;
    double* distance_matrix = new double[size];

    ExactSet::iterator it;

    // look at ExactValue struct to understand better
    for (it = dist_set.begin(); it != dist_set.end(); it++) {
        ExactValue curr_dist = *it;
        double val = curr_dist.double_value;
        for (unsigned j = 0; j < curr_dist.indexes.size(); j++) {
            unsigned index = curr_dist.indexes[j];
            distance_matrix[index] = val;
        }
    }

    if (k == 0)
        k = 1; // set default
    if (k > num_points - 1)
        k = num_points - 1; // return farthest neighbor if out of bounds

    // store all distance values of a point in a vector
    // sort it
    // select kth value
    for (unsigned i = 0; i < num_points; i++) {
        exact value;
        std::vector<double> d;
        for (unsigned j = 0; j < num_points; j++) {
            if (i > j)
                d.push_back(distance_matrix[(i * (i - 1)) / 2 + j + 1]);
            if (j > i)
                d.push_back(distance_matrix[(j * (j - 1)) / 2 + i + 1]);
        }
        std::sort(d.begin(), d.end());
        value = d[k - 1];


        if (input_params.x_reverse)
            value = -1*value;
        ret = function_set.insert(ExactValue(value));
        (ret.first)->indexes.push_back(i);

        std::vector<double>().swap(d); // free up space used by vector
    }

    delete[] distance_matrix; // free up the memory
}

void DistanceMatrix::eccentricity_estimator(int p)
{
    // first we reconstruct the actual distance matrix from the distance set
    // it is a diagonal matrix
    unsigned size = (num_points * (num_points - 1)) / 2 + 1;
    double* distance_matrix = new double[size];

    ExactSet::iterator it;

    // look at ExactValue struct to understand better
    for (it = dist_set.begin(); it != dist_set.end(); it++) {
        ExactValue curr_dist = *it;
        double val = curr_dist.double_value;
        for (unsigned j = 0; j < curr_dist.indexes.size(); j++) {
            unsigned index = curr_dist.indexes[j];
            distance_matrix[index] = val;
        }
    }

    if (p == 0)
        p = 1; // set default

    // take a distance for a point
    // raise it to pth power and keep adding
    // divide by num_points and take 1/p power
    for (unsigned i = 0; i < num_points; i++) {
        exact value;
        double d = 0;
        for (unsigned j = 0; j < num_points; j++) {
            if (i > j)
                d += pow(distance_matrix[(i * (i - 1)) / 2 + j + 1], p);
            if (j > i)
                d += pow(distance_matrix[(j * (j - 1)) / 2 + i + 1], p);
        }
        d = d / num_points;
        d = pow(d, 1.0 / p);
        value = d;

        if (input_params.x_reverse)
            value = -1*value;
        ret = function_set.insert(ExactValue(value));
        (ret.first)->indexes.push_back(i);
    }

    delete[] distance_matrix; // free up the memory
}

void DistanceMatrix::build_distance_matrix(std::vector<DataPoint>& points)
{
    ret = dist_set.insert(ExactValue(exact(0))); //distance from a point to itself is always zero
    (ret.first)->indexes.push_back(0); //store distance 0 at 0th index
    //consider all points
    for (unsigned i = 0; i < num_points; i++) {
        if (func_type == "user" && filtration == "function") {
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

            //store distance value, if it doesn't exist already
            ret = dist_set.insert(ExactValue(cur_dist));

            //remember that the pair of points (i,j) has this distance value, which will go in entry j(j-1)/2 + i + 1
            (ret.first)->indexes.push_back((j * (j - 1)) / 2 + i + 1);

            //need to keep track of degree for degree-Rips complex
            if ((max_dist == -1 || cur_dist <= max_dist) && filtration == "degree") {
                //there is an edge between i and j so update degree
                degree[i]++;
                degree[j]++;
            }
        }
    } //end for
}

void DistanceMatrix::build_all_vectors(InputData* data)
{
    // if a function has to be calculated
    if (filtration == "function") {
        if (func_type == "balldensity")
            ball_density_estimator(input_params.filter_param);
        if (func_type == "knndensity")
            knn_density_estimator(input_params.filter_param);
        if (func_type == "eccentricity")
            eccentricity_estimator(input_params.filter_param);
        if (func_type == "gaussian")
            gaussian_estimator(input_params.filter_param);
    }

    // remove all values from distance set that are greater than max_dist
    // dist_set has values in ascending order
    if (max_dist != -1) {
        ExactSet::iterator it;
        for (it = dist_set.begin(); it != dist_set.end(); it++) {
            if (it->double_value > max_dist) {
                break;
            }
        }
        dist_set.erase(it, dist_set.end());
    }

    if (filtration == "degree") {
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

void DistanceMatrix::read_distance_matrix(std::vector<exact>& values)
{
    std::ifstream input_file(input_params.fileName);
    FileInputReader reader(input_file);

    for (int i = 0; i < input_params.to_skip; i++)
        reader.next_line(0);

    std::pair<std::vector<std::string>, unsigned> line_info;
    unsigned expectedNumTokens = num_points;

    ret = dist_set.insert(ExactValue(exact(0))); //distance from a point to itself is always zero
    //store distance 0 at index 0
    (ret.first)->indexes.push_back(0);

    //consider all points
    for (unsigned i = 0; i < num_points; i++) {
        if (func_type == "user" && filtration == "function") {
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

                    //store distance value, if it doesn't exist already
                    ret = dist_set.insert(ExactValue(cur_dist));

                    //remember that the pair of points (i,j) has this distance value, which will go in entry j(j-1)/2 + i + 1
                    (ret.first)->indexes.push_back((j * (j - 1)) / 2 + i + 1);

                    //need to keep track of degree for degree-Rips complex
                    if ((max_dist == -1 || cur_dist <= max_dist) && filtration == "degree") {
                        //there is an edge between i and j so update degree
                        degree[i]++;
                        degree[j]++;
                    }
                }
            } catch (std::exception& e) {
                throw InputError(line_info.second, e.what());
            }
        }
    } //end for

    input_file.close();
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