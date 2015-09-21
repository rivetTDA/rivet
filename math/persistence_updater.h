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

        PersistenceUpdater(Mesh* m, std::vector<xiPoint>& xi_pts); //constructor for when we load the pre-computed barcode templates from a RIVET data file

        //functions to compute and store barcode templates in each 2-cell of the mesh
        void store_barcodes_with_reset(std::vector<Halfedge*>& path, ComputationThread* cthread);   //hybrid approach -- for expensive crossings, resets the matrices and does a standard persistence calculation
        void store_barcodes_quicksort(std::vector<Halfedge*>& path);    ///TODO -- for expensive crossings, rearranges columns via quicksort and fixes the RU-decomposition globally

        //function to set the "edge weights" for each anchor line
        void set_anchor_weights(std::vector<Halfedge*>& path);

        //function to clear the levelset lists -- e.g., following the edge-weight calculation
        void clear_levelsets();

    private:
      //data structures

        Mesh* mesh;                 //pointer to the DCEL arrangement in which the barcodes will be stored
        SimplexTree* bifiltration;  //pointer to the bifiltration --- TODO: convert this to a reference
        int dim;                    //dimension of homology to be computed

        xiSupportMatrix xi_matrix;   //sparse matrix to hold xi support points -- used for finding anchors (to build the arrangement) and tracking simplices during the vineyard updates (when computing barcodes to store in the arrangement)

        std::map<unsigned, xiMatrixEntry*> lift_low;   //map from "low" columns to xiMatrixEntrys
        std::map<unsigned, xiMatrixEntry*> lift_high;  //map from "high" columns to xiMatrixEntrys

        MapMatrix_Perm* R_low;               //boundary matrix for "low" simplices
        MapMatrix_Perm* R_high;              //boundary matrix for "high" simplices
        MapMatrix_RowPriority_Perm* U_low;   //upper-trianglular matrix that records the reductions for R_low
        MapMatrix_RowPriority_Perm* U_high;  //upper-trianglular matrix that records the reductions for R_high

        ///TODO: is there a way to avoid maintaining the following permutation vectors?
        std::vector<unsigned> perm_low;      //map from column index at initial cell to column index at current cell
        std::vector<unsigned> inv_perm_low;  //inverse of the previous map
        std::vector<unsigned> perm_high;     //map from column index at initial cell to column index at current cell
        std::vector<unsigned> inv_perm_high; //inverse of the previous map

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
        //  NOTE: If a simplex with dim_index i does not appear in the order (i.e. its grade is not less than the LUB of all xi support points), then simplex_order[i] = -1.
        //  returns the number of simplices in the order
        unsigned build_simplex_order(IndexMatrix* ind, bool low, std::vector<int>& simplex_order);

        //counts the number of transpositions that will happen if we cross an anchor and do vineyeard-updates
        unsigned long count_transpositions(xiMatrixEntry* at_anchor, bool from_below);

        //counts the number of transpositions that result from separations; used in the above function
        void count_transpositions_from_separations(xiMatrixEntry* greater, xiMatrixEntry* lesser, bool horiz, bool low, unsigned long& count_trans, unsigned& count_lesser);

        //moves grades associated with xiMatrixEntry greater, that come before xiMatrixEntry lesser in R^2, so that they become associated with lesser
        //   horiz is true iff greater and lesser are on the same horizontal line (i.e., they share the same y-coordinate)
        //   returns a count of the number transpositions performed
        unsigned long split_grade_lists(xiMatrixEntry* greater, xiMatrixEntry* lesser, bool horiz);

        //moves all grades associated with xiMatrixEntry lesser so that they become associated with xiMatrixEntry greater
        void merge_grade_lists(xiMatrixEntry* greater, xiMatrixEntry* lesser);

        //moves columns from an equivalence class given by xiMatrixEntry* first to their new positions after or among the columns in the equivalence class given by xiMatrixEntry* second
        //  the boolean argument indicates whether an anchor is being crossed from below (or from above)
        //  returns a count of the number of transpositions performed
        unsigned long move_columns(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below);

        //moves a block of n columns, the rightmost of which is column s, to a new position following column t (NOTE: assumes s <= t)
        //  returns a count of the number of transpositions performed
        unsigned long move_low_columns(int s, unsigned n, int t);
        unsigned long move_high_columns(int s, unsigned n, int t);

        //swaps two blocks of columns by updating the total order on columns, then rebuilding the matrices and computing a new RU-decomposition
        void update_order_and_reset_matrices(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL_initial, MapMatrix_Perm* RH_initial);

        //updates the total order on columns, rebuilds the matrices, and computing a new RU-decomposition for a NON-STRICT anchor
        void update_order_and_reset_matrices(xiMatrixEntry* anchor, xiMatrixEntry* generator, MapMatrix_Perm* RL_initial, MapMatrix_Perm* RH_initial);

        //swaps two blocks of simplices in the total order, and counts switches and separations
        void count_switches_and_separations(xiMatrixEntry* at_anchor, bool from_below, unsigned long &switches, unsigned long &seps);

        //used by the previous function to split grade lists at each anchor crossing
        void do_separations(xiMatrixEntry* greater, xiMatrixEntry* lesser, bool horiz);

        //swaps two blocks of columns by using a quicksort to update the matrices, then fixing the RU-decomposition (Gaussian elimination on U followed by reduction of R)
        ///TODO: IMPLEMENT THIS!
        void quicksort_and_reduce(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below);

        //removes entries corresponding to an xiMatrixEntry from lift_low and lift_high
        void remove_lift_entries(xiMatrixEntry* entry);

        //creates the appropriate entries in lift_low and lift_high for an xiMatrixEntry with nonempty sets of "low" or "high" simplices
        void add_lift_entries(xiMatrixEntry* entry);

        //stores a barcode template in a 2-cell of the arrangement
        ///TODO: IMPROVE THIS -- track most recent barcode at the simplicial level and re-examine only the necessary columns!!!
        void store_barcode_template(Face* cell);

        ///TESTING ONLY
        void check_low_matrix(MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL);
        void check_high_matrix(MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH);

        void print_perms(Perm& per, Perm& inv);
        void print_high_partition();

};

#endif // __PERSISTENCE_UPDATER_H__
