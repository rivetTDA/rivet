/**
 * \brief	Saves an augmented arrangement to a file.
 * \author	Matthew L. Wright
 * \date	2016
 */


#ifndef FILEWRITER_H
#define FILEWRITER_H

#include "input_parameters.h"
#include "../dcel/mesh.h"
#include "../math/xi_point.h"

#include <QFile>

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;

#include <vector>


class FileWriter
{
    public:
        FileWriter(InputParameters& ip, Mesh* m, std::vector<exact>& x, std::vector<exact>& y, std::vector<xiPoint>& xi);

        void write_augmented_arrangement(QFile& file);

    private:
        InputParameters& input_params;
        Mesh* arrangement; //pointer to the DCEL arrangement
        std::vector<exact>& x_exact;       //exact (e.g. rational) values of all x-grades, sorted
        std::vector<exact>& y_exact;       //exact (e.g. rational) values of all y-grades, sorted
        std::vector<xiPoint>& xi_support;  //stores discrete coordinates of xi support points, with multiplicities
};

#endif // FILEWRITER_H
