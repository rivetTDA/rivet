/* simplex tree class
 * works together with STNode class; also requires Point class
 *
 * based on a 2012 paper by Boissonnat and Maria
 */

#ifndef __SimplexTree_H__
#define __SimplexTree_H__

#include "st_node.h"
#include "map_matrix.h"

class SimplexTree {
	public:
		SimplexTree(std::vector<Point> &, int, int, double);		//constructor
		
		int dist_index(double);				//returns the index of a distance value, or -1 if not found
		int time_index(double);				//returns the index of a time value, or -1 if not found
		
		MapMatrix* get_boundary_mx(int time, int dist, int dim);	//computes a boundary matrix for simplices of a given dimension at the specified multi-index
		
		std::vector<int> find_vertices(int);	//given a global index, return (a vector containing) the vertices of the simplex
		int find_index(std::vector<int>&);		//given a sorted vector of vertex indexes, return the global index of the corresponding simplex (or -1 if it doesn't exist)
		
		void print();	//prints a representation of the simplex tree
		int get_num_simplices();		//returns the total number of simplices represented in the simplex tree
		
		
	private:
		std::vector<Point> points;	//points from which this complex is built
		int point_dimension;	//dimension of the data
		int max_depth;		//max depth of tree (equivalently, one more than max dimension of largest permitted simplex)
		double max_distance;	//we create edges between simplices that are no further than max_distance apart
		double * distances;	//an array of distances between all pairs of points; this array is built within this class
		std::vector<double> dist_list;	//sorted list of unique distances less than max_distance, used for creating integer indexes
		std::vector<double> time_list;	//sorted list of unique birth times, used for creating integer indexes
		STNode root;		//root node of the simplex tree
		
		static const bool verbose = true;	//controls display of output, for debugging
		
		void build_subtree(STNode&, std::vector<int>&, double, double, int, int&);	//recursive function used in the constructor
		
		void find_nodes(STNode &node, int level, std::vector<int> &vec, int time, int dist, int dim);	//recursively search tree for simplices of specified dimension that exist at specified multi-index
		
		void find_vertices_recursively(std::vector<int> &vertices, STNode &node, int key);	//recursively search for a global index and keep track of vertices

		
		void print_subtree(STNode&, int);	//recursive function used for printing the simplex tree
};

#include "simplex_tree.hpp"

#endif // __SimplexTree_H__
