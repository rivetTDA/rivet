#include "persistence_updater.h"

#include "index_matrix.h"
#include "map_matrix.h"
#include "multi_betti.h"
#include "simplex_tree.h"
#include "../computationthread.h"
#include "../dcel/anchor.h"
#include "../dcel/barcode_template.h"
#include "../dcel/dcel.h"
#include "../dcel/mesh.h"

#include <QDebug>
#include <QTime>    //NOTE: QTime::elapsed() wraps to zero 24 hours after the last call to start() or restart()

#include <stdlib.h> //for rand()


//constructor for when we must compute all of the barcode templates
PersistenceUpdater::PersistenceUpdater(Mesh *m, SimplexTree* b, std::vector<xiPoint> &xi_pts) :
    mesh(m), bifiltration(b), dim(b->hom_dim),
    xi_matrix(m->x_grades.size(), m->y_grades.size()),
    testing(false)
{
    //fill the xiSupportMatrix with the xi support points and anchors
    //  also stores the anchors in xi_pts and in the Mesh
    xi_matrix.fill_and_find_anchors(xi_pts, m);
}

//constructor for when we load the pre-computed barcode templates from a RIVET data file
PersistenceUpdater::PersistenceUpdater(Mesh* m, std::vector<xiPoint>& xi_pts) :
    mesh(m),
    xi_matrix(m->x_grades.size(), m->y_grades.size()),
    testing(false)
{
    //fill the xiSupportMatrix with the xi support points
    xi_matrix.fill_and_find_anchors(xi_pts, m);
}

