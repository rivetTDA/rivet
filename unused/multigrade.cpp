#include "multigrade.h"

Multigrade::Multigrade(unsigned x, unsigned y, unsigned num_cols, int simplex_index, xiMatrixEntry* xi)
    : x(x)
    , y(y)
    , num_cols(num_cols)
    , simplex_index(simplex_index)
    , xi_entry(xi)
{
}
