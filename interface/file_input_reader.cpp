#include "file_input_reader.h"

#include <boost/algorithm/string.hpp>

FileInputReader::FileInputReader(std::ifstream file) :
    in(&file),
    next_line_found(false)
{
    find_next_line();
}

//finds the next line in the file that is not empty and not a comment, if such line exists
//this is the only function that should call in.readLine()
void FileInputReader::find_next_line()
{
  std::string line;
  while(std::getline(in, line)) {
    boost::trim(line);
    if (line.empty() || line[0] == '#')
      continue;
    next_line_tokens = std::vector<std::string>;
    boost::split(next_line_tokens, line, boost::is_any_of("\t\n\r\b "));
  }
}

//indicates whether another line can be returned
bool FileInputReader::has_next_line()
{
    return next_line_found;
}

//returns the next line as a std::vector<std::string> of tokens
std::vector<std::string> FileInputReader::next_line()
{
    std::vector<std::string> current = next_line_tokens;

    next_line_found = false;
    find_next_line();

    return current;
}