//computes and stores a barcode template in each 2-cell of mesh
//resets the matrices and does a standard persistence calculation for expensive crossings
void PersistenceUpdater::store_barcodes_with_reset(std::vector<Halfedge*>& path, ComputationThread* cthread)
{
    QTime timer;    //for timing the computations

  // PART 1: GET THE BOUNDARY MATRICES WITH PROPER SIMPLEX ORDERING

    timer.start();

    //initialize the lift map from simplex grades to LUB-indexes
    if(mesh->verbosity >= 6) { qDebug() << "  Mapping low simplices:"; }
    IndexMatrix* ind_low = bifiltration->get_index_mx(dim);    //can we improve this with something more efficient than IndexMatrix?
    store_multigrades(ind_low, true);

    if(mesh->verbosity >= 6) { qDebug() << "  Mapping high simplices:"; }
    IndexMatrix* ind_high = bifiltration->get_index_mx(dim + 1);    //again, could be improved?
    store_multigrades(ind_high, false);

    //get the proper simplex ordering
    std::vector<int> low_simplex_order;     //this will be a map : dim_index --> order_index for dim-simplices; -1 indicates simplices not in the order
    unsigned num_low_simplices = build_simplex_order(ind_low, true, low_simplex_order);
    delete ind_low;

    std::vector<int> high_simplex_order;     //this will be a map : dim_index --> order_index for (dim+1)-simplices; -1 indicates simplices not in the order
    unsigned num_high_simplices = build_simplex_order(ind_high, false, high_simplex_order);
    delete ind_high;

    //get boundary matrices (R) and identity matrices (U) for RU-decomposition
    R_low = bifiltration->get_boundary_mx(low_simplex_order, num_low_simplices);
    R_high = bifiltration->get_boundary_mx(low_simplex_order, num_low_simplices, high_simplex_order, num_high_simplices);

    //print runtime data
    qDebug() << "  --> computing initial order on simplices and building the boundary matrices took" << timer.elapsed() << "milliseconds";

    //copy the boundary matrices (R) for fast reset later
    timer.start();
    MapMatrix_Perm* R_low_initial = new MapMatrix_Perm(*R_low);
    MapMatrix_Perm* R_high_initial = new MapMatrix_Perm(*R_high);
    qDebug() << "  --> copying the boundary matrices took" << timer.elapsed() << "milliseconds";

    //initialize the permutation vectors
    perm_low.resize(R_low->width());
    inv_perm_low.resize(R_low->width());
    perm_high.resize(R_high->width());
    inv_perm_high.resize(R_high->width());
    for(unsigned j = 0; j < perm_low.size(); j++)
    {
        perm_low[j] = j;
        inv_perm_low[j] = j;
    }
    for(unsigned j = 0; j < perm_high.size(); j++)
    {
        perm_high[j] = j;
        inv_perm_high[j] = j;
    }


  // PART 2: INITIAL PERSISTENCE COMPUTATION (RU-decomposition)

    timer.start();

    //initial RU-decomposition
    U_low = R_low->decompose_RU();
    U_high = R_high->decompose_RU();

    int time_for_initial_decomp = timer.elapsed();
    qDebug() << "  --> computing the RU decomposition took" << time_for_initial_decomp << "milliseconds";

    //store the barcode template in the first cell
    Face* first_cell = mesh->topleft->get_twin()->get_face();
    store_barcode_template(first_cell);

    qDebug() << "Initial persistence computation in cell " << mesh->FID(first_cell);
//    print_perms(perm_high, inv_perm_high);


  // PART 3: TRAVERSE THE PATH AND UPDATE PERSISTENCE AT EACH STEP

    qDebug() << "TRAVERSING THE PATH USING THE RESET ALGORITHM: path has" << path.size() << "steps";
    qDebug() << "                              ^^^^^^^^^^^^^^^";

    // choose the initial value of the threshold intelligently
    unsigned long threshold = choose_initial_threshold(time_for_initial_decomp);   //if the number of swaps might exceed this threshold, then do a persistence calculation from scratch
    qDebug() << "initial reset threshold set to" << threshold;

    timer.start();

    //data structures for analyzing the computation
    unsigned long total_transpositions = 0;
    unsigned total_time_for_transpositions = 0; //NEW
    unsigned number_of_resets = 1;  //we count the initial RU-decomposition as the first reset
    unsigned total_time_for_resets = time_for_initial_decomp;
    int max_time = 0;

    //traverse the path
    QTime steptimer;
    for(unsigned i=0; i<path.size(); i++)
    {
        cthread->setCurrentProgress(i);    //update progress bar

        steptimer.start();                //time update at each step of the path
        unsigned long num_trans = 0;      //count of how many transpositions we will have to do if we do vineyard updates
        unsigned long swap_counter = 0;   //count of how many transpositions we actually do

        //determine which anchor is represented by this edge
        Anchor* cur_anchor = (path[i])->get_anchor();
        xiMatrixEntry* at_anchor = cur_anchor->get_entry();

        //get equivalence classes for this anchor
        xiMatrixEntry* down = at_anchor->down;
        xiMatrixEntry* left = at_anchor->left;

        //if this is a strict anchor, then swap simplices
        if(down != NULL && left != NULL) //then this is a strict anchor and some simplices swap
        {
            qDebug().nospace() << "  step " << i << " of path: crossing (strict) anchor at ("<< cur_anchor->get_x() << ", " << cur_anchor->get_y() << ") into cell " << mesh->FID((path[i])->get_face()) << "; edge weight: " << cur_anchor->get_weight();

            //find out how many transpositions we will have to process if we do vineyard updates
            num_trans = count_transpositions(at_anchor, cur_anchor->is_above());

            if(cur_anchor->is_above()) //then the anchor is crossed from below to above
            {
                remove_lift_entries(at_anchor);        //this block of the partition might become empty
                remove_lift_entries(down);             //this block of the partition will move

                if(num_trans < threshold)   //then do vineyard updates
                {
                    swap_counter += split_grade_lists(at_anchor, left, true);   //move grades that come before left from anchor to left -- vineyard updates
                    swap_counter += move_columns(down, left, true);             //swaps blocks of columns at down and at left -- vineyard updates
                }
                else    //then reset the matrices
                {
                    split_grade_lists_no_vineyards(at_anchor, left, true);   //only updates the xiSupportMatrix and permutation vectors; no vineyard updates
                    update_order_and_reset_matrices(down, left, true, R_low_initial, R_high_initial);   //recompute the RU-decomposition
                }

                merge_grade_lists(at_anchor, down);     //move all grades from down to anchor
                add_lift_entries(at_anchor);       //this block of the partition might have previously been empty
                add_lift_entries(left);            //this block of the partition moved
            }
            else    //then anchor is crossed from above to below
            {
                remove_lift_entries(at_anchor);        //this block of the partition might become empty
                remove_lift_entries(left);             //this block of the partition will move

                if(num_trans < threshold)   //then do vineyard updates
                {
                    swap_counter += split_grade_lists(at_anchor, down, false);   //move grades that come before left from anchor to left -- vineyard updates
                    swap_counter += move_columns(left, down, false);             //swaps blocks of columns at down and at left -- vineyard updates
                }
                else    //then reset the matrices
                {
                    split_grade_lists_no_vineyards(at_anchor, down, false);  //only updates the xiSupportMatrix and permutation vectors; no vineyard updates
                    update_order_and_reset_matrices(left, down, false, R_low_initial, R_high_initial);  //recompute the RU-decomposition
                }

                merge_grade_lists(at_anchor, left);     //move all grades from down to anchor
                add_lift_entries(at_anchor);       //this block of the partition might have previously been empty
                add_lift_entries(down);            //this block of the partition moved
           }
        }
        else    //this is a non-strict anchor, and we just have to split or merge equivalence classes
        {
            qDebug().nospace() << "  step " << i << " of path: crossing (non-strict) anchor at ("<< cur_anchor->get_x() << ", " << cur_anchor->get_y() << ") into cell " << mesh->FID((path[i])->get_face()) << "; edge weight: " << cur_anchor->get_weight();

            xiMatrixEntry* generator = at_anchor->down;
            if(generator == NULL)
                generator = at_anchor->left;

            if((cur_anchor->is_above() && generator == at_anchor->down) || (!cur_anchor->is_above() && generator == at_anchor->left))
                //then merge classes -- there will never be any transpositions in this case
            {
                remove_lift_entries(generator);
                merge_grade_lists(at_anchor, generator);
                add_lift_entries(at_anchor);  //this is necessary in case the class was previously empty
            }
            else    //then split classes
            {
                //find out how many transpositions we will have to process if we do vineyard updates
                unsigned junk = 0;
                bool horiz = (generator == at_anchor->left);
                count_transpositions_from_separations(at_anchor, generator, horiz, true, num_trans, junk);
                count_transpositions_from_separations(at_anchor, generator, horiz, false, num_trans, junk);

                //now do the updates
                remove_lift_entries(at_anchor);   //this is necessary because the class corresponding to at_anchor might become empty

                if(num_trans < threshold)   //then do vineyard updates
                    swap_counter += split_grade_lists(at_anchor, generator, horiz);
                else    //then reset the matrices
                {
                    split_grade_lists_no_vineyards(at_anchor, generator, horiz);   //only updates the xiSupportMatrix; no vineyard updates
                    update_order_and_reset_matrices(at_anchor, generator, R_low_initial, R_high_initial);   //recompute the RU-decomposition
                }

                add_lift_entries(at_anchor);
                add_lift_entries(generator);
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
            store_barcode_template(cur_face);

        //print/store data for analysis
        int step_time = steptimer.elapsed();        

        if(num_trans < threshold)   //then we did vineyard-updates
        {
            qDebug() << "    --> this step took" << step_time << "milliseconds and involved" << swap_counter << "transpositions; estimate was" << num_trans;
            if(swap_counter != num_trans)
                qDebug() << "    ========>>> ERROR: transposition count doesn't match estimate!";
            total_transpositions += swap_counter;
            total_time_for_transpositions += step_time;
        }
        else
        {
            qDebug() << "    --> this step took" << step_time << "milliseconds; reset matrices to avoid" << num_trans << "transpositions";
            if(swap_counter > 0)
                qDebug() << "    ========>>> ERROR: swaps occurred on a matrix reset!";
            number_of_resets++;
            total_time_for_resets += step_time;
        }

        if(step_time > max_time)
            max_time = step_time;

        //update the treshold
        if(total_time_for_transpositions > 0 && total_transpositions > total_time_for_transpositions)
        {
            threshold = (unsigned long) ((double) total_transpositions / (double) total_time_for_transpositions)*(total_time_for_resets/number_of_resets);
            qDebug() << "       new threshold:" << threshold;
        }
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

//function to set the "edge weights" for each anchor line
void PersistenceUpdater::set_anchor_weights(std::vector<Halfedge*>& path)
{
  // PART 1: GET THE PROPER SIMPLEX ORDERING

    //initialize the lift map from simplex grades to LUB-indexes
    if(mesh->verbosity >= 6) { qDebug() << "  Mapping low simplices:"; }
    IndexMatrix* ind_low = bifiltration->get_index_mx(dim);    //can we improve this with something more efficient than IndexMatrix?
    store_multigrades(ind_low, true);
    delete ind_low;

    if(mesh->verbosity >= 6) { qDebug() << "  Mapping high simplices:"; }
    IndexMatrix* ind_high = bifiltration->get_index_mx(dim + 1);    //again, could be improved?
    store_multigrades(ind_high, false);
    delete ind_high;


  // PART 2: TRAVERSE THE PATH AND COUNT SWITCHES & SEPARATIONS AT EACH STEP

    for(unsigned i=0; i<path.size(); i++)
    {
      unsigned long switches = 0;
      unsigned long separations = 0;

      //determine which anchor is represented by this edge
      Anchor* cur_anchor = (path[i])->get_anchor();
      xiMatrixEntry* at_anchor = cur_anchor->get_entry();

      qDebug() << "  step" << i << "of the short path: crossing anchor at ("<< cur_anchor->get_x() << "," << cur_anchor->get_y() << ") into cell" << mesh->FID((path[i])->get_face());

      //if this is a strict anchor, then there can be switches and separations
      if(at_anchor->down != NULL && at_anchor->left != NULL) //then this is a strict anchor
      {
          count_switches_and_separations(at_anchor, cur_anchor->is_above(), switches, separations);
      }
      else    //this is a non-strict anchor, so there can be separations but not switches
      {
          xiMatrixEntry* generator = (at_anchor->down != NULL) ? at_anchor->down : at_anchor->left;

          if((cur_anchor->is_above() && generator == at_anchor->down) || (!cur_anchor->is_above() && generator == at_anchor->left))
              //then merge classes
          {
              separations += generator->low_count * at_anchor->low_count + generator->high_count * at_anchor->high_count;
              merge_grade_lists(at_anchor, generator);
          }
          else    //then split classes
          {
              do_separations(at_anchor, generator, (at_anchor->y == generator->y));
              separations += generator->low_count * at_anchor->low_count + generator->high_count * at_anchor->high_count;
          }
      }

      //store data
      cur_anchor->set_weight(switches + separations/4);     //we expect that each separation produces a transposition about 25% of the time

      qDebug() << "     edge weight:" << cur_anchor->get_weight() << "; (" << switches << "," << separations << ")";

    }//end path traversal
}//end set_anchor_weights()

//function to clear the levelset lists -- e.g., following the edge-weight calculation
void PersistenceUpdater::clear_levelsets()
{
    xi_matrix.clear_grade_lists();
}

//stores multigrade info for the persistence computations (data structures prepared with respect to a near-vertical line positioned to the right of all \xi support points)
//  that is, this function creates the level sets of the lift map
//  low is true for simplices of dimension hom_dim, false for simplices of dimension hom_dim+1
//NOTE: this function has been updated for the new (unfactored) lift map of August 2015
void PersistenceUpdater::store_multigrades(IndexMatrix* ind, bool low)
{
    if(mesh->verbosity >= 6) { qDebug() << "STORING MULTIGRADES: low =" << low; }

    //initialize linked list to track the "frontier"
    typedef std::list<xiMatrixEntry*> Frontier;
    Frontier frontier;

    //loop through rows of xiSupportMatrix, from top to bottom
    for(unsigned y = ind->height(); y-- > 0; )  //y counts down from (ind->height() - 1) to 0
    {
        //update the frontier for row y:
        //  if the last element of frontier has the same x-coord as cur, then replace that element with cur
        //  otherwise, append cur to the end of frontier
        xiMatrixEntry* cur = xi_matrix.get_row(y);
        if(cur != NULL)
        {
            Frontier::iterator it = frontier.end();  //the past-the-end element of frontier
            if(it != frontier.begin())  //then the frontier is not empty
            {
                --it;  //the last element of frontier
                if((*it)->x == cur->x)  //then erase the last element of frontier
                    frontier.erase(it);
            }

            //append cur to the end of the frontier
            frontier.push_back(cur);
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

            //if there are any simplices at (x,y),
            //    and if x is not greater than the x-coordinate of the rightmost element of the frontier,
            //    then map multigrade (x,y) to the last element of the frontier such that x <= (*it)->x
            if(last_col > first_col && it != frontier.end() && x <= (*it)->x )
            {
                //advance the iterator to the first element of the frontier such that (*it)->x < x
                while( it != frontier.end() && (*it)->x >= x )
                    ++it;

                //back up one position, to the last element of the frontier such that (*it)->x >= x
                --it;

                //now map the multigrade to the xi support entry
                (*it)->add_multigrade(x, y, last_col - first_col, last_col, low);

                if(mesh->verbosity >= 6) { qDebug() << "    simplices at (" << x << "," << y << "), in columns" << (first_col + 1) << "to" << last_col << ", mapped to xiMatrixEntry at (" << (*it)->x << ", " << (*it)->y << ")"; }
            }
        }//end x loop
    }//end y loop
}//end store_multigrades()

//finds the proper order of simplexes for the persistence calculation (with respect to a near-vertical line positioned to the right of all \xi support points)
//  NOTE: within each equivalence class, multigrades will occur in lexicographical order
//  PARAMETERS:
//    low is true for simplices of dimension hom_dim, false for simplices of dimension hom_dim+1
//    simplex_order will be filled with a map : dim_index --> order_index for simplices of the given dimension
//           If a simplex with dim_index i does not appear in the order (i.e. its grade is not less than the LUB of all xi support points), then simplex_order[i] = -1.
//  RETURN VALUE: the number of simplices in the order
unsigned PersistenceUpdater::build_simplex_order(IndexMatrix* ind, bool low, std::vector<int>& simplex_order)
{
    //count the number of simplices that will be in the order (i.e. simplices with grades less than the LUB of all xi support points)
    unsigned num_simplices = 0;
    for(unsigned row = 0; row < xi_matrix.height(); row++)
    {
        xiMatrixEntry* cur = xi_matrix.get_row(row);
        if(cur == NULL)
            continue;
        std::list<Multigrade*>* mgrades = (low) ? &(cur->low_simplices) : &(cur->high_simplices);
        for(std::list<Multigrade*>::iterator it = mgrades->begin(); it != mgrades->end(); ++it)
        {
            num_simplices += (*it)->num_cols;
        }
    }

    //we will create the map starting by identifying the order index of each simplex, starting with the last simplex
    int o_index = num_simplices - 1;

    //prepare the vector
    simplex_order.clear();
    simplex_order.resize(ind->last() + 1, -1);  //all entries -1 by default

    //consider the rightmost xiMatrixEntry in each row
    for(unsigned row = xi_matrix.height(); row-- > 0; )  //row counts down from (xi_matrix->height() - 1) to 0
    {
        xiMatrixEntry* cur = xi_matrix.get_row(row);
        if(cur == NULL)
            continue;

        if(mesh->verbosity >= 6) { qDebug() << "----xiMatrixEntry (" << cur->x << "," << cur->y << ")"; }

        //store index of rightmost column that is mapped to this equivalence class
        int* cur_ind = (low) ? &(cur->low_index) : &(cur->high_index);
        *cur_ind = o_index;

        //get the multigrade list for this xiMatrixEntry
        std::list<Multigrade*>* mgrades = (low) ? &(cur->low_simplices) : &(cur->high_simplices);

        //sort the multigrades in lexicographical order
        mgrades->sort(Multigrade::LexComparator);

        //store map values for all simplices at these multigrades
        for(std::list<Multigrade*>::iterator it = mgrades->begin(); it != mgrades->end(); ++it)
        {
            Multigrade* mg = *it;
            if(mesh->verbosity >= 6) { qDebug() << "  multigrade (" << mg->x << "," << mg->y << ") has" << mg->num_cols << "simplices with last dim_index" << mg->simplex_index << "which will map to order_index" << o_index; }

            for(unsigned s=0; s < mg->num_cols; s++)  // simplex with dim_index (mg->simplex_index - s) has order_index o_index
            {
                simplex_order[mg->simplex_index - s] = o_index;
                o_index--;
            }
        }

        //if any simplices of the specified dimension were mapped to this equivalence class, then store information about this class
        if(*cur_ind != o_index)
        {
            if(low)
                lift_low.insert( std::pair<unsigned, xiMatrixEntry*>(*cur_ind, cur) );
            else
                lift_high.insert( std::pair<unsigned, xiMatrixEntry*>(*cur_ind, cur) );
        }
    }//end for(row > 0)

    return num_simplices;
}//end build_simplex_order()

//counts the number of transpositions that will happen if we cross an anchor and do vineyeard-updates
//this function DOES NOT MODIFY the xiSupportMatrix
unsigned long PersistenceUpdater::count_transpositions(xiMatrixEntry* anchor, bool from_below)
{
    //identify entries
    xiMatrixEntry* first = from_below ? anchor->down : anchor->left;
    xiMatrixEntry* second = from_below ? anchor->left : anchor->down;
    xiMatrixEntry* temp = new xiMatrixEntry(anchor->left->x, anchor->down->y);

    //counters
    unsigned long count = 0;
    unsigned first_simplices_low = 0;
    unsigned first_simplices_high = 0;
    unsigned second_simplices_low = 0;
    unsigned second_simplices_high = 0;

    //count transpositions that occur when we separate out the grades that lift to the anchor from those that lift to second
    count_transpositions_from_separations(anchor, second, from_below, true, count, second_simplices_low);
    count_transpositions_from_separations(anchor, second, from_below, false, count, second_simplices_high);

    //count transpositions that occur when we separate out the grades that lift to GLB(first, second) from those that lift to first
    unsigned temp_simplices = 0;
    count_transpositions_from_separations(first, temp, from_below, true, count, temp_simplices);
    first_simplices_low = first->low_count - temp_simplices;
    temp_simplices = 0;
    count_transpositions_from_separations(first, temp, from_below, false, count, temp_simplices);
    first_simplices_high = first->high_count - temp_simplices;

    //count switches
    count += ((unsigned long) first_simplices_low) * ((unsigned long) second_simplices_low);
    count += ((unsigned long) first_simplices_high) * ((unsigned long) second_simplices_high);

    return count;
}//end count_transpositions()

//counts the number of transpositions that result from separations
//this function DOES NOT MODIFY the xiSupportMatrix
void PersistenceUpdater::count_transpositions_from_separations(xiMatrixEntry* greater, xiMatrixEntry* lesser, bool horiz, bool low, unsigned long& count_trans, unsigned& count_lesser)
{
    int gr_col = greater->low_index;
    int cur_col = gr_col;
    std::list<Multigrade*> grades = low ? greater->low_simplices : greater->high_simplices;
    for(std::list<Multigrade*>::iterator it = grades.begin(); it != grades.end(); ++it)
    {
        Multigrade* cur_grade = *it;
        if((horiz && cur_grade->x > lesser->x) || (!horiz && cur_grade->y > lesser->y))   //then there will be transpositions from separations
        {
            count_trans += cur_grade->num_cols * (gr_col - cur_col);
            gr_col -= cur_grade->num_cols;
        }
        else
            count_lesser += cur_grade->num_cols;
        cur_col -= cur_grade->num_cols;
    }
}//end count_transpositions_from_separations()

//moves grades associated with xiMatrixEntry greater, that come before xiMatrixEntry lesser in R^2, so that they become associated with lesser
//  precondition: no grades lift to lesser (it has empty level sets under the lift map)
unsigned long PersistenceUpdater::split_grade_lists(xiMatrixEntry* greater, xiMatrixEntry* lesser, bool horiz)
{
    unsigned long swap_counter = 0;

    //low simplices
    int gr_col = greater->low_index;
    int cur_col = gr_col;
    std::list<Multigrade*> grades = greater->low_simplices;
    greater->low_simplices.clear();
    for(std::list<Multigrade*>::iterator it = grades.begin(); it != grades.end(); ++it)
    {
        Multigrade* cur_grade = *it;
        if((horiz && cur_grade->x > lesser->x) || (!horiz && cur_grade->y > lesser->y))  //then this grade lifts to greater, so move columns to the right
        {
            if(cur_col != gr_col)   //then we must move the columns
                swap_counter += move_low_columns(cur_col, cur_grade->num_cols, gr_col);

            greater->low_simplices.push_back(cur_grade);
            gr_col -= cur_grade->num_cols;
        }
        else    //then this grade lifts to lesser, so update lift map, but no need to move columns
            lesser->low_simplices.push_back(cur_grade);

        cur_col -= cur_grade->num_cols;
    }
    lesser->low_index = gr_col;
    lesser->low_count = gr_col - cur_col;
    greater->low_count = greater->low_index - lesser->low_index;

    //high simplices
    gr_col = greater->high_index;
    cur_col = gr_col;
    grades = greater->high_simplices;
    greater->high_simplices.clear();
    for(std::list<Multigrade*>::iterator it = grades.begin(); it != grades.end(); ++it)
    {
        Multigrade* cur_grade = *it;
        if((horiz && cur_grade->x > lesser->x) || (!horiz && cur_grade->y > lesser->y))  //then this grade lifts to greater, so move columns to the right
        {
            if(cur_col != gr_col)   //then we must move the columns
                swap_counter += move_high_columns(cur_col, cur_grade->num_cols, gr_col);

            greater->high_simplices.push_back(cur_grade);
            gr_col -= cur_grade->num_cols;
        }
        else    //then this grade lifts to lesser, so update lift map, but no need to move columns
            lesser->high_simplices.push_back(cur_grade);

        cur_col -= cur_grade->num_cols;
    }
    lesser->high_index = gr_col;
    lesser->high_count = gr_col - cur_col;
    greater->high_count = greater->high_index - lesser->high_index;

    return swap_counter;
}//end split_grade_lists()

//splits grade lists and updates the permutation vectors, but does NOT do vineyard updates
void PersistenceUpdater::split_grade_lists_no_vineyards(xiMatrixEntry* greater, xiMatrixEntry* lesser, bool horiz)
{
  //STEP 1: update the lift map for all multigrades and store the current column index for each multigrade

    //first, low simpilices
    int gr_col = greater->low_index;
    int cur_col = gr_col;
    std::list<Multigrade*> grades = greater->low_simplices;
    greater->low_simplices.clear();       ///this isn't so efficient...
    for(std::list<Multigrade*>::iterator it = grades.begin(); it != grades.end(); ++it)
    {
        Multigrade* cur_grade = *it;
        cur_grade->simplex_index = cur_col;
        if((horiz && cur_grade->x > lesser->x) || (!horiz && cur_grade->y > lesser->y))  //then this grade lifts to greater
        {
            greater->low_simplices.push_back(cur_grade);
            gr_col -= cur_grade->num_cols;
        }
        else    //then this grade lifts to lesser
            lesser->low_simplices.push_back(cur_grade);

        cur_col -= cur_grade->num_cols;
    }
    lesser->low_index = gr_col;
    lesser->low_count = gr_col - cur_col;
    greater->low_count = greater->low_index - lesser->low_index;

    //now high simplices
    gr_col = greater->high_index;
    cur_col = gr_col;
    std::list<Multigrade*> grades_h = greater->high_simplices;
    greater->high_simplices.clear();       ///this isn't so efficient...
    for(std::list<Multigrade*>::iterator it = grades_h.begin(); it != grades_h.end(); ++it)
    {
        Multigrade* cur_grade = *it;
        cur_grade->simplex_index = cur_col;
        if((horiz && cur_grade->x > lesser->x) || (!horiz && cur_grade->y > lesser->y))  //then this grade lifts to greater
        {
            greater->high_simplices.push_back(cur_grade);
            gr_col -= cur_grade->num_cols;
        }
        else    //then this grade lifts to lesser
            lesser->high_simplices.push_back(cur_grade);

        cur_col -= cur_grade->num_cols;
    }
    lesser->high_index = gr_col;
    lesser->high_count = gr_col - cur_col;
    greater->high_count = greater->high_index - lesser->high_index;

  //STEP 2: traverse grades (backwards) in the new order and update the permutation vectors to reflect the new order on matrix columns

    //temporary data structures
    xiMatrixEntry* cur_entry = greater;
    int low_col = cur_entry->low_index;
    int high_col = cur_entry->high_index;

    //loop over xiMatrixEntrys
    while(true) //loop ends with break statement
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
        if(cur_entry == greater)
            cur_entry = lesser;
        else
            break;
    }//end while

    //fix inverse permutation vectors -- is there a better way to do this?
    for(unsigned i=0; i < perm_low.size(); i++)
        inv_perm_low[perm_low[i]] = i;
    for(unsigned i=0; i < perm_high.size(); i++)
        inv_perm_high[perm_high[i]] = i;

}//end split_grade_lists_no_vineyards()

//moves all grades associated with xiMatrixEntry lesser so that they become associated with xiMatrixEntry greater
void PersistenceUpdater::merge_grade_lists(xiMatrixEntry* greater, xiMatrixEntry* lesser)
{
    //low simplices
    greater->low_simplices.splice(greater->low_simplices.end(), lesser->low_simplices);
    greater->low_count += lesser->low_count;
    lesser->low_count = 0;

    //high simplices
    greater->high_simplices.splice(greater->high_simplices.end(), lesser->high_simplices);
    greater->high_count += lesser->high_count;
    lesser->high_count = 0;
}//end merge_grade_lists()

//moves columns from an equivalence class given by xiMatrixEntry* first to their new positions after or among the columns in the equivalence class given by xiMatrixEntry* second
// the boolean argument indicates whether an anchor is being crossed from below (or from above)
// this version updates the permutation vectors required for the "reset" approach
///NOTE: this function has been updated for the July/August 2015 bug fix
unsigned long PersistenceUpdater::move_columns(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below)
{
    ///DEBUGGING
    if(first->low_index + second->low_count != second->low_index || first->high_index + second->high_count != second->high_index)
    {
        qDebug() << "  ===>>> ERROR: swapping non-consecutive column blocks!";
    }

    //get column indexes (so we know which columns to move)
    int low_col = first->low_index;   //rightmost column index of low simplices for the block that moves
    int high_col = first->high_index; //rightmost column index of high simplices for the block that moves

    //set column indexes for the first class to their final position
    first->low_index = second->low_index;
    first->high_index = second->high_index;

    //initialize counter
    unsigned long swap_counter = 0;

    //move all "low" simplices for xiMatrixEntry first (start with rightmost column, end with leftmost)
    for(std::list<Multigrade*>::iterator it = first->low_simplices.begin(); it != first->low_simplices.end(); ) //NOTE: iterator advances in loop
    {
        Multigrade* cur_grade = *it;

        if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
            //then move columns at cur_grade past columns at xiMatrixEntry second; lift map does not change ( lift : multigrades --> xiSupportElements )
        {
            swap_counter += move_low_columns(low_col, cur_grade->num_cols, second->low_index);
            second->low_index -= cur_grade->num_cols;
            ++it;
        }
        else    //then cur_grade now lifts to xiMatrixEntry second; columns don't move
        {
            //associate cur_grade with second
            second->insert_multigrade(cur_grade, true);
            it = first->low_simplices.erase(it);    //NOTE: advances the iterator!!!

            //update column counts
            first->low_count -= cur_grade->num_cols;
            second->low_count += cur_grade->num_cols;
        }

        //update column index
        low_col -= cur_grade->num_cols;
    }//end "low" simplex loop

    //move all "high" simplices for xiMatrixEntry first (start with rightmost column, end with leftmost)
    for(std::list<Multigrade*>::iterator it = first->high_simplices.begin(); it != first->high_simplices.end(); ) //NOTE: iterator advances in loop
    {
        Multigrade* cur_grade = *it;

//        qDebug() << "  ====>>>> moving high simplices at grade (" << cur_grade->x << "," << cur_grade->y << ")";

        if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
            //then move columns at cur_grade past columns at xiMatrixEntry second; lift map does not change ( lift : multigrades --> xiSupportElements )
        {
            swap_counter += move_high_columns(high_col, cur_grade->num_cols, second->high_index);
            second->high_index -= cur_grade->num_cols;
            ++it;
        }
        else    //then cur_grade now lifts to xiMatrixEntry second; columns don't move
        {
//            qDebug() << "====>>>> simplex at (" << cur_grade->x << "," << cur_grade->y << ") now lifts to (" << second->x << "," << second->y << ")";

            //associate cur_grade with second
            second->insert_multigrade(cur_grade, false);
            it = first->high_simplices.erase(it);    //NOTE: advances the iterator!!!

            //update column counts
            first->high_count -= cur_grade->num_cols;
            second->high_count += cur_grade->num_cols;
        }

        //update column index
        high_col -= cur_grade->num_cols;
    }//end "high" simplex loop

    ///DEBUGGING
    if(second->low_index + first->low_count != first->low_index || second->high_index + first->high_count != first->high_index)
    {
        qDebug() << "  ===>>> ERROR: swap resulted in non-consecutive column blocks!";
    }
    return swap_counter;
}//end move_columns()

//moves a block of n columns, the rightmost of which is column s, to a new position following column t (NOTE: assumes s <= t)
// this version maintains the permutation arrays required for the "reset" approach
unsigned long PersistenceUpdater::move_low_columns(int s, unsigned n, int t)
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
            unsigned a = i - c;
            unsigned b = a + 1;

            //update the permutation vectors
            unsigned s = inv_perm_low[a];
            unsigned t = inv_perm_low[b];
            inv_perm_low[a] = t;
            inv_perm_low[b] = s;
            perm_low[t] = a;
            perm_low[s] = b;

            //now for the vineyards algorithm
            vineyard_update_low(a);

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
unsigned long PersistenceUpdater::move_high_columns(int s, unsigned n, int t)
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
            unsigned a = i - c;
            unsigned b = a + 1;

            //update the permutation vectors
            unsigned s = inv_perm_high[a];
            unsigned t = inv_perm_high[b];
            inv_perm_high[a] = t;
            inv_perm_high[b] = s;
            perm_high[t] = a;
            perm_high[s] = b;

            //now for the vineyards algorithm
            vineyard_update_high(a);

            /// TESTING ONLY - FOR CHECKING THAT D=RU
            if(testing)
            {
                D_high->swap_columns(a, false);
            }
        }//end for(i=...)
    }//end for(c=...)

    return n*(t-s);
}//end move_high_columns()

