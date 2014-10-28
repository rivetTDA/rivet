#ifndef MULTIGRADE_H
#define MULTIGRADE_H


struct xiMatrixEntry;


struct Multigrade
{
    unsigned x;     //x-coordinate of this multigrade
    unsigned y;     //y-coordinate of this multigrade

    unsigned first; //index of the beginning of the block of columns corresponding to this multigrade in the boundary matrix
    unsigned last;  //index of the end of the block of columns corresponding to this multigrade in the boundary matrix

    xiMatrixEntry* xi_entry; //pointer to the entry in the xiSupportMatrix to which this multigrade is assocated

    Multigrade(unsigned x, unsigned y, unsigned first_col, unsigned last_col, xiMatrixEntry* xi);   //constructor
};

#endif // MULTIGRADE_H
