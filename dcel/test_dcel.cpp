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
	Mesh dcel;
	dcel.print();
	
	//add a new curve
	std::cout << "ADDING A CURVE FOR LCM(1,2)\n";
	dcel.add_curve(1,2);
	dcel.print();
	
	
	
	
	
	
	
	
	
	
	
	std::cout << "Done.\n\n";
}
