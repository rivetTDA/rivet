/* implementation of Mesh class
 * Stores and manipulates the DCEL decomposition of the affine Grassmannian.
 */

#include "mesh.h"

#include "dcel.h"
#include "../dcel/barcode_template.h"
#include "../math/multi_betti.h"            //this include might not be necessary
#include "../math/persistence_updater.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
//#include <boost/graph/metric_tsp_approx.hpp> -- NOT CURRENTLY USED

#include <limits>	//necessary for infinity
#include <queue>    //for std::priority_queue
#include <chrono>
#include <interface/progress.h>
#include <math/simplex_tree.h>
#include <timer.h>
#include <sstream>

// Mesh constructor; sets up bounding box (with empty interior) for the affine Grassmannian
Mesh::Mesh(std::vector<exact> xe,
           std::vector<exact> ye,
unsigned verbosity):
    x_exact(xe), y_exact(ye), x_grades(xe.size()), y_grades(ye.size()),
    INFTY(std::numeric_limits<double>::infinity()), verbosity(verbosity), halfedges(), vertices(), faces()
{
    std::transform(x_exact.begin(), x_exact.end(), x_grades.begin(), [](exact num) {
        return numerator(num).convert_to<double>() / denominator(num).convert_to<double>();
    });
    std::transform(y_exact.begin(), y_exact.end(), y_grades.begin(), [](exact num) {
        return numerator(num).convert_to<double>() / denominator(num).convert_to<double>();
    });
    //create vertices
    vertices.push_back( new Vertex(0, INFTY) );         //index 0
    vertices.push_back( new Vertex(INFTY, INFTY) );     //index 1
    vertices.push_back( new Vertex(INFTY, -INFTY) );    //index 2
    vertices.push_back( new Vertex(0, -INFTY) );        //index 3

    //create halfedges
    for(int i=0; i<4; i++)
    {
        halfedges.push_back( new Halfedge( vertices[i], NULL) );		//index 0, 2, 4, 6 (inside halfedges)
        halfedges.push_back( new Halfedge( vertices[(i+1)%4], NULL) );		//index 1, 3, 5, 7 (outside halfedges)
        halfedges[2*i]->set_twin( halfedges[2*i+1] );
        halfedges[2*i+1]->set_twin( halfedges[2*i] );
    }

    topleft = halfedges[7];     //remember this halfedge to make curve insertion easier
    topright = halfedges[2];    //remember this halfedge for starting the path that we use to find edge weights
    bottomleft = halfedges[6];  //remember these halfedges
    bottomright = halfedges[3]; //    for the Bentley-Ottmann algorithm

    //set pointers on vertices
    for(int i=0; i<4; i++)
    {
        vertices[i]->set_incident_edge( halfedges[2*i] );
    }

    //create face
    faces.push_back( new Face( halfedges[0] ) );

    //set the remaining pointers on the halfedges
    for(int i=0; i<4; i++)
    {
        Halfedge* inside = halfedges[2*i];
        inside->set_next( halfedges[(2*i+2)%8] );
        inside->set_prev( halfedges[(2*i+6)%8] );
        inside->set_face( faces[0] );

        Halfedge* outside = halfedges[2*i+1];
        outside->set_next( halfedges[(2*i+7)%8] );
        outside->set_prev( halfedges[(2*i+3)%8] );
    }
}//end constructor

//destructor
Mesh::~Mesh()
{
    for(std::vector<Vertex*>::iterator it = vertices.begin(); it != vertices.end(); ++it)
        delete (*it);

    for(std::vector<Halfedge*>::iterator it = halfedges.begin(); it != halfedges.end(); ++it)
        delete (*it);

    for(std::vector<Face*>::iterator it = faces.begin(); it != faces.end(); ++it)
        delete (*it);

    for(std::set<Anchor*>::iterator it = all_anchors.begin(); it != all_anchors.end(); ++it)
        delete (*it);
}//end destructor



