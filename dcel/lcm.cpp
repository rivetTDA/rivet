#include "lcm.h"

#include <cstddef>  //for NULL
#include <math.h>


/*** IMPLEMENTATION OF LCM CLASS ***/

LCM::LCM(double x, double y) : x_coord(x), y_coord(y), curve(NULL)
{ }

LCM::LCM(double x, double y, Halfedge* e) : x_coord(x), y_coord(y), curve(e)
{ }

LCM::LCM(const LCM& other)
{
    x_coord = other.x_coord;
    y_coord = other.y_coord;
    curve = other.curve;
    position = other.position;
}

LCM& LCM::operator= (const LCM& other)
{
    //check for self-assignment
    if(this == &other)
        return *this;

    //do the copy
    x_coord = other.x_coord;
    y_coord = other.y_coord;
    curve = other.curve;
    position = other.position;

    return *this;
}

bool LCM::operator== (const LCM& other) const
{
    return (x_coord == other.x_coord && y_coord == other.y_coord);
}

double LCM::get_x() const
{
    return x_coord;
}

double LCM::get_y() const
{
    return y_coord;
}

void LCM::set_curve(Halfedge* e)
{
    curve = e;
}

Halfedge* LCM::get_curve() const
{
    return curve;
}

void LCM::set_position(unsigned p)
{
    position = p;
}

unsigned LCM::get_position() const
{
    return position;
}

double LCM::get_r_coord(double theta)
{
    return sqrt( x_coord*x_coord + y_coord*y_coord ) * sin( atan(y_coord/x_coord) - theta );
}
