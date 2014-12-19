/*********************************************
 * program for testing and timing our algorithm for computing the bi-graded Beti numbers
 * created by Matthew Wright, December 2014
 */

#include <iostream>

#include "input_manager.h"
#include "math/simplex_tree.h"
#include "math/multi_betti.h"

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;

#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;


int main(int argc, char* argv[])
{
  // STEP 1: INPUT PARAMETERS

    int verbosity = 6;

    //check for name of data file
    if(argc == 1)
    {
        std::cout << "USAGE: run <filename> [dimension of homology]\n";
        return 1;
    }

    //set dimension of homology
    int dim = 1;		//default
    if(argc >= 3)
        dim = std::atoi(argv[2]);
    std::cout << "Homology dimension set to " << dim << ".\n";


  // STEP 2: READ DATA AND CREATE BIFILTRATION

    //start the input manager
    InputManager* im = new InputManager(dim, verbosity);
    std::string filestr = argv[1];
    im->start(filestr, 0, 0);   //NOTE: last two arguments are bins

    //get the data
    std::vector<double> x_grades = im->get_x_grades();
    std::vector<double> y_grades = im->get_y_grades();
//    std::vector<exact> x_exact = im->get_x_exact();
//    std::vector<exact> y_exact = im->get_y_exact();
    SimplexTree* bifiltration = im->get_bifiltration();

    //get data extents
    double data_xmin = x_grades.front();
    double data_xmax = x_grades.back();
    double data_ymin = y_grades.front();
    double data_ymax = y_grades.back();

    //print bifiltration statistics
    std::cout << "\nBIFILTRATION:\n";
    std::cout << "   Number of simplices of dimension " << dim << ": " << bifiltration->get_size(dim) << "\n";
    std::cout << "   Number of simplices of dimension " << (dim+1) << ": " << bifiltration->get_size(dim+1) << "\n";
    std::cout << "   Number of x-grades: " << x_grades.size() << "; values " << data_xmin << " to " << data_xmax << "\n";
    std::cout << "   Number of y-grades: " << y_grades.size() << "; values " << data_ymin << " to " << data_ymax << "\n";
    std::cout << "\n";


      std::cout << "\n";


  // STEP 3: COMPUTE SUPPORT POINTS OF MULTI-GRADED BETTI NUMBERS


    //build column labels for output
    std::string col_labels = "         x = ";
    std::string hline = "    --------";
    for(int j=0; j<x_grades.size(); j++)
    {
        std::ostringstream oss;
        oss << j;
        col_labels += oss.str() + "  ";
        hline += "---";
    }
    col_labels = hline + "\n" + col_labels + "\n";

    // ===== OLD ALGORITHM =====
    std::cout << "COMPUTING xi_0 AND xi_1 FOR HOMOLOGY DIMENSION " << dim << " -- OLD ALGORITHM:\n";
    MultiBetti mb_old(bifiltration, dim, verbosity);

    ptime time1_start(microsec_clock::local_time());  //start timer

    mb_old.compute_fast();

    ptime time1_end(microsec_clock::local_time());    //stop timer
    time_duration duration1(time1_end - time1_start);

    std::cout << "   OLD ALGORITHM: xi_i computation took " << duration1 << "\n";

    //output xi_0
    std::cout << "  VALUES OF xi_0 for dimension " << dim << ":\n";
    for(unsigned i=y_grades.size(); i>0; i--)
    {
        std::cout << "     y = " << (i-1) << " | ";
        for(unsigned j=0; j<x_grades.size(); j++)
            std::cout << mb_old.xi0(j,i-1) << "  ";
        std::cout << "\n";
    }
    std::cout << col_labels << "\n";




    // ===== NEW ALGORITHM =====
    std::cout << "COMPUTING xi_0 AND xi_1 FOR HOMOLOGY DIMENSION " << dim << " -- NEW ALGORITHM:\n";
    MultiBetti mb_new(bifiltration, dim, verbosity);

    ptime time2_start(microsec_clock::local_time());  //start timer

    mb_new.compute_parallel();

    ptime time2_end(microsec_clock::local_time());    //stop timer
    time_duration duration2(time2_end - time2_start);

    std::cout << "   NEW ALGORITHM: xi_i computation took " << duration2 << "\n";

    //output xi_0
    std::cout << "  VALUES OF xi_0 for dimension " << dim << ":\n";
    for(unsigned i=y_grades.size(); i>0; i--)
    {
        std::cout << "     y = " << (i-1) << " | ";
        for(unsigned j=0; j<x_grades.size(); j++)
            std::cout << mb_new.xi0(j,i-1) << "  ";
        std::cout << "\n";
    }
    std::cout << col_labels;


    //output xi_1
/*    std::cout << "\n  VALUES OF xi_1 for dimension " << dim << ":\n";
    for(int i=bifiltration->num_y_grades()-1; i>=0; i--)
    {
        std::cout << "     y = " << i << " | ";
        for(int j=0; j<bifiltration->num_x_grades(); j++)
        {
            std::cout << mb_new.xi1(j,i) << "  ";
        }
        std::cout << "\n";
    }
    std::cout << col_labels << "\n";
*/

}//end main()
