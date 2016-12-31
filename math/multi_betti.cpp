/**********************************************************************
Copyright 2014-2016 The RIVET Devlopers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/
/* multi-graded Betti number class
 * takes a bifiltration and computes the multi-graded Betti numbers
 */

#include "multi_betti.h"

#include "debug.h"
#include "index_matrix.h"
#include "map_matrix.h"
#include "simplex_tree.h"
#include "template_point.h"

#include <interface/progress.h>
#include <set>

//struct to record which columns of a slave matrix correspond to zero columns of the reduced matrix
struct ColumnList {
    std::vector< std::set<int> > columns; //stores indexes of columns, by y-grade

    ColumnList(unsigned num_y_grades):
        columns(num_y_grades)
    { 
    }

    void insert(int col_index, unsigned y_grade)
    {
        if(y_grade >= columns.size())
            throw std::runtime_error("MultiBetti.cpp: attempting to insert column pointer with inproper y-grade");

        columns[y_grade].insert(col_index);
    }

    std::set<int>::iterator get(unsigned y_grade) //gets smallest column index for this y-grade
    {
        return columns[y_grade].begin();
    }

    std::set<int>::iterator end(unsigned y_grade) //gets largest column index for this y-grade
    {
        return columns[y_grade].end();
    }

    unsigned count(unsigned y_grade) //gets the number of columns with this y-grade
    {
        return columns[y_grade].size();
    }
};


//constructor: sets up the data structure but does not compute xi_0 or xi_1
MultiBetti::MultiBetti(SimplexTree& st, int dim)
    : bifiltration(st)
    , dimension(dim)
    , num_x_grades(bifiltration.num_x_grades())
    , num_y_grades(bifiltration.num_y_grades())
    , verbosity(st.verbosity)
{
    xi.resize(boost::extents[num_x_grades][num_y_grades][3]);
}//end constructor


