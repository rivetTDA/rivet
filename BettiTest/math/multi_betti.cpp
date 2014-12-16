/* multi-graded Betti number class
 * takes a bifiltration and computes the multi-graded Betti numbers
 */

#include <limits>   // for max unsigned int

#include "multi_betti.h"


//ordered list used to record which columns of the merge/split matrix correspond to zero columns of the boundary matrix
struct ColumnList {
    std::set<unsigned> columns;  //stores indexes of columns
    std::vector<unsigned> grades;    //stores lowest column index associated with each y-grade

    ColumnList(unsigned num_y_grades): grades( num_y_grades, std::numeric_limits<unsigned>::max() )
    { }

    void insert(unsigned col_index, unsigned y_grade)
    {
        if(y_grade >= grades.size())
            throw std::runtime_error("attempting to insert column pointer with inproper y-grade");

        columns.insert(col_index);  //insert column

        if(grades[y_grade] == std::numeric_limits<unsigned>::max() || col_index < grades[y_grade])   //then update info for this y-grade
            grades[y_grade] = col_index;
    }

    std::set<unsigned>::iterator get(unsigned y_grade)    //gets lowest column index for this y-grade
    {
        return columns.find(grades[y_grade]);
    }

    std::set<unsigned>::iterator end()
    {
        return columns.end();
    }

    void print()    //TESTING ONLY
    {
        std::cout << "columns: ";
        for(std::set<unsigned>::iterator it=columns.begin(); it!=columns.end(); ++it)
            std::cout << *it << ", ";
        std::cout << "grades: ";
        for(unsigned i=0; i<grades.size(); i++)
            std::cout << grades[i] << ", ";
        std::cout << "\n";
    }
};








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

}//end constructor


//New! Optimized!
//computes some of the quantities required for xi_0 and xi_1 --- IMPROVE THIS!
void MultiBetti::compute_parallel()
{
  // DATA STRUCTURES
    //get data
    MapMatrix* bdry1 = bifiltration->get_boundary_mx(dimension);
    IndexMatrix* ind1 = bifiltration->get_index_mx(dimension);

    MapMatrix* bdry2 = bifiltration->get_boundary_mx(dimension + 1);
    IndexMatrix* ind2 = bifiltration->get_index_mx(dimension + 1);

    MapMatrix* merge = new MapMatrix(bdry1->width(), bdry1->width());
    for(unsigned i=0; i<merge->width(); i++)
        merge->set(i,i);


    //set up data structures for computation of nullities
    Vector current_lows_bdry1;                  //low arrays for matrix bdry1
    Vector first_row_lows_bdry1(bdry1->height(), -1);
    unsigned zero_cols_bdry1 = 0;               //stores number of zeroed columns in matrix bdry1 for the current multigrade
    Vector nullity(num_y_grades);             //stores nullity, by multigrade

    //set up data structures for computation of merge dimension
    ColumnList zero_col_list(num_y_grades);     //tracks which columns in bdry1 are zero
    Vector current_lows_merge;                  //low arrays for matrices bdry2 and merge
    Vector first_row_lows_merge(bdry_d->height(), -1);
    unsigned zero_cols_merge = 0;               //stores number of zeroed columns in matrices bdry2 and merge for the current multigrade
    Vector dim_merge(num_y_grades);                //stores dimenson of the sum Im(bdry_d) + Im(merge(ker(bdry_bc))), by multigrade


  // FIRST, HANDLE MULTIGRADE (0,0)
    //reduce bdry2 at (0,0)
    reduce(bdry2, 0, ind2->get(0,0), first_row_lows_merge, zero_cols_merge);

    //record merge dimension
    dim_merge[0] = (ind2->get(0,0) + 1) - zero_cols_merge;       //really: (ind2->get(0,0) - (-1)) - zeroed_cols_merge
    xi[0][0][0] -= dim_merge[0];
    xi[0][0][1] -= dim_merge[0];

    //reduce bdry1, and apply same col op's to merge, at (0,0)
    reduce_also(bdry1, merge, 0, ind1->get(0,0), first_row_lows_bdry1, 0, zero_col_list, zero_cols_bdry1);

    //record nullity
    nullity[0] = zero_cols_bdry1;
    xi[0][0][0] = zero_cols_bdry1;
    if(1 < num_x_grades)
        xi[1][0][1] += zero_cols_bdry1;   //adding nullity(boundary B)
    if(1 < num_y_grades)
        xi[0][1][1] += zero_cols_bdry1;   //adding nullity(boundary C)

    //copy first_row_lows to current_lows
    current_lows_merge = first_row_lows_merge;
    current_lows_bdry1 = first_row_lows_bdry1;

  // HANDLE THE REST OF THE FIRST MULTIGRADE COLUMN (x = 0)
    for(unsigned y=1; y<num_y_grades; y++)
    {
        //reset...
        zero_cols_merge = 0;
        zero_cols_bdry1 = 0;

        //reduce bdry2 and merge at (0,y)
        reduce_spliced(bdry2, merge, ind2, ind1, zero_col_list, 0, y, current_lows_merge, zero_cols_merge); //NEED AN UPDATED reduce_spliced METHOD!!!

        //record merge dimension
        dim_merge[y] = dim_merge[y-1] + (ind2->get(y,0) - ind2->get(y-1,num_x_grades-1)) + zero_cols_bdry1 - zero_cols_merge;   //UM....FIX THIS!!!
        xi[0][y][0] -= dim_dm[y];
        xi[0][y][1] -= dim_dm[y];

        //reduce bdry1 at (0,y)
        reduce_also(bdry1, merge, 0, ind1->get(0,0), first_row_lows_bdry1, 0, zero_col_list, zero_cols_bdry1);

        //record nullity
        nullity[y] = nullity[y-1] + zero_cols_bdry1;
        xi[0][y][0] = nullity[y];
        if(1 < num_x_grades)
            xi[1][y][1] += nullity[y];   //adding nullity(boundary B)
        if(y+1 < num_y_grades)
            xi[0][y+1][1] += nullity[y];   //adding nullity(boundary C)
    }

  // LOOP THROUGH MULTIGRADE COLUMNS AFTER THE FIRST (x > 0)
    for(unsigned x=1; x<num_x_grades; x++)
    {
      // HANDLE THE FIRST MULTIGRADE IN THIS COLUMN (x,0)
        //reset...

      // LOOP THROUGH MULTIGRADE ROWS AFTER THE FIRST
        for(unsigned y=1; y<num_y_grades; y++)
        {



        }
    }

}//end compute_parallel()