//inserts a new vertex on the specified edge, with the specified coordinates, and updates all relevant pointers
//  i.e. new vertex is between initial and termainal points of the specified edge
//returns pointer to a new halfedge, whose initial point is the new vertex, and that follows the specified edge around its face
Halfedge* Mesh::insert_vertex(Halfedge* edge, double x, double y)
{
	//create new vertex
    Vertex* new_vertex = new Vertex(x, y);
	vertices.push_back(new_vertex);
	
    //get twin and Anchor of this edge
	Halfedge* twin = edge->get_twin();
    Anchor* anchor = edge->get_anchor();
	
	//create new halfedges
    Halfedge* up = new Halfedge(new_vertex, anchor);
	halfedges.push_back(up);
    Halfedge* dn = new Halfedge(new_vertex, anchor);
	halfedges.push_back(dn);
		
	//update pointers
	up->set_next(edge->get_next());
	up->set_prev(edge);
	up->set_twin(twin);
    up->set_face(edge->get_face());
	
	up->get_next()->set_prev(up);
	
	edge->set_next(up);
	edge->set_twin(dn);
	
	dn->set_next(twin->get_next());
	dn->set_prev(twin);
	dn->set_twin(edge);
    dn->set_face(twin->get_face());

	dn->get_next()->set_prev(dn);
	
	twin->set_next(dn);
	twin->set_twin(up);
	
	new_vertex->set_incident_edge(up);
	
	//return pointer to up
	return up;
}//end insert_vertex()

//creates the first pair of Halfedges in an Anchor line, anchored on the left edge of the strip at origin of specified edge
//  also creates a new face (the face below the new edge)
//  CAUTION: leaves NULL: new_edge.next and new_twin.prev
Halfedge* Mesh::create_edge_left(Halfedge* edge, Anchor* anchor)
{
    //create new halfedges
    Halfedge* new_edge = new Halfedge(edge->get_origin(), anchor); //points AWAY FROM left edge
    halfedges.push_back(new_edge);
    Halfedge* new_twin = new Halfedge(NULL, anchor);   //points TOWARDS left edge
    halfedges.push_back(new_twin);

    //create new face
    Face* new_face = new Face(new_edge);
    faces.push_back(new_face);

    //update Halfedge pointers
    new_edge->set_prev(edge->get_prev());
    new_edge->set_twin(new_twin);
    new_edge->set_face(new_face);

    edge->get_prev()->set_next(new_edge);
    edge->get_prev()->set_face(new_face);
    if(edge->get_prev()->get_prev() != NULL)
        edge->get_prev()->get_prev()->set_face(new_face);

    new_twin->set_next(edge);
    new_twin->set_twin(new_edge);
    new_twin->set_face(edge->get_face());

    edge->set_prev(new_twin);

    //return pointer to new_edge
    return new_edge;
}//end create_edge_left()



//returns barcode template associated with the specified line (point)
//REQUIREMENT: 0 <= degrees <= 90
BarcodeTemplate& Mesh::get_barcode_template(double degrees, double offset)
{
    if(degrees == 90) //then line is vertical
    {
        Face* cell = find_vertical_line(-1*offset); //multiply by -1 to correct for orientation of offset

        ///TODO: store some point/cell to seed the next query

        if(verbosity >= 3) { debug() << " ||| vertical line found in cell " << FID(cell); }
        return cell->get_barcode();
    }

    if(degrees == 0) //then line is horizontal
    {
        Face* cell = topleft->get_twin()->get_face();    //default
        Anchor* anchor = find_least_upper_anchor(offset);

        if(anchor != NULL)
            cell = anchor->get_line()->get_face();

        ///TODO: store some point/cell to seed the next query

        if(verbosity >= 3) { debug() << " --- horizontal line found in cell " << FID(cell); }
        return cell->get_barcode();
    }

    //else: the line is neither horizontal nor vertical
    double radians = degrees * 3.14159265/180;
    double slope = tan(radians);
    double intercept = offset/cos(radians);

//    debug(true) << "  Line (deg, off) = (" << degrees << ", " << offset << ") transformed to (slope, int) = (" << slope << ", " << intercept << ")";

    Face* cell = find_point(slope, -1*intercept);   //multiply by -1 for point-line duality
        ///TODO: REPLACE THIS WITH A SEEDED SEARCH

    ///TODO: store seed

    return cell->get_barcode();  ////FIX THIS!!!
}//end get_barcode_template()

//returns the barcode template associated with faces[i]
BarcodeTemplate& Mesh::get_barcode_template(unsigned i)
{
    return faces[i]->get_barcode();
}

