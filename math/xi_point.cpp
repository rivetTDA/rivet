#include "xi_point.h"

xiPoint::xiPoint(unsigned xc, unsigned yc, int m0, int m1, int m2) :
    x(xc), y(yc),
    zero(m0), one(m1), two(m2)
{ }

xiPoint::xiPoint():x(0), y(0), zero(0), one(0), two(0) { }


bool operator==(xiPoint const &left, xiPoint const &right) {
    return left.one == right.one
            && left.two == right.two
            && left.x == right.x
            && left.y == right.y
            && left.zero == right.zero;
}
