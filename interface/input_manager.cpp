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
#include "../math/bifiltration_data.h"
#include "file_input_reader.h"
#include "input_parameters.h"
#include "debug.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <set>
#include <sstream>
#include <vector>
#include <memory>
#include <dcel/arrangement_message.h>
#include <api.h>

#include <ctime>

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

FileContent::FileContent() {
    type = FileContentType::INVALID;
    input_data = nullptr;
    result = nullptr;
}

FileContent::FileContent(InputData *data) {
    type = FileContentType::DATA;
    input_data = std::make_shared<InputData>(*data);
}

FileContent::FileContent(ComputationResult *result) {
    type = FileContentType::PRECOMPUTED;
    FileContent::result = std::make_shared<ComputationResult>(*result);
}

FileContent& FileContent::operator=(const FileContent &other) {
    type = other.type;
    input_data = other.input_data;
    result = other.result;
    return *this;
}

FileContent::FileContent(const FileContent &other) {
    type = other.type;
    input_data = other.input_data;
    result = other.result;
}

//FileContent::~FileContent() {
//
//}

//a function to detect the axis reversal flag
//the first return value is true iff the axis is reversed
//and the second is the axis label
std::pair<bool, std::string> detect_axis_reversed(std::vector<std::string> line)
{

    bool is_reversed=false;
    std::string label;

    std::string joined=join(line);

    if(boost::starts_with(line[0],"[")){
        auto close_position=joined.find_first_of("]");
        if(close_position==std::string::npos){
            throw std::runtime_error("No closing bracket in axis label");
        }
        //get the nonwhitespace characters between the two brackets
        std::string between=joined.substr(1, close_position-1);
        boost::trim(between);

        if(between=="-"){
            is_reversed=true;
        }
        //if there is a +, or more than one whitespace character before the closing bracket, then don't do anything
        //if there is exactly one non "+" charachter before the closing bracket, throw an error

        else if(between!="+" && between.length()==1){
            throw std::runtime_error("An invalid character was received for the axis direction");

        }
        //if we are at this point, then line[0] begins with either [+] or [-]
        //the label is the rest of the string
        //the trim is there because there may or may not be whitespace between the bracket and the label
        label=joined.substr(close_position+1);
        boost::trim(label);

    }

    else{
        //there is no direction flag, the xlabel is just the line
        //and x_reverse retains its default value of false
        label= joined;
        }


    return std::make_pair(is_reversed,label);


}







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
    register_file_type(FileType{ "firep", "free implicit representation data", true,
        std::bind(&InputManager::read_firep, this, std::placeholders::_1, std::placeholders::_2) });
    register_file_type(FileType{ "RIVET_msgpack", "pre-computed RIVET data", false,
                                 std::bind(&InputManager::read_messagepack, this, std::placeholders::_1, std::placeholders::_2) });
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
FileContent InputManager::start(Progress& progress)
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
    return data;
} //end start()

InputError::InputError(unsigned line, std::string message)
    : std::runtime_error("line " + std::to_string(line) + ": " + message)
{
}

FileContent InputManager::read_messagepack(std::ifstream& stream, Progress& progress) {

    std::string type;
    std::getline(stream, type);
    InputParameters params;
    TemplatePointsMessage templatePointsMessage;
    ArrangementMessage arrangementMessage;
    std::string buffer((std::istreambuf_iterator<char>(stream)),
                       std::istreambuf_iterator<char>());

    msgpack::unpacker pac;
    pac.reserve_buffer( buffer.size() );
    std::copy( buffer.begin(), buffer.end(), pac.buffer() );
    pac.buffer_consumed( buffer.size() );
    progress.setProgressMaximum(100);

    msgpack::object_handle oh;
    pac.next(oh);
    auto m1 = oh.get();
    m1.convert(params);
    progress.progress(20);
    pac.next(oh);
    auto m2 = oh.get();
    m2.convert(templatePointsMessage);
    progress.progress(50);
    pac.next(oh);
    auto m3 = oh.get();
    m3.convert(arrangementMessage);
    progress.progress(100);
    return FileContent(
            from_messages(templatePointsMessage, arrangementMessage).release());
}

