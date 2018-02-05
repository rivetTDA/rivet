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

#include "input_manager.h"
#include "../computation.h"
#include "../math/simplex_tree.h"
#include "file_input_reader.h"
#include "input_parameters.h"

#include "debug.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

//epsilon value for use in comparisons
double ExactValue::epsilon = pow(2, -30);

std::string join(const std::vector<std::string>& strings)
{
    std::stringstream ss;
    for (size_t i = 0; i < strings.size(); i++) {
        if (i > 0)
            ss << " ";
        ss << strings[i];
    }
    return ss.str();
}

std::vector<std::string> split(std::string& str, std::string separators)
{
    std::vector<std::string> strings;
    boost::split(strings, str, boost::is_any_of(separators));
    return strings;
}

class TokenReader {
public:
    TokenReader(FileInputReader& reader)
        : reader(reader)
        , tokens()
        , it(tokens.end())
        , line(0)
    {
    }
    bool has_next_token()
    {
        if (it != tokens.end()) {
            return true;
        }
        if (reader.has_next_line()) {
            auto info = reader.next_line();
            tokens = info.first;
            line = info.second;
            it = tokens.begin();
            return true;
        }
        return false;
    }

    std::string next_token()
    {
        if (has_next_token()) {
            auto val = *it;
            it++;
            return val;
        }
        return "";
    }

    unsigned line_number() const
    {
        return line;
    }

private:
    FileInputReader& reader;
    std::vector<std::string> tokens;
    std::vector<std::string>::iterator it;
    unsigned line;
};

//==================== InputManager class ====================
using namespace rivet::numeric;

//constructor
InputManager::InputManager(InputParameters& params)
    : input_params(params)
    , verbosity(params.verbosity)
{
    register_file_type(FileType{ "points", "point-cloud data", true,
        std::bind(&InputManager::read_point_cloud, this, std::placeholders::_1, std::placeholders::_2) });

    register_file_type(FileType{ "metric", "metric data", true,
        std::bind(&InputManager::read_discrete_metric_space, this, std::placeholders::_1, std::placeholders::_2) });
    register_file_type(FileType{ "bifiltration", "bifiltration data", true,
        std::bind(&InputManager::read_bifiltration, this, std::placeholders::_1, std::placeholders::_2) });
    //    register_file_type(FileType {"RIVET_0", "pre-computed RIVET data", false,
    //                                 std::bind(&InputManager::read_RIVET_data, this, std::placeholders::_1, std::placeholders::_2) });
}

void InputManager::register_file_type(FileType file_type)
{
    supported_types.push_back(file_type);
}
FileType& InputManager::get_file_type(std::string fileName)
{
    std::ifstream stream(fileName);
    if (!stream.is_open()) {
        throw std::runtime_error("Could not open " + fileName);
    }
    FileInputReader reader(stream);
    if (!reader.has_next_line()) {
        throw std::runtime_error("Empty file: " + fileName);
    }
    std::string filetype_name = reader.next_line().first[0];

    auto it = std::find_if(supported_types.begin(), supported_types.end(), [filetype_name](FileType t) { return t.identifier == filetype_name; });

    if (it == supported_types.end()) {
        throw std::runtime_error("Unsupported file type");
    }

    return *it;
}

//function to run the input manager, requires a filename
//  post condition: x_grades and x_exact have size x_bins, and they contain the grade values for the 2-D persistence module in double and exact form (respectively)
//                  similarly for y_grades and y_exact
std::unique_ptr<InputData> InputManager::start(Progress& progress)
{
    //read the file
    if (verbosity >= 2) {
        debug() << "READING FILE:" << input_params.fileName;
    }
    auto file_type = get_file_type(input_params.fileName);
    std::ifstream infile(input_params.fileName); //input file
    if (!infile.is_open()) {
        throw std::runtime_error("Could not open input file.");
    }
    auto data = file_type.parser(infile, progress);
    data->file_type = file_type;
    data->is_data = file_type.is_data;
    return data;
} //end start()

InputError::InputError(unsigned line, std::string message)
    : std::runtime_error("line " + std::to_string(line) + ": " + message)
{
}

