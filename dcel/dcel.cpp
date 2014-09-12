#include "dcel.h"


#include <iostream> //for testing only



/*** implementation of class Vertex **/

Vertex::Vertex(double theta_coord, double r_coord) :
    incident_edge(NULL),
    theta(theta_coord),
    r(r_coord)
{ }

void Vertex::set_incident_edge(Halfedge* edge)
{
    incident_edge = edge;
}

Halfedge* Vertex::get_incident_edge()
{
    return incident_edge;
}

double Vertex::get_theta()
{
    return theta;
}

double Vertex::get_r()
{
    return r;
}

std::ostream& operator<<(std::ostream& os, const Vertex& v)
{
    os << "(" << v.theta << ", " << v.r << ")";
    return os;
}


/*** implementation of class Halfedge ***/

Halfedge::Halfedge(Vertex* v, LCM* p) :
    origin(v),
    twin(NULL),
    next(NULL),
    prev(NULL),
    face(NULL),
    lcm(p)
{ }

Halfedge::Halfedge() :
    origin(NULL),
    twin(NULL),
    next(NULL),
    prev(NULL),
    face(NULL),
    lcm(NULL)
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

LCM* Halfedge::get_LCM() const
{
    return lcm;
}

std::ostream& operator<<(std::ostream& os, const Halfedge& e)
{
    Halfedge* t = e.twin;
    os << *(e.origin) << "--" << *(t->origin) << "; ";
    if(e.lcm == NULL)
        os << "LCM null; ";
    else
        os << "LCM coords (" << e.lcm->get_x() << ", " << e.lcm->get_y() << "); ";
//	os << "twin: " << (e.twin) << "; next: " << (e.next) << "; prev: " << (e.prev) << "; face: " << (e.face);
    return os;
}



/*** implementation of class Face ***/

Face::Face(Halfedge* e, int v) : boundary(e), pdata(v)
{ }

void Face::set_boundary(Halfedge* e)
{
    boundary = e;
}

Halfedge* Face::get_boundary()
{
    return boundary;
}

CellPersistenceData* Face::get_data()
{
    return &pdata;
}

void Face::store_interior_point()
{
    //find min and max theta coordinates of vertices adjacent to this face
    double min_theta = boundary->get_origin()->get_theta();
    double max_theta = min_theta;

    Halfedge* current = boundary->get_next();

    while(current != boundary)
    {
        double current_theta = current->get_origin()->get_theta();

        if(current_theta < min_theta)
            min_theta = current_theta;

        if(current_theta > max_theta)
            max_theta = current_theta;

        current = current->get_next();
    }

    //find theta coordinate between min and max theta
    double mid_theta = (min_theta + max_theta)/2;

    //find top and bottom edges that cross the vertical line theta=mid_theta
    Halfedge* top_edge;
    Halfedge* bottom_edge;

    current = boundary;
    do{
        double origin = current->get_origin()->get_theta();
        double end = current->get_next()->get_origin()->get_theta();

        if(origin <= mid_theta && end > mid_theta)
            top_edge = current;

        if(origin >= mid_theta && end < mid_theta)
            bottom_edge = current;

        current = current->get_next();
    }while(current != boundary);

    //find midpoint along vertical line theta=mid_theta
    double mid_r = 0;

    //handle boundary cases
    if(top_edge->get_LCM() == NULL)	//then top edge is at r=infinity
    {
        if(bottom_edge->get_LCM() == NULL)	//then bottom edge is also at r=infinity
        {
            mid_r = 0;
        }
        else
        {
            mid_r = bottom_edge->get_LCM()->get_r_coord(mid_theta) + 1;
        }
    }
    else	//then top edge is not at r=infinity
    {
        if(bottom_edge->get_LCM() == NULL)	//then bottom edge is at r=infinity
        {
            mid_r = top_edge->get_LCM()->get_r_coord(mid_theta) - 1;
        }
        else
        {
            double top = top_edge->get_LCM()->get_r_coord(mid_theta);
            double bottom = bottom_edge->get_LCM()->get_r_coord(mid_theta);
            mid_r = (top + bottom)/2;
        }

    }

    //store the coordinates
    pdata.set_theta(mid_theta);
    pdata.set_r(mid_r);

}//end store_interior_point()

std::ostream& operator<<(std::ostream& os, const Face& f)
{
    Halfedge* start = f.boundary;
    Halfedge* curr = start;
    do{
        os << *(curr->get_origin()) << "--";
        curr = curr->get_next();
    }while(curr != start);
    os << "cycle; ";
    return os;
}