//computes xi_0 and xi_1, and also stores dimension of homology at each grade in the supplied matrix
void MultiBetti::compute(unsigned_matrix& hom_dims, Progress& progress)
{
    //ensure hom_dims is the correct size
    hom_dims.resize(boost::extents[num_x_grades][num_y_grades]);

    //input to the algorithm: two boundary matrices, with index data
    MapMatrix* bdry1 = bifiltration.get_boundary_mx(dimension);
    IndexMatrix* ind1 = bifiltration.get_index_mx(dimension);

    MapMatrix* bdry2 = bifiltration.get_boundary_mx(dimension + 1);
    IndexMatrix* ind2 = bifiltration.get_index_mx(dimension + 1);


    // STEP 1: reduce bdry2, record its pointwise rank, and build a partially-reduced copy for later use
    //   this approach aims to maximize memory usage by deleting bdry2 matrix before building bdry2s matrix

    //data structures used for reducing bdry2 matrix
    Vector lows_bdry2(bdry2->height(), -1); //low array for bdry2
    long nonzero_cols_bdry2 = 0; //number of nonzero columns in bdry2 at <= current grade
    long nonzero_cols_b2_y0 = 0; //number of nonzero columns in bdry2 at y=0 grade
    MapMatrix* bdry2m = new MapMatrix(bdry2->height(), bdry2->width()); //partially-reduced copy of bdry2, to be "spliced" with merge matrix later

    //reduce bdry2 at (0,0) and record rank
    reduce(bdry2, 0, ind2->get(0, 0), lows_bdry2, nonzero_cols_b2_y0);
    nonzero_cols_bdry2 = nonzero_cols_b2_y0;
    xi[0][0][1] += nonzero_cols_bdry2; //adding rank(bdry2_D)
    hom_dims[0][0] -= nonzero_cols_bdry2; //subtracting rank(bdry2) at (0,0)
    xi[1][0][1] -= nonzero_cols_bdry2; //subtracting dim(U)=rank(bdry2_B) at (1,0)
    xi[0][1][1] -= nonzero_cols_bdry2; //subtracting dim(U)=rank(bdry2_C) at (0,1)
    bdry2m->copy_cols_same_indexes(bdry2, 0, ind2->get(0, 0));

    for(unsigned y = 1; y < num_y_grades; y++) { //reduce bdry2 at (0,y) for y > 0 and record rank
        reduce(bdry2, ind2->get(y - 1, num_x_grades - 1) + 1, ind2->get(y, 0), lows_bdry2, nonzero_cols_bdry2);
        xi[0][y][1] += nonzero_cols_bdry2; //adding rank(bdry2_D)
        hom_dims[0][y] -= nonzero_cols_bdry2; //subtracting rank(bdry2) at (0,y)
        if(y + 1 < num_y_grades)
            xi[0][y+1][1] -= nonzero_cols_bdry2; //subtracting dim(U)=rank(bdry2_C) at (0,y+1)
        bdry2m->copy_cols_same_indexes(bdry2, ind2->get(y - 1, num_x_grades - 1) + 1, ind2->get(y, 0));
    }

    for(unsigned x = 1; x < num_x_grades; x++) {
        //reduce bdry2 at (x,0) and record rank
        reduce(bdry2, ind2->get(0, x - 1) + 1, ind2->get(0, x), lows_bdry2, nonzero_cols_b2_y0);
        nonzero_cols_bdry2 = nonzero_cols_b2_y0;
        xi[x][0][1] += nonzero_cols_bdry2; //adding rank(bdry2_D)
        hom_dims[x][0] -= nonzero_cols_bdry2; //subtracting rank(bdry2) at (x,0)
        if(x + 1 < num_x_grades)
            xi[x+1][0][1] -= nonzero_cols_bdry2; //subtracting dim(U)=rank(bdry2_B) at (x+1,0)
        bdry2m->copy_cols_same_indexes(bdry2, ind2->get(0, x - 1) + 1, ind2->get(0, x));

        for(unsigned y = 1; y < num_y_grades; y++) { //reduce bdry2 at (x,y) and record rank
            reduce(bdry2, ind2->get(y - 1, num_x_grades - 1) + 1, ind2->get(y, x), lows_bdry2, nonzero_cols_bdry2);
            xi[x][y][1] += nonzero_cols_bdry2; //adding rank(bdry2_D)
            hom_dims[x][y] -= nonzero_cols_bdry2; //homology dimension at (x,y)
            bdry2m->copy_cols_same_indexes(bdry2, ind2->get(y, x - 1) + 1, ind2->get(y, x));
        }
    }

    //remove zero columns from bdry2m
    IndexMatrix* ind2m = new IndexMatrix(num_y_grades, num_x_grades); //stores grade info for bdry2m
    bdry2m->remove_zero_cols(ind2, ind2m);

    //clean up
    delete bdry2;
    delete ind2;

    //emit progress message
    progress.progress(40);


    // STEP 2: reduce bdry1, applying column operations to merge and split as "slave" matrices
    //   recurd pointwise nullity of bdry1
    //   also reduce "spliced" matrices and record pointwise dimension of vector spaces U and V

    //data structures used for reducing bdry1 matrix
    Vector lows_bdry1(bdry1->height(), -1); //low array for bdry1
    long zero_cols_bdry1 = 0; //number of zeroed columns in bdry1 at <= current grade
    long zero_cols_b1_y0 = 0; //number of zeroed columns in bdry1 at y=0 grade
    ColumnList zero_list_bdry1(num_y_grades); //tracks which columns in bdry1 are zero

    //data structures used for reducing the spliced matrix: bdry2 merge
    MapMatrix* merge = new MapMatrix(bdry1->width()); //initialize merge to an identity matrix
    Vector lows_b2merge(bdry2m->height(), -1); //low array for spliced matrix
    long nonzero_b2m_steps12 = 0; //number of nonzero columns reduced at steps 1 and 2 of reduce_spliced()
    long nonzero_b2m_step3 = 0; //number of nonzero columns reduced at step 3 of reduce_spliced()
    long dim_bdry2_y0; //number of nonzero columns in bdry2 at y=0 grades
    long dim_b2m_yprev; //number of nonzero columns in spliced matrix at previous y-grade, not re-reduced at current y-grade
    long dim_b2merge; //used for temporary calculation of dimension of V = Im(bdry_D) + Im(merge(ker(bdry_BC))

    //data structures used for reducing the spliced matrix: bdry2 split
    MapMatrix* bdry2s = new MapMatrix(2*bdry2m->height(), 0); //copy of bdry2 for the direct sum B+C, to be "spliced" with split matrix
    IndexMatrix* ind2s = new IndexMatrix(num_y_grades, num_x_grades); //grade data for bdry2s
    build_bdry2s_mx(bdry2m, ind2m, bdry2s, ind2s); //stores data in bdry2s and ind2s
    MapMatrix* split = build_split_mx(bdry1->width()); //two stacked copies of identity matrix
    Vector lows_b2split(bdry2s->height(), -1); //low array for spliced matrix
    long nonzero_cols_b2split = 0; //number of nonzero columns in spliced matrix at <= current grade

    ///TESTING
//    debug() << "bdry2s:";
//    bdry2s->print();
//    debug() << "bdry2s index matrix:";
//    ind2s->print();

    //CALCULATIONS AT GRADE (0,0)
    //reduce bdry2 and merge at (0,0) and record dimension (nonzero_b2m_step3 will remain 0)
    reduce_spliced(bdry2m, merge, ind2m, ind1, zero_list_bdry1, 0, 0, lows_b2merge, nonzero_b2m_steps12, nonzero_b2m_step3);
    dim_bdry2_y0 = nonzero_b2m_steps12;
    dim_b2m_yprev = dim_bdry2_y0;
    xi[0][0][0] -= dim_b2m_yprev; //subtracting dim(V)
    xi[0][0][1] -= dim_b2m_yprev; //subtracting dim(V)

    //reduce bdry1 at (0,0), apply same column operations to merge and split, and record nullity
    reduce_slave(bdry1, merge, split, 0, ind1->get(0,0), lows_bdry1, 0, zero_list_bdry1, zero_cols_b1_y0);
    zero_cols_bdry1 = zero_cols_b1_y0;
    xi[0][0][0] += zero_cols_bdry1; //adding nullity(bdry1_D)
    if(1 < num_x_grades)
        xi[1][0][1] += zero_cols_bdry1; //adding nullity(bdry1_B)
    if(1 < num_y_grades)
        xi[0][1][1] += zero_cols_bdry1; //adding nullity(bdry1_C)

    hom_dims[0][0] += zero_cols_bdry1; //homology dimension at (0,0)

    //reduce bdry2 and split at (0,0) and record dimension of U
    reduce_spliced(bdry2s, split, ind2s, ind1, zero_list_bdry1, 0, 0, lows_b2split, nonzero_cols_b2split);
    xi[1][1][1] -= nonzero_cols_b2split; //subtracting dim(U)


    //CALCULATIONS AT GRADES (0,y) FOR y > 0
    for(unsigned y = 1; y < num_y_grades; y++) {
        //reduce bdry2 and merge at (0,y) and record dimension
        reduce_spliced(bdry2m, merge, ind2m, ind1, zero_list_bdry1, 0, y, lows_b2merge, nonzero_b2m_steps12, nonzero_b2m_step3);
        dim_b2m_yprev += nonzero_b2m_steps12;
        dim_b2merge = dim_b2m_yprev + nonzero_b2m_step3;
        xi[0][y][0] -= dim_b2merge; //subtracting dim(V)
        xi[0][y][1] -= dim_b2merge; //subtracting dim(V)

        //reduce bdry1 at (0,y), apply same column operations to merge and split, and record nullity
        reduce_slave(bdry1, merge, split, ind1->get(y - 1, num_x_grades - 1) + 1, ind1->get(y, 0), lows_bdry1, y, zero_list_bdry1, zero_cols_bdry1);
        xi[0][y][0] += zero_cols_bdry1; //adding nullity(bdry1_D)
        if(1 < num_x_grades)
            xi[1][y][1] += zero_cols_bdry1; //adding nullity(bdry1_B)
        if(y + 1 < num_y_grades)
            xi[0][y+1][1] += zero_cols_bdry1; //adding nullity(bdry1_C)

        hom_dims[0][y] += zero_cols_bdry1; //homology dimension at (0,y)

        //reduce bdry2 and split at (0,y) and record dimension of U
        reduce_spliced(bdry2s, split, ind2s, ind1, zero_list_bdry1, 0, y, lows_b2split, nonzero_cols_b2split);
        if(y + 1 < num_y_grades) {
            xi[1][y+1][1] -= nonzero_cols_b2split; //subtracting dim(U) at (1,y+1)
        }
    }

    //CALCULATIONS AT GRADES (x,y) FOR x > 0, y >= 0
    for(unsigned x = 1; x < num_x_grades; x++) {
        //CALCULATIONS AT GRADES (x,0) FOR x > 0
        //reduce bdry2 and merge at (x,0) and record dimension of V
        reduce_spliced(bdry2m, merge, ind2m, ind1, zero_list_bdry1, x, 0, lows_b2merge, nonzero_b2m_steps12, nonzero_b2m_step3);
        dim_bdry2_y0 += nonzero_b2m_steps12;
        dim_b2m_yprev = dim_bdry2_y0;
        dim_b2merge = dim_b2m_yprev + nonzero_b2m_step3;
        xi[x][0][0] -= dim_b2merge; //subtracting dim(V)
        xi[x][0][1] -= dim_b2merge; //subtracting dim(V)

        //reduce bdry1 at (x,0), apply same column operations to merge and split, and record nullity
        reduce_slave(bdry1, merge, split, ind1->get(0, x - 1) + 1, ind1->get(0, x), lows_bdry1, 0, zero_list_bdry1, zero_cols_b1_y0);
        zero_cols_bdry1 = zero_cols_b1_y0;
        xi[x][0][0] += zero_cols_bdry1; //adding nullity(bdry1_D)
        if(x + 1 < num_x_grades)
            xi[x+1][0][1] += zero_cols_bdry1; //adding nullity(bdry1_B)
        if(1 < num_y_grades)
            xi[x][1][1] += zero_cols_bdry1; //adding nullity(bdry1_C)

        hom_dims[x][0] += zero_cols_bdry1; //homology dimension at (x,0)

        //reduce bdry2 and split at (x,0) and record dimension of U
        nonzero_cols_b2split = 0;
        reduce_spliced(bdry2s, split, ind2s, ind1, zero_list_bdry1, x, 0, lows_b2split, nonzero_cols_b2split);
        if(x + 1 < num_x_grades) {
            xi[x+1][1][1] -= nonzero_cols_b2split; //subtracting dim(U) at (x+1,1)
        }

        //CALCULATIONS AT GRADES (x,y) FOR x,y > 0
        for(unsigned y = 1; y < num_y_grades; y++) {
            //reduce bdry2 and merge at (x,y) and record dimension of V
            reduce_spliced(bdry2m, merge, ind2m, ind1, zero_list_bdry1, x, y, lows_b2merge, nonzero_b2m_steps12, nonzero_b2m_step3);
            dim_b2m_yprev += nonzero_b2m_steps12;
            dim_b2merge = dim_b2m_yprev + nonzero_b2m_step3;
            xi[x][y][0] -= dim_b2merge; //subtracting dim(V)
            xi[x][y][1] -= dim_b2merge; //subtracting dim(V)

            //reduce bdry1 at (0,y) through (x,y), apply same column operations to merge and split, and record nullity
            reduce_slave(bdry1, merge, split, ind1->get(y - 1, num_x_grades - 1) + 1, ind1->get(y, x), lows_bdry1, y, zero_list_bdry1, zero_cols_bdry1);
            xi[x][y][0] += zero_cols_bdry1; //adding nullity(bdry1_D)
            if(x + 1 < num_x_grades)
                xi[x+1][y][1] += zero_cols_bdry1; //adding nullity(bdry1_B)
            if(y + 1 < num_y_grades)
                xi[x][y+1][1] += zero_cols_bdry1; //adding nullity(bdry1_C)

            hom_dims[x][y] += zero_cols_bdry1; //homology dimension at (x,y)

            //reduce bdry2 and split at (x,y) and record dimension of U
            reduce_spliced(bdry2s, split, ind2s, ind1, zero_list_bdry1, x, y, lows_b2split, nonzero_cols_b2split);
            if(x + 1 < num_x_grades && y + 1 < num_y_grades)
                xi[x+1][y+1][1] -= nonzero_cols_b2split; //subtracting dim(U) at (x+1,y+1)
        }
    }

    //emit progress message
    progress.progress(95);

    //clean up
    delete bdry1;
    delete ind1;
    delete bdry2m;
    delete ind2m;
    delete split;
    delete bdry2s;
    delete ind2s;
    delete merge;
}//end compute()


