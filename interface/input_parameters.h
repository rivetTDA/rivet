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

#ifndef INPUT_PARAMETERS_H
#define INPUT_PARAMETERS_H

#include <string>

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;
//TODO: this class currently conflates 3 things: command line arguments, file load dialog arguments, and viewer configuration state

// Error that is raised when the input file does not match format correctly
class InputError : public std::runtime_error {
public:
    InputError(unsigned line, std::string message)
        : std::runtime_error("line " + std::to_string(line) + ": " + message)
    {
    }
};

//these parameters are set by the user via the console or the DataSelectDialog before computation can begin
struct InputParameters {
    std::string fileName; //name of data file
    std::string shortName; //name of data file, without path
    std::string outputFile; //name of the file where the augmented arrangement should be saved
    int hom_degree; //degree of homology to compute
    int x_bins; //number of bins for x-coordinate (if 0, then bins are not used for x)
    int y_bins; //number of bins for y-coordinate (if 0, then bins are not used for y)
    int verbosity; //controls the amount of console output printed
    std::string x_label; //used by configuration dialog
    std::string y_label; //used by configuration dialog
    std::string outputFormat; // Supported values: R0, R1
    int num_threads; //number of openmp threads to be created
    bool binary; //include binary data in MI file
    bool minpres; //print minimal presentation and exit
    bool betti; //print betti number information
    bool bounds; //print lower and upper bounds of module in MI file
    bool koszul; //use koszul homology based algorithm
    exact max_dist; //maximum distance to be considered while building Rips complex
    std::string md_string; //holds max distance in string format
    unsigned dimension; //dimension of the space where the points lie
    bool old_function; //specifies if the data has a function value like the old format
    bool new_function; //specifies if the data has a --function flag followed by values
    bool x_reverse, y_reverse; //specifies if the axes need to be reversed or not
    std::string type; //type of file being worked with
    std::string bifil; //type of bifiltration to build
    int to_skip; //number of lines after which the actual data begins
    std::string filtration; //type of filtration to perform on data without function
    double filter_param; //parameter value for performing the filtration

    InputParameters()
    {
        // default values for all input parameters - should include first 3?
        type = "points";
        bifil = "";
        hom_degree = 0;
        x_bins = 0;
        y_bins = 0;
        verbosity = 0;
        outputFormat = "msgpack";
        num_threads = 0;
        old_function = false;
        new_function = false;
        max_dist = -1;
        dimension = 0;
        x_reverse = false;
        y_reverse = false;
        x_label = "";
        y_label = "distance";
        binary = false;
        minpres = false;
        betti = false;
        bounds = false;
        koszul = false;
    }

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar& fileName& shortName& outputFile& hom_degree& x_bins& y_bins& verbosity& x_label& y_label& outputFormat;
    }
    MSGPACK_DEFINE(fileName, shortName, outputFile, hom_degree, x_bins, y_bins, verbosity, x_label, y_label, outputFormat);
};

#endif // INPUT_PARAMETERS_H
