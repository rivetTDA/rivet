/* multi-graded Betti number class
 * takes a bifiltration and computes the multi-graded Betti numbers
 */

#include "multi_betti.h"

//constructor
MultiBetti::MultiBetti(SimplexTree* st, int dim, int v) :
	bifiltration(st),		//remember location of the simplex tree
	dimension(dim),			//remember the dimension
	verbosity(v)			//controls the amount of output
{
	//ensure that xi matrix is the correct size
    num_x_grades = bifiltration->num_x_grades();
    num_y_grades = bifiltration->num_y_grades();
    xi.resize(boost::extents[num_x_grades][num_y_grades][2]);

    //SUPPORT FOR DEPRECATED METHODS
    num_times = num_x_grades;
    num_dists = num_y_grades;

}//end constructor

//computes xi_0 and xi_1 at all multi-indexes in a fast way
void MultiBetti::compute_fast()
{
    // STEP 1: compute nullity
    compute_nullities();

    // STEP 2: compute rank
    compute_ranks();

    // STEP 3: compute alpha (concludes computation of xi_0)
    compute_alpha();

    // STEP 4: compute eta (concludes computation of xi_1)
    compute_eta();

}//end compute_fast();

//compute nullities, add to xi matrices
//TODO: when testing finished, remove print statements
//TODO: add to xi_1 matrix
//NOTE: must run this before compute_rank() or compute_alpha()
void MultiBetti::compute_nullities()
{
//    std::cout << "  computing nullities; num_x_grades = " << num_x_grades << ", num_y_grades = " << num_y_grades << "\n";

    //get data
    MapMatrix* bdry1 = bifiltration->get_boundary_mx(dimension);
    IndexMatrix* ind1 = bifiltration->get_index_mx(dimension);

    //set up data structures
    Vector current_lows;
    Vector first_row_lows(bdry1->height(), -1);

    Vector nullities(num_y_grades);
    int zero_cols = 0;

    //first, handle multi-grade (0,0)
    //  do column reduction on columns for multi-grade (0,0) using first_row_lows
    reduce(bdry1, 0, ind1->get(0,0), first_row_lows, zero_cols);

    //  copy first_row_lows to current_lows
    current_lows = first_row_lows;

    //  record data
    nullities[0] = zero_cols;
    xi[0][0][0] = zero_cols;
    if(1 < num_x_grades)
        xi[1][0][1] += zero_cols;   //adding nullity(boundary B)
    if(1 < num_y_grades)
        xi[0][1][1] += zero_cols;   //adding nullity(boundary C)

    //handle the rest of the first column
    for(int y=1; y<num_y_grades; y++)
    {
        zero_cols = 0;

        //  do column reduction on columns for multi-grade (0,y) using current_lows
        reduce(bdry1, ind1->get(y-1,num_x_grades-1) + 1, ind1->get(y,0), current_lows, zero_cols);

        //  record data
        nullities[y] = nullities[y-1] + zero_cols;
        xi[0][y][0] = nullities[y];
        if(1 < num_x_grades)
            xi[1][y][1] += nullities[y];   //adding nullity(boundary B)
        if(y+1 < num_y_grades)
            xi[0][y+1][1] += nullities[y];   //adding nullity(boundary C)
    }

    //loop through columns after first column
    for(int x=1; x<num_x_grades; x++)
    {
        //handle first multi-grade in this column
        zero_cols = 0;

        //  do column reduction on columns for multi-grade (x,0) using first_row_lows
        reduce(bdry1, ind1->get(0,x-1) + 1, ind1->get(0,x), first_row_lows, zero_cols);

        //  copy first_row_lows to current_lows
        current_lows = first_row_lows;

        //  record data
        nullities[0] += zero_cols;
        xi[x][0][0] = nullities[0];
        if(x+1 < num_x_grades)
            xi[x+1][0][1] += nullities[0];   //adding nullity(boundary B)
        if(1 < num_y_grades)
            xi[x][1][1] += nullities[0];   //adding nullity(boundary C)

        //now loop through rows after first row
        for(int y=1; y<num_y_grades; y++)
        {
            zero_cols = 0;

            //  do column reduction on columns for multi-grades (0,y) through (x,y) using current_lows
            reduce(bdry1, ind1->get(y-1,num_x_grades-1) + 1, ind1->get(y,x), current_lows, zero_cols);

            //  record data
            nullities[y] = nullities[y-1] + zero_cols;
            xi[x][y][0] = nullities[y];     //adding nullity(boundary D)
            if(x+1 < num_x_grades)
                xi[x+1][y][1] += nullities[y];   //adding nullity(boundary B)
            if(y+1 < num_y_grades)
                xi[x][y+1][1] += nullities[y];   //adding nullity(boundary C)
        }
    }

    //testing
//    std::cout << "  boundary matrix 1:\n";
//    bdry1->print();

    //clean up
    delete bdry1;
    delete ind1;
}//end compute_nullities()

