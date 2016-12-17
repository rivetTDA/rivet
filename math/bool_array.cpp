//
// Created by Bryn Keller on 11/22/16.
//

#include "bool_array.h"

bool_array::bool_array(unsigned long rows, unsigned long cols)
    : cols(cols)
    , array(new bool[rows * cols])
{
}

bool& bool_array::at(unsigned long row, unsigned long col) { return array[row * cols + col]; }
