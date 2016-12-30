/**********************************************************************
Copyright 2014-2016 The RIVET Devlopers. See the COPYRIGHT file at
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

#include "numerics.h"
namespace rivet {

namespace numeric {
    std::vector<double> to_doubles(const std::vector<exact> exacts)
    {
        std::vector<double> doubles(exacts.size());
        std::transform(exacts.begin(), exacts.end(), doubles.begin(), [](exact num) {
            return numerator(num).convert_to<double>() / denominator(num).convert_to<double>();
        });
        return doubles;
    }

    bool is_number(const std::string& s)
    {
        std::string::const_iterator it = s.begin();
        if (it != s.end() && *it == '-')
            ++it;
        while (it != s.end() && (std::isdigit(*it) || *it == '.'))
            ++it;
        return !s.empty() && it == s.end();
    }

    //helper function to convert a string to an exact (rational)
    //accepts string such as "12.34", "765", and "-10.8421"
    exact str_to_exact(const std::string& str)
    {
        if (!is_number(str)) {
            throw std::runtime_error("'" + str + "' is not a number");
        }
        exact r;

        //find decimal point, if it exists
        std::string::size_type dec = str.find(".");

        if (dec == std::string::npos) //then decimal point not found
        {
            r = exact(str);
        } else //then decimal point found
        {
            //get whole part and fractional part
            std::string whole = str.substr(0, dec);
            std::string frac = str.substr(dec + 1);
            unsigned exp = frac.length();

            //test for negative, and remove minus sign character
            bool neg = false;
            if (whole.length() > 0 && whole[0] == '-') {
                neg = true;
                whole.erase(0, 1);
            }

            //remove leading zeros (otherwise, c++ thinks we are using octal numbers)
            std::string num_str = whole + frac;
            boost::algorithm::trim_left_if(num_str, boost::is_any_of("0"));

            //now it is safe to convert to rational
            std::istringstream s(num_str);
            boost::multiprecision::cpp_int num;
            s >> num;
            boost::multiprecision::cpp_int ten = 10;
            boost::multiprecision::cpp_int denom = boost::multiprecision::pow(ten, exp);

            r = exact(num, denom);
            if (neg)
                r = -1 * r;
        }
        return r;
    }

    //computes the projection of the lower-left corner of the line-selection window onto the specified line
    double project_zero(double angle, double offset, double x_0, double y_0)
    {
        if (angle == 0) //then line is horizontal
            return x_0;

        if (angle == 90) //then line is vertical
            return y_0;

        //if we get here, then line is neither horizontal nor vertical
        double radians = angle * PI / 180;

        if (y_0 > x_0 * tan(radians) + offset / cos(radians)) //then point is above line
            return y_0 / sin(radians) - offset / tan(radians); //project right

        return x_0 / cos(radians) + offset * tan(radians); //project up
    } //end project_zero()
}
}
