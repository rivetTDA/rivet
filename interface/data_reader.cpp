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

#include "data_reader.h"
#include "../computation.h"
#include "../debug.h"
#include "../math/bifiltration_data.h"
#include "../math/distance_matrix.h"
#include "file_input_reader.h"
#include "input_parameters.h"

#include <algorithm>
#include <api.h>
#include <boost/algorithm/string.hpp>
#include <dcel/arrangement_message.h>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

#include <ctime>

//epsilon value for use in comparisons
double ExactValue::epsilon = pow(2, -30);

// FileContent stores the data
FileContent::FileContent()
{
    type = FileContentType::INVALID;
    input_data = nullptr;
    result = nullptr;
}

FileContent::FileContent(InputData* data)
{
    type = FileContentType::DATA;
    input_data = std::make_shared<InputData>(*data);
}

FileContent::FileContent(ComputationResult* result)
{
    type = FileContentType::PRECOMPUTED;
    FileContent::result = std::make_shared<ComputationResult>(*result);
}

FileContent& FileContent::operator=(const FileContent& other)
{
    type = other.type;
    input_data = other.input_data;
    result = other.result;
    return *this;
}

FileContent::FileContent(const FileContent& other)
{
    type = other.type;
    input_data = other.input_data;
    result = other.result;
}

//FileContent::~FileContent() {
//
//}

//==================== DataReader class ====================
using namespace rivet::numeric;

//constructor
DataReader::DataReader(InputParameters& params)
    : input_params(params)
{
    register_file_type(FileType{ "points", "point-cloud data", true,
        std::bind(&DataReader::read_point_cloud, this, std::placeholders::_1, std::placeholders::_2) });
    register_file_type(FileType{ "metric", "metric data", true,
        std::bind(&DataReader::read_discrete_metric_space, this, std::placeholders::_1, std::placeholders::_2) });
    register_file_type(FileType{ "bifiltration", "bifiltration data", true,
        std::bind(&DataReader::read_bifiltration, this, std::placeholders::_1, std::placeholders::_2) });
    register_file_type(FileType{ "firep", "free implicit representation data", true,
        std::bind(&DataReader::read_firep, this, std::placeholders::_1, std::placeholders::_2) });
    register_file_type(FileType{ "RIVET_msgpack", "pre-computed RIVET data", false,
        std::bind(&DataReader::read_messagepack, this, std::placeholders::_1, std::placeholders::_2) });
    //    register_file_type(FileType {"RIVET_0", "pre-computed RIVET data", false,
    //                                 std::bind(&DataReader::read_RIVET_data, this, std::placeholders::_1, std::placeholders::_2) });

    // determine the file type
    file_type = get_supported_type(input_params.type).second;
}

void DataReader::register_file_type(FileType file_type)
{
    supported_types.push_back(file_type);
}

// function to process the data in the file
//  post condition: x_grades and x_exact have size x_bins, and they contain the grade values for the 2-D persistence module in double and exact form (respectively)
//                  similarly for y_grades and y_exact
FileContent DataReader::process(Progress& progress)
{
    verbosity = input_params.verbosity;
    if (verbosity >= 2) {
        debug() << "READING FILE:" << input_params.fileName.c_str();
    }
    std::ifstream infile(input_params.fileName); //input file
    if (!infile.is_open()) {
        throw std::runtime_error("Could not open input file.");
    }
    // register_file_type() determines what function the parser is
    auto data = file_type.parser(infile, progress);
    return data;
}

