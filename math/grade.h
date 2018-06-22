/**********************************************************************
 Copyright 2014-2017 The RIVET Developers. See the COPYRIGHT file at
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

/*

 Author: Roy Zhao (2017)
 
 Struct: Grade
 
 Description: Pair of coordinates specifying grade of appearance,
 together with an operator < which compares COLEXICOGRAPHICALLY,
 i.e., first by y-coordinate,  then by x-coordinate.
 NOTE: This Struct appeared earlier in bifiltration_data.h.  Since the same struct is
 useful elsewhere, it is better in its own file.
 
*/

#ifndef grade_h
#define grade_h

struct Grade {
    int x;
    int y;

    bool operator==(const Grade& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator<(const Grade& other) const
    {
        if (y != other.y)
            return y < other.y;
        else
            return x < other.x;
    }

    Grade() {}

    Grade(int set_x, int set_y)
        : x(set_x)
        , y(set_y)
    {
    }
};

#endif /* grade_h */