// OLD: computes xi_0 and xi_1 at all multi-indexes in a fast way
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


//compute alpha, add to xi matrices ------- THIS IS MERGE
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

//    std::cout << "Index matrix BC:\n";
//    ind_bc->print();
//    std::cout << "Index matrix D:\n";
//    ind_d->print();

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

        //TESTING!!!
//        std::cout <<"Matrix D:\n";
//        bdry_d->print();
//        std::cout <<"Merge Matrix:\n";
//        merge->print();
//        std::cout << "Low array:";
//        print_lows(first_row_lows_dm);
//        std::cout << "\n";

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

            //TESTING!!!
//            std::cout <<"Matrix D:\n";
//            bdry_d->print();
//            std::cout <<"Merge Matrix:\n";
//            merge->print();
//            std::cout << "Low array:";
//            print_lows(current_lows_dm);
//            std::cout << "\n";

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

//compute eta, add to xi matrices ------- THIS IS SPLIT
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
    if(last_col >= first_col)
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
    for(unsigned i=0; i<lows.size(); i++)
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
    {
        first_col_left = ind_left->get(0, grade_x - 1) + 1;
//        std::cout << "      first_col_left = " << first_col_left << "\n";
    }

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
    for(int x=0; x <= grade_x; x++)     //NOTE: THIS LOOP SHOULD BE UNNECESSARY IF grade_y == 0
    {
        int last_col_left = ind_left->get(grade_y, x);

//        std::cout << "  --[left matrix] reducing columns " << first_col_left << " to " << last_col_left << "\n";

        //reduce these columns from the left matrix
        for(int i = first_col_left; i <= last_col_left; i++)
        {
            //while column i is nonempty and its low number is found in the low array, do column operations
            while(m_left->low(i) >= 0 && lows[m_left->low(i)] >= 0)
            {
//                std::cout << "  --[left matrix] adding column with absolute index " << lows[m_left->low(i)] << " to column " << i << " from the left matrix\n";

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
        if(grade_y > 0)
            first_col_left = last_col_left + 1; //prep for next iteration of the outer loop

        //determine end column from right matrix
        int last_col_right = ind_right->get(grade_y, x);

        //reduce columns from the right matrix
        while(cur_col <= last_col_right)
        {
            //while column is nonempty and its low number is found in the low array, do column operations
            while(m_right->low(cur_col) >= 0 && lows[m_right->low(cur_col)] >= 0)
            {
//                std::cout << "  --[right matrix] adding column with absolute index " << lows[m_right->low(cur_col)] << " to column " << cur_col << " in right matrix\n";

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




///// FUNCTIONS TO ACCESS THE MULTI-GRADED BETTI NUMBERS

//returns xi_0 at the specified multi-index
int MultiBetti::xi0(unsigned x, unsigned y)
{
    return xi[x][y][0];
}

//returns xi_1 at the specified multi-index
int MultiBetti::xi1(unsigned x, unsigned y)
{
    return xi[x][y][1];
}
	



