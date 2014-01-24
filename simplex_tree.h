/* simplex tree class
 * works together with STNode class; also requires Point class
 *
 * based on a 2012 paper by Boissonnat and Maria
 */

#ifndef __SimplexTree_H__
#define __SimplexTree_H__

#include "st_node.h"

class SimplexTree {
	public:
		SimplexTree(std::vector<Point> &, int, int, double);		//constructor
		
		void build_subtree(STNode&, int, double, double, int);
		
		void print();	//prints a representation of the simplex tree
		
		
	private:
		std::vector<Point> points;	//points from which this complex is built
		int point_dimension;	//dimension of the data
		double * distances;	//an array of distances between the points; this array is built within this class
		STNode root;		//root node of the simplex tree
		int max_depth;		//max depth of tree (equivalently, one more than max dimension of largest permitted simplex)
		double max_distance;	//we create edges between simplices that are no further than max_distance apart
		
		static const bool verbose = false;	//display lots of output, for debugging
		
		void print_subtree(STNode&, int);	//recursive function used for printing the simplex tree
};

#include "simplex_tree.hpp"

#endif // __SimplexTree_H__
