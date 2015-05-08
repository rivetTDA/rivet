#include "persistence_updater.h"

#include <qdebug.h>
#include <QTime>


//constructor -- also fills xi_matrix with the xi support points
PersistenceUpdater::PersistenceUpdater(Mesh *m, MultiBetti &mb, std::vector<xiPoint> &xi_pts) :
    mesh(m), bifiltration(mb.bifiltration), dim(mb.dimension),
    xi_matrix(m->x_grades.size(), m->y_grades.size()),
    testing(false)
{
    //fill the xiSupportMatrix with the xi support points
    xi_matrix.fill(mb, xi_pts);

    //create partition entries for infinity (which never change)
    unsigned infty = -1;    // = MAX_UNSIGNED, right?
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
void PersistenceUpdater::store_barcodes(std::vector<Halfedge*>& path)
{
    QTime timer;    //for timing the computations

    // PART 1: GET THE BOUNDARY MATRICES WITH PROPER SIMPLEX ORDERING

    timer.start();

    //get multi-grade data in each dimension
    if(mesh->verbosity >= 6) { qDebug() << "  Mapping low simplices:"; }
    IndexMatrix* ind_low = bifiltration->get_index_mx(dim);    //can we improve this with something more efficient than IndexMatrix?
    std::vector<int> low_simplex_order;     //this will be a map : dim_index --> order_index for dim-simplices
    store_multigrades(ind_low, true, low_simplex_order);
    delete ind_low;

    if(mesh->verbosity >= 6) { qDebug() << "  Mapping high simplices:"; }
    IndexMatrix* ind_high = bifiltration->get_index_mx(dim + 1);    //again, could be improved?
    std::vector<int> high_simplex_order;     //this will be a map : dim_index --> order_index for (dim+1)-simplices
    store_multigrades(ind_high, false, high_simplex_order);
    delete ind_high;

    //testing only
//    std::cout << "== low_simplex_order: ";
//    for(int i=0; i<low_simplex_order.size(); i++)
//        std::cout << low_simplex_order[i] << ", ";
//    std::cout << "\n== high_simplex_order: ";
//    for(int i=0; i<high_simplex_order.size(); i++)
//        std::cout << high_simplex_order[i] << ", ";
//    std::cout << "\n";

    //get boundary matrices (R) and identity matrices (U) for RU-decomposition
    MapMatrix_Perm* R_low = bifiltration->get_boundary_mx(low_simplex_order);
    MapMatrix_Perm* R_high = bifiltration->get_boundary_mx(low_simplex_order, high_simplex_order);

    //print runtime data
    qDebug() << "  --> computing initial order on simplices and building the boundary matrices took" << timer.elapsed() << "milliseconds";

/// TESTING ONLY - CHECK THAT D=RU
    if(testing)
    {
//        qDebug() << "  Boundary matrix for low simplices:\n";
//        R_low->print();
//        qDebug() << "  Boundary matrix for high simplices:\n";
//        R_high->print();

        D_low = bifiltration->get_boundary_mx(low_simplex_order);
        D_high = bifiltration->get_boundary_mx(low_simplex_order, high_simplex_order);
    }


  // PART 2: INITIAL PERSISTENCE COMPUTATION (RU-decomposition)

    timer.start();

    MapMatrix_RowPriority_Perm* U_low = R_low->decompose_RU();
    MapMatrix_RowPriority_Perm* U_high = R_high->decompose_RU();

    qDebug() << "  --> computing the RU decomposition took" << timer.elapsed() << "milliseconds";

    //store the barcode template in the first cell
    Face* first_cell = mesh->topleft->get_twin()->get_face();
    store_barcode_template(first_cell, R_low, R_high);

    if(testing)
    {
        qDebug() << "Initial persistence computation in cell " << mesh->FID(first_cell);
//        qDebug() << "  Reduced matrix for low simplices:";
//        R_low->print();
//        R_low->check_lows();
//        qDebug() << "  Matrix U for low simplices:";
//        U_low->print();
//        qDebug() << "  Reduced matrix for high simplices:";
//        R_high->print();
//        R_high->check_lows();
//        qDebug() << "  Matrix U for high simplices:";
//        U_high->print();
//        qDebug() << "  Low partition: ";
//        for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_low.begin(); it != partition_low.end(); ++it)
//            qDebug() << "     " << it->first << "->" << it->second->index << ", ";
//        qDebug() << "  High partition: ";
//        for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_high.begin(); it != partition_high.end(); ++it)
//            qDebug() << "     " << it->first << "->" << it->second->index << ", ";
//        qDebug() << "  Barcode Template: ";
//        first_cell->get_barcode().print();
    }


  // PART 3: TRAVERSE THE PATH AND DO VINEYARD UPDATES

    qDebug() << "TRAVERSING THE PATH: path has" << path.size() << "steps";
    timer.start();

    ///TEMPORARY: data structures for analyzing the computation
    unsigned long total_transpositions = 0;
//    std::vector<unsigned long> swap_counters(path.size(),0);
//    std::vector<int> crossing_times(path.size(),0);
    unsigned long max_swaps = 0;
    int max_time = 0;

    //traverse the path
    QTime steptimer;
    for(unsigned i=0; i<path.size(); i++)
    {
        steptimer.start();          //time update at each step of the path
        unsigned long swap_counter = 0;   //counts number of transpositions at each step

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

            if(cur_anchor->is_above()) //then anchor is crossed from below to above
            {
                if(mesh->verbosity >= 9) { qDebug() << " == strict anchor crossed below to above =="; }

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
                    remove_partition_entries(left);     //this partition will move

                remove_partition_entries(down);         //this partition will move

                //now move the columns
                swap_counter += move_columns(down, left, true, R_low, U_low, R_high, U_high);

                //post-move updates to equivalance class info
                if(at_anchor != NULL)  //this anchor is supported
                {
                    at_anchor->low_class_size = at_anchor->low_count + down->low_class_size;
                    at_anchor->high_class_size = at_anchor->high_count + down->high_class_size;
                    down->low_class_size = -1;  //this xiMatrixEntry is no longer the head of an equivalence class
                    add_partition_entries(at_anchor);
                }
                else    //this anchor is not supported
                    add_partition_entries(down);

                add_partition_entries(left);
            }
            else    //then anchor is crossed from above to below
            {
                if(mesh->verbosity >= 9) { qDebug() << " == strict anchor crossed above to below =="; }

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
                    remove_partition_entries(down);     //this partition will move

                remove_partition_entries(left);         //this partition will move

                //now move the columns
                swap_counter += move_columns(left, down, false, R_low, U_low, R_high, U_high);

                //post-move updates to equivalance class info
                if(at_anchor != NULL)  //this anchor is supported
                {
                    at_anchor->low_class_size = at_anchor->low_count + left->low_class_size;
                    at_anchor->high_class_size = at_anchor->high_count + left->high_class_size;
                    left->low_class_size = -1;  //this xiMatrixEntry is no longer the head of an equivalence class
                    add_partition_entries(at_anchor);
                }
                else    //this anchor is not supported
                    add_partition_entries(left);

                add_partition_entries(down);
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

        //testing
//        std::cout << "  Anchors above the current line: ";
//        for(std::set<Anchor*, Anchor_LeftComparator>::iterator it = mesh->all_anchors.begin(); it != mesh->all_anchors.end(); ++it)
//            if((*it)->is_above())
//                std::cout << "(" << (*it)->get_x() << "," << (*it)->get_y() << ") ";
//        std::cout << "\n";

        //if this cell does not yet have a barcode template, then store it now
        Face* cur_face = (path[i])->get_face();
        if(!cur_face->has_been_visited())
            store_barcode_template(cur_face, R_low, R_high);

        //testing
        if(testing)
        {
//            qDebug() << "  Reduced matrix for low simplices:";
//            R_low->print();
//            R_low->check_lows();
//            qDebug() << "  Matrix U for low simplices:";
//            U_low->print();
//            qDebug() << "  Matrix D for low simplices:";
//            D_low->print();
//            qDebug() << "  Reduced matrix for high simplices:";
//            R_high->print();
//            R_high->check_lows();
//            qDebug() << "  Matrix U for high simplices:";
//            U_high->print();
//            qDebug() << "  Matrix D for high simplices:";
//            D_high->print();
//            qDebug() << "  Low partition: ";
//            for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_low.begin(); it != partition_low.end(); ++it)
//                qDebug() << "     " << it->first << "->" << it->second->index << ", ";
//            qDebug() << "  High partition: ";
//            for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_high.begin(); it != partition_high.end(); ++it)
//                qDebug() << "     " << it->first << "->" << it->second->index << ", ";
//            qDebug() << "  Barcode Template: ";
//            cur_face->get_barcode().print();
            check_low_matrix(R_low, U_low);
//            check_high_matrix(R_high, U_high);
        }

        //print runtime data
        int step_time = steptimer.elapsed();
//        qDebug() << "    --> this step took" << step_time << "milliseconds and involved" << stepcounter << "transpositions";

        //store data for analysis
        total_transpositions += swap_counter;
//        swap_counters[i] = swap_counter;
//        crossing_times[i] = step_time;
        if(swap_counter > max_swaps)
            max_swaps = swap_counter;
        if(step_time > max_time)
            max_time = step_time;

    }//end path traversal

    //print runtime data
    qDebug() << "  --> path traversal and persistence updates took" << timer.elapsed() << "milliseconds and involved" << total_transpositions << "transpositions";

    ///TEMPORARY
//    qDebug() << "DATA: number of transpositions and runtime for each crossing:";
//    for(unsigned i=0; i<swap_counters.size(); i++)
//        qDebug().nospace() << swap_counters[i] << ", " << crossing_times[i];
    qDebug() << "DATA: max number of swaps per crossing:" << max_swaps << "; max time per crossing:" << max_time;


  // PART 4: CLEAN UP

    delete R_low;
    delete R_high;
    delete U_low;
    delete U_high;

}//end store_barcodes()

//computes and stores barcode templates using lazy updates
//uses lazy updates and unsorted "bins" for each row and column
void PersistenceUpdater::store_barcodes_lazy(std::vector<Halfedge*>& path)
{
    QTime timer;    //for timing the computations

    // PART 1: GET THE BOUNDARY MATRICES WITH PROPER SIMPLEX ORDERING

    timer.start();

    //get multi-grade data in each dimension
    if(mesh->verbosity >= 6) { qDebug() << "  Mapping low simplices:"; }
    IndexMatrix* ind_low = bifiltration->get_index_mx(dim);    //can we improve this with something more efficient than IndexMatrix?
    std::vector<int> low_simplex_order;     //this will be a map : dim_index --> order_index for dim-simplices
    store_multigrades(ind_low, true, low_simplex_order);
    delete ind_low;

    if(mesh->verbosity >= 6) { qDebug() << "  Mapping high simplices:"; }
    IndexMatrix* ind_high = bifiltration->get_index_mx(dim + 1);    //again, could be improved?
    std::vector<int> high_simplex_order;     //this will be a map : dim_index --> order_index for (dim+1)-simplices
    store_multigrades(ind_high, false, high_simplex_order);
    delete ind_high;

    //get boundary matrices (R) and identity matrices (U) for RU-decomposition
    MapMatrix_Perm* R_low = bifiltration->get_boundary_mx(low_simplex_order);
    MapMatrix_Perm* R_high = bifiltration->get_boundary_mx(low_simplex_order, high_simplex_order);

    //print runtime data
    qDebug() << "  --> computing initial order on simplices and building the boundary matrices took" << timer.elapsed() << "milliseconds";

/// TESTING ONLY - CHECK THAT D=RU
    if(testing)
    {
//        qDebug() << "  Boundary matrix for low simplices:\n";
//        R_low->print();
//        qDebug() << "  Boundary matrix for high simplices:\n";
//        R_high->print();

        D_low = bifiltration->get_boundary_mx(low_simplex_order);
        D_high = bifiltration->get_boundary_mx(low_simplex_order, high_simplex_order);
    }


  // PART 2: INITIAL PERSISTENCE COMPUTATION (RU-decomposition)

    timer.start();

    MapMatrix_RowPriority_Perm* U_low = R_low->decompose_RU();
    MapMatrix_RowPriority_Perm* U_high = R_high->decompose_RU();

    qDebug() << "  --> computing the RU decomposition took" << timer.elapsed() << "milliseconds";

    //store the barcode template in the first cell
    Face* first_cell = mesh->topleft->get_twin()->get_face();
    store_barcode_template(first_cell, R_low, R_high);

    if(testing)
    {
//        qDebug() << "Initial persistence computation in cell " << mesh->FID(first_cell);
//        qDebug() << "  Reduced matrix for low simplices:";
//        R_low->print();
//        R_low->check_lows();
//        qDebug() << "  Matrix U for low simplices:";
//        U_low->print();
//        qDebug() << "  Reduced matrix for high simplices:";
//        R_high->print();
//        R_high->check_lows();
//        qDebug() << "  Matrix U for high simplices:";
//        U_high->print();
//        qDebug() << "  Low partition: ";
//        for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_low.begin(); it != partition_low.end(); ++it)
//            qDebug() << "     " << it->first << "->" << it->second->index << ", ";
//        qDebug() << "  High partition: ";
//        for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_high.begin(); it != partition_high.end(); ++it)
//            qDebug() << "     " << it->first << "->" << it->second->index << ", ";
//        qDebug() << "  Barcode Template: ";
//        first_cell->get_barcode().print();
    }


  // PART 3: TRAVERSE THE PATH AND DO VINEYARD UPDATES

    qDebug() << "TRAVERSING THE PATH USING LAZY UPDATES: path has" << path.size() << "steps";
    qDebug() << "                          ^^^^^^^^^^^^";
    timer.start();

    ///TEMPORARY: data structures for analyzing the computation
    unsigned long total_transpositions = 0;
//    std::vector<unsigned long> swap_counters(path.size(),0);
//    std::vector<int> crossing_times(path.size(),0);
    unsigned long max_swaps = 0;
    int max_time = 0;

    //traverse the path
    QTime steptimer;
    for(unsigned i=0; i<path.size(); i++)
    {
        steptimer.start();          //time update at each step of the path
        unsigned long swap_counter = 0;   //counts number of transpositions at each step

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

            if(cur_anchor->is_above()) //then anchor is crossed from below to above (prior to swap: down class is first, left class is second)
            {
                if(mesh->verbosity >= 9) { qDebug() << " == strict anchor crossed below to above =="; }

                if(at_anchor != NULL)  //this anchor is supported
                {
                    //find columns from the row bin that must be mapped to the anchor
                    swap_counter += move_columns_from_bin_horizontal(xi_matrix.get_row_bin(at_anchor->y), at_anchor, R_low, U_low, R_high, U_high);

                    //update equivalence class info
                    left->low_index = at_anchor->low_index - at_anchor->low_count;                //necessary since low_index, low_class_size,
                    left->low_class_size = at_anchor->low_class_size - at_anchor->low_count;      //  high_index, and high_class_size
                    left->high_index = at_anchor->high_index - at_anchor->high_count;             //  are only reliable for the head
                    left->high_class_size = at_anchor->high_class_size - at_anchor->high_count;   //  of each equivalence class
                    remove_partition_entries(at_anchor);   //this block of the partition might become empty
                }
                else    //this anchor is not supported
                    remove_partition_entries(left);     //this block of the partition will move

                remove_partition_entries(down);         //this block of the partition will move

                //now move columns from the down class, either past the left class or into the row bin
                swap_counter += move_columns_lazy(down, left, true, R_low, U_low, R_high, U_high);

                //post-move updates to equivalance class info
                if(at_anchor != NULL)  //this anchor is supported
                {
                    at_anchor->low_class_size = at_anchor->low_count + down->low_class_size;
                    at_anchor->high_class_size = at_anchor->high_count + down->high_class_size;
                    down->low_class_size = -1;  //this xiMatrixEntry is no longer the head of an equivalence class
                    add_partition_entries(at_anchor);
                }
                else    //this anchor is not supported
                    add_partition_entries(down);

                add_partition_entries(left);
            }
            else    //then anchor is crossed from above to below (prior to swap: left class is first, down class is second)
            {
                if(mesh->verbosity >= 9) { qDebug() << " == strict anchor crossed above to below ==\n"; }

                if(at_anchor != NULL)   //this anchor is supported
                {
                    //find columns from the column bin that must be mapped to the anchor
                    swap_counter += move_columns_from_bin_vertical(xi_matrix.get_col_bin(at_anchor->x), at_anchor, R_low, U_low, R_high, U_high);

                    //update equivalence class info
                    down->low_index = at_anchor->low_index - at_anchor->low_count;                //necessary since low_index, low_class_size,
                    down->low_class_size = at_anchor->low_class_size - at_anchor->low_count;      //  high_index, and high_class_size
                    down->high_index = at_anchor->high_index - at_anchor->high_count;             //  are only reliable for the head
                    down->high_class_size = at_anchor->high_class_size - at_anchor->high_count;   //  of each equivalence class
                    remove_partition_entries(at_anchor);   //this partition might become empty
                }
                else    //this anchor is not supported
                    remove_partition_entries(down);     //this partition will move

                remove_partition_entries(left);         //this partition will move

                //now move the columns
                swap_counter += move_columns_lazy(left, down, false, R_low, U_low, R_high, U_high);

                //post-move updates to equivalance class info
                if(at_anchor != NULL)  //this anchor is supported
                {
                    at_anchor->low_class_size = at_anchor->low_count + left->low_class_size;
                    at_anchor->high_class_size = at_anchor->high_count + left->high_class_size;
                    left->low_class_size = -1;  //this xiMatrixEntry is no longer the head of an equivalence class
                    add_partition_entries(at_anchor);
                }
                else    //this anchor is not supported
                    add_partition_entries(left);

                add_partition_entries(down);
            }
        }
        else    //then this is a supported, non-strict anchor, and we have to split or merge equivalence classes, but lazy updates might cause some swaps
        {
            if(mesh->verbosity >= 9) { qDebug() << " == non-strict anchor crossed =="; }

            xiMatrixEntry* at_anchor = down;
            xiMatrixEntry* generator = at_anchor->down;

            if(generator != NULL)   //then the class is vertical
            {
                if(generator->low_class_size != -1)    //then merge classes
                {
                    //map simplices from unsorted bin to generator
                    at_anchor->move_bin_here( xi_matrix.get_row_bin(at_anchor->y) );    //also updates at_anchor counters

                    //update data structures for merge
                    at_anchor->low_class_size = at_anchor->low_count + generator->low_class_size;
                    at_anchor->high_class_size = at_anchor->high_count + generator->high_class_size;
                    generator->low_class_size = -1;    //indicates that this xiMatrixEntry is NOT the head of an equivalence class

                    remove_partition_entries(generator);
                    add_partition_entries(at_anchor);  //this is necessary in case the class was previously empty
                }
                else    //then split classes
                {
                    //find simplices in the unsorted bin that must be mapped to the anchor
                    swap_counter += move_columns_from_bin_vertical(xi_matrix.get_col_bin(at_anchor->x), at_anchor, R_low, U_low, R_high, U_high);

                    //update data structures for split
                    generator->low_index = at_anchor->low_index - at_anchor->low_count;
                    generator->low_class_size = at_anchor->low_class_size - at_anchor->low_count;
                    at_anchor->low_class_size = at_anchor->low_count;
                    generator->high_index = at_anchor->high_index - at_anchor->high_count;
                    generator->high_class_size = at_anchor->high_class_size - at_anchor->high_count;
                    at_anchor->high_class_size = at_anchor->high_count;

                    xiMatrixEntry* row_bin = xi_matrix.get_row_bin(at_anchor->y);
                    row_bin->low_index = generator->low_index;
                    row_bin->high_index = generator->high_index;

                    remove_partition_entries(at_anchor);   //this is necessary because the class corresponding
                    add_partition_entries(at_anchor);      //  to at_anchor might have become empty
                    add_partition_entries(generator);
                }
            }
            else    //then the class is horizontal
            {
                generator = at_anchor->left;

                if(generator->low_class_size != -1)    //then merge classes
                {
                    //map simplices from unsorted bin to generator
                    at_anchor->move_bin_here( xi_matrix.get_col_bin(at_anchor->x) );

                    //update data structures for merge
                    at_anchor->low_class_size = at_anchor->low_count + generator->low_class_size;
                    at_anchor->high_class_size = at_anchor->high_count + generator->high_class_size;
                    generator->low_class_size = -1;    //indicates that this xiMatrixEntry is NOT the head of an equivalence class

                    remove_partition_entries(generator);
                    add_partition_entries(at_anchor);  //this is necessary in case the class was previously empty
                }
                else    //then split classes
                {
                    //find simplices in the unsorted bin that must be mapped to the anchor
                    swap_counter += move_columns_from_bin_horizontal(xi_matrix.get_row_bin(at_anchor->y), at_anchor, R_low, U_low, R_high, U_high);

                    //update data structures for split
                    generator->low_index = at_anchor->low_index - at_anchor->low_count;
                    generator->low_class_size = at_anchor->low_class_size - at_anchor->low_count;
                    at_anchor->low_class_size = at_anchor->low_count;
                    generator->high_index = at_anchor->high_index - at_anchor->high_count;
                    generator->high_class_size = at_anchor->high_class_size - at_anchor->high_count;
                    at_anchor->high_class_size = at_anchor->high_count;

                    xiMatrixEntry* col_bin = xi_matrix.get_col_bin(at_anchor->x);
                    col_bin->low_index = generator->low_index;
                    col_bin->high_index = generator->high_index;

                    remove_partition_entries(at_anchor);   //this is necessary because the class corresponding
                    add_partition_entries(at_anchor);      //  to at_anchor might have become empty
                    add_partition_entries(generator);
                }
            }
        }

        //remember that we have crossed this anchor
        cur_anchor->toggle();

        //if this cell does not yet have a barcode template, then store it now
        Face* cur_face = (path[i])->get_face();
        if(!cur_face->has_been_visited())
            store_barcode_template(cur_face, R_low, R_high);

        //testing
        if(testing)
        {
//            qDebug() << "  Reduced matrix for low simplices:";
//            R_low->print();
//            R_low->check_lows();
//            qDebug() << "  Matrix U for low simplices:";
//            U_low->print();
//            qDebug() << "  Matrix D for low simplices:";
//            D_low->print();
//            qDebug() << "  Reduced matrix for high simplices:";
//            R_high->print();
//            R_high->check_lows();
//            qDebug() << "  Matrix U for high simplices:";
//            U_high->print();
//            qDebug() << "  Matrix D for high simplices:";
//            D_high->print();
//            qDebug() << "  Low partition: ";
//            for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_low.begin(); it != partition_low.end(); ++it)
//                qDebug() << "     " << it->first << "->" << it->second->index << ", ";
//            qDebug() << "  High partition: ";
//            for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_high.begin(); it != partition_high.end(); ++it)
//                qDebug() << "     " << it->first << "->" << it->second->index << ", ";
//            qDebug() << "  Barcode Template: ";
//            first_cell->get_barcode().print();
            check_low_matrix(R_low, U_low);
            check_high_matrix(R_high, U_high);
        }

        //print runtime data
        int step_time = steptimer.elapsed();
//        qDebug() << "    --> this step took" << step_time << "milliseconds and involved" << swap_counter << "transpositions";

        //store data for analysis
        total_transpositions += swap_counter;
//        swap_counters[i] = swap_counter;
//        crossing_times[i] = step_time;
        if(swap_counter > max_swaps)
            max_swaps = swap_counter;
        if(step_time > max_time)
            max_time = step_time;

    }//end path traversal

    //print runtime data
    qDebug() << "  --> path traversal and persistence updates took" << timer.elapsed() << "milliseconds and involved" << total_transpositions << "transpositions";

    ///TEMPORARY
//    qDebug() << "DATA: number of transpositions and runtime for each crossing:";
//    for(unsigned i=0; i<swap_counters.size(); i++)
//        qDebug().nospace() << swap_counters[i] << ", " << crossing_times[i];
    qDebug() << "DATA: max number of swaps per crossing:" << max_swaps << "; max time per crossing:" << max_time;


  // PART 4: CLEAN UP

    delete R_low;
    delete R_high;
    delete U_low;
    delete U_high;

}//end store_barcodes_lazy()

//computes and stores a barcode template in each 2-cell of mesh
//resets the matrices and does a standard persistence calculation for expensive crossings
void PersistenceUpdater::store_barcodes_with_reset(std::vector<Halfedge*>& path)
{
    QTime timer;    //for timing the computations

    // PART 1: GET THE BOUNDARY MATRICES WITH PROPER SIMPLEX ORDERING

    timer.start();

    //get multi-grade data in each dimension
    if(mesh->verbosity >= 6) { qDebug() << "  Mapping low simplices:"; }
    IndexMatrix* ind_low = bifiltration->get_index_mx(dim);    //can we improve this with something more efficient than IndexMatrix?
    std::vector<int> low_simplex_order;     //this will be a map : dim_index --> order_index for dim-simplices
    store_multigrades(ind_low, true, low_simplex_order);
    delete ind_low;

    if(mesh->verbosity >= 6) { qDebug() << "  Mapping high simplices:"; }
    IndexMatrix* ind_high = bifiltration->get_index_mx(dim + 1);    //again, could be improved?
    std::vector<int> high_simplex_order;     //this will be a map : dim_index --> order_index for (dim+1)-simplices
    store_multigrades(ind_high, false, high_simplex_order);
    delete ind_high;

    //testing only
//    std::cout << "== low_simplex_order: ";
//    for(int i=0; i<low_simplex_order.size(); i++)
//        std::cout << low_simplex_order[i] << ", ";
//    std::cout << "\n== high_simplex_order: ";
//    for(int i=0; i<high_simplex_order.size(); i++)
//        std::cout << high_simplex_order[i] << ", ";
//    std::cout << "\n";

    //get boundary matrices (R) and identity matrices (U) for RU-decomposition
    MapMatrix_Perm* R_low = bifiltration->get_boundary_mx(low_simplex_order);
    MapMatrix_Perm* R_high = bifiltration->get_boundary_mx(low_simplex_order, high_simplex_order);

    //print runtime data
    qDebug() << "  --> computing initial order on simplices and building the boundary matrices took" << timer.elapsed() << "milliseconds";


  // PART 2: INITIAL PERSISTENCE COMPUTATION (RU-decomposition)

    timer.start();

    MapMatrix_RowPriority_Perm* U_low = R_low->decompose_RU();
    MapMatrix_RowPriority_Perm* U_high = R_high->decompose_RU();

    qDebug() << "  --> computing the RU decomposition took" << timer.elapsed() << "milliseconds";

    //store the barcode template in the first cell
    Face* first_cell = mesh->topleft->get_twin()->get_face();
    store_barcode_template(first_cell, R_low, R_high);

    qDebug() << "Initial persistence computation in cell " << mesh->FID(first_cell);


  // PART 3: TRAVERSE THE PATH AND DO VINEYARD UPDATES

    qDebug() << "TRAVERSING THE PATH USING THE RESET ALGORITHM: path has" << path.size() << "steps";
    qDebug() << "                              ^^^^^^^^^^^^^^^";

    ///TODO: set the threshold dynamically
    unsigned long threshold = 10000000;     //if the number of swaps might exceed this threshold, then do a persistence calculation from scratch
    qDebug() << "reset threshold set to" << threshold;

    timer.start();

    ///TEMPORARY: data structures for analyzing the computation
    unsigned long total_transpositions = 0;
//    std::vector<unsigned long> swap_counters(path.size(),0);
//    std::vector<int> crossing_times(path.size(),0);
    unsigned long max_swaps = 0;
    int max_time = 0;

    //traverse the path
    QTime steptimer;
    for(unsigned i=0; i<path.size(); i++)
    {
        steptimer.start();          //time update at each step of the path
        unsigned long swap_counter = 0;   //counts number of transpositions at each step

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

            //estimate how many swaps will occur
            unsigned long swap_estimate = static_cast<unsigned long>(left->low_class_size) * static_cast<unsigned long>(down->low_class_size)
                    + static_cast<unsigned long>(left->high_class_size) * static_cast<unsigned long>(down->high_class_size);

            //process the swaps
            if(cur_anchor->is_above()) //then anchor is crossed from below to above
            {
                if(mesh->verbosity >= 9) { qDebug() << " == strict anchor crossed below to above =="; }

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
                    remove_partition_entries(left);     //this block of the partition will move

                remove_partition_entries(down);         //this block of the partition will move

                //now move the columns
                swap_counter += move_columns(down, left, true, R_low, U_low, R_high, U_high);

                //post-move updates to equivalance class info
                if(at_anchor != NULL)  //this anchor is supported
                {
                    at_anchor->low_class_size = at_anchor->low_count + down->low_class_size;
                    at_anchor->high_class_size = at_anchor->high_count + down->high_class_size;
                    down->low_class_size = -1;  //this xiMatrixEntry is no longer the head of an equivalence class
                    add_partition_entries(at_anchor);
                }
                else    //this anchor is not supported
                    add_partition_entries(down);        //this block of the partition moved

                add_partition_entries(left);            //this block of the partition moved
            }
            else    //then anchor is crossed from above to below
            {
                if(mesh->verbosity >= 9) { qDebug() << " == strict anchor crossed above to below =="; }

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
                    remove_partition_entries(down);     //this block of the partition will move

                remove_partition_entries(left);         //this block of the partition will move

                //now move the columns
                swap_counter += move_columns(left, down, false, R_low, U_low, R_high, U_high);

                //post-move updates to equivalance class info
                if(at_anchor != NULL)  //this anchor is supported
                {
                    at_anchor->low_class_size = at_anchor->low_count + left->low_class_size;
                    at_anchor->high_class_size = at_anchor->high_count + left->high_class_size;
                    left->low_class_size = -1;  //this xiMatrixEntry is no longer the head of an equivalence class
                    add_partition_entries(at_anchor);
                }
                else    //this anchor is not supported
                    add_partition_entries(left);        //this block of the partition moved

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

        //if this cell does not yet have a barcode template, then store it now
        Face* cur_face = (path[i])->get_face();
        if(!cur_face->has_been_visited())
            store_barcode_template(cur_face, R_low, R_high);

        //print runtime data
        int step_time = steptimer.elapsed();
//        qDebug() << "    --> this step took" << step_time << "milliseconds and involved" << stepcounter << "transpositions";

        //store data for analysis
        total_transpositions += swap_counter;
//        swap_counters[i] = swap_counter;
//        crossing_times[i] = step_time;
        if(swap_counter > max_swaps)
            max_swaps = swap_counter;
        if(step_time > max_time)
            max_time = step_time;

    }//end path traversal

    //print runtime data
    qDebug() << "  --> path traversal and persistence updates took" << timer.elapsed() << "milliseconds and involved" << total_transpositions << "transpositions";

    ///TEMPORARY
//    qDebug() << "DATA: number of transpositions and runtime for each crossing:";
//    for(unsigned i=0; i<swap_counters.size(); i++)
//        qDebug().nospace() << swap_counters[i] << ", " << crossing_times[i];
    qDebug() << "DATA: max number of swaps per crossing:" << max_swaps << "; max time per crossing:" << max_time;


  // PART 4: CLEAN UP

    delete R_low;
    delete R_high;
    delete U_low;
    delete U_high;

}//end store_barcodes_with_reset()


//stores multigrade info for the persistence computations (data structures prepared with respect to a near-vertical line positioned to the right of all \xi support points)
//  low is true for simplices of dimension hom_dim, false for simplices of dimension hom_dim+1
//  simplex_order will be filled with a map : dim_index --> order_index for simplices of the given dimension, for use in creating the boundary matrices
void PersistenceUpdater::store_multigrades(IndexMatrix* ind, bool low, std::vector<int>& simplex_order)
{
    if(mesh->verbosity >= 6) { qDebug() << "STORING MULTIGRADES: low =" << low; }

  //STEP 1: store multigrade data in the xiSupportMatrix

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

                    //now map the multigrade to the element given by the iterator
                    (*it)->add_multigrade(x, y, last_col - first_col, last_col, low);

                    if(mesh->verbosity >= 6) { qDebug() << "    simplices at (" << x << "," << y << "), in columns" << (first_col + 1) << "to" << last_col << ", mapped to xi support point (" << (*it)->x << ", " << (*it)->y << ")"; }
                }
            }
        }//end x loop
    }//end y loop


  //STEP 2: update index data for each row in the xiSupportMatrix AND create a map : dim_index --> order_index for all simplices

    //we will create the map starting by identifying the order index of each simplex, starting with the last simplex
    int o_index = ind->last();
    simplex_order.resize(o_index + 1);

    //first consider all simplices that map to the xiMatrixEntry infinity
    xiMatrixEntry* cur = xi_matrix.get_infinity();
    std::list<Multigrade*>* mgrades = (low) ? &(cur->low_simplices) : &(cur->high_simplices);
    for(std::list<Multigrade*>::iterator it = mgrades->begin(); it != mgrades->end(); ++it)
    {
        Multigrade* mg = *it;
        if(mesh->verbosity >= 6) { qDebug() << "  multigrade (" << mg->x << "," << mg->y << ") at infinity has" << mg->num_cols << "simplices with last index" << mg->simplex_index<< "which will map to order_index" << o_index; }

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
                if(mesh->verbosity >= 6) { qDebug() << "  multigrade (" << mg->x << "," << mg->y << ") has" << mg->num_cols << "simplices with last dim_index" << mg->simplex_index << "which will map to order_index" << o_index; }

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
        xiMatrixEntry* cur_row = xi_matrix.get_row(row);
        if(cur_row->low_class_size != -1)
        {
            if(low)
                xi_matrix.get_row_bin(row)->low_index = cur_row->low_index - cur_row->low_class_size;
            else
                xi_matrix.get_row_bin(row)->high_index = cur_row->high_index - cur_row->high_class_size;
        }

    }//end for(row > 0)

}//end store_multigrades()


