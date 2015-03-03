/**
 * \class	SimplexTree
 * \brief	Stores a bifiltered simplicial complex in a simplex tree structure.
 * \author	Matthew L. Wright
 * \date	February 2014
 * 
 * The SimplexTree class stores a bifiltered simplicial complex in a simplex tree structure.
 * Each node in the simplex tree (implemented by the STNode class) represents one simplex in the bifiltration.
 * Each simplex has a multi-index at which it is born.
 * Implementation is based on a 2012 paper by Boissonnat and Maria.
 */


#ifndef __SimplexTree_H__
#define __SimplexTree_H__

#include <utility>	// std::pair
#include <map>
#include <set>
#include <stdexcept>

#include "st_node.h"
#include "map_matrix.h"
#include "index_matrix.h"


//struct SimplexData used for return type of SimplexTree::get_simplex_data()
struct SimplexData
{
    int x;  //integer (relative) x-coordinate of multi-grade
    int y;  //integer (relative) x-coordinate of multi-grade
    int dim;
};

//struct DirectSumMatrices used for return type of SimplexTree::get_merge_mxs()
struct DirectSumMatrices
{
    MapMatrix* boundary_matrix;     //points to a boundary matrix for B+C
    MapMatrix* map_matrix;          //points to a matrix for a merge or split map
    IndexMatrix* column_indexes;    //points to a matrix of column indexes, one for each multi-grade
};

//comparison functor for sorting std::set<STNode*> by REVERSE-LEXICOGRAPHIC multi-grade order
struct NodeComparator
{
    bool operator()(const STNode* left, const STNode* right) const
    {
        if(left->grade_y() < right->grade_y())
            return true;
        if(left->grade_y() == right->grade_y() && left->grade_x() < right->grade_x())
            return true;
        return false;
    }
};

//typedef
typedef std::multiset<STNode*, NodeComparator> SimplexSet;

//now the SimplexTree class
class SimplexTree {
	public:
        SimplexTree(int dim, int v);	//constructor; requires verbosity parameter

        ~SimplexTree(); //destructor
		
        void build_VR_complex(std::vector<unsigned>& times, std::vector<unsigned>& distances, unsigned num_x, unsigned num_y);
                    //builds SimplexTree representing a bifiltered Vietoris-Rips complex from discrete data
                    //requires a list of birth times (one for each point), a list of distances between pairs of points, max dimension of simplices to construct, and number of grade values in x- and y-directions
                    //NOTE: automatically computes global indexes and dimension indexes
                    //CONVENTION: the x-coordinate is "birth time" for points and the y-coordinate is "distance" between points

 //UPDATE THIS:       void add_simplex(std::vector<int> & vertices, int x, int y);	//adds a simplex (and its faces) to the SimplexTree; multi-grade is (x,y)
		
        void update_global_indexes();			//updates the global indexes of all simplices in this simplex tree
        void update_dim_indexes();              //updates the dimension indexes (reverse-lexicographical multi-grade order) for simplices of dimension (hom_dim-1), hom_dim, and (hom_dim+1)

        MapMatrix* get_boundary_mx(int dim);    //returns a matrix of boundary information for simplices
        DirectSumMatrices get_merge_mxs();      //returns matrices for the merge map [B+C,D], the boundary map B+C, and the multi-grade information
        DirectSumMatrices get_split_mxs();      //returns matrices for the split map [A,B+C], the boundary map B+C, and the multi-grade information
        IndexMatrix* get_index_mx(int dim);     //returns a matrix of column indexes to accompany MapMatrices
        IndexMatrix* get_offset_index_mx(int dim);  //returns a matrix of column indexes offset in each direction, for the boundary_A matrix in compute_eta()

        MapMatrix* get_boundary_mx(int dim, std::vector<unsigned>& block_counts, std::vector<int>& block_indexes);  //returns a boundary matrix with respect to a total order on simplices, as necessary for the "vineyard-update" algorithm


        std::vector<int> find_vertices(int gi);	//given a global index, return (a vector containing) the vertices of the simplex
        STNode* find_simplex(std::vector<int>& vertices);   //given a sorted vector of vertex indexes, return a pointer to the node representing the corresponding simplex

        unsigned num_x_grades();                     //returns the number of unique x-coordinates of the multi-grades
        unsigned num_y_grades();                     //returns the number of unique y-coordinates of the multi-grades

        int get_size(int dim);                  //returns the number of simplices of dimension (hom_dim-1), hom_dim, or (hom_dim+1)

        ///// THESE FUNCTIONS MIGHT NEED TO BE UPDATED
            MapMatrix* get_boundary_mx(std::vector<int> coface_global, std::map<int,int> face_order);
                //computes a boundary matrix, using given orders on simplices of dimensions d (cofaces) and d-1 (faces)
                //used in persistence_data.cpp

            SimplexData get_simplex_data(int index);	//returns the multi-grade of the simplex with given global simplex index, as well as the dimension of the simplex

            int get_num_simplices();		//returns the total number of simplices represented in the simplex tree
                //TODO: would it be more efficient to store the total number of simplices???
		
        ///// FUNCTIONS FOR TESTING
            void print();				//prints a representation of the simplex tree

		
	private:
        STNode* root;		//root node of the simplex tree

        unsigned x_grades;  //the number of x-grades that exist in this bifiltration
        unsigned y_grades;  //the number of y-grades that exist in this bifiltration

        SimplexSet ordered_high_simplices;   //pointers to simplices of dimension (hom_dim + 1) in reverse-lexicographical multi-grade order
        SimplexSet ordered_simplices;        //pointers to simplices of dimension hom_dim in reverse-lexicographical multi-grade order
        SimplexSet ordered_low_simplices;    //pointers to simplices of dimension (hom_dim - 1) in reverse-lexicographical multi-grade order

        const int hom_dim;  //the dimension of homology to be computed; max dimension of simplices is one more than this
		
		const int verbosity;	//controls display of output, for debugging
		
        void build_VR_subtree(std::vector<unsigned>& times, std::vector<unsigned>& distances, STNode &parent, std::vector<unsigned> &parent_indexes, unsigned prev_time, unsigned prev_dist, unsigned cur_dim, unsigned& gic);	//recursive function used in build_VR_complex()

        void add_faces(std::vector<int> & vertices, int x, int y);	//recursively adds faces of a simplex to the SimplexTree; WARNING: doesn't update global data structures (time_list, dist_list, or global indexes), so should only be called from add_simplex()
		
        void update_gi_recursively(STNode* node, int &gic); 		//recursively update global indexes of simplices
		
        void build_dim_lists_recursively(STNode* node, int cur_dim, int hom_dim);        //recursively build lists to determine dimension indexes

		void find_nodes(STNode &node, int level, std::vector<int> &vec, int time, int dist, int dim);	//recursively search tree for simplices of specified dimension that exist at specified multi-index
		
        void find_vertices_recursively(std::vector<int> &vertices, STNode* node, int key);	//recursively search for a global index and keep track of vertices

        void write_boundary_column(MapMatrix* mat, STNode* sim, int col, int offset);   //writes boundary information for simplex represented by sim in column col of matrix mat; offset allows for block matrices such as B+C

        void print_subtree(STNode *, int);	//recursive function that prints the simplex tree
};

#endif // __SimplexTree_H__