//reads a point cloud
//  points are given by coordinates in Euclidean space, and each point has a "birth time" unless no function specified
//  stores the bifiltered Bifiltration/Vietoris-Rips complex in BifiltrationData
FileContent InputManager::read_point_cloud(std::ifstream& stream, Progress& progress)
{
    //TODO : switch to YAML or JSON input or switch to proper parser generator or combinators
    FileInputReader reader(stream);
    auto data = new InputData();
    if (verbosity >= 6) {
        debug() << "InputManager: Found a point cloud file.";
    }
    unsigned dimension;
    exact max_dist;
    bool hasFunction;

    bool x_reverse=false;
    bool y_reverse=false;

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

        //read maximum distance for edges in Degree-Rips complex
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
            debug() << "  Maximum distance of edges in Degree-Rips complex:" << oss.str();
        }

        unsigned expectedNumTokens;

        //read label for x-axis
        line_info = reader.next_line();
        if (join(line_info.first).compare("no function") == 0) {
            hasFunction = false;
            //set label for x-axis to "degree"
            data->x_label = "degree";
            expectedNumTokens = dimension;
            if (verbosity >= 6) {
                debug() << "InputManager: Point cloud file does not have function values. Creating Degree-Rips complex.";
            }

            x_reverse=true; //higher degrees will be shown on the left

        } else {
            hasFunction = true;

            auto is_reversed_and_label=detect_axis_reversed(line_info.first);
            x_reverse=is_reversed_and_label.first;
            data->x_label=is_reversed_and_label.second;

            expectedNumTokens = dimension + 1;
            if (verbosity >= 6) {
                debug() << "InputManager: Point cloud file has function values. Creating Vietoris-Rips complex.";
            }
        }

        //set label for y-axis to "distance"
        data->y_label = "distance";

        while (reader.has_next_line()) {
            line_info = reader.next_line();
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

            //Add artificial birth value of 0 if no function value provided
            if (!hasFunction) {
                tokens.push_back("0");
            }
            DataPoint p(tokens);
            if (x_reverse&& hasFunction) {
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

    ExactSet dist_set; //stores all unique distance values
    ExactSet time_set; //stores all unique time values
    std::pair<ExactSet::iterator, bool> ret; //for return value upon insert()
    unsigned* degree; //stores the degree of each point; must FREE later if used
    if (!hasFunction) {
        degree = new unsigned[num_points]();
    }

    ret = dist_set.insert(ExactValue(exact(0))); //distance from a point to itself is always zero
    (ret.first)->indexes.push_back(0); //store distance 0 at 0th index
    //consider all points
    for (unsigned i = 0; i < num_points; i++) {
        if (hasFunction) {
            //store time value, if it doesn't exist already
            ret = time_set.insert(ExactValue(points[i].birth));

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

            if (cur_dist <= max_dist) //then this distance is allowed
            {
                //store distance value, if it doesn't exist already
                ret = dist_set.insert(ExactValue(cur_dist));

                //remember that the pair of points (i,j) has this distance value, which will go in entry j(j-1)/2 + i + 1
                (ret.first)->indexes.push_back((j * (j - 1)) / 2 + i + 1);

                //need to keep track of degree for degree-Rips complex
                if (!hasFunction) {
                    //there is an edge between i and j so update degree
                    degree[i]++;
                    degree[j]++;
                }
            }
        }
    } //end for
    if (verbosity >= 4) {
        debug() << "  Finished reading data.";
    }

    // STEP 3: build vectors of discrete indexes for constructing the bifiltration

    std::vector<unsigned> time_indexes, degree_indexes;
    ExactSet degree_set;
    unsigned max_unsigned = std::numeric_limits<unsigned>::max();

    //X axis is degrees for degree-Rips complex
    if (!hasFunction) {
        //determine the max degree
        unsigned maxDegree = 0;
        for (unsigned i = 0; i < num_points; i++)
            if (maxDegree < degree[i])
                maxDegree = degree[i];

        //build vector of discrete degree indices from 0 to maxDegree and bins those degree values
        //WARNING: assumes that the number of distinct degree grades will be equal to maxDegree which may not hold
        for (unsigned i = 0; i <= maxDegree; i++) {
            ret = degree_set.insert(ExactValue(maxDegree - i)); //store degree -i because degree is wrt opposite ordering on R
            (ret.first)->indexes.push_back(i); //degree i is stored at index i
        }
        //make degrees
        degree_indexes = std::vector<unsigned>(maxDegree + 1, 0);

        build_grade_vectors(*data, degree_set, degree_indexes, data->x_exact, input_params.x_bins);
        //data->x_exact is now an increasing list of codegrees
        //consider the sublevel set codeg<=k
        //this is the same as deg>=maxDeg-k
        //thus to convert from sublevelset to superlevel set, must replace the kth element of data->x_exact
        //with -1*(codeg-(kth element)) (the negative 1 is to the values are still increasing)
        //this is just the sequence of NEGATIVE degrees
        //e.g codeg=(0,1,2,...,10)
        //goes to (-(10-0), -(10-1), ...,-(10-10))=(-10,-9,...,0)


    }
    //X axis is given by function in Vietoris-Rips complex
    else {
        //vector of discrete time indexes for each point; max_unsigned shall represent undefined time (is this reasonable?)
        time_indexes = std::vector<unsigned>(num_points, max_unsigned);
        build_grade_vectors(*data, time_set, time_indexes, data->x_exact, input_params.x_bins);
    }

    //discrete distance matrix (triangle); max_unsigned shall represent undefined distance
    std::vector<unsigned> dist_indexes((num_points * (num_points - 1)) / 2 + 1, max_unsigned);
    build_grade_vectors(*data, dist_set, dist_indexes, data->y_exact, input_params.y_bins);

    //update progress
    progress.progress(30);

    // STEP 4: build the bifiltration

    //bifiltration_data stores only DISCRETE information!
    //this only requires (suppose there are k points):
    //  1. a list of k discrete times (if a function is included)
    //  2. a list of k(k-1)/2 discrete distances
    //  3. max dimension of simplices to construct, which is one more than the dimension of homology to be computed

    if (verbosity >= 4) {
        if (hasFunction) {
            debug() << "  Building Vietoris-Rips bifiltration.";
        } else {
            debug() << "  Building Degree-Rips bifiltration.";
        }
        debug() << "     x-grades: " << data->x_exact.size();
        debug() << "     y-grades: " << data->y_exact.size();
    }

    data->bifiltration_data.reset(new BifiltrationData(input_params.dim, input_params.verbosity));
    if (hasFunction) {
        data->bifiltration_data->build_VR_complex(time_indexes, dist_indexes, data->x_exact.size(), data->y_exact.size());

    } else {

        data->bifiltration_data->build_DR_complex(num_points, dist_indexes, degree_indexes, data->x_exact.size(), data->y_exact.size());
        //convert data->x_exact from codegree sequence to negative degree sequence
        exact max_x_exact=*(data->x_exact.end()-1);//should it be maxdegree instead?
        std::transform(data->x_exact.begin(),data->x_exact.end(), data->x_exact.begin(),[max_x_exact](exact x){return x-max_x_exact;});
    }

    if (verbosity >= 8) {
        int size;
        size = data->bifiltration_data->get_size(input_params.dim - 1);
        debug() << "There are" << size << "simplices of dimension" << input_params.dim - 1;
        size = data->bifiltration_data->get_size(input_params.dim);
        debug() << "There are" << size << "simplices of dimension" << input_params.dim;
        size = data->bifiltration_data->get_size(input_params.dim + 1);
        debug() << "There are" << size << "simplices of dimension" << input_params.dim + 1;
    }

    data->free_implicit_rep.reset(new FIRep(*(data->bifiltration_data), input_params.verbosity));

    if(!hasFunction){
        delete degree;
    }

    //remember the axis directions
    data->x_reverse=x_reverse;
    data->y_reverse=y_reverse;

    return FileContent(data);




} //end read_point_cloud()

//reads data representing a discrete metric space with a real-valued function and stores in a BifiltrationData
FileContent InputManager::read_discrete_metric_space(std::ifstream& stream, Progress& progress)
{
    if (verbosity >= 2) {
        debug() << "InputManager: Found a discrete metric space file.";
    }
    auto data = new InputData();
    FileInputReader reader(stream);

    //prepare data structures
    ExactSet value_set; //stores all unique values of the function; must DELETE all elements later
    ExactSet dist_set; //stores all unique values of the distance metric; must DELETE all elements later
    unsigned num_points;


    bool hasFunction;
    unsigned* degree; //stores the degree of each point; must FREE later if used
    std::pair<ExactSet::iterator, bool> ret; //for return value upon insert()

    bool x_reverse=false;
    bool y_reverse=false;


    // STEP 1: read data file and store exact (rational) values of the function for each point

    //skip 'metric'
    auto line_info = reader.next_line();
    line_info = reader.next_line();
    //first read the label for x-axis
    try {
        std::vector<exact> values;

        if (join(line_info.first).compare("no function") == 0) {
            hasFunction = false;
            //set label for x-axis to "degree"
            data->x_label = "degree";
            if (verbosity >= 6) {
                debug() << "InputManager: Discrete metric space file does not have function values. Creating Degree-Rips complex.";
            }

            //x_reverse is true in this case
            x_reverse=true;

            //TODO Probably should find new way to get number of points
            //now read the number of points
            line_info = reader.next_line();
            if (verbosity >= 4) {
                debug() << "  Number of points:" << line_info.first[0];
            }

            int dim = std::stoi(line_info.first[0]);
            if (dim < 1) {
                throw std::runtime_error("Number of points must be at least 1");
            }
            num_points = static_cast<unsigned>(dim);
        }
        else {
            hasFunction = true;

            //check for axis reversal
            auto is_reversed_and_label=detect_axis_reversed(line_info.first);
            x_reverse=is_reversed_and_label.first;
            data->x_label = is_reversed_and_label.second;

            exact xrev_sign= x_reverse? -1 :1;


            if (verbosity >= 6) {
                debug() << "InputManager: Discrete metric space file has function values. Creating Vietoris-Rips complex.";
            }
            //now read the values
            line_info = reader.next_line();
            std::vector<std::string> line = line_info.first;
            values.reserve(line.size());

            for (size_t i = 0; i < line.size(); i++) {
                values.push_back(xrev_sign*str_to_exact(line.at(i)));
            }
            num_points = values.size();
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
            debug() << "  Maximum distance of edges in Rips complex:" << oss.str();
        }

        if (!hasFunction) {
            degree = new unsigned[num_points]();
        }

        ret = dist_set.insert(ExactValue(exact(0))); //distance from a point to itself is always zero
        //store distance 0 at index 0
        (ret.first)->indexes.push_back(0);

        TokenReader tokens(reader);

        //consider all points
        for (unsigned i = 0; i < num_points; i++) {
            if (hasFunction) {
                //store value, if it doesn't exist already
                ret = value_set.insert(ExactValue(values[i]));

                //remember that point i has this value
                (ret.first)->indexes.push_back(i);
            }

            //read distances from this point to all following points
            if (i < num_points - 1) //then there is at least one point after point i, and there should be another line to read
            {
                try {
                    for (unsigned j = i + 1; j < num_points; j++) {

                        //read distance between points i and j
                        if (!tokens.has_next_token())
                            throw std::runtime_error("no distance between points " + std::to_string(i)
                                + " and " + std::to_string(j));

                        std::string str = tokens.next_token();

                        exact cur_dist = str_to_exact(str);


                        if (cur_dist <= max_dist) //then this distance is allowed
                        {
                            //store distance value, if it doesn't exist already
                            ret = dist_set.insert(ExactValue(cur_dist));

                            //remember that the pair of points (i,j) has this distance value, which will go in entry j(j-1)/2 + i + 1
                            (ret.first)->indexes.push_back((j * (j - 1)) / 2 + i + 1);

                            //need to keep track of degree for degree-Rips complex
                            if (!hasFunction) {
                                //there is an edge between i and j so update degree
                                degree[i]++;
                                degree[j]++;
                            }
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
    if (verbosity >= 4) {
        debug() << "  Finished reading data.";
    }

    progress.advanceProgressStage(); //advance progress box to stage 2: building bifiltration
    // STEP 3: build vectors of discrete indexes for constructing the bifiltration

    std::vector<unsigned> value_indexes, degree_indexes;
    ExactSet degree_set;
    unsigned max_unsigned = std::numeric_limits<unsigned>::max();

    //X axis is degrees for Degree-Rips complex
    if (!hasFunction) {
        //determine the max degree
        unsigned maxDegree = 0;
        for (unsigned i = 0; i < num_points; i++)
            if (maxDegree < degree[i])
                maxDegree = degree[i];


        //build vector of discrete degree indices from 0 to maxDegree and bins those degree values
        //WARNING: assumes that the number of distinct degree grades will be equal to maxDegree which may not hold
        for (unsigned i = 0; i <= maxDegree; i++) {
            ret = degree_set.insert(ExactValue(maxDegree - i)); //store degree -i because degree is wrt opposite ordering on R
            (ret.first)->indexes.push_back(i); //degree i is stored at index i
        }
        //make degrees
        degree_indexes = std::vector<unsigned>(maxDegree + 1, 0);
        build_grade_vectors(*data, degree_set, degree_indexes, data->x_exact, input_params.x_bins);
    }
    //X axis is given by function in Vietoris-Rips complex
    else {
        //vector of discrete time indexes for each point; max_unsigned shall represent undefined time (is this reasonable?)
        value_indexes = std::vector<unsigned>(num_points, max_unsigned);
        build_grade_vectors(*data, value_set, value_indexes, data->x_exact, input_params.x_bins);
    }

    //second, distances
    std::vector<unsigned> dist_indexes((num_points * (num_points - 1)) / 2 + 1, max_unsigned); //discrete distance matrix (triangle); max_unsigned shall represent undefined distance
    build_grade_vectors(*data, dist_set, dist_indexes, data->y_exact, input_params.y_bins);

    //update progress
    progress.progress(30);

    // STEP 4: build the bifiltration

    if (verbosity >= 4) {
        if (hasFunction) {
            debug() << "  Building Vietoris-Rips bifiltration.";
        } else {
            debug() << "  Building Degree-Rips bifiltration.";
        }
        debug() << "     x-grades: " << data->x_exact.size();
        debug() << "     y-grades: " << data->y_exact.size();
    }

    //build the Vietoris-Rips bifiltration from the discrete index vectors
    data->bifiltration_data.reset(new BifiltrationData(input_params.dim, input_params.verbosity));
    if (hasFunction) {
        data->bifiltration_data->build_VR_complex(value_indexes, dist_indexes, data->x_exact.size(), data->y_exact.size());
    } else {

        data->bifiltration_data->build_DR_complex(num_points, dist_indexes, degree_indexes, data->x_exact.size(), data->y_exact.size());

        //convert data->x_exact from codegree sequence to negative degree sequence
        exact max_x_exact=*(data->x_exact.end()-1);//should it be maxdegree instead?
        std::transform(data->x_exact.begin(),data->x_exact.end(), data->x_exact.begin(), [max_x_exact](exact x){return x-max_x_exact;});
    }

    if (verbosity >= 8) {
        int size;
        size = data->bifiltration_data->get_size(input_params.dim - 1);
        debug() << "There are" << size << "simplices of dimension" << input_params.dim - 1;
        size = data->bifiltration_data->get_size(input_params.dim);
        debug() << "There are" << size << "simplices of dimension" << input_params.dim;
        size = data->bifiltration_data->get_size(input_params.dim + 1);
        debug() << "There are" << size << "simplices of dimension" << input_params.dim + 1;
    }

    data->free_implicit_rep.reset(new FIRep(*(data->bifiltration_data), input_params.verbosity));
    data->x_reverse=x_reverse;
    data->y_reverse=y_reverse;

    //clean up
    if (!hasFunction) {
        delete degree;
    }

    return FileContent(data);
} //end read_discrete_metric_space()

//reads a bifiltration and stores in BifiltrationData
FileContent InputManager::read_bifiltration(std::ifstream& stream, Progress& progress)
{
    auto data = new InputData();
    FileInputReader reader(stream);
    if (verbosity >= 2) {
        debug() << "InputManager: Found a bifiltration file.\n";
    }


    //Skip file type line
    reader.next_line();

    //read the label for x-axis
    auto is_xreversed_and_label=detect_axis_reversed(reader.next_line().first);
    bool x_reverse=is_xreversed_and_label.first;
    data->x_label = is_xreversed_and_label.second;
    exact xrev_sign=x_reverse? -1:1;


    //read the label for y-axis
    auto is_yreversed_and_label=detect_axis_reversed(reader.next_line().first);
    bool y_reverse=is_yreversed_and_label.first;
    data->y_label = is_yreversed_and_label.second;
    exact yrev_sign=y_reverse? -1:1;

    data->bifiltration_data.reset(new BifiltrationData(input_params.dim, input_params.verbosity));

    //temporary data structures to store grades
    ExactSet x_set; //stores all unique x-values; must DELETE all elements later!
    ExactSet y_set; //stores all unique x-values; must DELETE all elements later!
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
                ret = x_set.insert(ExactValue(xrev_sign*str_to_exact(tokens.at(pos))));
                (ret.first)->indexes.push_back(num_grades);
                ret = y_set.insert(ExactValue(yrev_sign*str_to_exact(tokens.at(pos + 1))));
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

    data->x_reverse=x_reverse;
    data->y_reverse=y_reverse;

    return FileContent(data);
} //end read_bifiltration()

//reads a firep and stores in FIRep, does not create BifiltrationData
FileContent InputManager::read_firep(std::ifstream& stream, Progress& progress)
{
    auto data = new InputData;
    FileInputReader reader(stream);
    if (verbosity >= 2) {
        debug() << "InputManager: Found a firep file.\n";
    }
    bool x_reverse=false;
    bool y_reverse=false;

    //Skip file type line
    reader.next_line();

    data->bifiltration_data.reset(new BifiltrationData(input_params.dim, input_params.verbosity)); //Will be dummy bifiltration with no stored data

    //temporary data structures to store grades
    ExactSet x_set; //stores all unique x-values; must DELETE all elements later!
    ExactSet y_set; //stores all unique x-values; must DELETE all elements later!
    std::pair<ExactSet::iterator, bool> ret; //for return value upon insert()

    //Temporary data structures to store matrices
    int t, s, r;
    std::vector<exact> x_values, y_values;
    std::vector<std::vector<unsigned>> d2, d1; //matrices d2: hom_dim+1->hom_dim, d1: hom_dim->hom_dim-1

    //read simplices
    while (reader.has_next_line()) {
        auto line_info = reader.next_line();
        try {
            //read the label for x-axis
            auto is_xreversed_and_label=detect_axis_reversed(line_info.first);
            x_reverse=is_xreversed_and_label.first;
            data->x_label = is_xreversed_and_label.second;
            exact xrev_sign=x_reverse? -1:1;


            line_info = reader.next_line();
            //read the label for y-axis
            auto is_yreversed_and_label=detect_axis_reversed(line_info.first);
            y_reverse=is_yreversed_and_label.first;
            data->y_label = is_yreversed_and_label.second;
            exact yrev_sign=y_reverse? -1:1;

            //Read dimensions of matrices
            line_info = reader.next_line();
            std::vector<std::string> tokens = line_info.first;

            if (tokens.size() != 3) {
                throw InputError(line_info.second,
                    "Expected 3 tokens");
            }

            t = std::stoi(line_info.first[0]);
            s = std::stoi(line_info.first[1]);
            r = std::stoi(line_info.first[2]);

            //read matrices
            //Read d2 first
            x_values.reserve(t + s);
            y_values.reserve(t + s);
            for (int i = 0; i < t; i++) {
                d2.push_back(std::vector<unsigned>());
                line_info = reader.next_line();
                tokens = line_info.first;
                x_values.push_back(xrev_sign*str_to_exact(tokens.at(0)));
                y_values.push_back(yrev_sign*str_to_exact(tokens.at(1)));
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
                    if (v < 0 || v >= s) {
                        throw InputError(line_info.second, "Matrix index input out of bounds.");
                    }
                    d2[i].push_back(v);
                }
            }

            //read d1
            for (int i = 0; i < s; i++) {
                d1.push_back(std::vector<unsigned>());
                line_info = reader.next_line();
                tokens = line_info.first;
                x_values.push_back(xrev_sign*str_to_exact(tokens.at(0)));
                y_values.push_back(yrev_sign*str_to_exact(tokens.at(1)));
                //store value, if it doesn't exist already
                ret = x_set.insert(ExactValue(x_values[i + t]));
                //remember that point i has this value
                (ret.first)->indexes.push_back(i + t);

                //store value, if it doesn't exist already
                ret = y_set.insert(ExactValue(y_values[i + t]));
                //remember that point i has this value
                (ret.first)->indexes.push_back(i + t);

                //Process ith column
                if (tokens.at(2).at(0) != ';') {
                    throw InputError(line_info.second, "Expected ';' after coordinates");
                }
                for (unsigned pos = 3; pos < tokens.size(); pos++) {
                    int v = std::stoi(tokens[pos]);
                    if (v < 0 || v >= r) {
                        throw InputError(line_info.second, "Matrix index input out of bounds.");
                    }
                    d1[i].push_back(v);
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
    std::vector<unsigned> x_indexes(t + s, max_unsigned); //x_indexes[i] gives the discrete x-index for simplex i in the input order
    std::vector<unsigned> y_indexes(t + s, max_unsigned); //y_indexes[i] gives the discrete y-index for simplex i in the input order

    build_grade_vectors(*data, x_set, x_indexes, data->x_exact, input_params.x_bins);
    build_grade_vectors(*data, y_set, y_indexes, data->y_exact, input_params.y_bins);

    //Set x_grades and y_grades
    data->bifiltration_data->set_xy_grades(data->x_exact.size(), data->y_exact.size());
    data->free_implicit_rep.reset(new FIRep(*(data->bifiltration_data), t, s, r, d2, d1, x_indexes, y_indexes, input_params.verbosity));

    data->x_reverse=x_reverse;
    data->y_reverse=y_reverse;

    return FileContent(data);
} //end read_firep()

//reads a file of previously-computed data from RIVET
FileContent InputManager::read_RIVET_data(std::ifstream& stream, Progress& progress)
{
    auto data = new InputData();
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

    return FileContent(data);
} //end read_RIVET_data()

//converts an ExactSet of values to the vectors of discrete
// values that BifiltrationTree uses to build the bifiltration,
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
