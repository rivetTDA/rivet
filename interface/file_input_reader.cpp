#include "file_input_reader.h"

#include <boost/algorithm/string.hpp>

FileInputReader::FileInputReader(std::ifstream& file)
    : in(file)
    , line_number(0)
    , next_line_found(false)
{
    find_next_line();
}

//finds the next line in the file that is not empty and not a comment, if such line exists
//this is the only function that should call in.readLine()
void FileInputReader::find_next_line()
{
    std::string line;
    while (std::getline(in, line)) {
        line_number++;
        boost::trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        next_line_tokens.clear();
        boost::split(next_line_tokens, line, boost::is_space(std::locale()), boost::token_compress_on);
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
std::pair<std::vector<std::string>, unsigned> FileInputReader::next_line()
{
    std::vector<std::string> current = next_line_tokens;
    auto num = line_number;

    next_line_found = false;
    find_next_line();

    return std::make_pair(current, num);
}
