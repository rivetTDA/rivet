//
// Created by bkeller on 11/22/16.
//

#include "grades.h"

Grades::Grades()
    : x()
    , y()
{
}

Grades::Grades(std::vector<exact> x, std::vector<exact> y)
    : x(rivet::numeric::to_doubles(x))
    , y(rivet::numeric::to_doubles(y))
{
}

double Grades::min_offset()
{
    if (x.empty() || y.empty()) {
        return 0;
    }
    return std::min(-1 * x.back(), y.front());
}

double Grades::max_offset()
{
    if (x.empty() || y.empty()) {
        return 0;
    }
    return std::max(y.back(), -1 * x.front());
}

double Grades::relative_offset_to_absolute(double offset)
{
    if (offset < 0 || offset > 1) {
        throw std::runtime_error("offset must be between 0 and 1 (inclusive)");
    }
    auto diff = max_offset() - min_offset();
    return min_offset() + (diff * offset);
}
