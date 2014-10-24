#include "xi_support_matrix.h"

//constructor for xiMatrixEntry
xiMatrixEntry::xiMatrixEntry(unsigned x, unsigned y, unsigned i, xiMatrixEntry* d, xiMatrixEntry* l) :
    x(x), y(y), index(i), down(d), left(l)
{ }



//constructor for xiSupportMatrix
xiSupportMatrix::xiSupportMatrix(unsigned width, unsigned height) :
    columns(width), rows(height)
{ }

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
void xiSupportMatrix::fill(MultiBetti& mb, std::vector<xiPoint>& xi_pts)
{
    unsigned num_xi_pts = 0;

    for(unsigned i=0; i<columns.size(); i++)
    {
        for(unsigned j=0; j<rows.size(); j++)
        {
            if(mb.xi0(i,j) != 0 || mb.xi1(i,j) != 0)    //then we have found an xi support point
            {
                //add this point to the vector
                xi_pts.push_back( xiPoint(i, j, mb.xi0(i,j), mb.xi1(i,j)) );   //index in the vector is num_xi_pts

                //add this point to the sparse matrix
                xiMatrixEntry* cur_entry = new xiMatrixEntry(i, j, num_xi_pts, columns[i], rows[j]);
                columns[i] = cur_entry;
                rows[j] = cur_entry;

                //increment the index
                num_xi_pts++;
            }
        }
    }
}//end fill()

//gets a pointer to the leftmost entry in row r; returns NULL if row r is empty
xiMatrixEntry* xiSupportMatrix::get_row(unsigned r)
{
    return rows[r];
}

//gets a pointer to the top entry in column c; returns NULL if column c is empty
xiMatrixEntry* xiSupportMatrix::get_col(unsigned c)
{
    return columns[c];
}