//computes xi_2 from the values of xi_0, xi_1 and the dimensions
void MultiBetti::compute_xi2(unsigned_matrix& hom_dims)
{
    //calculate xi_2 at (0,0)
    int row_sum = xi[0][0][0] - xi[0][0][1];
    xi[0][0][2] = hom_dims[0][0] - row_sum;

    //calculate xi_2 at (x,0) for x > 0
    for(unsigned x = 1; x < num_x_grades; x++)
    {
        row_sum += xi[x-1][0][2] + xi[x][0][0] - xi[x][0][1];
        xi[x][0][2] = hom_dims[x][0] - row_sum;
    }

    //calcuate xi_2 at (x,y) for y > 0
    for(unsigned y = 1; y < num_y_grades; y++)
    {
        //calculate xi_2 at (0,y)
        row_sum = xi[0][y][0] - xi[0][y][1];
        xi[0][y][2] = hom_dims[0][y] - (hom_dims[0][y-1] + row_sum);

        //calculate xi_2 at (x,y) for x > 0 and y > 0
        for(unsigned x = 1; x < num_x_grades; x++)
        {
            row_sum += xi[x-1][y][2] + xi[x][y][0] - xi[x][y][1];
            xi[x][y][2] = hom_dims[x][y] - (hom_dims[x][y-1] + row_sum);
        }
    }
}//end compute_xi2()