//performs a vineyard update corresponding to the transposition of columns a and (a + 1)
//  for LOW simplices
void PersistenceUpdater::vineyard_update_low(unsigned a)
{
    unsigned b = a + 1;

    bool a_pos = (R_low->low(a) == -1);    //true iff simplex corresponding to column a is positive
    bool b_pos = (R_low->low(b) == -1);    //true iff simplex corresponding to column b=a+1 is positive

    if(a_pos)  //simplex a is positive (Vineyards paper - Cases 1 and 4)
    {
        if(b_pos)   //simplex b is positive (Case 1)
        {
            //look for columns k and l in RH with low(k)=a, low(l)=b, and RH(a,l)=1 -- if these exist, then we must fix matrix RH following row/column swaps (Case 1.1)
            int k = R_high->find_low(a);
            int l = R_high->find_low(b);
            bool RHal = (l > -1 && R_high->entry(a, l));  //entry (a,l) in matrix RH

            //ensure that UL[a,b]=0
            U_low->clear(a, b);

            //transpose rows and columns (don't need to swap columns of RL, because these columns are zero)
            U_low->swap_columns(a);
            U_low->swap_rows(a);

            //swap rows, and fix RH if necessary
            if(k > -1 && RHal)  //case 1.1
            {
                if(k < l)
                {
                    R_high->swap_rows(a, true);  //in this case, low entries change
                    R_high->add_column(k, l);
                    U_high->add_row(l, k);
                }
                else
                {
                    R_high->swap_rows(a, false);  //in this case, low entries do not change
                    R_high->add_column(l, k);
                    U_high->add_row(k, l);
                }
            }
            else
                R_high->swap_rows(a, !RHal);  //in this case, only necessary to update low entries if RH(a,l)=0 or if column l does not exist
        }
        else    //simplex b is negative (Case 4)
        {
            //ensure that UL[a,b]=0
            U_low->clear(a, b);

            //transpose rows and columns and update low arrays
            R_low->swap_columns(a, true);
            R_high->swap_rows(a, true);
            U_low->swap_columns(a);
            U_low->swap_rows(a);
        }
    }
    else    //simplex a is negative (Vineyards paper - Cases 2 and 3)
    {
        if(b_pos)   //simplex b is positive (Case 3)
        {
            //look for column l in RH with low(l)=b and RH(a,l)=1
            int l = R_high->find_low(b);
            bool RHal = (l > -1 && R_high->entry(a, l));    //entry (a,l) in matrix RH

            //transpose rows of R; update low array if necessary
            R_high->swap_rows(a, !RHal);

            if(U_low->entry(a, b))    //case 3.1 -- here, R = RWPW, so no further action required on R
            {
               U_low->add_row(b, a);
               U_low->swap_rows(a);
               U_low->add_row(b, a);
            }
            else    //case 3.2
            {
                R_low->swap_columns(a, true);
                U_low->swap_rows(a);
            }
        }
        else    //simplex b is negative (Case 2)
        {
            //transpose rows of R
            R_high->swap_rows(a, false);   //neither of these rows contain lowest 1's in any column

            if(U_low->entry(a, b)) //case 2.1
            {
                U_low->add_row(b, a);  //so that U will remain upper-triangular
                U_low->swap_rows(a);   //swap rows of U

                if(R_low->low(a) < R_low->low(b)) //case 2.1.1
                {
                    R_low->add_column(a, b);       //necessary due to the row addition on U; this doesn't change low entries
                    R_low->swap_columns(a, true);  //now swap columns of R and update low entries
                }
                else //case 2.1.2
                {
                    R_low->add_column(a, b);       //necessary due to the row addition on U; this doesn't change low entries
                    R_low->swap_columns(a, false); //now swap columns of R but DO NOT update low entries
                    R_low->add_column(a, b);       //restore R to reduced form; low entries now same as they were initially
                    U_low->add_row(b, a);          //necessary due to column addition on R
                }
            }
            else    //case 2.2
            {
                R_low->swap_columns(a, true);  //swap columns of R and update low entries
                U_low->swap_rows(a);           //swap rows of U
            }
        }

        //finally, for cases 2 and 3, transpose columns of U
        U_low->swap_columns(a);
    }
}//end vineyard_update_low()

