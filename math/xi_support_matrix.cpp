#include "xi_support_matrix.h"
#include "../dcel/mesh.h"

#include "multi_betti.h"
#include "xi_point.h"

#include "debug.h"

#include <cstddef>  //for NULL


/********** xiMatrixEntry **********/

//empty constructor
xiMatrixEntry::xiMatrixEntry() :
    x(-1), y(-1), index(-1),    //sets these items to MAX_UNSIGNED, right?
    down(NULL), left(NULL),
    low_count(0), high_count(0), low_index(0), high_index(0)
{ }

//regular constructor
xiMatrixEntry::xiMatrixEntry(unsigned x, unsigned y, unsigned i, xiMatrixEntry* d, xiMatrixEntry* l) :
    x(x), y(y), index(i), down(d), left(l),
    low_count(0), high_count(0), low_index(0), high_index(0)
{ }

//constructor for temporary entries used in counting switches
xiMatrixEntry::xiMatrixEntry(unsigned x, unsigned y) :
    x(x), y(y), down(NULL), left(NULL),
    low_count(0), high_count(0), low_index(0), high_index(0)
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
    columns(width), rows(height)
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

//stores the supplied xi support points in the xiSupportMatrix
//  also finds anchors, which are stored in the matrix, the vector xi_pts, AND in the Mesh
//  precondition: xi_pts contains the support points in lexicographical order
///NOTE: WRITTEN FOR JULY 2015 BUG FIX
///      Runtime complexity of this function is O(n_x * n_y). We can probably do better, but it probably doesn't matter.
void xiSupportMatrix::fill_and_find_anchors(std::vector<xiPoint>& xi_pts, Mesh& mesh)
{
    unsigned next_xi_pt = 0;    //tracks the index of the next xi support point to insert

    //loop over all grades in lexicographical order
    for(unsigned i = 0; i < columns.size(); i++)
    {
        for(unsigned j = 0; j < rows.size(); j++)
        {
            //see if the next xi support point is in position (i,j)
            bool xi_pt = false;
            if(xi_pts.size() > next_xi_pt && xi_pts[next_xi_pt].x == i && xi_pts[next_xi_pt].y == j)
                xi_pt = true;

            //see if there is an anchor at position (i,j)
            bool anchor = false;
            if(columns[i] != NULL && rows[j] != NULL)   //then there is a strict anchor at (i,j)
                anchor = true;
            else if(xi_pt && (columns[i] != NULL || rows[j] != NULL))   //then there is a non-strict anchor at (i,j)
                anchor = true;

            //insert a new xiMatrixEntry
            if(xi_pt)
            {
                debug() << "  creating xiMatrixEntry at (" << i << ", " << j << ") for xi point " << next_xi_pt ;

                //create a new xiMatrixEntry
                xiMatrixEntry* new_entry = new xiMatrixEntry(i, j, next_xi_pt, columns[i], rows[j]);
                columns[i] = new_entry;
                rows[j] = new_entry;
                next_xi_pt++;

                //if this is also an anchor, send it to the Mesh
                if(anchor)
                    mesh.add_anchor(new_entry);
            }
            else if(anchor)
            {
                //create a new xiMatrixEntry
                unsigned entry_index = xi_pts.size();

                debug() << "  creating xiMatrixEntry at (" << i << "," << j << ") for an anchor; index = " << entry_index ;

                xiMatrixEntry* new_entry = new xiMatrixEntry(i, j, entry_index, columns[i], rows[j]);
                columns[i] = new_entry;
                rows[j] = new_entry;

                //add this point to xi_pts
                xi_pts.push_back( xiPoint(i, j, 0, 0, 0) );

                //send this anchor to the Mesh
                mesh.add_anchor(new_entry);
            }
        }
    }
}//end fill_and_find_anchors()

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

//retuns the number of rows
unsigned xiSupportMatrix::height()
{
    return rows.size();
}

//clears the level set lists for all entries in the matrix
void xiSupportMatrix::clear_grade_lists()
{
    for(unsigned i = 0; i < columns.size(); i++)
    {
        xiMatrixEntry* cur_entry = columns[i];
        while(cur_entry != NULL)
        {
            cur_entry->low_simplices.clear();
            cur_entry->high_simplices.clear();
            cur_entry->low_count = 0;
            cur_entry->high_count = 0;
            cur_entry->low_index = 0;
            cur_entry->high_index = 0;
            cur_entry = cur_entry->down;
        }
    }
}
