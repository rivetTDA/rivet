/* simplex tree class
 * works together with STNode class
 */

#include "simplex_tree.h"


#include <set>
#include <vector>
#include <math.h>
#include <iostream>
#include <limits>   //std::numeric_limits


//SimplexTree constructor; requires dimension of homology to be computed and verbosity parameter
SimplexTree::SimplexTree(int dim, int v) :
    root(new STNode()), x_grades(0), y_grades(0), hom_dim(dim), verbosity(v)
{ }

//destructor
SimplexTree::~SimplexTree()
{
    delete root;
}

//adds a simplex (including all of its faces) to the SimplexTree
//if simplex or any of its faces already exist, they are not re-added
//WARNING: doesn't update global indexes or dimension indexes!!!
/* THIS FUNCTION NO LONGER WORKS AND MUST BE UPDATED!!!
void SimplexTree::add_simplex(std::vector<int> & vertices, int x, int y)
{
	//add the simplex and all of its faces
    add_faces(vertices, x, y);
	
    //update grade_x_values (this is sort of a hack; the grade_x_values structure could be improved)
    for(int i = grade_x_values.size(); i <= x; i++)
        grade_x_values.push_back(i);

    //update grade_y_values
    for(int i = grade_y_values.size(); i <= y; i++)
        grade_y_values.push_back(i);
	
    //update global indexes  ---- IS THIS INEFFICIENT???
//	update_global_indexes();
}//end add_simplex()*/

//recursively adds faces of a simplex to the SimplexTree
//WARNING: doesn't update global data structures (grade_x_values, grade_y_values, or global indexes), so should only be called from add_simplex()
void SimplexTree::add_faces(std::vector<int> & vertices, int x, int y)
{
	//loop through vertices, adding them if necessary to the SimplexTree
    STNode* node = root;
    for(unsigned i=0; i<vertices.size(); i++)
	{
        node = (*node).add_child(vertices[i], x, y);	//if a child with specified vertex index already exists, then nothing is added, but that child is returned
	}
	
	//ensure that the other faces of this simplex are in the tree
    for(unsigned i=0; i<vertices.size()-1; i++) // -1 is because we already know that the face consisting of all the vertices except for the last one is in the SimplexTree
	{
		//form vector consisting of all the vertices except for vertices[i]
		std::vector<int> facet;
        for(unsigned k=0; k<vertices.size(); k++)
		{
			if(k != i)
				facet.push_back(vertices[k]);
		}
		
		//ensure that facet is a simplex in SimplexTree
        add_faces(facet, x, y);
	}
}//end add_faces()



//updates the global indexes of all simplices in this simplex tree
void SimplexTree::update_global_indexes()
{
	int gic = 0;	//global index counter
    update_gi_recursively(root, gic);
}

//recursively update global indexes of simplices
void SimplexTree::update_gi_recursively(STNode* node, int &gic)
{
	//loop through children of current node
    std::vector<STNode*> kids = node->get_children();
    for(unsigned i=0; i<kids.size(); i++)
	{
		//update global index of this child
		(*kids[i]).set_global_index(gic);
		gic++;
		
		//move on to its children
        update_gi_recursively(kids[i], gic);
	}
}

//updates the dimension indexes (reverse-lexicographical multi-grade order) for simplices of dimension (hom_dim-1), hom_dim, and (hom_dim+1)
void SimplexTree::update_dim_indexes()
{
    //build the lists of pointers to simplices of appropriate dimensions
    build_dim_lists_recursively(root, 0, hom_dim);

    //update the dimension indexes in the tree
    int i=0;
    for(SimplexSet::iterator it=ordered_low_simplices.begin(); it!=ordered_low_simplices.end(); ++it)
        (*it)->set_dim_index(i++);

    i=0;
    for(SimplexSet::iterator it=ordered_simplices.begin(); it!=ordered_simplices.end(); ++it)
        (*it)->set_dim_index(i++);

    i=0;
    for(SimplexSet::iterator it=ordered_high_simplices.begin(); it!=ordered_high_simplices.end(); ++it)
        (*it)->set_dim_index(i++);
}