//performs a vineyard update corresponding to the transposition of columns a and (a + 1)
//  for HIGH simplices
void PersistenceUpdater::vineyard_update_high(unsigned a)
{
    unsigned b = a + 1;

    bool a_pos = (R_high->low(a) == -1);    //true iff simplex corresponding to column a is positive
    bool b_pos = (R_high->low(b) == -1);    //true iff simplex corresponding to column b is positive

    if(a_pos)   //simplex a is positive, so its column is zero, and the fix is easy  (Vineyards paper - Cases 1 and 4)
    {
        if(!b_pos)   //only have to swap columns of R if column b is nonzero
            R_high->swap_columns(a, true);

        //ensure that UL[a,b]=0
        U_high->clear(a, b);

        //transpose rows and columns of U
        U_high->swap_columns(a);
        U_high->swap_rows(a);

        //done -- we don't care about the ROWS corresponding to simplices a and b, because we don't care about the boundaries of (d+2)-simplices
    }
    else    //simplex a is negative (Vineyards paper - Cases 2 and 3)
    {
        if(b_pos)   //simplex b is positive (Case 3)
        {
            if(U_high->entry(a, b))    //case 3.1 -- here, R = RWPW, so no further action required on R
            {
               U_high->add_row(b, a);
               U_high->swap_rows(a);
               U_high->add_row(b, a);
            }
            else    //case 3.2
            {
                R_high->swap_columns(a, true);
                U_high->swap_rows(a);
            }
        }
        else    //simplex b is negative (Case 2)
        {
            if(U_high->entry(a, b)) //case 2.1
            {
                U_high->add_row(b, a);  //so that U will remain upper-triangular
                U_high->swap_rows(a);   //swap rows of U

                if(R_high->low(a) < R_high->low(b)) //case 2.1.1
                {
                    R_high->add_column(a, b);       //necessary due to the row addition on U; this doesn't change low entries
                    R_high->swap_columns(a, true);  //now swap columns of R and update low entries
                }
                else //case 2.1.2
                {
                    R_high->add_column(a, b);       //necessary due to the row addition on U; this doesn't change low entries
                    R_high->swap_columns(a, false); //now swap columns of R but DO NOT update low entries
                    R_high->add_column(a, b);       //restore R to reduced form; low entries now same as they were initially
                    U_high->add_row(b, a);          //necessary due to column addition on R
                }
            }
            else    //case 2.2
            {
                R_high->swap_columns(a, true);  //swap columns and update low entries
                U_high->swap_rows(a);           //swap rows of U
            }
        }

        //finally, for Cases 2 and 3, transpose columns of U
        U_high->swap_columns(a);
    }
}//end vineyard update_high()