FileContent DataReader::read_messagepack(std::ifstream& stream, Progress& progress)
{

    std::string type;
    std::getline(stream, type);
    InputParameters params;
    TemplatePointsMessage templatePointsMessage;
    ArrangementMessage arrangementMessage;
    std::string buffer((std::istreambuf_iterator<char>(stream)),
        std::istreambuf_iterator<char>());

    msgpack::unpacker pac;
    pac.reserve_buffer(buffer.size());
    std::copy(buffer.begin(), buffer.end(), pac.buffer());
    pac.buffer_consumed(buffer.size());
    progress.setProgressMaximum(100);

    msgpack::object_handle oh;
    pac.next(oh);
    try {
        auto m1 = oh.get();
        m1.convert(params);
        progress.progress(20);
    } catch (const std::exception& e) {
        throw std::runtime_error("Could not process input parameters section, bad encoding?");
    }
    pac.next(oh);
    try {
        auto m2 = oh.get();
        m2.convert(templatePointsMessage);
        progress.progress(50);
    } catch (const std::exception& e) {
        throw std::runtime_error("Could not process template points section, bad encoding?");
    }
    pac.next(oh);
    try {
        auto m3 = oh.get();
        m3.convert(arrangementMessage);
        progress.progress(100);
    } catch (const std::exception& e) {
        throw std::runtime_error("Could not process arrangement section, bad encoding?");
    }
    return FileContent(
        from_messages(templatePointsMessage, arrangementMessage).release());
}

