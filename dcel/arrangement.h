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
 * \class	Arrangement
 * \brief	Stores and manipulates the DCEL decomposition of the affine Grassmannian.
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_Mesh_H__
#define __DCEL_Mesh_H__

//forward declarations
class BarcodeTemplate;
class ComputationThread;
class Face;
class Halfedge;
class MultiBetti;
class PersistenceUpdater;
class Vertex;

#include "anchor.h"
#include "interface/progress.h"
#include "math/template_point.h"
#include "numerics.h"
#include "pointer_comparator.h"
#include <set>
#include <vector>

class ArrangementMessage;


class Arrangement {
    //TODO: refactor so Arrangement doesn't need friends.
    friend class PersistenceUpdater;
    friend class ArrangementBuilder;
    friend class ArrangementMessage;
    friend Arrangement to_arrangement(ArrangementMessage const& msg);

public:
    Arrangement(); //For serialization

    //constructor; sets up bounding box (with empty interior) for the affine Grassmannian
    //  requires references to vectors of all multi-grade values (both double and exact values)
    Arrangement(std::vector<exact> xe, std::vector<exact> ye, unsigned verbosity);

    ~Arrangement();

    //returns barcode template associated with the specified line (point)
    BarcodeTemplate& get_barcode_template(double degrees, double offset);

    //returns the barcode template associated with faces[i]
    BarcodeTemplate& get_barcode_template(unsigned i);

    //returns the number of 2-cells, and thus the number of barcode templates, in the arrangement
    unsigned num_faces();

    //creates a new anchor in the vector all_anchors
    void add_anchor(Anchor anchor);

    //FUNCTIONS FOR TESTING
    void print_stats(); //prints a summary of the arrangement information, such as the number of anchors, vertices, halfedges, and faces
    void print(); //prints all the data from the arrangement
    void test_consistency(); //attempts to find inconsistencies in the DCEL arrangement

    //references to vectors of multi-grade values
    std::vector<exact> x_exact; //exact values for all x-grades
    std::vector<exact> y_exact; //exact values for all y-grades

    //these are necessary for comparisons; TODO: should they be static members of Arrangement?
    static double epsilon;
    static bool almost_equal(const double a, const double b);

    friend std::ostream& operator<<(std::ostream&, const Arrangement&);
    friend std::istream& operator>>(std::istream&, Arrangement&);
    Halfedge* insert_vertex(Halfedge* edge, double x, double y); //inserts a new vertex on the specified edge, with the specified coordinates, and updates all relevant pointers

private:
    //data structures
    std::vector<double> x_grades; //floating-point values for x-grades
    std::vector<double> y_grades; //floating-point values for y-grades
    std::vector<Vertex*> vertices; //all vertices in the arrangement
    std::vector<Halfedge*> halfedges; //all halfedges in the arrangement
    std::vector<Face*> faces; //all faces in the arrangement

    unsigned verbosity;

    //set of Anchors that are represented in the arrangement, ordered by position of curve along left side of the arrangement, from bottom to top
    std::set<Anchor*, PointerComparator<Anchor, Anchor_LeftComparator>> all_anchors;

    Halfedge* topleft; //pointer to Halfedge that points down from top left corner (0,infty)
    Halfedge* topright; //pointer to Halfedge that points down from the top right corner (infty,infty)
    Halfedge* bottomleft; //pointer to Halfedge that points up from bottom left corner (0,-infty)
    Halfedge* bottomright; //pointer to Halfedge that points up from bottom right corner (infty,-infty)

    //stores a pointer to the rightmost Halfedge of the "top" line of each unique slope, ordered from small slopes to big slopes (each Halfedge points to Anchor and Face for vertical-line queries)
    std::vector<Halfedge*> vertical_line_query_list;

    ///// functions for creating the arrangement /////

    //creates the first pair of Halfedges in an anchor line, anchored on the left edge of the strip
    Halfedge* create_edge_left(Halfedge* edge, Anchor* anchor);

    //computes and stores the edge weight for each anchor line
    void find_edge_weights(PersistenceUpdater& updater);

    //finds a pseudo-optimal path through all 2-cells of the arrangement
    void find_path(std::vector<Halfedge*>& pathvec);

    //builds the path recursively
    void find_subpath(unsigned cur_node, std::vector<std::vector<unsigned>>& adj, std::vector<Halfedge*>& pathvec, bool return_path);

    //stores (a copy of) the given barcode template in faces[i]; used for re-building the arrangement from a RIVET data file
    void set_barcode_template(unsigned i, BarcodeTemplate& bt);

    ///// functions for searching the arrangement /////

    //finds the first anchor that intersects the left edge of the arrangement at a point not less than the specified y-coordinate; if no such anchor, returns NULL
    Anchor* find_least_upper_anchor(double y_coord);

    //finds the (unbounded) cell associated to dual point of the vertical line with the given x-coordinate
    //  i.e. finds the Halfedge whose anchor x-coordinate is the largest such coordinate not larger than than x_coord; returns the Face corresponding to that Halfedge
    Face* find_vertical_line(double x_coord);

    //finds a 2-cell containing the specified point
    Face* find_point(double x_coord, double y_coord);

    ///// functions for testing /////

    long HID(Halfedge* h) const; //halfedge ID, for printing and debugging
    long FID(Face* f) const; //face ID, for printing and debugging
    long AID(Anchor* a) const; //anchor ID, for printing and debugging
    long VID(Vertex* v) const; //vertex ID, for printing and debugging

    void announce_next_point(Halfedge* finder, Vertex* next_pt);

    //struct to hold a future intersection event -- used when building the arrangement
    struct Crossing {
        Anchor* a; //pointer to one line
        Anchor* b; //pointer to the other line -- must ensure that line for anchor a is below line for anchor b just before the crossing point!!!!!
        double x; //x-coordinate of intersection point (floating-point)
        Arrangement* m; //pointer to the arrangement, so the Crossing has access to the vectors x_grades, x_exact, y_grades, and y_exact

        Crossing(Anchor* a, Anchor* b, Arrangement* m); //precondition: Anchors a and b must be comparable
        bool x_equal(const Crossing* other) const; //returns true iff this Crossing has (exactly) the same x-coordinate as other Crossing
    };

    //comparator class for ordering crossings: first by x (left to right); for a given x, then by y (low to high)
    struct CrossingComparator {
        bool operator()(const Crossing& c1, const Crossing& c2) const; //returns true if c1 comes after c2
    };

}; //end class Arrangement

#endif // __DCEL_Mesh_H__
