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
 * \class	InputManager
 * \brief	The InputManager is able to identify the type of input
 */

#ifndef __InputManager_H__
#define __InputManager_H__

#include "dcel/barcode_template.h"
#include "interface/file_input_reader.h"
#include "interface/input_parameters.h"

#include <fstream>
#include <sstream>
#include <vector>

//TODO: the input manager doesn't really hold an appreciable
//amount of state, there's really no reason to instantiate a class
//for this job, a collection of functions would do.

//now the InputManager class
class InputManager {
public:
    InputManager(InputParameters& input_params);

    void start(); //function that parses input file for key values

    static std::string join(const std::vector<std::string>& strings)
    {
        std::stringstream ss;
        for (size_t i = 0; i < strings.size(); i++) {
            if (i > 0)
                ss << " ";
            ss << strings[i];
        }
        return ss.str();
    }

    static std::vector<std::string> split(std::string& str, std::string separators)
    {
        std::vector<std::string> strings;
        boost::split(strings, str, boost::is_any_of(separators));
        return strings;
    }

private:
    InputParameters& input_params; //parameters supplied by the user
    int verbosity; //controls display of output, for debugging

    int to_skip; // stores the number of non-data lines in the input file

    void parse_args(); // goes through supplied arguments and sets parameters

    // methods to support parsing the old file formats
    void parse_points_old();
    void parse_metric_old();
    void parse_bifiltration_old();
    void parse_firep_old();

    bool is_flag(std::string str); // determines if a line in the input file is an input parameter
};

#endif // __InputManager_H__