//recursively build lists to determine dimension indexes
void SimplexTree::build_dim_lists_recursively(STNode* node, int cur_dim, int hom_dim)
{
    //get children of current node
    std::vector<STNode*> kids = node->get_children();

    //check dimensions and add childrein to appropriate list
    if(cur_dim == hom_dim - 1)
        ordered_low_simplices.insert(kids.begin(), kids.end());
    else if(cur_dim == hom_dim)
        ordered_simplices.insert(kids.begin(), kids.end());
    else if(cur_dim == hom_dim + 1)
        ordered_high_simplices.insert(kids.begin(), kids.end());

    //recurse through children
    for(unsigned i=0; i<kids.size(); i++)
    {
        build_dim_lists_recursively(kids[i], cur_dim+1, hom_dim);
    }
}


//builds SimplexTree representing a bifiltered Vietoris-Rips complex from discrete data
//requires a list of birth times (one for each point) and a list of distances between pairs of points
//NOTE: automatically computes global indexes and dimension indexes
//CONVENTION: the x-coordinate is "birth time" for points and the y-coordinate is "distance" between points
void SimplexTree::build_VR_complex(std::vector<unsigned>& times, std::vector<unsigned>& distances, unsigned num_x, unsigned num_y)
{
    x_grades = num_x;
    y_grades = num_y;

    //build simplex tree recursively
	//this also assigns global indexes to each simplex
	if(verbosity >= 2) { std::cout << "BUILDING SIMPLEX TREE:\n"; }
    unsigned gic=0;	//global index counter
    for(unsigned i=0; i<times.size(); i++)
	{
		//create the node and add it as a child of root
        if(verbosity >= 10) { std::cout << "  adding node " << i << " as child of root \n"; }
		
        STNode* node = new STNode(i, root, times[i], 0, gic);			//delete THIS OBJECT LATER!
        root->append_child(node);
		gic++;	//increment the global index counter
		
		//recursion
        std::vector<unsigned> parent_indexes; //knowledge of ALL parent nodes is necessary for computing distance index of each simplex
		parent_indexes.push_back(i);
		
        build_VR_subtree(times, distances, *node, parent_indexes, times[i], 0, 1, gic);
	}

    //compute dimension indexes
    update_dim_indexes();
}//end build_VR_complex()

//function to build (recursively) a subtree of the simplex tree
// IMPROVEMENT: this function could be rewritten to use INTEGER time and distance INDEXES, rather than DOUBLE time and distance VALUES
void SimplexTree::build_VR_subtree(std::vector<unsigned>& times, std::vector<unsigned>& distances, STNode& parent, std::vector<unsigned>& parent_indexes, unsigned prev_time, unsigned prev_dist, unsigned cur_dim, unsigned& gic)
{
    //loop through all points that could be children of this node
    for(unsigned j=parent_indexes.back()+1; j<times.size(); j++)
	{
		//look up distances from point j to each of its parents
		//distance index is maximum of prev_distance and each of these distances
        unsigned current_dist = prev_dist;
        for(unsigned k=0; k<parent_indexes.size(); k++)
		{
                unsigned p = parent_indexes[k];
                unsigned d = distances[ (j*(j-1))/2 + p ]; //the distance between points p and j, with p < j
                if(d > current_dist)
					current_dist = d;
		}
		
        //see if the distance is permitted
        if(current_dist < std::numeric_limits<unsigned>::max())	//then we will add another node to the simplex tree
		{
			//compute time index of this new node
            unsigned current_time = times[j];
			if(current_time < prev_time)
				current_time = prev_time;
			
			//create the node and add it as a child of its parent
            if(verbosity >= 10) { std::cout << "  adding node " << j << " as child of " << parent_indexes.back() << "; current_dist = " << current_dist << "\n"; }
			
            STNode* node = new STNode(j, &parent, current_time, current_dist, gic);				//delete THIS OBJECT LATER!
			parent.append_child(node);
			gic++;	//increment the global index counter
			
			//recursion
            if(cur_dim <= hom_dim)  //then consider simplices of the next dimension
			{
				parent_indexes.push_back(j); //next we will look for children of node j
                build_VR_subtree(times, distances, *node, parent_indexes, current_time, current_dist, cur_dim+1, gic);
				parent_indexes.pop_back(); //finished adding children of node j
			}
		}
	}
}//end build_subtree()

