/* simplex tree class
 * works together with STNode class
 */

#include <set>

using namespace std;

//constructor: builds SimplexTree from a vector of points, with certain parameters
SimplexTree::SimplexTree(std::vector<Point> & pts, int dim, int depth, double dist)
{
	//store vector of points
	points = pts;
	point_dimension = dim;
	
	//store maximum depth and distance parameters
	max_depth = depth;
	max_distance = dist;
	
	//root node instantiated automatically??? I think so...
	
	//compute distances, stored in an array so that we can quickly look up the distance between any two points
	//also create sets of all the unique times and distances
	if(verbose) { cout << "COMPUTING DISTANCES:\n"; }
	int num_points = points.size();
	distances = new double[(num_points*(num_points-1))/2];					//DO I HAVE TO "delete" THIS LATER???
	int c=0;	//counter to track position in array of distances
	set<double> time_set;
	set<double> dist_set;
	dist_set.insert(0);
	for(int i=0; i<num_points; i++)
	{
		double *pc = points[i].get_coords();
		time_set.insert(points[i].get_birth());
		for(int j=i+1; j<num_points; j++)
		{
			double *qc = points[j].get_coords();
			double s=0;
			for(int k=0; k<point_dimension; k++)
				s += (pc[k] - qc[k])*(pc[k] - qc[k]);
			distances[c] = sqrt(s);
			if(distances[c] <= max_distance)
				dist_set.insert(distances[c]);
			c++;
		}
	}
	
	//convert distance and time sets to lists, for use in multi-indexing
	for(set<double>::iterator it=dist_set.begin(); it!=dist_set.end(); ++it)
    	dist_list.push_back(*it);		//is this inefficient, since it might involve resizing dist_list many times?
	
	for(set<double>::iterator it=time_set.begin(); it!=time_set.end(); ++it)
    	time_list.push_back(*it);		//is this also inefficient?
	
	//testing
	if(verbose)
	{	
		for(int i=0; i<num_points; i++)
			for(int j=i+1; j<num_points; j++)
			{
				int k = num_points*i - i*(3+i)/2 + j - 1;
				cout << "  distance from point " << i << " to point " << j << ": " << distances[k] << "\n";
			}
		
		cout << "  unique distances less than " << max_distance << ": ";
		for(int i=0; i<dist_list.size(); i++)
			cout << dist_list[i] << ", ";
		cout << "\n";
		
		cout << "  unique times: ";
		for(int i=0; i<time_list.size(); i++)
			cout << time_list[i] << ", ";
		cout << "\n";
	}
	
	//build simplex tree recursively
	//this also assigns global indexes to each simplex
	if(verbose) { cout << "BUILDING SIMPLEX TREE:\n"; }
	int gic=0;	//global index counter
	for(int i=0; i<points.size(); i++)
	{
		if(verbose) { cout << "  adding node " << i << " as child of root \n"; }
		
		STNode * node = new STNode(i, &root, time_index(points[i].get_birth()), 0, gic);			//DO I HAVE TO delete THIS OBJECT LATER???
		gic++;	//increment the global index counter
		
		vector<int> parent_indexes; //knowledge of ALL parent nodes is necessary for computing distance index of each simplex
		parent_indexes.push_back(i);
		
		build_subtree(*node, parent_indexes, points[i].get_birth(), 0, 1, gic);
	}
	
}//end constructor

//function to build (recursively) a subtree of the simplex tree
// IMPROVEMENT: this function could be rewritten to use INTEGER time and distance INDEXES, rather than DOUBLE time and distance VALUES
void SimplexTree::build_subtree(STNode &parent, vector<int> &parent_indexes, double prev_time, double prev_dist, int current_depth, int& gic)
{
	//no more recursion if we are at max depth
	if(current_depth == max_depth)
		return;
	
	//loop through all points that could be children of this node
	for(int j=parent_indexes.back()+1; j<points.size(); j++)
	{
		//look up distances from point j to each of its parents
		//distance index is maximum of prev_distance and each of these distances
		double current_dist = prev_dist;
		for(int k=0; k<parent_indexes.size(); k++)
		{
				int par = parent_indexes[k];
				double d = distances[points.size()*par - par*(3+par)/2 + j -1];
				if(d > current_dist)
					current_dist = d;
		}
		
		//compare distance to max distance
		if(current_dist <= max_distance)	//then we will add another node to the simplex tree
		{
			//compute time index of this new node
			double current_time = points[j].get_birth();
			if(current_time < prev_time)
				current_time = prev_time;
			
			//add the node
			if(verbose) { cout << "  adding node " << j << " as child of " << parent_indexes.back() << "; current_dist = " << current_dist << "\n"; }
			
			STNode * node = new STNode(j, &parent, time_index(current_time), dist_index(current_dist), gic);				//DO I HAVE TO "delete" THIS LATER???
			gic++;	//increment the global index counter
			
			parent_indexes.push_back(j); //next we will look for children of node j
			build_subtree(*node, parent_indexes, current_time, current_dist, current_depth+1, gic);
			parent_indexes.pop_back(); //finished adding children of node j
		}
	}
}

