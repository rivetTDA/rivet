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

    ///TODO: choose the initial value of the threshold intelligently
    unsigned long threshold = 500000;     //if the number of swaps might exceed this threshold, then do a persistence calculation from scratch
    unsigned long swap_estimate = 0;
    qDebug() << "reset threshold set to" << threshold;

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
        unsigned long swap_counter = 0;   //counts number of transpositions at each step
        swap_estimate = 0;

        //determine which anchor is represented by this edge
        Anchor* cur_anchor = (path[i])->get_anchor();
        xiMatrixEntry* at_anchor = cur_anchor->get_entry();

        qDebug() << "  step" << i << "of path: crossing anchor at ("<< cur_anchor->get_x() << "," << cur_anchor->get_y() << ") into cell" << mesh->FID((path[i])->get_face());

        //get equivalence classes for this anchor
        xiMatrixEntry* down = at_anchor->down;
        xiMatrixEntry* left = at_anchor->left;

        //if this is a strict anchor, then swap simplices
        if(down != NULL && left != NULL) //then this is a strict anchor and some simplices swap
        {
            //process the swaps
            if(cur_anchor->is_above()) //then the anchor is crossed from below to above
            {
                remove_lift_entries(at_anchor);        //this block of the partition might become empty
                remove_lift_entries(down);             //this block of the partition will move
                split_grade_lists(at_anchor, left, true);   //move grades that come before left from anchor to left

                //now permute the columns and fix the RU-decomposition
                swap_estimate = static_cast<unsigned long>(left->low_count) * static_cast<unsigned long>(down->low_count)
                        + static_cast<unsigned long>(left->high_count) * static_cast<unsigned long>(down->high_count);
                if(swap_estimate < threshold)
                    swap_counter += move_columns(down, left, true);
                else
                    update_order_and_reset_matrices(down, left, true, R_low_initial, R_high_initial);

                //post-move updates to equivalance class info
                merge_grade_lists(at_anchor, down);     //move all grades from down to anchor
                add_lift_entries(at_anchor);       //this block of the partition might have previously been empty
                add_lift_entries(left);            //this block of the partition moved
            }
            else    //then anchor is crossed from above to below
            {
                //pre-move updates to equivalence class info
                remove_lift_entries(at_anchor);        //this block of the partition might become empty
                remove_lift_entries(left);             //this block of the partition will move
                split_grade_lists(at_anchor, down, false);  //move grades that come before down from anchor to down

                //now permute the columns and fix the RU-decomposition
                swap_estimate = static_cast<unsigned long>(left->low_count) * static_cast<unsigned long>(down->low_count)
                        + static_cast<unsigned long>(left->high_count) * static_cast<unsigned long>(down->high_count);
                if(swap_estimate < threshold)
                    swap_counter += move_columns(left, down, false);
                else
                    update_order_and_reset_matrices(left, down, false, R_low_initial, R_high_initial);

                //post-move updates to equivalance class info
                merge_grade_lists(at_anchor, left);     //move all grades from down to anchor
                add_lift_entries(at_anchor);       //this block of the partition might have previously been empty
                add_lift_entries(down);            //this block of the partition moved
            }
        }
        else    //this is a non-strict anchor, and we just have to split or merge equivalence classes
        {
            xiMatrixEntry* generator = at_anchor->down;
            if(generator == NULL)
                generator = at_anchor->left;

            if((cur_anchor->is_above() && generator == at_anchor->down) || (!cur_anchor->is_above() && generator == at_anchor->left))
                //then merge classes
            {
                remove_lift_entries(generator);
                merge_grade_lists(at_anchor, generator);
                add_lift_entries(at_anchor);  //this is necessary in case the class was previously empty
            }
            else    //then split classes
            {
                remove_lift_entries(at_anchor);   //this is necessary because the class corresponding
                split_grade_lists(at_anchor, generator, (at_anchor->y == generator->y));
                add_lift_entries(at_anchor);      //  to at_anchor might have become empty
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
        if(swap_estimate < threshold)
        {
            qDebug() << "    --> this step took" << step_time << "milliseconds and involved" << swap_counter << "transpositions";
            total_transpositions += swap_counter;
            total_time_for_transpositions += step_time;
        }
        else
        {
            qDebug() << "    --> this step took" << step_time << "milliseconds -- reset matrices to avoid an estimated" << swap_estimate << "transpositions";
            number_of_resets++;
            total_time_for_resets += step_time;
        }

        if(step_time > max_time)
            max_time = step_time;

        //update the treshold
        if(total_time_for_transpositions > 0 && total_transpositions > total_time_for_transpositions)
        {
            ///TODO: integer division OK here???
            threshold = (total_transpositions/total_time_for_transpositions)*(total_time_for_resets/number_of_resets);
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

//moves grades associated with xiMatrixEntry greater, that come before xiMatrixEntry lesser in R^2, so that they become associated with lesser
//  precondition: no grades lift to lesser (it has empty level sets under the lift map)
void PersistenceUpdater::split_grade_lists(xiMatrixEntry* greater, xiMatrixEntry* lesser, bool horiz)
{
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
                move_low_columns(cur_col, cur_grade->num_cols, gr_col);

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
                move_high_columns(cur_col, cur_grade->num_cols, gr_col);

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
}//end split_grade_lists()

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
            unsigned a = i - c; //TODO: cast i to unsigned???
            unsigned b = a + 1;

            //we must swap the (d+1)-simplices currently corresponding to columns a and b=a+1
//            qDebug() << "(" << a << "," << b << ")";

            bool a_pos = (R_high->low(a) == -1);    //true iff simplex corresponding to column a is positive
            bool b_pos = (R_high->low(b) == -1);    //true iff simplex corresponding to column b is positive

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

    //loop over xiMatrixEntrys that have simplices which move
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