//moves columns from an equivalence class given by xiMatrixEntry* first to their new positions after or among the columns in the equivalence class given by xiMatrixEntry* second
// the boolean argument indicates whether an anchor is being crossed from below (or from above)
unsigned long PersistenceUpdater::move_columns(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
{
//    ///DEBUGGING
//    if(first->low_index + second->low_class_size != second->low_index || first->high_index + second->high_class_size != second->high_index)
//    {
//        qDebug() << "  ===>>> ERROR: swapping non-consecutive column blocks!";
//    }

    //get column indexes (so we know which columns to move)
    int low_col = first->low_index;   //rightmost column index of low simplices for the equivalence class to move
    int high_col = first->high_index; //rightmost column index of high simplices for the equivalence class to move

    //set column indexes for the first class to their final position
    first->low_index = second->low_index;
    first->high_index = second->high_index;

    //remember the "head" of the first equivalence class, so that we can update its class size
    xiMatrixEntry* first_head = first;

    //initialize counter
    unsigned long swap_counter = 0;

    //loop over all xiMatrixEntrys in the first equivalence class
    while(first != NULL)
    {
        //move all "low" simplices for this xiMatrixEntry (start with rightmost column, end with leftmost)
        std::list<Multigrade*>::iterator it = first->low_simplices.begin();
        while(it != first->low_simplices.end())
        {
            Multigrade* cur_grade = *it;

            if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
                //then move columns at cur_grade past columns at xiMatrixEntry second; map F does not change ( F : multigrades --> xiSupportElements )
            {
                swap_counter += move_low_columns(low_col, cur_grade->num_cols, second->low_index, RL, UL, RH, UH);
                second->low_index -= cur_grade->num_cols;
                ++it;
            }
            else    //then move columns at cur_grade to some position in the equivalence class given by xiMatrixEntry second; map F changes
            {
                //determine where these columns map to under F
                xiMatrixEntry* target = second;
                int target_col = second->low_index - second->low_count;
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
                it = first->low_simplices.erase(it);    //NOTE: advances the iterator!!!

                //if target is not the leftmost entry in its equivalence class, then move columns at cur_grade to the block of columns for target
                if( (from_below && target->left != NULL) || (!from_below && target->down != NULL) )
                    swap_counter += move_low_columns(low_col, cur_grade->num_cols, target_col, RL, UL, RH, UH);
                //else, then the columns don't actually have to move

                //update column counts
                first->low_count -= cur_grade->num_cols;
                target->low_count += cur_grade->num_cols;

                //update equivalence class sizes
                first_head->low_class_size -= cur_grade->num_cols;
                second->low_class_size += cur_grade->num_cols;
            }

            //update column index
            low_col -= cur_grade->num_cols;
        }//end "low" simplex loop

        //move all "high" simplices for this xiMatrixEntry
        it = first->high_simplices.begin();
        while(it != first->high_simplices.end())
        {
            Multigrade* cur_grade = *it;

            if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
                //then move columns at cur_grade past columns at xiMatrixEntry second; map F does not change
            {
                swap_counter += move_high_columns(high_col, cur_grade->num_cols, second->high_index, RH, UH);
                second->high_index -= cur_grade->num_cols;
                ++it;
            }
            else    //then move columns at cur_grade to some position in the equivalence class given by xiMatrixEntry second; map F changes
            {
                //determine where these columns map to under F
                xiMatrixEntry* target = second;
                int target_col = second->high_index - second->high_count;
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

                //associate cur_grade with target
                target->insert_multigrade(cur_grade, false);
                it = first->high_simplices.erase(it);    //NOTE: advances the iterator!!!

                //if target is not the leftmost entry in its equivalence class, then move columns at cur_grade to the block of columns for target
                if( (from_below && target->left != NULL) || (!from_below && target->down != NULL) )
                    swap_counter += move_high_columns(high_col, cur_grade->num_cols, target_col, RH, UH);
                //else, then the columns don't actually have to move

                //update column counts
                first->high_count -= cur_grade->num_cols;
                target->high_count += cur_grade->num_cols;

                //update equivalence class sizes
                first_head->high_class_size -= cur_grade->num_cols;
                second->high_class_size += cur_grade->num_cols;
            }

            //update column index
            high_col -= cur_grade->num_cols;
        }//end "high" simplex loop

        //advance to the next xiMatrixEntry in the first equivalence class
        first = from_below ? first->down : first->left;
    }//end while

//    ///DEBUGGING
//    if(second->low_index + first_head->low_class_size != first_head->low_index || second->high_index + first_head->high_class_size != first_head->high_index)
//    {
//        qDebug() << "  ===>>> ERROR: swap resulted in non-consecutive column blocks!";
//    }
    return swap_counter;
}//end move_columns()

//for lazy updates -- moves columns from an equivalence class given by xiMatrixEntry* first either past the equivalence class given by xiMatrixEntry* second or into its bin
//  the boolean argument indicates whether an anchor is being crossed from below (or from above)
//  returns a count of the number of transpositions performed
unsigned long PersistenceUpdater::move_columns_lazy(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
{
    ///DEBUGGING
    if(first->low_index + second->low_class_size != second->low_index || first->high_index + second->high_class_size != second->high_index)
    {
        qDebug() << "  ===>>> ERROR: swapping non-consecutive column blocks!";
    }
//    if((first->x == 8 && second->y == 9) || (second->x == 8 && first->y == 9))
//    {

//        qDebug() << "at anchor (8,9)";
//    }

    //get column indexes (so we know which columns to move)
    int low_col = first->low_index;   //rightmost column index of low simplices for the equivalence class to move
    int high_col = first->high_index; //rightmost column index of high simplices for the equivalence class to move

    //set column indexes for the first class to their final positions
    first->low_index = second->low_index;
    first->high_index = second->high_index;

    //get the bins
    xiMatrixEntry* first_bin = from_below ? xi_matrix.get_col_bin(first->x) : xi_matrix.get_row_bin(first->y);
    xiMatrixEntry* second_bin = from_below ? xi_matrix.get_row_bin(second->y) : xi_matrix.get_col_bin(second->x);

    //initialize counter
    unsigned long swap_counter = 0;

    //loop over all xiMatrixEntrys in the first equivalence class
    xiMatrixEntry* cur_entry = first;   //NOTE: THIS IS DIFFERENT (AND CLEARER) THAN IN THE NON-LAZY VERSION
    while(cur_entry != NULL)
    {
        //move all "low" simplices for this xiMatrixEntry (start with rightmost column, end with leftmost)
        for(std::list<Multigrade*>::iterator it = cur_entry->low_simplices.begin(); it != cur_entry->low_simplices.end(); ) //NOTE: iterator advances in loop
        {
            Multigrade* cur_grade = *it;

            if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
                //then move columns at cur_grade past columns at xiMatrixEntry second
            {
                swap_counter += move_low_columns(low_col, cur_grade->num_cols, second->low_index, RL, UL, RH, UH);
                second->low_index -= cur_grade->num_cols;
                second_bin->low_index -= cur_grade->num_cols;
                ++it;
            }
            else    //then columns at cur_grade join the second bin (but they don't have to move)
            {
                second_bin->insert_multigrade(cur_grade, true);
                it = cur_entry->low_simplices.erase(it);    //NOTE: advances the iterator!!!
                cur_entry->low_count -= cur_grade->num_cols;
                second_bin->low_count += cur_grade->num_cols;
                first->low_class_size -= cur_grade->num_cols;
                second->low_class_size += cur_grade->num_cols;
            }

            //update column index
            low_col -= cur_grade->num_cols;
        }//end "low" simplex loop

        //move all "high" simplices for this xiMatrixEntry (start with rightmost column, end with leftmost)
        for(std::list<Multigrade*>::iterator it = cur_entry->high_simplices.begin(); it != cur_entry->high_simplices.end(); )   //NOTE: iterator advances in loop
        {
            Multigrade* cur_grade = *it;

            if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
                //then move columns at cur_grade past columns at xiMatrixEntry second
            {
                swap_counter += move_high_columns(high_col, cur_grade->num_cols, second->high_index, RH, UH);
                second->high_index -= cur_grade->num_cols;
                second_bin->high_index -= cur_grade->num_cols;
                ++it;
            }
            else    //then columns at cur_grade join the second bin
            {
                second_bin->insert_multigrade(cur_grade, false);
                it = cur_entry->high_simplices.erase(it);    //NOTE: advances the iterator!!!
                cur_entry->high_count -= cur_grade->num_cols;
                second_bin->high_count += cur_grade->num_cols;
                first->high_class_size -= cur_grade->num_cols;
                second->high_class_size += cur_grade->num_cols;
            }

            //update column index
            high_col -= cur_grade->num_cols;
        }//end "low" simplex loop

        //advance to the next xiMatrixEntry in the first equivalence class
        if(cur_entry != first_bin)  //then look for the next entry in the equivalence class
        {
            xiMatrixEntry* prev_entry = cur_entry;
            cur_entry = from_below ? cur_entry->down : cur_entry->left;

            if(cur_entry == NULL)   //then there is no next entry, so we must advance to the bin
            {
                //if prev_entry is not an anchor, then we must check its other bin and move simplices from it (this corrects for the problem that occurs when crossing a non-anchor xi support point with nonempty bin)
                xiMatrixEntry* other_direction = from_below ? prev_entry->left : prev_entry->down;
                if(other_direction == NULL) //then prev_entry is not an anchor
                {
                    xiMatrixEntry* other_bin = from_below ? xi_matrix.get_row_bin(prev_entry->y) : xi_matrix.get_col_bin(prev_entry->x);
                    if(other_bin->low_count > 0 || other_bin->high_count > 0)
                    {
//                        qDebug() << "       adjusting bins for (" << prev_entry->x << "," << prev_entry->y << ")";
                        first_bin->move_bin_here(other_bin);
                    }
                }

                //now we can advance to the bin in the correct direction
                first_bin->low_index = second->low_index;       //set the bin indexes
                first_bin->high_index = second->high_index;     //  to their new positions
                cur_entry = first_bin;
            }
        }
        else    //then we just processed the bin, so we are now finished
            cur_entry = NULL;
    }//end xiMatrixEntry loop

    ///DEBUGGING
    if(second->low_index + first->low_class_size != first->low_index || second->high_index + first->high_class_size != first->high_index)
    {
        qDebug() << "  ===>>> ERROR: swap resulted in non-consecutive column blocks!";
    }
//    if((first->x == 8 && second->y == 9) || (second->x == 8 && first->y == 9))
//    {
//        qDebug() << "at anchor (8,9)";
//    }

    return swap_counter;
}//end move_columns_lazy()

//for lazy updates -- finds columns in the unsorted bin that should map to the anchor, and moves them
//  this version is for horizontal classes!
//  returns a count of the number of transpositions performed
unsigned long PersistenceUpdater::move_columns_from_bin_horizontal(xiMatrixEntry* bin, xiMatrixEntry* at_anchor, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
{
    unsigned threshold = at_anchor->left->x;
    unsigned long swap_counter = 0;

    //process "low" simplices
    int col = bin->low_index;
    for(std::list<Multigrade*>::iterator it = bin->low_simplices.begin(); it != bin->low_simplices.end(); ) //NOTE: iterator advances in loop
    {
        if( (*it)->x > threshold )    //then columns at this grade moves
        {
            swap_counter += move_low_columns(col, (*it)->num_cols, (at_anchor->low_index - at_anchor->low_count), RL, UL, RH, UH);

            at_anchor->insert_multigrade(*it, true);
            at_anchor->low_count += (*it)->num_cols;
            bin->low_count -= (*it)->num_cols;
            bin->low_index -= (*it)->num_cols;
            col -= (*it)->num_cols;
            it = bin->low_simplices.erase(it);  //NOTE: advances the iterator!
        }
        else    //then columns at this grade don't move
        {
            col -= (*it)->num_cols;
            ++it;
        }
    }

    //process "high" simplices
    col = bin->high_index;
    for(std::list<Multigrade*>::iterator it = bin->high_simplices.begin(); it != bin->high_simplices.end(); ) //NOTE: iterator advances in loop
    {
        if( (*it)->x > threshold )    //then columns at this grade moves
        {
            swap_counter += move_high_columns(col, (*it)->num_cols, (at_anchor->high_index - at_anchor->high_count), RH, UH);

            at_anchor->insert_multigrade(*it, false);
            at_anchor->high_count += (*it)->num_cols;
            bin->high_count -= (*it)->num_cols;
            bin->high_index -= (*it)->num_cols;
            col -= (*it)->num_cols;
            it = bin->high_simplices.erase(it);  //NOTE: advances the iterator!
        }
        else    //then columns at this grade don't move
        {
            col -= (*it)->num_cols;
            ++it;
        }
    }

    return swap_counter;
}//end move_columns_from_bin_horizontal()

//for lazy updates -- finds columns in the unsorted bin that should map to the anchor, and moves them
//  this version is for vertical classes!
//  returns a count of the number of transpositions performed
unsigned long PersistenceUpdater::move_columns_from_bin_vertical(xiMatrixEntry* bin, xiMatrixEntry* at_anchor, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
{
    unsigned threshold = at_anchor->down->y;
    unsigned long swap_counter = 0;

    //process "low" simplices
    int col = bin->low_index;
    for(std::list<Multigrade*>::iterator it = bin->low_simplices.begin(); it != bin->low_simplices.end(); ) //NOTE: iterator advances in loop
    {
        if( (*it)->y > threshold )    //then columns at this grade moves
        {
            swap_counter += move_low_columns(col, (*it)->num_cols, (at_anchor->low_index - at_anchor->low_count), RL, UL, RH, UH);

            at_anchor->insert_multigrade(*it, true);
            at_anchor->low_count += (*it)->num_cols;
            bin->low_count -= (*it)->num_cols;
            bin->low_index -= (*it)->num_cols;
            col -= (*it)->num_cols;
            it = bin->low_simplices.erase(it);  //NOTE: advances the iterator!
        }
        else    //then columns at this grade don't move
        {
            col -= (*it)->num_cols;
            ++it;
        }
    }

    //process "high" simplices
    col = bin->high_index;
    for(std::list<Multigrade*>::iterator it = bin->high_simplices.begin(); it != bin->high_simplices.end(); ) //NOTE: iterator advances in loop
    {
        if( (*it)->y > threshold )    //then columns at this grade moves
        {
            swap_counter += move_high_columns(col, (*it)->num_cols, (at_anchor->high_index - at_anchor->high_count), RH, UH);

            at_anchor->insert_multigrade(*it, false);
            at_anchor->high_count += (*it)->num_cols;
            bin->high_count -= (*it)->num_cols;
            bin->high_index -= (*it)->num_cols;
            col -= (*it)->num_cols;
            it = bin->high_simplices.erase(it);  //NOTE: advances the iterator!
        }
        else    //then columns at this grade don't move
        {
            col -= (*it)->num_cols;
            ++it;
        }
    }

    return swap_counter;
}//end move_columns_from_bin_vertical()

//moves a block of n columns, the rightmost of which is column s, to a new position following column t (NOTE: assumes s <= t)
unsigned long PersistenceUpdater::move_low_columns(int s, unsigned n, int t, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
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
unsigned long PersistenceUpdater::move_high_columns(int s, unsigned n, int t, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
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
            if(mesh->verbosity >= 9) { qDebug() << "(" << a << "," << b << ")"; }

            bool a_pos = (RH->low(a) == -1);    //true iff simplex corresponding to column a is positive
            bool b_pos = (RH->low(b) == -1);    //true iff simplex corresponding to column b is positive

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
void PersistenceUpdater::update_order_and_reset_matrices(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
{
  //STEP 1: update the lift map for all multigrades, storing the current column index for each multigrade

    //get column indexes (so we know which columns to move)
    int low_col = first->low_index;   //rightmost column index of low simplices for the equivalence class to move
    int high_col = first->high_index; //rightmost column index of high simplices for the equivalence class to move

    //set column indexes for the first class to their final position
    first->low_index = second->low_index;
    first->high_index = second->high_index;

    //loop over all xiMatrixEntrys in the first equivalence class
    xiMatrixEntry* cur_entry = first;
    while(cur_entry != NULL)
    {
        //move all "low" simplices for this xiMatrixEntry (start with rightmost column, end with leftmost)
        for(std::list<Multigrade*>::iterator it = first->low_simplices.begin(); it != first->low_simplices.end(); ) //NOTE: iterator advances in loop
        {
            Multigrade* cur_grade = *it;

            //remember current position of this grade
            cur_grade->simplex_index = ???

            ///TODO: improve this!
            if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
                //then move columns at cur_grade past columns at xiMatrixEntry second; lift map does not change ( lift : multigrades --> xiSupportElements )
            {
                ///FIX THIS: swap_counter += move_low_columns(low_col, cur_grade->num_cols, second->low_index, RL, UL, RH, UH);
                second->low_index -= cur_grade->num_cols;
                ++it;
            }
            else    //then move columns at cur_grade to some position in the equivalence class given by xiMatrixEntry second; lift map changes
            {
                //determine the new lift map for these columns
                xiMatrixEntry* target = second;
                int target_col = second->low_index - second->low_count;
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
                it = first->low_simplices.erase(it);    //NOTE: advances the iterator!!!

                //if target is not the leftmost entry in its equivalence class, then move columns at cur_grade to the block of columns for target
                if( (from_below && target->left != NULL) || (!from_below && target->down != NULL) )
                    ///FIX THIS: swap_counter += move_low_columns(low_col, cur_grade->num_cols, target_col, RL, UL, RH, UH);
                //else, then the columns don't actually have to move

                //update column counts
                first->low_count -= cur_grade->num_cols;
                target->low_count += cur_grade->num_cols;

                //update equivalence class sizes
                first_head->low_class_size -= cur_grade->num_cols;
                second->low_class_size += cur_grade->num_cols;
            }

            //update column index
            low_col -= cur_grade->num_cols;
        }//end "low" simplex loop

        ///TODO: high simplices



        //advance to the next xiMatrixEntry in the first equivalence class
        cur_entry = from_below ? cur_entry->down : cur_entry->left;
    }


  //STEP 2: traverse grades in the new order and build the new order on matrix columns


    //re-build the matrix R based on the new order


    //compute the new RU-decomposition


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
//    QDebug qd = qDebug().nospace();
//    qd << "  -----barcode: ";

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

//                qd << "(" << c << "," << s << ")-->(" << a << "," << b << ") ";

                if(a != b)  //then we have a bar of positive length
                    dbc.add_bar(a, b);
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