//stores (a copy of) the given barcode template in faces[i]
void Mesh::set_barcode_template(unsigned i, BarcodeTemplate& bt)
{
    faces[i]->set_barcode(bt);
}

//returns the number of 2-cells, and thus the number of barcode templates, in the arrangement
unsigned Mesh::num_faces()
{
    return faces.size();
}

//creates a new anchor in the vector all_anchors
void Mesh::add_anchor(Anchor anchor)
{
    //TODO: not clear why all_anchors should contain anchor pointers and not just anchors.
    all_anchors.insert(new Anchor(anchor.get_entry()));
}

//finds the first anchor that intersects the left edge of the arrangement at a point not less than the specified y-coordinate
//  if no such anchor, returns NULL
Anchor* Mesh::find_least_upper_anchor(double y_coord)
{
    //binary search to find greatest y-grade not greater than than y_coord
    unsigned best = 0;
    if(y_grades.size() >= 1 && y_grades[0] <= y_coord)
    {
        //binary search the vector y_grades
        unsigned min = 0;
        unsigned max = y_grades.size() - 1;

        while(max >= min)
        {
            unsigned mid = (max + min)/2;

            if(y_grades[mid] <= y_coord)    //found a lower bound, but search upper subarray for a better lower bound
            {
                best = mid;
                min = mid + 1;
            }
            else    //search lower subarray
                max = mid - 1;
        }
    }
    else
        return NULL;

    //if we get here, then y_grades[best] is the greatest y-grade not greater than y_coord
    //now find Anchor whose line intersects the left edge of the arrangement lowest, but not below y_grade[best]
    unsigned int zero = 0;  //disambiguate the following function call
    Anchor* test = new Anchor(zero, best);
    std::set<Anchor*, Anchor_LeftComparator>::iterator it = all_anchors.lower_bound(test);
    delete test;

    if(it == all_anchors.end())    //not found
    {
        return NULL;
    }
    //else
    return *it;
}//end find_least_upper_anchor()

//finds the (unbounded) cell associated to dual point of the vertical line with the given x-coordinate
//  i.e. finds the Halfedge whose Anchor x-coordinate is the largest such coordinate not larger than than x_coord; returns the Face corresponding to that Halfedge
Face* Mesh::find_vertical_line(double x_coord)
{
    //is there an Anchor with x-coordinate not greater than x_coord?
    if(vertical_line_query_list.size() >= 1 && x_grades[ vertical_line_query_list[0]->get_anchor()->get_x() ] <= x_coord)
    {
        //binary search the vertical line query list
        unsigned min = 0;
        unsigned max = vertical_line_query_list.size() - 1;
        unsigned best = 0;

        while(max >= min)
        {
            unsigned mid = (max + min)/2;
            Anchor* test = vertical_line_query_list[mid]->get_anchor();

            if(x_grades[test->get_x()] <= x_coord)    //found a lower bound, but search upper subarray for a better lower bound
            {
                best = mid;
                min = mid + 1;
            }
            else    //search lower subarray
                max = mid - 1;
        }

        //testing
        if(verbosity >= 6) { debug() << "----vertical line search: found anchor with x-coordinate " << vertical_line_query_list[best]->get_anchor()->get_x(); }

        return vertical_line_query_list[best]->get_face();
    }

    //if we get here, then either there are no Anchors or x_coord is less than the x-coordinates of all Anchors
    if(verbosity >= 6) { debug() << "----vertical line search: returning lowest face"; }
    return bottomright->get_twin()->get_face();

}//end find_vertical_line()

