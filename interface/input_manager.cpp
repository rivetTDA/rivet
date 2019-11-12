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
#include "../debug.h"
#include "file_input_reader.h"
#include "input_parameters.h"

#include <boost/algorithm/string.hpp>
#include <sstream>
#include <vector>

//a function to detect the axis reversal flag
//the first return value is true iff the axis is reversed
//and the second is the axis label
std::pair<bool, std::string> detect_axis_reversed(std::vector<std::string> line)
{

    bool is_reversed = false;
    std::string label;

    std::string joined = InputManager::join(line);

    if (boost::starts_with(line[0], "[")) {
        auto close_position = joined.find_first_of("]");
        if (close_position == std::string::npos) {
            throw std::runtime_error("No closing bracket in axis label");
        }
        //get the nonwhitespace characters between the two brackets
        std::string between = joined.substr(1, close_position - 1);
        boost::trim(between);

        if (between == "-") {
            is_reversed = true;
        }
        //if there is a +, or more than one whitespace character before the closing bracket, then don't do anything
        //if there is exactly one non "+" charachter before the closing bracket, throw an error

        else if (between != "+" && between.length() == 1) {
            throw std::runtime_error("An invalid character was received for the axis direction");
        }
        //if we are at this point, then line[0] begins with either [+] or [-]
        //the label is the rest of the string
        //the trim is there because there may or may not be whitespace between the bracket and the label
        label = joined.substr(close_position + 1);
        boost::trim(label);

    }

    else {
        //there is no direction flag, the xlabel is just the line
        //and x_reverse retains its default value of false
        label = joined;
    }

    return std::make_pair(is_reversed, label);
}

//==================== InputManager class ====================
using namespace rivet::numeric;

//constructor
InputManager::InputManager(InputParameters& params)
    : input_params(params)
{
}

//function to run the input manager, requires a filename
//parses the file for input parameters
void InputManager::start()
{
    // parses everything before data
    // sets up input_params
    parse_args();

} //end start()

// if it starts with -- or -<non-digit> or <non-digit>, it is a flag
bool InputManager::is_flag(std::string str)
{
    // use ASCII values to check if digit or not
    int first = (char)str[0];
    int second = (char)str[1];

    if (first == 45 && second == 45)
        return true;
    if (first == 45 && (second < 48 || second > 57))
        return true;
    if (first != 45 && (first < 48 || first > 57))
        return true;

    return false;
}

