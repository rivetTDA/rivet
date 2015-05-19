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

#include <vector>


class STNode {
	public:
		STNode();					//constructor for empty node
        STNode(int v, STNode* p, int x, int y, int g);		//constructor for non-empty node; parameters are: (vertex, parent, multi-grade x, multi-grade y, global index)
		
        ~STNode();  //destructor

		int get_vertex();			//returns the vertex index
		STNode get_parent();			//returns a pointer to the parent node

        int grade_x() const;      //returns the first component of the multi-grade for this simplex
        int grade_y() const;      //returns the second component of the multi-grade for this simplex

        void set_global_index(int i);	//sets the global index for the simplex represented by this node
        int global_index();			//returns the global index for the simplex represented by this node

        void set_dim_index(int i);	//sets the dimension index for the simplex represented by this node
        int dim_index();            //returns the dimension index for the simplex represented by this node
		
		void append_child(STNode*);		//appends a new child to this node; should only be called if vertex index of child is greater than vertex indexes of all other children
        STNode* add_child(int v, int x, int y);			//creates a new child node with given parameters and returns a pointer to the new node; if child with given vertex index already exists, then returns pointer to this node; NOTE: global indexes must be re-computed after calling this function
		std::vector<STNode*> get_children();		//returns a vector of pointers to children nodes
		
private:
		int vertex;			//the index of the vertex represented by this node
		STNode * parent;		//pointer to the parent node
		std::vector<STNode*> children;	//pointers to children nodes -- these should remain sorted by vertex index

        int mg_x;        //first component of the multi-grade for this simplex   (e.g. time)
        int mg_y;        //second component of the multi-grade for this simplex  (e.g. distance)

        int d_index;        //dimension index of this simplex (provides a total order of all simplices of a given dimension, REVERSE-LEXICOGRAPHIC with respect to the multi-grades)
		int g_index;		//global index of this simplex (global indexes provide a total ordering of simplices in the tree)
		
};

#endif // __STNode_H__

