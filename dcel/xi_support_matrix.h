#ifndef XI_SUPPORT_MATRIX_H
#define XI_SUPPORT_MATRIX_H


#include <vector>

#include "../math/multi_betti.h"

#include "xi_point.h"

//// these are the nodes in the sparse matrix
struct xiMatrixEntry
{
    unsigned x;     //discrete x-grade of this support point
    unsigned y;     //discrete y-grade of this support point
    unsigned index; //index of this support point in the vector of support points stored in VisualizationWindow
    xiMatrixEntry* down;     //pointer to the next support point below this one
    xiMatrixEntry* left;     //pointer to the next support point left of this one
    ///TODO: also needs a set of associated multi-grades

    xiMatrixEntry(unsigned x, unsigned y, unsigned i, xiMatrixEntry* d, xiMatrixEntry* l);  //constructor
};

//// sparse matrix to store the set U of support points of the multi-graded Betti numbers
class xiSupportMatrix
{
    public:
        xiSupportMatrix(unsigned width, unsigned height);
        ~xiSupportMatrix();

        void fill(MultiBetti& mb, std::vector<xiPoint>& xi_pts); //stores xi support points from MultiBetti in the xiSupportMatrix and in the supplied vector

        xiMatrixEntry* get_row(unsigned r); //gets a pointer to the leftmost entry in row r; returns NULL if row r is empty
        xiMatrixEntry* get_col(unsigned c); //gets a pointer to the top entry in column c; returns NULL if column c is empty

        ///TODO: how to return the function F: S --> U???

private:
        std::vector<xiMatrixEntry*> columns;
        std::vector<xiMatrixEntry*> rows;
};

#endif // XI_SUPPORT_MATRIX_H
