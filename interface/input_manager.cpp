/* InputManager class
 */

#include "input_manager.h"

//constructor
InputManager::InputManager(int d, int v) :
    hom_dim(d), verbosity(v), simplex_tree(d, v)
{ }

//function to run the input manager, requires a filename
void InputManager::start(const char* arg)
{
	//read the file
	if(verbosity >= 2) { std::cout << "READING FILE:\n"; }
	std::string line;
	infile.open(arg);
	if(infile.is_open())
	{
		//determine what type of file it is
		std::getline(infile,line);
		std::istringstream iss(line);
		std::string filetype;
		iss >> filetype;
		
		//call appropriate handler function
		if(filetype == "points")
		{
			read_point_cloud();
		}
		else if(filetype == "bifiltration")
		{
			read_bifiltration();
		}
		else
		{
			std::cout << "Error: Unrecognized file type.\n";
			throw std::exception();
		}
	}
	else
	{
		std::cout << "Error: Unable to open file " << arg << ".\n";
		throw std::exception();
	}
	
	infile.close();
	
}//end start()

//returns a pointer to the simplex tree representing the bifiltration
SimplexTree* InputManager::get_bifiltration()
{
	return &simplex_tree;
}


//reads a point cloud and constructs a simplex tree representing the bifiltered Vietoris-Rips complex
void InputManager::read_point_cloud()
{
	if(verbosity >= 2) { std::cout << "  Found a point cloud file.\n"; }
	
	//prepare (temporary) data structures
    int dimension;              //integer dimension of data
    int max_dim = hom_dim + 1;	//maximum dimension of simplices in Vietoris-Rips complex
    double max_dist;            //maximum distance for edges in Vietoris-Rips complex
	std::vector<Point> points;	//create for points
	std::string line;		//string to hold one line of input
		
	//read dimension of the points from the first line of the file
	std::getline(infile,line);
	std::stringstream(line) >> dimension;
    if(verbosity >= 4) { std::cout << "  dimension of data: " << dimension << "; max dimension of simplices: " << hom_dim << "\n"; }
	
	//read maximum distance for edges in Vietoris-Rips complex
	std::getline(infile,line);
	std::stringstream(line) >> max_dist;
	if(verbosity >= 4) { std::cout << "  maximum distance: " << max_dist << "\n"; }
		
	//read points
	while( std::getline(infile,line) )
	{
		//parse current point from string
		std::istringstream iss(line);
		double* n = new double[dimension];
		for(int i=0; i<dimension; i++)
		{
			iss >> n[i];	//extract the next double from the string
		}
		double t;	//time of birth for this point
		iss >> t; 
		
		Point p(n, t);
		
		//add current point to the vector
		points.push_back(p);
	}
	
	if(verbosity >= 4) { std::cout << "  read " << points.size() << " points; input finished\n"; }
	
	//sort the points
	if(verbosity >= 2) { std::cout << "SORTING POINTS BY BIRTH TIME\n"; }
	sort(points.begin(), points.end());
	
	//test points vector
	if(verbosity >= 6) 
	{
		std::cout << "TESTING VECTOR:\n";
		for(int i=0; i<points.size(); i++)
		{
			Point p = points.at(i);
			double *m = p.get_coords();
			std::cout << "  point " << i << ": (";
			for(int i=0; i<dimension; i++)
			{
				std::cout << m[i];
				if(i<dimension-1) { std::cout << ", "; }
			}
			std::cout << ") born at time " << p.get_birth() << "\n";		
		}
		std::cout << "  found " << points.size() << " points\n";
	}
	
	//build the filtration
	if(verbosity >= 2) { std::cout << "BUILDING VIETORIS-RIPS BIFILTRATION\n"; }
    simplex_tree.build_VR_complex(points, dimension, max_dim, max_dist);
	
	//clean up
	for(int i=0; i<points.size(); i++)
	{
		double* n = points[i].get_coords();
		delete[] n;
	}
}//end read_point_cloud()

//reads a bifiltration and constructs a simplex tree
void InputManager::read_bifiltration()
{
	if(verbosity >= 2) { std::cout << "  Found a bifiltration file.\n"; }
	
	//prepare (temporary) data structures
	std::string line;		//string to hold one line of input
	
	//read simplices
	while( std::getline(infile,line) )
	{
		std::istringstream iss(line);
		
		//read dimension of simplex
		int dim;
		iss >> dim;
		
		//read vertices
		std::vector<int> verts;
		for(int i=0; i<=dim; i++)
		{
			int v;
			iss >> v;
			verts.push_back(v);
		}
		
		//read multi-index
		int time, dist;
		iss >> time;
		iss >> dist;
		
		if(verbosity >= 4)
		{
			std::cout << "    simplex [" << verts[0];
			for(int i=1; i<=dim; i++)
				std::cout  << " " << verts[i];
			std::cout << "] with multi-index (" << time << ", " << dist << ")\n"; 
		}
		
		//add the simplex to the simplex tree
		simplex_tree.add_simplex(verts, time, dist);
	}

    //compute indexes
    simplex_tree.update_global_indexes();
    simplex_tree.update_dim_indexes();

}//end read_bifiltration()