//returns xi_0 at the specified grade
int MultiBetti::xi0(unsigned x, unsigned y)
{
    return xi[x][y][0];
}//end xi0()

//returns xi_1 at the specified grade
int MultiBetti::xi1(unsigned x, unsigned y)
{
    return xi[x][y][1];
}//end xi1()

//stores the xi support points in lexicographical order
void MultiBetti::store_support_points(std::vector<TemplatePoint>& tpts)
{
    for(unsigned i = 0; i < num_x_grades; i++)
    {
        for(unsigned j = 0; j < num_y_grades; j++)
        {
            int xi0 = xi[i][j][0];
            int xi1 = xi[i][j][1];
            int xi2 = xi[i][j][2];

            if(xi0 != 0 || xi1 != 0 || xi2 != 0) //then we have found an xi support point
                tpts.push_back( TemplatePoint(i, j, xi0, xi1, xi2) );
        }
    }
}//end store_support_points()

//simple column reduction algorithm
//  pivot columns are first_col to last_col, inclusive
//  increments nonzero_cols by the number of columns in [first_col, last_col] that remained nonzero
void MultiBetti::reduce(MapMatrix* mm, int first_col, int last_col, Vector& lows, long& nonzero_cols)
{
    for(int j = first_col; j <= last_col; j++) {
        //while column j is nonempty and its low number is found in the low array, do column operations
        while(mm->low(j) >= 0 && lows[mm->low(j)] >= 0 && lows[mm->low(j)] < j) {
            mm->add_column(lows[mm->low(j)], j);
        }

        if(mm->low(j) >= 0) { //column is still nonempty
            lows[mm->low(j)] = j;
            nonzero_cols++;
        }
    }
}//end reduce()