//find a 2-cell containing the specified point
Face* Mesh::find_point(double x_coord, double y_coord)
{
    //start on the left edge of the arrangement, at the correct y-coordinate
    Anchor* start = find_least_upper_anchor(-1*y_coord);

    Face* cell = NULL;		//will later point to the cell containing the specified point
    Halfedge* finger = NULL;	//for use in finding the cell

    if(start == NULL)	//then starting point is in the top (unbounded) cell
    {
        finger = topleft->get_twin()->get_next();   //this is the top edge of the top cell (at y=infty)
        if(verbosity >= 8) { debug() << "  Starting in top (unbounded) cell"; }
    }
    else
    {
        finger = start->get_line();
        if(verbosity >= 8) { debug() << "  Reference Anchor: (" << x_grades[start->get_x()] << "," << y_grades[start->get_y()] << "); halfedge" << HID(finger); }
    }

    while(cell == NULL) //while not found
    {
        if(verbosity >= 8) { debug() << "  Considering cell " << FID(finger->get_face()); }

        //find the edge of the current cell that crosses the horizontal line at y_coord
        Vertex* next_pt = finger->get_next()->get_origin();

        if(verbosity >= 8)
        {
            if(finger->get_anchor() != NULL)
                debug() << "     -- next point: (" << next_pt->get_x() << "," << next_pt->get_y() << ") vertex ID" << VID(next_pt) << "; along line corresponding to anchor at (" << finger->get_anchor()->get_x() << "," << finger->get_anchor()->get_y() << ")";
            else
                debug() << "     -- next point: (" << next_pt->get_x() << "," << next_pt->get_y() << ") vertex ID" << VID(next_pt) << "; along line corresponding to NULL anchor";
        }

        while(next_pt->get_y() > y_coord)
        {
            finger = finger->get_next();
            next_pt = finger->get_next()->get_origin();

            if(verbosity >= 8)
            {
                if(finger->get_anchor() != NULL)
                    debug() << "     -- next point: (" << next_pt->get_x() << "," << next_pt->get_y() << ") vertex ID" << VID(next_pt) << "; along line corresponding to anchor at (" << finger->get_anchor()->get_x() << "," << finger->get_anchor()->get_y() << ")";
                else
                    debug() << "     -- next point: (" << next_pt->get_x() << "," << next_pt->get_y() << ") vertex ID" << VID(next_pt) << "; along line corresponding to NULL anchor";
            }
        }

        //now next_pt is at or below the horizontal line at y_coord
        //if (x_coord, y_coord) is to the left of crossing point, then we have found the cell; otherwise, move to the adjacent cell

        if(next_pt->get_y() == y_coord) //then next_pt is on the horizontal line
        {
            if(next_pt->get_x() >= x_coord)	//found the cell
            {
                cell = finger->get_face();
            }
            else	//move to adjacent cell
            {
                //find degree of vertex
                Halfedge* thumb = finger->get_next();
                int deg = 1;
                while(thumb != finger->get_twin())
                {
                    thumb = thumb->get_twin()->get_next();
                    deg++;
                }

                //move halfway around the vertex
                finger = finger->get_next();
                for(int i=0; i<deg/2; i++)
                {
                    finger = finger->get_twin()->get_next();
                }
            }
        }
        else	//then next_pt is below the horizontal line
        {
            if(finger->get_anchor() == NULL)	//then edge is vertical, so we have found the cell
            {
                cell = finger->get_face();
            }
            else	//then edge is not vertical
            {
                Anchor* temp = finger->get_anchor();
                double x_pos = ( y_coord + y_grades[temp->get_y()] )/x_grades[temp->get_x()];  //NOTE: division by zero never occurs because we are searching along a horizontal line, and thus we never cross horizontal lines in the arrangement

                if(x_pos >= x_coord)	//found the cell
                {
                    cell = finger->get_face();
                }
                else	//move to adjacent cell
                {
                    finger = finger->get_twin();

                    if(verbosity >= 8) { debug(true) << "   --- crossing line dual to anchor (" << temp->get_x() << "," << temp->get_y() << ") at x = " << x_pos; }
                }
            }
        }//end else
    }//end while(cell not found)

    if(verbosity >= 3) { debug() << "  Found point (" << x_coord << "," << y_coord << ") in cell" << FID(cell); }

    return cell;
}//end find_point()


//prints a summary of the arrangement information, such as the number of anchors, vertices, halfedges, and faces
void Mesh::print_stats()
{
    debug() << "The arrangement contains: " << all_anchors.size() << " anchors, " << vertices.size() << " vertices, " << halfedges.size() << " halfedges, and " << faces.size() << " faces";
}

