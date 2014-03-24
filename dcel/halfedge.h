/**
 * \class	Halfedge
 * \brief	Stores one halfedge in the DCEL decomposition of the affine Grassmannian; two halfedges make one edge.
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_Halfedge_H__
#define __DCEL_Halfedge_H__

//includes????  namespace????
#include <utility>

class Vertex;
class Face;

class Halfedge
{
	public:
		Halfedge(Vertex* v, double time, double dist);	//constructor, requires origin vertex as well as time and distance coordinates for the LCM corresponding to this halfedge (LCM never changes)
		
		void set_twin(Halfedge* e);	//set the twin halfedge
		Halfedge* get_twin();		//get the twin halfedge
		
		void set_next(Halfedge* e);	//set the next halfedge in the boundary of the face that this halfedge borders
		Halfedge* get_next();		//get the next halfedge
		
		void set_prev(Halfedge* e);	//set the previous halfedge in the boundary of the face that this halfedge borders
		Halfedge* get_prev();		//get the previous halfedge
		
		//void set_origin(Vertex* v); -- I don't think we will ever change the origin vertex
		Vertex* get_origin();		//get the origin vertex
		
		void set_face(Face* f);		//get the face that this halfedge borders
		Face* get_face();		//set the face that this halfedge borders
		
		std::pair<double, double> get_LCM();	//get the LCM coordinates; ---> WARNING: THIS MAY UNNECESSARILY COPY A PAIR OBJECT
		
		friend std::ostream& operator<<(std::ostream& os, const Halfedge& e);	//for printing the halfedge
		
	private:
		Vertex* origin;		//pointer to the vertex from which this halfedge originates
		Halfedge* twin;		//pointer to the halfedge that, together with this halfedge, make one edge
		Halfedge* next;		//pointer to the next halfedge around the boundary of the face to the right of this halfedge
		Halfedge* prev;		//pointer to the previous halfedge around the boundary of the face to the right of this halfedge
		Face* face;		//pointer to the face to the right of this halfedge
		std::pair<double, double> lcm_coords;	//coordinates (in the two-parameter persistence space) of the LCM corresponding to this halfedge
	
};//end class Halfedge


////////// implementation //////////

Halfedge::Halfedge(Vertex* v, double time, double dist) : lcm_coords(time, dist), face(NULL)	//IS THIS WHAT WE WANT???
{
	origin = v;
}

void Halfedge::set_twin(Halfedge* e)
{
	twin = e;
}

Halfedge* Halfedge::get_twin()
{
	return twin;
}

void Halfedge::set_next(Halfedge* e)
{
	next = e;
}

Halfedge* Halfedge::get_next()
{
	return next;
}

void Halfedge::set_prev(Halfedge* e)
{
	prev = e;
}

Halfedge* Halfedge::get_prev()
{
	return prev;
}

Vertex* Halfedge::get_origin()
{
	return origin;
}

void Halfedge::set_face(Face* f)
{
	face = f;
}

Face* Halfedge::get_face()
{
	return face;
}

std::pair<double, double> Halfedge::get_LCM()
{
	return lcm_coords;
}

std::ostream& operator<<(std::ostream& os, const Halfedge& e)
{
	Halfedge* t = e.twin;
	os << *(e.origin) << "--" << *(t->origin) << ", LCM coords (" << e.lcm_coords.first << ", " << e.lcm_coords.second << ") face:" << (e.face);
	return os;
}


#endif // __DCEL_Halfedge_H__

