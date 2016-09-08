/**
 * \brief	Saves an augmented arrangement to a file.
 * \author	Matthew L. Wright
 * \date	2016
 */

#ifndef FILEWRITER_H
#define FILEWRITER_H

#include "dcel/arrangement.h"
#include "math/template_point.h"
#include "input_manager.h"
#include "input_parameters.h"

#include <fstream>

#include <boost/multiprecision/cpp_int.hpp>

#include <dcel/arrangement_message.h>
#include <vector>

class FileWriter {
public:
    FileWriter(InputParameters& ip, InputData& input, Arrangement& m, std::vector<TemplatePoint>& points);

    void write_augmented_arrangement(std::ofstream& file);

private:
    InputParameters& input_params;
    InputData& input_data;
    Arrangement& arrangement; //reference to the DCEL arrangement
    std::vector<TemplatePoint>& template_points; //stores discrete coordinates of xi support points, with multiplicities
};

#endif // FILEWRITER_H
