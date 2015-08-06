/**
 * \class	PersistenceUpdater
 * \brief	Computes barcode templates (using the mathematics of "vineyard updates" to store in the Mesh)
 * \author	Matthew L. Wright
 * \date	March 2015
 */

#ifndef __PERSISTENCE_UPDATER_H__
#define __PERSISTENCE_UPDATER_H__

//forward declarations
class ComputationThread;
class Face;
class Halfedge;
class IndexMatrix;
class MapMatrix_Perm;
class MapMatrix_RowPriority_Perm;
class Mesh;
class MultiBetti;
class SimplexTree;
class xiPoint;
class xiMatrixEntry;

#include "xi_support_matrix.h"

#include <map>
#include <vector>


class PersistenceUpdater
{
    public:

        PersistenceUpdater(Mesh* m, SimplexTree* b, std::vector<xiPoint>& xi_pts);  //constructor for when we must compute all of the barcode templates

// MUST BE UPDATED AFTER JULY 2015 BUG FIX:
        PersistenceUpdater(Mesh* m, std::vector<xiPoint>& xi_pts); //constructor for when we load the pre-computed barcode templates from a RIVET data file

//JULY 2015 BUG FIX: the following function is obsolete
//        void find_anchors(); //computes anchors and stores them in mesh->all_anchors; anchor-lines will be created when mesh->build_interior() is called

        //functions to compute and store barcode templates in each 2-cell of the mesh
// MUST BE UPDATED AFTER JULY 2015 BUG FIX:       void store_barcodes(std::vector<Halfedge *> &path);             //standard algorithm with non-lazy swaps
// MUST BE UPDATED AFTER JULY 2015 BUG FIX:       void store_barcodes_lazy(std::vector<Halfedge*>& path);         //uses lazy updates and unsorted "bins" for each row and column
        void store_barcodes_with_reset(std::vector<Halfedge*>& path, ComputationThread* cthread);   //hybrid approach -- for expensive crossings, resets the matrices and does a standard persistence calculation
        void store_barcodes_quicksort(std::vector<Halfedge*>& path);    //hybrid approach -- for expensive crossings, rearranges columns via quicksort and fixes the RU-decomposition globally


    private:
      //data structures

        Mesh* mesh;                 //pointer to the DCEL arrangement in which the barcodes will be stored
        SimplexTree* bifiltration;  //pointer to the bifiltration --- TODO: convert this to a reference
        int dim;                    //dimension of homology to be computed

        xiSupportMatrix xi_matrix;   //sparse matrix to hold xi support points -- used for finding anchors (to build the arrangement) and tracking simplices during the vineyard updates (when computing barcodes to store in the arrangement)

        std::map<unsigned, xiMatrixEntry*> partition_low;   //map from "low" columns to equivalence-class representatives -- implicitly stores the partition of the set of \xi support points
        std::map<unsigned, xiMatrixEntry*> partition_high;  //map from "high" columns to equivalence-class representatives -- implicitly stores the partition of the set of \xi support points
            ///IDEA: maybe the above should be called "block_lift" instead of "partition" since this would be more consistent with the paper

        ///TESTING ONLY
        bool testing;
        MapMatrix_Perm* D_low;
        MapMatrix_Perm* D_high;

      //functions

        typedef std::vector<unsigned> Perm; //for storing permutations

        //stores multigrade info for the persistence computations (data structures prepared with respect to a near-vertical line positioned to the right of all \xi support points)
        //  low is true for simplices of dimension hom_dim, false for simplices of dimension hom_dim+1
        void store_multigrades(IndexMatrix* ind, bool low);

        //finds the proper order of simplexes for the persistence calculation (with respect to a near-vertical line positioned to the right of all \xi support points)
        //  low is true for simplices of dimension hom_dim, false for simplices of dimension hom_dim+1
        //  simplex_order will be filled with a map : dim_index --> order_index for simplices of the given dimension
        void build_simplex_order(IndexMatrix* ind, bool low, std::vector<int>& simplex_order);

        //moves grades associated with xiMatrixEntry greater, that come before xiMatrixEntry lesser in R^2, so that they become associated with lesser
        void split_grade_lists(xiMatrixEntry* greater, xiMatrixEntry* lesser, bool horizontal);

        //moves all grades associated with xiMatrixEntry lesser so that they become associated with xiMatrixEntry greater
        void merge_grade_lists(xiMatrixEntry* greater, xiMatrixEntry* lesser);

        //moves columns from an equivalence class given by xiMatrixEntry* first to their new positions after or among the columns in the equivalence class given by xiMatrixEntry* second
        //  the boolean argument indicates whether an anchor is being crossed from below (or from above)
        //  returns a count of the number of transpositions performed
        unsigned long move_columns(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH, Perm& perm_low, Perm& inv_perm_low, Perm& perm_high, Perm& inv_perm_high);

        //moves a block of n columns, the rightmost of which is column s, to a new position following column t (NOTE: assumes s <= t)
        //  returns a count of the number of transpositions performed
        unsigned long move_low_columns(int s, unsigned n, int t, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH, Perm& perm_low, Perm& inv_perm_low);
        unsigned long move_high_columns(int s, unsigned n, int t, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH, Perm& perm_high, Perm& inv_perm_high);

        //swaps two blocks of columns by updating the total order on columns, then rebuilding the matrices and computing a new RU-decomposition
        void update_order_and_reset_matrices(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm*& UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm*& UH, MapMatrix_Perm* RL_initial, MapMatrix_Perm* RH_initial, Perm& perm_low, Perm& inv_perm_low, Perm& perm_high, Perm& inv_perm_high);

        //swaps two blocks of columns by using a quicksort to update the matrices, then fixing the RU-decomposition (Gaussian elimination on U followed by reduction of R)
        ///TODO: IMPLEMENT THIS!
        void quicksort_and_reduce(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH);

        //removes entries corresponding to xiMatrixEntry head from partition_low and partition_high
        void remove_partition_entries(xiMatrixEntry* head);

        //if the equivalence class corresponding to xiMatrixEntry head has nonempty sets of "low" or "high" simplices, then this function creates the appropriate entries in partition_low and partition_high
        void add_partition_entries(xiMatrixEntry* head);

        //stores a barcode template in a 2-cell of the arrangement
        ///TODO: IMPROVE THIS -- track most recent barcode at the simplicial level and re-examine only the necessary columns!!!
        void store_barcode_template(Face* cell, MapMatrix_Perm* RL, MapMatrix_Perm* RH);

        ///TESTING ONLY
        void check_low_matrix(MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL);
        void check_high_matrix(MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH);

        void print_perms(Perm& per, Perm& inv);
        void print_high_partition();

};

#endif // __PERSISTENCE_UPDATER_H__
