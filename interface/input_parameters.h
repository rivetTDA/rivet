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
//TODO: this class currently conflates 3 things: command line arguments, file load dialog arguments, and viewer configuration state

//these parameters are set by the user via the console or the DataSelectDialog before computation can begin
struct InputParameters {
    std::string fileName; //name of data file
    std::string shortName; //name of data file, without path
    std::string outputFile; //name of the file where the augmented arrangement should be saved
    int dim; //dimension of homology to compute
    unsigned x_bins; //number of bins for x-coordinate (if 0, then bins are not used for x)
    unsigned y_bins; //number of bins for y-coordinate (if 0, then bins are not used for y)
    int verbosity; //controls the amount of console output printed
    std::string x_label; //used by configuration dialog
    std::string y_label; //used by configuration dialog
    std::string outputFormat; // Supported values: R0, R1
    
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar& fileName& shortName& outputFile& dim& x_bins& y_bins& verbosity& x_label& y_label& outputFormat;
    }
    MSGPACK_DEFINE(fileName, shortName, outputFile, dim, x_bins, y_bins, verbosity, x_label, y_label, outputFormat);

};

#endif // INPUT_PARAMETERS_H
