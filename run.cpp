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
			//parse current point from string
			istringstream iss(line);
			double* n = new double[dimension];				//DO I NEED TO delete THESE LATER???
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
	
	
	//build matrices
	cout << "COMPUTING BOUNDARY MATRICES\n";
	int time = 1;
	int dist = 1;
	int dim = 1;
	
	//MapMatrix* m = simplex_tree.get_boundary_mx(time, dist, dim);
	
	
	cout << "  boundary matrix A:";
	MapMatrix* ma = simplex_tree.get_boundary_mx(time-1, dist-1, dim);
	(*ma).print();
	
	cout << "  boundary matrix B:";
	MapMatrix* mb = simplex_tree.get_boundary_mx(time, dist-1, dim);
	(*mb).print();
	
	cout << "  boundary matrix C:";
	MapMatrix* mc = simplex_tree.get_boundary_mx(time-1, dist, dim);
	(*mc).print();
	
	cout << "  boundary matrix D:";
	MapMatrix* md = simplex_tree.get_boundary_mx(time, dist, dim);
	(*md).print();
	
	cout << "  boundary matrix for direct sum B+C:\n";
	(*mb).append_block( (*mc), (*mb).height() );
	(*mb).print();
	
	cout << "  map matrix [B+C,D]:";
	MapMatrix* bcd = simplex_tree.get_merge_mx(time, dist, dim);
	(*bcd).print();
	
	//apply column-reduction algorithm to boundary matrix for B+C, and do the same column operations to [B+C,D]
	cout << "  reducing boundary matrix B+C, and applying same column operations to [B+C,D]\n";
	(*mb).col_reduce(bcd);
	cout << "    reduced boundary matrix B+C:\n";
	(*mb).print();
	cout << "    reduced matrix [B+C,D]:\n";
	(*bcd).print();
	
	//identify zero columns from boundary matrix for B+C, and select those columns from [B+C,D]
	int c=1;	//keep track of current column in [B+C,D]
	for(int j=1; j<=(*mb).width(); j++)
	{
		if((*mb).low(j) > 0)	//then delete this column from [B+C,D]
			(*bcd).remove_column(c);
		else
			c++;
	}
	//now bcd contains a basis for [B+C,D](ker(boundary map B+C))
	cout << "    basis for [B+C,D](ker(boundary map B+C)):\n";
	(*bcd).print();
	
	//form concatenated matrix
	cout << "  boundary matrix D in one dimension higher:";
	MapMatrix* boundary_d2 = simplex_tree.get_boundary_mx(time, dist, dim+1);
	(*boundary_d2).print();
	
	cout << "  concatenating D2 with [B+C,D](ker(..)):\n";
	int right_block = (*boundary_d2).width() + 1;
	(*boundary_d2).append_block( (*bcd), 0);
	(*boundary_d2).print();
	
	//reduce the concatenated matrix
	(*boundary_d2).col_reduce();
	cout << "  reduced form of concatenated matrix:\n";
	(*boundary_d2).print();
	
	//count nonzero columns in reduced form of right block of concatenated matrix
	int nonzero_cols = 0;
	for(int j = right_block; j<=(*boundary_d2).width(); j++)
		if((*boundary_d2).low(j) > 0)
			nonzero_cols++;
	cout << "  number of nonzero columns in right block: " << nonzero_cols << "\n";
	
	//compute nullity of boundary matrix D
	cout << "  nullity of boundary matrix D:\n";
	(*md).col_reduce();
	(*md).print();
	
	int nullity = 0;
	for(int j=1; j<=(*md).width(); j++)
		if( (*md).low(j) == 0)
			nullity++;
	cout << "    nullity is: " << nullity << "\n";
	
	//compute rank of boundary matrix D2
	cout << "  rank of boundary matrix D2:\n";
	MapMatrix* bd2 = simplex_tree.get_boundary_mx(time, dist, dim+1);
	(*bd2).col_reduce();
	(*bd2).print();
	
	int rank = 0;
	for(int j=1; j<=(*bd2).width(); j++)
		if( (*bd2).low(j) > 0)
			rank++;
	cout << "    rank is: " << rank << "\n";
	
	//compute \xi_0
	int xi0 = nullity - rank - nonzero_cols;
	cout << "  finally: xi_0(" << time << "," << dist << ") = " << xi0 << "\n";
	
	
	//end
	cout << "Done.\n\n";
}//end main()



