/**
 * \class	InputManager
 * \brief	Manages input for the persistence visualization program.
 * \author	Matthew L. Wright
 * \date	February 2014
 * 
 * The InputManager is able to identify the type of input, read the input, and construct the appropriate bifiltration.
 */

 
#ifndef __InputManager_H__
#define __InputManager_H__

#include "dcel/barcode_template.h"
#include "interface/file_input_reader.h"
#include "interface/input_parameters.h"
#include "math/simplex_tree.h"
#include "math/xi_point.h"
#include "math/xi_support_matrix.h"
#include "progress.h"
#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;

#include <math.h>
#include <set>
#include <sstream>
#include <fstream>
#include <vector>


//first, a struct to help sort multi-grade values
struct ExactValue
{
    double double_value;
    exact exact_value;

    std::vector<unsigned> indexes;   //indexes of points corresponding to this value (e.g. points whose birth time is this value)

    static double epsilon;

    ExactValue(exact e) : exact_value(e)
    {
        double_value = numerator(e).convert_to<double>() / denominator(e).convert_to<double>();  //can aos use static_cast in C++11
    }

    bool operator<=(const ExactValue& other) const
    {
        //if the two double values are nearly equal, then compare exact values
        if(almost_equal(double_value, other.double_value))
            return exact_value <= other.exact_value;

        //otherwise, compare double values
        return double_value <= other.double_value;
    }

    static bool almost_equal(const double a, const double b)
    {
        double diff = std::abs(a - b);
        if(diff <= epsilon)
            return true;

        if(diff <= (std::abs(a) + std::abs(b))*epsilon)
            return true;
        return false;
    }
};

//comparator for ExactValue pointers
struct ExactValueComparator
{
    bool operator()(const ExactValue* lhs, const ExactValue* rhs) const
    {
        //if the two double values are nearly equal, then compare exact values
        if(ExactValue::almost_equal(lhs->double_value, rhs->double_value))
            return lhs->exact_value < rhs->exact_value;

        //otherwise, compare double values
        return lhs->double_value < rhs->double_value;
    }
};

//ExactSet will help sort grades
typedef std::set<ExactValue*, ExactValueComparator> ExactSet;

struct InputData {
    bool is_data;
  std::vector<double> x_grades;  //floating-point values of all x-grades, sorted exactly
  std::vector<exact> x_exact;    //exact (e.g. rational) values of all x-grades, sorted
  std::vector<double> y_grades;  //floating-point values of all y-grades, sorted exactly
  std::vector<exact> y_exact;    //exact (e.g. rational) values of all y-grades, sorted
  std::shared_ptr<SimplexTree> simplex_tree; // will be non-null if we read raw data
  std::vector<xiPoint> xi_support; // will be non-empty if we read RIVET data
  std::vector<BarcodeTemplate> barcode_templates; //only used if we read a RIVET data file and need to store the barcode templates before the arrangement is ready

};



struct FileType {
  std::string identifier;
  std::string description;
  bool is_data;
  std::function<std::shared_ptr<InputData> (std::ifstream&, Progress&)> parser;
};

//TODO: the input manager doesn't really hold an appreciable
//amount of state, there's really no reason to instantiate a class
//for this job, a collection of functions would do.

//now the InputManager class
class InputManager
{
	public:
        InputManager(InputParameters &input_params);

        std::shared_ptr<InputData> start(Progress &progress);	//function to run the input manager

    private:
        InputParameters& input_params;  //parameters supplied by the user
        const int verbosity;			//controls display of output, for debugging
        int hom_dim;                    //dimension of homology to be computed

        std::vector<FileType> supported_types;

        std::pair<bool, FileType> get_supported_type(const std::string name) {
          auto it = std::find_if(supported_types.begin(), supported_types.end(), [name](FileType &t) { return name == t.identifier; });
          return std::pair<bool,FileType>(it != supported_types.end(), *it);
        }

        void register_file_type(FileType file_type);

        std::shared_ptr<InputData> read_point_cloud(std::ifstream &stream, Progress &progress);		//reads a point cloud and constructs a simplex tree representing the bifiltered Vietoris-Rips complex
        std::shared_ptr<InputData> read_discrete_metric_space(std::ifstream &stream, Progress &progress);   //reads data representing a discrete metric space with a real-valued function and constructs a simplex tree
        std::shared_ptr<InputData> read_bifiltration(std::ifstream &stream, Progress &progress);	//reads a bifiltration and constructs a simplex tree
        std::shared_ptr<InputData> read_RIVET_data(std::ifstream &stream, Progress &progress);      //reads a file of previously-computed data from RIVET

        void build_grade_vectors(InputData &data, ExactSet& value_set, std::vector<unsigned>& indexes, std::vector<double>& grades_fp, std::vector<exact>& grades_exact, unsigned num_bins); //converts an ExactSets of values to the vectors of discrete values that SimplexTree uses to build the bifiltration, and also builds the grade vectors (floating-point and exact)

        exact approx(double x);         //finds a rational approximation of a floating-point value; precondition: x > 0
    FileType &get_file_type(std::string fileName);
};

//helper function for converting a string to an exact value
exact str_to_exact(std::string str);

//a struct to store exact coordinates of a point, along with a "birth time"
struct DataPoint {

    std::vector<double> coords;
    exact birth;

    DataPoint(std::vector<std::string>& strs)   //first (size - 1) elements of vector are coordinates, last element is birth time
    {
        coords.reserve(strs.size() - 1);

        for(unsigned i=0; i < strs.size() - 1; i++)
        {
            int value;
            std::stringstream convert(strs[i]);
            convert >> value;
            coords.push_back(value);
        }

        birth = str_to_exact(strs.back());
    }

};


#endif // __InputManager_H__
