/* simplex tree node class
 * stores a node that is used to build a simplex tree
 */

#include "st_node.h"

#include <cstddef>  //for NULL keyword
#include <iostream>  //for std::cout, for testing only

//constructor for empty node
STNode::STNode() :
    vertex(-1), parent(NULL), mg_x(-1), mg_y(-1), d_index(-1), g_index(-1)
{ }

//constructor for non-empty node
//NOTE: node must still be added as a child of its parent after this constructor is called
STNode::STNode(int v, STNode* p, int x, int y, int g) :
    vertex(v), parent(p), mg_x(x), mg_y(y), d_index(-1), g_index(g)
{ }

//destructor -- IS THIS ADEQUATE???
STNode::~STNode()
{
    for(std::vector<STNode*>::iterator it = children.begin(); it != children.end(); ++it)
        delete *it;
}

//returns the vertex index
int STNode::get_vertex()
{
	return vertex;
}

//returns a pointer to the parent node
STNode STNode::get_parent()
{
	return *parent;
}

//sets the first component of the multigrade for this simplex
void STNode::set_x(int x)
{
    mg_x = x;
}

//returns the first component of the multigrade for this simplex
int STNode::grade_x() const
{
    return mg_x;
}

//sets the second component of the multigrade for this simplex
void STNode::set_y(int y)
{
    mg_y = y;
}

//returns the second component of the multigrade for this simplex
int STNode::grade_y() const
{
    return mg_y;
}

//sets the global index for the simplex represented by this node
void STNode::set_global_index(int i)
{
	g_index = i;
}

//returns the global index for the simplex represented by this node
int STNode::global_index()
{
	return g_index;
}

//sets the dimension index for the simplex represented by this node
void STNode::set_dim_index(int i)
{
    d_index = i;
}

//returns the dimension index for the simplex represented by this node
int STNode::dim_index()
{
    return d_index;
}

//appends a new child to this node
//WARNING: this should only be used if vertex index of child is greater than vertex indexes of all other children (to preserve order of children vector)
void STNode::append_child(STNode* child)
{
	children.push_back(child);
}


//creates a new child node with given parameters and returns a pointer to the new node
// NOTE: if child with given vertex index already exists, then returns pointer to this node
// NOTE: global indexes must be re-computed after calling this function
STNode* STNode::add_child(int v, int x, int y)
{
    //if node has children, binary search to see if a child node with given vertex index already exists
	int min = 0;
	int max = children.size()-1;
	int found = -1;
	while(max >= min)
	{
		int mid = (min + max)/2;
		if( (*children[mid]).get_vertex() == v )
		{
			found = mid;
			break;
		}
		else if( (*children[mid]).get_vertex() < v )
			min = mid + 1;
		else
			max = mid -1;
	}
	
	//if found, then return pointer to the node
	if(found != -1)
	{
		return children[found];
	}
	
	//if not found, create a new node
    STNode* newnode = new STNode(v, this, x, y, -1);
	children.insert(children.begin() + max + 1, newnode);
	return newnode;
}//end add_child()


//returns a vector of pointers to children nodes
std::vector<STNode*> &STNode::get_children()
{
	return children;
}

//TESTING
void STNode::print()
{
    std::cout << "NODE: vertex " << vertex <<  "; global index: " << g_index << "; dim index: " << d_index << "; multi-index: (" << mg_x << ", " << mg_y << "); parent: ";
    if(parent)
        std::cout << (*parent).get_vertex() << "; ";
    else
        std::cout << "NULL; ";
    std::cout << "children: ";
    if(children.size() == 0)

        std::cout << "NONE";
    for(int i=0; i<children.size(); i++)
    {
        if(i>0)
            std::cout << ", ";
        std::cout << (*children[i]).get_vertex();
    }
    std::cout << "\n";
}