//reads a point cloud
//  points are given by coordinates in Euclidean space, and each point has a "birth time" unless no function specified
//  stores the bifiltered Bifiltration/Vietoris-Rips complex in BifiltrationData
FileContent DataReader::read_point_cloud(std::ifstream& stream, Progress& progress)
{
    FileInputReader reader(stream);
    auto data = new InputData();
    if (verbosity >= 6) {
        debug() << "DataReader: Found a point cloud file.";
    }

    if (!input_params.old_function && !input_params.new_function) {
        input_params.x_label = "degree";
        input_params.x_reverse = true;
    }

    // set all variables from input_params
    unsigned dimension = input_params.dimension;
    exact max_dist = input_params.max_dist;
    bool hasFunction = input_params.old_function || input_params.new_function;

    bool x_reverse = input_params.x_reverse;
    bool y_reverse = input_params.y_reverse;

    unsigned expectedNumTokens = dimension;
    if (input_params.old_function)
        expectedNumTokens++;

    data->x_label = input_params.x_label;
    data->y_label = input_params.y_label;

    if (!hasFunction && input_params.filtration == "none")
        input_params.filtration = "degree";

    std::vector<DataPoint> points;

    std::pair<std::vector<std::string>, unsigned> line_info;

    // if a --function was specified, read in the function values
    std::vector<std::string> values;
    if (input_params.new_function) {
        for (int i = 0; i < input_params.function_line; i++)
            line_info = reader.next_line(0);

        for (unsigned i = 0; i < line_info.first.size(); i++) {
            values.push_back(line_info.first[i]);
        }
    }

    // skip lines with flags
    for (int i = 0; i < input_params.to_skip - input_params.function_line; i++)
        reader.next_line(0);

    // STEP 1: read data file and store exact (rational) values

    try {
        int k = 0;
        while (reader.has_next_line()) {
            line_info = reader.next_line(0);
            std::vector<std::string> tokens = line_info.first;
            if (tokens.size() != expectedNumTokens) {
                std::stringstream ss;
                ss << "invalid line (should be " << expectedNumTokens << " tokens but was " << tokens.size() << ")"
                   << std::endl;
                ss << "[";
                for (auto t : tokens) {
                    ss << t << " ";
                }
                ss << "]" << std::endl;

                throw std::runtime_error(ss.str());
            }

            // Add function values if supplied
            if (input_params.new_function) {
                tokens.push_back(values[k]);
                k++;
            }

            //Add artificial birth value of 0 if no function value provided
            if (!hasFunction) {
                tokens.push_back("0");
            }
            DataPoint p(tokens);
            if (x_reverse && hasFunction) {
                p.birth *= -1;
            }

            points.push_back(p);
        }

    } catch (std::exception& e) {
        throw InputError(line_info.second, e.what());
    }
    if (verbosity >= 4) {
        debug() << "  Finished reading" << points.size() << "points. Input finished.";
    }

    if (points.empty()) {
        throw std::runtime_error("No points loaded.");
    }

    // STEP 2: compute distance matrix, and create ordered lists of all unique distance and time values

    if (verbosity >= 4) {
        debug() << "  Building lists of grade values.";
    }
    progress.advanceProgressStage();

    unsigned num_points = points.size();

    DistanceMatrix dist_mat(input_params, num_points);
    dist_mat.build_distance_matrix(points);
    dist_mat.build_all_vectors(data);

    if (verbosity >= 4) {
        debug() << "  Finished reading data.";
    }

    // STEP 3: build vectors of discrete indexes for constructing the bifiltration

    //update progress
    progress.progress(30);

    // STEP 4: build the bifiltration

    //bifiltration_data stores only DISCRETE information!
    //this only requires (suppose there are k points):
    //  1. a list of k discrete times (if a function is included)
    //  2. a list of k(k-1)/2 discrete distances
    //  3. max dimension of simplices to construct, which is one more than the dimension of homology to be computed

    if (verbosity >= 4) {
        if (!hasFunction && input_params.filtration == "degree") {
            debug() << "  Building Degree-Rips bifiltration.";
        } else {
            debug() << "  Building Vietoris-Rips bifiltration.";
        }
        debug() << "     x-grades: " << data->x_exact.size();
        debug() << "     y-grades: " << data->y_exact.size();
    }

    data->bifiltration_data.reset(new BifiltrationData(input_params.hom_degree, input_params.verbosity));
    if (!hasFunction && input_params.filtration == "degree") {
        data->bifiltration_data->build_DR_complex(num_points, dist_mat.dist_indexes, dist_mat.degree_indexes, data->x_exact.size(), data->y_exact.size());
        //convert data->x_exact from codegree sequence to negative degree sequence
        exact max_x_exact = *(data->x_exact.end() - 1); //should it be max_degree instead?
        std::transform(data->x_exact.begin(), data->x_exact.end(), data->x_exact.begin(), [max_x_exact](exact x) { return x - max_x_exact; });

    } else {
        data->bifiltration_data->build_VR_complex(dist_mat.function_indexes, dist_mat.dist_indexes, data->x_exact.size(), data->y_exact.size());
    }

    if (verbosity >= 8) {
        int size;
        size = data->bifiltration_data->get_size(input_params.hom_degree - 1);
        debug() << "There are" << size << "simplices of dimension" << input_params.hom_degree - 1;
        size = data->bifiltration_data->get_size(input_params.hom_degree);
        debug() << "There are" << size << "simplices of dimension" << input_params.hom_degree;
        size = data->bifiltration_data->get_size(input_params.hom_degree + 1);
        debug() << "There are" << size << "simplices of dimension" << input_params.hom_degree + 1;
    }

    data->free_implicit_rep.reset(new FIRep(*(data->bifiltration_data), input_params.verbosity));

    // if (!hasFunction) {
    //     delete dist_mat.degree;
    // }

    //remember the axis directions
    data->x_reverse = x_reverse;
    data->y_reverse = y_reverse;

    return FileContent(data);

} //end read_point_cloud()

