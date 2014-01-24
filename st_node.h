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
		STNode(int, STNode*, double, double);		//constructor for non-empty node
		
		int get_vertex();			//returns the vertex index
		STNode get_parent();			//returns a pointer to the parent node
		double get_birth();			//returns the minimum time at which this simplex exits
		double get_dist();			//returns the minimum distance at which this simplex exists
		
		void add_child(STNode*);		//adds a new child to this node
		std::vector<STNode*> get_children();		//returns a vector of pointers to children nodes
		
		//also needs a function to return the SIMPLEX represented by this node??
		
		void print();				//print a text representation of this node
		
	private:
		int vertex;			//the index of the vertex represented by this node
		STNode * parent;		//pointer to the parent node
		std::vector<STNode*> children;	//pointers to children nodes
		double birth;			//minimum time at which this simplex exists
		double dist;			//minimum distance at which this simplex exists
		
		
};

#include "st_node.hpp"

#endif // __STNode_H__