//compute nullities, add to xi matrices
//TODO: add to xi_1 matrix
//TODO: when testing finished, remove print statements
void MultiBetti::compute_ranks()
{
//    std::cout << "  computing ranks; num_x_grades = " << num_x_grades << ", num_y_grades = " << num_y_grades << "\n";

    //get data
    MapMatrix* bdry2 = bifiltration->get_boundary_mx(dimension + 1);
    IndexMatrix* ind2 = bifiltration->get_index_mx(dimension + 1);

    //set up data structures
    Vector current_lows;
    Vector first_row_lows(bdry2->height(), -1);

    Vector ranks(num_y_grades);
    int zero_cols = 0;

    //first, handle multi-grade (0,0)
    //  do column reduction on columns for multi-grade (0,0) using first_row_lows
    reduce(bdry2, 0, ind2->get(0,0), first_row_lows, zero_cols);

    //  copy first_row_lows to current_lows
    current_lows = first_row_lows;

    //  record data
    ranks[0] = ind2->get(0,0) + 1 - zero_cols;       //really: (ind2->get(0,0) - (-1)) - zero_cols
    xi[0][0][1] += ranks[0];

    //handle the rest of the first column
    for(int y=1; y<num_y_grades; y++)
    {
//        std::cout << "---multi-grade (0," << y << ")\n";
        zero_cols = 0;

        //  do column reduction on columns for multi-grade (0,y) using current_lows
        reduce(bdry2, ind2->get(y-1,num_x_grades-1) + 1, ind2->get(y,0), current_lows, zero_cols);

        //  record data
        ranks[y] = ranks[y-1] + (ind2->get(y,0) - ind2->get(y-1,num_x_grades-1) - zero_cols);
        xi[0][y][1] += ranks[y];
    }

    //loop through columns after first column
    for(int x=1; x<num_x_grades; x++)
    {
        //handle first multi-grade in this column
        zero_cols = 0;

        //  do column reduction on columns for multi-grade (x,0) using first_row_lows
        reduce(bdry2, ind2->get(0,x-1) + 1, ind2->get(0,x), first_row_lows, zero_cols);

        //  copy first_row_lows to current_lows
        current_lows = first_row_lows;

        //  record data
        ranks[0] += (ind2->get(0,x) - ind2->get(0,x-1)) - zero_cols;
        xi[x][0][1] += ranks[0];

        //now loop through rows after first row
        for(int y=1; y<num_y_grades; y++)
        {
//            std::cout << "---multi-grade (" << x << "," << y << ")\n";
            zero_cols = 0;

            //  do column reduction on columns for multi-grades (0,y) through (x,y) using current_lows
            reduce(bdry2, ind2->get(y-1,num_x_grades-1) + 1, ind2->get(y,x), current_lows, zero_cols);

            //  record data
            ranks[y] = ranks[y-1] + (ind2->get(y,x) - ind2->get(y-1,num_x_grades-1)) - zero_cols;
            xi[x][y][1] += ranks[y];    //adding rank(boundary D)
        }
    }

    //clean up
    delete bdry2;
    delete ind2;
}//end compute_ranks()


//ordered list for the following functions (compute_alpha() and compute_eta()
//   used to record which columns of the merge/split matrix correspond to zero columns of the boundary matrix
struct ColumnList {
    std::set<int> columns;  //stores indexes of columns
    std::vector<int> grades;    //stores lowest column index associated with each y-grade

    ColumnList(int num_y_grades): grades(num_y_grades, -1)
    { }

    void insert(int col_index, unsigned y_grade)
    {
        if(y_grade >= grades.size())
            throw std::runtime_error("attempting to insert column pointer with inproper y-grade");
        columns.insert(col_index);  //insert column
        if(grades[y_grade] == -1 || col_index < grades[y_grade])   //then update info for this y-grade
            grades[y_grade] = col_index;
    }

    std::set<int>::iterator get(unsigned y_grade)    //gets lowest column index for this y-grade
    {
        return columns.find(grades[y_grade]);
    }

    std::set<int>::iterator end()
    {
        return columns.end();
    }

    void print()    //TESTING ONLY
    {
        std::cout << "columns: ";
        for(std::set<int>::iterator it=columns.begin(); it!=columns.end(); ++it)
            std::cout << *it << ", ";
        std::cout << "grades: ";
        for(int i=0; i<grades.size(); i++)
            std::cout << grades[i] << ", ";
        std::cout << "\n";
    }
};