//returns a matrix of boundary information for simplices of the given dimension (with multi-grade info)
//columns ordered according to dimension index (reverse-lexicographic order with respect to multi-grades)
MapMatrix* SimplexTree::get_boundary_mx(int dim)
{
    //select set of simplices of dimension dim
    SimplexSet* simplices;
    int num_rows;

    if(dim == hom_dim)
    {
        simplices = &ordered_simplices;
        num_rows = ordered_low_simplices.size();
    }
    else if(dim == hom_dim + 1)
    {
        simplices = &ordered_high_simplices;
        num_rows = ordered_simplices.size();
    }
    else
        throw std::runtime_error("attempting to compute boundary matrix for improper dimension");

    //create the MapMatrix
    MapMatrix* mat = new MapMatrix(num_rows, simplices->size());			//DELETE this object later!

    //loop through simplices, writing columns to the matrix
    int col = 0;    //column counter
    for(SimplexSet::iterator it=simplices->begin(); it!=simplices->end(); ++it)
    {
        write_boundary_column(mat, *it, col, 0);

        col++;
    }

    //return the matrix
    return mat;
}//end get_boundary_mx()

//returns matrices for the merge map [B+C,D], the boundary map B+C, and the multi-grade information
DirectSumMatrices SimplexTree::get_merge_mxs()
{
    //data structures
    int num_rows = ordered_low_simplices.size();
    int num_cols = ordered_simplices.size();
    MapMatrix* boundary = new MapMatrix(2*num_rows, 2*num_cols);			//DELETE this object later!
    MapMatrix* merge = new MapMatrix(num_cols, 2*num_cols);       //DELETE this object later!
    IndexMatrix* end_cols = new IndexMatrix(y_grades + 1, x_grades + 1);     //DELETE later; is this the correct size?
    int col = - 1;  //column counter for boundary and merge matrices
    int b = 0;      //counter for simplices in B component
    int c = 0;      //counter for simplices in C component
    SimplexSet::iterator it_b = ordered_simplices.begin(); //iterator for simplices in B component
    SimplexSet::iterator it_c = ordered_simplices.begin(); //iterator for simplices in C component

    //loop through multi-grades, writing columns into the matrices
    for(int y=0; y<=y_grades; y++)  //rows                 <--- CHECK! DO WE WANT <= HERE? FOR NOW, YES.
    {
        for(int x=0; x<=x_grades; x++)  //columns          <--- CHECK! DO WE WANT <= HERE? FOR NOW, YES.
        {
            //process simplices for the current multi-grade (x,y)
//            std::cout << "grade (" << x << "," << y << "): ";

            //first, insert columns for simplices that appear in B at the multi-grade (x-1,y)
            while( (it_b != ordered_simplices.end()) && ((*it_b)->grade_x() == x - 1) && ((*it_b)->grade_y() == y) )
            {
//                std::cout << "adding column from B, ";
                col++;
                write_boundary_column(boundary, *it_b, col, 0);
                merge->set(b, col);
                b++;
                ++it_b;
            }

            //second, insert columns for simplices that appear in C at the multi-grade (x,y-1)
            while( (it_c != ordered_simplices.end()) && ((*it_c)->grade_x() == x) && ((*it_c)->grade_y() == y - 1) )
            {
//                std::cout << "adding column from C, ";
                col++;
                write_boundary_column(boundary, *it_c, col, num_rows);
                merge->set(c, col);
                c++;
                ++it_c;
            }

            //finished a multi-grade; record column index
            end_cols->set(y, x, col);

//            std::cout << std::endl;
        }
    }

    //return data
    DirectSumMatrices dsm = { boundary, merge, end_cols };
    return dsm;
}//end get_merge_mxs()

