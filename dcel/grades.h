//
// Created by Bryn Keller on 11/22/16.
//

#ifndef RIVET_CONSOLE_GRADES_H
#define RIVET_CONSOLE_GRADES_H

#include <numerics.h>
#include <vector>

struct Grades {
    std::vector<double> x;
    std::vector<double> y;

    Grades();

    Grades(std::vector<exact> x, std::vector<exact> y);

    double min_offset();
    double max_offset();

    double relative_offset_to_absolute(double offset);
};

#endif //RIVET_CONSOLE_GRADES_H