//compute alpha, add to xi matrices
//TODO: add to xi_1 matrix
//TODO: when testing finished, remove print statements
void MultiBetti::compute_alpha()
{
//    std::cout << "  computing alpha; num_x_grades = " << num_x_grades << ", num_y_grades = " << num_y_grades << "\n";

    //get data
    DirectSumMatrices dsm = bifiltration->get_merge_mxs();
    MapMatrix* bdry_bc = dsm.boundary_matrix;
    MapMatrix* merge = dsm.map_matrix;
    IndexMatrix* ind_bc = dsm.column_indexes;
    MapMatrix* bdry_d = bifiltration->get_boundary_mx(dimension + 1);
    IndexMatrix* ind_d = bifiltration->get_index_mx(dimension + 1);

    //set up data structures
    Vector current_lows_bc;                             //low arrays for matrix bdry_bc
    Vector first_row_lows_bc(bdry_bc->height(), -1);

    int zero_cols_bc = 0;                             //counts number of zeroed columns in bdry_bc for the current multi-grade
    ColumnList zero_col_list(num_y_grades + 1);     // OFF BY 1 ????? NO, BUT FIX THIS AFTER OPTIMIZING THE MERGE MATRIX

    Vector current_lows_dm;                             //low arrays for matrices bdry_d and merge
    Vector first_row_lows_dm(bdry_d->height(), -1);

    int zero_cols_dm = 0;                     //stores number of zeroed columns in matrices bdry_d and merge for the current multi-grade
    Vector dim_dm(num_y_grades);              //stores dimenson of the sum Im(bdry_d) + Im(f(ker(bdry_bc))) by multi-grade


    //first, handle multi-grade (0,0)

    //  do column reduction on columns for multi-grade (0,0) using first_row_lows (first on bdry_bc and on merge, and then on the spliced matrix bdry_d and merge)
    reduce_also(bdry_bc, merge, 0, ind_bc->get(0,0), first_row_lows_bc, 0, zero_col_list, zero_cols_bc);
    reduce_spliced(bdry_d, merge, ind_d, ind_bc, zero_col_list, 0, 0, first_row_lows_dm, zero_cols_dm);

    //  record data
    dim_dm[0] = (ind_d->get(0,0) + 1) + zero_cols_bc - zero_cols_dm;       //really: (ind_d->get(0,0) - (-1)) + (number of "in-play" columns in merge matrix) - zeroed_cols
//    std::cout << "    grade (0,0): " << dim_dm[0] << "\n";
    xi[0][0][0] -= dim_dm[0];
    xi[0][0][1] -= dim_dm[0];

    //  copy first_row_lows to current_lows
    current_lows_bc = first_row_lows_bc;
    current_lows_dm = first_row_lows_dm;

    //handle the rest of the first column
    for(int y=1; y<num_y_grades; y++)
    {
        zero_cols_bc = 0;
        zero_cols_dm = 0;

        //  do column reductions on columns for multi-grade (0,y) using current_lows (first on bdry_bc and on merge, and then on the spliced matrix bdry_d and merge)
//        std::cout << "---multi-grade (0," << y << ")\n";
        reduce_also(bdry_bc, merge, ind_bc->get(y-1,num_x_grades) + 1, ind_bc->get(y,0), current_lows_bc, y, zero_col_list, zero_cols_bc);     // OFF BY 1 ????? NO, BUT FIX THIS AFTER OPTIMIZING THE MERGE MATRIX
        reduce_spliced(bdry_d, merge, ind_d, ind_bc, zero_col_list, 0, y, current_lows_dm, zero_cols_dm);

        //  record data
        dim_dm[y] = dim_dm[y-1] + (ind_d->get(y,0) - ind_d->get(y-1,num_x_grades-1)) + zero_cols_bc - zero_cols_dm;
//        std::cout << "    grade (0," << y << "): " << dim_dm[y] << "\n";
        xi[0][y][0] -= dim_dm[y];
        xi[0][y][1] -= dim_dm[y];
    }

    //loop through columns after first column
    for(int x=1; x<num_x_grades; x++)
    {
        //handle first multi-grade in this column
        zero_cols_bc = 0;
        zero_cols_dm = 0;
//        std::cout << "---multi-grade (" << x << ",0)\n";

        //  do column reductions on columns for multi-grade (x,0) using first_row_lows (first on bdry_bc and on merge, and then on the spliced matrix bdry_d and merge)
        reduce_also(bdry_bc, merge, ind_bc->get(0,x-1) + 1, ind_bc->get(0,x), first_row_lows_bc, 0, zero_col_list, zero_cols_bc);
        reduce_spliced(bdry_d, merge, ind_d, ind_bc, zero_col_list, x, 0, first_row_lows_dm, zero_cols_dm);

        //  record data
        dim_dm[0] += (ind_d->get(0,x) - ind_d->get(0,x-1)) + zero_cols_bc - zero_cols_dm;
//        std::cout << "    grade (" << x << ",0): " << dim_dm[0] << "\n";
        xi[x][0][0] -= dim_dm[0];
        xi[x][0][1] -= dim_dm[0];

        //  copy first_row_lows to current_lows
        current_lows_bc = first_row_lows_bc;
        current_lows_dm = first_row_lows_dm;

        //now loop through rows after first row
        for(int y=1; y<num_y_grades; y++)
        {
            zero_cols_bc = 0;
            zero_cols_dm = 0;
//            std::cout << "---multi-grade (" << x << "," << y << ")\n";

            //  do column reductions on columns for multi-grades (0,y) through (x,y) using current_lows (first on bdry_bc and on merge, and then on the spliced matrix bdry_d and merge)
            reduce_also(bdry_bc, merge, ind_bc->get(y-1,num_x_grades) + 1, ind_bc->get(y,x), current_lows_bc, y, zero_col_list, zero_cols_bc);     // OFF BY 1 ????? NO, BUT FIX THIS AFTER OPTIMIZING THE MERGE MATRIX
            reduce_spliced(bdry_d, merge, ind_d, ind_bc, zero_col_list, x, y, current_lows_dm, zero_cols_dm);

            //record data
//            cur_dim_dm->at(y) = (prev_dim_dm->at(y) - prev_dim_dm->at(y-1) + cur_dim_dm->at(y-1)) + (ind_d->get(y,x) - ind_d->get(y,x-1)) + zeroed_cols_bc - zeroed_cols_dm;
            dim_dm[y] = dim_dm[y-1] + (ind_d->get(y,x) - ind_d->get(y-1,num_x_grades-1)) + zero_cols_bc - zero_cols_dm;
//            std::cout << "    grade (" << x << "," << y << "): " << dim_dm[y] << "\n";
            xi[x][y][0] -= dim_dm[y];
            xi[x][y][1] -= dim_dm[y];
        }
    }

    //clean up
    delete bdry_bc;
    delete merge;
    delete ind_bc;
    delete bdry_d;
    delete ind_d;
}//end compute_alpha()