// function to parse all flags supplied in input file
// sets up input_params
void InputManager::parse_args()
{
    // open as a separate file, not the reference
    std::ifstream input_file(input_params.fileName);
    FileInputReader reader(input_file);

    std::pair<std::vector<std::string>, unsigned> line_info;
    int num_lines = 0;
    type_set = false;

    try {
        while (reader.has_next_line()) {
            line_info = reader.next_line(0);
            // support for old file format
            if (line_info.first[0] == "points" || line_info.first[0] == "metric" || line_info.first[0] == "bifiltration" || line_info.first[0] == "firep" || line_info.first[0] == "RIVET_msgpack") {
                input_params.type = line_info.first[0];
                input_file.close();
                type_set = true;
                if (input_params.type == "points")
                    parse_points_old();
                if (input_params.type == "metric")
                    parse_metric_old();
                if (input_params.type == "bifiltration")
                    parse_bifiltration_old();
                if (input_params.type == "firep")
                    parse_firep_old();
                return;
            }
            // check if line contains flags or data
            if (!is_flag(line_info.first[0]))
                break;
            num_lines++;

            std::vector<std::string> line = line_info.first;

            // determine input parameter being set
            // handle error checking here
            if (line[0] == "--maxdist") {
                try {
                    if (line[1] == "inf") {
                        input_params.max_dist = -1;
                        input_params.md_string = "inf";
                    } else {
                        // max distance cannot be less than 0
                        exact dist = str_to_exact(line[1]);
                        if (dist <= 0)
                            throw std::runtime_error("Error");
                        input_params.max_dist = dist;
                        input_params.md_string = line[1];
                    }
                } catch (std::exception& e) {
                    throw std::runtime_error("Invalid argument for --maxdist");
                }
            } else if (line[0] == "--homology" || line[0] == "-H") {
                try {
                    // homology degree cannot be less than 0
                    int hom = std::stoi(line[1]);
                    if (hom < 0)
                        throw std::runtime_error("Error");
                    input_params.hom_degree = hom;
                } catch (std::exception& e) {
                    throw std::runtime_error("Invalid argument for --homology");
                }
            } else if (line[0] == "--xbins" || line[0] == "-x") {
                try {
                    // x_bins degree cannot be less than 0
                    int x = std::stoi(line[1]);
                    if (x < 0)
                        throw std::runtime_error("Error");
                    input_params.x_bins = x;
                } catch (std::exception& e) {
                    throw std::runtime_error("Invalid argument for --xbins");
                }
            } else if (line[0] == "--ybins" || line[0] == "-y") {
                try {
                    // y_bins degree cannot be less than 0
                    int y = std::stoi(line[1]);
                    if (y < 0)
                        throw std::runtime_error("Error");
                    input_params.y_bins = y;
                } catch (std::exception& e) {
                    throw std::runtime_error("Invalid argument for --ybins");
                }
            } else if (line[0] == "--verbosity" || line[0] == "-V") {
                try {
                    // verbosity is between 0 and 10
                    int v = std::stoi(line[1]);
                    if (v < 0 || v > 10)
                        throw std::runtime_error("Error");
                    input_params.verbosity = v;
                } catch (std::exception& e) {
                    throw std::runtime_error("Invalid argument for --verbosity");
                }
            } else if (line[0] == "--format" || line[0] == "-f") {
                try {
                    // output format is either R0 or msgpack
                    std::string o = line[1];
                    if (o != "R0" && o != "msgpack")
                        throw std::runtime_error("Error");
                    input_params.outputFormat = o;
                } catch (std::exception& e) {
                    throw std::runtime_error("Invalid argument for --format");
                }
            } else if (line[0] == "--numthreads") {
                try {
                    // number of threads cannot be less than 0
                    int nt = std::stoi(line[1]);
                    if (nt < 0)
                        throw std::runtime_error("Error");
                    input_params.num_threads = nt;
                } catch (std::exception& e) {
                    throw std::runtime_error("Invalid argument for --numthreads");
                }
            } else if (line[0] == "--xlabel") {
                input_params.x_label = "";
                // everything coming after --x-label is the label
                for (unsigned i = 1; i < line.size(); i++)
                    input_params.x_label += line[i] + " ";
            } else if (line[0] == "--ylabel") {
                input_params.y_label = "";
                // everything coming after --y-label is the label
                for (unsigned i = 1; i < line.size(); i++)
                    input_params.y_label += line[i] + " ";
            } else if (line[0] == "--function") {
                if (line[1] != "density" && line[1] != "eccentricity" && line[1] != "knn" && line[1] != "user")
                    throw std::runtime_error("Invalid argument for --function");
                input_params.function_type = line[1];
            } else if (line[0] == "--xreverse") {
                input_params.x_reverse = true;
            } else if (line[0] == "--yreverse") {
                input_params.y_reverse = true;
            } else if (line[0] == "--binary") {
                input_params.binary = true;
            } else if (line[0] == "--minpres") {
                input_params.minpres = true;
            } else if (line[0] == "--betti" || line[0] == "-b") {
                input_params.betti = true;
            } else if (line[0] == "--bounds") {
                input_params.bounds = true;
            } else if (line[0] == "--koszul" || line[0] == "-k") {
                input_params.koszul = true;
            } else if (line[0] == "--datatype") {
                // specifies file type, throw error if unknown
                if (line[1] != "points" && line[1] != "points_fn" && 
                    line[1] != "metric" && line[1] != "metric_fn" && 
                    line[1] != "bifiltration" && line[1] != "firep" && line[1] != "RIVET_msgpack")
                    throw std::runtime_error("Invalid argument for --type");
                input_params.type = line[1];
                type_set = true;
                if (line[1] == "points_fn" || line[1] == "metric_fn") {
                    input_params.new_function = true;
                }
            } else if (line[0] == "--bifil") {
                if (line[1] != "degree" && line[1] != "function")
                    throw std::runtime_error("Invalid argument for --bifil");
                input_params.bifil = line[1];
            } else {
                throw std::runtime_error("Invalid option" + line[0] + "at line " + std::to_string(line_info.second));
            }
        }
    } catch (std::exception& e) {
        throw InputError(line_info.second, e.what());
    }

    if ((input_params.type == "points" || input_params.type == "metric") && input_params.bifil == "function")
        throw std::runtime_error("Cannot create function rips without function values. If you have provided function values, please specify the correct data type.");

    // skip stores number of lines to skip
    input_params.to_skip = num_lines;
    // determine dimension in which points live
    if (input_params.new_function)
        line_info = reader.next_line(0);
    input_params.dimension = line_info.first.size();
    if (input_params.type == "metric" || input_params.type == "metric_fn") {
        line_info = reader.next_line(0);
        if (input_params.dimension == line_info.first.size())
            ;
        else if (input_params.dimension == line_info.first.size() + 1)
            input_params.dimension++; // this is the number of points for metric space
    }

    if (input_params.bifil == "") {
        if (input_params.new_function)
            input_params.bifil = "function";
        else
            input_params.bifil = "degree";
    }
    if (input_params.type == "metric" || input_params.type == "points") {
        input_params.x_reverse = true;
        input_params.y_reverse = false;
    }
    if (input_params.type == "metric_fn" || input_params.type == "points_fn") {
        input_params.y_reverse = false;
    }
    if (input_params.type == "firep") {
        input_params.x_reverse = false;
        input_params.y_reverse = false;
    }

    if (input_params.bifil == "function" && input_params.function_type == "none") {
        input_params.function_type = "user";
    }

    input_file.close();
}

