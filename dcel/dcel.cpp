#include "dcel.h"

#include "anchor.h"

#include <QDebug>


/*** implementation of class Vertex **/

Vertex::Vertex(double x_coord, double y_coord) :
    incident_edge(NULL),
    x(x_coord),
    y(y_coord)
{ }

void Vertex::set_incident_edge(Halfedge* edge)
{
    incident_edge = edge;
}

Halfedge* Vertex::get_incident_edge()
{
    return incident_edge;
}

double Vertex::get_x()
{
    return x;
}

double Vertex::get_y()
{
    return y;
}

QDebug& operator<<(QDebug& qd, const Vertex& v)
{
    qd.nospace() << "(" << v.x << ", " << v.y << ")";
    return qd.space();
}


/*** implementation of class Halfedge ***/

Halfedge::Halfedge(Vertex* v, Anchor* p) :
    origin(v),
    twin(NULL),
    next(NULL),
    prev(NULL),
    face(NULL),
    anchor(p)
{ }

Halfedge::Halfedge() :
    origin(NULL),
    twin(NULL),
    next(NULL),
    prev(NULL),
    face(NULL),
    anchor(NULL)
{ }

void Halfedge::set_twin(Halfedge* e)
{
    twin = e;
}

Halfedge* Halfedge::get_twin() const
{
    return twin;
}

void Halfedge::set_next(Halfedge* e)
{
    next = e;
}

Halfedge* Halfedge::get_next() const
{
    return next;
}

void Halfedge::set_prev(Halfedge* e)
{
    prev = e;
}

Halfedge* Halfedge::get_prev() const
{
    return prev;
}

void Halfedge::set_origin(Vertex* v)
{
    origin = v;
}

Vertex* Halfedge::get_origin() const
{
    return origin;
}

void Halfedge::set_face(Face* f)
{
    face = f;
}

Face* Halfedge::get_face() const
{
    return face;
}

Anchor* Halfedge::get_anchor() const
{
    return anchor;
}

QDebug& operator<<(QDebug& qd, const Halfedge& e)
{
    Halfedge* t = e.twin;
    qd.nospace() << *(e.origin) << "--" << *(t->origin) << "; ";
    if(e.anchor == NULL)
        qd.nospace() << "Anchor null; ";
    else
        qd.nospace() << "Anchor coords (" << e.anchor->get_x() << ", " << e.anchor->get_y() << "); ";
    return qd;
}



/*** implementation of class Face ***/

Face::Face(Halfedge* e) : boundary(e), visited(false)
{ }

Face::~Face()
{ }

void Face::set_boundary(Halfedge* e)
{
    boundary = e;
}

Halfedge* Face::get_boundary()
{
    return boundary;
}

BarcodeTemplate& Face::get_barcode()
{
    return dbc;
}

bool Face::has_been_visited()
{
    return visited;
}

void Face::mark_as_visited()
{
    visited = true;
}

QDebug& operator<<(QDebug& qd, const Face& f)
{
    Halfedge* start = f.boundary;
    Halfedge* curr = start;
    do{
        qd.nospace() << *(curr->get_origin()) << "--";
        curr = curr->get_next();
    }while(curr != start);
    qd.nospace() << "cycle; ";
    return qd;
}

