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
