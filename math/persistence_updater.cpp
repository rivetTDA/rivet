#include "persistence_updater.h"

#include "index_matrix.h"
#include "map_matrix.h"
#include "multi_betti.h"
#include "simplex_tree.h"
#include "../computationthread.h"
#include "../dcel/barcode_template.h"
#include "../dcel/dcel.h"
#include "../dcel/mesh.h"

#include <QDebug>
#include <QTime>


//constructor for when we must compute all of the barcode templates
PersistenceUpdater::PersistenceUpdater(Mesh *m, SimplexTree* b, std::vector<xiPoint> &xi_pts) :
    mesh(m), bifiltration(b), dim(b->hom_dim),
    xi_matrix(m->x_grades.size(), m->y_grades.size()),
    testing(false)
{
    //fill the xiSupportMatrix with the xi support points
    xi_matrix.fill(xi_pts);

    //create partition entries for infinity (which never change)
    unsigned infty = -1;    // = MAX_UNSIGNED, right?
    partition_low.insert( std::pair<unsigned, xiMatrixEntry*>(infty, xi_matrix.get_infinity()) );
    partition_high.insert( std::pair<unsigned, xiMatrixEntry*>(infty, xi_matrix.get_infinity()) );
}

//constructor for when we load the pre-computed barcode templates from a RIVET data file
PersistenceUpdater::PersistenceUpdater(Mesh* m, std::vector<xiPoint>& xi_pts) :
    mesh(m),
    xi_matrix(m->x_grades.size(), m->y_grades.size()),
    testing(false)
{
    //fill the xiSupportMatrix with the xi support points
    xi_matrix.fill(xi_pts);

    //create partition entries for infinity (which never change)
    unsigned infty = -1;    // = MAX_UNSIGNED
    partition_low.insert( std::pair<unsigned, xiMatrixEntry*>(infty, xi_matrix.get_infinity()) );
    partition_high.insert( std::pair<unsigned, xiMatrixEntry*>(infty, xi_matrix.get_infinity()) );
}

//computes anchors and stores them in mesh->all_anchors; anchor-lines will be created when mesh->build_interior() is called
void PersistenceUpdater::find_anchors()
{
    if(mesh->verbosity >= 2) { qDebug() << "FINDING ANCHORS"; }

    //get pointers to top entries in nonempty columns
    std::list<xiMatrixEntry*> nonempty_cols;
    for(unsigned i = 0; i < mesh->x_grades.size(); i++)
    {
        xiMatrixEntry* col_entry = xi_matrix.get_col(i); //top entry in row j, possibly NULL
        if(col_entry != NULL)
            nonempty_cols.push_front(col_entry);
    }

    //compute and store anchors
    for(unsigned j = mesh->y_grades.size(); j-- > 0; )  //loop through all rows, top to bottom
    {
        xiMatrixEntry* row_entry = xi_matrix.get_row(j); //rightmost entry in row j, possibly NULL

        std::list<xiMatrixEntry*>::iterator it = nonempty_cols.begin();
        while( it != nonempty_cols.end() )  //loop through all nonempty columns, right to left
        {
            if(row_entry == NULL)   //then there is nothing else in this row
                break;

            //check if there is a Anchor in position (i,j)
            xiMatrixEntry* col_entry = *it;
            if(col_entry == NULL)
                 qDebug() << "ERROR in Mesh::store_xi_points() : NULL col_entry";
            if(row_entry != col_entry)  //then there is a strict, non-supported anchor at (col_entry->x, row_entry->y)
            {
                mesh->all_anchors.insert(new Anchor(col_entry, row_entry));

                ///TODO: BUG FIX JULY 2015 -- create a xiMatrixEntry for this anchor and add it to the list of xi support points for the VisualizationWindow
                ///     idea: could wait to build the list of points for VisualizationWindow until after find_anchors() completes, then just run through the xiSupportMatrix one time to build this list and set the index values of each xiMatrixEntry

                if(mesh->verbosity >= 10) { qDebug() << "  anchor (strict, non-supported) found at (" << col_entry->x << "," << row_entry->y << ")"; }
            }
            else    //then row_entry == col_entry, so there might be a supported anchor at (col_entry->x, row_entry->y), or there might be no anchor here
            {
                if(col_entry->down != NULL && row_entry->left != NULL)  //then there is a strict and supported anchor
                {
                    mesh->all_anchors.insert(new Anchor(col_entry, true));
                    if(mesh->verbosity >= 10) { qDebug() << "  anchor (strict and supported) found at (" << col_entry->x << "," << col_entry->y << ")"; }
                }
                else if(col_entry->down != NULL || row_entry->left != NULL)  //then there is a supported, non-strict anchor
                {
                    mesh->all_anchors.insert(new Anchor(col_entry, false));
                    if(mesh->verbosity >= 10) { qDebug() << "  anchor (supported, non-strict) found at (" << col_entry->x << "," << col_entry->y << ")"; }
                }
            }

            //update cur_row_entry, if necessary
            if( row_entry->x == col_entry->x )  //then set row_entry to the next non-null entry in this row, if such entry exists
                row_entry = row_entry->left;

            //update nonempty_cols, if necessary
            if( col_entry->y == j )  //then replace this column entry with the next non-null entry in this column, if such entry exists
            {
                it = nonempty_cols.erase(it);   //NOTE: this advances the column iterator!

                if(col_entry->down != NULL)
                    nonempty_cols.insert(it, col_entry->down);   ///TODO: CHECK THIS!!!
            }
            else    //then advance the column iterator
                ++it;
        }//end column loop
    }//end row loop
}//end find_anchors()

