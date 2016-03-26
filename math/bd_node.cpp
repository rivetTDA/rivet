/* raw b data node class
 * stores a node that is used to build the raw b data
 */

#include "bd_node.h"

#include <cstddef>  //for NULL keyword
#include <iostream>  //for std::cout, for testing only
#include <algorithm> //For sorting

//constructor for empty node
BDNode::BDNode() :
    vertex(-1), parent(NULL), mgrades(1, Grade(-1, -1)), d_index(-1), g_index(-1)
{ }

//constructor for non-empty node
//NOTE: node must still be added as a child of its parent after this constructor is called
BDNode::BDNode(int v, BDNode* p, std::vector<Grade>& grades, int g) :
    vertex(v), parent(p), mgrades(grades), d_index(-1), g_index(g)
{ }

//destructor -- IS THIS ADEQUATE???
BDNode::~BDNode()
{
    for(std::vector<BDNode*>::iterator it = children.begin(); it != children.end(); ++it)
        delete *it;
}

//returns the vertex index
int BDNode::get_vertex()
{
    return vertex;
}

//returns a pointer to the parent node
BDNode BDNode::get_parent()
{
    return *parent;
}

//sets the multigrades for this simplex
void BDNode::set_grades(std::vector<Grade>& grades)
{
    mgrades = grades;
    //Make sure grades are sorted
    update_grades();
}

//returns the multigrades for this simplex
const std::vector<Grade>* BDNode::grades() const
{
    return &mgrades;
}

//sets the global index for the simplex represented by this node
void BDNode::set_global_index(int i)
{
    g_index = i;
}

//returns the global index for the simplex represented by this node
int BDNode::global_index()
{
    return g_index;
}

//sets the dimension index for the simplex represented by this node
void BDNode::set_dim_index(int i)
{
    d_index = i;
}

//returns the dimension index for the simplex represented by this node
int BDNode::dim_index()
{
    return d_index;
}

//appends a new child to this node
//WARNING: this should only be used if vertex index of child is greater than vertex indexes of all other children (to preserve order of children vector)
void BDNode::append_child(BDNode* child)
{
    children.push_back(child);
}


//creates a new child node with given parameters and returns a pointer to the new node
// NOTE: if child with given vertex index already exists, then returns pointer to this node
// NOTE: global indexes must be re-computed after calling this function
BDNode* BDNode::add_child(int v, std::vector<Grade>& grades)
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
        //Add grades together and remove duplicates
        children[found]->mgrades.insert(children[found]->mgrades.end(), grades.begin(), grades.end());
        children[found]->update_grades();
        return children[found];
    }

    //if not found, create a new node
    BDNode* newnode = new BDNode(v, this, grades, -1);
    children.insert(children.begin() + max + 1, newnode);
    return newnode;
}//end add_child()


//returns a vector of pointers to children nodes
std::vector<BDNode*> BDNode::get_children()
{
    return children;
}

//TESTING
void BDNode::print()
{
    std::cout << "NODE: vertex " << vertex <<  "; global index: " << g_index << "; dim index: " << d_index << "; multi-indices: ";
    for (std::vector<Grade>::iterator it = mgrades.begin(); it != mgrades.end(); it++)
        std::cout << "(" << it->x << ", " << it->y << ") ";
    std::cout << "; parent: ";
    if(parent)
        std::cout << (*parent).get_vertex() << "; ";
    else
        std::cout << "NULL; ";
    std::cout << "children: ";
    if(children.size() == 0)

        std::cout << "NONE";
    for(unsigned i=0; i<children.size(); i++)
    {
        if(i>0)
            std::cout << ", ";
        std::cout << (*children[i]).get_vertex();
    }
    std::cout << "\n";
}

//makes sure the grades are all incomparable and in sorted order
//POSSIBLY CALLED TOO OFTEN, MAY SLOW CODE DOWN
void BDNode::update_grades()
{
    //Sort grades
    std::sort(mgrades.begin(), mgrades.end());

    //Iterate through the sorted grades and make sure they are all incomparable, delete the ones that are not
    for (std::vector<Grade>::iterator it = mgrades.begin(); it != mgrades.end() - 1;)
    {
        if (it->x <= (it + 1)->x && it->y <= (it + 1)->y) //grades are comparable
            it = mgrades.erase(it);
        else
            it++;
    }
}
