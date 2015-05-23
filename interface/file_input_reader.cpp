#include "file_input_reader.h"

#include <QChar>
#include <QString>

FileInputReader::FileInputReader(QFile& file) :
    in(&file),
    next_line_found(false)
{
    find_next();
}

void FileInputReader::find_next()
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
    }
}

bool FileInputReader::has_next()
{
    return next_line_found;
}

QStringList FileInputReader::next_line()   ///TODO: maybe too much copying of QStringLists?
{
    QStringList current = next_line_tokens;

    next_line_found = false;
    find_next();

    return current;
}