//compute eta, add to xi matrices
//TODO: add to xi_1 matrix
//TODO: when testing finished, remove print statements
void MultiBetti::compute_eta()
{
//    std::cout << "  computing eta; num_x_grades = " << num_x_grades << ", num_y_grades = " << num_y_grades << "\n";

    //get data
    DirectSumMatrices dsm = bifiltration->get_split_mxs();
    MapMatrix* bdry_bc = dsm.boundary_matrix;
    IndexMatrix* ind_bc = dsm.column_indexes;
    MapMatrix* split = dsm.map_matrix;
    MapMatrix* bdry_a = bifiltration->get_boundary_mx(dimension);
    IndexMatrix* ind_a = bifiltration->get_offset_index_mx(dimension);

//    std::cout << "  Boundary Matrix A:\n";
//    bdry_a->print();
//    std::cout << "  column indexes for A:\n";
//    ind_a->print();
//    std::cout << "  Boundary Matrix BC:\n";
//    bdry_bc->print();
//    std::cout << "  Split Matrix:\n";
//    split->print();
//    std::cout << "  column indexes for BC and split:\n";
//    ind_bc->print();

    //set up data structures
    Vector current_lows_a;                             //low arrays for matrix bdry_a
    Vector first_row_lows_a(bdry_bc->height(), -1);

    int zero_cols_a = 0;                             //counts number of zeroed columns in bdry_a for the current multi-grade
    ColumnList zero_col_list(num_y_grades);

    Vector current_lows_bcs;                             //low arrays for matrices bdry_bc and split
    Vector first_row_lows_bcs(bdry_bc->height(), -1);

    int zero_cols_bcs = 0;                     //stores number of zeroed columns in matrices bdry_bc and split for the current multi-grade
    Vector dim_bcs(num_y_grades);              //stores dimenson of the sum Im(bdry_bc) + Im(split(ker(bdry_a))) by multi-grade


    //first, handle multi-grade (0,0) ---- WAIT, THERE SHOULD NEVER BE ANYTHING TO DO HERE!!!!
    //  do column reduction on columns for multi-grade (0,0) using first_row_lows (first on bdry_a and on split, and then on the spliced matrix bdry_bc and split)
//    reduce_also(bdry_a, split, 0, ind_a->get(0,0), first_row_lows_a, 0, zero_col_list, zero_cols_a);
//    reduce_spliced(bdry_bc, split, ind_bc, ind_a, zero_col_list, 0, 0, first_row_lows_bcs, zero_cols_bcs);

    //  record data
//    std::cout << " ---At multigrade (0,0), " << zero_cols_a << " zero columns in A and " << zero_cols_bcs << " zero columns in BC-split.\n";
//    dim_bcs[0] = (ind_bc->get(0,0) + 1) + zero_cols_a - zero_cols_bcs;       //really: (ind_bc->get(0,0) - (-1)) + (number of "in-play" columns in split matrix) - zeroed_cols
//    xi[0][0][1] -= dim_bcs[0];

    //  copy first_row_lows to current_lows ---- THIS IS UNNECSSARY???
    current_lows_a = first_row_lows_a;
    current_lows_bcs = first_row_lows_bcs;

    //handle the rest of the first column
    for(int y=1; y<num_y_grades; y++)
    {
//        std::cout << "---multi-grade (0," << y << ")\n";
        zero_cols_a = 0;
        zero_cols_bcs = 0;

        //  do column reductions on columns for multi-grade (0,y) using current_lows (first on bdry_a and on split, and then on the spliced matrix bdry_bc and split)
        reduce_also(bdry_a, split, ind_a->get(y-1,num_x_grades) + 1, ind_a->get(y,0), current_lows_a, y, zero_col_list, zero_cols_a);     // OFF BY 1 ????? NO, BUT FIX THIS AFTER OPTIMIZING THE "A" INDEX MATRIX
        reduce_spliced(bdry_bc, split, ind_bc, ind_a, zero_col_list, 0, y, current_lows_bcs, zero_cols_bcs);

        //  record data
//        std::cout << "   ---At multigrade (0," << y << "), " << zero_cols_a << " zero columns in A and " << zero_cols_bcs << " zero columns in BC-split.\n";
        dim_bcs[y] = dim_bcs[y-1] + (ind_bc->get(y,0) - ind_bc->get(y-1,num_x_grades)) + zero_cols_a - zero_cols_bcs;     // OFF BY 1 ????? NO, BUT FIX THIS AFTER OPTIMIZING THE B+C BOUNDARY MATRIX
        xi[0][y][1] -= dim_bcs[y];
    }

    //loop through columns after first column
    for(int x=1; x<num_x_grades; x++)
    {
        //handle first multi-grade in this column
//        std::cout << "---multi-grade (" << x << ",0)\n";
        zero_cols_a = 0;
        zero_cols_bcs = 0;

        //  do column reductions on columns for multi-grade (x,0) using first_row_lows (first on bdry_a and on split, and then on the spliced matrix bdry_bc and split)
        reduce_also(bdry_a, split, ind_a->get(0,x-1) + 1, ind_a->get(0,x), first_row_lows_a, 0, zero_col_list, zero_cols_a);
        reduce_spliced(bdry_bc, split, ind_bc, ind_a, zero_col_list, x, 0, first_row_lows_bcs, zero_cols_bcs);

        //  record data
//        std::cout << "   ---At multigrade (" << x << ",0), " << zero_cols_a << " zero columns in A and " << zero_cols_bcs << " zero columns in BC-split.\n";
        dim_bcs[0] += (ind_bc->get(0,x) - ind_bc->get(0,x-1)) + zero_cols_a - zero_cols_bcs;
        xi[x][0][1] -= dim_bcs[0];

        //  copy first_row_lows to current_lows
        current_lows_a = first_row_lows_a;
        current_lows_bcs = first_row_lows_bcs;

        //now loop through rows after first row
        for(int y=1; y<num_y_grades; y++)
        {
//            std::cout << "---multi-grade (" << x << "," << y << ")\n";
            zero_cols_a = 0;
            zero_cols_bcs = 0;

            //  do column reductions on columns for multi-grades (0,y) through (x,y) using current_lows (first on bdry_a and on split, and then on the spliced matrix bdry_bc and split)
            reduce_also(bdry_a, split, ind_a->get(y-1, num_x_grades) + 1, ind_a->get(y,x), current_lows_a, y, zero_col_list, zero_cols_a);      // OFF BY 1 ????? NO, BUT FIX THIS AFTER OPTIMIZING THE "A" INDEX MATRIX
            reduce_spliced(bdry_bc, split, ind_bc, ind_a, zero_col_list, x, y, current_lows_bcs, zero_cols_bcs);

            //  record data
//            std::cout << "   ---At multigrade (" << x << "," << y << "), " << zero_cols_a << " zero columns in A and " << zero_cols_bcs << " zero columns in BC-split. ";
            dim_bcs[y] = dim_bcs[y-1] + (ind_bc->get(y,x) - ind_bc->get(y-1,num_x_grades)) + zero_cols_a - zero_cols_bcs;     // OFF BY 1 ????? NO, BUT FIX THIS AFTER OPTIMIZING THE B+C BOUNDARY MATRIX
//            std::cout << "  dim_bcs[" << y << "] = " << dim_bcs[y] << "\n";
            xi[x][y][1] -= dim_bcs[y];
        }
    }

    //clean up
    delete bdry_a;
    delete split;
    delete ind_a;
    delete bdry_bc;
    delete ind_bc;
}//end compute_eta()


