#include <algorithm>
#include <fstream>
#include <iostream>
#include <iostream>
#include <math.h>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;

#include "interface/input_manager.h"
#include "math/map_matrix.h"
#include "math/multi_betti.h"
#include "math/simplex_tree.h"
#include "math/st_node.h"

// STRUCT FOR EASILY COUNTING LCMs
struct LCM_Point {
    int x;
    int y;
    bool weak;

    LCM_Point(int x, int y)
        : x(x)
        , y(y)
        , weak(true)
    {
    }
};

struct LCM_Point_Comparator {
    bool operator()(const LCM_Point* lhs, const LCM_Point* rhs) const
    {
        if (lhs->x < rhs->x)
            return true;
        else if (lhs->x == rhs->x && lhs->y < rhs->y)
            return true;
        return false;
    }
};

// COUNT LCMs
int main(int argc, char* argv[])
{
    //check for name of data file
    if (argc == 1) {
        std::cout << "USAGE: run <filename> [dimension of homology] [x-bins] [y-bins]\n";
        return 1;
    }

    //set dimension of homology
    int dim = 1; //default
    if (argc >= 3)
        dim = std::atoi(argv[2]);
    std::cout << "Homology dimension set to " << dim << ".\n";

    //set numbers of bins
    int x_bins = 0; //default
    if (argc >= 4)
        x_bins = std::atoi(argv[3]);
    int y_bins = 0; //default
    if (argc >= 5)
        y_bins = std::atoi(argv[4]);
    std::cout << "Bins set to: x_bins = " << x_bins << ", y-bins = " << y_bins << ".\n";

    //start the input manager
    int verbosity = 3;
    InputManager im(dim, verbosity);
    im.start(argv[1], x_bins, y_bins);

    //get the bifiltration from the input manager
    SimplexTree* bifiltration = im.get_bifiltration();

    //get data extents
    double data_xmin = bifiltration->grade_x_value(0);
    double data_xmax = bifiltration->grade_x_value(bifiltration->num_x_grades() - 1);
    double data_ymin = bifiltration->grade_y_value(0);
    double data_ymax = bifiltration->grade_y_value(bifiltration->num_y_grades() - 1);

    //print bifiltration statistics
    std::cout << "\nBIFILTRATION:\n";
    std::cout << "   Number of simplices of dimension " << dim << ": " << bifiltration->get_size(dim) << "\n";
    std::cout << "   Number of simplices of dimension " << (dim + 1) << ": " << bifiltration->get_size(dim + 1) << "\n";
    std::cout << "   Number of x-grades: " << bifiltration->num_x_grades() << "; values " << data_xmin << " to " << data_xmax << "\n";
    std::cout << "   Number of y-grades: " << bifiltration->num_y_grades() << "; values " << data_ymin << " to " << data_ymax << "\n";
    std::cout << "\n";

    //initialize the MultiBetti object
    MultiBetti mb(bifiltration, dim, verbosity);

    //build column labels for output
    std::string col_labels = "         x = ";
    std::string hline = "    --------";
    for (int j = 0; j < bifiltration->num_x_grades(); j++) {
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

    //store support points
    std::vector<std::pair<int, int>> xi_support; //integer (relative) coordinates of xi support points
    /*    for(int i=bifiltration->num_y_grades()-1; i>=0; i--)
    {
        for(int j=0; j<bifiltration->num_x_grades(); j++)
        {
            if(mb.xi0(j,i) != 0 || mb.xi1(j,i) != 0)            //if this is an xi_i support point, then store its (relative) coordinates
                xi_support.push_back(std::pair<int,int>(j,i));
        }
    }*/
    for (int i = 0; i < bifiltration->num_x_grades(); i++) {
        for (int j = 0; j < bifiltration->num_y_grades(); j++) {
            if (mb.xi0(i, j) != 0 || mb.xi1(i, j) != 0) //if this is an xi_i support point, then store its (relative) coordinates
                xi_support.push_back(std::pair<int, int>(i, j));
        }
    }
    //print support points
    std::cout << "\nCOMPUTATION FINISHED:\n";

    if (verbosity >= 4) {
        //output xi_0
        std::cout << "  VALUES OF xi_0 for dimension " << dim << ":\n";
        for (int i = bifiltration->num_y_grades() - 1; i >= 0; i--) {
            std::cout << "     y = " << i << " | ";
            for (int j = 0; j < bifiltration->num_x_grades(); j++) {
                std::cout << mb.xi0(j, i) << "  ";
            }
            std::cout << "\n";
        }
        std::cout << col_labels;

        //output xi_1
        std::cout << "\n  VALUES OF xi_1 for dimension " << dim << ":\n";
        for (int i = bifiltration->num_y_grades() - 1; i >= 0; i--) {
            std::cout << "     y = " << i << " | ";
            for (int j = 0; j < bifiltration->num_x_grades(); j++) {
                std::cout << mb.xi1(j, i) << "  ";
            }
            std::cout << "\n";
        }
        std::cout << col_labels << "\n";
    }

    std::cout << "\nComputation took " << duration << ".\n";

    //print support points of xi_0 and xi_1
    std::cout << "\nSUPPORT POINTS OF xi_0 AND xi_1: " << xi_support.size() << " points total\n";
    for (int i = 0; i < xi_support.size(); i++)
        std::cout << "(" << xi_support[i].first << "," << xi_support[i].second << "), ";
    std::cout << "\n";

    //find LCMs
    std::set<LCM_Point*, LCM_Point_Comparator> lcms; // stores LCMs, indicating whether they arise from a pair of strongly incomparable support points

    for (int i = 0; i < xi_support.size(); i++) //TODO: THIS CAN BE IMPROVED
    {
        for (int j = i + 1; j < xi_support.size(); j++) {
            int ax = xi_support[i].first, ay = xi_support[i].second;
            int bx = xi_support[j].first, by = xi_support[j].second;

            //            std::cout << "    testing (" << ax << ", " << ay << ") with (" << bx << ", " << by << "): ";

            if ((ax - bx) * (ay - by) <= 0) //then we have found an LCM (it might be a weak LCM)
            {
                //insert this LCM into the LCM set (this does nothing if it is already inserted; new insertions are weak LCMs by default)
                LCM_Point* cur_lcm = new LCM_Point(std::max(ax, bx), std::max(ay, by));
                lcms.insert(cur_lcm);

                //                std::cout << "inserted LCM (" << std::max(ax,bx) << ", " << std::max(ay,by) << ") ";

                //if this is actually a strong LCM, then mark it as such
                if ((ax - bx) * (ay - by) < 0) {
                    std::set<LCM_Point*, LCM_Point_Comparator>::iterator it = lcms.find(cur_lcm);
                    (*it)->weak = false;
                    //                    std::cout << "(strong LCM)";
                }
            }
            //            std::cout << "\n";
        }
    }

    //count and output LCMs
    int weak_count = 0;

    std::cout << "\nLCMs FOUND:\n";
    for (std::set<LCM_Point*, LCM_Point_Comparator>::iterator it = lcms.begin(); it != lcms.end(); ++it) {
        if (verbosity >= 4)
            std::cout << "  (" << (*it)->x << ", " << (*it)->y << ") ";

        if ((*it)->weak == true) {
            weak_count++;
            if (verbosity >= 4)
                std::cout << "weak";
        }
        if (verbosity >= 4)
            std::cout << "\n";
    }

    std::cout << "\n --> Found " << lcms.size() << " LCMs, including " << weak_count << " weak LCMs.\n";
}