//computes and stores a barcode template in each 2-cell of mesh
//resets the matrices and does a standard persistence calculation for expensive crossings
void PersistenceUpdater::store_barcodes_with_reset(std::vector<Halfedge*>& path, ComputationThread* cthread)
{
    QTime timer;    //for timing the computations

    // PART 1: GET THE BOUNDARY MATRICES WITH PROPER SIMPLEX ORDERING

    timer.start();

    //initialize the map from simplex grades to xi support points
    if(mesh->verbosity >= 6) { qDebug() << "  Mapping low simplices:"; }
    IndexMatrix* ind_low = bifiltration->get_index_mx(dim);    //can we improve this with something more efficient than IndexMatrix?
    store_multigrades(ind_low, true);

    if(mesh->verbosity >= 6) { qDebug() << "  Mapping high simplices:"; }
    IndexMatrix* ind_high = bifiltration->get_index_mx(dim + 1);    //again, could be improved?
    store_multigrades(ind_high, false);

    //get the proper simplex ordering
    std::vector<int> low_simplex_order;     //this will be a map : dim_index --> order_index for dim-simplices
    build_simplex_order(ind_low, true, low_simplex_order);
    delete ind_low;

    std::vector<int> high_simplex_order;     //this will be a map : dim_index --> order_index for (dim+1)-simplices
    build_simplex_order(ind_high, false, high_simplex_order);
    delete ind_high;

    //get boundary matrices (R) and identity matrices (U) for RU-decomposition
    MapMatrix_Perm* R_low = bifiltration->get_boundary_mx(low_simplex_order);
    MapMatrix_Perm* R_high = bifiltration->get_boundary_mx(low_simplex_order, high_simplex_order);

    //print runtime data
    qDebug() << "  --> computing initial order on simplices and building the boundary matrices took" << timer.elapsed() << "milliseconds";

    //copy the boundary matrices (R) for fast reset later
    timer.start();
    MapMatrix_Perm* R_low_initial = new MapMatrix_Perm(*R_low);
    MapMatrix_Perm* R_high_initial = new MapMatrix_Perm(*R_high);
    qDebug() << "  --> copying the boundary matrices took" << timer.elapsed() << "milliseconds";

    //initialize the permutation vectors -- I wish I didn't have to maintain all this, but I don't know how to avoid it
    std::vector<unsigned> perm_low(R_low->width());         //map from column index at initial cell to column index at current cell
    std::vector<unsigned> inv_perm_low(R_low->width());     //inverse of the previous map
    std::vector<unsigned> perm_high(R_high->width());       //map from column index at initial cell to column index at current cell
    std::vector<unsigned> inv_perm_high(R_high->width());   //inverse of the previous map
    for(unsigned j=0; j < perm_low.size(); j++)
    {
        perm_low[j] = j;
        inv_perm_low[j] = j;
    }
    for(unsigned j=0; j < perm_high.size(); j++)
    {
        perm_high[j] = j;
        inv_perm_high[j] = j;
    }


  // PART 2: INITIAL PERSISTENCE COMPUTATION (RU-decomposition)

    timer.start();

    MapMatrix_RowPriority_Perm* U_low = R_low->decompose_RU();
    MapMatrix_RowPriority_Perm* U_high = R_high->decompose_RU();

    qDebug() << "  --> computing the RU decomposition took" << timer.elapsed() << "milliseconds";

    //store the barcode template in the first cell
    Face* first_cell = mesh->topleft->get_twin()->get_face();
    store_barcode_template(first_cell, R_low, R_high);

    qDebug() << "Initial persistence computation in cell " << mesh->FID(first_cell);
    print_perms(perm_high, inv_perm_high);

  // PART 3: TRAVERSE THE PATH AND UPDATE PERSISTENCE AT EACH STEP

    qDebug() << "TRAVERSING THE PATH USING THE RESET ALGORITHM: path has" << path.size() << "steps";
    qDebug() << "                              ^^^^^^^^^^^^^^^";

    ///TODO: set the threshold dynamically
    unsigned long threshold = 800000;     //if the number of swaps might exceed this threshold, then do a persistence calculation from scratch
    unsigned long swap_estimate = 0;
    qDebug() << "reset threshold set to" << threshold;

    timer.start();

    ///TEMPORARY: data structures for analyzing the computation
    unsigned long total_transpositions = 0;
    unsigned number_of_resets = 0;
    unsigned total_time_for_resets = 0;
    int max_time = 0;

    //traverse the path
    QTime steptimer;
    for(unsigned i=0; i<path.size(); i++)
    {
        cthread->setCurrentProgress(i);    //update progress bar

        steptimer.start();                //time update at each step of the path
        unsigned long swap_counter = 0;   //counts number of transpositions at each step
        swap_estimate = 0;

        //determine which anchor is represented by this edge
        Anchor* cur_anchor = (path[i])->get_anchor();

        qDebug() << "  step" << i << "of path: crossing anchor at ("<< cur_anchor->get_x() << "," << cur_anchor->get_y() << ") into cell" << mesh->FID((path[i])->get_face());

        //get equivalence classes for this anchor
        xiMatrixEntry* down = cur_anchor->get_down();
        xiMatrixEntry* left = cur_anchor->get_left();

        //if this is a strict anchor, then swap simplices
        if(left != NULL) //then this is a strict anchor and some simplices swap
        {
            xiMatrixEntry* at_anchor = NULL;   //remains NULL iff this anchor is not supported

            if(down == NULL)    //then this is also a supported anchor
            {
                at_anchor = left;
                down = left->down;
                left = left->left;
            }//now down and left are correct (and should not be NULL)

            //process the swaps
            if(cur_anchor->is_above()) //then the anchor is crossed from below to above
            {
                //pre-move updates to equivalence class info
                if(at_anchor != NULL)  //this anchor is supported
                {
                    left->low_index = at_anchor->low_index - at_anchor->low_count;                //necessary since low_index, low_class_size,
                    left->low_class_size = at_anchor->low_class_size - at_anchor->low_count;      //  high_index, and high_class_size
                    left->high_index = at_anchor->high_index - at_anchor->high_count;             //  are only reliable for the head
                    left->high_class_size = at_anchor->high_class_size - at_anchor->high_count;   //  of each equivalence class
                    remove_partition_entries(at_anchor);   //this partition might become empty
                }
                else    //this anchor is not supported
                {
                    remove_partition_entries(left);     //this block of the partition will move
                }

                remove_partition_entries(down);         //this block of the partition will move

                //now permute the columns and fix the RU-decomposition
                swap_estimate = static_cast<unsigned long>(left->low_class_size) * static_cast<unsigned long>(down->low_class_size)
                        + static_cast<unsigned long>(left->high_class_size) * static_cast<unsigned long>(down->high_class_size);
                if(swap_estimate < threshold)
                    swap_counter += move_columns(down, left, true, R_low, U_low, R_high, U_high, perm_low, inv_perm_low, perm_high, inv_perm_high);
                else
                    update_order_and_reset_matrices(down, left, true, R_low, U_low, R_high, U_high, R_low_initial, R_high_initial, perm_low, inv_perm_low, perm_high, inv_perm_high);

                //post-move updates to equivalance class info
                if(at_anchor != NULL)  //this anchor is supported
                {
                    at_anchor->low_class_size = at_anchor->low_count + down->low_class_size;
                    at_anchor->high_class_size = at_anchor->high_count + down->high_class_size;
                    down->low_class_size = -1;  //this xiMatrixEntry is no longer the head of an equivalence class
                    add_partition_entries(at_anchor);
                }
                else    //this anchor is not supported
                {
                    add_partition_entries(down);        //this block of the partition moved
                }

                add_partition_entries(left);            //this block of the partition moved
            }
            else    //then anchor is crossed from above to below
            {
                //pre-move updates to equivalence class info
                if(at_anchor != NULL)
                {
                    down->low_index = at_anchor->low_index - at_anchor->low_count;                //necessary since low_index, low_class_size,
                    down->low_class_size = at_anchor->low_class_size - at_anchor->low_count;      //  high_index, and high_class_size
                    down->high_index = at_anchor->high_index - at_anchor->high_count;             //  are only reliable for the head
                    down->high_class_size = at_anchor->high_class_size - at_anchor->high_count;   //  of each equivalence class
                    remove_partition_entries(at_anchor);   //this partition might become empty
                }
                else    //this anchor is not supported
                {
                    remove_partition_entries(down);     //this block of the partition will move
                }

                remove_partition_entries(left);         //this block of the partition will move

                //now permute the columns and fix the RU-decomposition
                swap_estimate = static_cast<unsigned long>(left->low_class_size) * static_cast<unsigned long>(down->low_class_size)
                        + static_cast<unsigned long>(left->high_class_size) * static_cast<unsigned long>(down->high_class_size);
                if(swap_estimate < threshold)
                    swap_counter += move_columns(left, down, false, R_low, U_low, R_high, U_high, perm_low, inv_perm_low, perm_high, inv_perm_high);
                else
                    update_order_and_reset_matrices(left, down, false, R_low, U_low, R_high, U_high, R_low_initial, R_high_initial, perm_low, inv_perm_low, perm_high, inv_perm_high);

                //post-move updates to equivalance class info
                if(at_anchor != NULL)  //this anchor is supported
                {
                    at_anchor->low_class_size = at_anchor->low_count + left->low_class_size;
                    at_anchor->high_class_size = at_anchor->high_count + left->high_class_size;
                    left->low_class_size = -1;  //this xiMatrixEntry is no longer the head of an equivalence class
                    add_partition_entries(at_anchor);
                }
                else    //this anchor is not supported
                {
                    add_partition_entries(left);        //this block of the partition moved
                }

                add_partition_entries(down);            //this block of the partition moved
            }
        }
        else    //then this is a supported, non-strict anchor, and we just have to split or merge equivalence classes
        {
            xiMatrixEntry* at_anchor = down;
            xiMatrixEntry* generator = at_anchor->down;
            if(generator == NULL)
                generator = at_anchor->left;

            if(generator->low_class_size != -1)    //then merge classes
            {
                at_anchor->low_class_size = at_anchor->low_count + generator->low_class_size;
                at_anchor->high_class_size = at_anchor->high_count + generator->high_class_size;
                generator->low_class_size = -1;    //indicates that this xiMatrixEntry is NOT the head of an equivalence class

                remove_partition_entries(generator);
                add_partition_entries(at_anchor);  //this is necessary in case the class was previously empty
            }
            else    //then split classes
            {
                generator->low_index = at_anchor->low_index - at_anchor->low_count;
                generator->low_class_size = at_anchor->low_class_size - at_anchor->low_count;
                at_anchor->low_class_size = at_anchor->low_count;
                generator->high_index = at_anchor->high_index - at_anchor->high_count;
                generator->high_class_size = at_anchor->high_class_size - at_anchor->high_count;
                at_anchor->high_class_size = at_anchor->high_count;

                remove_partition_entries(at_anchor);   //this is necessary because the class corresponding
                add_partition_entries(at_anchor);      //  to at_anchor might have become empty
                add_partition_entries(generator);
            }
        }

        //remember that we have crossed this anchor
        cur_anchor->toggle();

        ///TESTING
//        print_perms(perm_high, inv_perm_high);
//        print_high_partition();

        //if this cell does not yet have a barcode template, then store it now
        Face* cur_face = (path[i])->get_face();
        if(!cur_face->has_been_visited())
            store_barcode_template(cur_face, R_low, R_high);

        //print/store data for analysis
        int step_time = steptimer.elapsed();
        if(swap_estimate < threshold)
        {
            qDebug() << "    --> this step took" << step_time << "milliseconds and involved" << swap_counter << "transpositions";
            total_transpositions += swap_counter;
        }
        else
        {
            qDebug() << "    --> this step took" << step_time << "milliseconds -- reset matrices to avoid an estimated" << swap_estimate << "transpositions";
            number_of_resets++;
            total_time_for_resets += step_time;
        }

        if(step_time > max_time)
            max_time = step_time;
    }//end path traversal

    //print runtime data
    qDebug() << "DATA: path traversal and persistence updates took" << timer.elapsed() << "milliseconds";
    qDebug() << "    max time per anchor crossing:" << max_time;
    qDebug() << "    total number of transpositions:" << total_transpositions;
    qDebug() << "    matrices were reset" << number_of_resets << "times when estimated number of transpositions exceeded" << threshold;
    if(number_of_resets > 0)
        qDebug() << "    average time for reset:" << (total_time_for_resets/number_of_resets) << "milliseconds";


  // PART 4: CLEAN UP

    delete R_low;
    delete R_high;
    delete U_low;
    delete U_high;

}//end store_barcodes_with_reset()


