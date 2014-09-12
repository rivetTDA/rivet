#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <set>

#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;

#include "interface/input_manager.h"
#include "math/st_node.h"
#include "math/simplex_tree.h"
#include "math/map_matrix.h"
#include "math/multi_betti.h"
#include "dcel/mesh.h"


// RECURSIVELY PRINT TREE
void print_subtree(STNode &node, int indent)
{
	//print current node
	for(int i=0; i<indent; i++)
		std::cout << "  ";
	node.print();
	
	//print children nodes
    std::vector<STNode*> kids = node.get_children();
	for(int i=0; i<kids.size(); i++)
		print_subtree(*kids[i], indent+1);
}




// TESTING CONSTRUCTION OF THE ARRANGEMENT
int main(int argc, char* argv[])
{	
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

    //start the input manager
    int verbosity = 4;
    InputManager im(dim, verbosity);
    im.start(argv[1], 0, 0);    //NOTE: last two arguments are x-bins and y-bins

    //get the bifiltration from the input manager
    SimplexTree* bifiltration = im.get_bifiltration();


    //print simplex tree
    if(verbosity >= 8)
    {
        std::cout << "SIMPLEX TREE:\n";
        bifiltration->print();
    }

    //get data extents
    double data_xmin = bifiltration->grade_x_value(0);
    double data_xmax = bifiltration->grade_x_value(bifiltration->num_x_grades() - 1);
    double data_ymin = bifiltration->grade_y_value(0);
    double data_ymax = bifiltration->grade_y_value(bifiltration->num_y_grades() - 1);

    //print bifiltration statistics
    std::cout << "\nBIFILTRATION:\n";
    std::cout << "   Number of simplices of dimension " << dim << ": " << bifiltration->get_size(dim) << "\n";
    std::cout << "   Number of simplices of dimension " << (dim+1) << ": " << bifiltration->get_size(dim+1) << "\n";
    std::cout << "   Number of x-grades: " << bifiltration->num_x_grades() << "; values " << data_xmin << " to " << data_xmax << "\n";
    std::cout << "   Number of y-grades: " << bifiltration->num_y_grades() << "; values " << data_ymin << " to " << data_ymax << "\n";
    std::cout << "\n";

    //initialize the MultiBetti object
    MultiBetti mb(bifiltration, dim, verbosity);

/*    //build column labels for output
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
*/
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
    ptime xi_start(microsec_clock::local_time());

    //compute
    mb.compute_fast();

    //stop timer
    ptime xi_end(microsec_clock::local_time());
    time_duration xi_duration(xi_end - xi_start);

    //print
    std::cout << "\nCOMPUTATION FINISHED:\n";

    //output xi_0
/*    std::cout << "  VALUES OF xi_0 for dimension " << dim << ":\n";
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
*/
    std::cout << "\nComputation took " << xi_duration << ".\n";


    //find all support points of xi_0 and xi_1
    std::vector<std::pair<int, int> > xi_support;  //integer (relative) coordinates of xi support points

    for(int i=0; i < bifiltration->num_x_grades(); i++)
    {
        for(int j=0; j < bifiltration->num_y_grades(); j++)
        {
            if(mb.xi0(i,j) != 0 || mb.xi1(i,j) != 0)
            {
                xi_support.push_back(std::pair<int,int>(i,j));
            }
        }
    }

    //print support points of xi_0 and xi_1
    if(verbosity >= 2)
    {
        std::cout << "  SUPPORT POINTS OF xi_0 AND xi_1 (" << xi_support.size() << " points total):\n";
        for(unsigned i=0; i<xi_support.size(); i++)
            std::cout << "(" << xi_support[i].first << "," << xi_support[i].second << "), ";
        std::cout << "\n";
    }

    //TESTING
    std::cout << std::setprecision(20);

    //start timer
    ptime lcm_start(microsec_clock::local_time());

    //find LCMs, build decomposition of the affine Grassmannian                             TODO: THIS CAN BE IMPROVED
    if(verbosity >= 2) { std::cout << "CALCULATING LCMs AND DECOMPOSING THE STRIP:\n"; }
    Mesh* dcel = new Mesh(verbosity);

    for(unsigned i=0; i<xi_support.size(); i++)
    {
        for(unsigned j=i+1; j<xi_support.size(); j++)
        {
            int ax = xi_support[i].first, ay = xi_support[i].second;
            int bx = xi_support[j].first, by = xi_support[j].second;

            if((ax - bx)*(ay - by) <= 0)	//then the support points are (at least weakly) incomparable, so we have found an LCM
            {
                double t = bifiltration->grade_x_value(std::max(ax,bx));
                double d = bifiltration->grade_y_value(std::max(ay,by));

                if(verbosity >= 6) { std::cout << "  LCM at (" << std::max(ax,bx) << "," << std::max(ay,by) << ") => (" << t << "," << d << ") determined by (" << ax << "," << ay << ") and (" << bx << "," << by << ")\n"; }

                dcel->add_lcm(t, d);
            }
        }
    }

    //build the DCEL arrangement
    dcel->build_arrangement();
    dcel->print_stats();

    //stop timer
    ptime lcm_end(microsec_clock::local_time());
    time_duration lcm_duration(lcm_end - lcm_start);

    std::cout << "\nBuilding the arrangement took " << lcm_duration << ".\n";


    //print the DCEL arrangement
//    if(verbosity >= 4)
//    {
//        std::cout << "DCEL ARRANGEMENT:\n";
//        dcel->print();
//    }

    //test consistency of arrangement
//    std::cout << "TESTING CONSISTENCY OF THE DCEL ARRANGEMENT:\n";
//    dcel->test_consistency();


    //compute persistence data
    dcel->build_persistence_data(xi_support, bifiltration, dim);




}//end main()
