/**
 * \brief	reads a file, ignoring white space and comments. returns tokens by line or individually.
 * \author	matthew l. wright
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