//returns matrices for the split map [A,B+C], the boundary map B+C, and the multi-grade information
DirectSumMatrices SimplexTree::get_split_mxs()
{
    //sizes
    int num_rows = ordered_simplices.size();
    int num_cols = ordered_high_simplices.size();

    //first, produce the boundary matrix and its index matrix (for B+C "high" simplices)
    MapMatrix* boundary = new MapMatrix(2*num_rows, 2*num_cols);			//DELETE this object later!
    IndexMatrix* end_cols = new IndexMatrix(y_grades + 1, x_grades + 1);     //DELETE later; <<<==== OPTIMIZE TO GET RID OF THE +1
    SimplexSet::iterator it_b = ordered_high_simplices.begin(); //iterator for "high" simplices in B component
    SimplexSet::iterator it_c = ordered_high_simplices.begin(); //iterator for "high" simplices in C component
    int col = - 1;  //column counter for boundary matrix

    //  loop through multi-grades, writing columns into the matrices
    for(int y=0; y<=y_grades; y++)  //rows                 <--- CHECK! DO WE WANT <= HERE? FOR NOW, YES.
    {
        for(int x=0; x<=x_grades; x++)  //columns          <--- CHECK! DO WE WANT <= HERE? FOR NOW, YES.
        {
            //first, insert columns for simplices that appear in B at the multi-grade (x-1,y)
            while( (it_b != ordered_high_simplices.end()) && ((*it_b)->grade_x() == x - 1) && ((*it_b)->grade_y() == y) )
            {
                col++;
                write_boundary_column(boundary, *it_b, col, 0);
                ++it_b;
            }

            //second, insert columns for simplices that appear in C at the multi-grade (x,y-1)
            while( (it_c != ordered_high_simplices.end()) && ((*it_c)->grade_x() == x) && ((*it_c)->grade_y() == y - 1) )
            {
                col++;
                write_boundary_column(boundary, *it_c, col, num_rows);
                ++it_c;
            }

            //finished a multi-grade; record column index
            end_cols->set(y, x, col);
        }
    }

    //now, produce the split matrix [A,B+C]
    MapMatrix* split = new MapMatrix(2*num_rows, num_rows);       //DELETE this object later!
    for(int i=0; i<num_rows; i++)   //IS IT THIS SIMPLE???
    {
        split->set(i, i);
        split->set(i+num_rows, i);
    }

    //return data
    DirectSumMatrices dsm = { boundary, split, end_cols };
    return dsm;
}//end get_split_mxs()

//writes boundary information for simplex represented by sim in column col of matrix mat; offset allows for block matrices such as B+C
void SimplexTree::write_boundary_column(MapMatrix* mat, STNode* sim, int col, int offset)
{
    //get vertex list for this simplex
    std::vector<int> verts = find_vertices(sim->global_index());

    //if this is a 0-simplex, then there is nothing to do
    if(verts.size() == 1)
        return;

    //find all facets of this simplex
    for(unsigned k=0; k<verts.size(); k++)
    {
       //facet vertices are all vertices in verts[] except verts[k]
        std::vector<int> facet;
        for(unsigned l=0; l<verts.size(); l++)
            if(l != k)
                facet.push_back(verts[l]);

        //look up dimension index of the facet
        STNode* facet_node = find_simplex(facet);
        if(facet_node == NULL)
            throw std::runtime_error("facet simplex not found");
        int facet_di = facet_node->dim_index();

        //for this boundary simplex, enter "1" in the appropriate cell in the matrix
        mat->set(facet_di + offset, col);
    }
}//end write_col();


