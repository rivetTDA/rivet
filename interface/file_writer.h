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

#include <fstream>

#include <boost/multiprecision/cpp_int.hpp>

#include <vector>
#include <dcel/mesh_message.h>


class FileWriter
{
    public:
        FileWriter(InputParameters& ip, Mesh& m, std::vector<xiPoint>& xi);

        void write_augmented_arrangement(std::ofstream &file);

    private:
        InputParameters& input_params;
        Mesh& arrangement; //reference to the DCEL arrangement
        std::vector<xiPoint>& xi_support;  //stores discrete coordinates of xi support points, with multiplicities
};

#endif // FILEWRITER_H
