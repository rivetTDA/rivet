/* simplex tree node class
 * stores a node that is used to build a simplex tree
 */


//constructor for empty node
STNode::STNode()
{
	vertex = -1;
	parent = NULL;
	birth = -1;
	dist = -1;
	g_index = -1;
}

//constructor for non-empty node
STNode::STNode(int v, STNode* p, int b, int d, int g)
{
	//set private values
	vertex = v;
	parent = p;
	birth = b;
	dist = d;
	g_index = g;
	
	//add this node as a child of its parent
	if(parent)
		(*parent).add_child(this);
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

//returns the minimum time at which this simplex exits
int STNode::get_birth()
{
	return birth;
}	

//returns the minimum distance at which this simplex exists
int STNode::get_dist()
{
	return dist;
}

//sets the global index for the simplex represented by this node
void STNode::set_global_index(int i)
{
	g_index = i;
}

//returns the global index for the simplex represented by this node
int STNode::get_global_index()
{
	return g_index;
}


//adds a new child to this node
void STNode::add_child(STNode* child)
{
	children.push_back(child);
}

//returns a vector of pointers to children nodes
std::vector<STNode*> STNode::get_children()
{
	return children;
}





//print a text representation of this node
void STNode::print()
{
	std::cout << "NODE: vertex " << vertex <<  "; global index: " << g_index << "; multi-index: (" << birth << ", " << dist << "); parent: ";
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