//swaps two blocks of columns by updating the total order on columns, then rebuilding the matrices and computing a new RU-decomposition
void PersistenceUpdater::update_order_and_reset_matrices(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL_initial, MapMatrix_Perm* RH_initial)
{
  //STEP 1: update the lift map for all multigrades and store the current column index for each multigrade

    //store current column index for each multigrade that lifts to xiMatrixEntry second
    int low_col = second->low_index;
    for(std::list<Multigrade*>::iterator it = second->low_simplices.begin(); it != second->low_simplices.end(); ++it) //starts with rightmost column, ends with leftmost
    {
        (*it)->simplex_index = low_col;
        low_col -= (*it)->num_cols;
    }
    int high_col = second->high_index;
    for(std::list<Multigrade*>::iterator it = second->high_simplices.begin(); it != second->high_simplices.end(); ++it) //starts with rightmost column, ends with leftmost
    {
        (*it)->simplex_index = high_col;
        high_col -= (*it)->num_cols;
    }

    //get column indexes for the first equivalence class
    low_col = first->low_index;   //rightmost column index of low simplices for the equivalence class to move
    high_col = first->high_index; //rightmost column index of high simplices for the equivalence class to move

    //set column indexes for the first class to their final position
    first->low_index = second->low_index;
    first->high_index = second->high_index;

    //store current column index and update the lift map for each multigrade that lifts to xiMatrixEntry first
    //"low" simplices (start with rightmost column, end with leftmost)
    for(std::list<Multigrade*>::iterator it = first->low_simplices.begin(); it != first->low_simplices.end(); ) //NOTE: iterator advances in loop
    {
        Multigrade* cur_grade = *it;

        //remember current position of this grade
        cur_grade->simplex_index = low_col;

        if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
            //then move columns at cur_grade past columns at xiMatrixEntry second; lift map does not change ( lift : multigrades --> xiSupportElements )
        {
            second->low_index -= cur_grade->num_cols;
            ++it;
        }
        else    //then cur_grade now lifts to xiMatrixEntry second; columns don't move
        {
            //associate cur_grade with second
            second->insert_multigrade(cur_grade, true);
            it = first->low_simplices.erase(it);    //NOTE: advances the iterator!!!

            //update column counts
            first->low_count -= cur_grade->num_cols;
            second->low_count += cur_grade->num_cols;
        }

        //update column index
        low_col -= cur_grade->num_cols;
    }//end "low" simplex loop

    //"high" simplices (start with rightmost column, end with leftmost)
    for(std::list<Multigrade*>::iterator it = first->high_simplices.begin(); it != first->high_simplices.end(); ) //NOTE: iterator advances in loop
    {
        Multigrade* cur_grade = *it;

        //remember current position of this grade
        cur_grade->simplex_index = high_col;

        if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
            //then move columns at cur_grade past columns at xiMatrixEntry second; lift map does not change ( lift : multigrades --> xiSupportElements )
        {
            second->high_index -= cur_grade->num_cols;
            ++it;
        }
        else    //then cur_grade now lifts to xiMatrixEntry second; columns don't move
        {
            //associate cur_grade with target
            second->insert_multigrade(cur_grade, false);
            it = first->high_simplices.erase(it);    //NOTE: advances the iterator!!!

            //update column counts
            first->high_count -= cur_grade->num_cols;
            second->high_count += cur_grade->num_cols;
        }

        //update column index
        high_col -= cur_grade->num_cols;
    }//end "high" simplex loop


  //STEP 2: traverse grades (backwards) in the new order and update the permutation vectors to reflect the new order on matrix columns

    //temporary data structures
    xiMatrixEntry* cur_entry = first;
    low_col = first->low_index;
    high_col = first->high_index;

    //loop over xiMatrixEntrys
    while(first != second)
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
        if(cur_entry == first)
            cur_entry = second;
        else
            break;
    }//end while

    //fix inverse permutation vectors -- is there a better way to do this?
    for(unsigned i=0; i < perm_low.size(); i++)
        inv_perm_low[perm_low[i]] = i;
    for(unsigned i=0; i < perm_high.size(); i++)
        inv_perm_high[perm_high[i]] = i;


  //STEP 3: re-build the matrix R based on the new order

    R_low->rebuild(RL_initial, perm_low);
    R_high->rebuild(RH_initial, perm_high, perm_low);


  //STEP 4: compute the new RU-decomposition

    ///TODO: should I avoid deleting and reallocating matrix U?
    delete U_low;
    U_low = R_low->decompose_RU();
    delete U_high;
    U_high = R_high->decompose_RU();

}//end update_order_and_reset_matrices()

