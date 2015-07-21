#include "xi_support_matrix.h"

#include "multi_betti.h"
#include "xi_point.h"

#include <cstddef>  //for NULL


/********** xiMatrixEntry **********/

//empty constructor, e.g. for the entry representing infinity
xiMatrixEntry::xiMatrixEntry() :
    x(-1), y(-1), index(-1),    //sets these items to MAX_UNSIGNED, right?
    down(NULL), left(NULL),
    low_count(0), high_count(0), low_class_size(-1), high_class_size(0), low_index(0), high_index(0)
{ }

//regular constructor
xiMatrixEntry::xiMatrixEntry(unsigned x, unsigned y, unsigned i, xiMatrixEntry* d, xiMatrixEntry* l) :
    x(x), y(y), index(i), down(d), left(l),
    low_count(0), high_count(0), low_class_size(-1), high_class_size(0), low_index(0), high_index(0)
{ }

//associates a multigrades to this xi entry
//the "low" argument is true if this multigrade is for low_simplices, and false if it is for high_simplices
void xiMatrixEntry::add_multigrade(unsigned x, unsigned y, unsigned num_cols, int index, bool low)
{
    if(low)
    {
        low_simplices.push_back(new Multigrade(x, y, num_cols, index));
        low_count += num_cols;
    }
    else
    {
        high_simplices.push_back(new Multigrade(x, y, num_cols, index));
        high_count += num_cols;
    }
}

//inserts a Multigrade at the beginning of the list for the given dimension
void xiMatrixEntry::insert_multigrade(Multigrade* mg, bool low)
{
    if(low)
        low_simplices.push_back(mg);
    else
        high_simplices.push_back(mg);
}

//moves all Multigrades from bin to this entry
//  for use when merging two classes during lazy updates
void xiMatrixEntry::move_bin_here(xiMatrixEntry* bin)
{
    //move low simplices
    for(std::list<Multigrade*>::iterator it = bin->low_simplices.begin(); it != bin->low_simplices.end(); ++it)
        low_simplices.push_back(*it);
    bin->low_simplices.clear();

    //move high simplices
    for(std::list<Multigrade*>::iterator it = bin->high_simplices.begin(); it != bin->high_simplices.end(); ++it)
        high_simplices.push_back(*it);
    bin->high_simplices.clear();

    //update counters
    low_count += bin->low_count;
    bin->low_count = 0;
    high_count += bin->high_count;
    bin->high_count = 0;
}


/********** Multigrade **********/

//constructor
Multigrade::Multigrade(unsigned x, unsigned y, unsigned num_cols, int simplex_index) :
    x(x), y(y), num_cols(num_cols), simplex_index(simplex_index)
{ }

//comparator for sorting Multigrades (reverse) lexicographically
bool Multigrade::LexComparator(const Multigrade* first, const Multigrade* second)
{
    if( first->x > second->x || (first->x == second->x && first->y > second->y) )
        return true;
    //else
    return false;
}

/********** xiSupportMatrix **********/

//constructor for xiSupportMatrix
xiSupportMatrix::xiSupportMatrix(unsigned width, unsigned height) :
    columns(width), rows(height), infinity(),
    col_bins(width), row_bins(height)
{ }

//destructor
xiSupportMatrix::~xiSupportMatrix()
{
    //suffices to clear all columns
    for(unsigned i=0; i<columns.size(); i++)
    {
        while(columns[i] != NULL)
        {
            xiMatrixEntry* cur = columns[i];
            columns[i] = cur->down;
            delete cur;
        }
    }
}

//stores xi support points from MultiBetti in the xiSupportMatrix and in the supplied vector
///TODO: DEPRECATED
//void xiSupportMatrix::fill(MultiBetti& mb, std::vector<xiPoint>& xi_pts)
//{
//    unsigned num_xi_pts = 0;

