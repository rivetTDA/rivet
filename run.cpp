//test program
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <stdlib.h>

#include "input_manager.h"
#include "simplex_tree.h"
#include "multi_betti.h"



int main(int argc, char* argv[])
{	
	//verbose: true for more output; false for less output
	const bool verbose = true;
	
	//check for name of data file
	if(argc == 1)
	{
		std::cout << "USAGE: run <filename> [dimension of homology] [time] [distance]\n";
		return 1;
	}
	
	//start the input manager
	InputManager im;
	im.start(argv[1]);
	
	//get the bifiltration from the input manager
	SimplexTree* bifiltration = im.get_bifiltration();
	
	//print simplex tree
	std::cout << "SIMPLEX TREE:\n";
	(*bifiltration).print();	
	
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
	
	//compute xi_0 and xi_1
	std::cout << "COMPUTING xi_0 AND xi_1:\n";
	MultiBetti mb(bifiltration, dim);
	if(argc >= 5)	//then compute xi_0 and xi_1 at a specific multi-index
	{
		int time = std::atoi(argv[3]);
		int dist = std::atoi(argv[4]);
		
		mb.compute_xi(time, dist);
		
		//output
		std::cout << "COMPUTATION FINISHED:\n";
		std::cout << "  at multi-index (time, distance)=(" << time << "," << dist << "):\n";
		std::cout << "    xi_0 = " << mb.xi0(time, dist) << "\n";
		std::cout << "    xi_1 = " << mb.xi1(time, dist) << "\n";
	}
	else	//then compute xi_0 and xi_1 at ALL multi-indexes
	{
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
	}//end else
	
	
	
	
	
	
	
	//done
	std::cout << "Done.\n\n";
}//end main()



