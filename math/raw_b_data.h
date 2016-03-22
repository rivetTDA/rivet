#ifndef RAW_B_DATA_H
#define RAW_B_DATA_H


//forward declarations
class IndexMatrix;
class MapMatrix;
class MapMatrix_Perm;

#include "bd_node.h"

#include <set>
#include <vector>

//struct SimplexData used for return type of RawBData::get_simplex_data()
struct SimplexData
{
    std::vector<Grade>* grade;  //integer (relative) coordinates of multi-grade
    int dim;
};

//struct DirectSumMatrices used for return type of RawBData::get_merge_mxs()
struct DirectSumMatrices
{
    MapMatrix* boundary_matrix;     //points to a boundary matrix for B+C
    MapMatrix* map_matrix;          //points to a matrix for a merge or split map
    IndexMatrix* column_indexes;    //points to a matrix of column indexes, one for each multi-grade
};

//comparison functor for sorting std::set<BDNode*> by REVERSE-LEXICOGRAPHIC multi-grade order based on first grade of appearance (also in REVERSE-LEXICOGRAPHIC order)
struct NodeComparator
{
    bool operator()(const BDNode* left, const BDNode* right) const
    {
        //WILL BREAK ON EMPTY NODES
        return left->grades()[0] < right->grades()[0];
    }
};

//typedef
typedef std::multiset<BDNode*, NodeComparator> SimplexSet;

//now the RawBData class
class RawBData {
    public:
        RawBData(int dim, int v);	//constructor; requires verbosity parameter

        ~RawBData(); //destructor

        void build_BR_complex(std::vector<unsigned>& times, std::vector<unsigned>& distances, unsigned num_x, unsigned num_y);
                    //builds SimplexTree representing a bifiltered Vietoris-Rips complex from discrete data
                    //requires a list of birth times (one for each point), a list of distances between pairs of points, max dimension of simplices to construct, and number of grade values in x- and y-directions
                    //NOTE: automatically computes global indexes and dimension indexes
                    //CONVENTION: the x-coordinate is "birth time" for points and the y-coordinate is "distance" between points

        void add_simplex(std::vector<int>& vertices, std::vector<Grade>& y);	//adds a simplex (and its faces) to the SimplexTree; multi-grade is vector of grades; WARNING: doesn't update global data structures (e.g. global indexes)

        void update_xy_indexes(std::vector<std::vector<Grade> >& grades_ind, unsigned num_x, unsigned num_y);
                    //updates multigrades; for use when building simplexTree from a bifiltration file
                    //also requires the number of x- and y-grades that exist in the bifiltration

        void update_global_indexes();			//updates the global indexes of all simplices in this simplex tree
        void update_dim_indexes();              //updates the dimension indexes (reverse-lexicographical multi-grade order) for simplices of dimension (hom_dim-1), hom_dim, and (hom_dim+1)

        MapMatrix* get_boundary_mx(int dim);    //returns a matrix of boundary information for simplices
        MapMatrix_Perm* get_boundary_mx(std::vector<int>& coface_order, unsigned num_simplices);    //returns a boundary matrix for hom_dim-simplices with columns in a specified order -- for vineyard-update algorithm
        MapMatrix_Perm* get_boundary_mx(std::vector<int>& face_order, unsigned num_faces, std::vector<int>& coface_order, unsigned num_cofaces);    //returns a boundary matrix for (hom_dim+1)-simplices with columns and rows a specified orders -- for vineyard-update algorithm

        DirectSumMatrices get_merge_mxs();      //returns matrices for the merge map [B+C,D], the boundary map B+C, and the multi-grade information
        DirectSumMatrices get_split_mxs();      //returns matrices for the split map [A,B+C], the boundary map B+C, and the multi-grade information

        IndexMatrix* get_index_mx(int dim);     //returns a matrix of column indexes to accompany MapMatrices
        IndexMatrix* get_offset_index_mx(int dim);  //returns a matrix of column indexes offset in each direction, for the boundary_A matrix in compute_eta()

        std::vector<int> find_vertices(int gi);	//given a global index, return (a vector containing) the vertices of the simplex
        BDNode* find_simplex(std::vector<int>& vertices);   //given a sorted vector of vertex indexes, return a pointer to the node representing the corresponding simplex

        unsigned num_x_grades();                     //returns the number of unique x-coordinates of the multi-grades
        unsigned num_y_grades();                     //returns the number of unique y-coordinates of the multi-grades

        int get_size(int dim);                  //returns the number of simplices of dimension (hom_dim-1), hom_dim, or (hom_dim+1)

        ///// THESE FUNCTIONS ARE UNUSED AND MIGHT NEED TO BE UPDATED
            SimplexData get_simplex_data(int index);	//returns the multi-grade of the simplex with given global simplex index, as well as the dimension of the simplex
            int get_num_simplices();		//returns the total number of simplices represented in the simplex tree
                //TODO: would it be more efficient to store the total number of simplices???


        const int hom_dim;      //the dimension of homology to be computed; max dimension of simplices is one more than this
        const int verbosity;	//controls display of output, for debugging

        //TESTING
        void print();
        void print_subtree(BDNode *node, int indent);

    private:
        BDNode* root;		//root node of the simplex tree

        unsigned x_grades;  //the number of x-grades that exist in this bifiltration
        unsigned y_grades;  //the number of y-grades that exist in this bifiltration

        SimplexSet ordered_high_simplices;   //pointers to simplices of dimension (hom_dim + 1) in reverse-lexicographical multi-grade order
        SimplexSet ordered_simplices;        //pointers to simplices of dimension hom_dim in reverse-lexicographical multi-grade order
        SimplexSet ordered_low_simplices;    //pointers to simplices of dimension (hom_dim - 1) in reverse-lexicographical multi-grade order

        void build_BR_subtree(std::vector<unsigned>& times, std::vector<unsigned>& distances, BDNode &parent, std::vector<unsigned> &parent_indexes, unsigned prev_time, unsigned prev_dist, unsigned cur_dim, unsigned& gic);	//recursive function used in build_VR_complex()

        void add_faces(BDNode* node, std::vector<int>& vertices, std::vector<Grade>& grades);	//recursively adds faces of a simplex to the SimplexTree; WARNING: doesn't update global data structures (e.g. global indexes)

        void update_xy_indexes_recursively(BDNode* node, std::vector<std::vector<Grade> >& grades_ind);   //updates multigrades recursively

        void update_gi_recursively(BDNode* node, int &gic); 		//recursively update global indexes of simplices

        void build_dim_lists_recursively(BDNode* node, int cur_dim, int hom_dim);        //recursively build lists to determine dimension indexes

        void find_nodes(BDNode &node, int level, std::vector<int> &vec, int time, int dist, int dim);	//recursively search tree for simplices of specified dimension that exist at specified multi-index

        void find_vertices_recursively(std::vector<int> &vertices, BDNode* node, int key);	//recursively search for a global index and keep track of vertices

        void write_boundary_column(MapMatrix* mat, BDNode* sim, int col, int offset);   //writes boundary information for simplex represented by sim in column col of matrix mat; offset allows for block matrices such as B+C
};


#endif // RAW_B_DATA_H
