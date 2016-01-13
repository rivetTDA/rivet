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

void FileInputReader::find_next_line()
{
    QChar comment('#');
    while( !in.atEnd() && !next_line_found )
    {
        QString line = in.readLine().trimmed();
        if( line.isEmpty() || line.at(0) == comment)    //then skip this line
            continue;
        //else -- found a nonempty next line
        next_line_found = true;
        next_line_tokens = line.split(QRegExp("\\s+"));
        next_list_position = 0;
    }
}

bool FileInputReader::has_next_line()
{
    return next_line_found;
}

QStringList FileInputReader::next_line()   ///TODO: maybe too much copying of QStringLists?
{
    QStringList current = next_line_tokens;

    next_line_found = false;
    find_next_line();

    return current;
}

bool FileInputReader::has_next_token()
{
    if(next_list_position < next_line_tokens.size())    //then there is another token in the current line
        return true;

    //otherwise, see if there is another nonempty line
    next_line_found = false;
    find_next_line();
    return next_line_found;
}

QString FileInputReader::next_token()
{
    return next_line_tokens.at(next_list_position++);
}
