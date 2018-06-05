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
/**
 * \brief	Vertex, Halfedge, and Face classes for building a DCEL arrangement
 *
 * A DCEL is a doubly connected edge list, see e.g. de Berg et. al.'s Computational
 * Geometry: Algorithms and Applications for background.
 * 
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_H__
#define __DCEL_H__

class Halfedge;

#include "barcode_template.h"
#include <math/template_point.h>
#include <memory>
#include <numerics.h>
#include <vector>

#include "debug.h"

class Vertex {
public:
    Vertex(double x_coord, double y_coord); //constructor, sets (x, y)-coordinates of the vertex
    Vertex(); //For serialization

    void set_incident_edge(Halfedge* edge); //set the incident edge
    Halfedge* get_incident_edge(); //get the incident edge

    double get_x(); //get the x-coordinate
    double get_y(); //get the y-coordinate

    friend Debug& operator<<(Debug& qd, const Vertex& v); //for printing the vertex

    bool operator==(Vertex const& other);

private:
    Halfedge* incident_edge; //pointer to one edge incident to this vertex
    double x; //x-coordinate of this vertex
    double y; //y-coordinate of this vertex

}; //end class Vertex

class Face;
class Anchor;

class Halfedge {
public:
    Halfedge(Vertex* v, Anchor* p); //constructor, requires origin vertex as well as Anchor corresponding to this halfedge (Anchor never changes)
    Halfedge(); //constructor for a null Halfedge

    void set_twin(Halfedge* e); //set the twin halfedge
    Halfedge* get_twin() const; //get the twin halfedge

    void set_next(Halfedge* e); //set the next halfedge in the boundary of the face that this halfedge borders
    Halfedge* get_next() const; //get the next halfedge

    void set_prev(Halfedge* e); //set the previous halfedge in the boundary of the face that this halfedge borders
    Halfedge* get_prev() const; //get the previous halfedge

    void set_origin(Vertex* v); //set the origin vertex
    Vertex* get_origin() const; //get the origin vertex

    void set_face(Face* f); //set the face that this halfedge borders
    Face* get_face() const; //get the face that this halfedge borders

    void set_anchor(Anchor*); //set the Anchor
    Anchor* get_anchor() const; //get the Anchor

    friend Debug& operator<<(Debug& qd, const Halfedge& e); //for printing the halfedge

private:
    Vertex* origin; //pointer to the vertex from which this halfedge originates
    Halfedge* twin; //pointer to the halfedge that, together with this halfedge, make one edge
    Halfedge* next; //pointer to the next halfedge around the boundary of the face to the right of this halfedge
    Halfedge* prev; //pointer to the previous halfedge around the boundary of the face to the right of this halfedge
    Face* face; //pointer to the face to the right of this halfedge
    Anchor* anchor; //stores the coordinates of the anchor corresponding to this halfedge

}; //end class Halfedge

class Face {
public:
    Face(Halfedge* e, unsigned long id); //constructor: requires pointer to a boundary halfedge
    Face(); // For serialization
    ~Face(); //destructor: destroys barcode template

    void set_boundary(Halfedge* e); //set the pointer to a halfedge on the boundary of this face
    Halfedge* get_boundary(); //get the (pointer to the) boundary halfedge

    BarcodeTemplate& get_barcode(); //returns a reference to the barcode template stored in this cell
    void set_barcode(const BarcodeTemplate& bt); //stores (a copy of) the specified barcode template in this cell

    bool has_been_visited(); //true iff cell has been visited in the vineyard-update process (so that we can distinguish a cell with an empty barcode from an unvisited cell)
    void mark_as_visited(); //marks this cell as visited

    friend Debug& operator<<(Debug& os, const Face& f); //for printing the face

    unsigned long id() const;

private:
    Halfedge* boundary; //pointer to one halfedge in the boundary of this cell
    BarcodeTemplate dbc; //barcode template stored in this cell
    bool visited; //initially false, set to true after this cell has been visited in the vineyard-update process (so that we can distinguish a cell with an empty barcode from an unvisited cell)
    unsigned long identifier; // Arrangement-specific ID for this face
}; //end class Face

//This class exists only for data transfer between console and viewer
struct TemplatePointsMessage {
    std::string x_label;
    std::string y_label;
    std::vector<TemplatePoint> template_points;
    unsigned_matrix homology_dimensions;
    std::vector<exact> x_exact;
    std::vector<exact> y_exact;
    bool x_reverse;
    bool y_reverse;

    friend bool operator==(TemplatePointsMessage const& left, TemplatePointsMessage const& right);

    MSGPACK_DEFINE(x_label, y_label, template_points, homology_dimensions, x_exact, y_exact, x_reverse, y_reverse);
};

#endif // __DCEL_H__
