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
/**
 * \class	InputManager
 * \brief	The InputManager is able to identify the type of input, read the input, and construct the appropriate bifiltration.
 */

#ifndef __InputManager_H__
#define __InputManager_H__

#include "dcel/barcode_template.h"
#include "interface/file_input_reader.h"
#include "interface/input_parameters.h"
#include "math/bifiltration_data.h"
#include "math/firep.h"
#include "math/template_point.h"
#include "math/template_points_matrix.h"
#include "progress.h"
#include <boost/multiprecision/cpp_int.hpp>

#include "numerics.h"
#include <fstream>
#include <math.h>
#include <set>
#include <sstream>
#include <vector>
using namespace rivet::numeric;

//first, a struct to help sort multi-grade values
struct ExactValue {
    double double_value;
    exact exact_value;

    mutable std::vector<unsigned> indexes; //indexes of points corresponding to this value (e.g. points whose birth time is this value)

    static double epsilon;

    ExactValue(exact e)
        : exact_value(e)
    {
        double_value = numerator(e).convert_to<double>() / denominator(e).convert_to<double>(); //can aos use static_cast in C++11
    }

    bool operator<=(const ExactValue& other) const
    {
        //if the two double values are nearly equal, then compare exact values
        if (almost_equal(double_value, other.double_value))
            return exact_value <= other.exact_value;

        //otherwise, compare double values
        return double_value <= other.double_value;
    }

    static bool almost_equal(const double a, const double b)
    {
        double diff = std::abs(a - b);
        if (diff <= epsilon)
            return true;

        if (diff <= (std::abs(a) + std::abs(b)) * epsilon)
            return true;
        return false;
    }
};

//comparator for ExactValue pointers
struct ExactValueComparator {
    bool operator()(const ExactValue& lhs, const ExactValue& rhs) const
    {
        //if the two double values are nearly equal, then compare exact values
        if (ExactValue::almost_equal(lhs.double_value, rhs.double_value))
            return lhs.exact_value < rhs.exact_value;

        //otherwise, compare double values
        return lhs.double_value < rhs.double_value;
    }
};

//ExactSet will help sort grades
typedef std::set<ExactValue, ExactValueComparator> ExactSet;

struct ComputationResult;

struct InputData {
    std::string x_label;
    std::string y_label;
    std::vector<exact> x_exact; //exact (e.g. rational) values of all x-grades, sorted
    std::vector<exact> y_exact; //exact (e.g. rational) values of all y-grades, sorted

    bool x_reverse=false;//whether the simplices are to be added in order of decreasing x grades
    bool y_reverse=false;

    std::shared_ptr<BifiltrationData> bifiltration_data; //TODO: This is only needed to build the FIRep.  To save memory, I have fixed the code to set bifiltration_data to the null pointer once the FIRep is built.  But perhaps it would be better design to remove this member from the struct altogether. -Mike

    std::shared_ptr<FIRep> free_implicit_rep;
    std::vector<TemplatePoint> template_points; // will be non-empty if we read RIVET data
    std::vector<BarcodeTemplate> barcode_templates; //only used if we read a RIVET data file and need to store the barcode templates before the arrangement is ready
};

enum FileContentType {
    INVALID = 0,
    DATA = 1,
    PRECOMPUTED = 2
};

struct FileContent {
    FileContentType type;

    std::shared_ptr<InputData> input_data;

    std::shared_ptr<ComputationResult> result;

    FileContent();
    FileContent(InputData *data);
    FileContent(ComputationResult *result);
    FileContent& operator=(const FileContent &other);
    FileContent(const FileContent &other);
};

struct FileType {
    std::string identifier;
    std::string description;
    bool is_data;
    std::function<FileContent(std::ifstream&, Progress&)> parser;
};

class InputError : public std::runtime_error {
public:
    InputError(unsigned line, std::string message);
};

//TODO: the input manager doesn't really hold an appreciable
//amount of state, there's really no reason to instantiate a class
//for this job, a collection of functions would do.

//now the InputManager class
class InputManager {
public:
    InputManager(InputParameters& input_params);

    FileContent start(Progress& progress); //function to run the input manager

    FileType identify();

private:
    InputParameters& input_params; //parameters supplied by the user
    const int verbosity; //controls display of output, for debugging

    std::vector<FileType> supported_types;

    std::pair<bool, FileType> get_supported_type(const std::string name)
    {
        auto it = std::find_if(supported_types.begin(), supported_types.end(), [name](FileType& t) { return name == t.identifier; });
        return std::pair<bool, FileType>(it != supported_types.end(), *it);
    }

    void register_file_type(FileType file_type);

    //TODO: these methods could move to a separate file-per-filetype model rather
    //than living in InputManager.
    FileContent read_point_cloud(std::ifstream& stream, Progress& progress); //reads a point cloud and constructs a simplex tree representing the bifiltered Vietoris-Rips complex
    FileContent read_discrete_metric_space(std::ifstream& stream, Progress& progress); //reads data representing a discrete metric space with a real-valued function and constructs a simplex tree
    FileContent read_bifiltration(std::ifstream& stream, Progress& progress); //reads a bifiltration and constructs a simplex tree
    FileContent read_RIVET_data(std::ifstream& stream, Progress& progress); //reads a file of previously-computed data from RIVET
    FileContent read_messagepack(std::ifstream& stream, Progress& progress);
    FileContent read_firep(std::ifstream& stream, Progress& progress); //reads a free implicit representation and constructs a FIRep

    void build_grade_vectors(InputData& data, ExactSet& value_set, std::vector<unsigned>& indexes, std::vector<exact>& grades_exact, unsigned num_bins); //converts an ExactSets of values to the vectors of discrete values that BifiltrationData uses to build the bifiltration, and also builds the grade vectors (floating-point and exact)

    exact approx(double x); //finds a rational approximation of a floating-point value; precondition: x > 0
    FileType& get_file_type(std::string fileName);
};

//a struct to store exact coordinates of a point, along with a "birth time"
struct DataPoint {

    std::vector<double> coords;
    exact birth;

    DataPoint(std::vector<std::string>& strs) //first (size - 1) elements of vector are coordinates, last element is birth time
    {
        coords.reserve(strs.size() - 1);

        for (unsigned i = 0; i < strs.size() - 1; i++) {
            double value;
            std::stringstream convert(strs[i]);
            convert >> value;
            coords.push_back(value);
        }

        birth = str_to_exact(strs.back());
    }
};

#endif // __InputManager_H__