//reduce matrix: perform column operations from first_col to last_col, inclusive
//  counts the number of zero-columns in [first_col, last_col], regardless of whether they were zeroed out in this reduction or zero to begin with
//TODO: when testing finished, remove print statements
void MultiBetti::reduce(MapMatrix* mm, int first_col, int last_col, Vector& lows, int& zero_cols)
{
//    std::cout << "  reducing columns " << first_col << " to " << last_col << "...";
    for(int i = first_col; i <= last_col; i++)
    {
        //while column i is nonempty and its low number is found in the low array, do column operations
        while(mm->low(i) >= 0 && lows[mm->low(i)] >= 0)
        {
//            std::cout << "  --adding column " << lows[mm->low(i)] << " to column " << i << "\n";
            mm->add_column(lows[mm->low(i)], i);
        }

        if(mm->low(i) >= 0) //column is still nonempty, so update lows
            lows[mm->low(i)] = i;
        else //column is zero
            zero_cols++;
    }
}//end reduce()

//reduce matrix: perform column operations from first_col to last_col, inclusive
//  this version also performs the same column operations on a second matrix
//  counts the number of zero-columns in [first_col, last_col], regardless of whether they were zeroed out in this reduction or zero to begin with
//TODO: when testing finished, remove print statements
void MultiBetti::reduce_also(MapMatrix* mm, MapMatrix* m2, int first_col, int last_col, Vector& lows, int y_grade, ColumnList& zero_list, int& zero_cols)
{
    //testing
//    if(last_col >= first_col)
//        std::cout << "  reducing (2 matrices) columns " << first_col << " to " << last_col << "...";

    for(int i = first_col; i <= last_col; i++)
    {
        //while column i is nonempty and its low number is found in the low array, do column operations
        while(mm->low(i) >= 0 && lows[mm->low(i)] >= 0)
        {
            int col_to_add = lows[mm->low(i)];
//            std::cout << "  --adding column " << col_to_add << " to column " << i << "\n";
            mm->add_column(col_to_add, i);
            m2->add_column(col_to_add, i);
        }

        if(mm->low(i) >= 0) //column is still nonempty, so update lows
            lows[mm->low(i)] = i;
        else //column is zero
        {
            zero_cols++;
            zero_list.insert(i, y_grade);
//            std::cout << "  --column " << i << " is zero";
        }
    }
}//end reduce_also()