//    for(unsigned i=0; i<columns.size(); i++)
//    {
//        for(unsigned j=0; j<rows.size(); j++)
//        {
//            if(mb.xi0(i,j) != 0 || mb.xi1(i,j) != 0)    //then we have found an xi support point
//            {
//                //add this point to the vector -- THIS IS NOW DONE IN MultiBetti::store_support_points()
////                xi_pts.push_back( xiPoint(i, j, mb.xi0(i,j), mb.xi1(i,j)) );   //index in the vector is num_xi_pts

//                //add this point to the sparse matrix
//                xiMatrixEntry* cur_entry = new xiMatrixEntry(i, j, num_xi_pts, columns[i], rows[j]);
//                columns[i] = cur_entry;
//                rows[j] = cur_entry;

//                //increment the index
//                num_xi_pts++;
//            }
//        }
//    }
//}//end fill()

//stores xi support points in the xiSupportMatrix
// precondition: xi_pts contains the support points in lexicographical order
void xiSupportMatrix::fill(std::vector<xiPoint>& xi_pts)
{
    //store xi support points
    for(unsigned i = 0; i < xi_pts.size(); i++)
    {
        unsigned x = xi_pts[i].x;
        unsigned y = xi_pts[i].y;

        xiMatrixEntry* cur_entry = new xiMatrixEntry(x, y, i, columns[x], rows[y]);
        columns[x] = cur_entry;
        rows[y] = cur_entry;
    }
}//end fill()

//stores points of LUB^{e_0}, where e_0 is the cell corresponding to vertical lines to the right of all support points -- fix for major bug in July 2015
///  NOTE: DOES NOT UPDATE THE down POINTERS OF xiMatrixEntrys, BUT THIS SHOULD BE OK
///        LUB^{e_0} points that are not xi support points should be removed before the persistence update process!
void xiSupportMatrix::store_LUBe0_points()
{
    xiMatrixEntry* rightmost = rows[0];
    for(unsigned i = 1; i < rows.size(); i++)
    {
        if(rightmost == NULL)    //then row i is the lowest nonempty row
            rightmost = rows[i];
        else if(rows[i] != NULL)   //then see if we need to store a LUB point in row i
        {
            if(rows[i]->x < rightmost->x)
            {
                xiMatrixEntry* lub_entry = new xiMatrixEntry(i, rightmost->x, -1, NULL, rows[i]);
                rows[i] = lub_entry;
            }
            else if(rows[i]->x > rightmost->x)
                rightmost = rows[i];
        }
    }
}//end store_LUBe0_points()

//removes points of LUB^{e_0} that are not xi support points
void xiSupportMatrix::remove_LUBe0_points()
{
    for(unsigned i = 1; i < rows.size(); i++)   //all entries in row 0 must be xi support points
    {
        if(rows[i] != NULL && rows[i]->index == -1)    //then rightmost entry in row i is a LUB^{e_0} point but not a xi support point
        {
            xiMatrixEntry* entry = rows[i];
            rows[i] = entry->left;
            delete entry;
        }
    }
}//end remove_LUBe0_points()

//gets a pointer to the rightmost entry in row r; returns NULL if row r is empty
xiMatrixEntry* xiSupportMatrix::get_row(unsigned r)
{
    return rows[r];
}

//gets a pointer to the top entry in column c; returns NULL if column c is empty
xiMatrixEntry* xiSupportMatrix::get_col(unsigned c)
{
    return columns[c];
}

//gets a pointer to the infinity entry
xiMatrixEntry* xiSupportMatrix::get_infinity()
{
    return &infinity;
}

//retuns the number of rows
unsigned xiSupportMatrix::height()
{
    return rows.size();
}

//gets a pointer to the "bin" of unsorted grades for row r
xiMatrixEntry* xiSupportMatrix::get_row_bin(unsigned r)
{
    return &row_bins[r];
}

//gets a pointer to the "bin" of unsorted grades for column c
xiMatrixEntry* xiSupportMatrix::get_col_bin(unsigned c)
{
    return &col_bins[c];
}