//searches for a distance value
//returns the index if found; otherwise returns -1
int SimplexTree::dist_index(double key)
{
	int min = 0;
	int max = dist_list.size() - 1;
	
	while(max >= min)
	{
		int mid = (min+max)/2;
		if(dist_list[mid] == key)
			return mid;
		if(dist_list[mid] < key)
			min = mid + 1;
		else
			max = mid - 1;
	}
	return -1;
} 

//searches for a time value
//returns the index if found; otherwise returns -1
int SimplexTree::time_index(double key)
{
	int min = 0;
	int max = time_list.size() - 1;
	
	while(max >= min)
	{
		int mid = (min+max)/2;
		if(time_list[mid] == key)
			return mid;
		if(time_list[mid] < key)
			min = mid + 1;
		else
			max = mid - 1;
	}
	return -1;
} 

//computes a boundary matrix for simplices of a given dimension at the specified multi-index
MapMatrix* SimplexTree::get_boundary_mx(int time, int dist, int dim)
{
	if(verbose) { cout << "  boundary matrix for dimension " << dim << " at index (" << time << ", " << dist << "): \n"; }
	
	//find (global indexes of) all simplices of dimension dim that exist at (time, dist)
	vector<int> cols;
	find_nodes(root, 0, cols, time, dist, dim);
	
	if(verbose)
	{
		cout << "    simplices of dimension " << dim << " at index (" << time << ", " << dist << "): ";
		for(int i=0; i<cols.size(); i++)
			cout << cols[i] << ", ";
		cout << "\n";
	}
	
	//find (global indexes of) all simplices of dimension dim-1 that exist at (time, dist)
	vector<int> rows;
	find_nodes(root, 0, rows, time, dist, dim-1);
	
	if(verbose)
	{
		cout << "    simplices of dimension " << (dim-1) << " at index (" << time << ", " << dist << "): ";
		for(int i=0; i<rows.size(); i++)
			cout << rows[i] << ", ";
		cout << "\n";
	}
	
	//create the matrix
	MapMatrix* mat = new MapMatrix(rows.size(), cols.size());			//DELETE this object later???
	
	//loop through columns
	for(int j=0; j<cols.size(); j++)
	{
//		cout << "      finding boundary of simplex " << cols[j] << "\n";
		
		//find all vertices of the simplex corresponding to this column
		vector<int> verts = find_vertices(cols[j]);
		
		//find all boundary simplices of this simplex
		for(int k=0; k<verts.size(); k++)
		{
//			cout << "        finding boundary simplex " << k << "\n";
			
			//make a list of all vertices in verts[] except verts[k]
			vector<int> facet;
			for(int l=0; l<verts.size(); l++)
				if(l != k)
					facet.push_back(verts[l]);
			
			//look up global index of the boundary simplex
			int gi = find_index(facet);
//			cout << "        found boundary simplex with global index " << gi << "\n";
			
			//look up local index of the simplex (in the vector rows[])
			int min = 0;
			int max = rows.size();
			int mid;
			while(max >= min)
			{
				mid = (min+max)/2;
				if(rows[mid] == gi)
					break;	//found it at rows[mid]
				else if( rows[mid] < gi)
					min = mid + 1;
				else
					max = mid -1;
			}
			
			//for this boundary simplex, enter "1" in the appropriate cell in the matrix
//			cout << "     enter 1 in column " << j << ", row " << mid << "\n";
			(*mat).set(mid+1,j+1);
		}
		
	}
	
	//return the MapMatrix
	return mat;
}//end get_boundary_mx()

