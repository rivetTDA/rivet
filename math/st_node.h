/**
 * \class	STNode
 * \brief	Implements a node of a SimplexTree
 * \author	Matthew L. Wright
 * \date	January 2014
 * 
 * Stores a node that is used to build a simplex tree and provides operations.
 * Implementation is based on a 2012 paper by Boissonnat and Maria.
 */


#ifndef __STNode_H__
#define __STNode_H__

class STNode {
	public:
		STNode();					//constructor for empty node
		STNode(int v, STNode* p, int b, int d, int g);		//constructor for non-empty node; parameters are: (vertex, parent, birth time, distance, global index)
		
		int get_vertex();			//returns the vertex index
		STNode get_parent();			//returns a pointer to the parent node
		int get_birth();			//returns the minimum time index at which this simplex exits
		int get_dist();			//returns the minimum distance index at which this simplex exists
		
		void set_global_index(int);		//sets the global index for the simplex represented by this node
		int get_global_index();			//returns the global index for the simplex represented by this node
		
		void append_child(STNode*);		//appends a new child to this node; should only be called if vertex index of child is greater than vertex indexes of all other children
		STNode* add_child(int v, int t, int d);			//creates a new child node with given parameters and returns a pointer to the new node; if child with given vertex index already exists, then returns pointer to this node; NOTE: global indexes must be re-computed after calling this function
		std::vector<STNode*> get_children();		//returns a vector of pointers to children nodes
		
		void print();				//print a text representation of this node
		
	private:
		int vertex;			//the index of the vertex represented by this node
		STNode * parent;		//pointer to the parent node
		std::vector<STNode*> children;	//pointers to children nodes -- these should remain sorted by vertex index
		int birth;			//minimum time at which this simplex exists
		int dist;			//minimum distance at which this simplex exists
		int g_index;		//global index of this simplex (global indexes provide a total ordering of simplices in the tree)
		
};

#include "st_node.cpp"

#endif // __STNode_H__

