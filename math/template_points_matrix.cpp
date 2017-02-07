/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
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

#include "template_points_matrix.h"
#include "dcel/arrangement.h"

#include "multi_betti.h"
#include "template_point.h"

#include "debug.h"

#include <cstddef> //for NULL

/********** xiMatrixEntry **********/

//empty constructor
TemplatePointsMatrixEntry::TemplatePointsMatrixEntry()
    : x(-1)
    , y(-1)
    , index(-1)
    , //sets these items to MAX_UNSIGNED, right?
    down(NULL)
    , left(NULL)
    , low_count(0)
    , high_count(0)
    , low_index(0)
    , high_index(0)
{
}

//regular constructor
TemplatePointsMatrixEntry::TemplatePointsMatrixEntry(unsigned x, unsigned y, unsigned i, std::shared_ptr<TemplatePointsMatrixEntry> d, std::shared_ptr<TemplatePointsMatrixEntry> l)
    : x(x)
    , y(y)
    , index(i)
    , down(d)
    , left(l)
    , low_count(0)
    , high_count(0)
    , low_index(0)
    , high_index(0)
{
}

//constructor for temporary entries used in counting switches
TemplatePointsMatrixEntry::TemplatePointsMatrixEntry(unsigned x, unsigned y)
    : x(x)
    , y(y)
    , down(NULL)
    , left(NULL)
    , low_count(0)
    , high_count(0)
    , low_index(0)
    , high_index(0)
{
}

//associates a multigrades to this xi entry
//the "low" argument is true if this multigrade is for low_simplices, and false if it is for high_simplices
void TemplatePointsMatrixEntry::add_multigrade(unsigned x, unsigned y, unsigned num_cols, int index, bool low)
{
    if (low) {
        low_simplices.push_back(std::make_shared<Multigrade>(x, y, num_cols, index));
        low_count += num_cols;
    } else {
        high_simplices.push_back(std::make_shared<Multigrade>(x, y, num_cols, index));
        high_count += num_cols;
    }
}

//inserts a Multigrade at the beginning of the list for the given dimension
void TemplatePointsMatrixEntry::insert_multigrade(std::shared_ptr<Multigrade> mg, bool low)
{
    if (low)
        low_simplices.push_back(mg);
    else
        high_simplices.push_back(mg);
}

/********** Multigrade **********/

//constructor
Multigrade::Multigrade(unsigned x, unsigned y, unsigned num_cols, int simplex_index)
    : x(x)
    , y(y)
    , num_cols(num_cols)
    , simplex_index(simplex_index)
{
}

Multigrade::Multigrade()
    : x(0)
    , y(0)
    , num_cols(0)
    , simplex_index(0)
{
}

//comparator for sorting Multigrades (reverse) lexicographically
bool Multigrade::LexComparator(const Multigrade& first, const Multigrade& second)
{
    return first.x > second.x || (first.x == second.x && first.y > second.y);
}

/********** TemplatePointsMatrix **********/

//constructor for TemplatePointsMatrix
TemplatePointsMatrix::TemplatePointsMatrix(unsigned width, unsigned height)
    : columns(width)
    , rows(height)
{
}

//stores the supplied xi support points in the TemplatePointsMatrix
//  also finds anchors, which are stored in the matrix and in the vector xi_pts
//  precondition: xi_pts contains the support points in lexicographical order
//  Runtime complexity of this function is O(n_x * n_y). We can probably do better, but it probably doesn't matter.
std::vector<std::shared_ptr<TemplatePointsMatrixEntry>> TemplatePointsMatrix::fill_and_find_anchors(std::vector<TemplatePoint>& xi_pts)
{
    unsigned next_xi_pt = 0; //tracks the index of the next xi support point to insert

    std::vector<std::shared_ptr<TemplatePointsMatrixEntry>> matrix_entries;

    //loop over all grades in lexicographical order
    for (unsigned i = 0; i < columns.size(); i++) {
        for (unsigned j = 0; j < rows.size(); j++) {
            //see if the next xi support point is in position (i,j)
            bool xi_pt = (xi_pts.size() > next_xi_pt
                && xi_pts[next_xi_pt].x == i
                && xi_pts[next_xi_pt].y == j);

            //see if there is an anchor at position (i,j)
            bool anchor = (columns[i] != NULL && rows[j] != NULL) // strict anchor
                || (xi_pt && (columns[i] != NULL || rows[j] != NULL)); //non-strict anchor at (i,j)

            if (!(xi_pt || anchor))
                continue;

            //insert a new TemplatePointsMatrixEntry
            auto insertion_point = -1;

            if (xi_pt) {
                // debug() << "  creating TemplatePointsMatrixEntry at (" << i << ", " << j << ") for xi point " << next_xi_pt;
                insertion_point = next_xi_pt++;
            } else {
                insertion_point = xi_pts.size();
                //debug() << "  creating TemplatePointsMatrixEntry at (" << i << "," << j << ") for an anchor; index = " << insertion_point;

                //add this point to xi_pts
                xi_pts.push_back(TemplatePoint(i, j, 0, 0, 0));
            }

            //create a new TemplatePointsMatrixEntry
            std::shared_ptr<TemplatePointsMatrixEntry> new_entry(new TemplatePointsMatrixEntry(i, j, insertion_point, columns[i], rows[j]));
            columns[i] = new_entry;
            rows[j] = new_entry;

            if (anchor) {
                matrix_entries.push_back(new_entry);
            }
        }
    }
    return matrix_entries;
} //end fill_and_find_anchors()

//gets a pointer to the rightmost entry in row r; returns NULL if row r is empty
std::shared_ptr<TemplatePointsMatrixEntry> TemplatePointsMatrix::get_row(unsigned r)
{
    return rows[r];
}

//gets a pointer to the top entry in column c; returns NULL if column c is empty
std::shared_ptr<TemplatePointsMatrixEntry> TemplatePointsMatrix::get_col(unsigned c)
{
    return columns[c];
}

//retuns the number of rows
unsigned TemplatePointsMatrix::height()
{
    return rows.size();
}

//clears the level set lists for all entries in the matrix
void TemplatePointsMatrix::clear_grade_lists()
{
    for (unsigned i = 0; i < columns.size(); i++) {
        std::shared_ptr<TemplatePointsMatrixEntry> cur_entry = columns[i];
        while (cur_entry != NULL) {
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