//print all the data from the mesh
void Mesh::print()
{
    debug() << "  Vertices";
    for(unsigned i=0; i<vertices.size(); i++)
	{
        debug() << "    vertex " << i << ": " << *vertices[i] << "; incident edge: " << HID(vertices[i]->get_incident_edge());
	}
	
    debug() << "  Halfedges";
    for(unsigned i=0; i<halfedges.size(); i++)
	{
		Halfedge* e = halfedges[i];
		Halfedge* t = e->get_twin();
        debug() << "    halfedge " << i << ": " << *(e->get_origin()) << "--" << *(t->get_origin()) << "; ";
        if(e->get_anchor() == NULL)
            debug() << "Anchor null; ";
		else
            debug() << "Anchor coords (" << e->get_anchor()->get_x() << ", " << e->get_anchor()->get_y() << "); ";
        debug() << "twin: " << HID(t) << "; next: " << HID(e->get_next()) << "; prev: " << HID(e->get_prev()) << "; face: " << FID(e->get_face());
	}
	
    debug() << "  Faces";
    for(unsigned i=0; i<faces.size(); i++)
	{
        debug() << "    face " << i << ": " << *faces[i];
	}
	
/*    debug() << "  Outside (unbounded) region: ";
	Halfedge* start = halfedges[1];
	Halfedge* curr = start;
	do{
        debug() << *(curr->get_origin()) << "--";
		curr = curr->get_next();
	}while(curr != start);
    debug() << "cycle";
*/
    debug() << "  Anchor set: ";
    std::set<Anchor*>::iterator it;
    for(it = all_anchors.begin(); it != all_anchors.end(); ++it)
	{
        Anchor cur = **it;
        debug() << "(" << cur.get_x() << ", " << cur.get_y() << ") halfedge " << HID(cur.get_line()) << "; ";
	}	
}//end print()

/********** functions for testing **********/

//look up halfedge ID, used in print() for debugging
// HID = halfedge ID
unsigned Mesh::HID(Halfedge* h)
{
    for(unsigned i=0; i<halfedges.size(); i++)
	{
		if(halfedges[i] == h)
			return i;
	}
	
	//we should never get here
	return -1;	
}

//look up face ID, used in print() for debugging
// FID = face ID
unsigned Mesh::FID(Face* f)
{
    for(unsigned i=0; i<faces.size(); i++)
	{
		if(faces[i] == f)
			return i;
	}
	
	//we should only get here if f is NULL (meaning the unbounded, outside face)
	return -1;	
}

//look up vertex ID, used in print() for debugging
// VID = vertex ID
unsigned Mesh::VID(Vertex* v)
{
    for(unsigned i=0; i<vertices.size(); i++)
    {
        if(vertices[i] == v)
            return i;
    }

    //we should only get here if f is NULL (meaning the unbounded, outside face)
    return -1;
}

