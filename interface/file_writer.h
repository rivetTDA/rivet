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

        auto x_float = rivet::numeric::to_doubles(x_exact);
        auto y_float = rivet::numeric::to_doubles(y_exact);
        //write x-grades
        stream << "x-grades" << std::endl;
        for (std::vector<double>::const_iterator it = x_float.cbegin(); it != x_float.cend(); ++it) {
            std::ostringstream oss;
            oss << *it;
            stream << oss.str() << std::endl;
        }
        stream << std::endl;

        //write y-grades
        stream << "y-grades" << std::endl;
        for (std::vector<double>::const_iterator it = y_float.cbegin(); it != y_float.cend(); ++it) {
            std::ostringstream oss;
            oss << *it;
            stream << oss.str() << std::endl;
        }
        stream << std::endl;
        return stream;
    }

    void write_augmented_arrangement(std::ofstream& file);

private:
    InputParameters& input_params;
    InputData& input_data;
    Arrangement& arrangement; //reference to the DCEL arrangement
    std::vector<TemplatePoint>& template_points; //stores discrete coordinates of xi support points, with multiplicities
};

#endif // FILEWRITER_H
