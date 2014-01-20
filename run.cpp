//test program
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include "point.h"
#include "edge.h"


using namespace std;

//function to partition the vector of points, for use in sort_points()
int partition(vector<Point>& pts, int left, int right, int pivot)
{
	//get birth time of pivot point
	double pivot_value = pts[pivot].get_birth();
	
	//move pivot to end
	Point temp = pts[right];
	pts[right] = pts[pivot];
	pts[pivot] = temp;
	
	//initialize temporary index to left value
	int ind = left;
	
	//compare points with pivot point
	for(int i=left; i<right; i++)
	{
		if(pts[i].get_birth() <= pivot_value)
		{
			//swap point i with point ind
			temp = pts[i];
			pts[i] = pts[ind];
			pts[ind] = temp;
			
			//increment temporary index only if swap occurred
			ind++;
		}
	}
	
	//move pivot to its final place
	temp = pts[ind];
	pts[ind] = pts[right];
	pts[right] = temp;
	
	//return temporary index
	return ind;
}




//function to sort points by birth time
//implemented as an in-place quicksort
void sort_points(vector<Point>& pts, int left, int right)
{
	if(left < right)
	{
		//choose pivot
		int pivot_index = (left + right)/2;
		
		//partition
		pivot_index = partition(pts, left, right, pivot_index);
		
		//recursively sort smaller elements
		sort_points(pts, left, pivot_index - 1);
		
		//recursively sort larger elements
		sort_points(pts, pivot_index + 1, right);
	}
}


//function to partition the vector of distances, for use in dist_sort()
int dist_part(vector<double>& dst, int left, int right, int pivot)
{
	//get birth time of pivot point
	double pivot_value = dst[pivot];
	
	//move pivot to end
	double temp = dst[right];
	dst[right] = dst[pivot];
	dst[pivot] = temp;
	
	//initialize temporary index to left value
	int ind = left;
	
	//compare points with pivot point
	for(int i=left; i<right; i++)
	{
		if(dst[i] <= pivot_value)
		{
			//swap point i with point ind
			temp = dst[i];
			dst[i] = dst[ind];
			dst[ind] = temp;
			
			//increment temporary index only if swap occurred
			ind++;
		}
	}
	
	//move pivot to its final place
	temp = dst[ind];
	dst[ind] = dst[right];
	dst[right] = temp;
	
	//return temporary index
	return ind;
}



//function to sort distances
//implemented as an in-place quicksort
void dist_sort(vector<double>& dst, int left, int right)
{
	if(left < right)
	{
		//choose pivot
		int pivot_index = (left + right)/2;
		
		//partition
		pivot_index = dist_part(dst, left, right, pivot_index);
		
		//recursively sort smaller elements
		dist_sort(dst, left, pivot_index - 1);
		
		//recursively sort larger elements
		dist_sort(dst, pivot_index + 1, right);
	}
}




//PROGRAM BEGINS HERE
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
			/*if(verbose) 
			{
				double *m = p.get_coords();
				cout << "  point: (";
				for(int i=0; i<d; i++)
				{
					cout << m[i];
					if(i<d-1) { cout << ", "; }
				}
				cout << ") born at time " << p.get_birth() << "\n";
			}*/
			
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
	sort_points(points, 0, points.size()-1);
	
	
	//test points vector
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
	
	//compute distances
	int num_edges = (points.size() * (points.size() - 1))/2;
	Edge edges [num_edges];
	
	if(verbose) { cout << "COMPUTING DISTANCES:\n"; }
	int c=0;	//counter to track position in array of edges
	for(int i=1; i<points.size(); i++)
	{
		Point p = points.at(i);
		double *pc = p.get_coords();
		for(int j=0; j<i; j++)
		{
			Point q = points.at(j);
			double *qc = q.get_coords();
			double s=0;
			for(int k=0; k<dimension; k++)
				s += (pc[k] - qc[k])*(pc[k] - qc[k]);
			edges[c].set_all(i,j,sqrt(s),fmax(p.get_birth(),q.get_birth()));
			c++;
		}
	}
		
	cout << "TESTING EDGES:\n";
	for(c=0; c<num_edges; c++)
	{
		cout << "  distance from point " << edges[c].get_p1() << " to point " << edges[c].get_p2() << ": " << edges[c].get_length() << "; edge born at time " << edges[c].get_birth() << "\n";
	}
	
	cout << "BUILDING FILTRATION:\n";
	//prepare sorted list of birth times???
	vector<double> times;
	double now = points[0].get_birth();		//this could be dangerous (if points[] is empty)
	times.push_back(now);
	for(int i=1; i<points.size(); i++)
	{
		if(points[i].get_birth() > now)
		{
			now = points[i].get_birth();
			times.push_back(now);
		}
	}
	cout << "  birth times: ";
	for(int i=0; i<times.size(); i++)
		cout << times[i] << ", ";
	cout << "\n";
	
	//prepare sorted list of distances???
	vector<double> dists;
	dists.push_back(0);			//is this necessary?
	for(int i=0; i<num_edges; i++)
	{
		if(edges[i].get_length() <= max_dist)
			dists.push_back(edges[i].get_length());
	}
	dist_sort(dists, 0, dists.size()-1);
	//eliminate duplicate distances
	double temp = dists[0];
	int i=1;
	while(i<dists.size())
	{
		if(dists[i] == dists[i-1])
			dists.erase(dists.begin() + i);
		else
			i++;
	}
	cout << "  distances: ";
	for(int i=0; i<dists.size(); i++)
		cout << dists[i] << ", ";
	cout << "\n";
	
	//data structrue for bifiltration
	//vector<Simplex> bifil [times.size()][dists.size()];
	
	//loop through birth times
	//for(int i=0; i<times.size(); i++)
	//{
		
		//loop through distances involving all points born at current time
		//for(int j=0; j<dists.size(); j++)
		//{
			//for each distance, find which simplices have this time-distance multi-index
			
			
			//store simplices in a vector
			//store vector in a 2-d array
			
			
		//}
	//}
	
	
	
	
	
	
	//end
	cout << "\n";
	
	
}




