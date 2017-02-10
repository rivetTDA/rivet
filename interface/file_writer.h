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
 * \brief	Saves an augmented arrangement to a file.
 * \author	Matthew L. Wright
 * \date	2016
 */

#ifndef FILEWRITER_H
#define FILEWRITER_H

#include "dcel/arrangement.h"
#include "input_manager.h"
#include "input_parameters.h"
#include "math/template_point.h"

#include <fstream>

#include <boost/multiprecision/cpp_int.hpp>

#include <dcel/arrangement_message.h>
#include <vector>

class FileWriter {
public:
    FileWriter(InputParameters& ip, InputData& input, Arrangement& m, std::vector<TemplatePoint>& points);

    template <typename T>
    static T& write_grades(T& stream, const std::vector<exact>& x_exact, const std::vector<exact>& y_exact)
    {

        //write x-grades
        stream << "x-grades" << std::endl;
        for (std::vector<exact>::const_iterator it = x_exact.cbegin(); it != x_exact.cend(); ++it) {
            std::ostringstream oss;
            oss << *it;
            stream << oss.str() << std::endl;
        }
        stream << std::endl;

        //write y-grades
        stream << "y-grades" << std::endl;
        for (std::vector<exact>::const_iterator it = y_exact.cbegin(); it != y_exact.cend(); ++it) {
            std::ostringstream oss;
            oss << *it;
            stream << oss.str() << std::endl;
        }
        stream << std::endl;
        return stream;
    }

    void write_augmented_arrangement(std::ofstream& file);

private:
    InputData& input_data;
    InputParameters& input_params;
    Arrangement& arrangement; //reference to the DCEL arrangement
    std::vector<TemplatePoint>& template_points; //stores discrete coordinates of xi support points, with multiplicities
};

#endif // FILEWRITER_H
