#include "file_input_reader.h"

#include <QChar>
#include <QString>
#include <QDebug>

FileInputReader::FileInputReader(QFile& file) :
    in(&file),
    next_line_found(false)
{
    find_next_line();
}

//finds the next line in the file that is not empty and not a comment, if such line exists
//this is the only function that should call in.readLine()
void FileInputReader::find_next_line()
{
    QChar comment('#');
    while( !in.atEnd() && !next_line_found )
    {
        next_line_string = in.readLine().trimmed();
        if( line.isEmpty() || line.at(0) == comment)    //then skip this line
            continue;
        //else -- found a nonempty next line
        next_line_found = true;
        next_line_tokens = line.split(QRegExp("\\s+"));
        next_list_position = 0;
    }
}

//indicates whether another line can be returned
bool FileInputReader::has_next_line()
{
    return next_line_found;
}

//returns the next line as a QStringList of tokens
QStringList FileInputReader::next_line()   ///TODO: maybe too much copying of QStringLists?
{
    QStringList current = next_line_tokens;

    next_line_found = false;
    find_next_line();

    return current;
}

//returns the next line as a QString
QString FileInputReader::next_line_str()
{
    QString current = next_line_string;

    next_line_found = false;
    find_next_line();

    return current;
}

//indicates whether another token can be returned from the line
bool FileInputReader::has_next_token()
{
    if(next_list_position < next_line_tokens.size())    //then there is another token in the current line
        return true;

    //otherwise, see if there is another nonempty line
    next_line_found = false;
    find_next_line();
    return next_line_found;
}

//returns the next token
QString FileInputReader::next_token()
{
    return next_line_tokens.at(next_list_position++);
}
