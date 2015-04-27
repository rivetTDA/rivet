#include "barcode_calculator.h"

#include <qdebug.h>


//constructor -- also fills xi_matrix with the xi support points
BarcodeCalculator::BarcodeCalculator(Mesh *m, MultiBetti &mb, std::vector<xiPoint> &xi_pts) :
    mesh(m), bifiltration(mb.bifiltration), dim(mb.dimension),
    xi_matrix(m->x_grades.size(), m->y_grades.size())
{
    //fill the xiSupportMatrix with the xi support points
    xi_matrix.fill(mb, xi_pts);

    //create partition entries for infinity (which never change)
    unsigned infty = -1;    // = MAX_UNSIGNED, right?
    partition_low.insert( std::pair<unsigned, xiMatrixEntry*>(infty, xi_matrix.get_infinity()) );
    partition_high.insert( std::pair<unsigned, xiMatrixEntry*>(infty, xi_matrix.get_infinity()) );
}


//computes anchors and stores them in mesh->all_lcms; anchor-lines will be created when mesh->build_interior() is called
void BarcodeCalculator::find_anchors()
{
    if(mesh->verbosity >= 2) { std::cout << "Finding anchors...\n"; }

    //get pointers to top entries in nonempty columns
    std::list<xiMatrixEntry*> nonempty_cols;
    for(unsigned i = 0; i < mesh->x_grades.size(); i++)
    {
        xiMatrixEntry* col_entry = xi_matrix.get_col(i); //top entry in row j, possibly NULL
        if(col_entry != NULL)
            nonempty_cols.push_front(col_entry);
    }

    //compute and store LCMs
    for(int j = mesh->y_grades.size() - 1; j >= 0; j--)  //loop through all rows, top to bottom
    {
        xiMatrixEntry* row_entry = xi_matrix.get_row(j); //rightmost entry in row j, possibly NULL

        std::list<xiMatrixEntry*>::iterator it = nonempty_cols.begin();
        while( it != nonempty_cols.end() )  //loop through all nonempty columns, right to left
        {
            if(row_entry == NULL)   //then there is nothing else in this row
                break;

            //check if there is a LCM in position (i,j)
            xiMatrixEntry* col_entry = *it;
            if(col_entry == NULL)
                 qDebug() << "ERROR in Mesh::store_xi_points() : NULL col_entry";
            if(row_entry != col_entry)  //then there is a strict, non-supported anchor at (col_entry->x, row_entry->y)
            {
                mesh->all_lcms.insert(new LCM(col_entry, row_entry));
                if(mesh->verbosity >= 4) { std::cout << "  anchor (strict, non-supported) found at (" << col_entry->x << ", " << row_entry->y << ")\n"; }
            }
            else    //then row_entry == col_entry, so there might be a supported anchor at (col_entry->x, row_entry->y), or there might be no anchor here
            {
                if(col_entry->down != NULL && row_entry->left != NULL)  //then there is a strict and supported anchor
                {
                    mesh->all_lcms.insert(new LCM(col_entry, true));
                    if(mesh->verbosity >= 4) { std::cout << "  anchor (strict and supported) found at (" << col_entry->x << ", " << col_entry->y << ")\n"; }
                }
                else if(col_entry->down != NULL || row_entry->left != NULL)  //then there is a supported, non-strict anchor
                {
                    mesh->all_lcms.insert(new LCM(col_entry, false));
                    if(mesh->verbosity >= 4) { std::cout << "  anchor (supported, non-strict) found at (" << col_entry->x << ", " << col_entry->y << ")\n"; }
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


//computes and stores a discrete barcode in each 2-cell of mesh
void BarcodeCalculator::store_barcodes(std::vector<Halfedge*>& path)
{
  // PART 1: GET THE BOUNDARY MATRICES WITH PROPER SIMPLEX ORDERING

    //get multi-grade data in each dimension
    if(mesh->verbosity >= 4) { std::cout << "Mapping low simplices:\n"; }
    IndexMatrix* ind_low = bifiltration->get_index_mx(dim);    //can we improve this with something more efficient than IndexMatrix?
    std::vector<int> low_simplex_order;     //this will be a map : dim_index --> order_index for dim-simplices
    store_multigrades(ind_low, true, low_simplex_order);
    delete ind_low;

    if(mesh->verbosity >= 4) { std::cout << "Mapping high simplices:\n"; }
    IndexMatrix* ind_high = bifiltration->get_index_mx(dim + 1);    //again, could be improved?
    std::vector<int> high_simplex_order;     //this will be a map : dim_index --> order_index for (dim+1)-simplices
    store_multigrades(ind_high, false, high_simplex_order);
    delete ind_high;

    //testing only
    std::cout << "== low_simplex_order: ";
    for(int i=0; i<low_simplex_order.size(); i++)
        std::cout << low_simplex_order[i] << ", ";
    std::cout << "\n== high_simplex_order: ";
    for(int i=0; i<high_simplex_order.size(); i++)
        std::cout << high_simplex_order[i] << ", ";
    std::cout << "\n";

    //get boundary matrices (R) and identity matrices (U) for RU-decomposition
    MapMatrix_Perm* R_low = bifiltration->get_boundary_mx(low_simplex_order);
    MapMatrix_Perm* R_high = bifiltration->get_boundary_mx(low_simplex_order, high_simplex_order);

    if(mesh->verbosity >= 4)
    {
        std::cout << "  Boundary matrix for low simplices:\n";
        R_low->print();
        std::cout << "  Boundary matrix for high simplices:\n";
        R_high->print();
    }
    /// TESTING ONLY
    D = bifiltration->get_boundary_mx(low_simplex_order, high_simplex_order);


  // PART 2: INITIAL PERSISTENCE COMPUTATION (RU-decomposition)


    MapMatrix_RowPriority_Perm* U_low = R_low->decompose_RU();
    MapMatrix_RowPriority_Perm* U_high = R_high->decompose_RU();

    //store the discrete barcode in the first cell
    Face* first_cell = mesh->topleft->get_twin()->get_face();
    store_discrete_barcode(first_cell, R_low, R_high);

    std::cout << "Initial persistence computation in cell " << mesh->FID(first_cell) << ".\n";

    if(mesh->verbosity >= 4)
    {
        std::cout << "  Reduced matrix for low simplices:\n";
        R_low->print();
        R_low->check_lows();
        std::cout << "  Matrix U for low simplices:\n";
        U_low->print();
        std::cout << "  Reduced matrix for high simplices:\n";
        R_high->print();
        R_high->check_lows();
        std::cout << "  Matrix U for high simplices:\n";
        U_high->print();
        std::cout << "  Low partition: ";
        for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_low.begin(); it != partition_low.end(); ++it)
            std::cout << it->first << "->" << it->second->index << ", ";
        std::cout << "\n  High partition: ";
        for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_high.begin(); it != partition_high.end(); ++it)
            std::cout << it->first << "->" << it->second->index << ", ";
        std::cout << "\n  Discrete barcode: ";
        first_cell->get_barcode().print();
    }


  // PART 3: TRAVERSE THE PATH AND DO VINEYARD UPDATES

    //traverse the path
    for(unsigned i=0; i<path.size(); i++)
    {
        //determine which anchor is represented by this edge
        LCM* cur_lcm = (path[i])->get_LCM();

        std::cout << "Step " << i << " of the path: crossing LCM at (" << cur_lcm->get_x() << "," << cur_lcm->get_y() << ") into cell " << mesh->FID((path[i])->get_face()) << ".\n";

        //get equivalence classes for this LCM
        xiMatrixEntry* down = cur_lcm->get_down();
        xiMatrixEntry* left = cur_lcm->get_left();

        //if this is a strict anchor, then swap simplices
        if(left != NULL) //then this is a strict anchor and some simplices swap
        {
            xiMatrixEntry* at_LCM = NULL;   //remains NULL iff this anchor is not supported

            if(down == NULL)    //then this is also a supported anchor
            {
                at_LCM = left;
                down = left->down;
                left = left->left;
            }//now down and left are correct (and should not be NULL)

            if(cur_lcm->is_above()) //then anchor is crossed from below to above
            {
                std::cout << " == strict anchor crossed below to above ==\n";

                //pre-move updates to equivalence class info
                if(at_LCM != NULL)  //this anchor is supported
                {
                    left->low_index = at_LCM->low_index - at_LCM->low_count;                //necessary since low_index, low_class_size,
                    left->low_class_size = at_LCM->low_class_size - at_LCM->low_count;      //  high_index, and high_class_size
                    left->high_index = at_LCM->high_index - at_LCM->high_count;             //  are only reliable for the head
                    left->high_class_size = at_LCM->high_class_size - at_LCM->high_count;   //  of each equivalence class
                    remove_partition_entries(at_LCM);   //this partition might become empty
                }
                else    //this anchor is not supported
                    remove_partition_entries(left);     //this partition will move

                remove_partition_entries(down);         //this partition will move

                //now move the columns
                move_columns(down, left, true, R_low, U_low, R_high, U_high);

                //most-move updates to equivalance class info
                if(at_LCM != NULL)  //this anchor is supported
                {
                    at_LCM->low_class_size = at_LCM->low_count + down->low_class_size;
                    at_LCM->high_class_size = at_LCM->high_count + down->high_class_size;
                    down->low_class_size = -1;  //this xiMatrixEntry is no longer the head of an equivalence class
                    add_partition_entries(at_LCM);
                }
                else    //this anchor is not supported
                    add_partition_entries(down);

                add_partition_entries(left);
            }
            else    //then LCM is crossed from above to below
            {
                std::cout << " == strict anchor crossed above to below ==\n";

                //pre-move updates to equivalence class info
                if(at_LCM != NULL)
                {
                    down->low_index = at_LCM->low_index - at_LCM->low_count;                //necessary since low_index, low_class_size,
                    down->low_class_size = at_LCM->low_class_size - at_LCM->low_count;      //  high_index, and high_class_size
                    down->high_index = at_LCM->high_index - at_LCM->high_count;             //  are only reliable for the head
                    down->high_class_size = at_LCM->high_class_size - at_LCM->high_count;   //  of each equivalence class
                    remove_partition_entries(at_LCM);   //this partition might become empty
                }
                else    //this anchor is not supported
                    remove_partition_entries(down);     //this partition will move

                remove_partition_entries(left);         //this partition will move

                //now move the columns
                move_columns(left, down, false, R_low, U_low, R_high, U_high);

                //post-move updates to equivalance class info
                if(at_LCM != NULL)  //this anchor is supported
                {
                    at_LCM->low_class_size = at_LCM->low_count + left->low_class_size;
                    at_LCM->high_class_size = at_LCM->high_count + left->high_class_size;
                    left->low_class_size = -1;  //this xiMatrixEntry is no longer the head of an equivalence class
                    add_partition_entries(at_LCM);
                }
                else    //this anchor is not supported
                    add_partition_entries(left);

                add_partition_entries(down);
            }
        }
        else    //then this is a supported, non-strict anchor, and we just have to split or merge equivalence classes
        {
            xiMatrixEntry* at_LCM = down;
            xiMatrixEntry* generator = at_LCM->down;
            if(generator == NULL)
                generator = at_LCM->left;

            if(generator->low_class_size != -1)    //then merge classes
            {
                at_LCM->low_class_size = at_LCM->low_count + generator->low_class_size;
                at_LCM->high_class_size = at_LCM->high_count + generator->high_class_size;
                generator->low_class_size = -1;    //indicates that this xiMatrixEntry is NOT the head of an equivalence class

                remove_partition_entries(generator);
                add_partition_entries(at_LCM);  //this is necessary in case the class was previously empty
            }
            else    //then split classes
            {
                generator->low_index = at_LCM->low_index - at_LCM->low_count;
                generator->low_class_size = at_LCM->low_class_size - at_LCM->low_count;
                at_LCM->low_class_size = at_LCM->low_count;
                generator->high_index = at_LCM->high_index - at_LCM->high_count;
                generator->high_class_size = at_LCM->high_class_size - at_LCM->high_count;
                at_LCM->high_class_size = at_LCM->high_count;

                remove_partition_entries(at_LCM);   //this is necessary because the class corresponding
                add_partition_entries(at_LCM);      //  to at_LCM might have become empty
                add_partition_entries(generator);
            }
        }

        //remember that we have crossed this anchor
        cur_lcm->toggle();

        //testing
        std::cout << "  LCMS above the current line: ";
        for(std::set<LCM*, LCM_LeftComparator>::iterator it = mesh->all_lcms.begin(); it != mesh->all_lcms.end(); ++it)
            if((*it)->is_above())
                std::cout << "(" << (*it)->get_x() << "," << (*it)->get_y() << ") ";
        std::cout << "\n";

        //if this cell does not yet have a discrete barcode, then store the discrete barcode here
        Face* cur_face = (path[i])->get_face();
        if(!cur_face->has_been_visited())
            store_discrete_barcode(cur_face, R_low, R_high);

        //testing
        if(mesh->verbosity >= 4)
        {
            std::cout << "  Reduced matrix for low simplices:\n";
            R_low->print();
            R_low->check_lows();
            std::cout << "  Matrix U for low simplices:\n";
            U_low->print();
            std::cout << "  Reduced matrix for high simplices:\n";
            R_high->print();
            R_high->check_lows();
            std::cout << "  Matrix U for high simplices:\n";
            U_high->print();
//            std::cout << "  Matrix D for high simplices:\n";
//            D->print();
            std::cout << "  Low partition: ";
            for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_low.begin(); it != partition_low.end(); ++it)
                std::cout << it->first << "->" << it->second->index << ", ";
            std::cout << "\n  High partition: ";
            for(std::map<unsigned, xiMatrixEntry*>::iterator it = partition_high.begin(); it != partition_high.end(); ++it)
                std::cout << it->first << "->" << it->second->index << ", ";
            std::cout << "\n  Discrete barcode: ";
            cur_face->get_barcode().print();
        }

    }//end path traversal


  // PART 4: CLEAN UP

    delete R_low;
    delete R_high;
    delete U_low;
    delete U_high;

}//end store_persistence_data()


//stores multigrade info for the persistence computations (data structures prepared with respect to a near-vertical line positioned to the right of all \xi support points)
//  low is true for simplices of dimension hom_dim, false for simplices of dimension hom_dim+1
//  simplex_order will be filled with a map : dim_index --> order_index for simplices of the given dimension, for use in creating the boundary matrices
void BarcodeCalculator::store_multigrades(IndexMatrix* ind, bool low, std::vector<int>& simplex_order)
{
  //STEP 1: store multigrade data in the xiSupportMatrix

    //initialize linked list to track the "frontier"
    typedef std::list<xiMatrixEntry*> Frontier;
    Frontier frontier;

    //loop through rows of xiSupportMatrix, from top to bottom
    for(int y = ind->height() - 1; y >= 0; y--)
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
        for(int x = ind->width() - 1; x >= 0; x--)
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
//                    qDebug() << "      processing columns" << first_col << "to" << last_col << ": map to infinity";
                    xi_matrix.get_infinity()->add_multigrade(x, y, last_col - first_col, last_col, low);
                    if(mesh->verbosity >= 4) { std::cout << "    simplices at (" << x << ", " << y << "), in columns " << (first_col + 1) << " to " << last_col << ", mapped to infinity\n"; }
                }
                else    //then map multigrade (x,y) to the last element of the frontier such that (*it)->x >= x
                {
//                    qDebug() << "      processing columns" << first_col << "to" << last_col << ": map to finite point";
                    //advance the iterator to the first element of the frontier such that (*it)->x < x
                    while( it != frontier.end() && (*it)->x >= x )
                        ++it;

                    //back up one position, to the last element of the frontier such that (*it)->x >= x
                    --it;

                    //now map the multigrade to the element given by the iterator
                    (*it)->add_multigrade(x, y, last_col - first_col, last_col, low);
                    if(mesh->verbosity >= 4) { std::cout << "    simplices at (" << x << ", " << y << "), in columns " << (first_col + 1) << " to " << last_col << ", mapped to xi support point (" << (*it)->x << ", " << (*it)->y << ")\n"; }
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
        std::cout << "  multigrade (" << mg->x << "," << mg->y << ") at infinity has " << mg->num_cols << " simplices with last index " << mg->simplex_index << "\n";

        for(unsigned s=0; s < mg->num_cols; s++)  // simplex with dim_index (mg->simplex_inded - s) has order_index o_index
        {
            std::cout << "   -- simplex with dim_index " << (mg->simplex_index - s) << " has order_index " << o_index << "\n";
            simplex_order[mg->simplex_index - s] = o_index;
            o_index--;
        }
    }

    //now loop over all xiMatrixEntries in backwards reverse lexicographical order
    for(unsigned row = xi_matrix.height(); row > 0; )  //since row is unsigned, it will be one more than the current row, and the decrement operator appears inside the loop
    {
        cur = xi_matrix.get_row(--row);
        if(cur == NULL)
            continue;

        //if we get here, the current row is nonempty, so it forms an equivalence class in the partition

        //store index of rightmost column that is mapped to this equivalence class
        int* cur_ind = (low) ? &(cur->low_index) : &(cur->high_index);
        *cur_ind = o_index;

        //consider all xiMatrixEntries in this row
        while(cur != NULL)
        {
            //store map values for all simplices at all multigrades at this xiMatrixEntry
            mgrades = (low) ? &(cur->low_simplices) : &(cur->high_simplices);
            for(std::list<Multigrade*>::iterator it = mgrades->begin(); it != mgrades->end(); ++it)
            {
                Multigrade* mg = *it;
                std::cout << "  multigrade (" << mg->x << "," << mg->y << ") has " << mg->num_cols << " simplices with last index " << mg->simplex_index << "\n";

                for(unsigned s=0; s < mg->num_cols; s++)  // simplex with dim_index (mg->simplex_inded - s) has order_index o_index
                {
                    std::cout << "   -- simplex with dim_index " << (mg->simplex_index - s) << " has order_index " << o_index << "\n";
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
    }//end for(row > 0)

}//end store_multigrades()


//moves columns from an equivalence class given by xiMatrixEntry* first to their new positions after or among the columns in the equivalence class given by xiMatrixEntry* second
// the boolean argument indicates whether an LCM is being crossed from below (or from above)
///TODO: IMPLEMENT LAZY SWAPPING!
void BarcodeCalculator::move_columns(xiMatrixEntry* first, xiMatrixEntry* second, bool from_below, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
{
    //get column indexes (so we know which columns to move)
    int low_col = first->low_index;   //rightmost column index of low simplices for the equivalence class to move
    int high_col = first->high_index; //rightmost column index of high simplices for the equivalence class to move

    //set column indexes for the first class to their final position
    first->low_index = second->low_index;
    first->high_index = second->high_index;

    //remember the "head" of the first equivalence class, so that we can update its class size
    xiMatrixEntry* first_head = first;

    //loop over all xiMatrixEntrys in the first equivalence class
    while(first != NULL)
    {
        //move all "low" simplices for this xiMatrixEntry
        std::list<Multigrade*>::iterator it = first->low_simplices.begin();
        while(it != first->low_simplices.end())
        {
            Multigrade* cur_grade = *it;

            if( (from_below && cur_grade->x > second->x) || (!from_below && cur_grade->y > second->y) )
                //then move columns at cur_grade past columns at xiMatrixEntry second; map F does not change ( F : multigrades --> xiSupportElements )
            {
                move_low_columns(low_col, cur_grade->num_cols, second->low_index, RL, UL, RH, UH);
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
                cur_grade->xi_entry = target;
                target->insert_multigrade(cur_grade, true);
                first->low_simplices.erase(it);    //NOTE: advances the iterator!!!

                //if target is not the leftmost entry in its equivalence class, then move columns at cur_grade to the block of columns for target
                if( (from_below && target->left != NULL) || (!from_below && target->down != NULL) )
                    move_low_columns(low_col, cur_grade->num_cols, target_col, RL, UL, RH, UH);
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
                move_high_columns(high_col, cur_grade->num_cols, second->high_index, RH, UH);
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
                cur_grade->xi_entry = target;
                target->insert_multigrade(cur_grade, true);
                first->high_simplices.erase(it);    //NOTE: advances the iterator!!!

                //if target is not the leftmost entry in its equivalence class, then move columns at cur_grade to the block of columns for target
                if( (from_below && target->left != NULL) || (!from_below && target->down != NULL) )
                    move_high_columns(high_col, cur_grade->num_cols, target_col, RH, UH);
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
        if(from_below)
            first = first->down;
        else
            first = first->left;
    }//end while
}//end move_columns()


//moves a block of n columns, the rightmost of which is column s, to a new position following column t (NOTE: assumes s <= t)
void BarcodeCalculator::move_low_columns(int s, unsigned n, int t, MapMatrix_Perm* RL, MapMatrix_RowPriority_Perm* UL, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
{
    std::cout << "   --Transpositions for low simplices: [" << s << ", " << n << ", " << t << "] ";
    for(unsigned c=0; c<n; c++) //move column that starts at s-c
    {
        for(unsigned i=s; i<t; i++)
        {
            unsigned a = i - c;
            unsigned b = a + 1;

            //we must swap the d-simplices currently corresponding to columns a and b=a+1
            std::cout << "(" << a << "," << b << ")";

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
        }//end for(i=...)
    }//end for(c=...)

    std::cout << "\n";
}//end move_low_columns()


//moves a block of n columns, the rightmost of which is column s, to a new position following column t (NOTE: assumes s <= t)
void BarcodeCalculator::move_high_columns(int s, unsigned n, int t, MapMatrix_Perm* RH, MapMatrix_RowPriority_Perm* UH)
{
    std::cout << "   --Transpositions for high simplices: [" << s << ", " << n << ", " << t << "] ";
    for(unsigned c=0; c<n; c++) //move column that starts at s-c
    {
        for(unsigned i=s; i<t; i++)
        {
            unsigned a = i - c;
            unsigned b = a + 1;

            //we must swap the (d+1)-simplices currently corresponding to columns a and b=a+1
            std::cout << "(" << a << "," << b << ")";

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

            /// TESTING ONLY
            D->swap_columns(a, false);
            for(unsigned row = 0; row < D->height(); row++)
            {
                for(unsigned col = 0; col < D->width(); col++)
                {
                    bool temp = false;
                    for(unsigned e = 0; e < D->width(); e++)
                        temp = ( temp != (RH->entry(row, e) && UH->entry(e, col)) );
                    if(temp != D->entry(row, col))
                        std::cout << "====>>>> MATRIX ERROR AT THIS STEP!\n";
//                    else
//                        std::cout << "====NO ERROR";
               }
            }

        }//end for(i=...)
    }//end for(c=...)
    std::cout << "\n";
}//end move_high_columns()

//removes entries corresponding to xiMatrixEntry head from partition_low and partition_high
void BarcodeCalculator::remove_partition_entries(xiMatrixEntry* head)
{
    std::cout << "    ----removing partition entries for xiMatrixEntry " << head->index << " (" << head->low_index << "; " << head->high_index << ")\n";

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
void BarcodeCalculator::add_partition_entries(xiMatrixEntry* head)
{
    std::cout << "    ----adding partition entries for xiMatrixEntry " << head->index << " (" << head->low_index << "; " << head->high_index << ")\n";

    //low simplices
    if(head->low_class_size > 0)
        partition_low.insert( std::pair<unsigned, xiMatrixEntry*>(head->low_index, head) );

    //high simplices
    if(head->high_class_size > 0)
        partition_high.insert( std::pair<unsigned, xiMatrixEntry*>(head->high_index, head) );
}//end add_partition_entries()

//stores a discrete barcode in a 2-cell of the arrangement
///TODO: IMPROVE THIS!!! (store previous barcode at the simplicial level, and only examine columns that were modified in the recent update)
/// Is there a better way to handle endpoints at infinity?
void BarcodeCalculator::store_discrete_barcode(Face* cell, MapMatrix_Perm* RL, MapMatrix_Perm* RH)
{
    std::cout << "  -----barcode: ";

    //mark this cell as visited
    cell->mark_as_visited();

    //get a reference to the discrete barcode object
    DiscreteBarcode& dbc = cell->get_barcode();

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

                std::cout << "(" << c << "," << s << ")-->(" << a << "," << b << ") ";

                if(a != b)  //then we have a bar of positive length
                    dbc.add_bar(a, b);
            }
            else //then simplex c generates an essential cycle
            {
                std::cout << c << "-->" << a << " ";
                dbc.add_bar(a, -1);     //b = -1 = MAX_UNSIGNED indicates this is an essential cycle
            }
        }
    }
    std::cout << "\n";
}//end store_discrete_barcode()

