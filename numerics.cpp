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
            throw std::runtime_error("Error: " + str + " is not a number");
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
}
}
