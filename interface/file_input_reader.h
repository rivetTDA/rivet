#ifndef FILEINPUTREADER_H
#define FILEINPUTREADER_H

#include <QFile>
#include <QStringList>
#include <QTextStream>

class FileInputReader
{
    public:
        FileInputReader(QFile& file);
        bool has_next();
        QStringList next_line();

    private:
        QTextStream in;
        bool next_line_found;
        QStringList next_line_tokens;
        void find_next();
};

#endif // FILEINPUTREADER_H