//stores multigrade info for the persistence computations (data structures prepared with respect to a near-vertical line positioned to the right of all \xi support points)
//  low is true for simplices of dimension hom_dim, false for simplices of dimension hom_dim+1
void PersistenceUpdater::store_multigrades(IndexMatrix* ind, bool low)
{
    if(mesh->verbosity >= 6) { qDebug() << "STORING MULTIGRADES: low =" << low; }

    //initialize linked list to track the "frontier"
    typedef std::list<xiMatrixEntry*> Frontier;
    Frontier frontier;

    //loop through rows of xiSupportMatrix, from top to bottom
    for(unsigned y = ind->height(); y-- > 0; )  //y counts down from (ind->height() - 1) to 0
    {
        //update the frontier for row y
        xiMatrixEntry* cur = xi_matrix.get_row(y);
        if(cur != NULL)
        {
            //advance an iterator to the first entry that is not right of cur
            Frontier::iterator it = frontier.begin();
            while( it != frontier.end() && (*it)->x > cur->x )
                ++it;

            //erase any entries from this position to the end of the frontier
            frontier.erase(it, frontier.end());

            //insert cur at the end of the frontier
            frontier.push_back(cur);

            //now add all other non-null entries in this row to the frontier
            cur = cur->left;
            while(cur != NULL)
            {
                frontier.push_back(cur);
                cur = cur->left;
            }
        }

        //store all multigrades and simplices whose y-grade is y
        Frontier::iterator it = frontier.begin();
        for(unsigned x = ind->width(); x-- > 0; )  //x counts down from (ind->width() - 1) to 0
        {
            //get range of column indexes for simplices at multigrade (x,y)
            int last_col = ind->get(y, x);  //arguments are row, then column
            int first_col = -1;
            if(x > 0)
                first_col = ind->get(y, x-1);
            else if(y > 0)
                first_col = ind->get(y-1, ind->width()-1);

            //if there are any simplices at (x,y), then add multigrade (x,y)
            if(last_col > first_col)
            {
                //if the frontier is empty or if x is to the right of the first element, then map multigrade (x,y) to infinity
                if( it == frontier.end() || (*it)->x < x )    //NOTE: if iterator has advanced from frontier.begin(), then it MUST be the case that x < (*it)->x
                {
                    xi_matrix.get_infinity()->add_multigrade(x, y, last_col - first_col, last_col, low);

                    if(mesh->verbosity >= 6) { qDebug() << "    simplices at (" << x << "," << y << "), in columns" << (first_col + 1) << "to" << last_col << ", mapped to infinity"; }
                }
                else    //then map multigrade (x,y) to the last element of the frontier such that (*it)->x >= x
                {
                    //advance the iterator to the first element of the frontier such that (*it)->x < x
                    while( it != frontier.end() && (*it)->x >= x )
                        ++it;

                    //back up one position, to the last element of the frontier such that (*it)->x >= x
                    --it;

                    //now map the multigrade to the xi support entry
                    (*it)->add_multigrade(x, y, last_col - first_col, last_col, low);

                    if(mesh->verbosity >= 6) { qDebug() << "    simplices at (" << x << "," << y << "), in columns" << (first_col + 1) << "to" << last_col << ", mapped to xi support point (" << (*it)->x << ", " << (*it)->y << ")"; }
                }
            }
        }//end x loop
    }//end y loop
}//end store_multigrades()

