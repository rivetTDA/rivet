/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "simplex_tree.h"

#include "index_matrix.h"
#include "map_matrix.h"
#include "st_node.h"

#include "debug.h"

#include <algorithm>
#include <iostream> //for std::cout, for testing only
#include <limits> //std::numeric_limits
#include <sstream>
#include <stdexcept>

//SimplexTree constructor; requires dimension of homology to be computed and verbosity parameter
SimplexTree::SimplexTree(int dim, int v)
    : hom_dim(dim)
    , verbosity(v)
    , root(new STNode())
    , x_grades(0)
    , y_grades(0)
{
    if (hom_dim > 5) {
        throw std::runtime_error("SimplexTree: Dimensions greater than 5 probably don't make sense");
    }
    if (verbosity >= 8) {
        debug() << "Created SimplexTree(" << hom_dim << ", " << verbosity << ")";
    }
}

//destructor
SimplexTree::~SimplexTree()
{
    delete root;
}

//adds a simplex (including all of its faces) to the SimplexTree
//if simplex or any of its faces already exist, they are not re-added
//WARNING: doesn't update global data structures (e.g. dimension indexes or global indexes)
//also, could be made more efficient
//also, we don't need to add simplexes of dimension greater than hom_dim+1
void SimplexTree::add_simplex(std::vector<int>& vertices, int x, int y)
{
    //make sure vertices are sorted
    std::sort(vertices.begin(), vertices.end());

    //add the simplex and all of its faces
    add_faces(root, vertices, x, y);
} //end add_simplex()

//recursively adds faces of a simplex to the SimplexTree
void SimplexTree::add_faces(STNode* node, std::vector<int>& vertices, int x, int y)
{
    //ensure that the vertices are children of the node
    for (unsigned i = 0; i < vertices.size(); i++) {
        STNode* child = node->add_child(vertices[i], x, y); //if a child with specified vertex index already exists, then nothing is added, but that child is returned

        //form vector consisting of all the vertices except for vertices[i]
        std::vector<int> face;
        for (unsigned k = i + 1; k < vertices.size(); k++)
            face.push_back(vertices[k]);

        //add the face simplex to the SimplexTree
        add_faces(child, face, x, y);
    }
} //end add_faces()

//updates multigrades; for use when building simplexTree from a bifiltration file
void SimplexTree::update_xy_indexes(std::vector<unsigned>& x_ind, std::vector<unsigned>& y_ind, unsigned num_x, unsigned num_y)
{
    //store the number of grades
    x_grades = num_x;
    y_grades = num_y;

    //now update the indexes
    update_xy_indexes_recursively(root, x_ind, y_ind);
} //end update_xy_indexes();

//updates multigrades recursively
void SimplexTree::update_xy_indexes_recursively(STNode* node, std::vector<unsigned>& x_ind, std::vector<unsigned>& y_ind)
{
    //loop through children of current node
    std::vector<STNode*> kids = node->get_children();
    for (unsigned i = 0; i < kids.size(); i++) {
        //update multigrade of this child
        STNode* cur = kids[i];
        cur->set_x(x_ind[cur->grade_x()]);
        cur->set_y(y_ind[cur->grade_y()]);

        //move on to its children
        update_xy_indexes_recursively(cur, x_ind, y_ind);
    }
} //end update_xy_indexes_recursively();

//updates the global indexes of all simplices in this simplex tree
void SimplexTree::update_global_indexes()
{
    int gic = 0; //global index counter
    update_gi_recursively(root, gic);
}

