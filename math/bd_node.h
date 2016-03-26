#ifndef BD_NODE_H
#define BD_NODE_H
/**
 * \class	BDNode
 * \brief	Implements a node of a RawBData
 * \author  Roy Zhao
 * \date	March 2016
 *
 * Stores a node that is used to build a simplex tree with multiple grades and provides operations.
 * Implementation is based on STNode.
 */

#include <vector>

//Pair of coordinates specifying grade of appearance with additional sorting operator. Sorted first by y then x grade in REVERSE-LEXICOGRAPHIC ORDER.
struct Grade
{
    int x;
    int y;
    bool operator<(Grade other) const
    {
        if (y != other.y)
            return y < other.y;
        else
            return x < other.x;
    }

    Grade(int set_x, int set_y) : x(set_x), y(set_y)
    { }
};

class BDNode {
    public:
        BDNode();					//constructor for empty node
        BDNode(int v, BDNode* p, std::vector<Grade>& grades, int g);		//constructor for non-empty node; parameters are: (vertex, parent, multi-grade vector, global index)

        ~BDNode();  //destructor

        int get_vertex();			//returns the vertex index
        BDNode get_parent();			//returns a pointer to the parent node

        void set_grades(std::vector<Grade>& grades);      //sets the multigrades for this simplex
        const std::vector<Grade>* grades() const;    //returns the multigrades for this simplex

        void set_global_index(int i);	//sets the global index for the simplex represented by this node
        int global_index();			//returns the global index for the simplex represented by this node

        void set_dim_index(int i);	//sets the dimension index for the simplex represented by this node
        int dim_index();            //returns the dimension index for the simplex represented by this node

        void append_child(BDNode*);		//appends a new child to this node; should only be called if vertex index of child is greater than vertex indexes of all other children
        BDNode* add_child(int v, std::vector<Grade>& grades);			//creates a new child node with given parameters and returns a pointer to the new node; if child with given vertex index already exists, then returns pointer to this node; NOTE: global indexes must be re-computed after calling this function
        std::vector<BDNode*> get_children();		//returns a vector of pointers to children nodes

        //TESTING
        void print();

private:
        int vertex;			//the index of the vertex represented by this node
        BDNode * parent;		//pointer to the parent node
        std::vector<BDNode*> children;	//pointers to children nodes -- these should remain sorted by vertex index

        std::vector<Grade> mgrades;        //The multi-grades for this simplex   (vector to account for multicritical case)

        int d_index;        //dimension index of this simplex (provides a total order of all simplices of a given dimension, REVERSE-LEXICOGRAPHIC with respect to the multi-grades)
        int g_index;		//global index of this simplex (global indexes provide a total ordering of simplices in the tree)

        void update_grades(); //Make sure grades are sorted and all incomparable
};

#endif // BD_NODE_H