//finds the proper order of simplexes for the persistence calculation (with respect to a near-vertical line positioned to the right of all \xi support points)
//  NOTE: within each equivalence class, multigrades will occur in lexicographical order
//  PARAMETERS:
//    low is true for simplices of dimension hom_dim, false for simplices of dimension hom_dim+1
//    simplex_order will be filled with a map : dim_index --> order_index for simplices of the given dimension
void PersistenceUpdater::build_simplex_order(IndexMatrix* ind, bool low, std::vector<int>& simplex_order)
{
    //we will create the map starting by identifying the order index of each simplex, starting with the last simplex
    int o_index = ind->last();
    simplex_order.resize(o_index + 1);

    //first consider all simplices that map to the xiMatrixEntry infinity
    xiMatrixEntry* cur = xi_matrix.get_infinity();
    std::list<Multigrade*>* mgrades = (low) ? &(cur->low_simplices) : &(cur->high_simplices);
    for(std::list<Multigrade*>::iterator it = mgrades->begin(); it != mgrades->end(); ++it)
    {
        Multigrade* mg = *it;
        if(mesh->verbosity >= 2) { qDebug() << "  multigrade (" << mg->x << "," << mg->y << ") at infinity has" << mg->num_cols << "simplices with last index" << mg->simplex_index<< "which will map to order_index" << o_index; }

        for(unsigned s=0; s < mg->num_cols; s++)  // simplex with dim_index (mg->simplex_index - s) has order_index o_index
        {
            simplex_order[mg->simplex_index - s] = o_index;
            o_index--;
        }
    }

    //now loop over all xiMatrixEntries in backwards colex order
    for(unsigned row = xi_matrix.height(); row-- > 0; )  //row counts down from (xi_matrix->height() - 1) to 0
    {
        cur = xi_matrix.get_row(row);
        if(cur == NULL)
            continue;

        //if we get here, the current row is nonempty, so it forms an equivalence class in the partition
        if(cur->low_class_size == -1)
            cur->low_class_size = 0;

        //store index of rightmost column that is mapped to this equivalence class
        int* cur_ind = (low) ? &(cur->low_index) : &(cur->high_index);
        *cur_ind = o_index;

        //consider all xiMatrixEntries in this row
        while(cur != NULL)
        {
            if(mesh->verbosity >= 6) { qDebug() << "----xiMatrixEntry (" << cur->x << "," << cur->y << ")"; }

            //get the multigrade list for this xiMatrixEntry
            mgrades = (low) ? &(cur->low_simplices) : &(cur->high_simplices);

            //sort the multigrades in lexicographical order
            mgrades->sort(Multigrade::LexComparator);

            //store map values for all simplices at these multigrades
            for(std::list<Multigrade*>::iterator it = mgrades->begin(); it != mgrades->end(); ++it)
            {
                Multigrade* mg = *it;
                if(mesh->verbosity >= 2) { qDebug() << "  multigrade (" << mg->x << "," << mg->y << ") has" << mg->num_cols << "simplices with last dim_index" << mg->simplex_index << "which will map to order_index" << o_index; }

                for(unsigned s=0; s < mg->num_cols; s++)  // simplex with dim_index (mg->simplex_index - s) has order_index o_index
                {
                    simplex_order[mg->simplex_index - s] = o_index;
                    o_index--;
                }
            }

            //move to the next xiMatrixEntry in this row
            cur = cur->left;
        }

        //if any simplices of the specified dimension were mapped to this equivalence class, then store information about this class
        if(*cur_ind != o_index)
        {
            if(low)
            {
                xi_matrix.get_row(row)->low_class_size = *cur_ind - o_index;
                partition_low.insert( std::pair<unsigned, xiMatrixEntry*>(*cur_ind, xi_matrix.get_row(row)) );
            }
            else
            {
                xi_matrix.get_row(row)->high_class_size = *cur_ind - o_index;
                partition_high.insert( std::pair<unsigned, xiMatrixEntry*>(*cur_ind, xi_matrix.get_row(row)) );
            }
        }

        //now store column index in the "unsorted" bin for this row
        ///THIS SHOULD BE UNNECESSARY UNLESS WE ARE DOING LAZY UPDATES
//        xiMatrixEntry* cur_row = xi_matrix.get_row(row);
//        if(cur_row->low_class_size != -1)
//        {
//            if(low)
//                xi_matrix.get_row_bin(row)->low_index = cur_row->low_index - cur_row->low_class_size;
//            else
//                xi_matrix.get_row_bin(row)->high_index = cur_row->high_index - cur_row->high_class_size;
//        }
    }//end for(row > 0)
}//end build_simplex_order()

