#include "lcm.h"

#include <cstddef>  //for NULL
#include <math.h>

#include "xi_support_matrix.h"

//constructor for a NON-WEAK LCM, requires pointers to the "generators" of the LCM
LCM::LCM(xiMatrixEntry* down, xiMatrixEntry* left) : x_coord(down->x), y_coord(left->y), down(down), left(left)
{ }

//constructor for a WEAK LCM, requires pointer to the xi support at the LCM
LCM::LCM(xiMatrixEntry* point) : x_coord(point->x), y_coord(point->y), down(point), left(NULL)
{ }


///TODO: these other constructors might be unnecessary now
//LCM::LCM(unsigned x, unsigned y) : x_coord(x), y_coord(y), curve(NULL)
//{ }

//LCM::LCM(unsigned x, unsigned y, Halfedge* e) : x_coord(x), y_coord(y), curve(e)
//{ }

LCM::LCM(const LCM& other)
{
    x_coord = other.x_coord;
    y_coord = other.y_coord;
    down = other.down;
    left = other.left;
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

bool LCM::comparable(LCM *other) const   //tests whether two LCMs are (strongly) comparable
{
    return ( y_coord > other->get_y() && x_coord > other->get_x() ) || ( y_coord < other->get_y() && x_coord < other->get_x() );
}


unsigned LCM::get_x() const
{
    return x_coord;
}

unsigned LCM::get_y() const
{
    return y_coord;
}

void LCM::set_line(Halfedge* e)
{
    curve = e;
}

Halfedge* LCM::get_line() const
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

xiMatrixEntry* LCM::get_down()
{
    return down;
}

xiMatrixEntry* LCM::get_left()
{
    return left;
}

