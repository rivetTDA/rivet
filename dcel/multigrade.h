#ifndef MULTIGRADE_H
#define MULTIGRADE_H


struct xiMatrixEntry;


struct Multigrade
{
    unsigned x;     //x-coordinate of this multigrade
    unsigned y;     //y-coordinate of this multigrade

    unsigned num_cols; //number of columns (i.e. simplices) at this multigrade
    int simplex_index; //last dim_index of the simplices at this multigrade; necessary so that we can build the boundary matrix

    xiMatrixEntry* xi_entry; //pointer to the entry in the xiSupportMatrix to which this multigrade is assocated

    Multigrade(unsigned x, unsigned y, unsigned num_cols, int simplex_index, xiMatrixEntry* xi);   //constructor
};

#endif // MULTIGRADE_H