//moves columns from an equivalence class given by xiMatrixEntry* first to their new positions after or among the columns in the equivalence class given by xiMatrixEntry* second
// the boolean argument indicates whether an anchor is being crossed from below (or from above)
// this version updates the permutation vectors required for the "reset" approach
//NOTE: this function has been updated for the July 2015 bug fix
unsigned long PersistenceUpdater::move_columns(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH, Perm& perm_low, Perm& inv_perm_low, Perm& perm_high, Perm& inv_perm_high)
{
    ///DEBUGGING
    if(first->low_index + second->low_class_size != second->low_index || first->high_index + second->high_class_size != second->high_index)
    {
        qDebug() << "  ===>>> ERROR: swapping non-consecutive column blocks!";
    }

    //get column indexes (so we know which columns to move)
    int low_col = first->low_index;   //rightmost column index of low simplices for the equivalence class to move
    int high_col = first->high_index; //rightmost column index of high simplices for the equivalence class to move

    //set column indexes for the first class to their final position
    first->low_index = second->low_index;
    first->high_index = second->high_index;

    //initialize counter
    unsigned long swap_counter = 0;

    //loop over all xiMatrixEntrys in the first equivalence class
    xiMatrixEntry* cur_entry = first;
    while(cur_entry != NULL)
    {
        //move all "low" simplices for this xiMatrixEntry (start with rightmost column, end with leftmost)
        for(std::list<Multigrade*>::iterator it = cur_entry->low_simplices.begin(); it != cur_entry->low_simplices.end(); ) //NOTE: iterator advances in loop
        {
            Multigrade* cur_grade = *it;

            if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
                //then move columns at cur_grade past columns at xiMatrixEntry second; lift map does not change ( lift : multigrades --> xiSupportElements )
            {
                swap_counter += move_low_columns(low_col, cur_grade->num_cols, second->low_index, RL, UL, RH, UH, perm_low, inv_perm_low);
                second->low_index -= cur_grade->num_cols;
                ++it;
            }
            else    //then move columns at cur_grade to some position in the equivalence class given by xiMatrixEntry second; lift map changes
            {
                xiMatrixEntry* target = second;
                int target_col = second->low_index - second->low_count;

                //re-compute the lift map for cur_grade and find the new position for the columns
                if(from_below)
                {
                    while( (target->left != NULL) && (cur_grade->x <= target->left->x) )
                    {
                        target = target->left;
                        target_col -= target->low_count;
                    }
                }
                else
                {
                    while( (target->down != NULL) && (cur_grade->y <= target->down->y) )
                    {
                        target = target->down;
                        target_col -= target->low_count;
                    }
                }

                //associate cur_grade with target
                target->insert_multigrade(cur_grade, true);
                it = cur_entry->low_simplices.erase(it);    //NOTE: advances the iterator!!!

                //if target is not the leftmost entry in its equivalence class, then move columns at cur_grade to the block of columns for target
                if( (from_below && target->left != NULL) || (!from_below && target->down != NULL) )
                    swap_counter += move_low_columns(low_col, cur_grade->num_cols, target_col, RL, UL, RH, UH, perm_low, inv_perm_low);
                //else, then the columns don't actually have to move

                //update column counts
                cur_entry->low_count -= cur_grade->num_cols;
                target->low_count += cur_grade->num_cols;

                //update equivalence class sizes
                first->low_class_size -= cur_grade->num_cols;
                second->low_class_size += cur_grade->num_cols;
            }

            //update column index
            low_col -= cur_grade->num_cols;
        }//end "low" simplex loop

        //move all "high" simplices for this xiMatrixEntry
        for(std::list<Multigrade*>::iterator it = cur_entry->high_simplices.begin(); it != cur_entry->high_simplices.end(); ) //NOTE: iterator advances in loop
        {
            Multigrade* cur_grade = *it;

//            qDebug() << "  ====>>>> moving high simplices at grade (" << cur_grade->x << "," << cur_grade->y << ")";

            if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
                //then move columns at cur_grade past columns at xiMatrixEntry second; lift map does not change ( lift : multigrades --> xiSupportElements )
            {
                swap_counter += move_high_columns(high_col, cur_grade->num_cols, second->high_index, RH, UH, perm_high, inv_perm_high);
                second->high_index -= cur_grade->num_cols;
                ++it;
            }
            else    //then move columns at cur_grade to some position in the equivalence class given by xiMatrixEntry second; lift map changes
            {
                xiMatrixEntry* target = second;
                int target_col = second->high_index - second->high_count;

                //re-compute the lift map for cur_grade and find the new position for the columns
                if(from_below)
                {
                    while( (target->left != NULL) && (cur_grade->x <= target->left->x) )
                    {
                        target = target->left;
                        target_col -= target->high_count;
                    }
                }
                else
                {
                    while( (target->down != NULL) && (cur_grade->y <= target->down->y) )
                    {
                        target = target->down;
                        target_col -= target->high_count;
                    }
                }

                ///TESTING
//                qDebug() << "====>>>> simplex at (" << cur_grade->x << "," << cur_grade->y << ") now lifts to (" << target->x << "," << target->y << ")";

                //associate cur_grade with target
                target->insert_multigrade(cur_grade, false);
                it = cur_entry->high_simplices.erase(it);    //NOTE: advances the iterator!!!

                //if target is not the leftmost entry in its equivalence class, then move columns at cur_grade to the block of columns for target
                if( (from_below && target->left != NULL) || (!from_below && target->down != NULL) )
                    swap_counter += move_high_columns(high_col, cur_grade->num_cols, target_col, RH, UH, perm_high, inv_perm_high);
                //else, then the columns don't actually have to move

                //update column counts
                cur_entry->high_count -= cur_grade->num_cols;
                target->high_count += cur_grade->num_cols;

                //update equivalence class sizes
                first->high_class_size -= cur_grade->num_cols;
                second->high_class_size += cur_grade->num_cols;
            }

            //update column index
            high_col -= cur_grade->num_cols;
        }//end "high" simplex loop

        //advance to the next xiMatrixEntry in the first equivalence class
        cur_entry = from_below ? cur_entry->down : cur_entry->left;
    }//end while

    ///DEBUGGING
    if(second->low_index + first->low_class_size != first->low_index || second->high_index + first->high_class_size != first->high_index)
    {
        qDebug() << "  ===>>> ERROR: swap resulted in non-consecutive column blocks!";
    }
    return swap_counter;
}//end move_columns()

//moves a block of n columns, the rightmost of which is column s, to a new position following column t (NOTE: assumes s <= t)
// this version maintains the permutation arrays required for the "reset" approach
unsigned long PersistenceUpdater::move_low_columns(int s, unsigned n, int t, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH, Perm& perm_low, Perm& inv_perm_low)
{
//    qDebug() << "   --Transpositions for low simplices: [" << s << "," << n << "," << t << "]:" << (n*(t-s)) << "total";
    if(s > t)
    {
        qDebug() << "    ===>>> ERROR: illegal column move";
    }

    for(unsigned c=0; c<n; c++) //move column that starts at s-c
    {
        for(int i=s; i<t; i++)
        {
            unsigned a = i - c; //TODO: cast i to unsigned???
            unsigned b = a + 1;

            //we must swap the d-simplices currently corresponding to columns a and b=a+1
            if(mesh->verbosity >= 9) { qDebug() << "(" << a << "," << b << ")"; }

            //update the permutation vectors
            unsigned s = inv_perm_low[a];
            unsigned t = inv_perm_low[b];
            inv_perm_low[a] = t;
            inv_perm_low[b] = s;
            perm_low[t] = a;
            perm_low[s] = b;

            //now for the vineyards algorithm
            bool a_pos = (RL->low(a) == -1);    //true iff simplex corresponding to column a is positive
            bool b_pos = (RL->low(b) == -1);    //true iff simplex corresponding to column b=a+1 is positive

            if(a_pos)  //simplex a is positive (Vineyards paper - Cases 1 and 4)
            {
                if(b_pos)   //simplex b is positive (Case 1)
                {
                    //look for columns k and l in RH with low(k)=a, low(l)=b, and RH(a,l)=1 -- if these exist, then we must fix matrix RH following row/column swaps (Case 1.1)
                    int k = RH->find_low(a);
                    int l = RH->find_low(b);
                    bool RHal = (l > -1 && RH->entry(a, l));  //entry (a,l) in matrix RH

                    //ensure that UL[a,b]=0
                    UL->clear(a, b);

                    //transpose rows and columns (don't need to swap columns of RL, because these columns are zero)
                    UL->swap_columns(a);
                    UL->swap_rows(a);

                    //swap rows, and fix RH if necessary
                    if(k > -1 && RHal)  //case 1.1
                    {
                        if(k < l)
                        {
                            RH->swap_rows(a, true);  //in this case, low entries change
                            RH->add_column(k, l);
                            UH->add_row(l, k);
                        }
                        else
                        {
                            RH->swap_rows(a, false);  //in this case, low entries do not change
                            RH->add_column(l, k);
                            UH->add_row(k, l);
                        }
                    }
                    else
                        RH->swap_rows(a, !RHal);  //in this case, only necessary to update low entries if RH(a,l)=0 or if column l does not exist
                }
                else    //simplex b is negative (Case 4)
                {
                    //ensure that UL[a,b]=0
                    UL->clear(a, b);

                    //transpose rows and columns and update low arrays
                    RL->swap_columns(a, true);
                    RH->swap_rows(a, true);
                    UL->swap_columns(a);
                    UL->swap_rows(a);
                }
            }
            else    //simplex a is negative (Vineyards paper - Cases 2 and 3)
            {
                if(b_pos)   //simplex b is positive (Case 3)
                {
                    //look for column l in RH with low(l)=b and RH(a,l)=1
                    int l = RH->find_low(b);
                    bool RHal = (l > -1 && RH->entry(a, l));    //entry (a,l) in matrix RH

                    //transpose rows of R; update low array if necessary
                    RH->swap_rows(a, !RHal);

                    if(UL->entry(a, b))    //case 3.1 -- here, R = RWPW, so no further action required on R
                    {
                       UL->add_row(b, a);
                       UL->swap_rows(a);
                       UL->add_row(b, a);
                    }
                    else    //case 3.2
                    {
                        RL->swap_columns(a, true);
                        UL->swap_rows(a);
                    }
                }
                else    //simplex b is negative (Case 2)
                {
                    //transpose rows of R
                    RH->swap_rows(a, false);   //neither of these rows contain lowest 1's in any column

                    if(UL->entry(a, b)) //case 2.1
                    {
                        UL->add_row(b, a);  //so that U will remain upper-triangular
                        UL->swap_rows(a);   //swap rows of U

                        if(RL->low(a) < RL->low(b)) //case 2.1.1
                        {
                            RL->add_column(a, b);       //necessary due to the row addition on U; this doesn't change low entries
                            RL->swap_columns(a, true);  //now swap columns of R and update low entries
                        }
                        else //case 2.1.2
                        {
                            RL->add_column(a, b);       //necessary due to the row addition on U; this doesn't change low entries
                            RL->swap_columns(a, false); //now swap columns of R but DO NOT update low entries
                            RL->add_column(a, b);       //restore R to reduced form; low entries now same as they were initially
                            UL->add_row(b, a);          //necessary due to column addition on R
                        }
                    }
                    else    //case 2.2
                    {
                        RL->swap_columns(a, true);  //swap columns of R and update low entries
                        UL->swap_rows(a);           //swap rows of U
                    }
                }

                //finally, for cases 2 and 3, transpose columns of U
                UL->swap_columns(a);
            }

            /// TESTING ONLY - FOR CHECKING THAT D=RU
            if(testing)
            {
                D_low->swap_columns(a, false);
                D_high->swap_rows(a, false);
            }
        }//end for(i=...)
    }//end for(c=...)

    return n*(t-s);
}//end move_low_columns()