//column reduction algorithm that also performs column operations on a slave matrix
//  pivot columns are first_col to last_col, inclusive
//  increments zero_cols by the number of zero-columns in [first_col, last_col], regardless of whether they
//      were zeroed out in this reduction or zero before this function was called
//  indexes of columns that are zeroed out are inserted into zero_list at y_grade
void MultiBetti::reduce_slave(MapMatrix* mm, MapMatrix* slave1, MapMatrix* slave2, int first_col, int last_col,Vector& lows,
                              unsigned y_grade, ColumnList& zero_list, long& zero_cols)
{
    for(int j = first_col; j <= last_col; j++) {
        //while column j is nonempty and its low number is found in the low array, do column operations
        while(mm->low(j) >= 0 && lows[mm->low(j)] >= 0 && lows[mm->low(j)] < j) {
            int col_to_add = lows[mm->low(j)];
            mm->add_column(col_to_add, j);
            slave1->add_column(col_to_add, j);
            slave2->add_column(col_to_add, j);
        }

        if(mm->low(j) >= 0) //column is still nonempty, so update lows
            lows[mm->low(j)] = j;
        else { //column is zero
            zero_cols++;
            zero_list.insert(j, y_grade);
        }
    }
}//end reduce_slave()

//column reduction of two matrices spliced together by y-grade
//  pivot columns are:
//      step 1: if grade_y > 0, then reduce right matrix at (0...grade_x, grade_y - 1)
//      step 2: reduce left matrix at (0...grade_x, grade_y)
//      step 3: if grade_x > 0, then reduce right matrix at (0...grade_x - 1, grade_y)
//  returns values:
//      nonzero_cols_steps12: of the columns reduced in steps 1 and 2, the number that remained nonzero
//      nonzero_cols_step3: of the columns reduced in step 3, the number that remained nonzero
void MultiBetti::reduce_spliced(MapMatrix* m_left, MapMatrix* m_right, IndexMatrix* ind_left, IndexMatrix* ind_right,
                    ColumnList& right_cols, unsigned grade_x, unsigned grade_y, Vector& lows,
                    long& nonzero_cols_steps12, long& nonzero_cols_step3)
{
    nonzero_cols_steps12 = 0; //ensure zero
    nonzero_cols_step3 = 0; //ensure zero

    // STEP 1: if grade_y > 0, then reduce right matrix at (0...grade_x, grade_y - 1)
    if(grade_y > 0) {
        //determine ending column
        int last_col = ind_right->get(grade_y - 1, grade_x);

        //determine starting column
        std::set<int>::iterator col_iterator = right_cols.get(grade_y - 1);
        int cur_col = last_col + 1; //default, indicating that there are no more columns in play for the right matrix
        if(col_iterator != right_cols.end(grade_y - 1)) //then there are columns in play for the right matrix
            cur_col = *col_iterator;

        //reduce columns from the right matrix
        while(cur_col <= last_col) {
            int cur_low = m_right->low(cur_col);
            while( cur_low >= 0 && lows[cur_low] >= 0 ) { //column is nonempty and its low number is found in the low array
                unsigned ulow = static_cast<unsigned>(lows[cur_low]); //from the previous line, we know lows[cur_low] is nonnegative
                if( lows[cur_low] <= ind_left->get(grade_y - 1, grade_x) ) {
                    //then column to add is in the left matrix and occurs before cur_col in the spliced matrix
                    m_right->add_column(m_left, ulow, cur_col);
//                    debug() << "  --> Step 1: added column" << lows[cur_low] << "left to " << cur_col << "right";
                } else if( ulow >= m_left->width() && ulow < (m_left->width() + cur_col) ) {
                    //then column to add is in the right matrix and occurs before cur_col
                    m_right->add_column(ulow - m_left->width(), cur_col);
//                    debug() << "  --> Step 1: added column" << lows[cur_low] - m_left->width() << "right to " << cur_col << "right";
                } else { //then there is no valid column to add to cur_col
                    break;
                }
                cur_low = m_right->low(cur_col); //get new low index for this column
            }
            if(cur_low >= 0) { //column is still nonempty
                lows[cur_low] = m_left->width() + cur_col;
                nonzero_cols_steps12++;
            }

            //move to next column
            ++col_iterator;
            if(col_iterator == right_cols.end(grade_y - 1)) //then there are no columns in the right matrix at this grade
                cur_col = last_col + 1;
            else
                cur_col = *col_iterator;
        }//end while
    }//end if(grade_y > 0)

    // STEP 2: reduce left matrix at (0...grade_x, grade_y)
    //determine ending column
    int last_col_left = ind_left->get(grade_y, grade_x);

    //determine starting column
    int first_col_left = 0;
    if(grade_y > 0)
        first_col_left = ind_left->get(grade_y - 1, ind_left->width() - 1) + 1;
    else if(grade_x > 0)
        first_col_left = ind_left->get(0, grade_x - 1) + 1;

    //reduce columns from the left matrix
    for(int j = first_col_left; j <= last_col_left; j++) {
        int cur_low = m_left->low(j);
        while(cur_low >= 0 && lows[cur_low] >= 0) { //column is nonempty and its low number is found in the low array
            unsigned ulow = static_cast<unsigned>(lows[cur_low]); //from the previous line, we know lows[cur_low] is nonnegative
            if( lows[cur_low] < j ) {
                //then column to add is in the left matrix and occurs before column j in the spliced matrix
                m_left->add_column(ulow, j);
//                debug() << "  --> Step 2: added column" << lows[cur_low] << "left to " << j << "left";
            } else if( ulow >= m_left->width() && grade_y > 0 && ind_right->get(grade_y - 1, grade_x) >= 0 &&
                       ulow <= (ind_right->get(grade_y - 1, grade_x) + m_left->width()) ) {
                //then column to add is in the right matrix
                m_left->add_column(m_right, ulow - m_left->width(), j);
//                debug() << "  --> Step 2: added column" << lows[cur_low] - m_left->width() << "right to " << j << "left";
            } else { //then there is no valid column to add to column j
                break;
            }
            cur_low = m_left->low(j); //get new low index for this column
        }

        if(cur_low >= 0) { //column is still nonempty
            lows[cur_low] = j;
            nonzero_cols_steps12++;
        }
    }//end for

    // STEP 3: if grade_x > 0, then reduce right matrix at (0...grade_x - 1, grade_y)
    if(grade_x > 0) {
        //determine ending column
        int last_col = ind_right->get(grade_y, grade_x - 1);

        //determine starting column
        std::set<int>::iterator col_iterator = right_cols.get(grade_y);
        int cur_col = last_col + 1; //default, indicating that there are no more columns in play for the right matrix
        if(col_iterator != right_cols.end(grade_y)) //then there are columns in play for the right matrix
            cur_col = *col_iterator;

        //reduce columns from the right matrix
        while(cur_col <= last_col) {
            int cur_low = m_right->low(cur_col);
            while( cur_low >= 0 && lows[cur_low] >= 0 ) { //column is nonempty and its low number is found in the low array
                unsigned ulow = static_cast<unsigned>(lows[cur_low]); //from the previous line, we know lows[cur_low] is nonnegative
                if( lows[cur_low] <= ind_left->get(grade_y, grade_x) ) {
                    //then column to add is in the left matrix and occurs before cur_col in the spliced matrix
                    m_right->add_column(m_left, ulow, cur_col);
//                    debug() << "  --> Step 3: added column" << lows[cur_low] << "left to " << cur_col << "right";
                } else if( ulow >= m_left->width() && ulow < (m_left->width() + cur_col) ) {
                    //then column to add is in the right matrix and occurs before cur_col
                    m_right->add_column(ulow - m_left->width(), cur_col);
//                    debug() << "  --> Step 3: added column" << lows[cur_low] - m_left->width() << "right to " << cur_col << "right";
                } else { //then there is no valid column to add to cur_col
                    break;
                }
                cur_low = m_right->low(cur_col); //get new low index for this column
            }
            if(cur_low >= 0) { //column is still nonempty
                lows[cur_low] = m_left->width() + cur_col;
                nonzero_cols_step3++;
            }

            //move to next column
            ++col_iterator;
            if(col_iterator == right_cols.end(grade_y)) //then there are no columns in play for the right matrix
                cur_col = last_col + 1;
            else
                cur_col = *col_iterator;
        }//end while
    }//end if(grade_x > 0)
}//end reduce_spliced()

