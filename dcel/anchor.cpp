#include "anchor.h"

#include "../math/xi_support_matrix.h"

#include <cstddef>  //for NULL

Anchor::Anchor(xiMatrixEntry* e) :
    x_coord(e->x), y_coord(e->y), entry(e), above_line(true)
{ }

Anchor::Anchor(unsigned x, unsigned y) :
    x_coord(x), y_coord(y), entry(NULL), dual_line(NULL)
{ }

Anchor::Anchor(const Anchor& other)
{
    x_coord = other.x_coord;
    y_coord = other.y_coord;
    entry = other.entry;
    dual_line = other.dual_line;
    position = other.position;
    above_line = other.above_line;
}

Anchor& Anchor::operator= (const Anchor& other)
{
    //check for self-assignment
    if(this == &other)
        return *this;

    //do the copy
    x_coord = other.x_coord;
    y_coord = other.y_coord;
    dual_line = other.dual_line;
    position = other.position;
    above_line = other.above_line;

    return *this;
}

bool Anchor::operator== (const Anchor& other) const
{
    return (x_coord == other.x_coord && y_coord == other.y_coord);
}

bool Anchor::comparable(Anchor *other) const   //tests whether two Anchors are (strongly) comparable
{
    return ( y_coord > other->get_y() && x_coord > other->get_x() ) || ( y_coord < other->get_y() && x_coord < other->get_x() );
}

unsigned Anchor::get_x() const
{
    return x_coord;
}

unsigned Anchor::get_y() const
{
    return y_coord;
}

void Anchor::set_line(Halfedge* e)
{
    dual_line = e;
}

Halfedge* Anchor::get_line() const
{
    return dual_line;
}

void Anchor::set_position(unsigned p)
{
    position = p;
}

unsigned Anchor::get_position() const
{
    return position;
}

bool Anchor::is_above()
{
    return above_line;
}

void Anchor::toggle()
{
    above_line = !above_line;
}

xiMatrixEntry* Anchor::get_entry()
{
    return entry;
}

void Anchor::set_weight(unsigned long w)
{
    weight = w;
}

unsigned long Anchor::get_weight()
{
    return weight;
}
