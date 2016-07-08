#include "xi_point.h"

xiPoint::xiPoint(unsigned xc, unsigned yc, int m0, int m1, int m2) :
    x(xc), y(yc),
    zero(m0), one(m1), two(m2)
{ }

xiPoint::xiPoint() { }
