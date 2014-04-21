// program to test the DCEL structure

#include <iostream>
#include <limits>
#include "mesh.h"

int main(int argc, char* argv[])
{
	//first, test infinity
	std::cout << "TESTING INFINITY:\n";
	
	double max = std::numeric_limits<double>::max();
	double inf = std::numeric_limits<double>::infinity();
	if(inf > max)
		std::cout << "\t" << inf << " is greater than " << max << "\n";
		
	double min = std::numeric_limits<double>::min();
	double ninf = -1*inf;
	if(ninf < min)
		std::cout << "\t" << ninf << " is less than " << min << "\n";
	
	
	//initialize the mesh
	std::cout << "CREATING THE MESH:\n";
	Mesh dcel(10);
	dcel.print();
	
	//add curves
//	int x_vals[] = {2,2,4};
//	int y_vals[] = {1,3,5};
	int x_vals[] = {2,4,1,8,1};
	int y_vals[] = {3,5,2,9,5};
	std::vector<int> vx (x_vals, x_vals + sizeof(x_vals) / sizeof(int) );
	std::vector<int> vy (y_vals, y_vals + sizeof(y_vals) / sizeof(int) );
	
	for(int i=0; i<vx.size(); i++)
	{
		std::cout << "ADDING A CURVE FOR LCM(" << vx[i] << "," << vy[i] << ")\n";
		dcel.add_curve(vx[i],vy[i]);
		dcel.print();
	}
	
	//test interior point routine
	std::cout << "FINDING A POINT INSIDE EACH CELL:\n";
	dcel.build_persistence_data();
	

	std::cout << "Done.\n\n";
}
