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
		std::cout << "USAGE: run <filename> [dimension of homology]\n";
		return 1;
	}
	
	//start the input manager
	InputManager im(verbose);
	im.start(argv[1]);
	
	//get the bifiltration from the input manager
	SimplexTree* bifiltration = im.get_bifiltration();
	
	//print simplex tree
	std::cout << "SIMPLEX TREE:\n";
	bifiltration->print();	
	
	//TEST: 
	std::cout << "  vertex lists for each of the " << (*bifiltration).get_num_simplices() << " simplices:\n";
	for(int i=0; i < (*bifiltration).get_num_simplices(); i++)
	{
		std::cout << "    simplex " << i << ": ";
		std::vector<int> vert = (*bifiltration).find_vertices(i);
		for(int j=0; j<vert.size(); j++)
			std::cout << vert[j] << ", ";
		std::cout << "\n";
	}

	//set dimension of homology
	int dim = 1;	//default
	if(argc >= 3)
		dim = std::atoi(argv[2]);
	
	//compute xi_0 and xi_1 at ALL multi-indexes
	std::cout << "COMPUTING xi_0 AND xi_1...\n";
	MultiBetti mb(bifiltration, dim, false);
	mb.compute_all_xi();
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
	
	std::cout << "  SUPPORT POINTS: ";
	for(int i=0; i<xi_support.size(); i++)
	{
		std::cout << "(" << xi_support[i].first << "," << xi_support[i].second << "), ";
		
	}
	std::cout << "\n";
	
	//find the LCMs, and build the decomposition of the Grassmannian
	std::cout << "CALCULATING LCMs AND DECOMPOSING THE STRIP:\n";
	Mesh dcel;
	
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
				
				std::cout << "  LCM at (" << std::max(ax,bx) << "," << std::max(ay,by) << ") => (" << t << "," << d << ") determined by (" << ax << "," << ay << ") and (" << bx << "," << by << ")\n";
				
				dcel.add_curve(t,d);
			}
			
		}
	}
	
	//TEST: print the dcel arrangement
	std::cout << "DCEL ARRANGEMENT:\n";
	dcel.print();
	
	
	
	
	
	//done
	std::cout << "Done.\n\n";
}//end main()



