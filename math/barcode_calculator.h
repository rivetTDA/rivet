/**
 * \class	BarcodeCalculator
 * \brief	Computes barcode templates (using the mathematics of "vineyard updates" to store in the Mesh)
 * \author	Matthew L. Wright
 * \date	March 2015
 */

#ifndef __BARCODE_CALCULATOR_H__
#define __BARCODE_CALCULATOR_H__

#include "../dcel/mesh.h"
#include "map_matrix.h"
#include "index_matrix.h"
#include "../dcel/xi_support_matrix.h"
#include "../dcel/barcode_template.h"
class Mesh;

class BarcodeCalculator
{
    public:

        BarcodeCalculator(Mesh* m, MultiBetti& mb, std::vector<xiPoint>& xi_pts);  //constructor -- also fills xi_matrix with the xi support points

        void find_anchors(); //computes anchors and stores them in mesh->all_anchors; anchor-lines will be created when mesh->build_interior() is called

        void store_barcodes(std::vector<Halfedge *> &path);  //computes and stores a barcode template in each 2-cell of mesh

    private:
      //data structures

        Mesh* mesh;                 //pointer to the DCEL arrangement in which the barcodes will be stored
        SimplexTree* bifiltration;  //pointer to the bifiltration
        int dim;                    //dimension of homology to be computed

        xiSupportMatrix xi_matrix;   //sparse matrix to hold xi support points -- used for finding anchors (to build the arrangement) and tracking simplices during the vineyard updates (when computing barcodes to store in the arrangement)

        std::map<unsigned, xiMatrixEntry*> partition_low;   //map from "low" columns to equivalence-class representatives -- implicitly stores the partition of the set of \xi support points
        std::map<unsigned, xiMatrixEntry*> partition_high;  //map from "high" columns to equivalence-class representatives -- implicitly stores the partition of the set of \xi support points

        ///TESTING ONLY
        MapMatrix_Perm* D_low;
        MapMatrix_Perm* D_high;

      //functions

        //stores multigrade info for the persistence computations (data structures prepared with respect to a near-vertical line positioned to the right of all \xi support points)
        //  low is true for simplices of dimension hom_dim, false for simplices of dimension hom_dim+1
        //  simplex_order will be filled with a map : dim_index --> order_index for simplices of the given dimension
        void store_multigrades(IndexMatrix* ind, bool low, std::vector<int>& simplex_order);

        //moves columns from an equivalence class given by xiMatrixEntry* first to their new positions after or among the columns in the equivalence class given by xiMatrixEntry* second
        // the boolean argument indicates whether an anchor is being crossed from below (or from above)
        ///TODO: IMPLEMENT LAZY SWAPPING!
        void move_columns(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH);

        //moves a block of n columns, the rightmost of which is column s, to a new position following column t (NOTE: assumes s <= t)
        void move_low_columns(int s, unsigned n, int t, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH);

        //moves a block of n columns, the rightmost of which is column s, to a new position following column t (NOTE: assumes s <= t)
        void move_high_columns(int s, unsigned n, int t, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH);

        //removes entries corresponding to xiMatrixEntry head from partition_low and partition_high
        void remove_partition_entries(xiMatrixEntry* head);

        //if the equivalence class corresponding to xiMatrixEntry head has nonempty sets of "low" or "high" simplices, then this function creates the appropriate entries in partition_low and partition_high
        void add_partition_entries(xiMatrixEntry* head);

        //stores a barcode template in a 2-cell of the arrangement
        ///TODO: IMPROVE THIS -- track most recent barcode at the simplicial level and re-examine only the necessary columns!!!
        void store_barcode_template(Face* cell, MapMatrix_Perm* RL, MapMatrix_Perm* RH);
};

#endif // __BARCODE_CALCULATOR_H__