//attempts to find inconsistencies in the DCEL arrangement
void Mesh::test_consistency()
{
    //check faces
    debug() << "Checking faces:";
    bool face_problem = false;
    std::set<int> edges_found_in_faces;

    for(std::vector<Face*>::iterator it = faces.begin(); it != faces.end(); ++it)
    {
        Face* face = *it;
        debug() << "  Checking face " << FID(face);

        if(face->get_boundary() == NULL)
        {
            debug() << "    PROBLEM: face" << FID(face) << "has null edge pointer.";
            face_problem = true;
        }
        else
        {
            Halfedge* start = face->get_boundary();
            edges_found_in_faces.insert(HID(start));

            if(start->get_face() != face)
            {
                debug() << "    PROBLEM: starting halfedge edge" << HID(start) << "of face" << FID(face) << "doesn't point back to face.";
                face_problem = true;
            }

            if(start->get_next() == NULL)
                debug() << "    PROBLEM: starting halfedge" << HID(start) << "of face" << FID(face) << "has NULL next pointer.";
            else
            {
                Halfedge* cur = start->get_next();
                int i = 0;
                while(cur != start)
                {
                    edges_found_in_faces.insert(HID(cur));

                    if(cur->get_face() != face)
                    {
                        debug() << "    PROBLEM: halfedge edge" << HID(cur) << "points to face" << FID(cur->get_face()) << "instead of face" << FID(face);
                        face_problem = true;
                        break;
                    }

                    if(cur->get_next() == NULL)
                    {
                        debug() << "    PROBLEM: halfedge" << HID(cur) << "has NULL next pointer.";
                        face_problem = true;
                        break;
                    }
                    else
                        cur = cur->get_next();

                    i++;
                    if(i >= 1000)
                    {
                        debug() << "    PROBLEM: halfedges of face" << FID(face) << "do not form a cycle (or, if they do, it has more than 1000 edges).";
                        face_problem = true;
                        break;
                    }
                }
            }

        }
    }//end face loop
    if(!face_problem)
        debug() << "   ---No problems detected among faces.";
    else
        debug() << "   ---Problems detected among faces.";

    //find exterior halfedges
    if (halfedges.size() < 2) {
        debug() << "Only " << halfedges.size() << "halfedges present!";
    }
    Halfedge* start = halfedges[1];
    Halfedge* cur = start;
    do{
        edges_found_in_faces.insert(HID(cur));

        if(cur->get_next() == NULL)
        {
            debug() << "    PROBLEM: halfedge " << HID(cur) << " has NULL next pointer.";
            break;
        }
        cur = cur->get_next();
    }while(cur != start);

    //check if all edges were found
    bool all_edges_found = true;
    for(unsigned i=0; i<halfedges.size(); i++)
    {
        if(edges_found_in_faces.find(i) == edges_found_in_faces.end())
        {
            debug() << "  PROBLEM: halfedge" << i << "not found in any face";
            all_edges_found = false;
        }
    }
    if(all_edges_found)
        debug() << "   ---All halfedges found in faces, as expected.";


    //check anchor lines
    debug() << "Checking anchor lines:\n";
    bool curve_problem = false;
    std::set<int> edges_found_in_curves;

    for(std::set<Anchor*>::iterator it = all_anchors.begin(); it != all_anchors.end(); ++it)
    {
        Anchor* anchor = *it;
        debug() << "  Checking line for anchor (" << anchor->get_x() <<"," << anchor->get_y() << ")";

        Halfedge* edge = anchor->get_line();
        do{
            edges_found_in_curves.insert(HID(edge));
            edges_found_in_curves.insert(HID(edge->get_twin()));

            if(edge->get_anchor() != anchor)
            {
                debug() << "    PROBLEM: halfedge" << HID(edge) << "does not point to this Anchor.";
                curve_problem = true;
            }
            if(edge->get_twin()->get_anchor() != anchor)
            {
                debug() << "    PROBLEM: halfedge" << HID(edge->get_twin()) << ", twin of halfedge " << HID(edge) << ", does not point to this anchor.";
                curve_problem = true;
            }

            if(edge->get_next() == NULL)
            {
                debug() << "    PROBLEM: halfedge" << HID(edge) << "has NULL next pointer.";
                curve_problem = true;
                break;
            }

            //find next edge in this line
            edge = edge->get_next();
            while(edge->get_anchor() != anchor)
                edge = edge->get_twin()->get_next();

        }while(edge->get_origin()->get_x() < INFTY);
    }//end anchor line loop

    //ignore halfedges on both sides of boundary
    start = halfedges[1];
    cur = start;
    do{
        edges_found_in_curves.insert(HID(cur));
        edges_found_in_curves.insert(HID(cur->get_twin()));

        if(cur->get_next() == NULL)
        {
            debug() << "    PROBLEM: halfedge" << HID(cur) << "has NULL next pointer.";
            break;
        }
        cur = cur->get_next();
    }while(cur != start);

    //check if all edges were found
    all_edges_found = true;
    for(unsigned i=0; i<halfedges.size(); i++)
    {
        if(edges_found_in_curves.find(i) == edges_found_in_curves.end())
        {
            debug() << "  PROBLEM: halfedge" << i << "not found in any anchor line";
            all_edges_found = false;
        }
    }
    if(all_edges_found)
        debug() << "   ---All halfedges found in curves, as expected.";


    if(!curve_problem)
        debug() << "   ---No problems detected among anchor lines.";
    else
        debug() << "   ---Problems detected among anchor lines.";


    //check anchor lines
    debug() << "Checking order of vertices along right edge of the strip:";
    Halfedge* redge = halfedges[3];
    while(redge != halfedges[1])
    {
        debug() << " y = " << redge->get_origin()->get_y() << "at vertex" << VID(redge->get_origin());
        redge = redge->get_next();
    }
}//end test_consistency()



/********** the following objects and functions are for exact comparisons **********/

