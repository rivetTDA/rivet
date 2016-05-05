#include "file_writer.h"

#include "../dcel/barcode_template.h"

#include <QTextStream>
#include <QDateTime>

#include <set>


FileWriter::FileWriter(InputParameters& ip, Mesh* m, std::vector<exact>& x, std::vector<exact>& y, std::vector<xiPoint>& xi) :
    input_params(ip), arrangement(m),
    x_exact(x), y_exact(y), xi_support(xi)
{

}

void FileWriter::write_augmented_arrangement(QFile& file)
{
    QTextStream stream(&file);

    //write header info, in comment form
    stream << "# augmented arrangement data" << endl;
stream << "# computed by RIVET from the input file " << QString::fromStdString(input_params.fileName) << endl;
    stream << "# homology dimension: " << input_params.dim << endl;
    stream << "# bins: " << input_params.x_bins << " " << input_params.y_bins << endl;
    stream << "# file created at: " << QDateTime::currentDateTime().toString() << endl << endl;

    //write parameters
    stream << "RIVET_0" << endl;
    stream << input_params.dim << endl;
stream << QString::fromStdString(input_params.x_label) << endl;
stream << QString::fromStdString(input_params.y_label) << endl << endl;

    //write x-grades
    stream << "x-grades" << endl;
    for(std::vector<exact>::iterator it = x_exact.begin(); it != x_exact.end(); ++it)
    {
        std::ostringstream oss;
        oss << *it;
        stream << QString::fromStdString(oss.str()) << endl;
    }
    stream << endl;

    //write y-grades
    stream << "y-grades" << endl;
    for(std::vector<exact>::iterator it = y_exact.begin(); it != y_exact.end(); ++it)
    {
        std::ostringstream oss;
        oss << *it;
        stream << QString::fromStdString(oss.str()) << endl;
    }
    stream << endl;

    //write values of the multigraded Betti numbers
    stream << "xi values" << endl;
    for(std::vector<xiPoint>::iterator it = xi_support.begin(); it != xi_support.end(); ++it)
    {
        xiPoint p = *it;
        if(p.zero > 0 || p.one > 0 || p.two > 0)    //necessary because the vector xi_support also stores anchors (template points) that are not xi support points
            stream << p.x << " " << p.y << " " << p.zero << " " << p.one << " " << p.two << endl;
    }
    stream << endl;

    //write barcode templates
    stream << "barcode templates" << endl;
    for(unsigned i = 0; i < arrangement->num_faces(); i++)
    {
        BarcodeTemplate& bc = arrangement->get_barcode_template(i);
        if(bc.is_empty())
        {
            stream << "-";  //this denotes an empty barcode (necessary because FileInputReader ignores white space)
        }
        else
        {
            for(std::set<BarTemplate>::iterator it = bc.begin(); it != bc.end(); ++it)
            {
                stream << it->begin << ",";
                if(it->end == (unsigned) -1)    //then the bar ends at infinity, but we just write "i"
                    stream << "i";
                else
                    stream << it->end;
                stream << "," << it->multiplicity << " ";
            }
        }
        stream << endl;
    }

    file.close();   //close the file -- this might not be necessary
}