//TESTING ONLY
void MultiBetti::print_lows(Vector &lows)
{
    std::cout << "      low array: ";
    for(int i=0; i<lows.size(); i++)
        std::cout << lows[i] << ", ";
    std::cout << "\n";
}

//reduce matrix: perform column operations on TWO MATRICES, regarded as one matrix spliced to preserve multi-grade order of columns
//  requires the matrices of multi-grade indexes, the list of in-play columns in the right matrix, and the current multi-grade
//TODO: when testing finished, remove print statements
void MultiBetti::reduce_spliced(MapMatrix* m_left, MapMatrix* m_right, IndexMatrix* ind_left, IndexMatrix* ind_right, ColumnList& right_cols, int grade_x, int grade_y, Vector& lows, int& zero_cols)
{
//    std::cout << "  reducing spliced matrix for multi-grade (" << grade_x << ", " << grade_y << ") ...\n";

    //determine starting column for left matrix
    int first_col_left = 0;
    if(grade_y > 0)
        first_col_left = ind_left->get(grade_y - 1, ind_left->width() - 1) + 1;
    else if(grade_x > 0)
        first_col_left = ind_left->get(0, grade_x - 1) + 1;

    //determine starting column for right matrix
    std::set<int>::iterator col_iterator = right_cols.get(grade_y);
    int cur_col;
    if(col_iterator == right_cols.end())   //then there are no columns in play for the right matrix
        cur_col = ind_right->get(grade_y, grade_x) + 1;
    else
        cur_col = *col_iterator;

    if(grade_y == 0 && grade_x > 0)    //then skip columns from previous x-grades, since their low numbers are already in the low array
    {
        while(cur_col <= ind_right->get(0, grade_x -1))
        {
            ++col_iterator;
            if(col_iterator == right_cols.end())   //then there are no columns in play for the right matrix
                cur_col = ind_right->get(0, grade_x) + 1;
            else
                cur_col = *col_iterator;
        }
    }

    //loop through all x-grades at the current y-grade from 0 to grade_x
    for(int x = 0; x <= grade_x; x++)
    {
        int last_col_left = ind_left->get(grade_y, x);

        //reduce these columns from the left matrix
        for(int i = first_col_left; i <= last_col_left; i++)
        {
            //while column i is nonempty and its low number is found in the low array, do column operations
            while(m_left->low(i) >= 0 && lows[m_left->low(i)] >= 0)
            {
//                std::cout << "  --[left matrix] adding column " << lows[m_left->low(i)] << " to column " << i << "\n";

                if( lows[m_left->low(i)] < m_left->width() )    //then column to add is in the left matrix
                    m_left->add_column(lows[m_left->low(i)], i);
                else    //then column to add is in the right matrix
                {
                    if(m_right->low(lows[m_left->low(i)] - m_left->width()) == -1)  //TESTING ONLY
                        throw std::runtime_error("zero column addition error");

                    m_left->add_column(m_right, lows[m_left->low(i)] - m_left->width(), i);
                }
            }

            if(m_left->low(i) >= 0)     //column is still nonempty, so update lows
                lows[m_left->low(i)] = i;
            else //column is zero
            {
//                std::cout << "  --[left matrix] column " << i << " is zero";
                zero_cols++;
            }
        }
        first_col_left = last_col_left + 1; //prep for next iteration of the outer loop

        //determine end column from right matrix
        int last_col_right = ind_right->get(grade_y, x);

        //reduce columns from the right matrix
        while(cur_col <= last_col_right)
        {
            //while column is nonempty and its low number is found in the low array, do column operations
            while(m_right->low(cur_col) >= 0 && lows[m_right->low(cur_col)] >= 0)
            {
//                std::cout << "  --[right matrix] adding column " << lows[m_right->low(cur_col)] << " to column " << cur_col << "\n";

                if( lows[m_right->low(cur_col)] >= m_left->width() )    //then column to add is in the right matrix
                    m_right->add_column(lows[m_right->low(cur_col)] - m_left->width(), cur_col);
                else    //then column to add is in the left matrix
                    m_right->add_column(m_left, lows[m_right->low(cur_col)], cur_col);
            }

            if(m_right->low(cur_col) >= 0) //column is still nonempty, so update lows
            {
                lows[m_right->low(cur_col)] = m_left->width() + cur_col;
            }
            else //column is zero
            {
//                std::cout << "  --[right matrix] column " << cur_col << " is zero";
                zero_cols++;
            }

            //move to next column
            ++col_iterator;
            if(col_iterator == right_cols.end())   //then there are no columns in play for the right matrix
                cur_col = ind_right->get(grade_y, grade_x) + 1;
            else
                cur_col = *col_iterator;
        }
    }//end for(x=0..grade_x)
}//end reduce_spliced()