//recursively update global indexes of simplices
void SimplexTree::update_gi_recursively(STNode* node, int& gic)
{
    //loop through children of current node
    std::vector<STNode*> kids = node->get_children();
    for (unsigned i = 0; i < kids.size(); i++) {
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
    build_dim_lists_recursively(root, 0);

    //update the dimension indexes in the tree
    int i = 0;
    for (SimplexSet::iterator it = ordered_low_simplices.begin(); it != ordered_low_simplices.end(); ++it)
        (*it)->set_dim_index(i++);

    i = 0;
    for (SimplexSet::iterator it = ordered_simplices.begin(); it != ordered_simplices.end(); ++it)
        (*it)->set_dim_index(i++);

    i = 0;
    for (SimplexSet::iterator it = ordered_high_simplices.begin(); it != ordered_high_simplices.end(); ++it)
        (*it)->set_dim_index(i++);
}

//recursively build lists to determine dimension indexes
void SimplexTree::build_dim_lists_recursively(STNode* node, unsigned cur_dim)
{
    //get children of current node
    std::vector<STNode*> kids = node->get_children();

    //check dimensions and add children to appropriate list
    if (cur_dim == hom_dim - 1)
        ordered_low_simplices.insert(kids.begin(), kids.end());
    else if (cur_dim == hom_dim)
        ordered_simplices.insert(kids.begin(), kids.end());
    else if (cur_dim == hom_dim + 1)
        ordered_high_simplices.insert(kids.begin(), kids.end());

    //recurse through children
    for (unsigned i = 0; i < kids.size(); i++) {
        build_dim_lists_recursively(kids[i], cur_dim + 1);
    }
}

//builds SimplexTree representing a bifiltered Vietoris-Rips complex from discrete data
//requires a list of birth times (one for each point) and a list of distances between pairs of points
//NOTE: automatically computes global indexes and dimension indexes
//CONVENTION: the x-coordinate is "birth time" for points and the y-coordinate is "distance" between points
void SimplexTree::build_VR_complex(std::vector<unsigned>& times,
    std::vector<unsigned>& distances,
    unsigned num_x,
    unsigned num_y)
{
    x_grades = num_x;
    y_grades = num_y;

    //build simplex tree recursively
    //this also assigns global indexes to each simplex
    unsigned gic = 0; //global index counter
    for (unsigned i = 0; i < times.size(); i++) {
        //create the node and add it as a child of root
        STNode* node = new STNode(i, root, times[i], 0, gic); //delete later!
        root->append_child(node);
        gic++; //increment the global index counter

        //recursion
        std::vector<unsigned> parent_indexes; //knowledge of ALL parent nodes is necessary for computing distance index of each simplex
        parent_indexes.push_back(i);

        build_VR_subtree(times, distances, *node, parent_indexes, times[i], 0, 1, gic);
    }

    //compute dimension indexes
    update_dim_indexes();
} //end build_VR_complex()

//function to build (recursively) a subtree of the simplex tree
void SimplexTree::build_VR_subtree(std::vector<unsigned>& times,
    std::vector<unsigned>& distances,
    STNode& parent,
    std::vector<unsigned>& parent_indexes,
    unsigned prev_time,
    unsigned prev_dist,
    unsigned cur_dim,
    unsigned& gic)
{
    //loop through all points that could be children of this node
    for (unsigned j = parent_indexes.back() + 1; j < times.size(); j++) {
        //look up distances from point j to each of its parents
        //distance index is maximum of prev_distance and each of these distances
        unsigned current_dist = prev_dist;
        for (unsigned k = 0; k < parent_indexes.size(); k++) {
            unsigned p = parent_indexes[k];
            unsigned d = distances[(j * (j - 1)) / 2 + p]; //the distance between points p and j, with p < j
            if (d > current_dist)
                current_dist = d;
        }

        //see if the distance is permitted
        if (current_dist < std::numeric_limits<unsigned>::max()) //then we will add another node to the simplex tree
        {
            //compute time index of this new node
            unsigned current_time = times[j];
            if (current_time < prev_time)
                current_time = prev_time;

            //create the node and add it as a child of its parent
            STNode* node = new STNode(j, &parent, current_time, current_dist, gic); //delete THIS OBJECT LATER!
            parent.append_child(node);
            gic++; //increment the global index counter

            //recursion
            if (cur_dim <= hom_dim) //then consider simplices of the next dimension
            {
                parent_indexes.push_back(j); //next we will look for children of node j
                build_VR_subtree(times, distances, *node, parent_indexes, current_time, current_dist, cur_dim + 1, gic);
                parent_indexes.pop_back(); //finished adding children of node j
            }
        }
    }
} //end build_subtree()

//returns a matrix of boundary information for simplices of the given dimension (with multi-grade info)
//columns ordered according to dimension index (reverse-lexicographic order with respect to multi-grades)
MapMatrix* SimplexTree::get_boundary_mx(unsigned dim)
{
    //select set of simplices of dimension dim
    SimplexSet* simplices;
    size_t num_rows;

    if (dim == hom_dim) {
        simplices = &ordered_simplices;
        num_rows = ordered_low_simplices.size();
    } else if (dim == hom_dim + 1) {
        simplices = &ordered_high_simplices;
        num_rows = ordered_simplices.size();
    } else {
        std::stringstream ss;
        ss << "SimplexTree::get_boundary_mx(): Attempting to compute boundary matrix for improper dimension (" << dim << "), expected either "
           << hom_dim << " or " << hom_dim + 1;
        throw std::runtime_error(ss.str());
    }

    //create the MapMatrix
    MapMatrix* mat = new MapMatrix(num_rows, simplices->size()); //DELETE this object later!

    //if we want a matrix for 0-simplices, then we are done
    if (dim == 0)
        return mat;

    //loop through simplices, writing columns to the matrix
    int col = 0; //column counter
    for (SimplexSet::iterator it = simplices->begin(); it != simplices->end(); ++it) {
        write_boundary_column(mat, *it, col, 0);

        col++;
    }

    //return the matrix
    return mat;
} //end get_boundary_mx(int)

//returns a boundary matrix for hom_dim-simplices with columns in a specified order -- for vineyard-update algorithm
//    simplex_order is a map : dim_index --> order_index for simplices of the given dimension
//        if simplex_order[i] == -1, then simplex with dim_index i is NOT represented in the boundary matrix
//    num_simplices is the number of simplices in the order (i.e., the number of entries in the vector that are NOT -1)
MapMatrix_Perm* SimplexTree::get_boundary_mx(std::vector<int>& coface_order, unsigned num_simplices)
{
    //create the matrix
    MapMatrix_Perm* mat = new MapMatrix_Perm(ordered_low_simplices.size(), num_simplices);

    //loop through all simplices, writing columns to the matrix
    int dim_index = 0; //tracks our position in the list of hom_dim-simplices
    for (SimplexSet::iterator it = ordered_simplices.begin(); it != ordered_simplices.end(); ++it) {
        int order_index = coface_order[dim_index]; //index of the matrix column which will store the boundary of this simplex
        if (order_index != -1) {
            write_boundary_column(mat, *it, order_index, 0);
        }

        dim_index++; //increment the column counter
    }

    //return the matrix
    return mat;
} //end get_boundary_mx(int, vector<int>)

//returns a boundary matrix for (hom_dim+1)-simplices with columns and rows a specified orders -- for vineyard-update algorithm
//  PARAMETERS:
//    each vector is a map : dim_index --> order_index for simplices of the given dimension
//        if order[i] == -1, then simplex with dim_index i is NOT represented in the boundary matrix
//    each unsigned is the number of simplices in the corresponding order (i.e., the number of entries in the vector that are NOT -1)
MapMatrix_Perm* SimplexTree::get_boundary_mx(std::vector<int>& face_order, unsigned num_faces, std::vector<int>& coface_order, unsigned num_cofaces)
{
    //create the matrix
    MapMatrix_Perm* mat = new MapMatrix_Perm(num_faces, num_cofaces);

    //loop through all simplices, writing columns to the matrix
    int dim_index = 0; //tracks our position in the list of (hom_dim+1)-simplices
    for (SimplexSet::iterator it = ordered_high_simplices.begin(); it != ordered_high_simplices.end(); ++it) {
        int order_index = coface_order[dim_index]; //index of the matrix column which will store the boundary of this simplex
        if (order_index != -1) {
            //get vertex list for this simplex
            std::vector<int> verts = find_vertices((*it)->global_index());

            //find all facets of this simplex
            for (unsigned k = 0; k < verts.size(); k++) ///TODO: optimize! make this faster!
            {
                //facet vertices are all vertices in verts[] except verts[k]
                std::vector<int> facet;
                for (unsigned l = 0; l < verts.size(); l++)
                    if (l != k)
                        facet.push_back(verts[l]);

                //look up order index of the facet
                STNode* facet_node = find_simplex(facet);
                if (facet_node == NULL)
                    throw std::runtime_error("SimplexTree::get_boundary_mx(): Facet simplex not found.");
                int facet_order_index = face_order[facet_node->dim_index()];

                //for this boundary simplex, enter "1" in the appropriate cell in the matrix
                mat->set(facet_order_index, order_index);
            }
        }
        dim_index++; //increment the column counter
    }

    //return the matrix
    return mat;
} //end get_boundary_mx(int, vector<int>, vector<int>)

//writes boundary information for simplex represented by sim in column col of matrix mat; offset allows for block matrices such as B+C
void SimplexTree::write_boundary_column(MapMatrix* mat, STNode* sim, int col, int offset)
{
    //get vertex list for this simplex
    std::vector<int> verts = find_vertices(sim->global_index());

    //for a 0-simplex, there is nothing to do
    if (verts.size() == 1)
        return;

    //find all facets of this simplex
    for (unsigned k = 0; k < verts.size(); k++) {
        //facet vertices are all vertices in verts[] except verts[k]
        std::vector<int> facet;
        for (unsigned l = 0; l < verts.size(); l++)
            if (l != k)
                facet.push_back(verts[l]);

        //look up dimension index of the facet
        STNode* facet_node = find_simplex(facet);
        if (facet_node == NULL) {
            std::stringstream ss;
            for (unsigned i = 0; i < facet.size(); i++) {
                if (i != 0)
                    ss << ",";
                ss << facet[i];
            }

            throw std::runtime_error("SimplexTree::write_boundary_column(): Facet simplex not found: " + ss.str());
        }
        int facet_di = facet_node->dim_index();

        //for this boundary simplex, enter "1" in the appropriate cell in the matrix
        mat->set(facet_di + offset, col);
    }
} //end write_col();

//returns a matrix of column indexes to accompany MapMatrices
//  entry (i,j) gives the last column of the MapMatrix that corresponds to multigrade (i,j)
IndexMatrix* SimplexTree::get_index_mx(unsigned dim)
{
    //select set of simplices of dimension dim
    SimplexSet* simplices;

    if (dim == hom_dim)
        simplices = &ordered_simplices;
    else if (dim == hom_dim + 1)
        simplices = &ordered_high_simplices;
    else
        throw std::runtime_error("SimplexTree::get_index_mx(): Attempting to compute index matrix for improper dimension.");

    //create the IndexMatrix
    unsigned x_size = x_grades;
    unsigned y_size = y_grades;
    IndexMatrix* mat = new IndexMatrix(y_size, x_size); //DELETE this object later!

    if (!simplices->empty()) //then there is at least one simplex
    {
        //initialize to -1 the entries of end_col matrix before multigrade of the first simplex
        unsigned cur_entry = 0; //tracks previously updated multigrade in end-cols

        SimplexSet::iterator it = simplices->begin();
        for (; cur_entry < (*it)->grade_x() + (*it)->grade_y() * x_size; cur_entry++)
            mat->set(cur_entry / x_size, cur_entry % x_size, -1);

        //loop through simplices
        int col = 0; //column counter
        for (; it != simplices->end(); ++it) {
            //get simplex
            STNode* simplex = *it;
            auto cur_x = simplex->grade_x();
            auto cur_y = simplex->grade_y();

            //if some multigrades were skipped, store previous column number in skipped cells of end_col matrix
            for (; cur_entry < cur_x + cur_y * x_size; cur_entry++)
                mat->set(cur_entry / x_size, cur_entry % x_size, col - 1);

            //store current column number in cell of end_col matrix for this multigrade
            mat->set(cur_y, cur_x, col);

            //increment column counter
            col++;
        }

        //store final column number for any cells in end_cols not yet updated
        for (; cur_entry < x_size * y_size; cur_entry++)
            mat->set(cur_entry / x_size, cur_entry % x_size, col - 1);
    } else //then there are no simplices, so fill the IndexMatrix with -1 values
    {
        for (unsigned i = 0; i < x_size * y_size; i++)
            mat->set(i / x_size, i % x_size, -1);
    }

    //return the matrix
    return mat;
} //end get_index_mx()

////recursively search tree for simplices of specified dimension that exist at specified multi-index
//void SimplexTree::find_nodes(STNode& node, int level, std::vector<int>& vec, unsigned time, unsigned dist, unsigned dim)
//{
//
//    //consider current node
//    if ((level == dim + 1) && (node.grade_x() <= time) && (node.grade_y() <= dist)) {
//        vec.push_back(node.global_index());
//    }
//
//    //move on to children nodes
//    if (level <= dim) {
//        std::vector<STNode*> kids = node.get_children();
//        for (unsigned i = 0; i < kids.size(); i++)
//            find_nodes(*kids[i], level + 1, vec, time, dist, dim);
//    }
//}

//given a global index, return (a vector containing) the vertices of the simplex
std::vector<int> SimplexTree::find_vertices(int gi)
{
    std::vector<int> vertices;
    find_vertices_recursively(vertices, root, gi);
    return vertices;
}

//recursively search for a global index and keep track of vertices
void SimplexTree::find_vertices_recursively(std::vector<int>& vertices, STNode* node, int key)
{
    //search children of current node for greatest index less than or equal to key
    std::vector<STNode*>& kids = node->get_children();

    int min = 0;
    int max = kids.size() - 1;
    while (max >= min) {
        int mid = (max + min) / 2;
        if ((*kids[mid]).global_index() == key) //key found at this level
        {
            vertices.push_back((*kids[mid]).get_vertex());
            return;
        } else if ((*kids[mid]).global_index() <= key)
            min = mid + 1;
        else
            max = mid - 1;
    }

    //if we get here, then key not found
    //so we want greatest index less than key, which is found at kids[max]
    vertices.push_back((*kids[max]).get_vertex());
    //now search children of kids[max]
    find_vertices_recursively(vertices, kids[max], key);
}

//given a sorted vector of vertex indexes (labels), return a pointer to the node representing the corresponding simplex
STNode* SimplexTree::find_simplex(std::vector<int>& vertices)
{
    size_t size = vertices.size();
    if (size == 0)
        return root; //root is associated with the null simpex

    //search the vector of children nodes for each vertex
    STNode* node = root;
    for (unsigned i = 0; i < size && nullptr != node; i++) {
        std::vector<STNode*>& kids = node->get_children();
        int key = vertices[i];
        node = *(std::find_if(kids.begin(),
            kids.end(),
            [key](STNode* kid) { return kid->get_vertex() == key; }));
    }

    //return global index
    return node;
}

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
unsigned SimplexTree::get_size(unsigned dim)
{
    if (dim == hom_dim - 1)
        return ordered_low_simplices.size();
    else if (dim == hom_dim)
        return ordered_simplices.size();
    else if (dim == hom_dim + 1)
        return ordered_high_simplices.size();
    else
        throw std::runtime_error("SimplexTree::get_size(): invalid dimension");
}

//returns the total number of simplices represented in the simplex tree
int SimplexTree::get_num_simplices()
{
    //start at the root node
    STNode* node = root;
    std::vector<STNode*> kids = node->get_children();

    while (kids.size() > 0) //move to the last node in the next level of the tree
    {
        node = kids.back();
        kids = node->get_children();
    }

    //we have found the last node in the entire tree, so return its global index +1
    return node->global_index() + 1;
}

// TESTING -- RECURSIVELY PRINT TREE
void SimplexTree::print()
{
    print_subtree(root, 1);
}

void SimplexTree::print_subtree(STNode* node, int indent)
{
    //print current node
    for (int i = 0; i < indent; i++)
        std::cout << "  ";
    node->print();

    //print children nodes
    std::vector<STNode*> kids = node->get_children();
    for (size_t i = 0; i < kids.size(); i++)
        print_subtree(kids[i], indent + 1);
}

//print bifiltration in the RIVET bifiltration input format
//prints simplices in order of increasing dimension
void SimplexTree::print_bifiltration()
{
    std::vector<STNode*> kids = root->get_children();
    for (unsigned d = 0; d <= hom_dim + 1; d++) {
        //print simplices of dimension d
        for (unsigned i = 0; i < kids.size(); i++) {
            print_bifiltration(kids[i], std::string(), 0, static_cast<int>(d));
        }
    }
}

void SimplexTree::print_bifiltration(STNode* node, std::string parent, int cur_dim, int print_dim)
{
    if (cur_dim == print_dim) { //print current node
        if (cur_dim != 0) {
            std::cout << parent << " ";
        }
        std::cout << node->get_vertex() << " " << node->grade_x() << " " << node->grade_y() << std::endl;
    } else { //recurse on children nodes
        std::vector<STNode*> kids = node->get_children();
        for (unsigned i = 0; i < kids.size(); i++) {
            std::string simplex = parent;
            if (cur_dim != 0) {
                simplex += " ";
            }
            simplex += std::to_string(node->get_vertex());
            print_bifiltration(kids[i], simplex, cur_dim + 1, print_dim);
        }
    }
}