//reads data representing a discrete metric space with a real-valued function and stores in a BifiltrationData
FileContent DataReader::read_discrete_metric_space(std::ifstream& stream, Progress& progress)
{
    if (verbosity >= 2) {
        debug() << "DataReader: Found a discrete metric space file.";
    }
    auto data = new InputData();
    FileInputReader reader(stream);

    //prepare data structures
    ExactSet value_set; //stores all unique values of the function; must DELETE all elements later
    ExactSet dist_set; //stores all unique values of the distance metric; must DELETE all elements later
    unsigned num_points = input_params.dimension;
    unsigned expectedNumTokens = num_points;

    bool hasFunction = input_params.old_function || input_params.new_function;
    unsigned* degree; //stores the degree of each point; must FREE later if used
    std::pair<ExactSet::iterator, bool> ret; //for return value upon insert()

    // set all variables from input parameters
    bool x_reverse = input_params.x_reverse;
    bool y_reverse = input_params.y_reverse;

    data->x_label = input_params.x_label;
    data->y_label = input_params.y_label;

    exact max_dist = input_params.max_dist;

    if (!hasFunction && input_params.filtration == "none")
        input_params.filtration = "degree";

    std::pair<std::vector<std::string>, unsigned> line_info;

    // store function values if supplied
    std::vector<std::string> val;
    if (hasFunction) {
        for (int i = 0; i < input_params.function_line; i++)
            line_info = reader.next_line(0);

        num_points = line_info.first.size();
        for (unsigned i = 0; i < num_points; i++) {
            val.push_back(line_info.first[i]);
        }
    }

    // skip lines with flags
    for (int i = 0; i < input_params.to_skip - input_params.function_line - 1; i++)
        line_info = reader.next_line(0);

    DistanceMatrix dist_mat(input_params, num_points);

    try {
        std::vector<exact> values;

        if (hasFunction) {

            exact xrev_sign = x_reverse ? -1 : 1;

            if (verbosity >= 6) {
                debug() << "DataReader: Discrete metric space file has function values. Creating Vietoris-Rips complex.";
            }
            values.reserve(val.size());

            for (size_t i = 0; i < val.size(); i++) {
                values.push_back(xrev_sign * str_to_exact(val.at(i)));
            }

        } else {
            if (verbosity >= 6) {
                debug() << "DataReader: Discrete metric space file does not have function values. Creating Degree-Rips complex.";
            }
            if (verbosity >= 4) {
                debug() << "  Number of points:" << num_points;
            }
        }

        // STEP 2: read data file and store exact (rational) values for all distances

        if (verbosity >= 4) {
            std::ostringstream oss;
            oss << max_dist;
            debug() << "  Maximum distance of edges in Rips complex:" << oss.str().c_str();
        }

        if (!hasFunction) {
            degree = new unsigned[num_points]();
        }

        dist_mat.read_distance_matrix(stream, values);
        dist_mat.build_all_vectors(data);

    } catch (InputError& e) {
        throw;
    } catch (std::exception& e) {
        throw InputError(line_info.second, e.what());
    }
    if (verbosity >= 4) {
        debug() << "  Finished reading data.";
    }

    progress.advanceProgressStage(); //advance progress box to stage 2: building bifiltration
    // STEP 3: build vectors of discrete indexes for constructing the bifiltration

    //update progress
    progress.progress(30);

    // STEP 4: build the bifiltration

    if (verbosity >= 4) {
        if (!hasFunction && input_params.filtration == "degree") {
            debug() << "  Building Degree-Rips bifiltration.";
        } else {
            debug() << "  Building Vietoris-Rips bifiltration.";
        }
        debug() << "     x-grades: " << data->x_exact.size();
        debug() << "     y-grades: " << data->y_exact.size();
    }

    //build the Vietoris-Rips bifiltration from the discrete index vectors
    data->bifiltration_data.reset(new BifiltrationData(input_params.hom_degree, input_params.verbosity));
    if (!hasFunction && input_params.filtration == "degree") {
        data->bifiltration_data->build_DR_complex(num_points, dist_mat.dist_indexes, dist_mat.degree_indexes, data->x_exact.size(), data->y_exact.size());

        //convert data->x_exact from codegree sequence to negative degree sequence
        exact max_x_exact = *(data->x_exact.end() - 1); //should it be max_degree instead?
        std::transform(data->x_exact.begin(), data->x_exact.end(), data->x_exact.begin(), [max_x_exact](exact x) { return x - max_x_exact; });
    } else {
        data->bifiltration_data->build_VR_complex(dist_mat.function_indexes, dist_mat.dist_indexes, data->x_exact.size(), data->y_exact.size());
    }

    if (verbosity >= 8) {
        int size;
        size = data->bifiltration_data->get_size(input_params.hom_degree - 1);
        debug() << "There are" << size << "simplices of dimension" << input_params.hom_degree - 1;
        size = data->bifiltration_data->get_size(input_params.hom_degree);
        debug() << "There are" << size << "simplices of dimension" << input_params.hom_degree;
        size = data->bifiltration_data->get_size(input_params.hom_degree + 1);
        debug() << "There are" << size << "simplices of dimension" << input_params.hom_degree + 1;
    }

    data->free_implicit_rep.reset(new FIRep(*(data->bifiltration_data), input_params.verbosity));
    data->x_reverse = x_reverse;
    data->y_reverse = y_reverse;

    //clean up
    // if (!hasFunction) {
    //     delete dist_mat.degree;
    // }

    return FileContent(data);
} //end read_discrete_metric_space()