/////// DEPRECATED FUNCTIONS ///////

//computes xi_0 and xi_1 at ALL multi-indexes
//void MultiBetti::compute_all_xi()
//{
//	if(verbosity >= 4) { std::cout << "  computing xi_0 and xi_1; num_times=" << num_times << ", num_dists=" << num_dists << "\n"; }
	
//	//compute and store xi_0 and xi_1 at ALL multi-indexes
//	for(int time = 0; time < num_times; time++)
//	{
//		for(int dist = 0; dist < num_dists; dist++)
//		{
//			compute_xi(time, dist);
//		}
//	}
//}


//computes xi_0 and xi_1 at a specified multi-index
//void MultiBetti::compute_xi(int time, int dist)
//{
//	//xi_0 first
//	if(verbosity >= 6) { std::cout << "  COMPUTING xi_0(" << time << "," << dist << ") in dimension " << dimension << "\n"; }
	
//	//build boundary matrix B+C
//	MapMatrix* bdry_bc = (*bifiltration).get_boundary_mx(time, dist-1, dimension);
//	MapMatrix* bdry_c = (*bifiltration).get_boundary_mx(time-1, dist, dimension);
//	(*bdry_bc).append_block( (*bdry_c), (*bdry_bc).height() );
//	if(verbosity >= 8)
//	{
//		std::cout << "    boundary matrix for direct sum B+C:\n";
//		(*bdry_bc).print();
//	}

//	//build matrix [B+C,D]
//	MapMatrix* bcd = (*bifiltration).get_merge_mx(time, dist, dimension);
//	if(verbosity >= 8)
//	{
//		std::cout << "    map matrix [B+C,D]:\n";
//		(*bcd).print();
//	}

//	//apply column-reduction algorithm to boundary matrix B+C, and do the same column operations to [B+C,D]
//	(*bdry_bc).col_reduce(bcd);
//	if(verbosity >= 8)
//	{
//		std::cout << "    reducing boundary matrix B+C, and applying same column operations to [B+C,D]\n";
//		std::cout << "      reduced boundary matrix B+C:\n";
//		(*bdry_bc).print();
//		std::cout << "      reduced matrix [B+C,D]:\n";
//		(*bcd).print();
//	}

//	//identify zero columns from boundary matrix for B+C, and select those columns from [B+C,D]
//	int c=1;	//keep track of current column in [B+C,D]
//	for(int j=1; j<=(*bdry_bc).width(); j++)
//	{
//		if((*bdry_bc).low(j) > 0)	//then delete this column from [B+C,D]
//			(*bcd).remove_column(c);
//		else
//			c++;
//	}
//	int nullity_bc1 = c-1;	//we need nullity of B+C for later
//	//now bcd contains a basis for [B+C,D](ker(boundary map B+C))
//	if(verbosity >= 8)
//	{
//		std::cout << "      basis for [B+C,D](ker(boundary map B+C)):\n";
//		(*bcd).print();
//	}

//	//form concatenated matrix
//	MapMatrix* bdry_d2 = (*bifiltration).get_boundary_mx(time, dist, dimension+1);
//	if(verbosity >= 8)
//	{
//		std::cout << "    boundary matrix D2:";
//		(*bdry_d2).print();
//	}
//	int d2_width = (*bdry_d2).width();
//	(*bdry_d2).append_block( (*bcd), 0);
//	if(verbosity >= 8)
//	{
//		std::cout << "    concatenating D2 with [B+C,D](ker(..)):\n";
//		(*bdry_d2).print();
//	}

//	//reduce the concatenated matrix
//	(*bdry_d2).col_reduce();
//	if(verbosity >= 8)
//	{
//		std::cout << "    reduced form of concatenated matrix:\n";
//		(*bdry_d2).print();
//	}

//	//count nonzero columns in reduced form of right block of concatenated matrix
//	int alpha = 0;
//	for(int j = d2_width+1; j<=(*bdry_d2).width(); j++)
//		if((*bdry_d2).low(j) > 0)
//			alpha++;
//	if(verbosity >= 8) { std::cout << "    number of nonzero columns in right block (alpha): " << alpha << "\n"; }

//	//compute dimensionension of homology at D
//	MapMatrix* bdry_d1 = (*bifiltration).get_boundary_mx(time, dist, dimension);
//	if(verbosity >= 8)
//	{
//		std::cout << "    boundary matrix D1:\n";
//		(*bdry_d1).print();
//	}
//	//compute nullity of D1
//	(*bdry_d1).col_reduce();
//	int nullity_d1 = 0;
//	for(int j=1; j<=(*bdry_d1).width(); j++)
//		if( (*bdry_d1).low(j) == 0)
//			nullity_d1++;
//	//compute rank of D2
//	int rank_d2 = 0;
//	for(int j=1; j<=d2_width; j++)
//		if( (*bdry_d2).low(j) > 0)
//			rank_d2++;
//	if(verbosity >= 8)
//	{
//		std::cout << "      nullity of D1 is: " << nullity_d1 << "\n";
//		std::cout << "      rank of D2 is: " << rank_d2 << "\n";
//	}

