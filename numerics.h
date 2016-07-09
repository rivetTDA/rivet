//
// Created by Bryn Keller on 6/21/16.
//

#ifndef RIVET_CONSOLE_NUMERICS_H
#define RIVET_CONSOLE_NUMERICS_H

#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multi_array.hpp>

typedef boost::multiprecision::cpp_rational exact;

typedef boost::multi_array<unsigned, 2> unsigned_matrix;

namespace rivet
{
    namespace numeric {
        exact str_to_exact(const std::string& str);
        bool is_number(const std::string& str);
        std::vector<double> to_doubles(const std::vector<exact> exacts);
    }

}


#endif //RIVET_CONSOLE_NUMERICS_H