//returns a matrix of column indexes to accompany MapMatrices
//  entry (i,j) gives the last column of the MapMatrix that corresponds to multigrade (i,j)
IndexMatrix* SimplexTree::get_index_mx(int dim)
{
    //select set of simplices of dimension dim
    SimplexSet* simplices;

    if(dim == hom_dim)
        simplices = &ordered_simplices;
    else if(dim == hom_dim + 1)
        simplices = &ordered_high_simplices;
    else
        throw std::runtime_error("attempting to compute index matrix for improper dimension");

    //create the IndexMatrix
    int x_size = x_grades;
    int y_size = y_grades;
    IndexMatrix* mat = new IndexMatrix(y_size, x_size);			//DELETE this object later!

    if(!simplices->empty()) //then there is at least one simplex
    {
        //initialize to -1 the entries of end_col matrix before multigrade of the first simplex
        int cur_entry = 0;   //tracks previously updated multigrade in end-cols

        SimplexSet::iterator it = simplices->begin();
        for( ; cur_entry < (*it)->grade_x() + (*it)->grade_y()*x_size; cur_entry++)
            mat->set(cur_entry/x_size, cur_entry%x_size, -1);

        //loop through simplices
        int col = 0;    //column counter
        for( ; it!=simplices->end(); ++it)
        {
            //get simplex
            STNode* simplex = *it;
            int cur_x = simplex->grade_x();
            int cur_y = simplex->grade_y();

            //if some multigrades were skipped, store previous column number in skipped cells of end_col matrix
            for( ; cur_entry < cur_x + cur_y*x_size; cur_entry++)
                mat->set(cur_entry/x_size, cur_entry%x_size, col-1);

            //store current column number in cell of end_col matrix for this multigrade
            mat->set(cur_y, cur_x, col);

            //increment column counter
            col++;
        }

        //store final column number for any cells in end_cols not yet updated
        for( ; cur_entry < x_size * y_size; cur_entry++)
            mat->set(cur_entry/x_size, cur_entry%x_size, col-1);
    }
    else    //then there are no simplices, so fill the IndexMatrix with -1 values
    {
        for(int i=0 ; i < x_size * y_size; i++)
            mat->set(i/x_size, i%x_size, -1);
    }

    //return the matrix
    return mat;
}//end get_index_mx()

//returns a matrix of column indexes offset in each direction, for the boundary_A matrix in compute_eta()
IndexMatrix* SimplexTree::get_offset_index_mx(int dim)
{
    //select set of simplices of dimension dim
    SimplexSet* simplices;

    if(dim == hom_dim)
        simplices = &ordered_simplices;
    else if(dim == hom_dim + 1)
        simplices = &ordered_high_simplices;
    else
        throw std::runtime_error("attempting to compute index matrix for improper dimension");

    //create the IndexMatrix
    int x_size = x_grades + 1;     //    <<<==== OPTIMIZE TO GET RID OF THE +1
    int y_size = y_grades + 1;     //    <<<==== OPTIMIZE TO GET RID OF THE +1
    IndexMatrix* mat = new IndexMatrix(y_size, x_size);			//DELETE later

    if(!simplices->empty()) //then there is at least one simplex
    {
        //initialize to -1 the entries of end_col matrix before multigrade of the first simplex
        int cur_entry = 0;   //tracks previously updated multigrade in end-cols
        SimplexSet::iterator it = simplices->begin();
        int first_entry = ((*it)->grade_x() + 1) + ((*it)->grade_y() + 1)*x_size; //note that grades are offset by 1
        for( ; cur_entry < first_entry; cur_entry++)
            mat->set(cur_entry/x_size, cur_entry%x_size, -1);

        //loop through simplices
        int col = 0;    //column counter
        for( ; it!=simplices->end(); ++it)
        {
            //get simplex
            STNode* simplex = *it;
            int cur_x = simplex->grade_x() + 1; //note that grades are offset by 1
            int cur_y = simplex->grade_y() + 1; //note that grades are offset by 1

            //if some multigrades were skipped, store previous column number in skipped cells of end_col matrix
            for( ; cur_entry < cur_x + cur_y*x_size; cur_entry++)
                mat->set(cur_entry/x_size, cur_entry%x_size, col-1);

            //store current column number in cell of end_col matrix for this multigrade
            mat->set(cur_y, cur_x, col);

            //increment column counter
            col++;
        }

        //store final column number for any cells in end_cols not yet updated
        for( ; cur_entry < x_size * y_size; cur_entry++)
            mat->set(cur_entry/x_size, cur_entry%x_size, col-1);
    }
    else    //then there are no simplices, so fill the IndexMatrix with -1 values
    {
        for(int i=0 ; i < x_size * y_size; i++)
            mat->set(i/x_size, i%x_size, -1);
    }

    //return the matrix
    return mat;
}//end get_offset_index_mx()