// parse old point cloud input parameters
void InputManager::parse_points_old()
{
    std::ifstream input_file(input_params.fileName);
    FileInputReader reader(input_file);

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

        int dim = std::stoi(dimension_line[0]);
        if (dim < 1) {
            throw std::runtime_error("Dimension of data must be at least 1");
        }
        input_params.dimension = static_cast<unsigned>(dim);

        //read maximum distance for edges in Rips complex
        line_info = reader.next_line();
        std::vector<std::string> distance_line = line_info.first;
        if (distance_line.size() != 1) {
            throw std::runtime_error("There was more than one value in the expected distance line.");
        }

        input_params.max_dist = str_to_exact(distance_line[0]);

        if (input_params.max_dist <= 0) {
            throw std::runtime_error("An invalid input was received for the max distance.");
        }

        //read label for x-axis
        line_info = reader.next_line();
        if (InputManager::join(line_info.first).compare("no function") == 0) {
            input_params.old_function = false;
            input_params.bifil = "degree";
            //set label for x-axis to "degree"
            input_params.x_label = "degree";
            input_params.x_reverse = true; //higher degrees will be shown on the left

        } else {
            input_params.old_function = true;
            input_params.bifil = "function";
            auto is_reversed_and_label = detect_axis_reversed(line_info.first);
            input_params.x_reverse = is_reversed_and_label.first;
            input_params.x_label = is_reversed_and_label.second;
        }

        //set label for y-axis to "distance"
        input_params.y_label = "distance";

    } catch (std::exception& e) {
        throw InputError(line_info.second, e.what());
    }

    input_params.to_skip = 4;
    input_file.close();
}

// parse old metric space input parameters
void InputManager::parse_metric_old()
{

    std::ifstream input_file(input_params.fileName);
    FileInputReader reader(input_file);

    //skip 'metric'
    auto line_info = reader.next_line();
    line_info = reader.next_line();
    //first read the label for x-axis
    try {

        if (InputManager::join(line_info.first).compare("no function") == 0) {
            input_params.old_function = false;
            input_params.bifil = "degree";
            //set label for x-axis to "degree"
            input_params.x_label = "degree";

            //x_reverse is true in this case
            input_params.x_reverse = true;

            //now read the number of points
            line_info = reader.next_line();

            int dim = std::stoi(line_info.first[0]);
            if (dim < 1) {
                throw std::runtime_error("Number of points must be at least 1");
            }
            input_params.dimension = static_cast<unsigned>(dim);
        } else {
            // if function values are there
            input_params.old_function = true;
            input_params.bifil = "function";

            //check for axis reversal
            auto is_reversed_and_label = detect_axis_reversed(line_info.first);
            input_params.x_reverse = is_reversed_and_label.first;
            input_params.x_label = is_reversed_and_label.second;

            line_info = reader.next_line();

            // number of function values is number of points
            input_params.dimension = line_info.first.size();
        }

        //first read the label for y-axis
        line_info = reader.next_line();
        input_params.y_label = InputManager::join(line_info.first);

        //read the maximum length of edges to construct
        line_info = reader.next_line();
        input_params.max_dist = str_to_exact(line_info.first[0]);

    } catch (InputError& e) {
        throw;
    } catch (std::exception& e) {
        throw InputError(line_info.second, e.what());
    }

    input_params.to_skip = 5;
    input_file.close();
}

// parse old bifiltration input parameters
void InputManager::parse_bifiltration_old()
{
    std::ifstream input_file(input_params.fileName);
    FileInputReader reader(input_file);

    //Skip file type line
    reader.next_line();

    //read the label for x-axis
    auto is_xreversed_and_label = detect_axis_reversed(reader.next_line().first);
    input_params.x_reverse = is_xreversed_and_label.first;
    input_params.x_label = is_xreversed_and_label.second;

    //read the label for y-axis
    auto is_yreversed_and_label = detect_axis_reversed(reader.next_line().first);
    input_params.y_reverse = is_yreversed_and_label.first;
    input_params.y_label = is_yreversed_and_label.second;

    input_params.to_skip = 3;
}

// parse old firep input parameters
void InputManager::parse_firep_old()
{
    std::ifstream input_file(input_params.fileName);
    FileInputReader reader(input_file);

    //Skip file type line
    reader.next_line();

    //read the label for x-axis
    auto is_xreversed_and_label = detect_axis_reversed(reader.next_line().first);
    input_params.x_reverse = is_xreversed_and_label.first;
    input_params.x_label = is_xreversed_and_label.second;

    //read the label for y-axis
    auto is_yreversed_and_label = detect_axis_reversed(reader.next_line().first);
    input_params.y_reverse = is_yreversed_and_label.first;
    input_params.y_label = is_yreversed_and_label.second;

    input_params.to_skip = 3;
}
