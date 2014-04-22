//test program
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <utility> //std::pair
#include <math.h>
#include <algorithm>
#include <stdlib.h>

#include "input_manager.h"
#include "simplex_tree.h"
#include "multi_betti.h"
#include "dcel/mesh.h"



int main(int argc, char* argv[])
{	
	//verbose: true for more output; false for less output
	const bool verbose = false;
	
	//check for name of data file
	if(argc == 1)
	{
		std::cout << "USAGE: run <filename> [dimension of homology] [verbosity]\n";
		return 1;
	}
	
	//set verbosity
	int verbosity = 2;	//default
	if(argc >= 4)
		verbosity = std::atoi(argv[3]);
	if(verbosity >= 6) { std::cout << "Verbosity parameter set to " << verbosity << ".\n"; }
	
	//set dimension of homology
	int dim = 1;		//default
	if(argc >= 3)
		dim = std::atoi(argv[2]);
	if(verbosity >= 6) { std::cout << "Computing homology in dimension " << dim << ".\n"; }
	
	//start the input manager
	InputManager im(verbosity);
	im.start(argv[1]);
	
	//get the bifiltration from the input manager
	SimplexTree* bifiltration = im.get_bifiltration();
	
	//print simplex tree
	if(verbosity >= 2)
	{
		std::cout << "SIMPLEX TREE:\n";
		bifiltration->print();	
	}
	if(verbosity >= 4)
	{
		std::cout << "  vertex lists for each of the " << (*bifiltration).get_num_simplices() << " simplices:\n";
		for(int i=0; i < (*bifiltration).get_num_simplices(); i++)
		{
			std::cout << "    simplex " << i << ": ";
			std::vector<int> vert = (*bifiltration).find_vertices(i);
			for(int j=0; j<vert.size(); j++)
				std::cout << vert[j] << ", ";
			std::cout << "\n";
		}
	}

	//compute xi_0 and xi_1 at ALL multi-indexes
	if(verbosity >= 2) { std::cout << "COMPUTING xi_0 AND xi_1:\n"; }
	MultiBetti mb(bifiltration, dim, verbosity);
	mb.compute_all_xi();
	
	//print xi_0 and xi_1
	if(verbosity >= 4)
	{ 
		std::cout << "COMPUTATION FINISHED:\n";
	
		//build column labels for output
		std::string col_labels = "        dist ";
		std::string hline = "    --------";
		for(int j=0; j<(*bifiltration).get_num_dists(); j++)
		{
			std::ostringstream oss;
			oss << j;
			col_labels += oss.str() + "  ";
			hline += "---";
		}
		col_labels += "\n" + hline + "\n";

		//output xi_0
		std::cout << "  VALUES OF xi_0 for dimension " << dim << ":\n";
		std::cout << col_labels;
		for(int i=0; i<(*bifiltration).get_num_times(); i++)
		{
			std::cout << "    time " << i << " | ";
			for(int j=0; j<(*bifiltration).get_num_dists(); j++)
			{
				std::cout << mb.xi0(i,j) << "  ";
			}
			std::cout << "\n";
		}

		//output xi_1
		std::cout << "\n  VALUES OF xi_1 for dimension " << dim << ":\n";
		std::cout << col_labels;
		for(int i=0; i<(*bifiltration).get_num_times(); i++)
		{
			std::cout << "    time " << i << " | ";
			for(int j=0; j<(*bifiltration).get_num_dists(); j++)
			{
				std::cout << mb.xi1(i,j) << "  ";
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}
	
	//find all support points of xi_0 and xi_1
	std::vector<std::pair<int, int> > xi_support;	
	for(int i=0; i < bifiltration->get_num_times(); i++)
	{
		for(int j=0; j < bifiltration->get_num_dists(); j++)
		{
			if(mb.xi0(i,j) != 0 || mb.xi1(i,j) != 0)
				xi_support.push_back(std::pair<int,int>(i,j));
		}
	}
	
	//print support points of xi_0 and xi_1
	if(verbosity >= 2)
	{
		std::cout << "  SUPPORT POINTS OF xi_0 AND xi_1: ";
		for(int i=0; i<xi_support.size(); i++)
		{
			std::cout << "(" << xi_support[i].first << "," << xi_support[i].second << "), ";
		
		}
		std::cout << "\n";
	}
	
	//find the LCMs, and build the decomposition of the Grassmannian
	if(verbosity >= 2) { std::cout << "CALCULATING LCMs AND DECOMPOSING THE STRIP:\n"; }
	Mesh dcel(verbosity);
	
	for(int i=0; i<xi_support.size(); i++)
	{
		for(int j=i+1; j<xi_support.size(); j++)
		{
			int ax = xi_support[i].first, ay = xi_support[i].second;
			int bx = xi_support[j].first, by = xi_support[j].second; 
			
			if((ax - bx)*(ay - by) < 0)	//then the support points are incomparable, so we have found an LCM
			{
				double t = bifiltration->get_time(std::max(ax,bx));
				double d = bifiltration->get_dist(std::max(ay,by));
				
				if(!dcel.contains(t,d))	//then this LCM has not been inserted yet
				{
					if(verbosity >= 4) { std::cout << "  LCM at (" << std::max(ax,bx) << "," << std::max(ay,by) << ") => (" << t << "," << d << ") determined by (" << ax << "," << ay << ") and (" << bx << "," << by << ")\n"; }
					
					dcel.add_curve(t,d);
					
					if(verbosity >= 8) { dcel.print(); }
				}
				else
				{
					if(verbosity >= 4) { std::cout << "  LCM (" << t << "," << d << ") already inserted.\n"; }
				}
			}
		}
	}
	
	//print the dcel arrangement
	if(verbosity >= 2)
	{
		std::cout << "DCEL ARRANGEMENT:\n";
		dcel.print();
	}
	
	//do the persistence computations in each cell
	if(verbosity >= 2) { std::cout << "COMPUTING PERSISTENCE DATA FOR EACH CELL:\n"; }
	dcel.build_persistence_data(xi_support, bifiltration);
	
	
	
	//done
	std::cout << "Done.\n\n";
}//end main()