//reads a point cloud
//  points are given by coordinates in Euclidean space, and each point has a "birth time"
//  constructs a simplex tree representing the bifiltered Vietoris-Rips complex
std::unique_ptr<InputData> InputManager::read_point_cloud(std::ifstream& stream, Progress& progress)
{
    //TODO : switch to YAML or JSON input or switch to proper parser generator or combinators
    FileInputReader reader(stream);
    auto data = std::make_unique<InputData>();
    if (verbosity >= 6) {
        debug() << "InputManager: Found a point cloud file.";
    }
    unsigned dimension;
    exact max_dist;

    //read points
    std::vector<DataPoint> points;

    // STEP 1: read data file and store exact (rational) values
    //skip first line
    reader.next_line();

    auto line_info = reader.next_line();
    try {
        //read dimension of the points from the first line of the file
        std::vector<std::string> dimension_line = line_info.first;
        if (dimension_line.size() != 1) {
            debug() << "There was more than one value in the expected dimension line."
                       " There may be a problem with your input file.  ";
        }
        if (verbosity >= 4) {
            debug() << "  Point cloud lives in dimension:" << dimension_line[0];
        }

        int dim = std::stoi(dimension_line[0]);
        if (dim < 1) {
            throw std::runtime_error("Dimension of data must be at least 1");
        }
        dimension = static_cast<unsigned>(dim);

        //read maximum distance for edges in Vietoris-Rips complex
        line_info = reader.next_line();
        std::vector<std::string> distance_line = line_info.first;
        if (distance_line.size() != 1) {
            throw std::runtime_error("There was more than one value in the expected distance line.");
        }

        max_dist = str_to_exact(distance_line[0]);
        if (max_dist <= 0) {
            throw std::runtime_error("An invalid input was received for the max distance.");
        }

        if (verbosity >= 4) {
            std::ostringstream oss;
            oss << max_dist;
            debug() << "  Maximum distance of edges in Vietoris-Rips complex:" << oss.str();
        }

        //read label for x-axis
        line_info = reader.next_line();
        data->x_label = line_info.first[0];

        //set label for y-axis to "distance"
        data->y_label = "distance";

        while (reader.has_next_line()) {
            line_info = reader.next_line();
            std::vector<std::string> tokens = line_info.first;
            if (tokens.size() != dimension + 1) {
                std::stringstream ss;
                ss << "invalid line (should be " << dimension + 1 << " tokens but was " << tokens.size() << ")"
                   << std::endl;
                ss << "[";
                for (auto t : tokens) {
                    ss << t << " ";
                }
                ss << "]" << std::endl;

                throw std::runtime_error(ss.str());
            }
            DataPoint p(tokens);
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

    ExactSet dist_set; //stores all unique distance values
    ExactSet time_set; //stores all unique time values
    std::pair<ExactSet::iterator, bool> ret; //for return value upon insert()

    dist_set.insert(ExactValue(exact(0))); //distance from a point to itself is always zero

    //consider all points
    for (unsigned i = 0; i < num_points; i++) {
        //store time value, if it doesn't exist already
        ret = time_set.insert(ExactValue(points[i].birth));

        //remember that point i has this birth time value
        (ret.first)->indexes.push_back(i);

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

            if (cur_dist <= max_dist) //then this distance is allowed
            {
                //store distance value, if it doesn't exist already
                ret = dist_set.insert(ExactValue(cur_dist));

                //remember that the pair of points (i,j) has this distance value, which will go in entry j(j-1)/2 + i
                (ret.first)->indexes.push_back((j * (j - 1)) / 2 + i);
            }
        }
    } //end for

    // STEP 3: build vectors of discrete indexes for constructing the bifiltration

    unsigned max_unsigned = std::numeric_limits<unsigned>::max();

    //first, times

    //vector of discrete time indexes for each point; max_unsigned shall represent undefined time (is this reasonable?)
    std::vector<unsigned> time_indexes(num_points, max_unsigned);
    build_grade_vectors(*data, time_set, time_indexes, data->x_exact, input_params.x_bins);

    //second, distances

    //discrete distance matrix (triangle); max_unsigned shall represent undefined distance
    std::vector<unsigned> dist_indexes((num_points * (num_points - 1)) / 2, max_unsigned);
    build_grade_vectors(*data, dist_set, dist_indexes, data->y_exact, input_params.y_bins);

    //update progress
    progress.progress(30);

    // STEP 4: build the bifiltration

    //simplex_tree stores only DISCRETE information!
    //this only requires (suppose there are k points):
    //  1. a list of k discrete times
    //  2. a list of k(k-1)/2 discrete distances
    //  3. max dimension of simplices to construct, which is one more than the dimension of homology to be computed

    if (verbosity >= 4) {
        debug() << "  Building Vietoris-Rips bifiltration.";
        debug() << "     x-grades: " << data->x_exact.size();
        debug() << "     y-grades: " << data->y_exact.size();
    }

    data->simplex_tree.reset(new SimplexTree(input_params.dim, input_params.verbosity));
    data->simplex_tree->build_VR_complex(time_indexes, dist_indexes, data->x_exact.size(), data->y_exact.size());

    if (verbosity >= 8) {
        data->simplex_tree->print_bifiltration();
    }

    return data;
} //end read_point_cloud()

//reads data representing a discrete metric space with a real-valued function and constructs a simplex tree
std::unique_ptr<InputData> InputManager::read_discrete_metric_space(std::ifstream& stream, Progress& progress)
{
    if (verbosity >= 2) {
        debug() << "InputManager: Found a discrete metric space file.";
    }
    std::unique_ptr<InputData> data(new InputData);
    FileInputReader reader(stream);

    //prepare data structures
    ExactSet value_set; //stores all unique values of the function; must DELETE all elements later
    ExactSet dist_set; //stores all unique values of the distance metric; must DELETE all elements later
    unsigned num_points;

    // STEP 1: read data file and store exact (rational) values of the function for each point

    //skip 'metric'
    auto line_info = reader.next_line();
    line_info = reader.next_line();
    //first read the label for x-axis
    try {
        data->x_label = line_info.first[0];

        //now read the values
        line_info = reader.next_line();
        std::vector<std::string> line = line_info.first;
        std::vector<exact> values;
        values.reserve(line.size());

        for (size_t i = 0; i < line.size(); i++) {
            values.push_back(str_to_exact(line.at(i)));
        }

        // STEP 2: read data file and store exact (rational) values for all distances

        //first read the label for y-axis
        line_info = reader.next_line();
        data->y_label = join(line_info.first);

        //read the maximum length of edges to construct
        line_info = reader.next_line();
        exact max_dist;
        max_dist = str_to_exact(line_info.first[0]);
        if (verbosity >= 4) {
            std::ostringstream oss;
            oss << max_dist;
            debug() << "  Maximum distance of edges in Vietoris-Rips complex:" << oss.str();
        }

        std::pair<ExactSet::iterator, bool> ret; //for return value upon insert()

        dist_set.insert(ExactValue(exact(0))); //distance from a point to itself is always zero

        //consider all points
        num_points = values.size();
        for (unsigned i = 0; i < num_points; i++) {
            //store value, if it doesn't exist already
            ret = value_set.insert(ExactValue(values[i]));

            //remember that point i has this value
            (ret.first)->indexes.push_back(i);

            //read distances from this point to all following points
            if (i < num_points - 1) //then there is at least one point after point i, and there should be another line to read
            {
                TokenReader tokens(reader);
                try {
                    for (unsigned j = i + 1; j < num_points; j++) {
                        //read distance between points i and j
                        if (!tokens.has_next_token())
                            throw std::runtime_error("no distance between points " + std::to_string(i)
                                + "and" + std::to_string(j));

                        std::string str = tokens.next_token();

                        exact cur_dist = str_to_exact(str);

                        if (cur_dist <= max_dist) //then this distance is allowed
                        {
                            //store distance value, if it doesn't exist already
                            ret = dist_set.insert(ExactValue(cur_dist));

                            //remember that the pair of points (i,j) has this distance value, which will go in entry j(j-1)/2 + i
                            (ret.first)->indexes.push_back((j * (j - 1)) / 2 + i);
                        }
                    }
                } catch (std::exception& e) {
                    throw InputError(tokens.line_number(), e.what());
                }
            }
        } //end for

    } catch (InputError& e) {
        throw;
    } catch (std::exception& e) {
        throw InputError(line_info.second, e.what());
    }
    progress.advanceProgressStage(); //advance progress box to stage 2: building bifiltration

    // STEP 3: build vectors of discrete indexes for constructing the bifiltration

    unsigned max_unsigned = std::numeric_limits<unsigned>::max();

    //first, values
    std::vector<unsigned> value_indexes(num_points, max_unsigned); //vector of discrete value indexes for each point; max_unsigned shall represent undefined value (is this reasonable?)
    build_grade_vectors(*data, value_set, value_indexes, data->x_exact, input_params.x_bins);

    //second, distances
    std::vector<unsigned> dist_indexes((num_points * (num_points - 1)) / 2, max_unsigned); //discrete distance matrix (triangle); max_unsigned shall represent undefined distance
    build_grade_vectors(*data, dist_set, dist_indexes, data->y_exact, input_params.y_bins);

    //update progress
    progress.progress(30);

    // STEP 4: build the bifiltration

    if (verbosity >= 4) {
        debug() << "  Building Vietoris-Rips bifiltration.";
        debug() << "     x-grades: " << data->x_exact.size();
        debug() << "     y-grades: " << data->y_exact.size();
    }

    //build the Vietoris-Rips bifiltration from the discrete index vectors
    data->simplex_tree.reset(new SimplexTree(input_params.dim, input_params.verbosity));
    data->simplex_tree->build_VR_complex(value_indexes, dist_indexes, data->x_exact.size(), data->y_exact.size());

    return data;
} //end read_discrete_metric_space()

//reads a bifiltration and constructs a simplex tree
std::unique_ptr<InputData> InputManager::read_bifiltration(std::ifstream& stream, Progress& progress)
{
    std::unique_ptr<InputData> data(new InputData);
    FileInputReader reader(stream);
    if (verbosity >= 2) {
        debug() << "InputManager: Found a bifiltration file.\n";
    }

    //Skip file type line
    reader.next_line();

    //read the label for x-axis
    data->x_label = join(reader.next_line().first);

    //read the label for y-axis
    data->y_label = join(reader.next_line().first);

    data->simplex_tree.reset(new SimplexTree(input_params.dim, input_params.verbosity));

    //temporary data structures to store grades
    ExactSet x_set; //stores all unique x-values; must DELETE all elements later!
    ExactSet y_set; //stores all unique x-values; must DELETE all elements later!
    std::pair<ExactSet::iterator, bool> ret; //for return value upon insert()

    //read simplices
    unsigned num_simplices = 0;
    while (reader.has_next_line()) {
        auto line_info = reader.next_line();
        try {
            std::vector<std::string> tokens = line_info.first;

            if (tokens.size() > std::numeric_limits<unsigned>::max()) {
                throw InputError(line_info.second,
                    "line longer than " + std::to_string(std::numeric_limits<unsigned>::max()) + " tokens");
            }

            //read dimension of simplex
            unsigned dim = static_cast<unsigned>(tokens.size() - 3); //-3 because a n-simplex has (n+1) vertices, and the line also contains two grade values

            //read vertices
            std::vector<int> verts;
            for (unsigned i = 0; i <= dim; i++) {
                int v = std::stoi(tokens[i]);
                verts.push_back(v);
            }

            //read multigrade and remember that it corresponds to this simplex
            ret = x_set.insert(ExactValue(str_to_exact(tokens.at(dim + 1))));
            (ret.first)->indexes.push_back(num_simplices);
            ret = y_set.insert(ExactValue(str_to_exact(tokens.at(dim + 2))));
            (ret.first)->indexes.push_back(num_simplices);

            //add the simplex to the simplex tree
            data->simplex_tree->add_simplex(verts, num_simplices, num_simplices); //multigrade to be set later!
            num_simplices++;
        } catch (std::exception& e) {
            throw InputError(line_info.second, "Could not read vertex: " + std::string(e.what()));
        }
    }

    progress.advanceProgressStage(); //advance progress box to stage 2: building bifiltration

    //build vectors of discrete grades, using bins
    unsigned max_unsigned = std::numeric_limits<unsigned>::max();
    std::vector<unsigned> x_indexes(num_simplices, max_unsigned); //x_indexes[i] gives the discrete x-index for simplex i in the input order
    std::vector<unsigned> y_indexes(num_simplices, max_unsigned); //y_indexes[i] gives the discrete y-index for simplex i in the input order

    build_grade_vectors(*data, x_set, x_indexes, data->x_exact, input_params.x_bins);
    build_grade_vectors(*data, y_set, y_indexes, data->y_exact, input_params.y_bins);

    //update simplex tree nodes
    data->simplex_tree->update_xy_indexes(x_indexes, y_indexes, data->x_exact.size(), data->y_exact.size());

    //compute indexes
    data->simplex_tree->update_global_indexes();
    data->simplex_tree->update_dim_indexes();

    return data;
} //end read_bifiltration()

//reads a file of previously-computed data from RIVET
std::unique_ptr<InputData> InputManager::read_RIVET_data(std::ifstream& stream, Progress& progress)
{
    std::unique_ptr<InputData> data(new InputData);
    FileInputReader reader(stream);

    //read parameters
    auto line_info = reader.next_line();
    auto line = line_info.first;
    debug() << join(line);
    line_info = reader.next_line();
    try {
        input_params.dim = std::stoi(line_info.first[0]);
        data->x_label = join(reader.next_line().first);
        data->y_label = join(reader.next_line().first);

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

    return data;
} //end read_RIVET_data()

//converts an ExactSet of values to the vectors of discrete
// values that SimplexTree uses to build the bifiltration,
// and also builds the grade vectors (floating-point and exact)
void InputManager::build_grade_vectors(InputData& data,
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
exact InputManager::approx(double x)
{
    int d = 7; //desired number of significant digits
    int log = (int)floor(log10(x)) + 1;

    if (log >= d)
        return exact((int)floor(x));

    long denom = pow(10, d - log);
    return exact((long)floor(x * denom), denom);
}

FileType InputManager::identify()
{
    return this->get_file_type(this->input_params.fileName);
}
