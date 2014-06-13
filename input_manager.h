/**
 * \class	InputManager
 * \brief	Manages input for the persistence visualization program.
 * \author	Matthew L. Wright
 * \date	February 2014
 * 
 * The InputManager is able to identify the type of input, read the input, and construct the appropriate bifiltration.
 */

 
#ifndef __InputManager_H__
#define __InputManager_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>

#include "point.h"
#include "simplex_tree.h"
#include "map_matrix.h"

class InputManager
{
	public:
		InputManager(int v);		//constructor
		void start(const char* arg);	//function to run the input manager, requires a filename
		
		SimplexTree* get_bifiltration();	//returns a pointer to the simplex tree representing the bifiltration
		
		
		
	private:
		const int verbosity;			//controls display of output, for debugging
		
		std::ifstream infile;			//file stream for the file containing the input
		SimplexTree simplex_tree;		//simplex tree constructed from the input
		
		void read_point_cloud();		//reads a point cloud and constructs a simplex tree representing the bifiltered Vietoris-Rips complex
		void read_bifiltration();		//reads a bifiltration and constructs a simplex tree
		
		
		
};

#include "input_manager.hpp"

#endif // __InputManager_H__