//reads a bifiltration and stores in BifiltrationData
FileContent DataReader::read_bifiltration(std::ifstream& stream, Progress& progress)
{
    auto data = new InputData();
    FileInputReader reader(stream);
    if (verbosity >= 2) {
        debug() << "DataReader: Found a bifiltration file.\n";
    }

    // set variables from input parameters
    bool x_reverse = input_params.x_reverse;
    data->x_label = input_params.x_label;
    bool y_reverse = input_params.y_reverse;
    data->y_label = input_params.y_label;

    exact xrev_sign = x_reverse ? -1 : 1;
    exact yrev_sign = y_reverse ? -1 : 1;

    // skip lines with flags
    for (int i = 0; i < input_params.to_skip; i++)
        reader.next_line(0);

    data->bifiltration_data.reset(new BifiltrationData(input_params.hom_degree, input_params.verbosity));

    //temporary data structures to store grades
    ExactSet x_set; //stores all unique x-values; must DELETE all elements later!
    ExactSet y_set; //stores all unique y-values; must DELETE all elements later!
    std::pair<ExactSet::iterator, bool> ret; //for return value upon insert()

    //read simplices
    std::vector<std::pair<std::vector<int>, unsigned>> simplexList;
    unsigned num_grades = 0;
    while (reader.has_next_line()) {
        auto line_info = reader.next_line();
        try {
            std::vector<std::string> tokens = line_info.first;

            if (tokens.size() > std::numeric_limits<unsigned>::max()) {
                throw InputError(line_info.second,
                    "line longer than " + std::to_string(std::numeric_limits<unsigned>::max()) + " tokens");
            }

            //read vertices
            unsigned pos = 0;
            std::vector<int> verts;
            while (tokens[pos].at(0) != ';') {
                int v = std::stoi(tokens[pos]);
                verts.push_back(v);
                pos++;
            }
            pos++;
            unsigned grades = (tokens.size() - pos) / 2; //remaining tokens are xy pairs
            for (unsigned i = 0; i < grades; i++) {
                //read multigrade and remember that it corresponds to this grade
                ret = x_set.insert(ExactValue(xrev_sign * str_to_exact(tokens.at(pos))));
                (ret.first)->indexes.push_back(num_grades);
                ret = y_set.insert(ExactValue(yrev_sign * str_to_exact(tokens.at(pos + 1))));
                (ret.first)->indexes.push_back(num_grades);
                num_grades++;
                pos += 2;
            }

            simplexList.push_back({ verts, grades });
        } catch (std::exception& e) {
            throw InputError(line_info.second, "Could not read vertex: " + std::string(e.what()));
        }
    }

    if (verbosity >= 4) {
        debug() << "  Finished reading data.";
    }

    progress.advanceProgressStage(); //advance progress box to stage 2: building bifiltration

    //build vectors of discrete grades, using bins
    unsigned max_unsigned = std::numeric_limits<unsigned>::max();
    std::vector<unsigned> x_indexes(num_grades, max_unsigned); //x_indexes[i] gives the discrete x-index for simplex i in the input order
    std::vector<unsigned> y_indexes(num_grades, max_unsigned); //y_indexes[i] gives the discrete y-index for simplex i in the input order

    build_grade_vectors(*data, x_set, x_indexes, data->x_exact, input_params.x_bins);
    build_grade_vectors(*data, y_set, y_indexes, data->y_exact, input_params.y_bins);

    int current_grade = 0;
    for (std::vector<std::pair<std::vector<int>, unsigned>>::iterator it = simplexList.begin(); it != simplexList.end(); it++) {
        unsigned appearances = it->second;
        AppearanceGrades gradesOfApp;
        for (unsigned i = 0; i < appearances; i++) {
            gradesOfApp.push_back(Grade(x_indexes[current_grade], y_indexes[current_grade]));
            current_grade++;
        }

        //TODO: Double-check that this sorting hasn't been done earlier.
        //Mike: I reorganized the code slightly so that the arguments of add_simplex are const references, which seems better, but that means any sorting should happen before.
        std::sort(gradesOfApp.begin(), gradesOfApp.end());
        std::sort(it->first.begin(), it->first.end());
        data->bifiltration_data->add_simplex(it->first, gradesOfApp);
    }
    if (verbosity >= 10) {
        data->bifiltration_data->print_bifiltration();
    }
    //data->bifiltration_data->createGradeInfo();
    data->bifiltration_data->set_xy_grades(data->x_exact.size(), data->y_exact.size());

    data->free_implicit_rep.reset(new FIRep(*(data->bifiltration_data), input_params.verbosity));

    data->x_reverse = x_reverse;
    data->y_reverse = y_reverse;

    return FileContent(data);
} //end read_bifiltration()

