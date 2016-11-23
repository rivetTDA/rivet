/**
 * \brief	Saves an augmented arrangement to a file.
 * \author	Matthew L. Wright
 * \date	2016
 */

#ifndef FILEWRITER_H
#define FILEWRITER_H

#include "dcel/arrangement.h"
#include "input_manager.h"
#include "input_parameters.h"
#include "math/template_point.h"

#include <fstream>

#include <boost/multiprecision/cpp_int.hpp>

#include <dcel/arrangement_message.h>
#include <vector>

class FileWriter {
public:
    FileWriter(InputParameters& ip, InputData& input, Arrangement& m, std::vector<TemplatePoint>& points);

    template <typename T>
    static T& write_grades(T& stream, const std::vector<exact>& x_exact, const std::vector<exact>& y_exact)
    {

        //write x-grades
        stream << "x-grades" << std::endl;
        for (std::vector<exact>::const_iterator it = x_exact.cbegin(); it != x_exact.cend(); ++it) {
            std::ostringstream oss;
            oss << *it;
            stream << oss.str() << std::endl;
        }
        stream << std::endl;

        //write y-grades
        stream << "y-grades" << std::endl;
        for (std::vector<exact>::const_iterator it = y_exact.cbegin(); it != y_exact.cend(); ++it) {
            std::ostringstream oss;
            oss << *it;
            stream << oss.str() << std::endl;
        }
        stream << std::endl;
        return stream;
    }

    void write_augmented_arrangement(std::ofstream& file);

private:
    InputData& input_data;
    InputParameters& input_params;
    Arrangement& arrangement; //reference to the DCEL arrangement
    std::vector<TemplatePoint>& template_points; //stores discrete coordinates of xi support points, with multiplicities
};

#endif // FILEWRITER_H
