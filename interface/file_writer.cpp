#include "file_writer.h"

#include "../dcel/barcode_template.h"

#include <chrono>


FileWriter::FileWriter(InputParameters& ip, Mesh& m, std::vector<xiPoint>& xi) :
    input_params(ip), arrangement(m), xi_support(xi)
{

}

std::ofstream & operator<<(std::ofstream &stream, const Mesh &mesh) {//write x-grades
    stream << "x-grades" << std::endl;
    for(std::vector<exact>::const_iterator it = mesh.x_exact.begin(); it != mesh.x_exact.end(); ++it)
    {
        std::ostringstream oss;
        oss << *it;
        stream << oss.str() << std::endl;
    }
    stream << std::endl;

    //write y-grades
    stream << "y-grades" << std::endl;
    for(std::vector<exact>::const_iterator it = mesh.y_exact.begin(); it != mesh.y_exact.end(); ++it)
    {
        std::ostringstream oss;
        oss << *it;
        stream << oss.str() << std::endl;
    }
    stream << std::endl;
    return stream;
}

void FileWriter::write_augmented_arrangement(std::ofstream& stream)
{

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    //write header info, in comment form
    stream << "# augmented arrangement data" << std::endl;
stream << "# computed by RIVET from the input file " << input_params.fileName << std::endl;
    stream << "# homology dimension: " << input_params.dim << std::endl;
    stream << "# bins: " << input_params.x_bins << " " << input_params.y_bins << std::endl;
    stream << "# file created at: "
            << std::ctime(&now)
            << std::endl;

    //write parameters
    stream << "RIVET_0" << std::endl;
    stream << input_params.dim << std::endl;
stream << input_params.x_label << std::endl;
stream << input_params.y_label << std::endl << std::endl;
    stream << arrangement;

    //write values of the multigraded Betti numbers
    stream << "xi values" << std::endl;
    for(std::vector<xiPoint>::iterator it = xi_support.begin(); it != xi_support.end(); ++it)
    {
        xiPoint p = *it;
        if(p.zero > 0 || p.one > 0 || p.two > 0)    //necessary because the vector xi_support also stores anchors (template points) that are not xi support points
            stream << p.x << " " << p.y << " " << p.zero << " " << p.one << " " << p.two << std::endl;
    }
    stream << std::endl;

    //write barcode templates
    stream << "barcode templates" << std::endl;
    for(unsigned i = 0; i < arrangement.num_faces(); i++)
    {
        BarcodeTemplate& bc = arrangement.get_barcode_template(i);
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
        stream << std::endl;
    }
}