//reads a firep and stores in FIRep, does not create BifiltrationData
FileContent DataReader::read_firep(std::ifstream& stream, Progress& progress)
{
    auto data = new InputData;
    FileInputReader reader(stream);
    if (verbosity >= 2) {
        debug() << "DataReader: Found a firep file.\n";
    }

    // set up variables from input parameters
    bool x_reverse = input_params.x_reverse;
    bool y_reverse = input_params.y_reverse;

    data->x_label = input_params.x_label;
    data->y_label = input_params.y_label;

    // skip lines with flags
    for (int i = 0; i < input_params.to_skip; i++)
        reader.next_line(0);

    data->bifiltration_data.reset(new BifiltrationData(input_params.hom_degree, input_params.verbosity)); //Will be dummy bifiltration with no stored data

    //temporary data structures to store grades
    ExactSet x_set; //stores all unique x-values; must DELETE all elements later!
    ExactSet y_set; //stores all unique x-values; must DELETE all elements later!
    std::pair<ExactSet::iterator, bool> ret; //for return value upon insert()

    //Temporary data structures to store matrices
    int num_high_simplices, num_mid_simplices, num_low_simplices;
    std::vector<exact> x_values, y_values;
    std::vector<std::vector<unsigned>> boundary_mat_2, boundary_mat_1; //matrices boundary_mat_2: hom_dim+1->hom_dim, boundary_mat_1: hom_dim->hom_dim-1

    //read simplices
    while (reader.has_next_line()) {
        auto line_info = reader.next_line();
        try {
            //read the label for x-axis
            exact xrev_sign = x_reverse ? -1 : 1;

            //read the label for y-axis
            exact yrev_sign = y_reverse ? -1 : 1;

            //Read dimensions of matrices
            std::vector<std::string> tokens = line_info.first;

            if (tokens.size() != 3) {
                throw InputError(line_info.second,
                    "Expected 3 tokens");
            }

            num_high_simplices = std::stoi(line_info.first[0]);
            num_mid_simplices = std::stoi(line_info.first[1]);
            num_low_simplices = std::stoi(line_info.first[2]);

            //read matrices
            //Read boundary_mat_2 first
            x_values.reserve(num_high_simplices + num_mid_simplices);
            y_values.reserve(num_high_simplices + num_mid_simplices);
            for (int i = 0; i < num_high_simplices; i++) {
                boundary_mat_2.push_back(std::vector<unsigned>());
                line_info = reader.next_line();
                tokens = line_info.first;
                x_values.push_back(xrev_sign * str_to_exact(tokens.at(0)));
                y_values.push_back(yrev_sign * str_to_exact(tokens.at(1)));
                //store value, if it doesn't exist already
                ret = x_set.insert(ExactValue(x_values[i]));
                //remember that point i has this value
                (ret.first)->indexes.push_back(i);

                //store value, if it doesn't exist already
                ret = y_set.insert(ExactValue(y_values[i]));
                //remember that point i has this value
                (ret.first)->indexes.push_back(i);

                //Process ith column
                if (tokens.at(2).at(0) != ';') {
                    throw InputError(line_info.second, "Expected ';' after coordinates");
                }
                for (unsigned pos = 3; pos < tokens.size(); pos++) {
                    int v = std::stoi(tokens[pos]);
                    if (v < 0 || v >= num_mid_simplices) {
                        throw InputError(line_info.second, "Matrix index input out of bounds.");
                    }
                    boundary_mat_2[i].push_back(v);
                }
            }

            //read boundary_mat_1
            for (int i = 0; i < num_mid_simplices; i++) {
                boundary_mat_1.push_back(std::vector<unsigned>());
                line_info = reader.next_line();
                tokens = line_info.first;
                x_values.push_back(xrev_sign * str_to_exact(tokens.at(0)));
                y_values.push_back(yrev_sign * str_to_exact(tokens.at(1)));
                //store value, if it doesn't exist already
                ret = x_set.insert(ExactValue(x_values[i + num_high_simplices]));
                //remember that point i has this value
                (ret.first)->indexes.push_back(i + num_high_simplices);

                //store value, if it doesn't exist already
                ret = y_set.insert(ExactValue(y_values[i + num_high_simplices]));
                //remember that point i has this value
                (ret.first)->indexes.push_back(i + num_high_simplices);

                //Process ith column
                if (tokens.at(2).at(0) != ';') {
                    throw InputError(line_info.second, "Expected ';' after coordinates");
                }
                for (unsigned pos = 3; pos < tokens.size(); pos++) {
                    int v = std::stoi(tokens[pos]);
                    if (v < 0 || v >= num_low_simplices) {
                        throw InputError(line_info.second, "Matrix index input out of bounds.");
                    }
                    boundary_mat_1[i].push_back(v);
                }
            }
        } catch (std::exception& e) {
            throw InputError(line_info.second, "Could not read matrix: " + std::string(e.what()));
        }
    }

    if (verbosity >= 4) {
        debug() << "  Finished reading data.";
    }

    progress.advanceProgressStage(); //advance progress box to stage 2: building bifiltration

    //build vectors of discrete grades, using bins
    unsigned max_unsigned = std::numeric_limits<unsigned>::max();
    std::vector<unsigned> x_indexes(num_high_simplices + num_mid_simplices, max_unsigned); //x_indexes[i] gives the discrete x-index for simplex i in the input order
    std::vector<unsigned> y_indexes(num_high_simplices + num_mid_simplices, max_unsigned); //y_indexes[i] gives the discrete y-index for simplex i in the input order

    build_grade_vectors(*data, x_set, x_indexes, data->x_exact, input_params.x_bins);
    build_grade_vectors(*data, y_set, y_indexes, data->y_exact, input_params.y_bins);

    //Set x_grades and y_grades
    data->bifiltration_data->set_xy_grades(data->x_exact.size(), data->y_exact.size());
    data->free_implicit_rep.reset(new FIRep(*(data->bifiltration_data), num_high_simplices, num_mid_simplices, num_low_simplices, boundary_mat_2, boundary_mat_1, x_indexes, y_indexes, input_params.verbosity));

    data->x_reverse = x_reverse;
    data->y_reverse = y_reverse;

    return FileContent(data);
} //end read_firep()

