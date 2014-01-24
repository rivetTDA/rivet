/* simplex tree class
 * works together with STNode class
 */

using namespace std;

//constructor
SimplexTree::SimplexTree(std::vector<Point> & pts, int dim, int depth, double dist)
{
	//store vector of points
	points = pts;
	point_dimension = dim;
	
	//store maximum depth and distance parameters
	max_depth = depth;
	max_distance = dist;
	
	//root node instantiated automatically??? I think so...
	
	//compute distances
	if(verbose) { cout << "COMPUTING DISTANCES:\n"; }
	int num_points = points.size();
	distances = new double[(num_points*(num_points-1))/2];				//DO I HAVE TO "delete" THIS LATER???
	int c=0;	//counter to track position in array of edges
	for(int i=0; i<num_points; i++)
	{
		//Point p = points.at(i);
		double *pc = points[i].get_coords();
		for(int j=i+1; j<num_points; j++)
		{
			//Point q = points.at(j);
			double *qc = points[j].get_coords();
			double s=0;
			for(int k=0; k<point_dimension; k++)
				s += (pc[k] - qc[k])*(pc[k] - qc[k]);
			distances[c] = sqrt(s);
			c++;
		}
	}
	
	//testing
	if(verbose)
	{	
		for(int i=0; i<num_points; i++)
		{
			for(int j=i+1; j<num_points; j++)
			{
				int k = num_points*i - i*(3+i)/2 + j - 1;
				cout << "  distance from point " << i << " to point " << j << ": " << distances[k] << "\n";
			}
		}
	}
	
	//build simplex tree recursively
	if(verbose) { cout << "BUILDING SIMPLEX TREE:\n"; }
	for(int i=0; i<points.size(); i++)
	{
		if(verbose) { cout << "  adding node " << i << " as child of root \n"; }
		
		STNode * node = new STNode(i, &root, points[i].get_birth(), 0);	//DO I HAVE TO delete THIS???
		
		build_subtree(*node, i, points[i].get_birth(), 0, 1);
	}
	
	
}//end constructor




//function to build (recursively) a subtree of the simplex tree
void SimplexTree::build_subtree(STNode &parent, int index, double current_time, double current_dist, int current_depth)
{
	//no more recursion if we are at max depth
	if(current_depth == max_depth)
		return;
	
	//loop through all points that could be children of this node
	for(int j=index+1; j<points.size(); j++)
	{
		//look up distance from index point to point j
		double d = distances[points.size()*index - index*(3+index)/2 + j -1];
		
		//compare distance to max distance
		if(d <= max_distance)	//then we will add another node to the simplex tree
		{
			//compute multi-index of this new node
			if(current_dist > d)
				d = current_dist;
			double t = points[j].get_birth();
			if(current_time > t)
				t = current_time;
			
			//add the node
			if(verbose) { cout << "  adding node " << j << " as child of " << index << "\n"; }
			STNode * node = new STNode(j, &parent, t, d);			//DO I HAVE TO "delete" THIS LATER???
			build_subtree(*node, j, t, d, current_depth+1);
		}
	}
}

// RECURSIVELY PRINT TREE
void SimplexTree::print()
{
	print_subtree(root, 1);
	
}

void SimplexTree::print_subtree(STNode &node, int indent)
{
	//print current node
	for(int i=0; i<indent; i++)
		std::cout << "  ";
	node.print();
	
	//print children nodes
	vector<STNode*> kids = node.get_children();
	for(int i=0; i<kids.size(); i++)
		print_subtree(*kids[i], indent+1);
}