//version of reduce_spliced for computing dim(U) for xi_1
// increments nonzero_cols by the number of columns that were reduced and remained nonzero
///FIXME: NOT SURE IF THIS SHOULD BE A SEPARATE FUNCTION FROM THE ABOVE
void MultiBetti::reduce_spliced(MapMatrix* m_left, MapMatrix* m_right, IndexMatrix* ind_left, IndexMatrix* ind_right,
                    ColumnList& right_cols, unsigned grade_x, unsigned grade_y, Vector& lows, long& nonzero_cols)
{
//    debug() << "reduce_spliced(" << grade_x << "," << grade_y << "):";
    // STEP 1: reduce left matrix at (0...grade_x, grade_y)
    //determine ending column
    int last_col_left = ind_left->get(grade_y, grade_x);

    //determine starting column
    int first_col_left = 0;
    if(grade_y > 0)
        first_col_left = ind_left->get(grade_y - 1, ind_left->width() - 1) + 1;
///FIXME: the following slight optimization requires better bookkeeping of nonzero columns
//    else if(grade_x > 0)
//        first_col_left = ind_left->get(0, grade_x - 1) + 1;

    //reduce columns from the left matrix
//    debug() << "  reduce_spliced(" << grade_x << "," << grade_y << "): reducing LEFT matrix, columns" << first_col_left << "to" << last_col_left;
    for(int j = first_col_left; j <= last_col_left; j++) {
//        debug() << "  reducing LEFT column" << j;
        int cur_low = m_left->low(j);
        while(cur_low >= 0 && lows[cur_low] >= 0) { //column is nonempty and its low number is found in the low array
            unsigned ulow = static_cast<unsigned>(lows[cur_low]); //from the previous line, we know lows[cur_low] is nonnegative
            if( lows[cur_low] < j ) {
                //then column to add is in the left matrix and occurs before column j in the spliced matrix
                m_left->add_column(ulow, j);
//                debug() << "  --> Step 2: added column" << lows[cur_low] << "left to " << j << "left";
            } else if( ulow >= m_left->width() && grade_y > 0 && ind_right->get(grade_y - 1, grade_x) >= 0 &&
                       ulow <= (ind_right->get(grade_y - 1, grade_x) + m_left->width()) ) {
                //then column to add is in the right matrix
                m_left->add_column(m_right, ulow - m_left->width(), j);
//                debug() << "  --> Step 2: added column" << lows[cur_low] - m_left->width() << "right to " << j << "left";
            } else { //then there is no valid column to add to column j
                break;
            }
            cur_low = m_left->low(j); //get new low index for this column
        }

        if(cur_low >= 0) { //column is still nonempty
            lows[cur_low] = j;
            nonzero_cols++;
//            debug() << "    still nonempty";
        }
    }//end for

    ///DEBUGGING
//    long nz = nonzero_cols;
//    if(grade_y == 0) {
//        debug() << "  reduce_spliced(" << grade_x << "," << grade_y << "): nonzero cols in LEFT matrix:" << nonzero_cols;
//    }

    // STEP 2: reduce right matrix at (0...grade_x, grade_y)
    //determine ending column
    int last_col = ind_right->get(grade_y, grade_x);

    //determine starting column
    std::set<int>::iterator col_iterator = right_cols.get(grade_y);
    int cur_col = last_col + 1; //default, indicating that there are no more columns in play for the right matrix
    if(col_iterator != right_cols.end(grade_y)) //then there are columns in play for the right matrix
        cur_col = *col_iterator;

    //reduce columns from the right matrix
    while(cur_col <= last_col) {
//        debug() << "  reducing RIGHT column" << cur_col;
        int cur_low = m_right->low(cur_col);
        while( cur_low >= 0 && lows[cur_low] >= 0 ) { //column is nonempty and its low number is found in the low array
            unsigned ulow = static_cast<unsigned>(lows[cur_low]); //from the previous line, we know lows[cur_low] is nonnegative
            if( lows[cur_low] <= ind_left->get(grade_y, grade_x) ) {
                //then column to add is in the left matrix and occurs before cur_col in the spliced matrix
                m_right->add_column(m_left, ulow, cur_col);
//                    debug() << "  --> Step 3: added column" << lows[cur_low] << "left to " << cur_col << "right";
            } else if( ulow >= m_left->width() && ulow < (m_left->width() + cur_col) ) {
                //then column to add is in the right matrix and occurs before cur_col
                m_right->add_column(ulow - m_left->width(), cur_col);
//                    debug() << "  --> Step 3: added column" << lows[cur_low] - m_left->width() << "right to " << cur_col << "right";
            } else { //then there is no valid column to add to cur_col
                break;
            }
            cur_low = m_right->low(cur_col); //get new low index for this column
        }
        if(cur_low >= 0) { //column is still nonempty
            lows[cur_low] = m_left->width() + cur_col;
            nonzero_cols++;
//            debug() << "    still nonempty";
        }

        //move to next column
        ++col_iterator;
        if(col_iterator == right_cols.end(grade_y)) //then there are no columns in play for the right matrix
            cur_col = last_col + 1;
        else
            cur_col = *col_iterator;
    }//end while
    ///DEBUGGING
//    if(grade_y == 0) {
//        debug() << "       nonzero cols in split matrix:" << (nonzero_cols - nz);
//    }

}//end reduce_spliced()