//reads a file of previously-computed data from RIVET
FileContent DataReader::read_RIVET_data(std::ifstream& stream, Progress& progress)
{
    auto data = new InputData();
    FileInputReader reader(stream);

    //read parameters
    auto line_info = reader.next_line();
    auto line = line_info.first;
    debug() << DataReader::join(line).c_str();
    line_info = reader.next_line();
    try {
        input_params.hom_degree = std::stoi(line_info.first[0]);
        data->x_label = DataReader::join(reader.next_line().first);
        data->y_label = DataReader::join(reader.next_line().first);

        //read x-grades
        reader.next_line(); //this line should say "x-grades"
        line_info = reader.next_line();
        line = line_info.first;
        while (line[0][0] != 'y') //stop when we reach "y-grades"
        {
            exact num(line[0]);
            data->x_exact.push_back(num);
            line_info = reader.next_line();
            line = line_info.first;
        }

        //read y-grades

        line_info = reader.next_line();
        line = line_info.first;
        while (line[0][0] != 'x') //stop when we reach "xi"
        {
            exact num(line[0]);
            data->y_exact.push_back(num);
            line_info = reader.next_line();
            line = line_info.first;
        }

        //read xi values
        line_info = reader.next_line();
        line = line_info.first; //because the current line says "xi"
        while (line[0][0] != 'b') //stop when we reach "barcode templates"
        {
            unsigned x = std::stoi(line[0]);
            unsigned y = std::stoi(line[1]);
            int zero = std::stoi(line[2]);
            int one = std::stoi(line[3]);
            int two = std::stoi(line[4]);
            data->template_points.push_back(TemplatePoint(x, y, zero, one, two));
            line_info = reader.next_line();
            line = line_info.first;
        }

        //read barcode templates
        //  NOTE: the current line says "barcode templates"
        while (reader.has_next_line()) {
            line_info = reader.next_line();
            line = line_info.first;
            data->barcode_templates.push_back(BarcodeTemplate()); //create a new BarcodeTemplate

            if (line[0] != std::string("-")) //then the barcode is nonempty
            {
                for (size_t i = 0; i < line.size(); i++) //loop over all bars
                {
                    std::vector<std::string> nums = split(line[i], ",");
                    unsigned a = std::stol(nums[0]);
                    unsigned b = -1; //default, for b = infinity
                    if (nums[1][0] != 'i') //then b is finite
                        b = std::stol(nums[1]);
                    unsigned m = std::stol(nums[2]);
                    data->barcode_templates.back().add_bar(a, b, m);
                }
            }
        }

    } catch (std::exception& e) {
        throw InputError(line_info.second, e.what());
    }
    ///TODO: maybe make a different progress box for RIVET input???
    progress.advanceProgressStage(); //advance progress box to stage 2: building bifiltration

    return FileContent(data);
} //end read_RIVET_data()

//converts an ExactSet of values to the vectors of discrete
// values that BifiltrationTree uses to build the bifiltration,
// and also builds the grade vectors (floating-point and exact)
void DataReader::build_grade_vectors(InputData& data,
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

//finds a rational approximation of a floating-point value
// precondition: x > 0
exact DataReader::approx(double x)
{
    int d = 7; //desired number of significant digits
    int log = (int)floor(log10(x)) + 1;

    if (log >= d)
        return exact((int)floor(x));

    long denom = pow(10, d - log);
    return exact((long)floor(x * denom), denom);
}
