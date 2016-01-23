/**
 * \brief	Reads a file, ignoring white space and comments. Returns tokens by line or individually.
 * \author	Matthew L. Wright
 * \date	2015
 */

#ifndef FILEINPUTREADER_H
#define FILEINPUTREADER_H

#include <QFile>
#include <QStringList>
#include <QTextStream>

class FileInputReader
{
    public:
        FileInputReader(QFile& file);   //constructor

        bool has_next_line();       //true iff the file has another line of printable, non-commented characters
        QStringList next_line();    //returns the next line, as a QStringList
        QString next_line_str();    //returns the next line as a QString

        bool has_next_token();      //true iff the file has more printable, non-commented characters -- caution, this might advance to the next line!
        QString next_token();       //returns the next token (advancing to the next line if necessary)

    private:
        QTextStream in;
        bool next_line_found;
        QString next_line_string;
        QStringList next_line_tokens;
        void find_next_line();
        int next_list_position;
};

#endif // FILEINPUTREADER_H
