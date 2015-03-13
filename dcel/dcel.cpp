#include "dcel.h"


#include <iostream> //for testing only



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

std::ostream& operator<<(std::ostream& os, const Vertex& v)
{
    os << "(" << v.x << ", " << v.y << ")";
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

Face::Face(Halfedge* e) : boundary(e), dbc(NULL)
{ }

void Face::set_boundary(Halfedge* e)
{
    boundary = e;
}

Halfedge* Face::get_boundary()
{
    return boundary;
}

void Face::set_barcode(DiscreteBarcode* b)
{
    dbc = b;
}

DiscreteBarcode* Face::get_barcode()
{
    return dbc;
}

/*CellPersistenceData* Face::get_data()
{
    return &pdata;
}*/

//THIS WILL BE UNNECESSARY ONCE VINEYARDS ARE IMPLEMENTED
//THIS DOESN'T HANDLE THE CASE WHERE A CELL EXTENDS TO x = infinity
/*void Face::store_interior_point(const std::vector<double> &x_grades, const std::vector<double> &y_grades)
{
    //find min and max x-coordinates of vertices adjacent to this face
    double min_x = boundary->get_origin()->get_x();
    double max_x = min_x;

    Halfedge* current = boundary->get_next();

    while(current != boundary)
    {
        double current_x = current->get_origin()->get_x();

        if(current_x < min_x)
            min_x = current_x;

        if(current_x > max_x)
            max_x = current_x;

        current = current->get_next();
    }

    //find theta coordinate between min and max x
    double mid_x = (min_x + max_x)/2;

    //find top and bottom edges that cross the vertical line x = mid_x
    Halfedge* top_edge;
    Halfedge* bottom_edge;

    current = boundary;
    do{
        double origin = current->get_origin()->get_x();
        double end = current->get_next()->get_origin()->get_x();

        if(origin <= mid_x && end > mid_x)
            top_edge = current;

        if(origin >= mid_x && end < mid_x)
            bottom_edge = current;

        current = current->get_next();
    }while(current != boundary);

    //find midpoint along vertical line x = mid_x
    double mid_y = 0;

    //handle boundary cases
    if(top_edge->get_LCM() == NULL)	//then top edge is at y = infinity
    {
        if(bottom_edge->get_LCM() == NULL)	//then bottom edge is at y = -infinity
        {
            mid_y = 0;
        }
        else
        {
            LCM* temp = bottom_edge->get_LCM();
            mid_y = (x_grades[temp->get_x()]*mid_x - y_grades[temp->get_y()]) + 1;
        }
    }
    else	//then top edge is not at y = infinity
    {
        if(bottom_edge->get_LCM() == NULL)	//then bottom edge is at y = -infinity
        {
            LCM* temp = top_edge->get_LCM();
            mid_y = (x_grades[temp->get_x()]*mid_x - y_grades[temp->get_y()]) - 1;
        }
        else //then neither top nor bottom edge is at infinity
        {
            LCM* lcm_top = top_edge->get_LCM();
            LCM* lcm_bot = bottom_edge->get_LCM();

            double top = (x_grades[lcm_top->get_x()]*mid_x - y_grades[lcm_top->get_y()]);
            double bottom = (x_grades[lcm_bot->get_x()]*mid_x - y_grades[lcm_bot->get_y()]);
            mid_y = (top + bottom)/2;
        }

    }

    //store the coordinates
    pdata.set_x(mid_x);
    pdata.set_y(mid_y);

}//end store_interior_point()*/

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

