//test program
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>

#include "point.h"
#include "simplex_tree.h"
#include "st_node.h"
#include "map_matrix.h"

using namespace std;


int main(int argc, char* argv[])
{	
	//verbose: true for more output; false for less output
	const bool verbose = true;
	
	//check for name of data file
	if(argc == 1)
	{
		cout << "USAGE: run <filename>\n";
		return 1;
	}
	
	//integer dimension of data
	int dimension;	
	
	//maximum dimension of simplices in Vietoris-Rips complex
	int max_dim;
	
	//maximum distance for edges in Vietoris-Rips complex
	double max_dist;
	
	//create vector for points
	vector<Point> points;
		
	//read points from file
	if(verbose) { cout << "READING FILE:\n"; }
	string line;
	ifstream myfile(argv[1]);
	if(myfile.is_open())
	{
		//get dimension of the points from the first line of the file
		getline(myfile,line);
		stringstream(line) >> dimension;
		if(verbose) { cout << "  dimension of data: " << dimension << "\n"; }
		
		//get maximum dimension of simplices in Vietoris-Rips complex
		getline(myfile,line);
		stringstream(line) >> max_dim;
		if(verbose) { cout << "  maximum dimension of simplices: " << max_dim << "\n"; }
		
		//get maximum distance for edges in Vietoris-Rips complex
		getline(myfile,line);
		stringstream(line) >> max_dist;
		if(verbose) { cout << "  maximum distance: " << max_dist << "\n"; }
		
		//get points
		while( getline(myfile,line) )
		{
			//cout << "point: " << line << '\n'; //TESTING
			
			//parse current point from string
			istringstream iss(line);
			double* n = new double[dimension];				//DO I NEED TO delete THESE LATER???
			//cout << "address of n[]: " << n << " " << &n << "\n"; //TESTING
			for(int i=0; i<dimension; i++)
			{
				iss >> n[i];	//extract the next double from the string
			}
			double t;	//time of birth for this point
			iss >> t; 
			
			Point p (n, t);
			
			//add current point to the vector
			points.push_back(p);
		}
		myfile.close();
		
		if(verbose) { cout << "  read " << points.size() << " points; input finished\n"; }
	}
	else
	{
		cout << "Error: Unable to open file " << argv[1] << ".\n";
		return 1;
	}
	
	//sort the points
	if(verbose) { cout << "SORTING POINTS BY BIRTH TIME\n"; }
	sort(points.begin(), points.end());
	
	
	//test points vector
	if(verbose) {
		cout << "TESTING VECTOR:\n";
		for(int i=0; i<points.size(); i++)
		{
			Point p = points.at(i);
			double *m = p.get_coords();
			cout << "  point " << i << ": (";
			for(int i=0; i<dimension; i++)
			{
				cout << m[i];
				if(i<dimension-1) { cout << ", "; }
			}
			cout << ") born at time " << p.get_birth() << "\n";		
		}
		cout << "  found " << points.size() << " points\n";
	}
	
	
	//build the filtration
	if(verbose) { cout << "BUILDING FILTRATION\n"; }
	SimplexTree simplex_tree(points, dimension, max_dim+1, max_dist);
	
	//print simplex tree
	cout << "TESTING SIMPLEX TREE:\n";
	simplex_tree.print();	
	
	//TEST: 
	cout << "  vertex lists for each of the " << simplex_tree.get_num_simplices() << " simplices:\n";
	for(int i=0; i < simplex_tree.get_num_simplices(); i++)
	{
		cout << "    simplex " << i << ": ";
		vector<int> vert = simplex_tree.find_vertices(i);
		for(int j=0; j<vert.size(); j++)
			cout << vert[j] << ", ";
		cout << "\n";
	}
	
	
	//build boundary matrices
	cout << "COMPUTING BOUNDARY MATRICES\n";
	
	//MapMatrix* m = simplex_tree.get_boundary_mx(time, dist, dim);
	MapMatrix* m = simplex_tree.get_boundary_mx(4, 3, 1);
	(*m).print();
	
	m = simplex_tree.get_boundary_mx(4, 7, 1);
	(*m).print();
	
	m = simplex_tree.get_boundary_mx(4, 7, 2);
	(*m).print();
	
	m = simplex_tree.get_boundary_mx(4, 7, 3);
	(*m).print();
	
	m = simplex_tree.get_boundary_mx(4, 7, 4);
	(*m).print();

	m = simplex_tree.get_boundary_mx(4, 7, 5);
	(*m).print();
		
	cout << "TO DO: store local index lists with each boundary matrix\n";
	
	//end
	cout << "Done.\n\n";
}//end main()



