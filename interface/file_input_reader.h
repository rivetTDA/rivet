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
/**
 * \brief	Reads a file, ignoring white space and comments. returns tokens by line or individually.
 * \author	Matthew L. Wright
 * \date	2015
 */

#ifndef FILEINPUTREADER_H
#define FILEINPUTREADER_H

#include <fstream>
#include <vector>

class FileInputReader : public std::iterator<std::output_iterator_tag,
                            std::vector<std::string>,
                            std::ptrdiff_t,
                            std::vector<std::string>*,
                            std::vector<std::string>&> {
public:
    FileInputReader(std::ifstream& file); //constructor

    bool has_next_line(); //true iff the file has another line of printable, non-commented characters
    std::vector<std::string> next_line(); //returns the next line, as a vector of strings

private:
    std::ifstream& in;
    bool next_line_found;
    std::vector<std::string> next_line_tokens;
    void find_next_line();
};

#endif // FILEINPUTREADER_H