//recursively search tree for simplices of specified dimension that exist at specified multi-index
void SimplexTree::find_nodes(STNode &node, int level, std::vector<int> &vec, int time, int dist, int dim)
{
	//if either time, dist, or dim is negative, then there are no nodes
	if(time < 0 || dist < 0 || dim < 0)
	{
		return;
	}
	
	//consider current node
    if( (level == dim+1) && (node.grade_x() <= time) && (node.grade_y() <= dist) )
	{
        vec.push_back(node.global_index());
	}
	
	//move on to children nodes
	if(level <= dim)
	{
		std::vector<STNode*> kids = node.get_children();
        for(unsigned i=0; i<kids.size(); i++)
			find_nodes(*kids[i], level+1, vec, time, dist, dim);
	}
}

//given a global index, return (a vector containing) the vertices of the simplex
std::vector<int> SimplexTree::find_vertices(int gi)
{
	std::vector<int> vertices;
	find_vertices_recursively(vertices, root, gi);
	return vertices;
}

//recursively search for a global index and keep track of vertices
void SimplexTree::find_vertices_recursively(std::vector<int> &vertices, STNode* node, int key)
{
	//search children of current node for greatest index less than or equal to key
    std::vector<STNode*> kids = node->get_children();
	
	int min = 0;
	int max = kids.size() -1;
	while (max >= min)
	{
    	int mid = (max + min)/2;
        if( (*kids[mid]).global_index() == key)	//key found at this level
		{
			vertices.push_back( (*kids[mid]).get_vertex() );
			return;
		}
        else if( (*kids[mid]).global_index() <= key)
			min = mid +1;
		else
			max = mid -1;
    }
	
	//if we get here, then key not found
	//so we want greatest index less than key, which is found at kids[max]
	vertices.push_back( (*kids[max]).get_vertex() );
	//now search children of kids[max]
    find_vertices_recursively(vertices, kids[max], key);
}

//given a sorted vector of vertex indexes, return a pointer to the node representing the corresponding simplex
STNode* SimplexTree::find_simplex(std::vector<int>& vertices)
{
    //start at the root node
    STNode* node = root;
    std::vector<STNode*> kids = node->get_children();

    //search the vector of children nodes for each vertex
    for(unsigned i=0; i<vertices.size(); i++)
    {
        //binary search for vertices[i]
        int key = vertices[i];
        int min = 0;
        int max = kids.size() - 1;
        int mid;
        while(max >= min)
        {
            mid = (min+max)/2;
            if( (*kids[mid]).get_vertex() == key)
                break;	//found it at kids[mid]
            else if( (*kids[mid]).get_vertex() < key)
                min = mid + 1;
            else
                max = mid - 1;
        }

        if(max < min)	//didn't find it
            return NULL;
        else			//found it, so update node and kids
        {
            node = kids[mid];
            kids = node->get_children();
        }
    }

    //return global index
    return node;
}

