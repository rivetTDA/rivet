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