//updates the total order on columns, rebuilds the matrices, and computing a new RU-decomposition for a NON-STRICT anchor
void PersistenceUpdater::update_order_and_reset_matrices(xiMatrixEntry* anchor, xiMatrixEntry* generator, MapMatrix_Perm* RL_initial, MapMatrix_Perm* RH_initial)
{
    //anything to do here?????
    //anchor and generator?????

    //re-build the matrix R based on the new order
    R_low->rebuild(RL_initial, perm_low);
    R_high->rebuild(RH_initial, perm_high, perm_low);

    //compute the new RU-decomposition
    ///TODO: should I avoid deleting and reallocating matrix U?
    delete U_low;
    U_low = R_low->decompose_RU();
    delete U_high;
    U_high = R_high->decompose_RU();

}//end update_order_and_reset_matrices()

//swaps two blocks of simplices in the total order, and returns the number of transpositions that would be performed on the matrix columns if we were doing vineyard updates
void PersistenceUpdater::count_switches_and_separations(xiMatrixEntry* at_anchor, bool from_below, unsigned long& switches, unsigned long& seps)
{
    //identify entries
    xiMatrixEntry* first = from_below ? at_anchor->down : at_anchor->left;
    xiMatrixEntry* second = from_below ? at_anchor->left : at_anchor->down;
    xiMatrixEntry* temp = new xiMatrixEntry(at_anchor->left->x, at_anchor->down->y); //temporary entry for holding grades that come before BOTH first and second

    //separate out the grades that lift to anchor from those that lift to second
    do_separations(at_anchor, second, from_below);
    seps += second->low_count * at_anchor->low_count + second->high_count * at_anchor->high_count;
    do_separations(first, temp, from_below);
    seps += temp->low_count * first->low_count + temp->high_count * first->high_count;

    //count switches
    int i = first->low_index;
    first->low_index = second->low_index;
    second->low_index = i;
    i = first->high_index;
    first->high_index = second->high_index;
    second->high_index = i;
    switches += first->low_count * second->low_count + first->high_count * second->high_count;

    //count final separations
    seps += first->low_count * at_anchor->low_count + first->high_count * at_anchor->high_count;
    merge_grade_lists(at_anchor, first);
    seps += temp->low_count * second->low_count + temp->high_count * second->high_count;
    merge_grade_lists(second, temp);
}//end count_switches_and_separations()

