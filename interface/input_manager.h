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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <boost/multiprecision/cpp_int.hpp>

#include "../math/simplex_tree.h"
#include "../math/map_matrix.h"

typedef boost::multiprecision::cpp_rational exact;

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


//now the InputManager class
class InputManager
{
	public:
        InputManager(int dim, std::vector<double>& x_grades, std::vector<exact>& x_exact, std::vector<double>& y_grades, std::vector<exact>&y_exact, SimplexTree& bifiltration, int verbosity);
            //constructor; requires dimension of homology to be computed, vectors in which grades will be stored, and verbosity parameter

        void start(std::string filename, unsigned x_bins, unsigned y_bins);	//function to run the input manager, requires a filename
		
        //functions to access the grade values -- maybe returning vectors is not the best design here???
//DEPRECATED        std::vector<double> get_x_grades(); //returns a vector of floating-point values of x-grades, sorted exactly
//DEPRECATED        std::vector<exact> get_x_exact();     //exact (e.g. rational) values of all x-grades, sorted

//DEPRECATED        std::vector<double> get_y_grades();   //floating-point values of all y-grades, sorted exactly
//DEPRECATED        std::vector<exact> get_y_exact();     //exact (e.g. rational) values of all y-grades, sorted

//DEPRECATED        SimplexTree* get_bifiltration();	//returns a pointer to the simplex tree representing the bifiltration
		
		
	private:
		const int verbosity;			//controls display of output, for debugging

        int hom_dim;                    //dimension of homology to be computed
		
		std::ifstream infile;			//file stream for the file containing the input
		
        std::vector<double>& x_grades;  //floating-point values of all x-grades, sorted exactly
        std::vector<exact>& x_exact;    //exact (e.g. rational) values of all x-grades, sorted

        std::vector<double>& y_grades;  //floating-point values of all y-grades, sorted exactly
        std::vector<exact>& y_exact;    //exact (e.g. rational) values of all y-grades, sorted

        SimplexTree& simplex_tree;		//simplex tree constructed from the input; contains only discrete data (i.e. integer multi-grades)

        void read_point_cloud(unsigned x_bins, unsigned y_bins);		//reads a point cloud and constructs a simplex tree representing the bifiltered Vietoris-Rips complex
		void read_bifiltration();		//reads a bifiltration and constructs a simplex tree
		
        exact approx(double x);         //finds a rational approximation of a floating-point value; precondition: x > 0
		
};

//helper function for converting a string to an exact value
exact str_to_exact(std::string& str);

//a struct to store exact coordinates of a point, along with a "birth time"
struct ExactPoint {

    std::vector<exact> coords;
    exact birth;

    ExactPoint(std::vector<std::string>& strs)   //first (size - 1) elements of vector are coordinates, last element is birth time
    {
        coords.reserve(strs.size() - 1);

        for(unsigned i=0; i < strs.size() - 1; i++)
            coords.push_back(str_to_exact(strs[i]));

        birth = str_to_exact(strs.back());
    }


};




//#include "input_manager.cpp"

#endif // __InputManager_H__
