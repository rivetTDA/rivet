/* simplex tree node class
 * stores a node that is used to build a simplex tree
 *
 * based on a 2012 paper by Boissonnat and Maria
 */

#ifndef __STNode_H__
#define __STNode_H__

class STNode {
	public:
		STNode();					//constructor for empty node
		STNode(int, STNode*, int, int, int);		//constructor for non-empty node
		
		int get_vertex();			//returns the vertex index
		STNode get_parent();			//returns a pointer to the parent node
		int get_birth();			//returns the minimum time index at which this simplex exits
		int get_dist();			//returns the minimum distance index at which this simplex exists
		
		void set_global_index(int);		//sets the global index for the simplex represented by this node
		int get_global_index();			//returns the global index for the simplex represented by this node
		
		void add_child(STNode*);		//adds a new child to this node
		std::vector<STNode*> get_children();		//returns a vector of pointers to children nodes
		
		void print();				//print a text representation of this node
		
	private:
		int vertex;			//the index of the vertex represented by this node
		STNode * parent;		//pointer to the parent node
		std::vector<STNode*> children;	//pointers to children nodes
		int birth;			//minimum time at which this simplex exists
		int dist;			//minimum distance at which this simplex exists
		int g_index;		//global index of this simplex (global indexes provide a total ordering of simplices in the tree)
		
};

#include "st_node.hpp"

#endif // __STNode_H__

