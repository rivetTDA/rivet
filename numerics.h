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
//
// Created by Bryn Keller on 6/21/16.
//

#ifndef RIVET_CONSOLE_NUMERICS_H
#define RIVET_CONSOLE_NUMERICS_H

#include <boost/algorithm/string.hpp>
#include <boost/multi_array.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <string>

typedef boost::multiprecision::cpp_rational exact;

typedef boost::multi_array<unsigned, 2> unsigned_matrix;

namespace rivet {
namespace numeric {
    exact str_to_exact(const std::string& str);
    bool is_number(const std::string& str);
    std::vector<double> to_doubles(const std::vector<exact> exacts);
    double project_zero(double angle, double offset, double x_0, double y_0);
    const double INFTY(std::numeric_limits<double>::infinity());
    const double PI(3.14159265358979323846);
}
}

#endif //RIVET_CONSOLE_NUMERICS_H