//moves a block of n columns, the rightmost of which is column s, to a new position following column t (NOTE: assumes s <= t)
// this version maintains the permutation arrays required for the "reset" approach
unsigned long PersistenceUpdater::move_high_columns(int s, unsigned n, int t, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH,  Perm& perm_high, Perm& inv_perm_high)
{
//    qDebug() << "   --Transpositions for high simplices: [" << s << "," << n << "," << t << "]:" << (n*(t-s)) << "total";
    if(s > t)
    {
        qDebug() << "    ===>>> ERROR: illegal column move";
    }

    for(unsigned c=0; c<n; c++) //move column that starts at s-c
    {
        for(int i=s; i<t; i++)
        {
            unsigned a = i - c; //TODO: cast i to unsigned???
            unsigned b = a + 1;

            //we must swap the (d+1)-simplices currently corresponding to columns a and b=a+1
//            qDebug() << "(" << a << "," << b << ")";

            bool a_pos = (RH->low(a) == -1);    //true iff simplex corresponding to column a is positive
            bool b_pos = (RH->low(b) == -1);    //true iff simplex corresponding to column b is positive

            //update the permutation vectors
            unsigned s = inv_perm_high[a];
            unsigned t = inv_perm_high[b];
            inv_perm_high[a] = t;
            inv_perm_high[b] = s;
            perm_high[t] = a;
            perm_high[s] = b;

            //now for the vineyards algorithm
            if(a_pos)   //simplex a is positive, so its column is zero, and the fix is easy  (Vineyards paper - Cases 1 and 4)
            {
                if(!b_pos)   //only have to swap columns of R if column b is nonzero
                    RH->swap_columns(a, true);

                //ensure that UL[a,b]=0
                UH->clear(a, b);

                //transpose rows and columns of U
                UH->swap_columns(a);
                UH->swap_rows(a);

                //done -- we don't care about the ROWS corresponding to simplices a and b, because we don't care about the boundaries of (d+2)-simplices
            }
            else    //simplex a is negative (Vineyards paper - Cases 2 and 3)
            {
                if(b_pos)   //simplex b is positive (Case 3)
                {
                    if(UH->entry(a, b))    //case 3.1 -- here, R = RWPW, so no further action required on R
                    {
                       UH->add_row(b, a);
                       UH->swap_rows(a);
                       UH->add_row(b, a);
                    }
                    else    //case 3.2
                    {
                        RH->swap_columns(a, true);
                        UH->swap_rows(a);
                    }
                }
                else    //simplex b is negative (Case 2)
                {
                    if(UH->entry(a, b)) //case 2.1
                    {
                        UH->add_row(b, a);  //so that U will remain upper-triangular
                        UH->swap_rows(a);   //swap rows of U

                        if(RH->low(a) < RH->low(b)) //case 2.1.1
                        {
                            RH->add_column(a, b);       //necessary due to the row addition on U; this doesn't change low entries
                            RH->swap_columns(a, true);  //now swap columns of R and update low entries
                        }
                        else //case 2.1.2
                        {
                            RH->add_column(a, b);       //necessary due to the row addition on U; this doesn't change low entries
                            RH->swap_columns(a, false); //now swap columns of R but DO NOT update low entries
                            RH->add_column(a, b);       //restore R to reduced form; low entries now same as they were initially
                            UH->add_row(b, a);          //necessary due to column addition on R
                        }
                    }
                    else    //case 2.2
                    {
                        RH->swap_columns(a, true);  //swap columns and update low entries
                        UH->swap_rows(a);           //swap rows of U
                    }
                }

                //finally, for Cases 2 and 3, transpose columns of U
                UH->swap_columns(a);
            }

            /// TESTING ONLY - FOR CHECKING THAT D=RU
            if(testing)
            {
                D_high->swap_columns(a, false);
            }
        }//end for(i=...)
    }//end for(c=...)

    return n*(t-s);
}//end move_high_columns()

