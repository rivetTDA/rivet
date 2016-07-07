#include "dcel.h"

#include "anchor.h"

#include "debug.h"


/*** implementation of class Vertex **/

Vertex::Vertex(double x_coord, double y_coord) :
    incident_edge(NULL),
    x(x_coord),
    y(y_coord)
{ }

Vertex::Vertex() { }

void Vertex::set_incident_edge(std::shared_ptr<Halfedge> edge)
{
    incident_edge = edge;
}

std::shared_ptr<Halfedge> Vertex::get_incident_edge()
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

Debug& operator<<(Debug& qd, const Vertex& v)
{
    qd << "(" << v.x << ", " << v.y << ")";
    return qd;
}


/*** implementation of class Halfedge ***/

Halfedge::Halfedge(std::shared_ptr<Vertex> v, std::shared_ptr<Anchor> p) :
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

void Halfedge::set_twin(std::shared_ptr<Halfedge> e)
{
    twin = e;
}

std::shared_ptr<Halfedge> Halfedge::get_twin() const
{
    return twin;
}

void Halfedge::set_next(std::shared_ptr<Halfedge> e)
{
    next = e;
}

std::shared_ptr<Halfedge> Halfedge::get_next() const
{
    return next;
}

void Halfedge::set_prev(std::shared_ptr<Halfedge> e)
{
    prev = e;
}

std::shared_ptr<Halfedge> Halfedge::get_prev() const
{
    return prev;
}

void Halfedge::set_origin(std::shared_ptr<Vertex> v)
{
    origin = v;
}

std::shared_ptr<Vertex> Halfedge::get_origin() const
{
    return origin;
}

void Halfedge::set_face(std::shared_ptr<Face> f)
{
    face = f;
}

std::shared_ptr<Face> Halfedge::get_face() const
{
    return face;
}

std::shared_ptr<Anchor> Halfedge::get_anchor() const
{
    return anchor;
}

Debug& operator<<(Debug& qd, const Halfedge& e)
{
    std::shared_ptr<Halfedge> t = e.twin;
    qd << *(e.origin) << "--" << *(t->origin) << "; ";
    if(e.anchor == NULL)
        qd << "Anchor null; ";
    else
        qd << "Anchor coords (" << e.anchor->get_x() << ", " << e.anchor->get_y() << "); ";
    return qd;
}



/*** implementation of class Face ***/

Face::Face(std::shared_ptr<Halfedge> e) : boundary(e), visited(false)
{ }

Face::Face(): boundary(), visited(false) {}

Face::~Face()
{ }

void Face::set_boundary(std::shared_ptr<Halfedge> e)
{
    boundary = e;
}

std::shared_ptr<Halfedge> Face::get_boundary()
{
    return boundary;
}

BarcodeTemplate& Face::get_barcode()
{
    return dbc;
}

void Face::set_barcode(BarcodeTemplate& bt)
{
    dbc = bt;   ///TODO: is this good design???
}

bool Face::has_been_visited()
{
    return visited;
}

void Face::mark_as_visited()
{
    visited = true;
}

Debug& operator<<(Debug& qd, const Face& f)
{
    std::shared_ptr<Halfedge> start = f.boundary;
    std::shared_ptr<Halfedge> curr = start;
    do{
        qd << *(curr->get_origin()) << "--";
        curr = curr->get_next();
    }while(curr != start);
    qd << "cycle; ";
    return qd;
}

