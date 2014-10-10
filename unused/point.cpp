#include "point.h"

#include <iostream> //for testing only

Point::Point(double* cor, double bir)
{
    //std::cout << "in constructor, cor: " << cor[0] << ", " << cor[1] << "\n";
    coords = cor;
    //std::cout << "in constructor, coords: " << coords[0] << ", " << coords[1] << "\n";
    birth = bir;

}

double* Point::get_coords()
{
    return coords;
}

double Point::get_birth()
{
    return birth;
}

void Point::print()
{
    std::cout << "  Point.print(): " << coords[0] << ", " << coords[1] << "; " << birth << "; coords-address: " << coords << "\n ";
}

//overloaded less than operator, for sorting a list of Points
bool Point::operator < (const Point & other) const
{
    if(birth < other.birth)
        return true;
    /* else */
    return false;
}

//SHOULD OVERLOAD OTHER COMPARISON OPERATORS TOO!!!