//swaps two blocks of columns by updating the total order on columns, then rebuilding the matrices and computing a new RU-decomposition
void PersistenceUpdater::update_order_and_reset_matrices(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm*& UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm*& UH, MapMatrix_Perm* RL_initial, MapMatrix_Perm* RH_initial, Perm& perm_low, Perm& inv_perm_low, Perm& perm_high, Perm& inv_perm_high)
{
  //STEP 1: update the lift map for all multigrades and store the current column index for each multigrade

    //get column indexes for the second equivalence class
    int low_col = second->low_index;
    int high_col = second->high_index;

    //loop over all xiMatrixEntrys in the second equivalence class, storing current column indexes for each multigrade
    xiMatrixEntry* cur_entry = second;
    while(cur_entry != NULL)
    {
        //"low" simplices for this xiMatrixEntry (start with rightmost column, end with leftmost)
        for(std::list<Multigrade*>::iterator it = cur_entry->low_simplices.begin(); it != cur_entry->low_simplices.end(); ++it)
        {
            (*it)->simplex_index = low_col;
            low_col -= (*it)->num_cols;
        }

        //"high" simplices for this xiMatrixEntry (start with rightmost column, end with leftmost)
        for(std::list<Multigrade*>::iterator it = cur_entry->high_simplices.begin(); it != cur_entry->high_simplices.end(); ++it)
        {
            (*it)->simplex_index = high_col;
            high_col -= (*it)->num_cols;
        }

        //advance to the next xiMatrixEntry in the first equivalence class
        cur_entry = from_below ? cur_entry->left : cur_entry->down;
    }

    //get column indexes for the first equivalence class
    low_col = first->low_index;   //rightmost column index of low simplices for the equivalence class to move
    high_col = first->high_index; //rightmost column index of high simplices for the equivalence class to move

    //set column indexes for the first class to their final position
    first->low_index = second->low_index;
    first->high_index = second->high_index;

    //loop over all xiMatrixEntrys in the first equivalence class, storing current column indexes and updating the lift map
    cur_entry = first;
    while(cur_entry != NULL)
    {
        //move all "low" simplices for this xiMatrixEntry (start with rightmost column, end with leftmost)
        for(std::list<Multigrade*>::iterator it = cur_entry->low_simplices.begin(); it != cur_entry->low_simplices.end(); ) //NOTE: iterator advances in loop
        {
            Multigrade* cur_grade = *it;

            //remember current position of this grade
            cur_grade->simplex_index = low_col;

            if( (from_below && cur_grade->x > second->x && cur_grade->y <= first->y) || (!from_below && cur_grade->y > second->y && cur_grade->x <= first->x) )
                //then move columns at cur_grade past columns at xiMatrixEntry second; lift map does not change ( lift : multigrades --> xiSupportElements )
            {
                second->low_index -= cur_grade->num_cols;
                ++it;
            }
            else    //then move columns at cur_grade to some position in the equivalence class given by xiMatrixEntry second; lift map changes
            {
                xiMatrixEntry* target = second;
                int target_col = second->low_index - second->low_count;

                //if cur_grade comes before first_head, then cur_grade lifts to the LUB, and the columns should be associated with xiMatrixEntry second
                //otherwise, we must find re-compute the lift map for cur_grade and find the new position for the columns
                if(from_below && cur_grade->x <= second->x)
                {
                    while( (target->left != NULL) && (cur_grade->x <= target->left->x) )
                    {
                        target = target->left;
                        target_col -= target->low_count;
                    }
                }
                else if(!from_below && cur_grade->y <= second->y)
                {
                    while( (target->down != NULL) && (cur_grade->y <= target->down->y) )
                    {
                        target = target->down;
                        target_col -= target->low_count;
                    }
                }

                //associate cur_grade with target
                target->insert_multigrade(cur_grade, true);
                it = cur_entry->low_simplices.erase(it);    //NOTE: advances the iterator!!!

                //update column counts
                cur_entry->low_count -= cur_grade->num_cols;
                target->low_count += cur_grade->num_cols;

                //update equivalence class sizes
                first->low_class_size -= cur_grade->num_cols;
                second->low_class_size += cur_grade->num_cols;
            }

            //update column index
            low_col -= cur_grade->num_cols;
        }//end "low" simplex loop

        //move all "high" simplices for this xiMatrixEntry (start with rightmost column, end with leftmost)
        for(std::list<Multigrade*>::iterator it = cur_entry->high_simplices.begin(); it != cur_entry->high_simplices.end(); ) //NOTE: iterator advances in loop
        {
            Multigrade* cur_grade = *it;

            //remember current position of this grade
            cur_grade->simplex_index = high_col;

            if( (from_below && cur_grade->x > second->x && cur_grade->y <= first->y) || (!from_below && cur_grade->y > second->y && cur_grade->x <= first->x) )
                //then move columns at cur_grade past columns at xiMatrixEntry second; lift map does not change ( lift : multigrades --> xiSupportElements )
            {
                second->high_index -= cur_grade->num_cols;
                ++it;
            }
            else    //then move columns at cur_grade to some position in the equivalence class given by xiMatrixEntry second; lift map changes
            {
                xiMatrixEntry* target = second;
                int target_col = second->high_index - second->high_count;

                //if cur_grade comes before first_head, then cur_grade lifts to the LUB, and the columns should be associated with xiMatrixEntry second
                //otherwise, we must find re-compute the lift map for cur_grade and find the new position for the columns
                if(from_below && cur_grade->x <= second->x)
                {
                    while( (target->left != NULL) && (cur_grade->x <= target->left->x) )
                    {
                        target = target->left;
                        target_col -= target->high_count;
                    }
                }
                else if(!from_below && cur_grade->y <= second->y)
                {
                    while( (target->down != NULL) && (cur_grade->y <= target->down->y) )
                    {
                        target = target->down;
                        target_col -= target->high_count;
                    }
                }

                //associate cur_grade with target
                target->insert_multigrade(cur_grade, false);
                it = cur_entry->high_simplices.erase(it);    //NOTE: advances the iterator!!!

                //update column counts
                cur_entry->high_count -= cur_grade->num_cols;
                target->high_count += cur_grade->num_cols;

                //update equivalence class sizes
                first->high_class_size -= cur_grade->num_cols;
                second->high_class_size += cur_grade->num_cols;
            }

            //update column index
            high_col -= cur_grade->num_cols;
        }//end "low" simplex loop

        //advance to the next xiMatrixEntry in the first equivalence class
        cur_entry = from_below ? cur_entry->down : cur_entry->left;
    }//end while


  //STEP 2: traverse grades (backwards) in the new order and update the permutation vectors to reflect the new order on matrix columns

    //temporary data structures
    cur_entry = first;
    low_col = first->low_index;
    high_col = first->high_index;
    bool processing_first_block = true;

    //loop over xiMatrixEntrys that have simplices which move
    while(cur_entry != NULL)
    {
        //update positions of "low" simplices for this entry
        for(std::list<Multigrade*>::iterator it = cur_entry->low_simplices.begin(); it != cur_entry->low_simplices.end(); ++it)
        {
            Multigrade* cur_grade = *it;
            for(unsigned i=0; i<cur_grade->num_cols; i++)
            {
                //column currently in position (cur_grade->simplex_index - i) has new position low_col
                unsigned original_position = inv_perm_low[cur_grade->simplex_index - i];
                perm_low[original_position] = low_col;
                low_col--;
            }
        }

        //update positions of "high" simplices for this entry
        for(std::list<Multigrade*>::iterator it = cur_entry->high_simplices.begin(); it != cur_entry->high_simplices.end(); ++it)
        {
            Multigrade* cur_grade = *it;
            for(unsigned i=0; i<cur_grade->num_cols; i++)
            {
                //column currently in position (cur_grade->simplex_index - i) has new position high_col
                unsigned original_position = inv_perm_high[cur_grade->simplex_index - i];
                perm_high[original_position] = high_col;
                high_col--;
            }
        }

        //move to next entry
        if(processing_first_block)
        {
            cur_entry = from_below ? cur_entry->down : cur_entry->left;
            if(cur_entry == NULL)   //then reached end of the first block, so move to the second block
            {
                processing_first_block = false;
                cur_entry = second;
            }
        }
        else
            cur_entry = from_below ? cur_entry->left : cur_entry->down;
    }//end while

    //fix inverse permutation vectors -- is there a better way to do this?
    for(unsigned i=0; i < perm_low.size(); i++)
        inv_perm_low[perm_low[i]] = i;
    for(unsigned i=0; i < perm_high.size(); i++)
        inv_perm_high[perm_high[i]] = i;


  //STEP 3: re-build the matrix R based on the new order

    RL->rebuild(RL_initial, perm_low);
    RH->rebuild(RH_initial, perm_high, perm_low);


  //STEP 4: compute the new RU-decomposition

    ///TODO: should I avoid deleting and reallocating matrix U?
    delete UL;
    UL = RL->decompose_RU();
    delete UH;
    UH = RH->decompose_RU();

}//end update_order_and_reset_matrices()

