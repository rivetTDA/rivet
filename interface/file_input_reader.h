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

    //true iff the file has another line of printable, non-commented characters
    bool has_next_line();
    //returns the next line, as a vector of strings, plus the line number at which the line was found
    std::pair<std::vector<std::string>, unsigned> next_line();

private:
    std::ifstream& in;
    unsigned line_number;
    bool next_line_found;
    std::vector<std::string> next_line_tokens;
    void find_next_line();
};

#endif // FILEINPUTREADER_H