//	//compute xi_0
//	xi[time][dist][0] = nullity_d1 - rank_d2 - alpha;

//	//now for xi_1
//	if(verbosity >= 8) { std::cout << "  COMPUTING xi_1(" << time << "," << dist << ") in dimension " << dimension << "\n"; }

//	//build boundary matrix A
//	MapMatrix* bdry_a = (*bifiltration).get_boundary_mx(time-1, dist-1, dimension);
//	if(verbosity >= 8)
//	{
//		std::cout << "    boundary matrix A:\n";
//		(*bdry_a).print();
//	}

//	//build matrix [A,B+C]
//	MapMatrix* abc = (*bifiltration).get_split_mx(time, dist, dimension);
//	if(verbosity >= 8)
//	{
//		std::cout << "    map matrix [A,B+C]:\n";
//		(*abc).print();
//	}

//	//apply column-reduction algorithm to boundary matrix A, and do the same column operations to [A,B+C]
//	(*bdry_a).col_reduce(abc);
//	if(verbosity >= 8)
//	{
//		std::cout << "    reducing boundary matrix A, and applying same column operations to [A,B+C]\n";
//		std::cout << "      reduced boundary matrix A:\n";
//		(*bdry_a).print();
//		std::cout << "      reduced matrix [A,B+C]:\n";
//		(*abc).print();
//	}

//	//identify zero columns from boundary matrix for A, and select those columns from [A,B+C]
//	c=1;	//keep track of current column in [A,B+C]
//	for(int j=1; j<=(*bdry_a).width(); j++)
//	{
//		if((*bdry_a).low(j) > 0)	//then delete this column from [A,B+C]
//			(*abc).remove_column(c);
//		else
//			c++;
//	}
//	//now abc contains a basis for [A,B+C](ker(boundary map A))
//	if(verbosity >= 8)
//	{
//		std::cout << "      basis for [A,B+C](ker(boundary map A)):\n";
//		(*abc).print();
//	}

//	//build boundary matrix B+C for one dimension higher
//	MapMatrix* bdry_bc2 = (*bifiltration).get_boundary_mx(time, dist-1, dimension+1);
//	MapMatrix* bdry_c2 = (*bifiltration).get_boundary_mx(time-1, dist, dimension+1);
//	(*bdry_bc2).append_block( (*bdry_c2), (*bdry_bc2).height() );
//	if(verbosity >= 8)
//	{
//		std::cout << "    boundary matrix for direct sum B+C (2):\n";
//		(*bdry_bc2).print();
//	}

//	//form concatenated matrix
//	int bc2_width = (*bdry_bc2).width();
//	(*bdry_bc2).append_block( (*abc), 0);
//	if(verbosity >= 8)
//	{
//		std::cout << "    concatenating B+C (2) with [A,B+C](ker(..)):\n";
//		(*bdry_bc2).print();
//	}

//	//reduce the concatenated matrix
//	(*bdry_bc2).col_reduce();
//	if(verbosity >= 8)
//	{
//		std::cout << "    reduced form of concatenated matrix:\n";
//		(*bdry_bc2).print();
//	}

//	//count nonzero columns in reduced form of right block of concatenated matrix
//	int nu = 0;
//	for(int j = bc2_width+1; j<=(*bdry_bc2).width(); j++)
//		if((*bdry_bc2).low(j) > 0)
//			nu++;
//	if(verbosity >= 8) { std::cout << "    number of nonzero columns in right block (nu): " << nu << "\n"; }

//	//compute dimension of homology at B+C
//	//compute nullity of boundary B+C (1) was computed earlier and stored in nullity_bc
//	//rank of boundary B+C (2)

//	//compute rank of D2
//	int rank_bc2 = 0;
//	for(int j=1; j<=bc2_width; j++)
//		if( (*bdry_bc2).low(j) > 0)
//			rank_bc2++;
//	if(verbosity >= 8)
//	{
//		std::cout << "      nullity of B+C (1) is: " << nullity_bc1 << "\n";
//		std::cout << "      rank of B+C (2) is: " << rank_bc2 << "\n";
//		std::cout << "      nullity(gamma_M) = " << (nullity_bc1 - rank_bc2 - alpha) << "\n";
//		std::cout << "      alpha = " << alpha << "\n";
//	}

//	//compute \xi_1
//	xi[time][dist][1] = nullity_bc1 - rank_bc2 - alpha - nu;
//}//end compute_xi


///// FUNCTIONS TO ACCESS THE MULTI-GRADED BETTI NUMBERS

//returns xi_0 at the specified multi-index
int MultiBetti::xi0(int x, int y)
{
    return xi[x][y][0];
}

//returns xi_1 at the specified multi-index
int MultiBetti::xi1(int x, int y)
{
    return xi[x][y][1];
}
	



