/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "file_writer.h"

#include "../dcel/barcode_template.h"
#include "input_manager.h"

#include <chrono>

FileWriter::FileWriter(InputParameters& ip, InputData& data, Arrangement& m, std::vector<TemplatePoint>& points)
    : input_data(data)
    , input_params(ip)
    , arrangement(m)
    , template_points(points)
{
}

void FileWriter::write_augmented_arrangement(std::ofstream& stream)
{

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    //At some point just doing stream << std::ctime(&now) started giving ambiguous operator<< errors,
    //so let's be explicit.
    std::string now_as_string = std::ctime(&now);
    //write header info, in comment form
    stream << "# augmented arrangement data" << std::endl;
    stream << "# computed by RIVET from the input file " << input_params.fileName << std::endl;
    stream << "# homology dimension: " << input_params.hom_degree << std::endl;
    stream << "# bins: " << input_params.x_bins << " " << input_params.y_bins << std::endl;
    stream << "# file created at: "
           << now_as_string
           << std::endl;

    //write parameters
    stream << "RIVET_0" << std::endl;
    stream << input_params.hom_degree << std::endl;
    stream << input_data.x_label << std::endl;
    stream << input_data.y_label << std::endl
           << std::endl;
    write_grades(stream, arrangement.x_exact, arrangement.y_exact);

    //write values of the multigraded Betti numbers
    stream << "xi values" << std::endl;
    for (std::vector<TemplatePoint>::iterator it = template_points.begin(); it != template_points.end(); ++it) {
        TemplatePoint p = *it;
        if (p.zero > 0 || p.one > 0 || p.two > 0) //necessary because the vector template_points also stores anchors (template points) that are not xi support points
            stream << p.x << " " << p.y << " " << p.zero << " " << p.one << " " << p.two << std::endl;
    }
    stream << std::endl;

    //write barcode templates
    stream << "barcode templates" << std::endl;
    for (unsigned i = 0; i < arrangement.num_faces(); i++) {
        BarcodeTemplate& bc = arrangement.get_barcode_template(i);
        if (bc.is_empty()) {
            stream << "-"; //this denotes an empty barcode (necessary because FileInputReader ignores white space)
        } else {
            for (std::set<BarTemplate>::iterator it = bc.begin(); it != bc.end(); ++it) {
                stream << it->begin << ",";
                if (it->end == (unsigned)-1) //then the bar ends at infinity, but we just write "i"
                    stream << "i";
                else
                    stream << it->end;
                stream << "," << it->multiplicity << " ";
            }
        }
        stream << std::endl;
    }
}