//computes a matrix representing the map [B+C,D], the direct sum of two inclusion maps into the dim-skeleton at the specified multi-index
MapMatrix* SimplexTree::get_merge_mx(int time, int dist, int dim)
{
	if(verbose) { cout << "  boundary matrix for dimension " << dim << " at index (" << time << ", " << dist << "): \n"; }
	
	//find (global indexes of) all simplices of dimension dim that exist at (time, dist)
	vector<int> vec_d;
	find_nodes(root, 0, vec_d, time, dist, dim);
	
	if(verbose)
	{
		cout << "    D: simplices of dimension " << dim << " at index (" << time << ", " << dist << "): ";
		for(int i=0; i<vec_d.size(); i++)
			cout << vec_d[i] << ", ";
		cout << "\n";
	}
	
	//find (global indexes of) all simplices of dimension dim that exist at (time-1, dist)
	vector<int> vec_c;
	find_nodes(root, 0, vec_c, time-1, dist, dim);
	
	if(verbose)
	{
		cout << "    C: simplices of dimension " << dim << " at index (" << (time-1) << ", " << dist << "): ";
		for(int i=0; i<vec_c.size(); i++)
			cout << vec_c[i] << ", ";
		cout << "\n";
	}
	
	//find (global indexes of) all simplices of dimension dim that exist at (time, dist-1)
	vector<int> vec_b;
	find_nodes(root, 0, vec_b, time, dist-1, dim);
	
	if(verbose)
	{
		cout << "    B: simplices of dimension " << dim << " at index (" << time << ", " << (dist-1) << "): ";
		for(int i=0; i<vec_b.size(); i++)
			cout << vec_b[i] << ", ";
		cout << "\n";
	}
	
	//create the matrix
	MapMatrix* mat = new MapMatrix(vec_d.size(), vec_b.size()+vec_c.size());			//DELETE this object later???
	
	//add nodes for inclusion map B->D
	int r = 0;	//row counter
	for(int i=0; i<vec_b.size(); i++)
	{
		while(vec_d[r] != vec_b[i])
			r++;
		(*mat).set(r+1,i+1);
	}
	
	//add nodes for inclusion map C->D
	r = 0;	//row counter
	for(int i=0; i<vec_c.size(); i++)
	{
		while(vec_d[r] != vec_c[i])
			r++;
		(*mat).set(r+1,i+1+vec_b.size());
	}
	
	//return the MapMatrix
	return mat;
}//end get_merge_mx()


//recursively search tree for simplices of specified dimension that exist at specified multi-index
void SimplexTree::find_nodes(STNode &node, int level, vector<int> &vec, int time, int dist, int dim)
{
	//consider current node
	if( (level == dim+1) && (node.get_birth() <= time) && (node.get_dist() <= dist) )
	{
		vec.push_back(node.get_global_index());
	}
	
	//move on to children nodes
	if(level <= dim)
	{
		vector<STNode*> kids = node.get_children();
		for(int i=0; i<kids.size(); i++)
			find_nodes(*kids[i], level+1, vec, time, dist, dim);
	}
}

//given a global index, return (a vector containing) the vertices of the simplex
vector<int> SimplexTree::find_vertices(int gi)
{
	vector<int> vertices;
	find_vertices_recursively(vertices, root, gi);
	return vertices;
}

//recursively search for a global index and keep track of vertices
void SimplexTree::find_vertices_recursively(vector<int> &vertices, STNode &node, int key)
{
	//search children of current node for greatest index less than or equal to key
	vector<STNode*> kids = node.get_children();
	
	
	int min = 0;
	int max = kids.size() -1;
	while (max >= min)
	{
    	int mid = (max + min)/2;
		if( (*kids[mid]).get_global_index() == key)	//key found at this level
		{
			vertices.push_back( (*kids[mid]).get_vertex() );
			return;
		}
		else if( (*kids[mid]).get_global_index() <= key)
			min = mid +1;
		else
			max = mid -1;
    }
	
	//if we get here, then key not found
	//so we want greatest index less than key, which is found at kids[max]
	vertices.push_back( (*kids[max]).get_vertex() );
	//now search children of kids[max]
	find_vertices_recursively(vertices, *kids[max], key);
}

//given a sorted vector of vertex indexes, return the global index of the corresponding simplex (or -1 if it doesn't exist)
int SimplexTree::find_index(vector<int>& vertices)
{
	//start at the root node
	STNode* node = &root;
	vector<STNode*> kids = (*node).get_children();
	
	//search the vector of children nodes for each vertex
	for(int i=0; i<vertices.size(); i++)
	{
		//binary search for vertices[i]
		int key = vertices[i];
		int min = 0;
		int max = kids.size();
		int mid;
		while(max >= min)
		{
			mid = (min+max)/2;
//			cout << "          testing simplex " << (*kids[mid]).get_global_index() << ", vertex " << (*kids[mid]).get_vertex() << "; looking for " << key << "\n";
			if( (*kids[mid]).get_vertex() == key)
				break;	//found it at kids[mid]
			else if( (*kids[mid]).get_vertex() < key)
				min = mid + 1;
			else
				max = mid -1;
		}
		
		if(max < mid)	//didn't find it
			return -1;
		else			//found it, so update node and kids
		{
			node = kids[mid];
			kids = (*node).get_children();
		}
	}
	
	//return global index
	return (*node).get_global_index();
}//end find_index()




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


//returns the total number of simplices represented in the simplex tree
int SimplexTree::get_num_simplices()	
{
	//start at the root node
	STNode* node = &root;
	vector<STNode*> kids = (*node).get_children();
	
	while(kids.size() > 0) //move to the last node in the next level of the tree
	{
		node = kids.back();
		kids = (*node).get_children();
	}
	
	//we have found the last node in the entire tree, so return its global index +1
	return (*node).get_global_index() + 1;
}