//builds the bdry2 matrix of the direct sum B + C
//  input: matrix bdry2, corresponding index matrix ind2
//  output: matrix bdry2sum, corresponding index matrix ind2sum
void MultiBetti::build_bdry2s_mx(MapMatrix* bdry2, IndexMatrix* ind2, MapMatrix* bdry2sum, IndexMatrix* ind2sum)
{
    ///TESTING
//    debug() << "build_bdry2s_mx(): ind2 contains:";
//    ind2->print();

    //ensure bdry2sum is of the correct size
    int num_cols = ind2->get(num_y_grades - 2, num_x_grades - 1) + 1; //total number of columns in C component
    num_cols += ind2->get(0, num_x_grades - 2) + 1; //number of columns in B component at y=0 grades
    for(unsigned y = 1; y < num_y_grades; y++)
        num_cols += ind2->get(y, num_x_grades - 2) - ind2->get(y - 1, num_x_grades - 1); //number of columns in B component at y
    bdry2sum->reserve_cols(num_cols);

    //write columns at grades (x,y) for y > 0
    for(unsigned y = 0; y + 1 < num_y_grades; y++) { //not necessary to consider y = num_y_grades - 1
        for(unsigned x = 0; x < num_x_grades - 1; x++) { //not necessary to consider x = num_x_grades - 1
            //columns in the B summand live at <= (x,y+1), and are copied without offset
            if(y == 0) { //copy columns at (x,0)
                if(x > 0)
                    bdry2sum->copy_cols_from(bdry2, ind2->get(0, x - 1) + 1, ind2->get(0, x), 0);
                else
                    bdry2sum->copy_cols_from(bdry2, 0, ind2->get(0, 0), 0);
            }
            //copy columns at (x,y+1)
            if(x > 0)
                bdry2sum->copy_cols_from(bdry2, ind2->get(y + 1, x - 1) + 1, ind2->get(y + 1, x), 0);
            else
                bdry2sum->copy_cols_from(bdry2, ind2->get(y, num_x_grades - 1) + 1, ind2->get(y + 1, x), 0);

            //columns in the C summand live at <= (x+1,y), and are copied with offset
            if(x == 0) { //copy columns at (0,y)
                if(y > 0)
                    bdry2sum->copy_cols_from(bdry2, ind2->get(y - 1, num_x_grades - 1) + 1, ind2->get(y, 0), bdry2->height());
                else
                    bdry2sum->copy_cols_from(bdry2, 0, ind2->get(0, 0), bdry2->height());
            }
            //copy columns at (x+1,y)
            bdry2sum->copy_cols_from(bdry2, ind2->get(y, x) + 1, ind2->get(y, x + 1), bdry2->height());

            //record last column index for this grade
            ind2sum->set(y, x, bdry2sum->width() - 1);
        }
        //record last column index at (num_x_grades - 1, y) --- TODO: eliminate the need to do this!
        ind2sum->set(y, num_x_grades - 1, bdry2sum->width() - 1);
    }

    //record last column index grades y = num_y_grades - 1 --- TODO: eliminate the need to do this!
    for(unsigned x = 0; x < num_x_grades; x++) {
        ind2sum->set(num_y_grades - 1, x, bdry2sum->width() - 1);
    }

    ///TESTING ONLY
    //if(num_cols == bdry2sum->width())
    //    debug() << "build_bdry2s_mx(): num_cols OK";
    //else
    //    debug() << "=====>>>>> build_bdry2s_mx(): ERROR: num_cols DOES NOT MATCH bdry2sum->width()";
}//end build_bdry2s_mx

//builds the split matrix
//  input: number of columns in the matrix
//  output: matrix containing size columns and 2*size rows; matrix is two stacked copies of the (size x size) identity matrix
MapMatrix* MultiBetti::build_split_mx(unsigned size)
{
    MapMatrix* mat = new MapMatrix(2*size, size);
    for(unsigned i = 0; i < size; i++)
    {
        mat->set(i, i);
        mat->set(i+size, i);
    }
    return mat;
}//end build_split_mx()