//used by the previous function to split grade lists at each anchor crossing
void PersistenceUpdater::do_separations(xiMatrixEntry* greater, xiMatrixEntry* lesser, bool horiz)
{
    //first, low simpilices
    int gr_col = greater->low_index;
    int cur_col = gr_col;
    std::list<Multigrade*> grades = greater->low_simplices;
    greater->low_simplices.clear();       ///this isn't so efficient...
    for(std::list<Multigrade*>::iterator it = grades.begin(); it != grades.end(); ++it)
    {
        Multigrade* cur_grade = *it;
        if((horiz && cur_grade->x > lesser->x) || (!horiz && cur_grade->y > lesser->y))  //then this grade lifts to greater
        {
            greater->low_simplices.push_back(cur_grade);
            gr_col -= cur_grade->num_cols;
        }
        else    //then this grade lifts to lesser
            lesser->low_simplices.push_back(cur_grade);

        cur_col -= cur_grade->num_cols;
    }
    lesser->low_index = gr_col;
    lesser->low_count = gr_col - cur_col;
    greater->low_count = greater->low_index - lesser->low_index;

    //now high simplices
    gr_col = greater->high_index;
    cur_col = gr_col;
    std::list<Multigrade*> grades_h = greater->high_simplices;
    greater->high_simplices.clear();       ///this isn't so efficient...
    for(std::list<Multigrade*>::iterator it = grades_h.begin(); it != grades_h.end(); ++it)
    {
        Multigrade* cur_grade = *it;
        if((horiz && cur_grade->x > lesser->x) || (!horiz && cur_grade->y > lesser->y))  //then this grade lifts to greater
        {
            greater->high_simplices.push_back(cur_grade);
            gr_col -= cur_grade->num_cols;
        }
        else    //then this grade lifts to lesser
            lesser->high_simplices.push_back(cur_grade);

        cur_col -= cur_grade->num_cols;
    }
    lesser->high_index = gr_col;
    lesser->high_count = gr_col - cur_col;
    greater->high_count = greater->high_index - lesser->high_index;
}//end do_separations

//swaps two blocks of columns by using a quicksort to update the matrices, then fixing the RU-decomposition (Gaussian elimination on U followed by reduction of R)
void PersistenceUpdater::quicksort_and_reduce(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below)
{
    //update the lift map for all multigrades, storing the current column index for each multigrade


    //traverse grades in the new order and build the new order on matrix columns


    //quicksort the columns to put them in the new order, updating the permutation


    //re-build the matrix R based on the new


}//end quicksort_and_reduce()

