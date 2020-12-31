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
    //accepts string such as "12.34", "765", "-10.8421", "23.8e5", "60.31e-04"
    exact str_to_exact(const std::string& str)
    {
        exact r; //this will hold the result

        //first look for "e", indicating scientific notation
        std::string::size_type sci = str.find("e");

        unsigned exponent = 0;
        bool pos_exp = true;
        std::string base_str = str;

        if (sci != std::string::npos) { //then scientific notation detected
            //get the base and exponent strings
            std::string exp_str = str.substr(sci + 1);
            base_str = str.substr(0, sci);

            //test for negative, and sign character
            if (exp_str.length() > 0) {
                if (exp_str[0] == '-') {
                    pos_exp = false;
                    exp_str.erase(0, 1);
                } else if (exp_str[0] == '+') {
                    exp_str.erase(0, 1);
                }
            }

            //remove leading zeros (otherwise, c++ thinks we are using octal numbers)
            boost::algorithm::trim_left_if(exp_str, boost::is_any_of("0"));

            //confirm that exp_str contains only digits; otherwise error
            std::string::const_iterator it = exp_str.begin();
            while (it != exp_str.end() && (std::isdigit(*it) || *it == '.'))
                ++it;
            if ( exp_str.empty() || it != exp_str.end() ) {
                throw std::runtime_error("'" + exp_str + "' is not a number");
            }

            //now convert exp_str to numeric type
            std::istringstream s(exp_str);
            s >> exponent;
        }

        //the exponent has been removed, so now process the base
        //simple numeric check
        if (!is_number(base_str)) {
            throw std::runtime_error("'" + base_str + "' is not a number");
        }

        //find decimal point, if it exists
        std::string::size_type dec = base_str.find(".");

        boost::multiprecision::cpp_int ten = 10;

        if (dec == std::string::npos) { //then decimal point not found
            std::istringstream s(base_str);
            boost::multiprecision::cpp_int num;
            s >> num;

            if (exponent == 0) {
                r = exact(num);
            } else {
                boost::multiprecision::cpp_int exp_int = boost::multiprecision::pow(ten, exponent);
                if (pos_exp) {
                    r = exact(num * exp_int);
                } else {
                    r = exact(num, exp_int);
                }
            }

        } else { //then decimal point found
            //get whole part and fractional part
            std::string whole = base_str.substr(0, dec);
            std::string frac = base_str.substr(dec + 1);
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
            
            boost::multiprecision::cpp_int denom = boost::multiprecision::pow(ten, exp);

            if (exponent != 0) {
                if (pos_exp) {
                    num = num * boost::multiprecision::pow(ten, exponent);
                } else {
                    denom = denom * boost::multiprecision::pow(ten, exponent);
                }
            }

            r = exact(num, denom);
            
            if (neg)
                r = -1 * r;
        }

        return r;
    }

    //computes the projection of the lower-left corner of the line-selection window onto the specified line
    //  NOTE: parametrization of the line is as in the RIVET paper
    //  this function is similar to BarcodeTemplate::project()
    double project_to_line(double angle, double offset, double x_0, double y_0)
    {
        if (angle == 0) //then line is horizontal
            return x_0;

        if (angle == 90) //then line is vertical
            return y_0;

        //if we get here, then line is neither horizontal nor vertical
        double radians = angle * PI / 180;

        double yL = x_0 * tan(radians) + offset / cos(radians); // the point (x_0, yL) is on the line

        if (y_0 >= yL) { //then point is above line, so project to the right
            if (offset >= 0) {
                return (y_0 * cos(radians) - offset) / (sin(radians) * cos(radians));
            } //else
            return y_0 / sin(radians);
        } //else: point is below the line, so project up
        if (offset >= 0) {
            return x_0 / cos(radians);
        } //else 
        return yL / sin(radians);
    } //end project_zero()
}
}
