#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h> //atoi
#include <string>
#include <vector>
//#include <math.h>
//#include <algorithm>
//#include <set>

//#include "boost/date_time/posix_time/posix_time.hpp"
//using namespace boost::posix_time;

#include "interface/input_manager.h"
#include "math/simplex_tree.h"
#include "math/st_node.h"
//#include "math/map_matrix.h"
//#include "math/multi_betti.h"

// RECURSIVELY PRINT TREE
/*void print_subtree(STNode &node, int indent)
{
	//print current node
	for(int i=0; i<indent; i++)
		std::cout << "  ";
	node.print();
	
	//print children nodes
    std::vector<STNode*> kids = node.get_children();
	for(int i=0; i<kids.size(); i++)
		print_subtree(*kids[i], indent+1);
}*/

// TESTING SIMPLEX TREE
int main(int argc, char* argv[])
{
    //check for name of data file
    if (argc == 1) {
        std::cout << "USAGE: run <filename> [dimension of homology]\n";
        return 1;
    }

    //set dimension of homology
    int dim = 1; //default
    if (argc >= 3)
        dim = std::atoi(argv[2]);
    std::cout << "Homology dimension set to " << dim << ".\n";

    //start the input manager
    int verbosity = 10;
    InputManager im(dim, verbosity);
    im.start(argv[1], 2, 3); //last two arguments are bins

    //get the bifiltration from the input manager
    /*    SimplexTree* bifiltration = im.get_bifiltration();


    //print simplex tree
    if(verbosity >= 2)
    {
        std::cout << "SIMPLEX TREE:\n";
        bifiltration->print();
    }

    std::cout << "\nBIFILTRATION:\n";
    std::cout << "   Number of simplices of dimension " << dim << ": " << bifiltration->get_size(dim) << "\n";
    std::cout << "   Number of simplices of dimension " << (dim+1) << ": " << bifiltration->get_size(dim+1) << "\n";
    std::cout << "   Number of x-grades: " << bifiltration->num_x_grades() << "\n";
    std::cout << "   Number of y-grades: " << bifiltration->num_y_grades() << "\n";
    std::cout << "\n";

    //initialize the MultiBetti object
    MultiBetti mb(bifiltration, dim, verbosity);

    //build column labels for output
    std::string col_labels = "         x = ";
    std::string hline = "    --------";
    for(int j=0; j<bifiltration->num_x_grades(); j++)
    {
        std::ostringstream oss;
        oss << j;
        col_labels += oss.str() + "  ";
        hline += "---";
    }
    col_labels = hline + "\n" + col_labels + "\n";

    //make sure all xi_1 values are initially zero!!!!!
    //output xi_1
//    std::cout << "\n  INITIAL VALUES OF IN xi_1 MATRIX:\n";
//    for(int i=bifiltration->num_y_grades()-1; i>=0; i--)
//    {
//        std::cout << "     y = " << i << " | ";
//        for(int j=0; j<bifiltration->num_x_grades(); j++)
//            std::cout << mb.xi1(j,i) << "  ";
//        std::cout << "\n";
//    }
//    std::cout << col_labels << "\n";



    //compute xi_0 and xi_1
    std::cout << "COMPUTING XI_0 AND XI_1:\n";

    //start timer
    ptime time_start(microsec_clock::local_time());

    //compute
    mb.compute_fast();

    //stop timer
    ptime time_end(microsec_clock::local_time());
    time_duration duration(time_end - time_start);




    //print
    std::cout << "\nCOMPUTATION FINISHED:\n";

    //output xi_0
    std::cout << "  VALUES OF xi_0 for dimension " << dim << ":\n";
    for(int i=bifiltration->num_y_grades()-1; i>=0; i--)
    {
        std::cout << "     y = " << i << " | ";
        for(int j=0; j<bifiltration->num_x_grades(); j++)
        {
            std::cout << mb.xi0(j,i) << "  ";
        }
        std::cout << "\n";
    }
    std::cout << col_labels;


    //output xi_1
    std::cout << "\n  VALUES OF xi_1 for dimension " << dim << ":\n";
    for(int i=bifiltration->num_y_grades()-1; i>=0; i--)
    {
        std::cout << "     y = " << i << " | ";
        for(int j=0; j<bifiltration->num_x_grades(); j++)
        {
            std::cout << mb.xi1(j,i) << "  ";
        }
        std::cout << "\n";
    }
    std::cout << col_labels << "\n";

    std::cout << "\nComputation took " << duration << ".\n";
*/
}