//swaps two blocks of columns by using a quicksort to update the matrices, then fixing the RU-decomposition (Gaussian elimination on U followed by reduction of R)
void PersistenceUpdater::quicksort_and_reduce(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
{
    //update the lift map for all multigrades, storing the current column index for each multigrade


    //traverse grades in the new order and build the new order on matrix columns


    //quicksort the columns to put them in the new order, updating the permutation


    //re-build the matrix R based on the new


}//end quicksort_and_reduce()

//removes entries corresponding to xiMatrixEntry head from partition_low and partition_high
void PersistenceUpdater::remove_partition_entries(xiMatrixEntry* head)
{
    if(mesh->verbosity >= 9) { qDebug() << "    ----removing partition entries for xiMatrixEntry" << head->index << "(" << head->low_index << ";" << head->high_index << ")"; }

    //low simplices
    std::map<unsigned, xiMatrixEntry*>::iterator it1 = partition_low.find(head->low_index);
    if(it1 != partition_low.end() && it1->second == head)
        partition_low.erase(it1);

    //high simplices
    std::map<unsigned, xiMatrixEntry*>::iterator it2 = partition_high.find(head->high_index);
    if(it2 != partition_high.end() && it2->second == head)
        partition_high.erase(it2);

}//end remove_partition_entries()

//if the equivalence class corresponding to xiMatrixEntry head has nonempty sets of "low" or "high" simplices, then this function creates the appropriate entries in partition_low and partition_high
void PersistenceUpdater::add_partition_entries(xiMatrixEntry* head)
{
    if(mesh->verbosity >= 9) { qDebug() << "    ----adding partition entries for xiMatrixEntry" << head->index << "(" << head->low_index << ";" << head->high_index << ")"; }

    //low simplices
    if(head->low_class_size > 0)
        partition_low.insert( std::pair<unsigned, xiMatrixEntry*>(head->low_index, head) );

    //high simplices
    if(head->high_class_size > 0)
        partition_high.insert( std::pair<unsigned, xiMatrixEntry*>(head->high_index, head) );
}//end add_partition_entries()

//stores a barcode template in a 2-cell of the arrangement
///TODO: IMPROVE THIS!!! (store previous barcode at the simplicial level, and only examine columns that were modified in the recent update)
/// Is there a better way to handle endpoints at infinity?
void PersistenceUpdater::store_barcode_template(Face* cell, MapMatrix_Perm* RL, MapMatrix_Perm* RH)
{
    QDebug qd = qDebug().nospace();
    qd << "  -----barcode: ";

    //mark this cell as visited
    cell->mark_as_visited();

    //get a reference to the barcode template object
    BarcodeTemplate& dbc = cell->get_barcode();

    //loop over all zero-columns in matrix R_low
    for(unsigned c=0; c < RL->width(); c++)
    {
        if(RL->col_is_empty(c))  //then simplex corresponding to column c is positive
        {
            //find index of xi support point corresponding to simplex c
            unsigned a = ( (partition_low.lower_bound(c))->second )->index;

            //is simplex s paired?
            int s = RH->find_low(c);
            if(s != -1)  //then simplex c is paired with negative simplex s
            {
                //find index of xi support point corresponding to simplex s
                unsigned b = ( (partition_high.lower_bound(s) )->second)->index;



                if(a != b)  //then we have a bar of positive length
                {
                    qd << "(" << c << "," << s << ")-->(" << a << "," << b << ") ";
                    dbc.add_bar(a, b);
                }
            }
            else //then simplex c generates an essential cycle
            {
                qd << c << "-->" << a << " ";

                dbc.add_bar(a, -1);     //b = -1 = MAX_UNSIGNED indicates this is an essential cycle
            }
        }
    }
}//end store_barcode_template()


///TESTING ONLY
/// functions to check that D=RU
void PersistenceUpdater::check_low_matrix(MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL)
{
    bool err_low = false;
    for(unsigned row = 0; row < D_low->height(); row++)
    {
        for(unsigned col = 0; col < D_low->width(); col++)
        {
            bool temp = false;
            for(unsigned e = 0; e < D_low->width(); e++)
                temp = ( temp != (RL->entry(row, e) && UL->entry(e, col)) );
            if(temp != D_low->entry(row, col))
                err_low = true;
       }
    }
    if(err_low)
    {
        qDebug() << "====>>>> MATRIX ERROR (low) AT THIS STEP!";
//        qDebug() << "  Reduced matrix for low simplices:";
//        RL->print();
//        qDebug() << "  Matrix U for low simplices:";
//        UL->print();
//        qDebug() << "  Matrix D for low simplices:";
//        D_low->print();
    }
    else
        qDebug() << "low matrix ok";
}

void PersistenceUpdater::check_high_matrix(MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
{
    bool err_high = false;
    for(unsigned row = 0; row < D_high->height(); row++)
    {
        for(unsigned col = 0; col < D_high->width(); col++)
        {
            bool temp = false;
            for(unsigned e = 0; e < D_high->width(); e++)
                temp = ( temp != (RH->entry(row, e) && UH->entry(e, col)) );
            if(temp != D_high->entry(row, col))
                err_high = true;
       }
    }
    if(err_high)
        qDebug() << "====>>>> MATRIX ERROR (high) AT THIS STEP!\n";
    else
        qDebug() << "high matrix ok";
}

void PersistenceUpdater::print_perms(Perm& per, Perm& inv)
{
    QDebug qd = qDebug().nospace();
    qd << "  permutation: ";
    for(unsigned i=0; i<per.size(); i++)
        qd << per[i] << " ";
    qd << "\n  inverse permutation: ";
    for(unsigned i=0; i<inv.size(); i++)
        qd << inv[i] << " ";
}

void PersistenceUpdater::print_high_partition()
{
    QDebug qd = qDebug().nospace();
    qd << "  high partition: ";
    for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_high.begin(); it != partition_high.end(); ++it)
        qd << it->first << "->" << it->second->index << ", ";
}
