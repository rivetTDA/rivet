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

#include "file_input_reader.h"
#include <iostream>

#include <boost/algorithm/string.hpp>

FileInputReader::FileInputReader(std::ifstream& file)
    : in(file)
    , line_number(0)
    , next_line_found(false)
{
    find_next_line(0);
}

// if it starts with -- or -<non-digit> or <non-digit & non-dash>, it is a flag
bool FileInputReader::is_flag(std::string str)
{
    // use ASCII values to check if digit or not
    int first = (char)str[0];
    int second = (char)str[1];

    if (first == 45 && second == 45) return true;
    if (first == 45 && (second < 48 || second > 57)) return true;
    if (first != 45 && (first < 48 || first > 57)) return true;

    return false;
}

//finds the next line in the file that is not empty and not a comment, if such line exists
//this is the only function that should call in.readLine()
void FileInputReader::find_next_line(int old)
{
    std::string line;
    while (std::getline(in, line)) {
        line_number++;
        boost::trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        next_line_tokens.clear();
        // old = 1 means old file format, so split by space
        if (old || line.find(",") == std::string::npos || is_flag(line)) {
            boost::split(next_line_tokens, line, boost::is_space(std::locale()), boost::token_compress_on);
        }
        // if a "," is there in the line, split by ","
        else {
            boost::split(next_line_tokens, line, boost::is_any_of(", "), boost::token_compress_on);
        }
        next_line_found = true;
        break;
    }
}

//indicates whether another line can be returned
bool FileInputReader::has_next_line()
{
    return next_line_found;
}

//returns the next line as a std::vector<std::string> of tokens
std::pair<std::vector<std::string>, unsigned> FileInputReader::next_line(int old)
{
    std::vector<std::string> current = next_line_tokens;
    auto num = line_number;
    next_line_found = false;
    find_next_line(old);

    return std::make_pair(current, num);
}