//removes entries corresponding to xiMatrixEntry head from lift_low and lift_high
void PersistenceUpdater::remove_lift_entries(xiMatrixEntry* entry)
{
    if(mesh->verbosity >= 9) { qDebug() << "    ----removing partition entries for xiMatrixEntry" << entry->index << "(" << entry->low_index << ";" << entry->high_index << ")"; }

    //low simplices
    std::map<unsigned, xiMatrixEntry*>::iterator it1 = lift_low.find(entry->low_index);
    if(it1 != lift_low.end() && it1->second == entry)
        lift_low.erase(it1);

    //high simplices
    std::map<unsigned, xiMatrixEntry*>::iterator it2 = lift_high.find(entry->high_index);
    if(it2 != lift_high.end() && it2->second == entry)
        lift_high.erase(it2);

}//end remove_lift_entries()

//if the equivalence class corresponding to xiMatrixEntry head has nonempty sets of "low" or "high" simplices, then this function creates the appropriate entries in lift_low and lift_high
void PersistenceUpdater::add_lift_entries(xiMatrixEntry* entry)
{
    if(mesh->verbosity >= 9) { qDebug() << "    ----adding partition entries for xiMatrixEntry" << entry->index << "(" << entry->low_index << ";" << entry->high_index << ")"; }

    //low simplices
    if(entry->low_count > 0)
        lift_low.insert( std::pair<unsigned, xiMatrixEntry*>(entry->low_index, entry) );

    //high simplices
    if(entry->high_count > 0)
        lift_high.insert( std::pair<unsigned, xiMatrixEntry*>(entry->high_index, entry) );
}//end add_lift_entries()

//stores a barcode template in a 2-cell of the arrangement
///TODO: IMPROVE THIS!!! (store previous barcode at the simplicial level, and only examine columns that were modified in the recent update)
/// Is there a better way to handle endpoints at infinity?
void PersistenceUpdater::store_barcode_template(Face* cell)
{
//    QDebug qd = qDebug().nospace();
//    qd << "  -----barcode: ";

    //mark this cell as visited
    cell->mark_as_visited();

    //get a reference to the barcode template object
    BarcodeTemplate& dbc = cell->get_barcode();

    //loop over all zero-columns in matrix R_low
    for(unsigned c=0; c < R_low->width(); c++)
    {
        if(R_low->col_is_empty(c))  //then simplex corresponding to column c is positive
        {
            //find index of template point corresponding to simplex c
            std::map<unsigned, xiMatrixEntry*>::iterator tp1 = lift_low.lower_bound(c);
            unsigned a = (tp1 != lift_low.end()) ? tp1->second->index : -1;   //index is -1 iff the simplex maps to infinity
            ///TODO: CHECK -- SIMPLICES NEVER LIFT TO INFINITY NOW, RIGHT????

            //is simplex s paired?
            int s = R_high->find_low(c);
            if(s != -1)  //then simplex c is paired with negative simplex s
            {
                //find index of xi support point corresponding to simplex s
                std::map<unsigned, xiMatrixEntry*>::iterator tp2 = lift_high.lower_bound(s);
                unsigned b = (tp2 != lift_high.end()) ? tp2->second->index : -1;   //index is -1 iff the simplex maps to infinity
                ///TODO: CHECK -- SIMPLICES NEVER LIFT TO INFINITY NOW, RIGHT????

                if(a != b)  //then we have a bar of positive length
                {
//                    qd << "(" << c << "," << s << ")-->(" << a << "," << b << ") ";
                    dbc.add_bar(a, b);
                }
            }
            else //then simplex c generates an essential cycle
            {
//                qd << c << "-->" << a << " ";

                dbc.add_bar(a, -1);     //b = -1 = MAX_UNSIGNED indicates this is an essential cycle
            }
        }
    }
}//end store_barcode_template()

//chooses an initial threshold by timing vineyard updates corresponding to random transpositions
unsigned long PersistenceUpdater::choose_initial_threshold(int time_for_initial_decomp)
{
    qDebug() << "RANDOM VINEYARD UPDATES TO CHOOSE THE INITIAL THRESHOLD";

    //make a copy of the matrices
    MapMatrix_Perm* R_low_copy = new MapMatrix_Perm(*R_low);
    MapMatrix_Perm* R_high_copy = new MapMatrix_Perm(*R_high);
    MapMatrix_RowPriority_Perm* U_low_copy = new MapMatrix_RowPriority_Perm(*U_low);
    MapMatrix_RowPriority_Perm* U_high_copy = new MapMatrix_RowPriority_Perm(*U_high);

    //other data structures
    int num_cols = R_low->width() + R_high->width();
    std::list<int> trans_list;

    //avoid trivial cases
    if(num_cols <= 3)   //need either the low or high matrix to have at least 2 columns
        return 1000;

    //determine the time for which we will do transpositions
    int trans_time = time_for_initial_decomp / 20;
    if(trans_time < 100)
        trans_time = 100;   //run for at least 100 milliseconds

    //start the timer
    QTime timer;
    timer.start();

    //do transpositions
    qDebug() << "  -->Doing some random vineyard updates...";
    while(timer.elapsed() < trans_time || trans_list.size() == 0)   //do a transposition
    {
        int rand_col = rand() % (num_cols - 1);   //random integer in {0, 1, ..., num_cols - 2}

        if(rand_col + 1 < R_low->width()) //then transpose LOW columns rand_col and (rand_col + 1)
        {
            vineyard_update_low(rand_col);
            trans_list.push_back(rand_col);
        }
        else if(R_low->width() <= rand_col) //then transpose HIGH columns rand_col and (rand_col + 1)
        {
            vineyard_update_high(rand_col - R_low->width());
            trans_list.push_back(rand_col);
        }
        //note that if (rand_col + 1 == R_low->width()), then we don't do anything, since rand_col is a "low" simplex but rand_col + 1 is a "high" simplex, and these cannot swap
    }

    //do the inverse transpositions
    qDebug() << "  -->Undoing the random vineyard updates...";
    for(std::list<int>::reverse_iterator rit = trans_list.rbegin(); rit != trans_list.rend(); ++rit)
    {
        int col = *rit;
        if(col < R_low->width())
        {
            vineyard_update_low(col);
        }
        else
        {
            vineyard_update_high(col - R_low->width());
        }
    }

    //record the time
    trans_time = timer.elapsed();

    qDebug() << "  -->Did" << (2*trans_list.size()) << "vineyard updates in" << trans_time << "milliseconds.";

    //restore the matrices to the copies made at the beginning of this function
    ///TODO: is this good style?
    delete R_low;
    R_low = R_low_copy;
    delete R_high;
    R_high = R_high_copy;
    delete U_low;
    U_low = U_low_copy;
    delete U_high;
    U_high = U_high_copy;

    //return the threshold
    return (unsigned long) (((double) (2*trans_list.size()) / (double) trans_time) * time_for_initial_decomp);
}//end choose_initial_threshold()


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
    for(std::map<unsigned, xiMatrixEntry*>::iterator it = lift_high.begin(); it != lift_high.end(); ++it)
        qd << it->first << "->" << it->second->index << ", ";
}