//returns the (time, dist) multi-index of the simplex with given global simplex index
//also returns the dimension of the simplex
SimplexData SimplexTree::get_simplex_data(int index)
{
	STNode* target = NULL;
    std::vector<STNode*> kids = root->get_children();
	int dim = 0;
	
	while(target == NULL)
	{
		if(kids.size() == 0)
		{
			std::cerr << "ERROR: vector of size zero in SimplexTree::get_multi_index()\n";
			throw std::exception();
		}
		
		//binary search for index
		int min = 0;
		int max = kids.size() - 1;
		int mid;
		while(max >= min)
		{
			mid = (min+max)/2;
            if( kids[mid]->global_index() == index)	//found it at kids[mid]
			{
				target = kids[mid];
				break;
			}
            else if( kids[mid]->global_index() < index)
				min = mid + 1;
			else
				max = mid - 1;
		}
		if(max < min)	//didn't find it yet
		{
			kids = kids[max]->get_children();
			dim++;
		}
	}
	
    SimplexData sd = { target->grade_x(), target->grade_y(), dim };
	return sd;	//TODO: is this good design?
}

//computes a boundary matrix, using given orders on simplices of dimensions d (cofaces) and d-1 (faces)
	// coface_global is a map: order_simplex_index -> global_simplex_index
	// face_order is a map: global_simplex_index -> order_simplex_index
MapMatrix* SimplexTree::get_boundary_mx(std::vector<int> coface_global, std::map<int,int> face_order)
{

    //create the matrix
	int num_cols = coface_global.size();
	int num_rows = face_order.size();
    MapMatrix* mat = new MapMatrix(num_rows, num_cols);			//DELETE this object later???
	
	//loop through columns
	for(int j=0; j<num_cols; j++)
    {
		//find all vertices of the simplex corresponding to this column
		std::vector<int> verts = find_vertices(coface_global[j]);
		
		//if the simplex has a nontrivial boundary, then consider its facets
		if(verts.size() > 1)
		{
			//find all boundary simplices of this simplex
            for(unsigned k=0; k<verts.size(); k++)
			{
				//make a list of all vertices in verts[] except verts[k]
				std::vector<int> facet;
                for(unsigned l=0; l<verts.size(); l++)
					if(l != k)
						facet.push_back(verts[l]);
			
				//look up global index of the boundary simplex
                int gi = find_simplex(facet)->global_index();       //TODO: ought to check that find_simplex() returns a non-NULL value
			
				//look up order index of the simplex
				int oi = face_order.at(gi);
			
				//for this boundary simplex, enter "1" in the appropriate cell in the matrix (row oi, column j)
                (*mat).set(oi,j);
				
			}//end for(k=0;...)
		}//end if(verts.size() > 1)
	}//end for(j=0;...)
	
	//return the MapMatrix
	return mat;
	
}//end get_boundary_mx


//returns the number of unique x-coordinates of the multi-grades
unsigned SimplexTree::num_x_grades()
{
    return x_grades;
}

//returns the number of unique y-coordinates of the multi-grades
unsigned SimplexTree::num_y_grades()
{
    return y_grades;
}

//returns the number of simplices of dimension (hom_dim-1), hom_dim, or (hom_dim+1)
int SimplexTree::get_size(int dim)
{
    if(dim == hom_dim - 1)
        return ordered_low_simplices.size();
    else if(dim == hom_dim)
        return ordered_simplices.size();
    else if(dim == hom_dim + 1)
        return ordered_high_simplices.size();
    else
        return -1;
}


// RECURSIVELY PRINT TREE
void SimplexTree::print()
{
	print_subtree(root, 1);
	
}

void SimplexTree::print_subtree(STNode* node, int indent)
{
	//print current node
	for(int i=0; i<indent; i++)
		std::cout << "  ";
    node->print();
	
	//print children nodes
    std::vector<STNode*> kids = node->get_children();
    for(unsigned i=0; i<kids.size(); i++)
        print_subtree(kids[i], indent+1);
}


//returns the total number of simplices represented in the simplex tree
int SimplexTree::get_num_simplices()	
{
	//start at the root node
    STNode* node = root;
    std::vector<STNode*> kids = node->get_children();
	
	while(kids.size() > 0) //move to the last node in the next level of the tree
	{
		node = kids.back();
        kids = node->get_children();
	}
	
	//we have found the last node in the entire tree, so return its global index +1
    return node->global_index() + 1;
}

