/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "dcel.h"

#include "anchor.h"
#include <ostream>

#include "debug.h"

/*** implementation of class Vertex **/

Vertex::Vertex(double x_coord, double y_coord)
    : incident_edge(nullptr)
    , x(x_coord)
    , y(y_coord)
{
}

Vertex::Vertex()
    : incident_edge(nullptr)
    , x(0)
    , y(0)
{
}

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
    auto raw = reinterpret_cast<uintptr_t>(&*(v.incident_edge));
    qd << "(" << v.x << ", " << v.y << ", " << raw << ")";
    return qd;
}

bool Vertex::operator==(Vertex const& other)
{
    return x == other.x && y == other.y;
}

/*** implementation of class Halfedge ***/

Halfedge::Halfedge(std::shared_ptr<Vertex> v, std::shared_ptr<Anchor> p)
    : origin(v)
    , twin(nullptr)
    , next(nullptr)
    , prev(nullptr)
    , face(nullptr)
    , anchor(p)
{
}

Halfedge::Halfedge()
    : origin(nullptr)
    , twin(nullptr)
    , next(nullptr)
    , prev(nullptr)
    , face(nullptr)
    , anchor(nullptr)
{
}

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

void Halfedge::set_anchor(std::shared_ptr<Anchor> anchor)
{
    this->anchor = anchor;
}

std::shared_ptr<Anchor> Halfedge::get_anchor() const
{
    return anchor;
}

Debug& operator<<(Debug& qd, const Halfedge& e)
{
    std::shared_ptr<Halfedge> t = e.twin;
    qd << *(e.origin) << "--" << *(t->origin) << "; ";
    if (e.anchor == nullptr)
        qd << "Anchor null; ";
    else
        qd << "Anchor coords (" << e.anchor->get_x() << ", " << e.anchor->get_y() << "); ";
    return qd;
}

/*** implementation of class Face ***/

Face::Face(std::shared_ptr<Halfedge> e, unsigned long id)
    : boundary(e)
    , visited(false)
    , identifier(id)
{
}

Face::Face()
    : boundary()
    , visited(false)
    , identifier(-1)
{
}

Face::~Face()
{
}

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

void Face::set_barcode(const BarcodeTemplate& bt)
{
    dbc = bt; ///TODO: is this good design???
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
    do {
        qd << *(curr->get_origin()) << "--";
        curr = curr->get_next();
    } while (curr != start);
    qd << "cycle; ";
    return qd;
}

unsigned long Face::id() const
{
    return this->identifier;
}

bool operator==(TemplatePointsMessage const& left, TemplatePointsMessage const& right)
{
    bool support = left.template_points == right.template_points;
    bool hom = left.homology_dimensions == right.homology_dimensions;
    bool x = left.x_exact == right.x_exact;
    bool y = left.y_exact == right.y_exact;
    return support & hom & x & y;
}