//Crossing constructor
//precondition: Anchors a and b must be comparable
Mesh::Crossing::Crossing(Anchor* a, Anchor* b, Mesh* m) : a(a), b(b), m(m)
{
    //store the x-coordinate of the crossing for fast (inexact) comparisons
    x = (m->y_grades[a->get_y()] - m->y_grades[b->get_y()])/(m->x_grades[a->get_x()] - m->x_grades[b->get_x()]);
}

//returns true iff this Crossing has (exactly) the same x-coordinate as other Crossing
bool Mesh::Crossing::x_equal(const Crossing *other) const
{
    if(Mesh::almost_equal(x, other->x)) //then compare exact values
    {
        //find exact x-values
        exact x1 = (m->y_exact[a->get_y()] - m->y_exact[b->get_y()])/(m->x_exact[a->get_x()] - m->x_exact[b->get_x()]);
        exact x2 = (m->y_exact[other->a->get_y()] - m->y_exact[other->b->get_y()])/(m->x_exact[other->a->get_x()] - m->x_exact[other->b->get_x()]);

        return x1 == x2;
    }
    //otherwise, x-coordinates are not equal
    return false;
}

//CrossingComparator for ordering crossings: first by x (left to right); for a given x, then by y (low to high)
bool Mesh::CrossingComparator::operator()(const Crossing* c1, const Crossing* c2) const	//returns true if c1 comes after c2
{
    //TESTING
    if(c1->a->get_position() >= c1->b->get_position() || c2->a->get_position() >= c2->b->get_position())
    {
        debug() << "INVERTED CROSSING ERROR\n";
        debug() << "crossing 1 involves anchors " << c1->a << " (pos " << c1->a->get_position() << ") and " << c1->b << " (pos " << c1->b->get_position() << "),";
        debug() << "crossing 2 involves anchors " << c2->a << " (pos " << c2->a->get_position() << ") and " << c2->b << " (pos " << c2->b->get_position() << "),";
        throw std::exception();
    }

    Mesh* m = c1->m;    //makes it easier to reference arrays in the mesh

    //now do the comparison
    //if the x-coordinates are nearly equal as double values, then compare exact values
    if(Mesh::almost_equal(c1->x, c2->x))
    {
        //find exact x-values
        exact x1 = (m->y_exact[c1->a->get_y()] - m->y_exact[c1->b->get_y()])/(m->x_exact[c1->a->get_x()] - m->x_exact[c1->b->get_x()]);
        exact x2 = (m->y_exact[c2->a->get_y()] - m->y_exact[c2->b->get_y()])/(m->x_exact[c2->a->get_x()] - m->x_exact[c2->b->get_x()]);

        //if the x-values are exactly equal, then consider the y-values
        if(x1 == x2)
        {
            //find the y-values
            double c1y = m->x_grades[c1->a->get_x()]*(c1->x) - m->y_grades[c1->a->get_y()];
            double c2y = m->x_grades[c2->a->get_x()]*(c2->x) - m->y_grades[c2->a->get_y()];

            //if the y-values are nearly equal as double values, then compare exact values
            if(Mesh::almost_equal(c1y, c2y))
            {
                //find exact y-values
                exact y1 = m->x_exact[c1->a->get_x()]*x1 - m->y_exact[c1->a->get_y()];
                exact y2 = m->x_exact[c2->a->get_x()]*x2 - m->y_exact[c2->a->get_y()];

                //if the y-values are exactly equal, then sort by relative position of the lines
                if(y1 == y2)
                    return c1->a->get_position() > c2->a->get_position();   //Is there a better way???

                //otherwise, the y-values are not equal
                return y1 > y2;
            }
            //otherwise, the y-values are not almost equal
            return c1y > c2y;
        }
        //otherwise, the x-values are not equal
        return x1 > x2;
    }
    //otherwise, the x-values are not almost equal
    return c1->x > c2->x;
}

//epsilon value for use in comparisons
double Mesh::epsilon = pow(2,-30);

//test whether two double values are almost equal (indicating that we should do exact comparison)
bool Mesh::almost_equal(const double a, const double b)
{
    double diff = std::abs(a - b);
    if(diff <= epsilon)
        return true;

    if(diff <= (std::abs(a) + std::abs(b))*epsilon)
        return true;
    return false;
}

std::ostream & operator<<(std::ostream &stream, const Mesh &mesh) {
    return write_grades(stream, mesh.x_exact, mesh.y_exact);
}
